// Game.cpp
// Duel Tanks — ECE319H Spring 2026
// Complete game implementation: state machine, physics, rendering

#include "../DuelTanks/Inc/Game.h"
#include "../inc/Clock.h"
#include "../inc/ST7735.h"

// Trig lookup tables
const int8_t cos32[90] = {
     32, 32, 32, 32, 32, 32, 32, 31, 31, 31,
     31, 30, 30, 30, 29, 29, 28, 28, 27, 27,
     26, 25, 25, 24, 23, 22, 22, 21, 20, 19,
     18, 18, 17, 16, 15, 14, 13, 12, 11, 10,
      9,  8,  7,  6,  5,  4,  3,  2,  1,  0,
     -1, -2, -3, -4, -5, -6, -7, -8, -9,-10,
    -11,-12,-13,-14,-15,-16,-17,-18,-18,-19,
    -20,-21,-22,-22,-23,-24,-25,-25,-26,-27,
    -27,-28,-28,-29,-29,-30,-30,-30,-31,-31
};
const int8_t sin32[90] = {
      0,  1,  1,  2,  2,  3,  3,  4,  4,  5,
      6,  6,  7,  7,  8,  8,  9,  9, 10, 10,
     11, 11, 12, 12, 13, 14, 14, 15, 15, 16,
     16, 17, 17, 18, 18, 19, 19, 20, 20, 21,
     21, 22, 22, 23, 23, 24, 24, 25, 25, 26,
     26, 27, 27, 28, 28, 29, 29, 30, 30, 30,
     31, 31, 31, 32, 32, 32, 32, 32, 32, 32,
     32, 32, 32, 32, 32, 32, 31, 31, 31, 31,
     30, 30, 30, 29, 29, 28, 28, 27, 27, 26
};

// ================================================================
// Global game state — extern declared in Game.h
// ================================================================
GameState   gameState        = STATE_SPLASH;
uint8_t     lang             = 0;
uint8_t     rulesPage        = 0;
uint8_t     tankType         = 0;
uint8_t     oppTankType      = 0;
int16_t     tankX            = TANK_START_X;
int16_t     tankY            = 0;
bool        facingRight      = (bool)FACING_RIGHT;
uint8_t     myLives          = MAX_LIVES;
uint8_t     oppLives         = MAX_LIVES;
uint8_t     myKills          = 0;
uint8_t     invincibleTimer  = 0;
uint8_t     flashTimer       = 0;
uint8_t     incomingWarnTimer= 0;
ChargeState chargeState      = CHARGE_IDLE;
uint8_t     chargeLevel      = 0;
uint8_t     aimAngle         = 35;
Missile_t   missiles[MAX_MISSILES]    = {};
Explosion_t explosions[MAX_EXPLOSIONS]= {};
uint8_t     countdownNum     = 3;
uint8_t     countdownTimer   = 0;
bool        playerWon        = false;
bool        showPrompt       = true;
uint8_t     abilityCooldown  = 0;
uint8_t     speedBoostTimer  = 0;

// Previous aim line endpoints — for dirty-rect erase
static int16_t prevBarrelX   = 0, prevBarrelY   = 0;
static int16_t prevAimEndX   = 0, prevAimEndY   = 0;
static bool    aimLineDrawn  = false;

// Charge bar position — tracked so erase finds it even when tank moves
static int16_t chargeBarX    = -1, chargeBarY   = -1;
static uint8_t prevChargeLevel = 255;

// Splash screen animation
static int16_t splashMissileXfp[2], splashMissileYfp[2];
static int16_t splashMissileVx[2],  splashMissileVy[2];
static int16_t splashMissilePrevX[2], splashMissilePrevY[2];
static bool    splashMissileActive[2] = {false, false};
static uint8_t splashMissileTimer     = 0;
static uint8_t splashBlinkTimer       = 0;

// Game timer (counts down from 120 seconds)
static uint16_t matchTimerSec   = 120;
static uint8_t  matchTimerFrames= 0;
static uint8_t  timerBlinkTick  = 0;   // used for sub-second flash when <= 10 sec

// Battle ready semaphore
static bool readySignalSent = false;
static uint8_t waitingAnimTick = 0;

// Ricochet pending - the extra ability power up
static bool ricochetNextShot = false;

// Combo exit (FIRE + LEFT + RIGHT held)
static uint8_t comboHeldTicks = 0;

//-----------------------------------------------------------------------------------

// Language string selector
static const char* GS(const char* english, const char* spanish) {
    return lang ? spanish : english;
}

static bool RectsOverlap(int16_t ax, int16_t ay, int16_t aw, int16_t ah,
                          int16_t bx, int16_t by, int16_t bw, int16_t bh) {
    return ax < bx + bw  &&  ax + aw > bx  &&
           ay < by + bh  &&  ay + ah > by;
}

// find pos of missle
static int8_t FindFreeMissile(void) {
    for (int i = 0; i < MAX_MISSILES; i++) {
        if (!missiles[i].active) return i;
    }
    return -1;
}

// to find index of explosion
static int8_t FindFreeExplosion(void) {
    for (int i = 0; i < MAX_EXPLOSIONS; i++) {
        if (!explosions[i].active) return i;
    }
    return -1;
}

// create terrain
static uint8_t TerrainAt(int16_t x) {
    if (x < 0) x = 0;
    if (x >= SCREEN_W) x = SCREEN_W - 1;
    return TerrainY[x];
}


// Background color lookup  - used to redraw a pixel while bullet travels in the air

static const int16_t leftBldgX[]   = { 0, 15, 28, 39, 59, 69};
static const int16_t leftBldgTop[] = {26, 14, 30, 20, 34, 28};
static const int16_t leftBldgW[]   = {15, 13, 11, 20, 10,  4};
static const int16_t rightBldgX[]   = { 88, 102, 118, 131, 148};
static const int16_t rightBldgTop[] = { 22,  11,  26,  16,  30};
static const int16_t rightBldgW[]   = { 14,  16,  13,  17,  12};

static uint16_t GetBackgroundColor(int16_t x, int16_t y) {
    uint8_t terrainY = TerrainAt(x);

    // Below terrain = earth
    if (y >= (int16_t)terrainY + 3) return EARTH_COLOR;
    // On terrain = grass
    if (y >= (int16_t)terrainY)     return GRASS_COLOR;

    // Above terrain — check buildings
    uint16_t buildingColor = rgb(6, 9, 32);
    uint16_t warmWindow    = rgb(200, 180, 80);
    uint16_t coolWindow    = rgb(180, 200, 220);

    // Left buildings (even-indexed ones have sparse windows)
    for (int i = 0; i < 6; i++) {
        if (x >= leftBldgX[i] && x < leftBldgX[i] + leftBldgW[i] && y >= leftBldgTop[i]) {
            if (i % 2 == 0 && y >= leftBldgTop[i] + 4 && y < 80) {
                bool onWindowRow = (y - leftBldgTop[i] - 4) % 10 == 0;
                bool onWindowCol = (x - leftBldgX[i] - 2) % 6 == 0;
                bool inBounds    = x >= leftBldgX[i] + 2 && x < leftBldgX[i] + leftBldgW[i] - 1;
                if (onWindowRow && onWindowCol && inBounds && (x + y) % 7 != 0) {
                    return ((x + y) % 3 == 0) ? coolWindow : warmWindow;
                }
            }
            return buildingColor;
        }
    }

    // Right buildings
    for (int i = 0; i < 5; i++) {
        if (x >= rightBldgX[i] && x < rightBldgX[i] + rightBldgW[i] && y >= rightBldgTop[i]) {
            if (i % 2 == 0 && y >= rightBldgTop[i] + 4 && y < 80) {
                bool onWindowRow = (y - rightBldgTop[i] - 4) % 10 == 0;
                bool onWindowCol = (x - rightBldgX[i] - 2) % 6 == 0;
                bool inBounds    = x >= rightBldgX[i] + 2 && x < rightBldgX[i] + rightBldgW[i] - 1;
                if (onWindowRow && onWindowCol && inBounds && (x + y) % 7 != 0) {
                    return ((x + y) % 3 == 0) ? coolWindow : warmWindow;
                }
            }
            return buildingColor;
        }
    }

    return SKY_COLOR;
}

