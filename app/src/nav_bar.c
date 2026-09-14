#include "nav_bar.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "icon.h"
#include "shortcut_mod.h"
#include "util/log.h"


static const char *const BUTTON_SHORTCUTS[SC_NAV_BAR_BTN_COUNT] = {
    [SC_NAV_BAR_BTN_HOME] = "h",
    [SC_NAV_BAR_BTN_BACK] = "b",
    [SC_NAV_BAR_BTN_RECENTS] = "s",
    [SC_NAV_BAR_BTN_POWER] = "p",
    [SC_NAV_BAR_BTN_SCREEN_OFF] = "o",
    [SC_NAV_BAR_BTN_SCREEN_ON] = "Shift+o",
    [SC_NAV_BAR_BTN_SCREENSHOT] = "Shift+s",
    [SC_NAV_BAR_BTN_VOLUME_UP] = "Up",
    [SC_NAV_BAR_BTN_VOLUME_DOWN] = "Down",
    [SC_NAV_BAR_BTN_ROTATE_DEVICE] = "r",
    [SC_NAV_BAR_BTN_ROTATE_DISPLAY_LEFT] = "Left",
    [SC_NAV_BAR_BTN_ROTATE_DISPLAY_RIGHT] = "Right",
};

static const char *const ICON_FILENAMES[SC_NAV_BAR_BTN_COUNT] = {
    [SC_NAV_BAR_BTN_HOME] = "nav/nav_home.png",
    [SC_NAV_BAR_BTN_BACK] = "nav/nav_back.png",
    [SC_NAV_BAR_BTN_RECENTS] = "nav/nav_recents.png",
    [SC_NAV_BAR_BTN_POWER] = "nav/nav_power.png",
    [SC_NAV_BAR_BTN_SCREEN_OFF] = "nav/nav_screen_off.png",
    [SC_NAV_BAR_BTN_SCREEN_ON] = "nav/nav_screen_on.png",
    [SC_NAV_BAR_BTN_SCREENSHOT] = "nav/nav_screenshot.png",
    [SC_NAV_BAR_BTN_VOLUME_UP] = "nav/nav_volume_up.png",
    [SC_NAV_BAR_BTN_VOLUME_DOWN] = "nav/nav_volume_down.png",
    [SC_NAV_BAR_BTN_ROTATE_DEVICE] = "nav/nav_rotate_device.png",
    [SC_NAV_BAR_BTN_ROTATE_DISPLAY_LEFT] = "nav/nav_rotate_left.png",
    [SC_NAV_BAR_BTN_ROTATE_DISPLAY_RIGHT] = "nav/nav_rotate_right.png",
};

static const struct sc_nav_bar_action BUTTON_ACTIONS[SC_NAV_BAR_BTN_COUNT] = {
    [SC_NAV_BAR_BTN_HOME] = {
        .type = SC_NAV_BAR_ACTION_INJECT_KEYCODE,
        .keycode = AKEYCODE_HOME,
    },
    [SC_NAV_BAR_BTN_BACK] = {
        .type = SC_NAV_BAR_ACTION_INJECT_KEYCODE,
        .keycode = AKEYCODE_BACK,
    },
    [SC_NAV_BAR_BTN_RECENTS] = {
        .type = SC_NAV_BAR_ACTION_INJECT_KEYCODE,
        .keycode = AKEYCODE_APP_SWITCH,
    },
    [SC_NAV_BAR_BTN_POWER] = {
        .type = SC_NAV_BAR_ACTION_INJECT_KEYCODE,
        .keycode = AKEYCODE_POWER,
    },
    [SC_NAV_BAR_BTN_SCREEN_OFF] = {
        .type = SC_NAV_BAR_ACTION_SCREEN_OFF,
    },
    [SC_NAV_BAR_BTN_SCREEN_ON] = {
        .type = SC_NAV_BAR_ACTION_SCREEN_ON,
    },
    [SC_NAV_BAR_BTN_SCREENSHOT] = {
        .type = SC_NAV_BAR_ACTION_SCREENSHOT,
    },
    [SC_NAV_BAR_BTN_VOLUME_UP] = {
        .type = SC_NAV_BAR_ACTION_INJECT_KEYCODE,
        .keycode = AKEYCODE_VOLUME_UP,
    },
    [SC_NAV_BAR_BTN_VOLUME_DOWN] = {
        .type = SC_NAV_BAR_ACTION_INJECT_KEYCODE,
        .keycode = AKEYCODE_VOLUME_DOWN,
    },
    [SC_NAV_BAR_BTN_ROTATE_DEVICE] = {
        .type = SC_NAV_BAR_ACTION_ROTATE_DEVICE,
    },
    [SC_NAV_BAR_BTN_ROTATE_DISPLAY_LEFT] = {
        .type = SC_NAV_BAR_ACTION_ROTATE_DISPLAY_LEFT,
    },
    [SC_NAV_BAR_BTN_ROTATE_DISPLAY_RIGHT] = {
        .type = SC_NAV_BAR_ACTION_ROTATE_DISPLAY_RIGHT,
    },
};

