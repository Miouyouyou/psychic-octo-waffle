#ifndef MYY_HELPERS_OPENGL
#define MYY_HELPERS_OPENGL 1

#include <GLES2/gl2.h>
#include <helpers/struct.h>

struct point_2D { GLfloat x, y; } __PALIGN__;
struct point_3D { GLfloat x, y, z; } __PALIGN__;
struct triangle_2D { struct point_2D a, b, c; } __PALIGN__;
struct layered_triangle_2D { struct point_3D a, b, c; } __PALIGN__;
struct two_triangles_quad_2D { struct triangle_2D first, second; } __PALIGN__;
struct two_layered_triangles_quad_2D { struct layered_triangle_2D first, second; } __PALIGN__;

struct textured_point_2D { GLfloat x, y, s, t; } __PALIGN__;
struct textured_point_3D { GLfloat x, y, z, s, t; } __PALIGN__;
struct textured_triangle_2D { struct textured_point_2D a, b, c; } __PALIGN__;
struct textured_layered_triangle_2D { struct textured_point_3D a, b, c; } __PALIGN__;
struct two_triangles_textured_quad_2D {
  struct textured_triangle_2D first, second;
} __PALIGN__;
struct two_textured_layered_triangles_quad {
  struct textured_layered_triangle_2D first, second;
} __PALIGN__;

union two_triangles_textured_quad_2D_representations {
  struct two_triangles_textured_quad_2D quad;
  struct textured_triangle_2D triangles[2];
  struct textured_point_2D points[6];
  GLfloat raw_coords[24];
} __PALIGN__;

union two_textured_layered_triangles_quad_representations {
  struct two_textured_layered_triangles_quad quad;
  struct textured_layered_triangle_2D triangles[2];
  struct textured_point_3D points[6];
  GLfloat raw_coords[30];
} __PALIGN__;

typedef union two_triangles_textured_quad_2D_representations two_tris_quad;
typedef union two_textured_layered_triangles_quad_representations two_layered_tris_quad;

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
void copy_two_triangles_quad_with_offset(GLfloat *model_coords,
                                         GLfloat x_offset, GLfloat y_offset,
                                         GLfloat *card_copy_coords);
void copy_quad_to_offseted_layered_quad(GLfloat *card_copy_coords,
                                        GLfloat *model_coords,
                                        GLfloat x_offset, GLfloat y_offset,
                                        GLfloat z_layer);
void copy_quad_to_scaled_offseted_layered_quad
(GLfloat *card_copy_coords, GLfloat *model_coords,
 GLfloat x_offset, GLfloat y_offset, GLfloat z_layer, GLfloat scale);
#endif
