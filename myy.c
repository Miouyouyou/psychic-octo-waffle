#include "myy.h"

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <helpers/base_gl.h>
#include <helpers/struct.h>

#include <stddef.h>

#define GLCARD TWO_BYTES_TRIANGLES_TEX_QUAD

/**** CARD MODEL ****/
// Comments use a 1080p screen as a basis
#define CARD_WIDTH ((int8_t) (0.090625f*127)) // 174px - ~Bridge ratio
#define CARD_HEIGHT ((int8_t) (0.25f*127)) // 270px - ~Bridge ratio
#define CARD_ALPHA_PART_HEIGHT ((int8_t) (0.05f*127)) // 13.5px * 2 (Two parts)

#define CARD_TEX_LEFT 24
#define CARD_TEX_RIGHT 4071
#define CARD_TEX_ALPHA_BOTTOM_BOTTOM 60
#define CARD_TEX_ALPHA_BOTTOM_TOP 1639
#define CARD_TEX_ALPHA_TOP_BOTTOM 14745
#define CARD_TEX_ALPHA_TOP_TOP 16323
GLubyte v = 13, st = 0;
struct GLcard { BUS_two_tris_quad opaque, top, bottom; } gl_cards_parts = {
  .opaque = GLCARD(
    /* Coords : Left, Right, Down, Up - Tex : Left, Right, Down Up */
    -CARD_WIDTH, CARD_WIDTH,
    -(CARD_HEIGHT-CARD_ALPHA_PART_HEIGHT),
    (CARD_HEIGHT-CARD_ALPHA_PART_HEIGHT),
    CARD_TEX_LEFT, CARD_TEX_RIGHT,
    CARD_TEX_ALPHA_BOTTOM_TOP,
    CARD_TEX_ALPHA_TOP_BOTTOM
  ),
  .top = GLCARD(
    -CARD_WIDTH, CARD_WIDTH,
    (CARD_HEIGHT-CARD_ALPHA_PART_HEIGHT), CARD_HEIGHT,
    CARD_TEX_LEFT, CARD_TEX_RIGHT,
    CARD_TEX_ALPHA_TOP_BOTTOM,
    CARD_TEX_ALPHA_TOP_TOP
  ),
  .bottom = GLCARD(
    -CARD_WIDTH, CARD_WIDTH,
    -CARD_HEIGHT, -(CARD_HEIGHT-CARD_ALPHA_PART_HEIGHT),
    CARD_TEX_LEFT, CARD_TEX_RIGHT,
    CARD_TEX_ALPHA_BOTTOM_BOTTOM,
    CARD_TEX_ALPHA_BOTTOM_TOP
  )
};

typedef struct GLcard GLcard;

void
offseted_GLcard_copy
(GLbyte x_offset, GLbyte y_offset, GLbyte value, GLbyte suit,
 GLcard *mdl, GLcard* cpy) {
  GLushort 
    s_offset = 4096 * value,
    t_offset = 16383 * suit;

  for (int i = 0; i < two_triangles_corners; i ++) {
    cpy->opaque.points[i].s = mdl->opaque.points[i].s + s_offset;
    cpy->opaque.points[i].t = mdl->opaque.points[i].t + t_offset;
    cpy->opaque.points[i].x = mdl->opaque.points[i].x + x_offset;
    cpy->opaque.points[i].y = mdl->opaque.points[i].y + y_offset;
  }
  for (int i = 0; i < two_triangles_corners; i ++) {
    cpy->top.points[i].s = mdl->top.points[i].s + s_offset;
    cpy->top.points[i].t = mdl->top.points[i].t + t_offset;
    cpy->top.points[i].x = mdl->top.points[i].x + x_offset;
    cpy->top.points[i].y = mdl->top.points[i].y + y_offset;
  }
  for (int i = 0; i < two_triangles_corners; i ++) {
    cpy->bottom.points[i].s = mdl->bottom.points[i].s + s_offset;
    cpy->bottom.points[i].t = mdl->bottom.points[i].t + t_offset;
    cpy->bottom.points[i].x = mdl->bottom.points[i].x + x_offset;
    cpy->bottom.points[i].y = mdl->bottom.points[i].y + y_offset;
  }
  
}

/**** The actual code ****/
enum attributes { attr_xy, attr_st, attrs_n };

void generate_cards_coords_in_buffer(GLint *buffer_id) {
  GLcard cards[2];
  offseted_GLcard_copy(-5, 0, v, st, &gl_cards_parts, cards+0);
  offseted_GLcard_copy(31, 0, v, st, &gl_cards_parts, cards+1);

  glGenBuffers(1, buffer_id);
  glBindBuffer(GL_ARRAY_BUFFER, *buffer_id);
  glBufferData(GL_ARRAY_BUFFER, 2*sizeof(GLcard), cards, GL_DYNAMIC_DRAW);
}

GLint buffer_id;
void myy_init() {
  GLint tex_id;
  uploadTextures("tex/cards.raw", 1, &tex_id);
  GLuint program =
    glhSetupAndUse("shaders/standard.vert", "shaders/standard.frag",
                   attrs_n, "xy\0st");
  glUniform1i(glGetUniformLocation(program, "sid"), 0);

  generate_cards_coords_in_buffer(&buffer_id);

  glEnableVertexAttribArray(attr_st);
  glEnableVertexAttribArray(attr_xy);

  glBindBuffer(GL_ARRAY_BUFFER, buffer_id);

  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glCullFace(GL_BACK);
  glDepthFunc(GL_LESS);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void myy_draw() {
  glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT );
  glClearColor(0.8f,0.8f,0.8f,1.0f);

  glVertexAttribPointer(attr_st, 2, GL_UNSIGNED_SHORT, GL_TRUE, sizeof(struct BUS_textured_point_2D), 0);
  glVertexAttribPointer(attr_xy, 2, GL_BYTE, GL_TRUE, sizeof(struct BUS_textured_point_2D), (void *) offsetof(struct BUS_textured_point_2D, x));
  /* 6 points per quad, 3 quads per card, 2 cards */
  glDrawArrays(GL_TRIANGLES, 0, 6*3*2);
}

void myy_save() {}
void myy_stop() {}
void myy_animating() {}
void myy_pause() {}
void myy_resume() {}
void myy_click(int x, int y, unsigned int button) {}
void myy_move(int x, int y) {}

#define KEY_UP 111
#define KEY_DOWN 116
#define KEY_LEFT 113
#define KEY_RIGHT 114
void myy_key(unsigned int keycode) {
  switch(keycode) {
  case KEY_UP:   st += 1; break;
  case KEY_DOWN: st -= 1; break;
  case KEY_LEFT:  v -= 1; break;
  case KEY_RIGHT: v += 1; break;
  }
  st &= 0x3;
  v &= 0xf;
  GLcard cards[2];
  offseted_GLcard_copy(-5, 0, v, st, &gl_cards_parts, cards+0);
  offseted_GLcard_copy(31, 0, v, st, &gl_cards_parts, cards+1);
  
  glBindBuffer(GL_ARRAY_BUFFER, buffer_id);
  glBufferSubData(GL_ARRAY_BUFFER, 0, 2*sizeof(GLcard), cards);
}
void myy_display_initialised(int width, int height) {}
