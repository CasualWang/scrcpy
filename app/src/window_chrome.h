#ifndef SC_WINDOW_CHROME_H
#define SC_WINDOW_CHROME_H

#include "common.h"

#include <stdbool.h>
#include <stdint.h>

#include <SDL3/SDL.h>

#include "nav_bar.h"

#define SC_TITLE_BAR_HEIGHT 32
#define SC_WINDOW_CTRL_WIDTH 46
#define SC_RESIZE_BORDER 5

#define SC_CHROME_TITLE_BG_R 0x3C
#define SC_CHROME_TITLE_BG_G 0x3F
#define SC_CHROME_TITLE_BG_B 0x41

#define SC_CHROME_TOOLBAR_BG_R 0x2B
#define SC_CHROME_TOOLBAR_BG_G 0x2B
#define SC_CHROME_TOOLBAR_BG_B 0x2B

#define SC_CHROME_BORDER_R 0x51
#define SC_CHROME_BORDER_G 0x56
#define SC_CHROME_BORDER_B 0x58

#define SC_CHROME_TEXT_R 0xBB
#define SC_CHROME_TEXT_G 0xBB
#define SC_CHROME_TEXT_B 0xBB

#define SC_CHROME_HOVER_R 0x4B
#define SC_CHROME_HOVER_G 0x4F
#define SC_CHROME_HOVER_B 0x51

#define SC_CHROME_PRESSED_R 0x51
#define SC_CHROME_PRESSED_G 0x56
#define SC_CHROME_PRESSED_B 0x58

#define SC_CHROME_CLOSE_HOVER_R 0xE8
#define SC_CHROME_CLOSE_HOVER_G 0x11
#define SC_CHROME_CLOSE_HOVER_B 0x23

#define SC_CHROME_SET_COLOR(renderer, r, g, b) \
    SDL_SetRenderDrawColor(renderer, r, g, b, 255)

struct sc_screen;

bool
sc_window_chrome_register(struct sc_screen *screen);

void
sc_window_chrome_unregister(struct sc_screen *screen);

void
sc_window_chrome_apply_minimum_size(SDL_Window *window,
                                    const struct sc_screen *screen);

bool
sc_window_chrome_is_active(const struct sc_screen *screen);

uint16_t
sc_window_chrome_get_title_height(const struct sc_screen *screen);

uint16_t
sc_window_chrome_get_nav_width(const struct sc_screen *screen);

#endif
