#include <GLES2/gl2.h>

#include <helpers/base_gl.h>
#include <helpers/file.h>
#include <helpers/string.h>
#include <helpers/log.h>

#include <sys/types.h> // read, write, fstat, open
#include <sys/stat.h> // fstat, open
#include <unistd.h> // read, write, fstat, get_current_dir_name
#include <stdlib.h> // exit
#include <fcntl.h> // open


#define SCRATCH_SPACE 3555344
static uint8_t scratch[SCRATCH_SPACE+1];

static const struct gleanup cleanupMethods[] = {
  {
    .check = glGetShaderiv,
    .verif = GL_COMPILE_STATUS,
    .log   = glGetShaderInfoLog
  },
  {
    .check = glGetProgramiv,
    .verif = GL_LINK_STATUS,
    .log   = glGetProgramInfoLog
  }
};

static int check_if_ok(GLuint element_verifie,
                       GLuint type_methode) {
  struct gleanup gheckup = cleanupMethods[type_methode];

  GLint ok = GL_FALSE;
  gheckup.check(element_verifie, gheckup.verif, &ok);

  if (ok == GL_TRUE) return ok;

  int written = 0;
  gheckup.log(element_verifie, SCRATCH_SPACE, &written, scratch);
  scratch[written] = 0;
  LOG("Problem was : %s\n", scratch);

  return GL_FALSE;
}

int glhLoadShader(GLenum shaderType, const char *name, GLuint program) {

  LOG("Shader : %s - Type : %d\n", name, shaderType);
  GLuint shader = glCreateShader(shaderType);
  LOG("Loading shader : %s - glCreateShader : %d\n", name, shader);
  GLuint ok = 0;

  if (shader) {
    LOG("Shader %s seems ok...\n", name);
    fh_FileToStringBuffer(name, (char *) scratch, SCRATCH_SPACE);
    const char *pSource = scratch;
    glShaderSource(shader, 1, &pSource, NULL);
    glCompileShader(shader);
    ok = check_if_ok(shader, GL_SHADER_PROBLEMS);
    if (ok) glAttachShader(program, shader);
    glDeleteShader(shader);
  }
  LOG("Shader %s -> Status : %d\n", name, program);
  return ok;
}

GLuint glhSetupProgram(const char* vsh_filename, const char* fsh_filename,
                       uint8_t n_attributes, const char* attributes_names) {
  GLuint p = glCreateProgram();

  /* Shaders */
  if (glhLoadShader(GL_VERTEX_SHADER,   vsh_filename, p) &&
      glhLoadShader(GL_FRAGMENT_SHADER, fsh_filename, p)) {

    LOG("Shaders loaded\n");
    const char *bound_attribute_name = attributes_names;
    for (uint32_t i = 0; i < n_attributes; i++) {
      glBindAttribLocation(p, i, bound_attribute_name);
      LOG("Attrib : %s - Location : %d\n", bound_attribute_name, i);
      sh_pointToNextString(bound_attribute_name);
    }
    glLinkProgram(p);
    if (check_if_ok(p, GL_PROGRAM_PROBLEMS)) return p;
  }
  LOG("A problem occured during the creation of the program\n");
  return 0;

}

GLuint glhSetupAndUse(const char* vsh_filename, const char* fsh_filename,
                      uint8_t n_attributes, const char* attributes_names) {
  GLuint p =
    glhSetupProgram(vsh_filename, fsh_filename, n_attributes, attributes_names);
  glUseProgram(p);
  return p;
}

static void setupTexture() {
  glGenerateMipmap(GL_TEXTURE_2D);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

void uploadTextures(const char *textures_names, int n, GLuint *texid) {
  /* OpenGL 2.x way to load textures is certainly NOT intuitive !
   * From what I understand :
   * - The current activated texture unit is changed through
   *   glActiveTexture.
   * - glGenTextures will generate names for textures *storage* units.
   * - glBindTexture will bind the current *storage* unit to the current
   *   activated texture unit and, on the first time, will define the
   *   current *storage* unit parameters.
   *   Example : This storage unit must store GL_TEXTURE_2D textures.
   * - glTexImage2D will upload the provided data in the texture *storage* unit
   *   bound to the current texture unit.
   */

  glGenTextures(n, texid);

  const char *current_name = textures_names;

  for (int i = 0; i < n; i++) {
    /* glTexImage2D
       Specifies a two-dimensional or cube-map texture for the current
       texture unit, specified with glActiveTexture. */

    LOG("Loading texture : %s\n", current_name);

    struct stat boeuf;
    int fd = open(current_name, O_RDONLY);
    if (fd != -1) {
      fstat(fd, &boeuf);
      uint32_t width, height, gl_format, gl_type;
      read(fd, &width, 4);
      read(fd, &height, 4);
      read(fd, &gl_format, 4);
      read(fd, &gl_type, 4);
      LOG("Read %zd bytes\n", read(fd, scratch, boeuf.st_size));
      close(fd);
      glBindTexture(GL_TEXTURE_2D, texid[i]);
      glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, gl_format, gl_type, scratch);
      setupTexture();
      sh_pointToNextString(current_name);
    }
    else {
      LOG("You're sure about that file : %s ?\n", current_name);
      char *current_dir_name = getcwd(malloc(256), 256);
      if (current_dir_name) {
        LOG("--- Current directory : %s\n", current_dir_name);
        free(current_dir_name);
      }
      exit(1);
    }
  }
}