static void
destroy_icons(struct sc_nav_bar *bar) {
    for (int i = 0; i < SC_NAV_BAR_BTN_COUNT; ++i) {
        if (bar->icons[i]) {
            SDL_DestroyTexture(bar->icons[i]);
            bar->icons[i] = NULL;
        }
    }
}

static bool
load_icons(struct sc_nav_bar *bar) {
    int loaded = 0;
    for (int i = 0; i < SC_NAV_BAR_BTN_COUNT; ++i) {
        SDL_Surface *surface = sc_icon_load(ICON_FILENAMES[i]);
        if (!surface) {
            LOGW("Could not load nav bar icon: %s", ICON_FILENAMES[i]);
            continue;
        }

        bar->icons[i] = SDL_CreateTextureFromSurface(bar->renderer, surface);
        sc_icon_destroy(surface);
        if (!bar->icons[i]) {
            LOGW("Could not create nav bar icon texture: %s", SDL_GetError());
            continue;
        }
        ++loaded;
    }

    if (loaded == 0) {
        LOGW("No navigation bar icons loaded (expected next to scrcpy.exe: "
             "nav/*.png)");
    } else {
        LOGD("Loaded %d/%d navigation bar icons", loaded, SC_NAV_BAR_BTN_COUNT);
    }

    return loaded > 0;
}

static void
get_strip_rect(const SDL_FRect *content_rect, SDL_FRect *strip) {
    strip->x = content_rect->x + content_rect->w;
    strip->y = content_rect->y;
    strip->w = SC_NAV_BAR_WIDTH;
    strip->h = content_rect->h;
}

static float
compute_dock_x(const SDL_FRect *content_rect) {
    return content_rect->x + content_rect->w
         + (SC_NAV_BAR_WIDTH - SC_NAV_BTN_SIZE) / 2.f;
}

static float
compute_stack_y(const SDL_FRect *content_rect) {
    return content_rect->y + SC_NAV_MARGIN_TOP;
}

void
sc_nav_bar_get_button_rect(const struct sc_nav_bar *bar, int button_index,
                           float window_w, float window_h,
                           const SDL_FRect *content_rect, SDL_FRect *rect) {
    assert(button_index >= 0 && button_index < SC_NAV_BAR_BTN_COUNT);
    assert(content_rect);

    float dock_x = compute_dock_x(content_rect);
    float stack_y = compute_stack_y(content_rect);
    float y = stack_y + button_index * (SC_NAV_BTN_SIZE + SC_NAV_BTN_GAP);

    rect->x = dock_x;
    rect->y = y;
    rect->w = SC_NAV_BTN_SIZE;
    rect->h = SC_NAV_BTN_SIZE;

    (void) bar;
    (void) window_w;
    (void) window_h;
}

static void
init_tooltips(struct sc_nav_bar *bar, uint8_t shortcut_mods) {
    const char *mod_name = "Alt";
    if (shortcut_mods & (SC_SHORTCUT_MOD_LCTRL | SC_SHORTCUT_MOD_RCTRL)) {
        mod_name = "Ctrl";
    } else if (shortcut_mods & (SC_SHORTCUT_MOD_LSUPER | SC_SHORTCUT_MOD_RSUPER)) {
        mod_name = "Win";
    }

    for (int i = 0; i < SC_NAV_BAR_BTN_COUNT; ++i) {
        snprintf(bar->tooltip_texts[i], sizeof(bar->tooltip_texts[i]),
                 "%s+%s", mod_name, BUTTON_SHORTCUTS[i]);
    }
}

