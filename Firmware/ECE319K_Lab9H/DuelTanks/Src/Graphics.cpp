// Graphics.cpp
// Duel Tanks - Layer 1: Graphics
// ECE319H Spring 2026

#include "../DuelTanks/Inc/Graphics.h"
#include "../inc/ST7735.h"      
#include <string.h>    
#include <stdlib.h>      
#include <cmath>

// ================================================================
// Internal state
// ================================================================
static int8_t  shakeX        = 0;
static int8_t  shakeY        = 0;
static uint8_t shakeFrames   = 0;

static bool    incomingShown = false;

// Language string tables
// Index 0 = English, Index 1 = Spanish
static const char* STR_PRESS_START[2]  = { "PRESS ANY BTN",  "PRESIONA BTN"    };
static const char* STR_ENGLISH[2]      = { "ENGLISH",        "INGLES"          };
static const char* STR_SPANISH[2]      = { "SPANISH",        "ESPANOL"         };
static const char* STR_RULES_T[2]      = { "HOW TO PLAY",    "COMO JUGAR"      };
static const char* STR_SYMBOLS_T[2]    = { "HUD SYMBOLS",    "SIMBOLOS"        };
static const char* STR_CONTROLS_T[2]   = { "CONTROLS",       "CONTROLES"       };
static const char* STR_SELECT_T[2]     = { "SELECT TANK",    "ELIGE TANQUE"    };
static const char* STR_WAITING[2]      = { "WAITING",        "ESPERANDO"       };
static const char* STR_FIGHT[2]        = { "FIGHT!",         "A LUCHAR!"       };
static const char* STR_VICTORY[2]      = { "VICTORY!",       "VICTORIA!"       };
static const char* STR_DEFEATED[2]     = { "DEFEATED",       "DERROTADO"       };
static const char* STR_REMATCH[2]      = { "REMATCH?",       "REVANCHA?"       };
static const char* STR_YES[2]          = { "B1:YES",         "B1:SI"           };
static const char* STR_NO[2]           = { "B2:NO",          "B2:NO"           };
static const char* STR_INCOMING[2]     = { "INCOMING!",      "CUIDADO!"        };
static const char* STR_NEXT[2]         = { "B1:NEXT",        "B1:SGTE"         };
static const char* STR_BACK[2]         = { "B2:BACK",        "B2:ATRAS"        };
static const char* STR_READY[2]        = { "READY!",         "LISTO!"          };

// Tank names
static const char* TANK_NAMES[3][2] = {
    { "BRAWLER",   "PELEADOR"  },
    { "PHANTOM",   "FANTASMA"  },
    { "ARTILLERY", "ARTILLERO" }
};

// Tank ability names (A1, A2 per tank)
static const char* TANK_A1[3][2] = {
    { "DBL SHOT",  "DOBLE TIR" },
    { "DASH",      "IMPULSO"   },
    { "CHARGE",    "CARGA"     }
};
static const char* TANK_A2[3][2] = {
    { "ARMOR",     "ARMADURA"  },
    { "CLOAK",     "SIGILO"    },
    { "RICOCHET",  "REBOTE"    }
};

static int16_t clamp(int16_t v, int16_t lo, int16_t hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

static void intToStr(int32_t val, char* buf) {
    if (val == 0) { buf[0] = '0'; buf[1] = '\0'; return; }
    char tmp[12];
    uint8_t i = 0;
    bool neg = (val < 0);
    if (neg) val = -val;
    while (val > 0) { tmp[i++] = '0' + (val % 10); val /= 10; }
    uint8_t j = 0;
    if (neg) buf[j++] = '-';
    while (i > 0) buf[j++] = tmp[--i];
    buf[j] = '\0';
}

void GFX_Init(void) {
    ST7735_InitR(INITR_REDTAB);    // init LCD with red tab config
    ST7735_SetRotation(3);         // landscape: 160 wide x 128 tall
    GFX_FillScreen(COLOR_BACKGROUND);
}

void GFX_FillScreen(uint16_t color) {
    ST7735_FillScreen(color);
}

void GFX_ClearPlayArea(void) {
    GFX_FillRect(0, PLAY_Y_MIN, SCREEN_W, PLAY_H, COLOR_BACKGROUND);
}


void GFX_FillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    // clip
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > SCREEN_W) w = SCREEN_W - x;
    if (y + h > SCREEN_H) h = SCREEN_H - y;
    if (w <= 0 || h <= 0) return;
    ST7735_FillRect(x, y, w, h, color);
}

