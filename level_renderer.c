#include "level_renderer.h"

#include "level_t.h"
#include "gutil.h"

#include <string.h>
#include <stdlib.h>

#define WIN_TITLE   "Bounce"
#define INDEX_T     GLubyte
#define INDEX_MACRO GL_UNSIGNED_BYTE

const char *VERT =
"#version 330 core\n"
"layout (location = 0) in vec2 pos;\n"
"void main(void) { gl_Position = vec4(pos, 0, 1); }\n";

const char *FRAG =
"#version 330 core\n"
"out vec4 colour;\n"
"uniform float r, g, b;\n"
"void main(void) { colour = vec4(r, g, b, 1); }\n";

typedef enum PRG_VARS
{
    VAR_R,
    VAR_G,
    VAR_B,
    N_VARS
} PRG_VARS;

typedef struct object_t
{
    int n_indices, n_points;
    GLuint indices, points;
} object_t;

typedef struct renderer_t
{
    GLFWwindow *win;
    GLuint vao, prg;
    GLint locs[N_VARS];
    object_t fg, hero, dest;
} renderer_t;

typedef struct rect_t
{
    int x, y, w, h;
} rect_t;

typedef struct point_t
{
    int x, y;
} point_t;

static renderer_t ren;

static void init_fg(void);
static rect_t expand_rect(int x, int y);
static bool is_row(rect_t rect);
static void delete_rect(rect_t rect, intmap_t dmp);
static void attach_rect(rect_t rect, intmap_t imp, INDEX_T**is, vector_t**pts);
static void attach_point(point_t p, intmap_t imp, vector_t **points);
static void attach_index(point_t p, intmap_t imp, INDEX_T **indices);
static void mk_vbos(object_t *obj, INDEX_T *is, vector_t *pts, GLenum usage);
static void init_hero(void);
static void init_dest(void);
static void exit_obj(object_t *obj);
static void draw_obj(object_t obj, GLenum mode, colour_t rgb);
static void adjust_hero_position(void);

void init_renderer(void)
{
    const char   *SRCS[] = {VERT, FRAG};
    const GLenum TYPES[] = {GL_VERTEX_SHADER, GL_FRAGMENT_SHADER};
    const char  *NAMES[] = {"r", "g", "b"};

    init_glfw();
    ren.win = mk_win(lvl.win_w, lvl.win_h, WIN_TITLE, false, lvl.win_fscr);
    ren.vao = mk_vao();
    glEnableVertexAttribArray(0);
    ren.prg = mk_prg(2, SRCS, TYPES, N_VARS, NAMES, ren.locs);
    glClearColor(lvl.bg_clr.r, lvl.bg_clr.g, lvl.bg_clr.b, 1.0f);
    init_fg();
    init_hero();
    init_dest();
}

static void init_fg(void)
{
    size_t sz = lvl.bmp.w * lvl.bmp.h * sizeof(*lvl.bmp.i); // for deletion map
    int imp_w = lvl.bmp.w + 1, imp_h = lvl.bmp.h + 1; // for indices map
    intmap_t dmp = {lvl.bmp.w, lvl.bmp.h, memcpy(malloc(sz), lvl.bmp.i, sz)};
    intmap_t imp = {imp_w, imp_h, calloc(imp_w * imp_h, sizeof(*imp.i))};
    INDEX_T *indices = NULL;
    vector_t *points = NULL;

    for (int y = 0; y < lvl.bmp.h; ++y)
        for (int x = 0; x < lvl.bmp.w; ++x)
            if (dmp.i[y * lvl.bmp.w + x] == 1)
            {
                rect_t rect = expand_rect(x, y);
                delete_rect(rect, dmp);
                attach_rect(rect, imp, &indices, &points);
            }
    mk_vbos(&ren.fg, indices, points, GL_STATIC_DRAW);

    free(indices);
    free(points);
    free(dmp.i);
    free(imp.i);
}

static rect_t expand_rect(int x, int y)
{
    rect_t rect = {x, y, 1, 1};
    while (x + rect.w < lvl.bmp.w&&lvl.bmp.i[y*lvl.bmp.w+x+rect.w]==1)++rect.w;
    while (y + rect.h < lvl.bmp.h && is_row(rect)) ++rect.h;
    return rect;
}

static bool is_row(rect_t rect)
{
    int base = (rect.y + rect.h) * lvl.bmp.w + rect.x;
    for (int i = base; i < base + rect.w; ++i)if(lvl.bmp.i[i]!=1) return false;
    return true;
}

static void delete_rect(rect_t rect, intmap_t dmp)
{
    for (int y = rect.y; y < rect.y + rect.h; ++y)
        for (int x = rect.x; x < rect.x + rect.w; ++x)
            dmp.i[y * dmp.w + x] = 0;
}

static void attach_rect(rect_t rect, intmap_t imp, INDEX_T**is, vector_t**pts)
{
    point_t p1 = {rect.x,          rect.y};
    point_t p2 = {rect.x + rect.w, rect.y};
    point_t p3 = {rect.x + rect.w, rect.y + rect.h};
    point_t p4 = {rect.x,          rect.y + rect.h};
    point_t points[]  = {p1, p2, p3, p4};
    point_t indices[] = {p1, p2, p3, p3, p4, p1};
    for (int i = 0; i < 4; ++i) attach_point(points[i], imp, pts);
    for (int i = 0; i < 6; ++i) attach_index(indices[i],imp, is);
}