bool
sc_nav_bar_init(struct sc_nav_bar *bar, SDL_Window *window,
                SDL_Renderer *renderer, bool enabled, uint8_t shortcut_mods) {
    bar->enabled = enabled;
    bar->visible = enabled;
    bar->hovered = -1;
    bar->pressed = -1;
    bar->renderer = renderer;
    bar->window = window;
    memset(bar->icons, 0, sizeof(bar->icons));
    memset(bar->tooltip_texts, 0, sizeof(bar->tooltip_texts));

    if (!enabled) {
        return true;
    }

    if (!load_icons(bar)) {
        LOGW("Navigation bar icons could not be loaded");
    }

    init_tooltips(bar, shortcut_mods);

    return true;
}

void
sc_nav_bar_destroy(struct sc_nav_bar *bar) {
    destroy_icons(bar);
}

void
sc_nav_bar_set_visible(struct sc_nav_bar *bar, bool visible) {
    if (!bar->enabled) {
        return;
    }
    bar->visible = visible;
    if (!visible) {
        bar->hovered = -1;
        bar->pressed = -1;
    }
}

bool
sc_nav_bar_is_active(const struct sc_nav_bar *bar) {
    return bar->enabled && bar->visible;
}

uint16_t
sc_nav_bar_get_width(const struct sc_nav_bar *bar) {
    return sc_nav_bar_is_active(bar) ? SC_NAV_BAR_WIDTH : 0;
}

static Uint8
icon_alpha(const struct sc_nav_bar *bar, int index, bool interactive) {
    if (!interactive) {
        return 0x66;
    }
    if (bar->pressed == index) {
        return 0x99;
    }
    if (bar->hovered == index) {
        return 0xff;
    }
    return 0xcc;
}

