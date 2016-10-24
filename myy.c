#include "myy.h"

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <helpers/base_gl.h>
#include <helpers/struct.h>

#define GLCARD TWO_TRIANGLES_TEX_QUAD

/**** CARD MODEL ****/
// Comments use a 1080p screen as a basis
#define CARD_WIDTH 0.090625f // 174px - ~Bridge ratio
#define CARD_HEIGHT 0.25f // 270px - ~Bridge ratio
#define CARD_ALPHA_PART_HEIGHT 0.05f // 13.5px * 2 (Two parts)

#define CARD_X 14
#define CARD_Y 1

#define CARD_TEX_WIDTH 0.0625f
#define CARD_TEX_HEIGHT 0.25f
#define CARD_ALPHA_PART_TEX_HEIGHT 0.025f

#define CARD_TEX_LEFT (CARD_TEX_WIDTH*CARD_X)
#define CARD_TEX_RIGHT (CARD_TEX_WIDTH*(CARD_X+1))

/* Splitting cards mesh in two alpha parts and one opaque part */
#define CARD_TEX_ALPHA_BOTTOM_BOTTOM (CARD_TEX_HEIGHT*CARD_Y)
#define CARD_TEX_ALPHA_BOTTOM_TOP (CARD_TEX_HEIGHT*CARD_Y+CARD_ALPHA_PART_TEX_HEIGHT)
#define CARD_TEX_ALPHA_TOP_TOP (CARD_TEX_HEIGHT*(CARD_Y+1))
#define CARD_TEX_ALPHA_TOP_BOTTOM (CARD_TEX_ALPHA_TOP_TOP-CARD_ALPHA_PART_TEX_HEIGHT)
#define CARD_TEX_OPAQUE_BOTTOM (CARD_TEX_ALPHA_BOTTOM_TOP)
#define CARD_TEX_OPAQUE_TOP (CARD_TEX_ALPHA_TOP_BOTTOM)

struct GLcard { two_tris_quad opaque, top, bottom; } gl_cards_parts = {
  .opaque = GLCARD(
    /* Coords : Left, Right, Down, Up - Tex : Left, Right, Down Up */
    -CARD_WIDTH, CARD_WIDTH,
    -(CARD_HEIGHT-CARD_ALPHA_PART_HEIGHT),
     (CARD_HEIGHT-CARD_ALPHA_PART_HEIGHT),
     CARD_TEX_LEFT, CARD_TEX_RIGHT,
     CARD_TEX_OPAQUE_BOTTOM,
     CARD_TEX_OPAQUE_TOP
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
offseted_GLcard_copy(GLcard *model, GLcard *card,
                     GLfloat x_offset, GLfloat y_offset) {
  copy_two_triangles_quad_with_offset(
    gl_cards_parts.opaque.raw_coords, x_offset, y_offset,
    card->opaque.raw_coords
  );
  copy_two_triangles_quad_with_offset(
    gl_cards_parts.top.raw_coords, x_offset, y_offset,
    card->top.raw_coords
  );
  copy_two_triangles_quad_with_offset(
    gl_cards_parts.bottom.raw_coords, x_offset, y_offset,
    card->bottom.raw_coords
  );
}

/**** The actual code ****/
enum attributes { attr_xyst, attrs_n };

void generate_cards_coords_in_buffer(GLint *buffer_id) {
  GLcard cards[2];
  offseted_GLcard_copy(&gl_cards_parts, cards+0, -0.02f, 0);
  offseted_GLcard_copy(&gl_cards_parts, cards+1, 0.24f, 0);

  glGenBuffers(1, buffer_id);
  glBindBuffer(GL_ARRAY_BUFFER, *buffer_id);
  glBufferData(GL_ARRAY_BUFFER, 2*sizeof(GLcard),
               (GLfloat *) cards, GL_STATIC_DRAW);
}

void myy_init() {
  GLint tex_id;
  uploadTextures("tex/cards.raw", 1, &tex_id);
  GLuint program =
    glhSetupAndUse("shaders/standard.vert", "shaders/standard.frag",
                   attrs_n, "xyst");
  glUniform1i(glGetUniformLocation(program, "sid"), 0);

  GLint buffer_id;
  generate_cards_coords_in_buffer(&buffer_id);

  glEnableVertexAttribArray(attr_xyst);
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

  glVertexAttribPointer(attr_xyst, 4, GL_FLOAT, GL_FALSE, 0, 0);
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
void myy_key(unsigned int keycode) {}
void myy_display_initialised(int width, int height) {}
