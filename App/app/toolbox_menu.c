/*
 * Dondji Firmware
 *
 * Copyright (c) 2026 BD1AHN
 *
 * Licensed under the Apache License, Version 2.0
 *
 * Project:
 *      叮咚鸡 (Dondji)
 *
 * Maintainer:
 *      BD1AHN
 *
 * Official Website:
 *      https://ethanyan6.github.io/Dondji/
 *
 * The Dondji name, logo, and official project identity
 * are protected separately from the source code license.
 */

#include "app/toolbox_menu.h"
#include "app/cw.h"
#include "keyboard_state.h"
#include "../driver/bk4819.h"
#include "../driver/gpio.h"
#include "../audio.h"
#include "../font.h"
#include "../ui/helper.h"
#include "../settings.h"
#include <string.h>

#ifdef ENABLE_FEAT_F4HWN_SCREENSHOT
#include "screenshot.h"
#endif

static bool isInitialized;
static KeyboardState kbd;

static void DrawMenu(void)
{
    // Clear screen
    for (int row = 0; row < FRAME_LINES; row++) {
        memset(gFrameBuffer[row], 0, LCD_WIDTH);
    }
    memset(gStatusLine, 0, sizeof(gStatusLine));

    // Title at row 0, centered
    const char *title = (gUiLanguage == UI_LANGUAGE_CN) ? "工具箱" : "Toolbox";
    uint8_t title_x_start = (gUiLanguage == UI_LANGUAGE_CN) ? 51 : 0;
    uint8_t title_x_end = (gUiLanguage == UI_LANGUAGE_CN) ? 86 : 127;
    UI_PrintStringSmallAtPixel(title, title_x_start, title_x_end, 0, 11, 0);

    // Separator line at y=13
    for (int x = 0; x < LCD_WIDTH; x++) {
        UI_DrawPixelBuffer(gFrameBuffer, (uint8_t)x, 13, true);
    }

    // Menu item (Only CW left)
    const char *item = "cw";
    uint8_t row = 3;
    UI_PrintStringSmallAtPixel(item, 0, 127, row * 8 + 3, row * 8 + 14, 0);
    
    // Invert selection background
    uint8_t fb_start = (row * 8 >= 8) ? (row * 8 - 8) / 8 : 0;
    uint8_t fb_end = (row * 8 + 11 - 8) / 8;
    for (uint8_t r = fb_start; r <= fb_end; r++) {
        for (int x = 0; x < LCD_WIDTH; x++) {
            gFrameBuffer[r][x] ^= 0xFF;
        }
    }
}

static bool HandleInput(void)
{
    kbd.prev = kbd.current;
    kbd.current = KEYBOARD_GetKey();

    if (kbd.current == KEY_EXIT) {
        kbd.counter++;
        if (kbd.counter > 20) {
            isInitialized = false;
            return false;
        }
    } else if (kbd.prev == KEY_EXIT && kbd.counter > 0) {
        // Short press EXIT - exit menu
        isInitialized = false;
        return false;
    } else {
        kbd.counter = 0;
    }

    if (kbd.current == KEY_INVALID)
        return true;

    if (kbd.current != kbd.prev) {
        switch (kbd.current) {
        case KEY_MENU:
            // Enter CW
            APP_RunCW();
            break;
        default:
            break;
        }
    }

    return true;
}

void APP_RunToolboxMenu(void)
{
    BACKLIGHT_UpdateTickless();

    isInitialized = true;
    kbd.current = KEY_INVALID;
    kbd.prev = KEY_INVALID;
    kbd.counter = 0;

    DrawMenu();
    ST7565_BlitStatusLine();
    ST7565_BlitFullScreen();

    while (isInitialized) {
        #ifdef ENABLE_FEAT_F4HWN_SCREENSHOT
            SCREENSHOT_ParseInput();
        #endif
        HandleInput();
        DrawMenu();
        ST7565_BlitStatusLine();
        ST7565_BlitFullScreen();
        #ifdef ENABLE_FEAT_F4HWN_SCREENSHOT
            SCREENSHOT_Update(false);
        #endif
        SYSTEM_DelayMs(40);
    }
}