// Repaint a rectangle of pixels using GetBackgroundColor(). - used to prvent entire scren from flickering
static void RestoreBackground(int16_t x, int16_t y, int16_t w, int16_t h) {
    if (x >= SCREEN_W || y >= SCREEN_H || w <= 0 || h <= 0) return;
    int16_t xEnd = x + w; if (xEnd > SCREEN_W) xEnd = SCREEN_W;
    int16_t yEnd = y + h; if (yEnd > SCREEN_H) yEnd = SCREEN_H;
    if (x < 0) x = 0;
    if (y < HUD_HEIGHT) y = HUD_HEIGHT;
    for (int16_t px = x; px < xEnd; px++) {
        for (int16_t py = y; py < yEnd; py++) {
            if (py < HUD_HEIGHT) continue;
            GFX_DrawPixel(px, py, GetBackgroundColor(px, py));
        }
    }
}

// backgroudn city!!
static void Draw_CityBackground(void) {
    // Sky :)
    GFX_FillRect(0, HUD_HEIGHT, SCREEN_W, 40, 0x0212);
    GFX_FillRect(0, HUD_HEIGHT + 40, SCREEN_W, SCREEN_H - HUD_HEIGHT - 40, 0x020F);

    // Stars :)
    static const uint8_t starX[] = { 5, 18, 35, 48, 65, 88,105,118,135,150, 10, 42, 97,143, 25, 72,130};
    static const uint8_t starY[] = {16, 20, 15, 23, 18, 14, 19, 17, 21, 16, 27, 25, 28, 24, 22, 29, 26};
    for (int i = 0; i < 17; i++) {
        GFX_DrawPixel(starX[i], starY[i], (i % 3 == 0) ? 0xF79E : 0xAD55);
    }

    // Moon :)
    GFX_FillCircle(118, 28, 14, 0x1237);  // soft glow
    GFX_FillCircle(118, 28, 10, 0xB6DC);  // moon surface
    GFX_FillCircle(114, 25,  3, 0x8C9A);  // crater 1
    GFX_FillCircle(120, 31,  2, 0x8C9A);  // crater 2

    uint16_t buildingColor = rgb(6,   9,  32);
    uint16_t warmWindow    = rgb(200, 180,  80);
    uint16_t coolWindow    = rgb(180, 200, 220);

    // Left building silhouettes with sparse windows
    typedef struct { int16_t x, top, w; } Building;
    Building leftBuildings[] = {{0,26,15},{15,14,13},{28,30,11},{39,20,20},{59,34,10},{69,28,4}};
    for (int i = 0; i < 6; i++) {
        GFX_FillRect(leftBuildings[i].x, leftBuildings[i].top,
                     leftBuildings[i].w, SCREEN_H - leftBuildings[i].top, buildingColor);
        if (i % 2 == 0) {
            for (int wy = leftBuildings[i].top + 4; wy < 80; wy += 10) {
                for (int wx = leftBuildings[i].x + 2; wx < leftBuildings[i].x + leftBuildings[i].w - 1; wx += 6) {
                    if ((wx + wy) % 7 != 0) {
                        GFX_DrawPixel(wx, wy, ((wx + wy) % 3 == 0) ? coolWindow : warmWindow);
                    }
                }
            }
        }
    }

    // Right building silhouettes
    Building rightBuildings[] = {{88,22,14},{102,11,16},{118,26,13},{131,16,17},{148,30,12}};
    for (int i = 0; i < 5; i++) {
        GFX_FillRect(rightBuildings[i].x, rightBuildings[i].top,
                     rightBuildings[i].w, SCREEN_H - rightBuildings[i].top, buildingColor);
        if (i % 2 == 0) {
            for (int wy = rightBuildings[i].top + 4; wy < 80; wy += 10) {
                for (int wx = rightBuildings[i].x + 2; wx < rightBuildings[i].x + rightBuildings[i].w - 1; wx += 6) {
                    if ((wx + wy) % 7 != 0) {
                        GFX_DrawPixel(wx, wy, ((wx + wy) % 3 == 0) ? coolWindow : warmWindow);
                    }
                }
            }
        }
    }

    // Tree silhouette far left - i tried lol
    GFX_FillCircle(4, 82, 5, rgb(12, 45, 30));
    GFX_FillRect(3, 87, 3, 6, rgb(20, 28, 15));

    // Terrain
    for (int16_t x = 0; x < SCREEN_W; x++) {
        uint8_t ty = TerrainAt(x);
        GFX_DrawLine(x, ty,     x, ty + 2, GRASS_COLOR);
        GFX_DrawLine(x, ty + 3, x, SCREEN_H - 1, EARTH_COLOR);
    }
}

// tank pos - always snap to terrian (grass)
static int16_t GetTankGroundY(int16_t x) {
    int16_t lowestTerrain = SCREEN_H;
    for (int16_t tx = x; tx < x + TANK_W && tx < SCREEN_W; tx++) {
        uint8_t ty = TerrainAt(tx);
        if ((int16_t)ty < lowestTerrain) lowestTerrain = ty;
    }
    return lowestTerrain - TANK_H;
}

// tank redraawing the sprite
static void DrawSprite(int16_t left, int16_t top, const uint16_t* bitmap, int16_t w, int16_t h) {
    for (int16_t row = 0; row < h; row++) {
        int16_t sy = top + row;
        if (sy < HUD_HEIGHT || sy >= SCREEN_H) continue;
        for (int16_t col = 0; col < w; col++) {
            int16_t sx = left + col;
            if (sx < 0 || sx >= SCREEN_W) continue;
            uint16_t color = bitmap[(h - 1 - row) * w + col]; // bottom-row-first storage
            if (color == 0x0000) continue;                     // skip transparent
            GFX_DrawPixel(sx, sy, color);
        }
    }
}

// HUD elements redraw
static void DrawSpriteUnclipped(int16_t left, int16_t top, const uint16_t* bitmap, int16_t w, int16_t h) {
    for (int16_t row = 0; row < h; row++) {
        int16_t sy = top + row;
        if (sy < 0 || sy >= SCREEN_H) continue;
        for (int16_t col = 0; col < w; col++) {
            int16_t sx = left + col;
            if (sx < 0 || sx >= SCREEN_W) continue;
            uint16_t color = bitmap[(h - 1 - row) * w + col];
            if (color == 0x0000) continue;
            GFX_DrawPixel(sx, sy, color);
        }
    }
}

static void DrawTank(int16_t x, int16_t y, bool right, uint8_t type) {
    // Blink during invincibility
    if (invincibleTimer > 0 && (invincibleTimer % 4) >= 2) return;
    // Blink during speed boost ability
    if (speedBoostTimer > 0 && (speedBoostTimer % 4) >= 2) return;
    DrawSprite(x, y, TankSprites[type % 3][right ? 1 : 0], TANK_W, TANK_H);
}

static void EraseTank(int16_t x, int16_t y) {
    RestoreBackground(x, y, TANK_W, TANK_H);
}

// used for splash screen visual
static void DrawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    GFX_DrawLine(x + 3, y,         x + w - 4, y,         color);
    GFX_DrawLine(x + 3, y + h - 1, x + w - 4, y + h - 1, color);
    GFX_DrawLine(x,         y + 3, x,         y + h - 4, color);
    GFX_DrawLine(x + w - 1, y + 3, x + w - 1, y + h - 4, color);
    // Corner pixels
    GFX_DrawPixel(x + 1, y + 1, color); GFX_DrawPixel(x + 2, y + 1, color);
    GFX_DrawPixel(x + 1, y + 2, color);
    GFX_DrawPixel(x + w - 2, y + 1, color); GFX_DrawPixel(x + w - 3, y + 1, color);
    GFX_DrawPixel(x + w - 2, y + 2, color);
    GFX_DrawPixel(x + 1, y + h - 2, color); GFX_DrawPixel(x + 2, y + h - 2, color);
    GFX_DrawPixel(x + 1, y + h - 3, color);
    GFX_DrawPixel(x + w - 2, y + h - 2, color); GFX_DrawPixel(x + w - 3, y + h - 2, color);
    GFX_DrawPixel(x + w - 2, y + h - 3, color);
}

static void FillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    GFX_FillRect(x + 3, y,         w - 6, h,     color);
    GFX_FillRect(x,         y + 3, 3,     h - 6, color);
    GFX_FillRect(x + w - 3, y + 3, 3,     h - 6, color);
    GFX_FillRect(x + 1, y + 1,         2, 2, color);
    GFX_FillRect(x + w - 3, y + 1,     2, 2, color);
    GFX_FillRect(x + 1, y + h - 3,     2, 2, color);
    GFX_FillRect(x + w - 3, y + h - 3, 2, 2, color);
}

