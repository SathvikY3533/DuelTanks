// Graphics.h
// Duel Tanks - wrapper func for ST7735 Display
// ST7735 in landscape mode (160x128)
// LCD pins from schematic: PB15(CS) PB6(DC) PA13(RST) PB8(SCK) PB9(MOSI)
// Author: Sathvik
// ECE319H Spring 2026

#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdint.h>
#include <stdbool.h>

// screen size
#define SCREEN_W        160
#define SCREEN_H        128

// HUD rect strips - to prevent from entire screen from refreshing
#define HUD_TOP_H       14      // pixels tall: lives + kills strip
#define HUD_BOT_H       10      // pixels tall: cooldown bar strip
#define PLAY_Y_MIN      HUD_TOP_H
#define PLAY_Y_MAX      (SCREEN_H - HUD_BOT_H - 1)
#define PLAY_H          (SCREEN_H - HUD_TOP_H - HUD_BOT_H)

// colors!
#define COLOR_BLACK       ((uint16_t)0x0000)
#define COLOR_WHITE       ((uint16_t)0xFFFF)
#define COLOR_RED         ((uint16_t)0xF800)
#define COLOR_GREEN       ((uint16_t)0x07E0)
#define COLOR_BLUE        ((uint16_t)0x001F)
#define COLOR_YELLOW      ((uint16_t)0xFFE0)
#define COLOR_CYAN        ((uint16_t)0x07FF)
#define COLOR_MAGENTA     ((uint16_t)0xF81F)
#define COLOR_ORANGE      ((uint16_t)0xFC60)
#define COLOR_DARKGRAY    ((uint16_t)0x39E7)
#define COLOR_LIGHTGRAY   ((uint16_t)0xC618)
#define COLOR_DARKGREEN   ((uint16_t)0x03E0)
#define COLOR_GOLD        ((uint16_t)0xFEA0)
#define COLOR_BACKGROUND  ((uint16_t)0x0841)  // dark blue-gray battlefield

// ================================================================
// Sprite struct — enables "hit box" to prevent screen flicker
//
// only redraws when 'dirty' is true - meaning state of sprite has changed
// ================================================================
typedef struct {
    int16_t         x;          // current x (top-left)
    int16_t         y;          // current y (top-left)
    int16_t         prevX;      // previous x for erase
    int16_t         prevY;      // previous y for erase
    uint8_t         w;          // width  in pixels
    uint8_t         h;          // height in pixels
    bool            visible;    // false = skip draw (cloak ability)
    bool            dirty;      // true = needs redraw
    const uint16_t* bitmap;     // RGB565 pixel array, row-major
} Sprite_t;

// GFX init
void GFX_Init(void);

// color entire screen
void GFX_FillScreen(uint16_t color);

// Clears non-hud area (using the HUD strips above)
void GFX_ClearPlayArea(void);

// basic shape shi
void GFX_FillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void GFX_DrawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void GFX_DrawPixel(int16_t x, int16_t y, uint16_t color);
void GFX_DrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
void GFX_DrawCircle(int16_t cx, int16_t cy, int16_t r, uint16_t color);
void GFX_FillCircle(int16_t cx, int16_t cy, int16_t r, uint16_t color);

// Texst stuff
void GFX_DrawChar(int16_t x, int16_t y, char c,
                  uint16_t color, uint16_t bg, uint8_t size);
void GFX_DrawString(int16_t x, int16_t y, const char* str,
                    uint16_t color, uint16_t bg, uint8_t size);

// special string out for splash screens
void GFX_DrawStringCentered(int16_t y, const char* str,
                             uint16_t color, uint16_t bg, uint8_t size);

// fixed point helper func
void GFX_DrawInt(int16_t x, int16_t y, int32_t val,
                 uint16_t color, uint16_t bg, uint8_t size);




// Initialize a sprite. Sets prevX/prevY = x/y, dirty = true, visible = true
void GFX_SpriteInit(Sprite_t* s,
                    int16_t x, int16_t y,
                    uint8_t w, uint8_t h,
                    const uint16_t* bitmap);

// Update position — stores new coords, marks dirty (meaning need redraw)
void GFX_SpriteMove(Sprite_t* s, int16_t newX, int16_t newY);

// Main draw call — erases old position, draws at new position
// Only acts if dirty == true and visible == true - prevent from unneccesary processing
void GFX_SpriteDraw(Sprite_t* s);

// Erase sprite at current position
void GFX_SpriteErase(Sprite_t* s);

// Force a full redraw even if dirty == false (debugging - it works now!!!)
void GFX_SpriteForceRedraw(Sprite_t* s);

// Swap bitmap (mostly for animating the aim line)
void GFX_SpriteSetBitmap(Sprite_t* s, const uint16_t* newBitmap);


// GFX_DrawHUD = draw the full HUD + helper funcs to update HUD values
void GFX_DrawHUD(uint8_t lives, uint8_t kills, uint8_t cooldownPct);
void GFX_UpdateLives(uint8_t lives);          // redraws lives section only
void GFX_UpdateKills(uint8_t kills);          // redraws kills section only
void GFX_UpdateCooldownBar(uint8_t pct);      // 0=empty, 100=full


//flash entire screen for a very short time - used for shoig damage taken
void GFX_FlashScreen(uint16_t color, uint8_t numFrames);

// Explosion BOOOOM
void GFX_DrawExplosionFrame(int16_t cx, int16_t cy, uint8_t frame);

// Explosion bye :(
void GFX_EraseExplosion(int16_t cx, int16_t cy);

// INCOMING! warning - top right of screen
void GFX_DrawIncoming(void);
void GFX_ClearIncoming(void);

// Charge bar stuff for shooting shots
void GFX_DrawChargeBar(int16_t x, int16_t y, uint8_t pct);
void GFX_EraseChargeBar(int16_t x, int16_t y);

// screen shaking effects
void GFX_ShakeStart(int8_t intensity);
void GFX_ShakeTick(void);                    // call once per game frame
int8_t GFX_ShakeOffsetX(void);              // add to all x coords during shake
int8_t GFX_ShakeOffsetY(void);              // add to all y coords during shake




// --- Splash screens ------------------------------------------

// Each function does a full screen redraw — only call on state transitions

// Splash screen helper func
void GFX_DrawSplash(bool showPrompt);

// Language select helper func
void GFX_DrawLanguageSelect(uint8_t sel);

// Rules pages helper func
void GFX_DrawRulesPage(uint8_t page, uint8_t lang);

// Tank select helper func
void GFX_DrawTankSelect(uint8_t sel, int8_t oppSel, bool oppReady, uint8_t lang);

// Waiting for oppon screen func
void GFX_DrawWaiting(uint8_t dotFrame, uint8_t lang);

// Countdown and then FIGHT! screen transition
void GFX_DrawCountdown(uint8_t n);

// End screen - vicotry/defeat screen
void GFX_DrawEndScreen(bool won, uint8_t myKills, uint8_t lang);

// Rematch prompt screen
void GFX_DrawRematch(uint8_t lang);

#endif // GRAPHICS_H
