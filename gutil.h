#ifndef GUTIL_H
#define GUTIL_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdbool.h>

#define GUTIL_CONTEXT_VERSION_MAJOR 3
#define GUTIL_CONTEXT_VERSION_MINOR 3
#define GUTIL_OPENGL_FORWARD_COMPAT GL_TRUE
#define GUTIL_OPENGL_PROFILE        GLFW_OPENGL_CORE_PROFILE
#define GUTIL_GLEW_EXPERIMENTAL     GL_TRUE

void init_glfw(void);
void exit_glfw(void);
GLFWwindow *mk_win(int w, int h, const char *title, bool rsz, bool fscr);
GLFWwindow *rm_win(GLFWwindow *win);
GLuint mk_vao(void);
GLuint rm_vao(GLuint vao);
GLuint mk_prg(int n_shds, const char **srcs, const GLenum *types,
              int n_vars, const char **names, GLint *locs);
GLuint rm_prg(GLuint prg);
GLuint mk_vbo(GLenum type, size_t sz, void *data, GLenum usage);
GLuint rm_vbo(GLuint vbo);

#endif // GUTIL_H