// missle spawn pos
static void GetBarrelTip(int16_t* outX, int16_t* outY) {
    *outX = tankX + (facingRight ? BARREL_TIP_X_RIGHT : BARREL_TIP_X_LEFT);
    *outY = tankY + BARREL_TIP_Y;
}

// aim line remove + redraw
static void EraseAimLine(void) {
    if (!aimLineDrawn) return;
    int16_t minX = (prevBarrelX < prevAimEndX ? prevBarrelX : prevAimEndX) - 2;
    int16_t minY = (prevBarrelY < prevAimEndY ? prevBarrelY : prevAimEndY) - 2;
    int16_t maxX = (prevBarrelX > prevAimEndX ? prevBarrelX : prevAimEndX) + 2;
    int16_t maxY = (prevBarrelY > prevAimEndY ? prevBarrelY : prevAimEndY) + 2;
    if (minX < 0)       minX = 0;
    if (minY < HUD_HEIGHT) minY = HUD_HEIGHT;
    if (maxX >= SCREEN_W) maxX = SCREEN_W - 1;
    if (maxY >= SCREEN_H) maxY = SCREEN_H - 1;
    RestoreBackground(minX, minY, maxX - minX + 1, maxY - minY + 1);
    aimLineDrawn = false;
}

// drawing aim line!
static void DrawAimLine(void) {
    int16_t bx, by;
    GetBarrelTip(&bx, &by);

    uint8_t angle = (aimAngle > 89) ? 89 : aimAngle;
    int16_t dx = facingRight ?  (AIM_LINE_LEN * (int16_t)cos32[angle]) >> 5
                             : -(AIM_LINE_LEN * (int16_t)cos32[angle]) >> 5;
    int16_t dy = -(AIM_LINE_LEN * (int16_t)sin32[angle]) >> 5;

    int16_t endX = bx + dx;
    int16_t endY = by + dy;
    if (endX < 0) endX = 0;
    if (endX >= SCREEN_W)  endX = SCREEN_W - 1;
    if (endY < HUD_HEIGHT) endY = HUD_HEIGHT;
    if (endY >= SCREEN_H)  endY = SCREEN_H - 1;

    // Draw dots every 5 pixels along the line
    for (int16_t step = 0; step <= AIM_LINE_LEN; step += 5) {
        int16_t px = bx + (dx * step) / AIM_LINE_LEN;
        int16_t py = by + (dy * step) / AIM_LINE_LEN;
        if (py >= HUD_HEIGHT && py < SCREEN_H && px >= 0 && px < SCREEN_W) {
            GFX_DrawPixel(px, py, 0xFFFF);
        }
    }

    prevBarrelX  = bx;  prevBarrelY  = by;
    prevAimEndX  = endX; prevAimEndY = endY;
    aimLineDrawn = true;
}

// charge bar redraw
static void EraseChargeBar(void) {
    if (chargeBarX < 0) return;
    RestoreBackground(chargeBarX - 1, chargeBarY - 1, 24, 9);
    chargeBarX = -1;
    chargeBarY = -1;
}

static void DrawChargeBar(void) {
    int16_t bx = tankX + 2;
    int16_t by = tankY - 9;
    if (by < HUD_HEIGHT) return;

    chargeBarX = bx;
    chargeBarY = by;

    int16_t fillW = (chargeLevel * 20) / CHARGE_MAX;
    if (fillW > 20) fillW = 20;

    GFX_FillRect(bx, by, 20, 5, 0x39E7);  // gray background

    uint16_t fillColor;
    if      (chargeLevel < CHARGE_MAX / 3)     fillColor = 0x07E0;  // green
    else if (chargeLevel < 2 * CHARGE_MAX / 3) fillColor = 0xFFE0;  // yellow
    else                                        fillColor = 0xF800;  // red

    if (fillW > 0) GFX_FillRect(bx, by, fillW, 5, fillColor);
    GFX_DrawRect(bx - 1, by - 1, 22, 7, 0xC618);  // border
}

// HUD — hearts and timer only
static void DrawTimerString(void) {
    uint8_t minutes = matchTimerSec / 60;
    uint8_t seconds = matchTimerSec % 60;
    char buf[5] = {'0' + minutes, ':', '0' + seconds / 10, '0' + seconds % 10, 0};

    GFX_FillRect(64, 3, 28, 8, 0x0000);

    // Flash when <= 10 seconds
    if (matchTimerSec <= 10 && timerBlinkTick % 8 < 4) return;

    uint16_t color = (matchTimerSec > 30) ? 0xFFFF :
                     (matchTimerSec > 10) ? 0xFFE0 : 0xF800;
    GFX_DrawString(64, 3, buf, color, 0x0000, 1);
}

static void DrawLives(void) {
    // My hearts — top left
    GFX_FillRect(0, 0, 56, HUD_HEIGHT, 0x0000);
    for (uint8_t i = 0; i < MAX_LIVES; i++) {
        DrawSpriteUnclipped(2 + i * 18, 3,
                            (i < myLives) ? HeartFull : HeartEmpty, 8, 8);
    }
    // Opponent hearts — top right
    GFX_FillRect(104, 0, 56, HUD_HEIGHT, 0x0000);
    for (uint8_t i = 0; i < MAX_LIVES; i++) {
        DrawSpriteUnclipped(106 + i * 18, 3,
                            (i < oppLives) ? HeartFull : HeartEmpty, 8, 8);
    }
}

static void DrawHUD(void) {
    GFX_FillRect(0, 0, SCREEN_W, HUD_HEIGHT, 0x0000);
    DrawLives();
    DrawTimerString();
    GFX_DrawLine(0, HUD_HEIGHT - 1, SCREEN_W - 1, HUD_HEIGHT - 1, 0x39E7);
}

// Battle Ready GPIO Bit sending
static void BattleReady_Init(void) {
#if PLAYER_ID == 1
    IOMUX->SECCFG.PINCM[PA17INDEX] = 0x00000081;  // PA17 output
    GPIOA->DOE31_0  |=  (1U << 17);
    GPIOA->DOUTCLR31_0 = (1U << 17);              // start low
    IOMUX->SECCFG.PINCM[PA31INDEX] = 0x00050081;  // PA31 input + pull-down
    GPIOA->DOE31_0  &= ~(1U << 31);
#else
    IOMUX->SECCFG.PINCM[PA17INDEX] = 0x00050081;  // PA17 input + pull-down
    GPIOA->DOE31_0  &= ~(1U << 17);
    IOMUX->SECCFG.PINCM[PA31INDEX] = 0x00000081;  // PA31 output
    GPIOA->DOE31_0  |=  (1U << 31);
    GPIOA->DOUTCLR31_0 = (1U << 31);              // start low
#endif
}

static void BattleReady_AssertReady(void) {
#if PLAYER_ID == 1
    GPIOA->DOUTSET31_0 = (1U << 17);
#else
    GPIOA->DOUTSET31_0 = (1U << 31);
#endif
}

static bool BattleReady_OpponentIsReady(void) {
#if PLAYER_ID == 1
    return (GPIOA->DIN31_0 >> 31) & 1;
#else
    return (GPIOA->DIN31_0 >> 17) & 1;
#endif
}

// Splash screen
static void Draw_Splash(void) {
    GFX_FillScreen(0x0000);
    GFX_DrawStringCentered( 5, "DUEL",  0xFFE0, 0x0000, 3);
    GFX_DrawStringCentered(32, "TANKS", 0xF800, 0x0000, 3);
    GFX_DrawLine(10, 58, SCREEN_W - 10, 58, 0x39E7);
    GFX_DrawStringCentered(62, "ECE319H  SP2026", 0x39E7, 0x0000, 1);

    // Simple ground and two demo tanks
    uint8_t groundY = 88;
    GFX_DrawLine(0, groundY, SCREEN_W - 1, groundY, GRASS_COLOR);
    GFX_FillRect(0, groundY + 1, SCREEN_W, SCREEN_H - groundY - 1, EARTH_COLOR);
    DrawSpriteUnclipped(10,           groundY - TANK_H, TankSprites[0][1], TANK_W, TANK_H);
    DrawSpriteUnclipped(SCREEN_W - 34, groundY - TANK_H, TankSprites[2][0], TANK_W, TANK_H);

    splashMissileActive[0] = splashMissileActive[1] = false;
    splashMissileTimer = 0;
}

static void BlinkSplashPrompt(bool visible) {
    if (visible) GFX_DrawStringCentered(100, "PRESS ANY BUTTON", 0x07FF, 0x0000, 1);
    else         GFX_FillRect(0, 98, SCREEN_W, 10, 0x0000);
}