void
sc_nav_bar_render(struct sc_nav_bar *bar, float window_w, float window_h,
                  const SDL_FRect *content_rect, struct sc_nav_bar_bg_color bg,
                  float scale, bool interactive) {
    if (!sc_nav_bar_is_active(bar) || !content_rect) {
        return;
    }

    SDL_Renderer *renderer = bar->renderer;

    SDL_FRect strip;
    get_strip_rect(content_rect, &strip);
    SDL_FRect strip_scaled = {
        .x = strip.x * scale,
        .y = strip.y * scale,
        .w = strip.w * scale,
        .h = strip.h * scale,
    };
    SDL_SetRenderDrawColor(renderer, bg.r, bg.g, bg.b, 255);
    SDL_RenderFillRect(renderer, &strip_scaled);

    Uint8 icon_r = 255 - bg.r;
    Uint8 icon_g = 255 - bg.g;
    Uint8 icon_b = 255 - bg.b;

    Uint8 hl_val = bg.r < 128 ? 255 : 0;

    for (int i = 0; i < SC_NAV_BAR_BTN_COUNT; ++i) {
        if (!bar->icons[i]) {
            continue;
        }

        SDL_FRect btn;
        sc_nav_bar_get_button_rect(bar, i, window_w, window_h, content_rect,
                                   &btn);

        if (interactive && (bar->hovered == i || bar->pressed == i)) {
            SDL_FRect btn_rect = {
                .x = (btn.x + 2.f) * scale,
                .y = (btn.y + 2.f) * scale,
                .w = (btn.w - 4.f) * scale,
                .h = (btn.h - 4.f) * scale,
            };
            Uint8 hl_alpha = bar->pressed == i ? 65 : 35;
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, hl_val, hl_val, hl_val, hl_alpha);
            SDL_RenderFillRect(renderer, &btn_rect);
        }

        float icon_x = btn.x + (btn.w - SC_NAV_ICON_SIZE) / 2.f;
        float icon_y = btn.y + (btn.h - SC_NAV_ICON_SIZE) / 2.f;
        SDL_FRect dst = {
            .x = icon_x * scale,
            .y = icon_y * scale,
            .w = SC_NAV_ICON_SIZE * scale,
            .h = SC_NAV_ICON_SIZE * scale,
        };

        SDL_SetTextureColorMod(bar->icons[i], icon_r, icon_g, icon_b);
        SDL_SetTextureAlphaMod(bar->icons[i], icon_alpha(bar, i, interactive));
        SDL_RenderTexture(renderer, bar->icons[i], NULL, &dst);
    }

    if (interactive && bar->hovered >= 0 && bar->hovered < SC_NAV_BAR_BTN_COUNT &&
            bar->tooltip_texts[bar->hovered][0] != '\0') {
        int idx = bar->hovered;
        SDL_FRect btn;
        sc_nav_bar_get_button_rect(bar, idx, window_w, window_h, content_rect,
                                   &btn);

        const char *text = bar->tooltip_texts[idx];
        size_t len = strlen(text);
        float tw = (float) len * 8.f;
        float th = 8.f;
        float pad_x = 6.f;
        float pad_y = 5.f;

        float box_w = (tw + pad_x * 2.f) * scale;
        float box_h = (th + pad_y * 2.f) * scale;
        float box_x = (btn.x - 8.f) * scale - box_w;
        float box_y = (btn.y + btn.h / 2.f) * scale - box_h / 2.f;

        // Clamp within window bounds
        if (box_y < 4.f * scale) {
            box_y = 4.f * scale;
        }
        if (box_y + box_h > window_h * scale - 4.f * scale) {
            box_y = window_h * scale - 4.f * scale - box_h;
        }

        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

        // Border (1px)
        SDL_FRect border_rect = {
            .x = box_x,
            .y = box_y,
            .w = box_w,
            .h = box_h,
        };
        SDL_SetRenderDrawColor(renderer, 75, 75, 75, 240);
        SDL_RenderFillRect(renderer, &border_rect);

        // Background
        SDL_FRect inner_rect = {
            .x = box_x + scale,
            .y = box_y + scale,
            .w = box_w - 2.f * scale,
            .h = box_h - 2.f * scale,
        };
        SDL_SetRenderDrawColor(renderer, 28, 28, 28, 240);
        SDL_RenderFillRect(renderer, &inner_rect);

        // Text
        float orig_scale_x = 1.0f;
        float orig_scale_y = 1.0f;
        SDL_GetRenderScale(renderer, &orig_scale_x, &orig_scale_y);
        SDL_SetRenderScale(renderer, scale, scale);

        SDL_SetRenderDrawColor(renderer, 240, 240, 240, 255);
        SDL_RenderDebugText(renderer, box_x / scale + pad_x,
                                      box_y / scale + pad_y, text);

        SDL_SetRenderScale(renderer, orig_scale_x, orig_scale_y);
    }
}

int
sc_nav_bar_hit_test(const struct sc_nav_bar *bar, float x, float y,
                    float window_w, float window_h,
                    const SDL_FRect *content_rect) {
    if (!sc_nav_bar_is_active(bar) || !content_rect) {
        return -1;
    }

    for (int i = 0; i < SC_NAV_BAR_BTN_COUNT; ++i) {
        SDL_FRect btn;
        sc_nav_bar_get_button_rect(bar, i, window_w, window_h, content_rect,
                                   &btn);
        if (x >= btn.x && x < btn.x + btn.w && y >= btn.y && y < btn.y + btn.h) {
            return i;
        }
    }

    return -1;
}

void
sc_nav_bar_get_action(int button_index, struct sc_nav_bar_action *action) {
    assert(button_index >= 0 && button_index < SC_NAV_BAR_BTN_COUNT);
    assert(action);
    *action = BUTTON_ACTIONS[button_index];
}

bool
sc_nav_bar_set_hovered(struct sc_nav_bar *bar, int button_index) {
    if (bar->hovered == button_index) {
        return false;
    }
    bar->hovered = button_index;
    return true;
}

bool
sc_nav_bar_contains_point(const struct sc_nav_bar *bar, float x, float y,
                          float window_w, float window_h,
                          const SDL_FRect *content_rect) {
    return sc_nav_bar_hit_test(bar, x, y, window_w, window_h, content_rect) >= 0;
}
