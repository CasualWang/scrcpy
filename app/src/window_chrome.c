#include "window_chrome.h"

#include <SDL3/SDL.h>

#include "nav_bar.h"
#include "screen.h"
#include "title_bar.h"
#include "util/log.h"

static SDL_HitTestResult SDLCALL
sc_window_chrome_hit_test(SDL_Window *win, const SDL_Point *area, void *data) {
    (void) win;

    struct sc_screen *screen = data;
    if (!sc_window_chrome_is_active(screen)) {
        return SDL_HITTEST_NORMAL;
    }

    int w, h;
    if (!SDL_GetWindowSize(screen->window, &w, &h)) {
        return SDL_HITTEST_NORMAL;
    }

    float x = area->x;
    float y = area->y;

    if (sc_title_bar_hit_test_ctrl(&screen->title_bar, x, y, w)
            != SC_TITLE_BAR_CTRL_NONE) {
        return SDL_HITTEST_NORMAL;
    }

    if (sc_nav_bar_hit_test(&screen->nav_bar, x, y, w, h, &screen->rect) >= 0) {
        return SDL_HITTEST_NORMAL;
    }

    const int border = SC_RESIZE_BORDER;

    if (x < border) {
        if (y < border) {
            return SDL_HITTEST_RESIZE_TOPLEFT;
        }
        if (y >= h - border) {
            return SDL_HITTEST_RESIZE_BOTTOMLEFT;
        }
        return SDL_HITTEST_RESIZE_LEFT;
    }

    if (x >= w - border) {
        if (y < border) {
            return SDL_HITTEST_RESIZE_TOPRIGHT;
        }
        if (y >= h - border) {
            return SDL_HITTEST_RESIZE_BOTTOMRIGHT;
        }
        return SDL_HITTEST_RESIZE_RIGHT;
    }

    if (y < border) {
        return SDL_HITTEST_RESIZE_TOP;
    }

    if (y >= h - border) {
        return SDL_HITTEST_RESIZE_BOTTOM;
    }

    if (y < SC_TITLE_BAR_HEIGHT) {
        return SDL_HITTEST_DRAGGABLE;
    }

    return SDL_HITTEST_NORMAL;
}

bool
sc_window_chrome_is_active(const struct sc_screen *screen) {
    return sc_title_bar_is_active(&screen->title_bar);
}

uint16_t
sc_window_chrome_get_title_height(const struct sc_screen *screen) {
    return sc_title_bar_get_height(&screen->title_bar);
}

uint16_t
sc_window_chrome_get_nav_width(const struct sc_screen *screen) {
    return sc_nav_bar_get_width(&screen->nav_bar);
}

bool
sc_window_chrome_register(struct sc_screen *screen) {
    if (!sc_window_chrome_is_active(screen)) {
        return true;
    }

    bool ok = SDL_SetWindowHitTest(screen->window, sc_window_chrome_hit_test,
                                   screen);
    if (!ok) {
        LOGW("Could not set window hit test: %s", SDL_GetError());
    }
    return ok;
}

void
sc_window_chrome_unregister(struct sc_screen *screen) {
    if (!screen->window) {
        return;
    }
    SDL_SetWindowHitTest(screen->window, NULL, NULL);
}

void
sc_window_chrome_apply_minimum_size(SDL_Window *window,
                                    const struct sc_screen *screen) {
    if (!sc_window_chrome_is_active(screen)) {
        return;
    }

    int min_w = SC_NAV_BAR_WIDTH + 200;
    int min_h = SC_TITLE_BAR_HEIGHT + SC_NAV_MARGIN_TOP
              + SC_NAV_BAR_BTN_COUNT * SC_NAV_BTN_SIZE
              + (SC_NAV_BAR_BTN_COUNT - 1) * SC_NAV_BTN_GAP;
    if (min_w < 320) {
        min_w = 320;
    }
    if (min_h < 240) {
        min_h = 240;
    }

    if (!SDL_SetWindowMinimumSize(window, min_w, min_h)) {
        LOGW("Could not set window minimum size: %s", SDL_GetError());
    }
}
