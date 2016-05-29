#include "level_t.h"

#include <stdlib.h>
#include <stdio.h>

#define LEVEL_ADDR_HEAD "levels/level"
#define LEVEL_ADDR_TAIL ".txt"

level_t lvl;

static char *mk_addr(int level)
{
    static char buffer[100];
    sprintf(buffer, LEVEL_ADDR_HEAD "%d" LEVEL_ADDR_TAIL, level);
    return buffer;
}

bool init_level_t(int level)
{
    FILE *f = fopen(mk_addr(level), "r");
    if (f == NULL)
    {
        puts("Level does not exist.");
        return false;
    }
    fscanf(f, "%d%d%d%d%d%d%f%f%f%f%f%f%f%f%f%f%f%f%f%f%f"
              "%f%f%f%f%f%f%f%f%f%f%f%f%f%f%f%f%f%f%d%d",
            &lvl.win_w, &lvl.win_h, &lvl.win_fscr,
            &lvl.key_west, &lvl.key_east, &lvl.key_exit,
	    &lvl.bass_frequency, &lvl.bass_peak_volume, &lvl.bass_duration,
            &lvl.bg_clr.r, &lvl.bg_clr.g, &lvl.bg_clr.b,
            &lvl.fg_clr.r, &lvl.fg_clr.g, &lvl.fg_clr.b,
            &lvl.hero_clr.r, &lvl.hero_clr.g, &lvl.hero_clr.b,
            &lvl.dest_clr.r, &lvl.dest_clr.g, &lvl.dest_clr.b,
            &lvl.hero_pos.x, &lvl.hero_pos.y, &lvl.hero_vel.x, &lvl.hero_vel.y,
            &lvl.dest_pos.x, &lvl.dest_pos.y, &lvl.dest_sz.x,  &lvl.dest_sz.y,
            &lvl.grav_vel.x, &lvl.grav_vel.y, &lvl.term_vel.x, &lvl.term_vel.y,
            &lvl.bounce_vel.x, &lvl.bounce_vel.y,
            &lvl.key_west_vel.x, &lvl.key_west_vel.y,
            &lvl.key_east_vel.x, &lvl.key_east_vel.y,
            &lvl.bmp.w, &lvl.bmp.h);
    lvl.bmp.i = malloc(lvl.bmp.w * lvl.bmp.h * sizeof(*lvl.bmp.i));
    for (int y = lvl.bmp.h - 1; y >= 0; --y)
    {
        for (int x = 0; x < lvl.bmp.w; ++x)
        {
            fscanf(f, "%d", &lvl.bmp.i[y * lvl.bmp.w + x]);
        }
    }
    fclose(f);
    return true;
}

void exit_level_t(void)
{
    free(lvl.bmp.i);
}
