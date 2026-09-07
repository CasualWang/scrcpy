#ifndef SC_NAV_BAR_H
#define SC_NAV_BAR_H

#include "common.h"

#include <stdbool.h>
#include <stdint.h>

#include <SDL3/SDL.h>

#include "android/keycodes.h"

#define SC_NAV_BAR_WIDTH 40
#define SC_NAV_ICON_SIZE 18
#define SC_NAV_BTN_SIZE 32
#define SC_NAV_BTN_GAP 6
#define SC_NAV_MARGIN_TOP 8

enum sc_nav_bar_button {
    SC_NAV_BAR_BTN_HOME = 0,
    SC_NAV_BAR_BTN_BACK,
    SC_NAV_BAR_BTN_RECENTS,
    SC_NAV_BAR_BTN_POWER,
    SC_NAV_BAR_BTN_SCREEN_OFF,
    SC_NAV_BAR_BTN_SCREEN_ON,
    SC_NAV_BAR_BTN_VOLUME_UP,
    SC_NAV_BAR_BTN_VOLUME_DOWN,
    SC_NAV_BAR_BTN_ROTATE_DEVICE,
    SC_NAV_BAR_BTN_ROTATE_DISPLAY_LEFT,
    SC_NAV_BAR_BTN_ROTATE_DISPLAY_RIGHT,
    SC_NAV_BAR_BTN_COUNT,
};

enum sc_nav_bar_action_type {
    SC_NAV_BAR_ACTION_INJECT_KEYCODE,
    SC_NAV_BAR_ACTION_SCREEN_OFF,
    SC_NAV_BAR_ACTION_SCREEN_ON,
    SC_NAV_BAR_ACTION_ROTATE_DEVICE,
    SC_NAV_BAR_ACTION_ROTATE_DISPLAY_LEFT,
    SC_NAV_BAR_ACTION_ROTATE_DISPLAY_RIGHT,
};

struct sc_nav_bar_action {
    enum sc_nav_bar_action_type type;
    enum android_keycode keycode;
};

struct sc_nav_bar_bg_color {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

struct sc_nav_bar {
    bool enabled;
    bool visible;
    int hovered;
    int pressed;
    SDL_Renderer *renderer;
    SDL_Texture *icons[SC_NAV_BAR_BTN_COUNT];
};

bool
sc_nav_bar_init(struct sc_nav_bar *bar, SDL_Renderer *renderer, bool enabled);

void
sc_nav_bar_destroy(struct sc_nav_bar *bar);

void
sc_nav_bar_set_visible(struct sc_nav_bar *bar, bool visible);

bool
sc_nav_bar_is_active(const struct sc_nav_bar *bar);

uint16_t
sc_nav_bar_get_width(const struct sc_nav_bar *bar);

void
sc_nav_bar_render(struct sc_nav_bar *bar, float window_w, float window_h,
                  const SDL_FRect *content_rect,
                  struct sc_nav_bar_bg_color bg, float scale, bool interactive);

int
sc_nav_bar_hit_test(const struct sc_nav_bar *bar, float x, float y,
                    float window_w, float window_h,
                    const SDL_FRect *content_rect);

void
sc_nav_bar_get_action(int button_index, struct sc_nav_bar_action *action);

bool
sc_nav_bar_set_hovered(struct sc_nav_bar *bar, int button_index);

bool
sc_nav_bar_contains_point(const struct sc_nav_bar *bar, float x, float y,
                          float window_w, float window_h,
                          const SDL_FRect *content_rect);

void
sc_nav_bar_get_button_rect(const struct sc_nav_bar *bar, int button_index,
                           float window_w, float window_h,
                           const SDL_FRect *content_rect, SDL_FRect *rect);

#endif