// for rectangle redrawing smaller regions
static void RestoreSplashBackground(int16_t x, int16_t y, int16_t w, int16_t h) {
    uint8_t groundY = 88;
    for (int16_t py = y; py < y + h; py++) {
        for (int16_t px = x; px < x + w; px++) {
            if (px < 0 || px >= SCREEN_W || py < 0 || py >= SCREEN_H) continue;
            uint16_t color = (py < groundY)  ? 0x0000      :
                             (py == groundY) ? GRASS_COLOR : EARTH_COLOR;
            GFX_DrawPixel(px, py, color);
        }
    }
}

static void UpdateSplashAnimation(void) {
    uint8_t groundY = 88;
    splashMissileTimer++;

    // Left tank fires at t=1 and t=121
    if (splashMissileTimer == 1 || splashMissileTimer == 121) {
        splashMissileXfp[0] = (10 + BARREL_TIP_X_RIGHT) * FP_SCALE;
        splashMissileYfp[0] = (groundY - TANK_H + BARREL_TIP_Y) * FP_SCALE;
        splashMissileVx[0]  = 36;
        splashMissileVy[0]  = -22;
        splashMissilePrevX[0] = splashMissileXfp[0] >> 3;
        splashMissilePrevY[0] = splashMissileYfp[0] >> 3;
        splashMissileActive[0] = true;
    }
    // Right tank fires at t=60 and t=180
    if (splashMissileTimer == 60 || splashMissileTimer == 180) {
        splashMissileXfp[1] = (SCREEN_W - 34 + BARREL_TIP_X_LEFT) * FP_SCALE;
        splashMissileYfp[1] = (groundY - TANK_H + BARREL_TIP_Y) * FP_SCALE;
        splashMissileVx[1]  = -36;
        splashMissileVy[1]  = -22;
        splashMissilePrevX[1] = splashMissileXfp[1] >> 3;
        splashMissilePrevY[1] = splashMissileYfp[1] >> 3;
        splashMissileActive[1] = true;
    }
    if (splashMissileTimer >= 200) splashMissileTimer = 0;

    for (int i = 0; i < 2; i++) {
        if (!splashMissileActive[i]) continue;

        // Erase previous position
        RestoreSplashBackground(splashMissilePrevX[i] - 1, splashMissilePrevY[i] - 1,
                                MISSILE_W + 2, MISSILE_H + 2);

        // Redraw tanks and title if the erase may have clipped them
        DrawSpriteUnclipped(10,            groundY - TANK_H, TankSprites[0][1], TANK_W, TANK_H);
        DrawSpriteUnclipped(SCREEN_W - 34, groundY - TANK_H, TankSprites[2][0], TANK_W, TANK_H);
        if (splashMissilePrevY[i] < 75) {
            GFX_DrawStringCentered( 5, "DUEL",            0xFFE0, 0x0000, 3);
            GFX_DrawStringCentered(32, "TANKS",           0xF800, 0x0000, 3);
            GFX_DrawLine(10, 58, SCREEN_W - 10, 58, 0x39E7);
            GFX_DrawStringCentered(62, "ECE319H  SP2026", 0x39E7, 0x0000, 1);
        }

        // Move missile
        splashMissileXfp[i] += splashMissileVx[i];
        splashMissileYfp[i] += splashMissileVy[i];
        splashMissileVy[i]  += 2;  // gravity

        int16_t nx = splashMissileXfp[i] >> 3;
        int16_t ny = splashMissileYfp[i] >> 3;

        if (ny > groundY || nx < -10 || nx > SCREEN_W + 5) {
            splashMissileActive[i] = false;
            continue;
        }

        // Draw missile at new position
        const uint16_t* sprite = (splashMissileVx[i] > 0) ? MissileRight : MissileLeft;
        for (int16_t r = 0; r < MISSILE_H; r++) {
            int16_t sy = ny + r;
            if (sy < 0 || sy >= SCREEN_H) continue;
            for (int16_t c = 0; c < MISSILE_W; c++) {
                int16_t sx = nx + c;
                if (sx < 0 || sx >= SCREEN_W) continue;
                uint16_t color = sprite[(MISSILE_H - 1 - r) * MISSILE_W + c];
                if (color) GFX_DrawPixel(sx, sy, color);
            }
        }

        splashMissilePrevX[i] = nx;
        splashMissilePrevY[i] = ny;
    }
}

// language select splash screen
static void Draw_Language(void) {
    GFX_FillScreen(0x0000);
    GFX_DrawStringCentered(6, "SELECT LANGUAGE", 0xFFFF, 0x0000, 1);
    GFX_DrawLine(0, 16, SCREEN_W, 16, 0x39E7);

    uint16_t c0 = (lang == 0) ? 0x0000 : 0xFFFF;
    uint16_t b0 = (lang == 0) ? 0xFFE0 : 0x2945;
    uint16_t c1 = (lang == 1) ? 0x0000 : 0xFFFF;
    uint16_t b1 = (lang == 1) ? 0xFFE0 : 0x2945;

    FillRoundRect( 6, 28, 66, 22, b0); DrawRoundRect( 6, 28, 66, 22, 0xFFFF);
    GFX_DrawString(18, 35, "ENGLISH", c0, b0, 1);

    FillRoundRect(88, 28, 66, 22, b1); DrawRoundRect(88, 28, 66, 22, 0xFFFF);
    GFX_DrawString(100, 35, "ESPANOL", c1, b1, 1);

    GFX_DrawStringCentered(62, GS("< LEFT / RIGHT >", "< IZQ / DER >"), 0x39E7, 0x0000, 1);
    GFX_DrawStringCentered(72, GS("FIRE = CONFIRM",   "DISPARO = OK"),   0x39E7, 0x0000, 1);
    GFX_DrawStringCentered(88, (lang == 0) ? "[ ENGLISH ]" : "[ ESPANOL ]", 0x07FF, 0x0000, 1);
}

// ================================================================
// Rules screens (3 pages)
// ================================================================
static void Draw_Rules(void) {
    GFX_FillScreen(0x0000);

    const char* titles[3] = {
        GS("HOW TO PLAY", "COMO JUGAR"),
        GS("CONTROLS",    "CONTROLES"),
        GS("ON SCREEN",   "EN PANTALLA")
    };
    GFX_DrawStringCentered(3, titles[rulesPage], 0xFFE0, 0x0000, 1);
    GFX_DrawLine(0, 12, SCREEN_W, 12, 0x39E7);

    if (rulesPage == 0) {
        GFX_DrawString(4, 16, GS("Shoot enemy across",    "Dispara al enemigo"),   0xFFFF, 0, 1);
        GFX_DrawString(4, 26, GS("both screens!",         "por ambas pantallas!"), 0xFFFF, 0, 1);
        GFX_DrawString(4, 38, GS("Shells follow a",       "Balas en trayectoria"), 0x07FF, 0, 1);
        GFX_DrawString(4, 48, GS("parabolic arc.",        "parabolica."),           0x07FF, 0, 1);
        GFX_DrawString(4, 60, GS("2 min timer!",          "Limite 2 minutos!"),    0xFFE0, 0, 1);
        GFX_DrawString(4, 70, GS("Most lives = WIN",      "Mas vidas = ganar"),    0xFFE0, 0, 1);
        GFX_DrawString(4, 82, GS("3 kills = instant win", "3 bajas = ganar ya"),   0xF800, 0, 1);
    } else if (rulesPage == 1) {
        GFX_DrawString(4, 16, GS("LFT/RGT: move tank",   "IZQ/DER: mover"),     0x07FF, 0, 1);
        GFX_DrawString(4, 28, GS("SLIDE POT: aim angle",  "DESLIZ: angulo"),     0x07FF, 0, 1);
        GFX_DrawString(4, 40, GS("HOLD FIRE: charge",     "MANT FIRE: cargar"),  0x07FF, 0, 1);
        GFX_DrawString(4, 52, GS("RELEASE: SHOOT!",       "SOLTAR: DISPARAR!"),  0xFFFF, 0, 1);
        GFX_DrawString(4, 64, GS("SHIELD: ability",       "ESCUDO: habilidad"),  0xF81F, 0, 1);
        GFX_DrawString(4, 76, GS("FIRE+LFT+RGT: quit",   "FIRE+IZQ+DER: salir"),0xC618, 0, 1);
    } else {
        // Visual symbols page
        GFX_DrawString(4, 14, GS("YOUR LIVES:", "TUS VIDAS:"), 0x39E7, 0, 1);
        for (int i = 0; i < 3; i++) {
            DrawSpriteUnclipped(82 + i * 18, 14, (i < 2) ? HeartFull : HeartEmpty, 8, 8);
        }
        GFX_DrawString(4, 28, GS("AIM (dots):", "PUNTERIA:"), 0x39E7, 0, 1);
        for (int i = 0; i < 40; i += 5) GFX_DrawPixel(82 + i, 32, 0xFFFF);

        GFX_DrawString(4, 42, GS("CHARGE BAR:", "CARGA:"), 0x39E7, 0, 1);
        GFX_FillRect(82, 43, 25, 5, 0x07E0);
        GFX_DrawRect(81, 42, 27, 7, 0xC618);

        GFX_DrawString(4, 56, GS("TIMER:", "TIEMPO:"), 0x39E7, 0, 1);
        GFX_DrawString(82, 56, "1:45", 0xFFFF, 0x0000, 1);

        GFX_DrawString(4, 70, GS("INCOMING:", "CUIDADO:"), 0x39E7, 0, 1);
        GFX_FillRect(76, 69, 50, 9, 0xF800);
        GFX_DrawString(78, 71, GS("INCOMING!", "CUIDADO!"), 0xFFFF, 0xF800, 1);

        GFX_DrawString(4, 84, GS("EXPLOSION:", "EXPLOSION:"), 0x39E7, 0, 1);
        DrawSpriteUnclipped(82, 84, ExpF1, 14, 14);
    }

    // Page indicator dots
    for (int i = 0; i < 3; i++) {
        GFX_FillCircle(SCREEN_W / 2 - 8 + i * 8, SCREEN_H - 7, 2,
                       (i == rulesPage) ? 0xFFFF : 0x39E7);
    }

    GFX_DrawString(2,            SCREEN_H - 12, GS("< LEFT", "< IZQ"), 0x39E7, 0, 1);
    GFX_DrawString(SCREEN_W - 44, SCREEN_H - 12, GS("RIGHT >", "DER >"), 0x39E7, 0, 1);
    if (rulesPage == 2) {
        GFX_DrawStringCentered(SCREEN_H - 12, GS("FIRE=START", "DISP=INICIO"), 0x07FF, 0, 1);
    }
}

