#include "gutil.h"

#include <stdlib.h>

static GLuint mk_shd(const char *src, GLenum type);
static GLuint rm_shd(GLuint shd);

void init_glfw(void)
{
    glfwInit();
}

void exit_glfw(void)
{
    glfwTerminate();
}

GLFWwindow *mk_win(int w, int h, const char *title, bool rsz, bool fscr)
{
    glfwWindowHint(GLFW_RESIZABLE, rsz);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, GUTIL_CONTEXT_VERSION_MAJOR);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, GUTIL_CONTEXT_VERSION_MINOR);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GUTIL_OPENGL_FORWARD_COMPAT);
    glfwWindowHint(GLFW_OPENGL_PROFILE,        GUTIL_OPENGL_PROFILE);
    glewExperimental =                         GUTIL_GLEW_EXPERIMENTAL;
    GLFWmonitor *mon = fscr ? glfwGetPrimaryMonitor() : NULL;
    GLFWwindow  *win = glfwCreateWindow(w, h, title, mon, NULL);
    glfwMakeContextCurrent(win);
    glewInit();
    return win;
}

GLFWwindow *rm_win(GLFWwindow *win)
{
    glfwDestroyWindow(win);
    return NULL;
}

GLuint mk_vao(void)
{
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    return vao;
}

GLuint rm_vao(GLuint vao)
{
    glDeleteVertexArrays(1, &vao);
    return 0;
}

GLuint mk_prg(int n_shds, const char **srcs, const GLenum *types,
              int n_vars, const char **names, GLint *locs)
{
    GLuint prg = glCreateProgram();
    GLuint *shds = malloc(n_shds * sizeof(*shds));
    for (int i = 0; i < n_shds; ++i) shds[i] = mk_shd(srcs[i], types[i]);

    for (int i = 0; i < n_shds; ++i) glAttachShader(prg, shds[i]);
    glLinkProgram(prg);
    for (int i = 0; i < n_shds; ++i) glDetachShader(prg, shds[i]);

    glValidateProgram(prg);
    glUseProgram(prg);
    for (int i = 0; i < n_vars; ++i)locs[i]=glGetUniformLocation(prg,names[i]);

    for (int i = 0; i < n_shds; ++i) shds[i] = rm_shd(shds[i]);
    free(shds);
    return prg;
}

GLuint rm_prg(GLuint prg)
{
    glDeleteProgram(prg);
    return 0;
}

static GLuint mk_shd(const char *src, GLenum type)
{
    GLuint shd = glCreateShader(type);
    glShaderSource(shd, 1, &src, NULL);
    glCompileShader(shd);
    return shd;
}

static GLuint rm_shd(GLuint shd)
{
    glDeleteShader(shd);
    return 0;
}

GLuint mk_vbo(GLenum type, size_t sz, void *data, GLenum usage)
{
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(type, vbo);
    glBufferData(type, sz, data, usage);
    return vbo;
}

GLuint rm_vbo(GLuint vbo)
{
    glDeleteBuffers(1, &vbo);
    return 0;
}
