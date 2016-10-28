#ifndef MYY_HELPERS_OPENGL
#define MYY_HELPERS_OPENGL 1

#include <GLES2/gl2.h>
#include <helpers/struct.h>

struct BUS_textured_point_2D { GLushort s, t; GLbyte x, y; } __PALIGN__;
struct BUS_textured_triangle_2D { struct BUS_textured_point_2D a, b, c; } __PALIGN__;
struct BUS_two_triangles_textured_quad_2D { struct BUS_textured_triangle_2D first, second; } __PALIGN__;

union BUS_two_triangles_textured_quad_2D_representations {
  struct BUS_two_triangles_textured_quad_2D quad;
  struct BUS_textured_triangle_2D triangles[2];
  struct BUS_textured_point_2D points[6];
} __PALIGN__;

typedef union BUS_two_triangles_textured_quad_2D_representations BUS_two_tris_quad;

#define TWO_BYTES_TRIANGLES_TEX_QUAD(left, right, down, up, left_tex, right_tex, down_tex, up_tex) { \
  .points = { \
    { .s = left_tex,  .t = up_tex,   .x = left,  .y = up,  },  \
    { .s = left_tex,  .t = down_tex, .x = left,  .y = down },  \
    { .s = right_tex, .t = up_tex,   .x = right, .y = up,  },  \
    { .s = right_tex, .t = down_tex, .x = right, .y = down },  \
    { .s = right_tex, .t = up_tex,   .x = right, .y = up,  },  \
    { .s = left_tex,  .t = down_tex, .x = left,  .y = down },  \
  } \
}

#define TWO_TRIANGLES_TEX_QUAD(left, right, down, up, left_tex, right_tex, down_tex, up_tex) { \
  .points = { \
    { .x = left,  .y = up,   .s = left_tex,  .t = up_tex},   \
    { .x = left,  .y = down, .s = left_tex,  .t = down_tex}, \
    { .x = right, .y = up,   .s = right_tex, .t = up_tex},   \
    { .x = right, .y = down, .s = right_tex, .t = down_tex}, \
    { .x = right, .y = up,   .s = right_tex, .t = up_tex},   \
    { .x = left,  .y = down, .s = left_tex,  .t = down_tex}  \
  } \
}

enum quad_coords_order {
  upleft_corner, downleft_corner, upright_corner, downright_corner,
  repeated_upright_corner, repeated_downleft_corner, two_triangles_corners
};

struct gleanup {
  void (*check)(GLuint, GLenum, GLint* );
  int verif;
  void (*log)(GLuint, GLsizei, GLsizei*, GLchar*);
};

struct textures {
  int name_i;
  int size;
  int width;
  int height;
  GLuint GLformat;
  GLuint GLtype;
};

#define GL_SHADER_PROBLEMS 0
#define GL_PROGRAM_PROBLEMS 1
int glhLoadShader(GLenum shaderType, const char *name, GLuint program);
GLuint glhSetupProgram(
  const char* vsh_filename, const char* fsh_filename, uint8_t n_attributes,
  const char* attributes_names);
GLuint glhSetupAndUse(const char* vsh_filename, const char* fsh_filename,
                      uint8_t n_attributes, const char* attributes_names);
void uploadTextures(const char *textures_names, int n, GLuint *texid);

void copy_BUS_triangles_quad_with_offset
(BUS_two_tris_quad* mdl, GLbyte x_offset,
 GLbyte y_offset, BUS_two_tris_quad* cpy);

#endif