// ================================================================
// Tank selection screen — single centered card
// ================================================================
static const char* tankNames[3][2]  = {{"BRAWLER","PELEADOR"}, {"PHANTOM","FANTASMA"}, {"ARTY","ARTILL."}};
static const char* tankDescs[3][2]  = {{"Balanced fighter","Combatiente eq."}, {"Speed+Ghost","Velocidad+Sigilo"}, {"Max firepower","Max poder"}};
static const char* tankAbils[3][2]  = {{"Double Shot","Doble Tiro"}, {"Speed Boost","Impulso"}, {"Ricochet","Rebote"}};
static const uint8_t  tankSpeedStat[3] = {55, 90, 35};
static const uint8_t  tankArmorStat[3] = {80, 35, 55};
static const uint8_t  tankPowerStat[3] = {65, 55, 95};
static const uint16_t tankAccentColor[3] = {0x041F, 0x03E0, 0xF800};

static void DrawStatBar(int16_t x, int16_t y, uint8_t percent, uint16_t color) {
    GFX_FillRect(x, y, 80, 5, 0x1082);
    int16_t fillW = (percent * 80) / 100;
    if (fillW > 0) GFX_FillRect(x, y, fillW, 5, color);
    GFX_DrawRect(x - 1, y - 1, 82, 7, 0x39E7);
}

static void Draw_TankSelect(void) {
    GFX_FillScreen(0x0000);
    GFX_DrawStringCentered(2, GS("SELECT TANK", "ELIGE TANQUE"), 0xFFE0, 0x0000, 1);
    GFX_DrawLine(0, 11, SCREEN_W, 11, 0x39E7);

    uint8_t  t      = tankType;
    uint16_t accent = tankAccentColor[t];

    FillRoundRect(2, 13, SCREEN_W - 4, 100, 0x0820);
    DrawRoundRect(2, 13, SCREEN_W - 4, 100, accent);

    DrawSpriteUnclipped(10, 22, TankSprites[t][facingRight ? 1 : 0], TANK_W, TANK_H);

    GFX_DrawString(42, 16, tankNames[t][lang], accent,  0x0820, 2);
    GFX_DrawString(42, 32, tankDescs[t][lang], 0x39E7,  0x0820, 1);

    GFX_DrawString(42, 43, GS("SPD","VEL"), 0x07FF, 0x0820, 1);
    DrawStatBar(64, 44, tankSpeedStat[t], 0x07E0);

    GFX_DrawString(42, 54, GS("ARM","ARM"), 0x07FF, 0x0820, 1);
    DrawStatBar(64, 55, tankArmorStat[t], 0x07FF);

    GFX_DrawString(42, 65, GS("PWR","FUE"), 0x07FF, 0x0820, 1);
    DrawStatBar(64, 66, tankPowerStat[t], 0xF800);

    GFX_DrawString( 8, 80, "ABILITY:", 0xC618, 0x0820, 1);
    GFX_DrawString(56, 80, tankAbils[t][lang], 0xFFFF, 0x0820, 1);

    if (tankType > 0) GFX_DrawString(  4, 115, "<", 0x07FF, 0x0000, 2);
    if (tankType < 2) GFX_DrawString(148, 115, ">", 0x07FF, 0x0000, 2);
    GFX_DrawStringCentered(116, GS("FIRE = SELECT", "DISP = ELEGIR"), 0xFFE0, 0x0000, 1);
}

// ================================================================
// Waiting / countdown screens
// ================================================================
static void Draw_Waiting(void) {
    GFX_FillScreen(0x0000);
    GFX_DrawStringCentered(24, GS("DUEL",      "DUELO"),       0xF800, 0x0000, 3);
    GFX_DrawStringCentered(66, GS("WAITING...", "ESPERANDO..."),0xC618, 0x0000, 1);
}

static void Draw_Countdown(uint8_t number) {
    GFX_FillScreen(0x0000);
    if (number > 0) {
        char buf[2] = {(char)('0' + number), 0};
        uint16_t color = (number == 3) ? 0xF800 : (number == 2) ? 0xFFE0 : 0x07E0;
        GFX_DrawStringCentered(26, buf,                             color,  0x0000, 5);
        GFX_DrawStringCentered(88, GS("GET READY!", "PREPARATE!"), 0xC618, 0x0000, 1);
    } else {
        GFX_DrawStringCentered(30, GS("FIGHT!", "LUCHA!"), 0xFFE0, 0x0000, 3);
    }
}

// ================================================================
// Missile spawning
// ================================================================
static void FireMissile(uint8_t angle, uint8_t power, bool ricochet);  // forward declare

static void SpawnMissile(uint8_t angle, uint8_t power, bool isRicochet) {
    int8_t slot = FindFreeMissile();
    if (slot < 0) return;

    int16_t bx, by;
    GetBarrelTip(&bx, &by);

    missiles[slot].xfp      = bx * FP_SCALE;
    missiles[slot].yfp      = by * FP_SCALE;
    missiles[slot].prevX    = bx;
    missiles[slot].prevY    = by;
    missiles[slot].incoming = false;
    missiles[slot].active   = true;
    missiles[slot].ricochet = isRicochet;

    if (angle > 89) angle = 89;
    int16_t cosVal = (int16_t)cos32[angle];
    int16_t sinVal = (int16_t)sin32[angle];
    int16_t vx = (power * cosVal) >> 2;
    int16_t vy = -(power * sinVal) >> 2;
    if (vx < 1) vx = 1;

    missiles[slot].vxfp = facingRight ? vx : -vx;
    missiles[slot].vyfp = vy;

    SND(Sound_Shoot());
}

static void SpawnIncomingMissile(uint8_t yPixel, uint8_t vxEncoded, uint8_t vyEncoded) {
    int8_t slot = FindFreeMissile();
    if (slot < 0) return;

    int16_t vx = (int16_t)(vxEncoded & 0x0F) * 3;
    int16_t vy = ((int16_t)(vyEncoded & 0x0F) - 8) * 6;

    // Guarantee enough speed to cross the full screen
    if (vx < 12) vx = 12;

    int16_t yp = yPixel;
    if (yp < HUD_HEIGHT)        yp = HUD_HEIGHT;
    if (yp > SCREEN_H - MISSILE_H) yp = SCREEN_H - MISSILE_H;

    missiles[slot].yfp      = yp * FP_SCALE;
    missiles[slot].vyfp     = vy;
    missiles[slot].incoming = true;
    missiles[slot].active   = true;
    missiles[slot].ricochet = false;

#if PLAYER_ID == 1
    missiles[slot].xfp  = (SCREEN_W - MISSILE_W) * FP_SCALE;
    missiles[slot].vxfp = -vx;
#else
    missiles[slot].xfp  = 0;
    missiles[slot].vxfp = vx;
#endif
    missiles[slot].prevX = missiles[slot].xfp >> 3;
    missiles[slot].prevY = missiles[slot].yfp >> 3;
}