void GFX_DrawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    GFX_DrawLine(x,       y,       x+w-1, y,       color); // top
    GFX_DrawLine(x,       y+h-1,   x+w-1, y+h-1,   color); // bottom
    GFX_DrawLine(x,       y,       x,     y+h-1,   color); // left
    GFX_DrawLine(x+w-1,   y,       x+w-1, y+h-1,   color); // right
}

void GFX_DrawPixel(int16_t x, int16_t y, uint16_t color) {
    if (x < 0 || x >= SCREEN_W || y < 0 || y >= SCREEN_H) return;
    ST7735_DrawPixel(x, y, color);
}

void GFX_DrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
    int16_t dx  =  abs(x1 - x0);
    int16_t dy  = -abs(y1 - y0);
    int16_t sx  = (x0 < x1) ? 1 : -1;
    int16_t sy  = (y0 < y1) ? 1 : -1;
    int16_t err = dx + dy;
    while (true) {
        GFX_DrawPixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        int16_t e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

void GFX_DrawCircle(int16_t cx, int16_t cy, int16_t r, uint16_t color) {
    int16_t x = r, y = 0, err = 0;
    while (x >= y) {
        GFX_DrawPixel(cx+x, cy+y, color); GFX_DrawPixel(cx+y, cy+x, color);
        GFX_DrawPixel(cx-y, cy+x, color); GFX_DrawPixel(cx-x, cy+y, color);
        GFX_DrawPixel(cx-x, cy-y, color); GFX_DrawPixel(cx-y, cy-x, color);
        GFX_DrawPixel(cx+y, cy-x, color); GFX_DrawPixel(cx+x, cy-y, color);
        y++;
        err += 2*y + 1;
        if (2*(err - x) + 1 > 0) { x--; err += 1 - 2*x; }
    }
}

void GFX_FillCircle(int16_t cx, int16_t cy, int16_t r, uint16_t color) {
    for (int16_t dy = -r; dy <= r; dy++) {
        int16_t dx = (int16_t)sqrt((float)(r*r - dy*dy));
        GFX_FillRect(cx - dx, cy + dy, 2*dx + 1, 1, color);
    }
}

void GFX_DrawChar(int16_t x, int16_t y, char c,
                  uint16_t color, uint16_t bg, uint8_t size) {
    ST7735_DrawCharS(x, y, c, color, bg, size);
}

void GFX_DrawString(int16_t x, int16_t y, const char* str,
                    uint16_t color, uint16_t bg, uint8_t size) {
    int16_t cx = x;
    while (*str) {
        GFX_DrawChar(cx, y, *str++, color, bg, size);
        cx += 6 * size;
    }
}

void GFX_DrawStringCentered(int16_t y, const char* str,
                             uint16_t color, uint16_t bg, uint8_t size) {
    int16_t len   = (int16_t)strlen(str);
    int16_t textW = len * 6 * size;
    int16_t x     = (SCREEN_W - textW) / 2;
    GFX_DrawString(x, y, str, color, bg, size);
}

void GFX_DrawInt(int16_t x, int16_t y, int32_t val,
                 uint16_t color, uint16_t bg, uint8_t size) {
    char buf[12];
    intToStr(val, buf);
    GFX_DrawString(x, y, buf, color, bg, size);
}

void GFX_SpriteInit(Sprite_t* s,
                    int16_t x, int16_t y,
                    uint8_t w, uint8_t h,
                    const uint16_t* bitmap) {
    s->x       = x;
    s->y       = y;
    s->prevX   = x;
    s->prevY   = y;
    s->w       = w;
    s->h       = h;
    s->visible = true;
    s->dirty   = true;
    s->bitmap  = bitmap;
}

void GFX_SpriteMove(Sprite_t* s, int16_t newX, int16_t newY) {
    if (s->x == newX && s->y == newY) return;  // no change, skip
    s->prevX = s->x;
    s->prevY = s->y;
    s->x     = newX;
    s->y     = newY;
    s->dirty = true;
}

void GFX_SpriteDraw(Sprite_t* s) {
    if (!s->dirty) return;

    if (s->prevX != s->x || s->prevY != s->y) {
        GFX_FillRect(s->prevX, s->prevY, s->w, s->h, COLOR_BACKGROUND);
    }

    if (s->visible && s->bitmap != nullptr) {
        ST7735_DrawBitmap(s->x, s->y + s->h - 1, s->bitmap, s->w, s->h);
    }

    s->dirty = false;
    s->prevX = s->x;
    s->prevY = s->y;
}

void GFX_SpriteErase(Sprite_t* s) {
    GFX_FillRect(s->x, s->y, s->w, s->h, COLOR_BACKGROUND);
    s->dirty = true;
}

void GFX_SpriteForceRedraw(Sprite_t* s) {
    s->dirty = true;
    s->prevX = s->x;
    s->prevY = s->y;
}

void GFX_SpriteSetBitmap(Sprite_t* s, const uint16_t* newBitmap) {
    if (s->bitmap == newBitmap) return;
    s->bitmap  = newBitmap;
    s->dirty   = true;
    s->prevX   = s->x;  // erase at same position
    s->prevY   = s->y;
}

// ================================================================
// HUD
// ================================================================

// Top strip layout:  [♥♥♥]  ----------  [★ N]
// Bottom strip:      [===cooldown===]

void GFX_DrawHUD(uint8_t lives, uint8_t kills, uint8_t cooldownPct) {
    // Draw top strip background
    GFX_FillRect(0, 0, SCREEN_W, HUD_TOP_H, COLOR_BLACK);
    GFX_UpdateLives(lives);
    GFX_UpdateKills(kills);

    // Draw bottom strip background
    GFX_FillRect(0, SCREEN_H - HUD_BOT_H, SCREEN_W, HUD_BOT_H, COLOR_BLACK);
    GFX_UpdateCooldownBar(cooldownPct);

    // Separator lines between HUD and play area
    GFX_DrawLine(0, HUD_TOP_H,            SCREEN_W-1, HUD_TOP_H,            COLOR_DARKGRAY);
    GFX_DrawLine(0, SCREEN_H - HUD_BOT_H, SCREEN_W-1, SCREEN_H - HUD_BOT_H, COLOR_DARKGRAY);
}

void GFX_UpdateLives(uint8_t lives) {
    // Erase lives area
    GFX_FillRect(0, 0, 52, HUD_TOP_H, COLOR_BLACK);
    // Draw hearts: filled red heart = alive, dark gray = lost
    for (uint8_t i = 0; i < 3; i++) {
        uint16_t c = (i < lives) ? COLOR_RED : COLOR_DARKGRAY;
        // Simple heart: two circles + triangle approximation at 5px
        int16_t hx = 4 + i * 17;
        int16_t hy = HUD_TOP_H / 2;
        GFX_FillCircle(hx + 2, hy - 1, 3, c);
        GFX_FillCircle(hx + 6, hy - 1, 3, c);
        // Triangle bottom of heart
        for (int8_t dy = 0; dy <= 4; dy++) {
            int16_t hw = 6 - dy;
            GFX_FillRect(hx + 4 - hw/2, hy + dy, hw, 1, c);
        }
    }
}

void GFX_UpdateKills(uint8_t kills) {
    // Erase kills area (right side of top strip)
    GFX_FillRect(SCREEN_W - 28, 0, 28, HUD_TOP_H, COLOR_BLACK);
    // Draw star icon + number
    GFX_DrawChar(SCREEN_W - 26, 3, '*', COLOR_GOLD, COLOR_BLACK, 1);
    GFX_DrawInt (SCREEN_W - 18, 3, kills, COLOR_WHITE, COLOR_BLACK, 1);
}

void GFX_UpdateCooldownBar(uint8_t pct) {
    int16_t barX = 2;
    int16_t barY = SCREEN_H - HUD_BOT_H + 2;
    int16_t barW = SCREEN_W - 4;
    int16_t barH = HUD_BOT_H - 4;

    // Background
    GFX_FillRect(barX, barY, barW, barH, COLOR_DARKGRAY);

    // Fill — color transitions green → yellow → red
    if (pct > 0) {
        uint16_t fillColor;
        if      (pct < 40)  fillColor = COLOR_GREEN;
        else if (pct < 75)  fillColor = COLOR_YELLOW;
        else                fillColor = COLOR_RED;
        int16_t fillW = (int16_t)(((int32_t)pct * barW) / 100);
        GFX_FillRect(barX, barY, fillW, barH, fillColor);
    }
    // Border
    GFX_DrawRect(barX - 1, barY - 1, barW + 2, barH + 2, COLOR_LIGHTGRAY);
}

// ================================================================
// FX
// ================================================================
void GFX_FlashScreen(uint16_t color, uint8_t numFrames) {
    for (uint8_t i = 0; i < numFrames; i++) {
        GFX_FillScreen(color);
        // Short busy delay (~16ms per frame feel)
        for (volatile uint32_t d = 0; d < 80000; d++);
    }
}

// Explosion frames: expanding rings drawn with circles
// cx, cy = center of explosion
void GFX_DrawExplosionFrame(int16_t cx, int16_t cy, uint8_t frame) {
    static const uint8_t  radii[4]  = {  3,  6,  9, 12 };
    static const uint16_t colors[4] = { COLOR_WHITE, COLOR_YELLOW,
                                        COLOR_ORANGE, COLOR_RED };
    // Erase previous frame first
    if (frame > 0) {
        GFX_FillCircle(cx, cy, radii[frame - 1] + 1, COLOR_BACKGROUND);
    }
    if (frame < 4) {
        GFX_FillCircle(cx, cy, radii[frame], colors[frame]);
        // Bright core
        GFX_FillCircle(cx, cy, radii[frame] / 2, COLOR_WHITE);
    }
}

void GFX_EraseExplosion(int16_t cx, int16_t cy) {
    GFX_FillCircle(cx, cy, 13, COLOR_BACKGROUND);
}

void GFX_DrawIncoming(void) {
    // Top-right of play area, red background
    GFX_FillRect(SCREEN_W - 62, PLAY_Y_MIN + 2, 60, 10, COLOR_RED);
    GFX_DrawString(SCREEN_W - 61, PLAY_Y_MIN + 3, "INCOMING!", COLOR_WHITE, COLOR_RED, 1);
    incomingShown = true;
}

void GFX_ClearIncoming(void) {
    if (!incomingShown) return;
    GFX_FillRect(SCREEN_W - 62, PLAY_Y_MIN + 2, 60, 10, COLOR_BACKGROUND);
    incomingShown = false;
}

void GFX_DrawChargeBar(int16_t x, int16_t y, uint8_t pct) {
    // 20px wide, 4px tall bar above tank
    int16_t barW  = 20;
    int16_t barH  = 4;
    GFX_FillRect(x, y, barW, barH, COLOR_DARKGRAY);
    if (pct > 0) {
        uint16_t c = (pct < 50) ? COLOR_GREEN :
                     (pct < 85) ? COLOR_YELLOW : COLOR_RED;
        int16_t fw = (int16_t)(((int32_t)pct * barW) / 100);
        GFX_FillRect(x, y, fw, barH, c);
    }
    GFX_DrawRect(x - 1, y - 1, barW + 2, barH + 2, COLOR_LIGHTGRAY);
}

void GFX_EraseChargeBar(int16_t x, int16_t y) {
    GFX_FillRect(x - 1, y - 1, 22, 6, COLOR_BACKGROUND);
}

void GFX_ShakeStart(int8_t intensity) {
    shakeX      = intensity;
    shakeY      = intensity / 2;
    shakeFrames = 5;
}

void GFX_ShakeTick(void) {
    if (shakeFrames == 0) { shakeX = 0; shakeY = 0; return; }
    shakeFrames--;
    shakeX = (shakeFrames % 2 == 0) ? 2 : -2;
    shakeY = (shakeFrames % 2 == 0) ? 1 : -1;
    if (shakeFrames == 0) { shakeX = 0; shakeY = 0; }
}

int8_t GFX_ShakeOffsetX(void) { return shakeX; }
int8_t GFX_ShakeOffsetY(void) { return shakeY; }


void GFX_DrawSplash(bool showPrompt) {
    GFX_FillScreen(COLOR_BLACK);

    // Title: "DUEL" in large yellow
    GFX_DrawStringCentered(20, "DUEL",  COLOR_YELLOW, COLOR_BLACK, 3);
    // "TANKS" in large white
    GFX_DrawStringCentered(50, "TANKS", COLOR_WHITE,  COLOR_BLACK, 3);

    // Decorative line
    GFX_DrawLine(20, 82, SCREEN_W - 20, 82, COLOR_DARKGRAY);

    // Subtitle
    GFX_DrawStringCentered(90, "ECE319H  SP2026", COLOR_DARKGRAY, COLOR_BLACK, 1);

    // Blinking prompt
    if (showPrompt) {
        GFX_DrawStringCentered(108, STR_PRESS_START[0], COLOR_CYAN, COLOR_BLACK, 1);
    } else {
        GFX_FillRect(0, 106, SCREEN_W, 12, COLOR_BLACK);
    }
}

void GFX_DrawLanguageSelect(uint8_t sel) {
    GFX_FillScreen(COLOR_BLACK);
    GFX_DrawStringCentered(10, "SELECT LANGUAGE", COLOR_WHITE, COLOR_BLACK, 1);
    GFX_DrawLine(0, 22, SCREEN_W, 22, COLOR_DARKGRAY);

    uint16_t c0 = (sel == 0) ? COLOR_BLACK  : COLOR_WHITE;
    uint16_t b0 = (sel == 0) ? COLOR_YELLOW : COLOR_BLACK;
    uint16_t c1 = (sel == 1) ? COLOR_BLACK  : COLOR_WHITE;
    uint16_t b1 = (sel == 1) ? COLOR_YELLOW : COLOR_BLACK;

    // English option
    GFX_FillRect(10, 40, 60, 16, b0);
    GFX_DrawString(14, 44, "ENGLISH", c0, b0, 1);

    // Spanish option
    GFX_FillRect(SCREEN_W - 70, 40, 60, 16, b1);
    GFX_DrawString(SCREEN_W - 66, 44, "ESPANOL", c1, b1, 1);

    // Instruction
    GFX_DrawStringCentered(75, "SLIDE=SELECT  B1=OK", COLOR_DARKGRAY, COLOR_BLACK, 1);

    // Arrow pointing to selected
    int16_t ax = (sel == 0) ? 40 : SCREEN_W - 40;
    GFX_DrawStringCentered(95, (sel == 0) ? "< ENGLISH" : "ESPANOL >", COLOR_CYAN, COLOR_BLACK, 1);
}

void GFX_DrawRulesPage(uint8_t page, uint8_t lang) {
    GFX_FillScreen(COLOR_BLACK);

    // Page titles
    const char* titles[3] = {
        STR_RULES_T[lang],
        STR_SYMBOLS_T[lang],
        STR_CONTROLS_T[lang]
    };
    GFX_DrawStringCentered(4, titles[page], COLOR_YELLOW, COLOR_BLACK, 1);
    GFX_DrawLine(0, 14, SCREEN_W, 14, COLOR_DARKGRAY);

    if (page == 0) {
        // How to play
        GFX_DrawString(4, 20, "Destroy the enemy tank", COLOR_WHITE, COLOR_BLACK, 1);
        GFX_DrawString(4, 30, "before they destroy you.", COLOR_WHITE, COLOR_BLACK, 1);
        GFX_DrawString(4, 44, "Fire shells across the", COLOR_CYAN, COLOR_BLACK, 1);
        GFX_DrawString(4, 54, "battlefield -- they can", COLOR_CYAN, COLOR_BLACK, 1);
        GFX_DrawString(4, 64, "cross to your opponent!", COLOR_CYAN, COLOR_BLACK, 1);
        GFX_DrawString(4, 78, "You have 3 lives.", COLOR_RED, COLOR_BLACK, 1);
        GFX_DrawString(4, 88, "First to 0 loses.", COLOR_RED, COLOR_BLACK, 1);
    }
    else if (page == 1) {
        // HUD symbols
        // Hearts
        GFX_FillCircle(14, 32, 4, COLOR_RED);
        GFX_FillCircle(20, 32, 4, COLOR_RED);
        GFX_DrawString(30, 28, "= Your lives", COLOR_WHITE, COLOR_BLACK, 1);

        // Star
        GFX_DrawChar(14, 46, '*', COLOR_GOLD, COLOR_BLACK, 2);
        GFX_DrawString(30, 50, "= Kill count", COLOR_WHITE, COLOR_BLACK, 1);

        // Shell dot
        GFX_FillCircle(16, 68, 3, COLOR_YELLOW);
        GFX_DrawString(30, 65, "= Shell (dodge!)", COLOR_WHITE, COLOR_BLACK, 1);

        // Barrier
        GFX_FillRect(10, 80, 12, 12, COLOR_BLUE);
        GFX_DrawString(30, 82, "= Barrier wall", COLOR_WHITE, COLOR_BLACK, 1);

        // Cooldown bar
        GFX_DrawString(4, 98, "[====] = Ability CD", COLOR_GREEN, COLOR_BLACK, 1);
    }
    else {
        // Controls
        GFX_DrawString(4, 20, "SLIDE POT:", COLOR_CYAN, COLOR_BLACK, 1);
        GFX_DrawString(4, 30, "  Aim turret (360deg)", COLOR_WHITE, COLOR_BLACK, 1);

        GFX_DrawString(4, 44, "BUTTON 1 (FIRE):", COLOR_CYAN, COLOR_BLACK, 1);
        GFX_DrawString(4, 54, "  Shoot shell", COLOR_WHITE, COLOR_BLACK, 1);

        GFX_DrawString(4, 68, "BUTTON 2 (ABILITY):", COLOR_CYAN, COLOR_BLACK, 1);
        GFX_DrawString(4, 78, "  Use tank ability", COLOR_WHITE, COLOR_BLACK, 1);

        GFX_DrawString(4, 92, "Tank ability varies!", COLOR_YELLOW, COLOR_BLACK, 1);
        GFX_DrawString(4, 102, "Check tank select.", COLOR_YELLOW, COLOR_BLACK, 1);
    }

    for (uint8_t i = 0; i < 3; i++) {
        uint16_t c = (i == page) ? COLOR_WHITE : COLOR_DARKGRAY;
        GFX_FillCircle(SCREEN_W/2 - 8 + i*8, SCREEN_H - 8, 2, c);
    }

    // Navigation hints
    GFX_DrawString(2,          SCREEN_H - 12, STR_NEXT[lang], COLOR_DARKGRAY, COLOR_BLACK, 1);
    GFX_DrawString(SCREEN_W - 44, SCREEN_H - 12, STR_BACK[lang], COLOR_DARKGRAY, COLOR_BLACK, 1);
}

void GFX_DrawTankSelect(uint8_t sel, int8_t oppSel, bool oppReady, uint8_t lang) {
    GFX_FillScreen(COLOR_BLACK);
    GFX_DrawStringCentered(2, STR_SELECT_T[lang], COLOR_YELLOW, COLOR_BLACK, 1);
    GFX_DrawLine(0, 12, SCREEN_W, 12, COLOR_DARKGRAY);

    const uint8_t speeds[3]  = { 55, 90, 35 };
    const uint8_t armors[3]  = { 80, 35, 50 };
    const uint8_t powers[3]  = { 65, 55, 95 };

    int16_t cardW = 48;
    int16_t cardX[3] = { 4, 56, 108 };
    int16_t cardY = 16;
    int16_t cardH = 90;

    for (uint8_t t = 0; t < 3; t++) {
        bool isSelected  = (t == sel);
        bool isOppSelect = (oppSel >= 0 && (uint8_t)oppSel == t);

        uint16_t borderColor = isSelected  ? COLOR_YELLOW :
                               isOppSelect ? COLOR_CYAN   : COLOR_DARKGRAY;
        uint16_t bgColor     = isSelected  ? 0x1082       : COLOR_BLACK;

        GFX_FillRect(cardX[t], cardY, cardW, cardH, bgColor);
        GFX_DrawRect(cardX[t], cardY, cardW, cardH, borderColor);

        // Tank name
        GFX_DrawString(cardX[t] + 2, cardY + 3, TANK_NAMES[t][lang],
                       isSelected ? COLOR_YELLOW : COLOR_WHITE, bgColor, 1);

        // Stat bars
        const char* statLabels[3] = { "SPD", "ARM", "PWR" };
        uint8_t statVals[3] = { speeds[t], armors[t], powers[t] };
        uint16_t statColors[3] = { COLOR_CYAN, COLOR_GREEN, COLOR_RED };

        for (uint8_t s = 0; s < 3; s++) {
            int16_t sy = cardY + 18 + s * 12;
            GFX_DrawString(cardX[t] + 2, sy, statLabels[s], COLOR_DARKGRAY, bgColor, 1);
            // Bar: 28px wide
            int16_t bx = cardX[t] + 20;
            GFX_FillRect(bx, sy, 28, 5, COLOR_DARKGRAY);
            int16_t fw = (int16_t)(((int32_t)statVals[s] * 28) / 100);
            GFX_FillRect(bx, sy, fw, 5, statColors[s]);
        }

        // Abilities
        GFX_DrawString(cardX[t] + 2, cardY + 58, TANK_A1[t][lang],
                       COLOR_CYAN, bgColor, 1);
        GFX_DrawString(cardX[t] + 2, cardY + 70, TANK_A2[t][lang],
                       COLOR_CYAN, bgColor, 1);

        // Opponent marker
        if (isOppSelect) {
            GFX_DrawString(cardX[t] + 2, cardY + 80, "OPP",
                           COLOR_CYAN, bgColor, 1);
        }
        if (isOppSelect && oppReady) {
            GFX_DrawString(cardX[t] + 2, cardY + 80, STR_READY[lang],
                           COLOR_GREEN, bgColor, 1);
        }
    }

    GFX_DrawString(2, SCREEN_H - 10, "SLIDE=PICK  B1=OK", COLOR_DARKGRAY, COLOR_BLACK, 1);
}

void GFX_DrawWaiting(uint8_t dotFrame, uint8_t lang) {
    // Only update the dots, not the full screen (reduces flicker on this screen)
    // First call: draw full screen
    static bool firstCall = true;
    if (firstCall) {
        GFX_FillScreen(COLOR_BLACK);
        GFX_DrawStringCentered(50, STR_WAITING[lang], COLOR_WHITE, COLOR_BLACK, 2);
        firstCall = false;
    }

    // Animated dots
    static const char* dots[4] = { ".  ", ".. ", "...", "   " };
    GFX_DrawStringCentered(76, dots[dotFrame % 4], COLOR_CYAN, COLOR_BLACK, 2);
}

void GFX_DrawCountdown(uint8_t n) {
    if (n > 0) {
        GFX_FillScreen(COLOR_BLACK);
        char buf[2] = { (char)('0' + n), '\0' };
        // Giant number
        GFX_DrawStringCentered(44, buf, COLOR_WHITE, COLOR_BLACK, 4);
    } else {
        // FIGHT! — white flash then text
        GFX_FillScreen(COLOR_WHITE);
        for (volatile uint32_t d = 0; d < 120000; d++);
        GFX_FillScreen(COLOR_BLACK);
        GFX_DrawStringCentered(48, "FIGHT!", COLOR_YELLOW, COLOR_BLACK, 3);
        for (volatile uint32_t d = 0; d < 400000; d++);
    }
}

void GFX_DrawEndScreen(bool won, uint8_t myKills, uint8_t lang) {
    GFX_FillScreen(COLOR_BLACK);

    if (won) {
        GFX_DrawStringCentered(20, STR_VICTORY[lang], COLOR_GOLD,  COLOR_BLACK, 2);
        GFX_DrawStringCentered(50, "YOU WIN!",         COLOR_GREEN, COLOR_BLACK, 1);
    } else {
        GFX_DrawStringCentered(20, STR_DEFEATED[lang], COLOR_RED,   COLOR_BLACK, 2);
        GFX_DrawStringCentered(50, "BETTER LUCK",      COLOR_WHITE, COLOR_BLACK, 1);
        GFX_DrawStringCentered(62, "NEXT TIME",        COLOR_WHITE, COLOR_BLACK, 1);
    }

    // Kills this match
    GFX_DrawStringCentered(80, "KILLS THIS MATCH:", COLOR_DARKGRAY, COLOR_BLACK, 1);
    GFX_DrawStringCentered(92, "",                  COLOR_WHITE,    COLOR_BLACK, 1);
    GFX_DrawInt((SCREEN_W/2) - 3, 92, myKills, COLOR_WHITE, COLOR_BLACK, 1);
}

void GFX_DrawRematch(uint8_t lang) {
    // Drawn below end screen content
    GFX_DrawLine(0, 108, SCREEN_W, 108, COLOR_DARKGRAY);
    GFX_DrawStringCentered(112, STR_REMATCH[lang], COLOR_WHITE, COLOR_BLACK, 1);
    GFX_DrawString(4,            120, STR_YES[lang], COLOR_GREEN, COLOR_BLACK, 1);
    GFX_DrawString(SCREEN_W - 34, 120, STR_NO[lang],  COLOR_RED,   COLOR_BLACK, 1);
}
