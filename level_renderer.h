#ifndef LEVEL_RENDERER_H
#define LEVEL_RENDERER_H

#include <stdbool.h>

void init_renderer(void);
void exit_renderer(void);
void render_level(void);
bool window_should_close(void);
void reset_timer(void);
float get_time(void);
bool key_pressed(int key);

#endif
