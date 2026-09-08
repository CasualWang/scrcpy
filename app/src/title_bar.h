#ifndef SC_TITLE_BAR_H
#define SC_TITLE_BAR_H

#include "common.h"

#include <stdbool.h>
#include <stdint.h>

#include <SDL3/SDL.h>

enum sc_title_bar_ctrl {
    SC_TITLE_BAR_CTRL_MIN = 0,
    SC_TITLE_BAR_CTRL_MAX,
    SC_TITLE_BAR_CTRL_CLOSE,
    SC_TITLE_BAR_CTRL_COUNT,
    SC_TITLE_BAR_CTRL_NONE = -1,
};

struct sc_title_bar {
    bool enabled;
    bool visible;
    int hovered;
    int pressed;
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *title_texture;
};

bool
sc_title_bar_init(struct sc_title_bar *bar, SDL_Window *window,
                  SDL_Renderer *renderer, bool enabled, const char *title);

void
sc_title_bar_destroy(struct sc_title_bar *bar);

void
sc_title_bar_set_visible(struct sc_title_bar *bar, bool visible);

bool
sc_title_bar_is_active(const struct sc_title_bar *bar);

uint16_t
sc_title_bar_get_height(const struct sc_title_bar *bar);

void
sc_title_bar_render(struct sc_title_bar *bar, float window_w, float scale,
                    bool interactive);

enum sc_title_bar_ctrl
sc_title_bar_hit_test_ctrl(const struct sc_title_bar *bar, float x, float y,
                           float window_w);

bool
sc_title_bar_set_hovered(struct sc_title_bar *bar, int ctrl_index);

bool
sc_title_bar_contains_ctrl(const struct sc_title_bar *bar, float x, float y,
                           float window_w);

void
sc_title_bar_minimize(struct sc_title_bar *bar);

void
sc_title_bar_toggle_maximize(struct sc_title_bar *bar);

void
sc_title_bar_close(struct sc_title_bar *bar);

#endif
