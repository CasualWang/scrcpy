#include "font.h"

#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "util/env.h"
#include "util/file.h"
#include "util/log.h"

#define SCRCPY_DEFAULT_FONT_DIR PREFIX "/share/scrcpy"
#define SCRCPY_FONT_FILENAME "OpenSans-Regular.ttf"

#ifndef _WIN32
static char *
try_path(const char *path) {
    if (!path || !sc_file_is_regular(path)) {
        return NULL;
    }
    return strdup(path);
}
#endif

static char *
get_system_font_path(void) {
#ifdef _WIN32
    const char *windir = getenv("WINDIR");
    if (windir) {
        static const char *filenames[] = {
            "Fonts\\msyh.ttc",
            "Fonts\\msyh.ttf",
            "Fonts\\segoeui.ttf",
            "Fonts\\arial.ttf",
        };
        for (size_t i = 0; i < sizeof(filenames) / sizeof(filenames[0]); ++i) {
            char *path = sc_file_build_path(windir, filenames[i]);
            if (path && sc_file_is_regular(path)) {
                return path;
            }
            free(path);
        }
    }
#elif defined(__APPLE__)
    static const char *paths[] = {
        "/System/Library/Fonts/PingFang.ttc",
        "/System/Library/Fonts/Supplemental/Arial Unicode.ttf",
        "/System/Library/Fonts/Supplemental/Arial.ttf",
        "/System/Library/Fonts/Helvetica.ttc",
        "/Library/Fonts/Arial.ttf",
    };
    for (size_t i = 0; i < sizeof(paths) / sizeof(paths[0]); ++i) {
        char *path = try_path(paths[i]);
        if (path) {
            return path;
        }
    }
#else
    static const char *paths[] = {
        "/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc",
        "/usr/share/fonts/truetype/wqy/wqy-microhei.ttc",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/TTF/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
        "/usr/share/fonts/truetype/freefont/FreeSans.ttf",
    };
    for (size_t i = 0; i < sizeof(paths) / sizeof(paths[0]); ++i) {
        char *path = try_path(paths[i]);
        if (path) {
            return path;
        }
    }
#endif
    return NULL;
}

char *
sc_font_get_path(void) {
    char *font_path;

    char *font_dir = sc_get_env("SCRCPY_FONT_DIR");
    if (font_dir) {
        font_path = sc_file_build_path(font_dir, SCRCPY_FONT_FILENAME);
        free(font_dir);
        if (!font_path) {
            LOG_OOM();
            return NULL;
        }
        if (sc_file_is_regular(font_path)) {
            LOGD("Using font from SCRCPY_FONT_DIR: %s", font_path);
            return font_path;
        }
        free(font_path);
    }

#ifndef PORTABLE
    font_path = sc_file_build_path(SCRCPY_DEFAULT_FONT_DIR, SCRCPY_FONT_FILENAME);
    if (!font_path) {
        LOG_OOM();
        return NULL;
    }
    if (sc_file_is_regular(font_path)) {
        LOGD("Using font: %s", font_path);
        return font_path;
    }
    free(font_path);
#else
    font_path = sc_file_get_local_path(SCRCPY_FONT_FILENAME);
    if (font_path && sc_file_is_regular(font_path)) {
        LOGD("Using font (portable): %s", font_path);
        return font_path;
    }
    free(font_path);
#endif

    font_path = get_system_font_path();
    if (font_path) {
        LOGD("Using system font: %s", font_path);
    }
    return font_path;
}