// ================================================================
// Explosions
// ================================================================
static void SpawnExplosion(int16_t cx, int16_t cy) {
    int8_t slot = FindFreeExplosion();
    if (slot < 0) return;
    explosions[slot] = {cx, cy, 0, (uint8_t)(4 * EXP_TICKS_PER_FRAME), true};
    DrawSprite(cx - 7, cy - 7, ExpFrames[0], 14, 14);
}

static void UpdateExplosions(void) {
    for (int i = 0; i < MAX_EXPLOSIONS; i++) {
        if (!explosions[i].active) continue;

        if (--explosions[i].ticksLeft == 0) {
            RestoreBackground(explosions[i].cx - 7, explosions[i].cy - 7, 14, 14);
            explosions[i].active = false;
            continue;
        }

        uint8_t newFrame = (4 * EXP_TICKS_PER_FRAME - explosions[i].ticksLeft) / EXP_TICKS_PER_FRAME;
        if (newFrame >= 4) newFrame = 3;

        if (newFrame != explosions[i].frame) {
            explosions[i].frame = newFrame;
            RestoreBackground(explosions[i].cx - 7, explosions[i].cy - 7, 14, 14);
            DrawSprite(explosions[i].cx - 7, explosions[i].cy - 7, ExpFrames[newFrame], 14, 14);
        }
    }
}

// ================================================================
// Taking a hit
// ================================================================
static void TakeHit(int16_t ex, int16_t ey) {
    if (invincibleTimer > 0) return;

    myLives--;
    invincibleTimer = INVINCIBLE_TICKS;
    flashTimer      = FLASH_TICKS;

    SpawnExplosion(ex, ey);
    SND(Sound_Explosion());
    GameUART_SendHit(myLives);
    DrawLives();
}

// ================================================================
// Abilities
// ================================================================
static void FireMissile(uint8_t angle, uint8_t power, bool ricochet) {
    SpawnMissile(angle, power, ricochet);
}

static void UseAbility(void) {
    if (abilityCooldown > 0) return;
    abilityCooldown = ABILITY_COOLDOWN;

    switch (tankType) {
        case 0: {
            // Brawler — Double Shot: two missiles spread ±5 degrees
            uint8_t a1 = (aimAngle > 5)  ? aimAngle - 5 : 0;
            uint8_t a2 = (aimAngle < 84) ? aimAngle + 5 : 89;
            uint8_t pwr = POWER_MIN + (chargeLevel * (POWER_MAX - POWER_MIN)) / CHARGE_MAX;
            if (pwr < POWER_MIN) pwr = POWER_MIN;
            FireMissile(a1, pwr, false);
            FireMissile(a2, pwr, false);
            break;
        }
        case 1:
            // Phantom — Speed Boost
            speedBoostTimer = SPEED_BOOST_TICKS;
            break;

        case 2:
            // Artillery — Ricochet: next shot bounces off terrain
            ricochetNextShot = true;
            FireMissile(aimAngle, POWER_MAX, true);
            break;
    }
    SND(Sound_Barrier());
}

// ================================================================
// Missile physics update
// ================================================================
static void UpdateMissiles(void) {
    for (int i = 0; i < MAX_MISSILES; i++) {
        if (!missiles[i].active) continue;

        // Erase previous position
        int16_t ox = missiles[i].prevX;
        int16_t oy = missiles[i].prevY;
        if (oy >= HUD_HEIGHT) {
            RestoreBackground(ox, oy, MISSILE_W + 2, MISSILE_H + 2);
        }

        // Apply velocity and gravity
        missiles[i].xfp  += missiles[i].vxfp;
        missiles[i].yfp  += missiles[i].vyfp;
        missiles[i].vyfp += GRAVITY;

        int16_t px = missiles[i].xfp >> 3;
        int16_t py = missiles[i].yfp >> 3;
        int16_t clampedX = (px < 0) ? 0 : (px >= SCREEN_W) ? SCREEN_W - 1 : px;
        int16_t terrainHere = (int16_t)TerrainAt(clampedX);

        // Terrain collision
        if (py + MISSILE_H >= terrainHere) {
            if (missiles[i].ricochet) {
                // Bounce — reverse vertical velocity with damping
                missiles[i].vyfp  = -(missiles[i].vyfp * 3) / 4;
                missiles[i].yfp   = (terrainHere - MISSILE_H - 1) * FP_SCALE;
                missiles[i].ricochet = false;
            } else {
                SpawnExplosion(px + MISSILE_W / 2, terrainHere - 3);
                SND(Sound_Click());
                missiles[i].active = false;
                continue;
            }
        }

        bool exitedLeft  = (px + MISSILE_W < 0);
        bool exitedRight = (px >= SCREEN_W);

        // Our own missile exits screen — send to other board via UART
        if (!missiles[i].incoming && (exitedLeft || exitedRight)) {
            int16_t yp  = (py < 0) ? 0 : (py > 127) ? 127 : py;
            int16_t vxa = missiles[i].vxfp < 0 ? -missiles[i].vxfp : missiles[i].vxfp;
            uint8_t vxEnc = (uint8_t)((vxa / 3) & 0x0F);
            int16_t vyNorm = missiles[i].vyfp / 6 + 8;
            if (vyNorm < 0) vyNorm = 0;
            if (vyNorm > 15) vyNorm = 15;
            GameUART_SendFire((uint8_t)yp, vxEnc, (uint8_t)vyNorm);
            missiles[i].active = false;
            continue;
        }

        // Incoming missile that missed — just deactivate
        if (missiles[i].incoming && (exitedLeft || exitedRight)) {
            missiles[i].active = false;
            continue;
        }

        // Incoming missile hits our tank
        if (missiles[i].incoming && py >= HUD_HEIGHT) {
            if (RectsOverlap(px, py, MISSILE_W, MISSILE_H,
                             tankX + HIT_OFFSET_X, tankY + HIT_OFFSET_Y, HIT_W, HIT_H)) {
                TakeHit(px + MISSILE_W / 2, py + MISSILE_H / 2);
                missiles[i].active = false;
                continue;
            }
        }

        // Skip drawing if missile is in HUD zone
        if (py < HUD_HEIGHT) {
            missiles[i].prevX = px;
            missiles[i].prevY = py;
            continue;
        }

        // Draw missile at new position
        const uint16_t* sprite = (missiles[i].vxfp > 0) ? MissileRight : MissileLeft;
        DrawSprite(px, py, sprite, MISSILE_W, MISSILE_H);
        missiles[i].prevX = px;
        missiles[i].prevY = py;
    }
}

// ================================================================
// UART packet handler
// ================================================================
static void HandleUART(void) {
    GamePacket_t pkt;
    while (GameUART_Receive(&pkt)) {
        switch (pkt.type) {

            case PKT_FIRE:
                SpawnIncomingMissile(pkt.data1, pkt.data2 >> 4, pkt.data2 & 0x0F);
                GFX_DrawIncoming();
                incomingWarnTimer = INCOMING_WARN_TICKS;
                SND(Sound_Incoming());
                break;

            case PKT_HIT:
                myKills++;
                oppLives = pkt.data1;
                DrawLives();
                break;

            case PKT_WIN:
                if (gameState == STATE_GAMEPLAY || gameState == STATE_COUNTDOWN
                                               || gameState == STATE_WAITING) {
                    playerWon = true;
                    SND(Sound_Stop());
                    Clock_Delay1ms(300);
                    SND(Sound_Victory());
                    char pb[10] = "PLAYER 1";
                    pb[7] = '0' + PLAYER_ID;
                    GFX_FillScreen(0x0000);
                    GFX_DrawStringCentered(10, GS("VICTORY!",     "VICTORIA!"),        0xFFE0, 0, 2);
                    GFX_DrawStringCentered(34, pb,                                      0x07FF, 0, 1);
                    GFX_DrawStringCentered(50, GS("ENEMY DOWN",   "ENEMIGO VENCIDO"),  0x07E0, 0, 1);
                    GFX_DrawLine(0, 66, SCREEN_W, 66, 0x39E7);
                    GFX_DrawStringCentered(76, GS("FIRE=REMATCH", "DISP=REVANCHA"),    0x07FF, 0, 1);
                    GFX_DrawStringCentered(88, GS("SHLD=QUIT",    "ESCU=SALIR"),       0xC618, 0, 1);
                    gameState = STATE_END;
                }
                break;

            case PKT_TANK: oppTankType = pkt.data1; break;
            case PKT_LANG: lang        = pkt.data1; break;
            default: break;
        }
    }
}