void copy_two_triangles_quad_with_offset
(GLfloat *model_coords, GLfloat x_offset, GLfloat y_offset,
 GLfloat *card_copy_coords) {
  two_tris_quad
    *mdl    = (two_tris_quad *) model_coords,
    *c_copy = (two_tris_quad *) card_copy_coords;
  for (int i = 0; i < two_triangles_corners; i ++) {
    c_copy->points[i].x = mdl->points[i].x + x_offset;
    c_copy->points[i].y = mdl->points[i].y + y_offset;
    c_copy->points[i].s = mdl->points[i].s;
    c_copy->points[i].t = mdl->points[i].t;
  }
}

void copy_two_bytes_triangles_quad_with_offset
(two_BF_tris_quad* mdl, GLbyte x_offset,
 GLbyte y_offset, two_BF_tris_quad* cpy) {

  for (int i = 0; i < two_triangles_corners; i ++) {
    cpy->points[i].s = mdl->points[i].s;
    cpy->points[i].t = mdl->points[i].t;
    cpy->points[i].x = mdl->points[i].x + x_offset;
    cpy->points[i].y = mdl->points[i].y + y_offset;
  }

}

void copy_quad_to_offseted_layered_quad
(GLfloat *card_copy_coords, GLfloat *model_coords,
 GLfloat x_offset, GLfloat y_offset, GLfloat z_layer) {

   /*LOG("copy_coords : %p, model_coords : %p, x: %f, y: %f, z: %f\n",
       card_copy_coords, model_coords, x_offset, y_offset, z_layer);*/

  two_tris_quad *mdl    = (two_tris_quad *) model_coords;
  two_layered_tris_quad *c_copy = (two_layered_tris_quad *) card_copy_coords;

  for (int i = 0; i < two_triangles_corners; i ++) {
    c_copy->points[i].x = mdl->points[i].x + x_offset;
    c_copy->points[i].y = mdl->points[i].y + y_offset;
    c_copy->points[i].z = z_layer;
    c_copy->points[i].s = mdl->points[i].s;
    c_copy->points[i].t = mdl->points[i].t;
    /*LOG("copy_coords[%d] : x: %f, y: %f, z: %f, s: %f, t: %f\n",
        i, c_copy->points[i].x, c_copy->points[i].y, c_copy->points[i].z,
        c_copy->points[i].s, c_copy->points[i].t);
    LOG("model_coords[%d] : x: %f, y: %f, s: %f, t: %f\n",
        i, mdl->points[i].x, mdl->points[i].y, mdl->points[i].s, mdl->points[i].t);*/
  }

}

void copy_quad_to_scaled_offseted_layered_quad
(GLfloat *card_copy_coords, GLfloat *model_coords,
  GLfloat x_offset, GLfloat y_offset, GLfloat z_layer, GLfloat scale) {

    LOG("copy_coords : %p, model_coords : %p, x: %f, y: %f, z: %f, scale: %f\n",
        card_copy_coords, model_coords, x_offset, y_offset, z_layer, scale);
    two_tris_quad *mdl    = (two_tris_quad *) model_coords;
    two_layered_tris_quad *c_copy = (two_layered_tris_quad *) card_copy_coords;

    for (int i = 0; i < two_triangles_corners; i ++) {
      c_copy->points[i].x = mdl->points[i].x * scale + x_offset;
      c_copy->points[i].y = mdl->points[i].y * scale + y_offset;
      c_copy->points[i].z = z_layer;
      c_copy->points[i].s = mdl->points[i].s;
      c_copy->points[i].t = mdl->points[i].t;
      LOG("copy_coords[%d] : x: %f, y: %f, z: %f, s: %f, t: %f\n",
          i, c_copy->points[i].x, c_copy->points[i].y, c_copy->points[i].z,
          c_copy->points[i].s, c_copy->points[i].t);
      LOG("model_coords[%d] : x: %f, y: %f, s: %f, t: %f\n",
          i, mdl->points[i].x, mdl->points[i].y, mdl->points[i].s, mdl->points[i].t);
    }
 }
