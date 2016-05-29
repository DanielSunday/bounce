#include "level.h"

#include <stdio.h>

#include "level_t.h"
#include "level_renderer.h"
#include "bass.h"

static float last_frame_time = 0;
static float elapsed_time = 0;
static char *game_over_string = NULL;

static bool game_over(void);
static bool hero_reached_destination(void);
static void resolve_collisions(void);
static bool hero_is_outside(void);
static void do_empty(void);
static void do_full(void);
static void do_south(void);
static void do_north(void);
static void do_west(void);
static void do_east(void);
static void do_south_west_corner(void);
static void do_south_east_corner(void);
static void do_north_west_corner(void);
static void do_north_east_corner(void);
static void do_south_west(void);
static void do_south_east(void);
static void do_north_west(void);
static void do_north_east(void);
static void do_north_west_diag(void);
static void do_north_east_diag(void);
static void bounce_south(void);
static void bounce_north(void);
static void bounce_west(void);
static void bounce_east(void);
static float get_south_penetration(void);
static float get_north_penetration(void);
static float get_west_penetration(void);
static float get_east_penetration(void);
static void react_to_input(void);
static void push_west(void);
static void push_east(void);
static void apply_velocity(void);

void run_level(int level)
{
    if (!init_level_t(level)) return;
    init_renderer();
    init_bass(lvl.bass_frequency, lvl.bass_peak_volume, lvl.bass_duration);
    reset_timer();
    while (!game_over())
    {
        render_level();
        react_to_input();
        apply_velocity();
        resolve_collisions();
    }
    exit_bass();
    exit_renderer();
    exit_level_t();
}

static bool game_over(void)
{
    if (window_should_close()) game_over_string = "Window closed.";
    if (hero_reached_destination()) game_over_string = "You won!";
    if (game_over_string != NULL)
    {
        printf("%s Time: %fs.\n", game_over_string, get_time());
        game_over_string = NULL;
        elapsed_time = last_frame_time = 0;
        return true;
    }
    return false;
}

static bool hero_reached_destination(void)
{
    return lvl.hero_pos.x > lvl.dest_pos.x
        && lvl.hero_pos.y > lvl.dest_pos.y
        && lvl.hero_pos.x + 1 < lvl.dest_pos.x + lvl.dest_sz.x
        && lvl.hero_pos.y + 1 < lvl.dest_pos.y + lvl.dest_sz.y;
}

static void resolve_collisions(void)
{
    if (hero_is_outside())
    {
        lvl.hero_vel.x = lvl.hero_vel.y = 0;
        game_over_string = "You ventured outside the level bounds.";
        return;
    }
    else
    {
        elapsed_time = get_time() - last_frame_time;
        lvl.hero_vel.x += lvl.grav_vel.x * elapsed_time;
        lvl.hero_vel.y += lvl.grav_vel.y * elapsed_time;
        last_frame_time = get_time();
    }

    int south_west = (int) lvl.hero_pos.y * lvl.bmp.w + (int) lvl.hero_pos.x;
    int south_east = south_west + 1;
    int north_west = south_west + lvl.bmp.w;
    int north_east = north_west + 1;

    if (   lvl.bmp.i[south_west] || lvl.bmp.i[south_east]
        || lvl.bmp.i[north_west] || lvl.bmp.i[north_east]) drop_bass();

    if (lvl.bmp.i[south_west] == 0)
        if (lvl.bmp.i[south_east] == 0)
            if (lvl.bmp.i[north_west] == 0)
                if (lvl.bmp.i[north_east] == 0)
                    do_empty();
                else
                    do_north_east();
            else
                if (lvl.bmp.i[north_east] == 0)
                    do_north_west();
                else
                    do_north();
        else
            if (lvl.bmp.i[north_west] == 0)
                if (lvl.bmp.i[north_east] == 0)
                    do_south_east();
                else
                    do_east();
            else
                if (lvl.bmp.i[north_east] == 0)
                    do_north_west_diag();
                else
                    do_north_east_corner();
    else
        if (lvl.bmp.i[south_east] == 0)
            if (lvl.bmp.i[north_west] == 0)
                if (lvl.bmp.i[north_east] == 0)
                    do_south_west();
                else
                    do_north_east_diag();
            else
                if (lvl.bmp.i[north_east] == 0)
                    do_west();
                else
                    do_north_west_corner();
        else
            if (lvl.bmp.i[north_west] == 0)
                if (lvl.bmp.i[north_east] == 0)
                    do_south();
                else
                    do_south_east_corner();
            else
                if (lvl.bmp.i[north_east] == 0)
                    do_south_west_corner();
                else
                    do_full();
}