// ================================================================
// Combo exit — hold FIRE + LEFT + RIGHT for ~1.5 seconds
// ================================================================
static void CheckComboExit(void) {
    if (Switch_Held(SW_FIRE) && Switch_Held(SW_LEFT) && Switch_Held(SW_RIGHT)) {
        if (++comboHeldTicks >= 45) {
            comboHeldTicks = 0;
            SND(Sound_Stop());
            GFX_FillScreen(0x0000);
            GFX_DrawStringCentered(20, GS("GAME OVER",    "FIN"),          0xF800, 0, 2);
            GFX_DrawLine(0, 44, SCREEN_W, 44, 0x39E7);
            GFX_DrawStringCentered(54, GS("FIRE=REMATCH", "DISP=REVANCHA"),0x07FF, 0, 1);
            GFX_DrawStringCentered(66, GS("SHLD=QUIT",    "ESCU=SALIR"),   0xC618, 0, 1);
            for (int i = 0; i < MAX_MISSILES; i++) missiles[i].active = false;
            gameState = STATE_END;
        }
    } else {
        comboHeldTicks = 0;
    }
}

// ================================================================
// Timer expiry — compare lives and declare winner
// ================================================================
static void EndByTimer(void) {
    bool weWon = (myLives > oppLives);
    bool itsTie = (myLives == oppLives);

    SND(Sound_Stop());
    Clock_Delay1ms(300);

    GFX_FillScreen(0x0000);
    GFX_DrawStringCentered(6, GS("TIME'S UP!", "TIEMPO!"), 0xFFE0, 0x0000, 2);
    GFX_DrawLine(0, 26, SCREEN_W, 26, 0x39E7);

    const char* resultStr;
    uint16_t    resultColor;
    if (itsTie) {
        resultStr   = GS("TIE!",      "EMPATE!");
        resultColor = 0xFFFF;
        SND(Sound_Beep(1));
    } else if (weWon) {
        resultStr   = GS("YOU WIN!",  "GANASTE!");
        resultColor = 0x07E0;
        SND(Sound_Victory());
    } else {
        resultStr   = GS("YOU LOSE!", "PERDISTE!");
        resultColor = 0xF800;
        SND(Sound_Defeat());
    }

    GFX_DrawStringCentered(34, resultStr, resultColor, 0x0000, 2);

    char pb[10] = "PLAYER 1";
    pb[7] = '0' + PLAYER_ID;
    GFX_DrawStringCentered(56, pb, 0x39E7, 0x0000, 1);
    GFX_DrawString(4, 68, GS("YOUR LIVES:", "TUS VIDAS:"), 0xFFFF, 0, 1);
    for (uint8_t i = 0; i < MAX_LIVES; i++) {
        DrawSpriteUnclipped(80 + i * 18, 66, (i < myLives) ? HeartFull : HeartEmpty, 8, 8);
    }
    GFX_DrawLine(0, 82, SCREEN_W, 82, 0x39E7);
    GFX_DrawStringCentered(90,  GS("FIRE=REMATCH", "DISP=REVANCHA"), 0x07FF, 0, 1);
    GFX_DrawStringCentered(100, GS("SHLD=QUIT",    "ESCU=SALIR"),    0xC618, 0, 1);

    playerWon = weWon;
    gameState = STATE_END;

    if (!itsTie) {
        GameUART_SendWin(weWon ? (uint8_t)PLAYER_ID : (uint8_t)(3 - PLAYER_ID));
    }
}

// ================================================================
// State entry helpers
// ================================================================
static void EnterSplash(void) {
    gameState     = STATE_SPLASH;
    splashBlinkTimer = 0;
    Draw_Splash();
    BlinkSplashPrompt(true);
}

static void EnterGameplay(void) {
    tankX = TANK_START_X;
    tankY = GetTankGroundY(TANK_START_X);
    facingRight = (bool)FACING_RIGHT;

    myLives   = MAX_LIVES;
    oppLives  = MAX_LIVES;
    myKills   = 0;

    invincibleTimer   = 0;
    flashTimer        = 0;
    incomingWarnTimer = 0;
    chargeState       = CHARGE_IDLE;
    chargeLevel       = 0;
    aimAngle          = 35;
    aimLineDrawn      = false;
    prevChargeLevel   = 255;
    chargeBarX        = -1;
    chargeBarY        = -1;
    abilityCooldown   = 0;
    speedBoostTimer   = 0;
    ricochetNextShot  = false;
    comboHeldTicks    = 0;
    matchTimerSec     = 120;
    matchTimerFrames  = 0;
    timerBlinkTick    = 0;

    for (int i = 0; i < MAX_MISSILES;    i++) missiles[i].active    = false;
    for (int i = 0; i < MAX_EXPLOSIONS;  i++) explosions[i].active  = false;

    Draw_CityBackground();
    DrawHUD();
    DrawTank(tankX, tankY, facingRight, tankType);
    DrawAimLine();

    gameState = STATE_GAMEPLAY;
}

// ================================================================
// State handlers (called once per tick by Game_Update)
// ================================================================

static void State_Splash(uint32_t adc) {
    if (++splashBlinkTimer >= 20) {
        splashBlinkTimer = 0;
        showPrompt = !showPrompt;
        BlinkSplashPrompt(showPrompt);
    }
    UpdateSplashAnimation();

    bool anyButton = Switch_Pressed(SW_FIRE)   || Switch_Pressed(SW_SHIELD) ||
                     Switch_Pressed(SW_LEFT)   || Switch_Pressed(SW_RIGHT);
    if (anyButton) {
        SND(Sound_Click());
        lang      = 0;
        gameState = STATE_LANGUAGE;
        Draw_Language();
    }
}

static void State_Language(uint32_t adc) {
    bool changed = false;
    if (Switch_Pressed(SW_LEFT)  && lang == 1) { lang = 0; changed = true; }
    if (Switch_Pressed(SW_RIGHT) && lang == 0) { lang = 1; changed = true; }
    if (changed) {
        SND(Sound_Click());
        Draw_Language();
        GameUART_SendLang(lang);
    }
    if (Switch_Pressed(SW_FIRE)) {
        SND(Sound_Click());
        rulesPage = 0;
        gameState = STATE_RULES;
        Draw_Rules();
    }
}

static void State_Rules(uint32_t adc) {
    if (Switch_Pressed(SW_RIGHT)) {
        SND(Sound_Click());
        if (rulesPage < 2) {
            rulesPage++;
            Draw_Rules();
        } else {
            tankType  = 0;
            gameState = STATE_TANK_SELECT;
            Draw_TankSelect();
        }
    }
    if (Switch_Pressed(SW_LEFT)) {
        SND(Sound_Click());
        if (rulesPage > 0) { rulesPage--; Draw_Rules(); }
        else               { gameState = STATE_LANGUAGE; Draw_Language(); }
    }
    if (Switch_Pressed(SW_FIRE) && rulesPage == 2) {
        SND(Sound_Click());
        tankType  = 0;
        gameState = STATE_TANK_SELECT;
        Draw_TankSelect();
    }
}

static void State_TankSelect(uint32_t adc) {
    bool changed = false;
    if (Switch_Pressed(SW_LEFT)  && tankType > 0) { tankType--; changed = true; }
    if (Switch_Pressed(SW_RIGHT) && tankType < 2) { tankType++; changed = true; }
    if (changed) { SND(Sound_Click()); Draw_TankSelect(); }

    if (Switch_Pressed(SW_FIRE)) {
        SND(Sound_Click());
        GameUART_SendTank(tankType);
        BattleReady_Init();
        readySignalSent  = false;
        waitingAnimTick  = 0;
        Draw_Waiting();
        gameState = STATE_WAITING;
    }
}

static void State_Waiting(uint32_t adc) {
    HandleUART();

    if (!readySignalSent) {
        BattleReady_AssertReady();
        readySignalSent = true;
    }

    // Animate waiting dots
    if (++waitingAnimTick % 20 == 0) {
        uint8_t phase = (waitingAnimTick / 20) % 4;
        static const char* dotFrames[4] = {"   ", "  .", "  ..", ".."};
        GFX_FillRect(50, 72, 60, 12, 0x0000);
        GFX_DrawString(52, 74, dotFrames[phase], 0xC618, 0x0000, 1);
    }

    if (BattleReady_OpponentIsReady()) {
        countdownNum   = 3;
        countdownTimer = 28;
        gameState      = STATE_COUNTDOWN;
        Draw_Countdown(3);
        SND(Sound_Beep(0));
    }
}

