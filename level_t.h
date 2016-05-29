#ifndef LEVEL_T_H
#define LEVEL_T_H

#include <stdbool.h>

typedef struct colour_t
{
    float r, g, b;
} colour_t;

typedef struct vector_t
{
    float x, y;
} vector_t;

typedef struct intmap_t
{
    int w, h, *i;
} intmap_t;

typedef struct level_t
{
    int win_w, win_h, win_fscr, key_west, key_east, key_exit;
    float bass_frequency, bass_peak_volume, bass_duration;
    colour_t bg_clr, fg_clr, hero_clr, dest_clr;
    vector_t hero_pos, hero_vel, dest_pos, dest_sz;
    vector_t grav_vel, term_vel, bounce_vel, key_west_vel, key_east_vel;
    intmap_t bmp; // Bit map
} level_t;

extern level_t lvl;

bool init_level_t(int level);
void exit_level_t(void);

#endif
