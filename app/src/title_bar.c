#include "title_bar.h"

#include <assert.h>
#include <string.h>

#include <SDL3_ttf/SDL_ttf.h>

#include "events.h"
#include "font.h"
#include "util/log.h"
#include "window_chrome.h"

static bool ttf_initialized;

static bool
ensure_ttf(void) {
    if (ttf_initialized) {
        return true;
    }
    if (!TTF_Init()) {
        LOGW("Could not init SDL_ttf for title bar: %s", SDL_GetError());
        return false;
    }
    ttf_initialized = true;
    return true;
}

static void
maybe_quit_ttf(void) {
    if (ttf_initialized) {
        TTF_Quit();
        ttf_initialized = false;
    }
}

static SDL_Texture *
create_title_texture(SDL_Renderer *renderer, const char *title) {
    if (!ensure_ttf()) {
        return NULL;
    }

    char *font_path = sc_font_get_path();
    if (!font_path) {
        LOGW("Could not find a font for the title bar");
        return NULL;
    }

    TTF_Font *font = TTF_OpenFont(font_path, 13.f);
    free(font_path);
    if (!font) {
        LOGW("Could not open title bar font: %s", SDL_GetError());
        return NULL;
    }

    SDL_Color color = { 0xbb, 0xbb, 0xbb, 255 };
    SDL_Surface *surface =
        TTF_RenderText_Blended(font, title, strlen(title), color);
    TTF_CloseFont(font);
    if (!surface) {
        LOGW("Could not render title bar text: %s", SDL_GetError());
        return NULL;
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_DestroySurface(surface);
    if (!texture) {
        LOGW("Could not create title bar texture: %s", SDL_GetError());
    }
    return texture;
}

static void
get_ctrl_rect(int ctrl, float window_w, SDL_FRect *rect) {
    assert(ctrl >= 0 && ctrl < SC_TITLE_BAR_CTRL_COUNT);
    rect->x = window_w - (SC_TITLE_BAR_CTRL_COUNT - ctrl) * SC_WINDOW_CTRL_WIDTH;
    rect->y = 0;
    rect->w = SC_WINDOW_CTRL_WIDTH;
    rect->h = SC_TITLE_BAR_HEIGHT;
}

static void
draw_minimize_icon(SDL_Renderer *renderer, const SDL_FRect *rect, float scale,
                   Uint8 r, Uint8 g, Uint8 b) {
    float cx = (rect->x + rect->w / 2.f) * scale;
    float cy = (rect->y + rect->h / 2.f) * scale;
    float w = 10.f * scale;
    SDL_FRect line = {
        .x = cx - w / 2.f,
        .y = cy,
        .w = w,
        .h = scale,
    };
    SDL_SetRenderDrawColor(renderer, r, g, b, 255);
    SDL_RenderFillRect(renderer, &line);
}

static void
draw_maximize_icon(SDL_Renderer *renderer, const SDL_FRect *rect, float scale,
                   bool maximized, Uint8 r, Uint8 g, Uint8 b) {
    float cx = (rect->x + rect->w / 2.f) * scale;
    float cy = (rect->y + rect->h / 2.f) * scale;
    float s = 10.f * scale;

    SDL_SetRenderDrawColor(renderer, r, g, b, 255);
    if (!maximized) {
        SDL_FRect square = {
            .x = cx - s / 2.f,
            .y = cy - s / 2.f,
            .w = s,
            .h = s,
        };
        SDL_RenderRect(renderer, &square);
    } else {
        SDL_FRect back = {
            .x = cx - s / 2.f + 2.f * scale,
            .y = cy - s / 2.f - 2.f * scale,
            .w = s,
            .h = s,
        };
        SDL_RenderRect(renderer, &back);
        SDL_FRect front = {
            .x = cx - s / 2.f - 2.f * scale,
            .y = cy - s / 2.f + 2.f * scale,
            .w = s,
            .h = s,
        };
        SDL_RenderFillRect(renderer, &front);
        SDL_RenderRect(renderer, &front);
    }
}

static void
draw_close_icon(SDL_Renderer *renderer, const SDL_FRect *rect, float scale,
                Uint8 r, Uint8 g, Uint8 b) {
    float cx = (rect->x + rect->w / 2.f) * scale;
    float cy = (rect->y + rect->h / 2.f) * scale;
    float d = 5.f * scale;

    SDL_SetRenderDrawColor(renderer, r, g, b, 255);
    SDL_RenderLine(renderer, cx - d, cy - d, cx + d, cy + d);
    SDL_RenderLine(renderer, cx + d, cy - d, cx - d, cy + d);
}

bool
sc_title_bar_init(struct sc_title_bar *bar, SDL_Window *window,
                  SDL_Renderer *renderer, bool enabled, const char *title) {
    bar->enabled = enabled;
    bar->visible = enabled;
    bar->hovered = SC_TITLE_BAR_CTRL_NONE;
    bar->pressed = SC_TITLE_BAR_CTRL_NONE;
    bar->window = window;
    bar->renderer = renderer;
    bar->title_texture = NULL;

    if (!enabled) {
        return true;
    }

    assert(title);
    bar->title_texture = create_title_texture(renderer, title);
    if (!bar->title_texture) {
        LOGW("Title bar will render without device name");
    }

    return true;
}

void
sc_title_bar_destroy(struct sc_title_bar *bar) {
    if (bar->title_texture) {
        SDL_DestroyTexture(bar->title_texture);
        bar->title_texture = NULL;
    }
    if (bar->enabled) {
        maybe_quit_ttf();
    }
}

void
sc_title_bar_set_visible(struct sc_title_bar *bar, bool visible) {
    if (!bar->enabled) {
        return;
    }
    bar->visible = visible;
    if (!visible) {
        bar->hovered = SC_TITLE_BAR_CTRL_NONE;
        bar->pressed = SC_TITLE_BAR_CTRL_NONE;
    }
}

bool
sc_title_bar_is_active(const struct sc_title_bar *bar) {
    return bar->enabled && bar->visible;
}

uint16_t
sc_title_bar_get_height(const struct sc_title_bar *bar) {
    return sc_title_bar_is_active(bar) ? SC_TITLE_BAR_HEIGHT : 0;
}

void
sc_title_bar_render(struct sc_title_bar *bar, float window_w, float scale,
                    bool interactive) {
    if (!sc_title_bar_is_active(bar)) {
        return;
    }

    SDL_Renderer *renderer = bar->renderer;

    SDL_FRect title_rect = {
        .x = 0,
        .y = 0,
        .w = window_w * scale,
        .h = SC_TITLE_BAR_HEIGHT * scale,
    };
    SDL_SetRenderDrawColor(renderer, SC_CHROME_TITLE_BG_R, SC_CHROME_TITLE_BG_G,
                           SC_CHROME_TITLE_BG_B, 255);
    SDL_RenderFillRect(renderer, &title_rect);

    SDL_FRect border = {
        .x = 0,
        .y = (SC_TITLE_BAR_HEIGHT - 1) * scale,
        .w = window_w * scale,
        .h = scale,
    };
    SDL_SetRenderDrawColor(renderer, SC_CHROME_BORDER_R, SC_CHROME_BORDER_G,
                           SC_CHROME_BORDER_B, 255);
    SDL_RenderFillRect(renderer, &border);

    if (bar->title_texture) {
        float tw, th;
        SDL_GetTextureSize(bar->title_texture, &tw, &th);
        float max_w = (window_w - SC_WINDOW_CTRL_WIDTH * SC_TITLE_BAR_CTRL_COUNT
                       - 12.f) * scale;
        if (tw > max_w) {
            tw = max_w;
        }
        SDL_FRect dst = {
            .x = 12.f * scale,
            .y = (SC_TITLE_BAR_HEIGHT * scale - th) / 2.f,
            .w = tw,
            .h = th,
        };
        SDL_RenderTexture(renderer, bar->title_texture, NULL, &dst);
    }

    SDL_WindowFlags flags = SDL_GetWindowFlags(bar->window);
    bool maximized = flags & SDL_WINDOW_MAXIMIZED;

    for (int i = 0; i < SC_TITLE_BAR_CTRL_COUNT; ++i) {
        SDL_FRect ctrl;
        get_ctrl_rect(i, window_w, &ctrl);

        bool close = i == SC_TITLE_BAR_CTRL_CLOSE;
        bool hover = interactive && bar->hovered == i;
        bool pressed = interactive && bar->pressed == i;

        if (hover || pressed) {
            SDL_FRect hi = {
                .x = ctrl.x * scale,
                .y = ctrl.y * scale,
                .w = ctrl.w * scale,
                .h = ctrl.h * scale,
            };
            if (close && hover) {
                SDL_SetRenderDrawColor(renderer, SC_CHROME_CLOSE_HOVER_R,
                                       SC_CHROME_CLOSE_HOVER_G,
                                       SC_CHROME_CLOSE_HOVER_B, 255);
            } else if (pressed) {
                SDL_SetRenderDrawColor(renderer, SC_CHROME_PRESSED_R,
                                       SC_CHROME_PRESSED_G,
                                       SC_CHROME_PRESSED_B, 255);
            } else {
                SDL_SetRenderDrawColor(renderer, SC_CHROME_HOVER_R,
                                       SC_CHROME_HOVER_G, SC_CHROME_HOVER_B,
                                       255);
            }
            SDL_RenderFillRect(renderer, &hi);
        }

        Uint8 r = SC_CHROME_TEXT_R;
        Uint8 g = SC_CHROME_TEXT_G;
        Uint8 b = SC_CHROME_TEXT_B;
        if (close && hover) {
            r = g = b = 0xff;
        }

        switch (i) {
            case SC_TITLE_BAR_CTRL_MIN:
                draw_minimize_icon(renderer, &ctrl, scale, r, g, b);
                break;
            case SC_TITLE_BAR_CTRL_MAX:
                draw_maximize_icon(renderer, &ctrl, scale, maximized, r, g, b);
                break;
            case SC_TITLE_BAR_CTRL_CLOSE:
                draw_close_icon(renderer, &ctrl, scale, r, g, b);
                break;
        }
    }
}

enum sc_title_bar_ctrl
sc_title_bar_hit_test_ctrl(const struct sc_title_bar *bar, float x, float y,
                           float window_w) {
    if (!sc_title_bar_is_active(bar)) {
        return SC_TITLE_BAR_CTRL_NONE;
    }

    if (y < 0 || y >= SC_TITLE_BAR_HEIGHT) {
        return SC_TITLE_BAR_CTRL_NONE;
    }

    for (int i = 0; i < SC_TITLE_BAR_CTRL_COUNT; ++i) {
        SDL_FRect ctrl;
        get_ctrl_rect(i, window_w, &ctrl);
        if (x >= ctrl.x && x < ctrl.x + ctrl.w) {
            return (enum sc_title_bar_ctrl) i;
        }
    }

    return SC_TITLE_BAR_CTRL_NONE;
}

bool
sc_title_bar_set_hovered(struct sc_title_bar *bar, int ctrl_index) {
    if (bar->hovered == ctrl_index) {
        return false;
    }
    bar->hovered = ctrl_index;
    return true;
}

bool
sc_title_bar_contains_ctrl(const struct sc_title_bar *bar, float x, float y,
                           float window_w) {
    return sc_title_bar_hit_test_ctrl(bar, x, y, window_w)
        != SC_TITLE_BAR_CTRL_NONE;
}

void
sc_title_bar_minimize(struct sc_title_bar *bar) {
    if (!bar->window) {
        return;
    }
    SDL_MinimizeWindow(bar->window);
}

void
sc_title_bar_toggle_maximize(struct sc_title_bar *bar) {
    if (!bar->window) {
        return;
    }
    SDL_WindowFlags flags = SDL_GetWindowFlags(bar->window);
    if (flags & SDL_WINDOW_MAXIMIZED) {
        SDL_RestoreWindow(bar->window);
    } else {
        SDL_MaximizeWindow(bar->window);
    }
}

void
sc_title_bar_close(struct sc_title_bar *bar) {
    (void) bar;
    sc_push_event(SDL_EVENT_QUIT);
}