static void attach_point(point_t p, intmap_t imp, vector_t **points)
{
    if (imp.i[p.y * imp.w + p.x] == 0)
    {
        *points = realloc(*points, ++ren.fg.n_points * sizeof(**points));
        (*points)[ren.fg.n_points - 1] = (vector_t) {p.x, p.y};
        imp.i[p.y * imp.w + p.x] = ren.fg.n_points;
    }
}

static void attach_index(point_t p, intmap_t imp, INDEX_T **indices)
{
    *indices = realloc(*indices, ++ren.fg.n_indices * sizeof(**indices));
    (*indices)[ren.fg.n_indices - 1] = imp.i[p.y * imp.w + p.x] - 1;
}

static void init_hero(void)
{
    INDEX_T indices[] = {0, 1, 2, 2, 3, 0};
    vector_t points[]={{0,0}, {1,0}, {1,1}, {0,1}};
    ren.hero.n_indices = 6;
    ren.hero.n_points = 4;
    mk_vbos(&ren.hero, indices, points, GL_DYNAMIC_DRAW);
}

static void init_dest(void)
{
    INDEX_T indices[] = {0, 1, 1, 2, 2, 3, 3, 0};
    vector_t points[] = {
        {lvl.dest_pos.x, lvl.dest_pos.y},
        {lvl.dest_pos.x + lvl.dest_sz.x, lvl.dest_pos.y},
        {lvl.dest_pos.x + lvl.dest_sz.x, lvl.dest_pos.y + lvl.dest_sz.y},
        {lvl.dest_pos.x, lvl.dest_pos.y + lvl.dest_sz.y}};
    ren.dest.n_indices = 8;
    ren.dest.n_points = 4;
    mk_vbos(&ren.dest, indices, points, GL_STATIC_DRAW);
}

static void mk_vbos(object_t *obj, INDEX_T *is, vector_t *pts, GLenum usage)
{
    for (int i = 0; i < obj->n_points; ++i)
    {
        pts[i].x = (pts[i].x / lvl.bmp.w) * 2 - 1;
        pts[i].y = (pts[i].y / lvl.bmp.h) * 2 - 1;
    }
    size_t sz_i = obj->n_indices * sizeof(*is);
    size_t sz_p = obj->n_points  * sizeof(*pts);
    obj->indices = mk_vbo(GL_ELEMENT_ARRAY_BUFFER,sz_i,is,GL_STATIC_DRAW);
    obj->points  = mk_vbo(GL_ARRAY_BUFFER, sz_p, pts, usage);
}

void exit_renderer(void)
{
    exit_obj(&ren.dest);
    exit_obj(&ren.hero);
    exit_obj(&ren.fg);
    ren.prg = rm_prg(ren.prg);
    glDisableVertexAttribArray(0);
    ren.vao = rm_vao(ren.vao);
    ren.win = rm_win(ren.win);
    exit_glfw();
}

static void exit_obj(object_t *obj)
{
    obj->n_indices = obj->n_points = 0;
    obj->indices = rm_vbo(obj->indices);
    obj->points = rm_vbo(obj->points);
}

void render_level(void)
{
    glClear(GL_COLOR_BUFFER_BIT);
    draw_obj(ren.fg, GL_TRIANGLES, lvl.fg_clr);
    draw_obj(ren.dest, GL_LINES, lvl.dest_clr);
    adjust_hero_position();
    draw_obj(ren.hero,GL_TRIANGLES, lvl.hero_clr);
    glfwSwapBuffers(ren.win);
}

static void draw_obj(object_t obj, GLenum mode, colour_t rgb)
{
    glUniform1f(ren.locs[0], rgb.r);
    glUniform1f(ren.locs[1], rgb.g);
    glUniform1f(ren.locs[2], rgb.b);
    glBindBuffer(GL_ARRAY_BUFFER, obj.points);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, obj.indices);
    glDrawElements(mode, obj.n_indices, INDEX_MACRO, 0);
}

static void adjust_hero_position(void)
{
    vector_t points[] = {{lvl.hero_pos.x, lvl.hero_pos.y},
                         {lvl.hero_pos.x + 1, lvl.hero_pos.y},
                         {lvl.hero_pos.x + 1, lvl.hero_pos.y + 1},
                         {lvl.hero_pos.x, lvl.hero_pos.y + 1}};
    for (int i = 0; i < 4; ++i)
    {
        points[i].x = (points[i].x / lvl.bmp.w) * 2 - 1;
        points[i].y = (points[i].y / lvl.bmp.h) * 2 - 1;
    }
    glBindBuffer(GL_ARRAY_BUFFER, ren.hero.points);
    glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_DYNAMIC_DRAW);
}

bool window_should_close(void)
{
    glfwPollEvents();
    return glfwWindowShouldClose(ren.win);
}

void reset_timer(void)
{
    glfwSetTime(0);
}

float get_time(void)
{
    return glfwGetTime();
}

bool key_pressed(int key)
{
    return glfwGetKey(ren.win, key);
}