static void State_Countdown(uint32_t adc) {
    HandleUART();
    if (--countdownTimer == 0) {
        if (countdownNum > 0) {
            countdownNum--;
            countdownTimer = 28;
            if (countdownNum > 0) {
                Draw_Countdown(countdownNum);
                SND(Sound_Beep(3 - countdownNum));
            } else {
                Draw_Countdown(0);
                SND(Sound_Fight());
                countdownTimer = 22;
            }
        } else {
            EnterGameplay();
        }
    }
}

static void State_Gameplay(uint32_t adc) {
    CheckComboExit();
    if (gameState != STATE_GAMEPLAY) return;

    // Match timer countdown
    timerBlinkTick++;
    if (++matchTimerFrames >= 30) {
        matchTimerFrames = 0;
        if (matchTimerSec > 0) {
            matchTimerSec--;
            DrawTimerString();
        } else {
            EndByTimer();
            return;
        }
    }

    // Aim angle from slide pot
    uint8_t newAngle = (uint8_t)((adc * 89) / 4095);
    if (newAngle != aimAngle) {
        EraseAimLine();
        EraseTank(tankX, tankY);
        DrawTank(tankX, tankY, facingRight, tankType);
        aimAngle = newAngle;
        DrawAimLine();
    }

    // Tank movement
    int16_t speed   = (speedBoostTimer > 0) ? TANK_SPEED * 2 : TANK_SPEED;
    bool    moved   = false;

    if (Switch_Held(SW_LEFT) && tankX > TANK_X_MIN) {
        EraseAimLine();
        EraseTank(tankX, tankY);
        if (chargeState == CHARGE_CHARGING) EraseChargeBar();
        tankX -= speed;
        if (tankX < TANK_X_MIN) tankX = TANK_X_MIN;
        tankY = GetTankGroundY(tankX);
        moved = true;
    }
    if (Switch_Held(SW_RIGHT) && tankX < TANK_X_MAX) {
        EraseAimLine();
        EraseTank(tankX, tankY);
        if (chargeState == CHARGE_CHARGING) EraseChargeBar();
        tankX += speed;
        if (tankX > TANK_X_MAX) tankX = TANK_X_MAX;
        tankY = GetTankGroundY(tankX);
        moved = true;
    }
    if (moved) {
        DrawTank(tankX, tankY, facingRight, tankType);
        if (chargeState == CHARGE_CHARGING) DrawChargeBar();
        DrawAimLine();
    }

    // Charge and fire
    bool fireHeld     = Switch_Held(SW_FIRE);
    bool fireReleased = Switch_Released(SW_FIRE);

    if (fireHeld && chargeState == CHARGE_IDLE) {
        chargeState  = CHARGE_CHARGING;
        chargeLevel  = 0;
        prevChargeLevel = 0;
        DrawChargeBar();
    }
    if (chargeState == CHARGE_CHARGING) {
        if (fireHeld && chargeLevel < CHARGE_MAX) {
            chargeLevel++;
            if (chargeLevel != prevChargeLevel) {
                DrawChargeBar();
                prevChargeLevel = chargeLevel;
            }
        }
        if (fireReleased) {
            uint8_t power = POWER_MIN + (chargeLevel * (POWER_MAX - POWER_MIN)) / CHARGE_MAX;
            EraseChargeBar();
            chargeState = CHARGE_IDLE;
            chargeLevel = 0;
            FireMissile(aimAngle, power, ricochetNextShot);
            ricochetNextShot = false;
        }
    }

    // Ability button
    if (Switch_Pressed(SW_SHIELD)) UseAbility();

    // Tick cooldowns
    if (abilityCooldown > 0) abilityCooldown--;
    if (speedBoostTimer > 0) speedBoostTimer--;

    // Physics updates
    UpdateMissiles();
    UpdateExplosions();

    // Redraw tank (missiles or explosions may have erased it)
    DrawTank(tankX, tankY, facingRight, tankType);

    // Hit flash effect
    if (flashTimer > 0) {
        flashTimer--;
        if (flashTimer % 2 == 0) {
            GFX_FillRect(0, HUD_HEIGHT, SCREEN_W, SCREEN_H - HUD_HEIGHT, 0xF800);
            Clock_Delay1ms(18);
            Draw_CityBackground();
            DrawTank(tankX, tankY, facingRight, tankType);
            DrawAimLine();
        }
    }

    // Incoming warning banner
    if (incomingWarnTimer > 0) {
        incomingWarnTimer--;
        if (incomingWarnTimer == 0) GFX_ClearIncoming();
    }

    // Invincibility blink
    if (invincibleTimer > 0) {
        invincibleTimer--;
        EraseTank(tankX, tankY);
        DrawTank(tankX, tankY, facingRight, tankType);
    }

    // Check win / loss conditions
    if (myLives == 0) {
        GameUART_SendWin((PLAYER_ID == 1) ? 2 : 1);
        SND(Sound_Stop());
        Clock_Delay1ms(300);
        SND(Sound_Defeat());
        char pb[10] = "PLAYER 1";
        pb[7] = '0' + PLAYER_ID;
        GFX_FillScreen(0x0000);
        GFX_DrawStringCentered( 8, GS("DEFEATED",     "DERROTADO"),   0xF800, 0, 2);
        GFX_DrawStringCentered(32, pb,                                  0x39E7, 0, 1);
        GFX_DrawStringCentered(48, GS("YOU LOSE!",    "PERDISTE!"),   0xFFFF, 0, 1);
        GFX_DrawLine(0, 64, SCREEN_W, 64, 0x39E7);
        GFX_DrawStringCentered(74, GS("FIRE=REMATCH", "DISP=REVANCHA"),0x07FF, 0, 1);
        GFX_DrawStringCentered(86, GS("SHLD=QUIT",    "ESCU=SALIR"),   0xC618, 0, 1);
        playerWon = false;
        gameState = STATE_END;
        return;
    }

    if (oppLives == 0) {
        playerWon = true;
        SND(Sound_Stop());
        Clock_Delay1ms(300);
        SND(Sound_Victory());
        char pb[10] = "PLAYER 1";
        pb[7] = '0' + PLAYER_ID;
        GFX_FillScreen(0x0000);
        GFX_DrawStringCentered(10, GS("VICTORY!",     "VICTORIA!"),        0xFFE0, 0, 2);
        GFX_DrawStringCentered(34, pb,                                       0x07FF, 0, 1);
        GFX_DrawStringCentered(50, GS("ENEMY DOWN",   "ENEMIGO VENCIDO"),  0x07E0, 0, 1);
        GFX_DrawLine(0, 66, SCREEN_W, 66, 0x39E7);
        GFX_DrawStringCentered(76, GS("FIRE=REMATCH", "DISP=REVANCHA"),    0x07FF, 0, 1);
        GFX_DrawStringCentered(88, GS("SHLD=QUIT",    "ESCU=SALIR"),       0xC618, 0, 1);
        gameState = STATE_END;
        return;
    }

    HandleUART();
}

static void State_End(uint32_t adc) {
    if (Switch_Pressed(SW_FIRE)) {
        SND(Sound_Click());
        tankType  = 0;
        gameState = STATE_TANK_SELECT;
        Draw_TankSelect();
    }
    if (Switch_Pressed(SW_SHIELD)) {
        SND(Sound_Click());
        EnterSplash();
    }
    HandleUART();
}

// ================================================================
// Main funcs for main.cpp to use
// ================================================================

void Game_Init(void) {
    EnterSplash();
}

void Game_Update(uint32_t adcValue) {
    Switch_Tick();
    switch (gameState) {
        case STATE_SPLASH:      State_Splash(adcValue);      break;
        case STATE_LANGUAGE:    State_Language(adcValue);    break;
        case STATE_RULES:       State_Rules(adcValue);       break;
        case STATE_TANK_SELECT: State_TankSelect(adcValue);  break;
        case STATE_WAITING:     State_Waiting(adcValue);     break;
        case STATE_COUNTDOWN:   State_Countdown(adcValue);   break;
        case STATE_GAMEPLAY:    State_Gameplay(adcValue);    break;
        case STATE_END:         State_End(adcValue);         break;
    }
}