static bool hero_is_outside(void)
{
    return lvl.hero_pos.x < 0 || lvl.hero_pos.y < 0 ||
           lvl.hero_pos.x >= lvl.bmp.w - 1 || lvl.hero_pos.y >= lvl.bmp.h - 1;
}

static void do_empty(void) {}

static void do_full(void)
{
    lvl.hero_vel.x = lvl.hero_vel.y = 0;
    game_over_string = "You were crushed.";
}

static void do_south(void)
{
    bounce_north();
}

static void do_north(void)
{
    bounce_south();
}

static void do_west(void)
{
    bounce_east();
}

static void do_east(void)
{
    bounce_west();
}

static void do_south_west_corner(void)
{
    do_south();
    do_west();
}

static void do_south_east_corner(void)
{
    do_south();
    do_east();
}

static void do_north_west_corner(void)
{
    do_north();
    do_west();
}

static void do_north_east_corner(void)
{
    do_north();
    do_east();
}

static void do_south_west(void)
{
    if (get_south_penetration() < get_west_penetration()) bounce_north();
    else bounce_east();
}

static void do_south_east(void)
{
    if (get_south_penetration() < get_east_penetration()) bounce_north();
    else bounce_west();
}

static void do_north_west(void)
{
    if (get_north_penetration() < get_west_penetration()) bounce_south();
    else bounce_east();
}
static void do_north_east(void)
{
    if (get_north_penetration() < get_east_penetration()) bounce_south();
    else bounce_west();
}

static void do_north_west_diag(void)
{
    if (get_south_penetration() <= 0.5) do_south_west_corner();
    else do_north_east_corner();
}

static void do_north_east_diag(void)
{
    if (get_south_penetration() <= 0.5) do_south_east_corner();
    else do_north_west_corner();
}

static void bounce_south(void)
{
    lvl.hero_pos.y = (int) lvl.hero_pos.y;
    lvl.hero_vel.y = -lvl.bounce_vel.y;
}

static void bounce_north(void)
{
    lvl.hero_pos.y = (int) lvl.hero_pos.y + 1;
    lvl.hero_vel.y = lvl.bounce_vel.y;
}

static void bounce_west(void)
{
    lvl.hero_pos.x = (int) lvl.hero_pos.x ;
    lvl.hero_vel.x = -lvl.bounce_vel.x;
}

static void bounce_east(void)
{
    lvl.hero_pos.x = (int) lvl.hero_pos.x + 1;
    lvl.hero_vel.x = lvl.bounce_vel.x;
}

static float get_south_penetration(void)
{
    return ((int) lvl.hero_pos.y + 1) - lvl.hero_pos.y;
}

static float get_north_penetration(void)
{
    return lvl.hero_pos.y - (int) lvl.hero_pos.y;
}

static float get_west_penetration(void)
{
    return ((int) lvl.hero_pos.x + 1) - lvl.hero_pos.x;
}

static float get_east_penetration(void)
{
    return lvl.hero_pos.x - (int) lvl.hero_pos.x;
}

static void react_to_input(void)
{
    if (key_pressed(lvl.key_west)) push_west();
    if (key_pressed(lvl.key_east)) push_east();
    if (key_pressed(lvl.key_exit)) game_over_string = "Aborted by user.";
}

static void push_west(void)
{
    lvl.hero_vel.x += lvl.key_west_vel.x;
    lvl.hero_vel.y += lvl.key_east_vel.y;
}

static void push_east(void)
{
    lvl.hero_vel.x += lvl.key_east_vel.x;
    lvl.hero_vel.y += lvl.key_east_vel.y;
}

static void apply_velocity(void)
{
    if (lvl.hero_vel.x > lvl.term_vel.x) lvl.hero_vel.x = lvl.term_vel.x;
    if (lvl.hero_vel.x <-lvl.term_vel.x) lvl.hero_vel.x =-lvl.term_vel.x;
    if (lvl.hero_vel.y > lvl.term_vel.y) lvl.hero_vel.y = lvl.term_vel.y;
    if (lvl.hero_vel.y <-lvl.term_vel.y) lvl.hero_vel.y =-lvl.term_vel.y;
    lvl.hero_pos.x += lvl.hero_vel.x * elapsed_time;
    lvl.hero_pos.y += lvl.hero_vel.y * elapsed_time;
}
