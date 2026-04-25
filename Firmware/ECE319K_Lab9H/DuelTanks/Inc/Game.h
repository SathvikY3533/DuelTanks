// Game.h
// Duel Tanks — complete game header
// Side-view, parabolic physics, full state machine
// ECE319H Spring 2026

#ifndef GAME_H
#define GAME_H

#include <stdint.h>
#include <stdbool.h>
#include "Graphics.h"
#include "Sprites.h"
#include "Sound.h"
#include "GameUART.h"
#include "Switches.h"
#include <ti/devices/msp/msp.h>
#include "../../inc/LaunchPad.h"

// RGB565 helper macro - lowkey this is chat helping me
#define rgb(r, g, b) \
    ((uint16_t)(((uint16_t)((r) >> 3) << 11) | \
                ((uint16_t)((g) >> 2) <<  5) | \
                ((uint8_t) ((b) >> 3)      )))

// Board identity — set PLAYER_ID to 1 on left board, 2 on right board
#define PLAYER_ID       1

// Sound toggle — set to 0 to silence everything during debugging
#define SOUND_ENABLE    1
#if SOUND_ENABLE
    #define SND(fn)     (fn)
#else
    #define SND(fn)     ((void)0)
#endif

// Battle Ready GPIO
// PA17: P1=output (assert ready), P2=input (read P1)
// PA31: P1=input  (read P2),      P2=output (assert ready)
// Verify PINCM indices against your LaunchPad.h
#ifndef PA17INDEX
    #define PA17INDEX   36
#endif
#ifndef PA31INDEX
    #define PA31INDEX   8
#endif

// Screen layout
#define HUD_HEIGHT      14

#define SKY_COLOR       ((uint16_t)0x0212)   // deep blue - buildings
#define GRASS_COLOR     ((uint16_t)0x2320)
#define EARTH_COLOR     ((uint16_t)0x5268)

// Tank constants
#define TANK_W              24
#define TANK_H              12
#define TANK_SPEED          3

#define BARREL_TIP_X_RIGHT  22     // x offset from sprite left when facing right
#define BARREL_TIP_X_LEFT   1      // x offset from sprite left when facing left
#define BARREL_TIP_Y        1      // y offset from sprite top

#define TANK_X_MIN          4
#define TANK_X_MAX          (SCREEN_W - TANK_W - 4)
#define AIM_LINE_LEN        20

#if PLAYER_ID == 1
    #define TANK_START_X    14
    #define FACING_RIGHT    1
#else
    #define TANK_START_X    (SCREEN_W - TANK_W - 14)
    #define FACING_RIGHT    0
#endif

// Missile & physics!!! - all fixed point :)
#define MISSILE_W           10
#define MISSILE_H           5
#define MAX_MISSILES        3

// Fixed-point scale: store positions as pixel * FP_SCALE
// Actual pixel = value >> 3
#define FP_SCALE            8
#define GRAVITY             2       // fixed-point gravity per tick (0.25 px/tick^2)

#define POWER_MIN           3       // minimum shot power
#define POWER_MAX           12      // maximum shot power
#define CHARGE_MAX          40      // ticks to reach full charge (~1.3 sec at 30Hz)

// Hitbox size
#define HIT_OFFSET_X        2
#define HIT_OFFSET_Y        4
#define HIT_W               12
#define HIT_H               5

// Gameplay settings
#define MAX_LIVES               3
#define INVINCIBLE_TICKS        25      // invincibility frames after a hit
#define FLASH_TICKS             6       // red screen flash frames after a hit
#define INCOMING_WARN_TICKS     18      // frames to show INCOMING! warning
#define MAX_EXPLOSIONS          4
#define EXP_TICKS_PER_FRAME     3       // explosion animation speed
#define ABILITY_COOLDOWN        75      // ~2.5 sec cooldown
#define SPEED_BOOST_TICKS       45      // special ability duration
#define CLOAK_TICKS             45      // ability as well

// States - splash screens to gameplay
typedef enum {
    STATE_SPLASH,
    STATE_LANGUAGE,
    STATE_RULES,
    STATE_TANK_SELECT,
    STATE_WAITING,
    STATE_COUNTDOWN,
    STATE_GAMEPLAY,
    STATE_END
} GameState;

typedef enum {
    CHARGE_IDLE,
    CHARGE_CHARGING
} ChargeState;

// Missile struct to send pos
typedef struct {
    int16_t xfp, yfp;       // position * FP_SCALE
    int16_t vxfp, vyfp;     // velocity * FP_SCALE per tick
    int16_t prevX, prevY;   // last drawn pixel position (for erase)
    bool    active;
    bool    incoming;        // came from UART, heading toward our tank
    bool    ricochet;        // will bounce off terrain once
} Missile_t;

typedef struct {
    int16_t cx, cy;         // center in pixels
    uint8_t frame;          // current frame 0-3
    uint8_t ticksLeft;
    bool    active;
} Explosion_t;

// State variables - used to keep track and sync game state between both MSPM0s
extern GameState    gameState;
extern uint8_t      lang;               // 0=English, 1=Spanish
extern uint8_t      rulesPage;          // 0, 1, or 2
extern uint8_t      tankType;           // 0=Brawler, 1=Phantom, 2=Artillery
extern uint8_t      oppTankType;
extern int16_t      tankX, tankY;       // tank sprite top-left pixel
extern bool         facingRight;
extern uint8_t      myLives;
extern uint8_t      oppLives;           // updated via UART PKT_HIT
extern uint8_t      myKills;
extern uint8_t      invincibleTimer;
extern uint8_t      flashTimer;
extern uint8_t      incomingWarnTimer;
extern ChargeState  chargeState;
extern uint8_t      chargeLevel;        // 0 to CHARGE_MAX
extern uint8_t      aimAngle;           // 0 to 89 degrees
extern Missile_t    missiles[MAX_MISSILES];
extern Explosion_t  explosions[MAX_EXPLOSIONS];
extern uint8_t      countdownNum;
extern uint8_t      countdownTimer;
extern bool         playerWon;
extern bool         showPrompt;         // splash screen prompt blink state
extern uint8_t      abilityCooldown;
extern uint8_t      speedBoostTimer;

extern const int8_t cos32[90];
extern const int8_t sin32[90];

void Game_Init(void);                   // call once at startup
void Game_Update(uint32_t adcValue);    // call every tick with slide pot reading

#endif // GAME_H