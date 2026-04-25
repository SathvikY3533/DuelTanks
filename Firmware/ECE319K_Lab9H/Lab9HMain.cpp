// Lab9HMain.cpp
// Runs on MSPM0G3507
// Lab 9 ECE319H
// Sathvik Yechuri
// Last Modified: April 23, 2026

#include "../DuelTanks/Inc/Game.h"
#include "../DuelTanks/Inc/Graphics.h"
#include "../DuelTanks/Inc/Sound.h"
#include "../DuelTanks/Inc/StatusLEDs.h"
#include "../DuelTanks/Inc/Switches.h"
#include "../inc/Clock.h"
#include "../inc/DAC5.h"
#include "../inc/LaunchPad.h"
#include "../inc/ST7735.h"
#include "../inc/SlidePot.h"
#include "../inc/TExaS.h"
#include "../inc/Timer.h"
#include "LED.h"
#include "SmallFont.h"
#include "images/images.h"
#include "../DuelTanks/Inc/GameUART.h"
#include <stdint.h>
#include <stdio.h>
#include <ti/devices/msp/msp.h>

extern "C" void __disable_irq(void);
extern "C" void __enable_irq(void);
extern "C" void TIMG12_IRQHandler(void);
// ****note to ECE319K students****
// the data sheet says the ADC does not work when clock is 80 MHz
// however, the ADC seems to work on my boards at 80 MHz
// I suggest you try 80MHz, but if it doesn't work, switch to 40MHz
void PLL_Init(void) { // set phase lock loop (PLL)
  // Clock_Init40MHz(); // run this line for 40MHz
  Clock_Init80MHz(0); // run this line for 80MHz
}

// test bitmap
static const uint16_t testBitmap[100] = {
    0xFFE0, 0xFFE0, 0xFFE0, 0xFFE0, 0xFFE0, 0xFFE0, 0xFFE0, 0xFFE0, 0xFFE0,
    0xFFE0, 0xFFE0, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800,
    0xF800, 0xFFE0, 0xFFE0, 0xF800, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xF800, 0xFFE0, 0xFFE0, 0xF800, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xF800, 0xFFE0, 0xFFE0, 0xF800, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xF800, 0xFFE0, 0xFFE0, 0xF800, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xF800, 0xFFE0, 0xFFE0, 0xF800, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xF800, 0xFFE0, 0xFFE0, 0xF800,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xF800, 0xFFE0, 0xFFE0,
    0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xFFE0,
    0xFFE0, 0xFFE0, 0xFFE0, 0xFFE0, 0xFFE0, 0xFFE0, 0xFFE0, 0xFFE0, 0xFFE0,
    0xFFE0,
};

uint32_t M = 1;
uint32_t Random32(void) {
  M = 1664525 * M + 1013904223;
  return M;
}
uint32_t Random(uint32_t n) { return (Random32() >> 16) % n; }

SlidePot Sensor(1500, 0); // copy calibration from Lab 7

// games  engine runs at 30Hz
void TIMG12_IRQHandler(void) {
  uint32_t pos, msg;
  if ((TIMG12->CPU_INT.IIDX) == 1) { // this will acknowledge
    GPIOB->DOUTTGL31_0 = GREEN; // toggle PB27 (minimally intrusive debugging)
    GPIOB->DOUTTGL31_0 = GREEN; // toggle PB27 (minimally intrusive debugging)
                                // game engine goes here
    // 1) sample slide pot
    // 2) read input switches
    // 3) move sprites
    // 4) start sounds
    // 5) set semaphore
    // NO LCD OUTPUT IN INTERRUPT SERVICE ROUTINES
    GPIOB->DOUTTGL31_0 = GREEN; // toggle PB27 (minimally intrusive debugging)
  }
}

uint8_t TExaS_LaunchPadLogicPB27PB26(void) {
  return (0x80 | ((GPIOB->DOUT31_0 >> 26) & 0x03));
}

typedef enum { English, Spanish, Portuguese, French } Language_t;
Language_t myLanguage = English;
typedef enum { HELLO, GOODBYE, LANGUAGE } phrase_t;
const char Hello_English[] = "Hello";
const char Hello_Spanish[] = "\xADHola!";
const char Hello_Portuguese[] = "Ol\xA0";
const char Hello_French[] = "All\x83";
const char Goodbye_English[] = "Goodbye";
const char Goodbye_Spanish[] = "Adi\xA2s";
const char Goodbye_Portuguese[] = "Tchau";
const char Goodbye_French[] = "Au revoir";
const char Language_English[] = "English";
const char Language_Spanish[] = "Espa\xA4ol";
const char Language_Portuguese[] = "Portugu\x88s";
const char Language_French[] = "Fran\x87"
                               "ais";
const char *Phrases[3][4] = {
    {Hello_English, Hello_Spanish, Hello_Portuguese, Hello_French},
    {Goodbye_English, Goodbye_Spanish, Goodbye_Portuguese, Goodbye_French},
    {Language_English, Language_Spanish, Language_Portuguese, Language_French}};



int main(void) {
    __disable_irq();
    PLL_Init();          // 80 MHz
    LaunchPad_Init();    // GPIO, ports A and B
    GFX_Init();          // LCD landscape 160x128
    Switch_Init();       // PB12 PB16 PB17 PA27
    Sound_Init();        // SysTick + DAC at 11025 Hz
    GameUART_Init();     // UART1 TX PA8, UART2 RX PA22
    Sensor.Init();       // ADC for slide pot PB18
    __enable_irq();

    Game_Init();         // draws splash screen, sets up state machine

    while (1) {
        Game_Update(Sensor.In());   // pass ADC reading every tick
        Clock_Delay1ms(33);         // ~30 Hz game loop
    }
}











//------------------ Testing Code -----------------------

int main1(void) { // Graphics
  __disable_irq();
  PLL_Init(); // set bus speed
  LaunchPad_Init();
  GFX_Init();

  // Sensor.Init(); // PB18 = ADC1 channel 5, slidepot
  // Switch_Init(); // initialize switches
  // LED_Init();    // initialize LED
  // Sound_Init();  // initialize sound

  TExaS_Init(0, 0, &TExaS_LaunchPadLogicPB27PB26); // PB27 and PB26

  __enable_irq();

  // basic shapes
  GFX_FillScreen(COLOR_BLACK);
  GFX_DrawString(2, 2, "TEST 1: BASIC SHAPES", COLOR_WHITE, COLOR_BLACK, 1);

  GFX_FillRect(10, 20, 40, 20, COLOR_RED);
  GFX_DrawRect(60, 20, 40, 20, COLOR_GREEN);
  GFX_DrawLine(110, 20, 150, 40, COLOR_BLUE);
  GFX_DrawCircle(30, 70, 12, COLOR_YELLOW);
  GFX_FillCircle(80, 70, 10, COLOR_MAGENTA);
  GFX_DrawLine(0, SCREEN_H - 1, SCREEN_W - 1, SCREEN_H - 1, COLOR_CYAN);

  Clock_Delay1ms(2000);

  // Rext sizes + colors
  GFX_FillScreen(COLOR_BLACK);
  GFX_DrawString(2, 4, "SIZE 1 TEXT - ABCDEF 0123", COLOR_WHITE, COLOR_BLACK,
                 1);
  GFX_DrawString(2, 20, "SIZE 2", COLOR_YELLOW, COLOR_BLACK, 2);
  GFX_DrawString(2, 40, "SZ3", COLOR_CYAN, COLOR_BLACK, 3);
  GFX_DrawStringCentered(90, "CENTERED", COLOR_GREEN, COLOR_BLACK, 2);
  GFX_DrawInt(2, 112, -31415, COLOR_RED, COLOR_BLACK, 1);

  Clock_Delay1ms(2000);

  // Sprite moving test
  GFX_FillScreen(COLOR_BACKGROUND);
  GFX_DrawString(2, 2, "TEST 3: SPRITE MOVE", COLOR_WHITE, COLOR_BACKGROUND, 1);

  Sprite_t testSprite;
  GFX_SpriteInit(&testSprite, 0, 60, 10, 10, testBitmap);

  // Move sprite left to right across screen
  for (int16_t x = 0; x <= SCREEN_W - 10; x += 2) {
    GFX_SpriteMove(&testSprite, x, 60);
    GFX_SpriteDraw(&testSprite);
    Clock_Delay1ms(15);
  }

  // diagonal move
  for (int16_t i = 0; i < 50; i++) {
    GFX_SpriteMove(&testSprite, SCREEN_W - 10 - i * 2, 60 + i);
    GFX_SpriteDraw(&testSprite);
    Clock_Delay1ms(15);
  }

  // Test visibility toggle (cloak) - for tank ability
  GFX_DrawString(2, 2, "CLOAK TEST: BLINK  ", COLOR_WHITE, COLOR_BACKGROUND, 1);
  for (uint8_t i = 0; i < 6; i++) {
    testSprite.visible = !testSprite.visible;
    testSprite.dirty = true;
    GFX_SpriteDraw(&testSprite);
    Clock_Delay1ms(200);
  }
  testSprite.visible = true;

  Clock_Delay1ms(500);

  // HUD test
  GFX_FillScreen(COLOR_BACKGROUND);
  GFX_DrawHUD(3, 0, 0);
  GFX_DrawStringCentered(60, "HUD TEST", COLOR_WHITE, COLOR_BACKGROUND, 1);
  Clock_Delay1ms(800);

  // Simulate taking damage + getting kills + charging ability
  GFX_UpdateLives(2);
  Clock_Delay1ms(400);
  GFX_UpdateLives(1);
  Clock_Delay1ms(400);
  GFX_UpdateKills(1);
  Clock_Delay1ms(300);
  GFX_UpdateKills(2);
  Clock_Delay1ms(300);

  for (uint8_t p = 0; p <= 100; p += 5) {
    GFX_UpdateCooldownBar(p);
    Clock_Delay1ms(40);
  }

  // BOOM explode animation
  GFX_FillScreen(COLOR_BACKGROUND);
  GFX_DrawHUD(3, 0, 0);
  GFX_DrawString(2, 20, "TEST 5: EXPLOSION", COLOR_WHITE, COLOR_BACKGROUND, 1);

  int16_t ex = SCREEN_W / 2;
  int16_t ey = SCREEN_H / 2;

  for (uint8_t frame = 0; frame < 4; frame++) {
    GFX_DrawExplosionFrame(ex, ey, frame);
    Clock_Delay1ms(80);
  }
  GFX_EraseExplosion(ex, ey);
  Clock_Delay1ms(500);

  // Incoming warning test
  GFX_FillScreen(COLOR_BACKGROUND);
  GFX_DrawHUD(3, 0, 0);
  GFX_DrawString(2, 40, "TEST 6: INCOMING", COLOR_WHITE, COLOR_BACKGROUND, 1);
  GFX_DrawIncoming();
  Clock_Delay1ms(1500);
  GFX_ClearIncoming();
  Clock_Delay1ms(500);

  //  charge bar animation test
  GFX_FillScreen(COLOR_BACKGROUND);
  GFX_DrawHUD(3, 0, 0);
  GFX_DrawString(2, 40, "TEST 7: CHARGE BAR", COLOR_WHITE, COLOR_BACKGROUND, 1);
  for (uint8_t p = 0; p <= 100; p += 2) {
    GFX_DrawChargeBar(75, 60, p);
    Clock_Delay1ms(30);
  }
  GFX_EraseChargeBar(75, 60);
  Clock_Delay1ms(500);

  // text animation shaking
  
  GFX_FillScreen(COLOR_BACKGROUND);
  GFX_DrawHUD(3, 0, 0);
  GFX_DrawString(2, 50, "TEST 8: SHAKE", COLOR_WHITE, COLOR_BACKGROUND, 1);
  GFX_ShakeStart(3);
  for (uint8_t i = 0; i < 10; i++) {
    GFX_ShakeTick();
    // In real game you would shift all sprite draw calls by ShakeOffsetX/Y
    // Here just visualize the offset values
    GFX_FillRect(60, 60, 40, 10, COLOR_BACKGROUND);
    GFX_DrawString(60, 62 + GFX_ShakeOffsetY(), "SHAKING", COLOR_RED,
                   COLOR_BACKGROUND, 1);
    Clock_Delay1ms(50);
  }
  Clock_Delay1ms(500);

  // splash screens -------------------------------------------------------

  // Splash — blink prompt 4 times
  for (uint8_t i = 0; i < 4; i++) {
    GFX_DrawSplash(i % 2 == 0);
    Clock_Delay1ms(600);
  }

  // Language select — both options highlighted
  GFX_DrawLanguageSelect(0);
  Clock_Delay1ms(1000);
  GFX_DrawLanguageSelect(1);
  Clock_Delay1ms(1000);

  // Rules pages
  GFX_DrawRulesPage(0, 0);
  Clock_Delay1ms(2000);
  GFX_DrawRulesPage(1, 0);
  Clock_Delay1ms(2000);
  GFX_DrawRulesPage(2, 0);
  Clock_Delay1ms(2000);

  // Rules pages in Spanish
  GFX_DrawRulesPage(0, 1);
  Clock_Delay1ms(1500);
  GFX_DrawRulesPage(1, 1);
  Clock_Delay1ms(1500);
  GFX_DrawRulesPage(2, 1);
  Clock_Delay1ms(1500);

  // Tank select — cycle through tanks
  GFX_DrawTankSelect(0, -1, false, 0);
  Clock_Delay1ms(800);
  GFX_DrawTankSelect(1, -1, false, 0);
  Clock_Delay1ms(800);
  GFX_DrawTankSelect(2, 0, false, 0);
  Clock_Delay1ms(800); // opponent on tank 0
  GFX_DrawTankSelect(2, 0, true, 0);
  Clock_Delay1ms(800); // opponent ready

  // Waiting screen
  for (uint8_t d = 0; d < 12; d++) {
    GFX_DrawWaiting(d, 0);
    Clock_Delay1ms(300);
  }

  // Countdown
  GFX_DrawCountdown(3);
  Clock_Delay1ms(800);
  GFX_DrawCountdown(2);
  Clock_Delay1ms(800);
  GFX_DrawCountdown(1);
  Clock_Delay1ms(800);
  GFX_DrawCountdown(0); // FIGHT! includes its own delay

  // end screens
  GFX_DrawEndScreen(true, 3, 0);
  GFX_DrawRematch(0);
  Clock_Delay1ms(2000);

  GFX_DrawEndScreen(false, 1, 0);
  GFX_DrawRematch(0);
  Clock_Delay1ms(2000);

  // Spanish end screen
  GFX_DrawEndScreen(true, 2, 1);
  GFX_DrawRematch(1);
  Clock_Delay1ms(2000);

  // ================================================================
  // TEST COMPLETE
  // ================================================================
  GFX_FillScreen(COLOR_BLACK);
  GFX_DrawStringCentered(50, "GFX TESTS: JOLLY GOOD", COLOR_GREEN, COLOR_BLACK, 1);
}

int main2(void) {
  __disable_irq();
  PLL_Init();
  LaunchPad_Init();
  GFX_Init();
  Switch_Init();
  Sensor.Init(); // PB18 = ADC1 ch5, slide pot
  __enable_irq();

  // ---- Draw static labels (only once) ----
  GFX_FillScreen(COLOR_BLACK);
  GFX_DrawStringCentered(2, "INPUT TEST", COLOR_YELLOW, COLOR_BLACK,
                         1);
  GFX_DrawLine(0, 12, SCREEN_W, 12, COLOR_DARKGRAY);

  // Button labels
  GFX_DrawString(4, 16, "FIRE", COLOR_DARKGRAY, COLOR_BLACK, 1);
  GFX_DrawString(44, 16, "SHIELD", COLOR_DARKGRAY, COLOR_BLACK, 1);
  GFX_DrawString(92, 16, "LEFT", COLOR_DARKGRAY, COLOR_BLACK, 1);
  GFX_DrawString(130, 16, "RIGHT", COLOR_DARKGRAY, COLOR_BLACK, 1);

  // Slide pot labels
  GFX_DrawString(4, 65, "SLIDE POT RAW:", COLOR_DARKGRAY, COLOR_BLACK, 1);
  GFX_DrawString(4, 77, "CONVERTED mm: ", COLOR_DARKGRAY, COLOR_BLACK, 1);
  GFX_DrawString(4, 90, "BAR:", COLOR_DARKGRAY, COLOR_BLACK, 1);

  // charge ticks label
  GFX_DrawString(4, 108, "FIRE HELD TICKS:", COLOR_DARKGRAY, COLOR_BLACK, 1);

  // Press count labels
  GFX_DrawString(4, 118, "PRESS COUNTS:", COLOR_DARKGRAY, COLOR_BLACK, 1);

  // button boundary
  const int16_t BX[4] = {2, 42, 90, 128};
  const int16_t BY = 26;
  const int16_t BW = 36;
  const int16_t BH = 18;

  // storing button states
  uint8_t prevRaw = 0;
  uint8_t pressCount[4] = {0, 0, 0, 0};
  uint8_t prevButtons = 0;

  // Slide pot bar - animation (use for charge? idk yet)
  const int16_t BAR_X = 28;
  const int16_t BAR_Y = 92;
  const int16_t BAR_W = 128;
  const int16_t BAR_H = 8;
  GFX_DrawRect(BAR_X - 1, BAR_Y - 1, BAR_W + 2, BAR_H + 2, COLOR_LIGHTGRAY);

  while (1) {

    Switch_Tick(); //get switch states
    uint8_t buttons = Switch_In();
    uint32_t rawADC = Sensor.In();            // 12-bit ADC value, 0-4095
    uint32_t convMM = Sensor.Convert(rawADC); // converted distance for fun

    const uint8_t MASKS[4] = {SW_FIRE, SW_SHIELD, SW_LEFT, SW_RIGHT};
    const char *LABELS[4] = {"FIRE", "SHLD", "LEFT", "RGHT"};

    for (int i = 0; i < 4; i++) {
      bool pressed = (buttons & MASKS[i]) != 0;
      bool wasPressed = (prevRaw & MASKS[i]) != 0;

      // redraw stuff
      if (pressed != wasPressed) {
        uint16_t bg = pressed ? COLOR_GREEN : COLOR_BLACK;
        uint16_t fg = pressed ? COLOR_BLACK : COLOR_DARKGRAY;
        uint16_t bdr = pressed ? COLOR_GREEN : COLOR_DARKGRAY;
        GFX_FillRect(BX[i], BY, BW, BH, bg);
        GFX_DrawRect(BX[i], BY, BW, BH, bdr);
        GFX_DrawString(BX[i] + 2, BY + 5, LABELS[i], fg, bg, 1);
      }

      // Count rising edges for counting stuff
      if (Switch_Pressed(MASKS[i])) {
        pressCount[i]++;
      }
    }

    // ---- Update slide pot raw value ----
    // Only redraw number if it changed significantly - no no flicker
    static uint32_t lastRaw = 9999;
    if ((rawADC > lastRaw + 20) || (lastRaw > rawADC + 20) || lastRaw == 9999) {
      GFX_FillRect(90, 65, 60, 8, COLOR_BLACK);
      GFX_DrawInt(90, 65, (int32_t)rawADC, COLOR_WHITE, COLOR_BLACK, 1);

      GFX_FillRect(90, 77, 60, 8, COLOR_BLACK);
      GFX_DrawInt(90, 77, (int32_t)convMM, COLOR_CYAN, COLOR_BLACK, 1);

      // bar animation updationg
      int16_t fillW = (int16_t)(((uint32_t)rawADC * BAR_W) / 4095);
      if (fillW > BAR_W)
        fillW = BAR_W;
      GFX_FillRect(BAR_X, BAR_Y, fillW, BAR_H, COLOR_CYAN);
      if (fillW < BAR_W) {
        GFX_FillRect(BAR_X + fillW, BAR_Y, BAR_W - fillW, BAR_H, COLOR_BLACK);
      }

      lastRaw = rawADC;
    }

    // charge animation counter!!!
    static uint8_t lastHeld = 255;
    uint8_t heldNow = Switch_HeldTicks(SW_FIRE);
    if (heldNow != lastHeld) {
      GFX_FillRect(100, 108, 40, 8, COLOR_BLACK);
      GFX_DrawInt(100, 108, heldNow, COLOR_ORANGE, COLOR_BLACK, 1);
      uint8_t chargePct = (heldNow >= 90) ? 100 : (heldNow * 100) / 90;
      GFX_DrawChargeBar(BAR_X, 108, chargePct);
      lastHeld = heldNow;
    }

    //update btn presses om the ui
    static uint8_t lastCounts[4] = {255, 255, 255, 255};
    for (int i = 0; i < 4; i++) {
      if (pressCount[i] != lastCounts[i]) {
        GFX_FillRect(BAR_X + i * 28, 118, 24, 8, COLOR_BLACK);
        GFX_DrawInt(BAR_X + i * 28, 118, pressCount[i], COLOR_WHITE,
                    COLOR_BLACK, 1);
        lastCounts[i] = pressCount[i];
      }
    }

    prevRaw = buttons;
    prevButtons = buttons;

    // Poll at ~60Hz (faster than game loop so UI feels snappy)
    Clock_Delay1ms(16);
  }
}

int main3(void) { // testing switches
  __disable_irq();
  PLL_Init();
  LaunchPad_Init();
  GFX_Init();
  Switch_Init();
  Sound_Init();
  TExaS_Init(ADC0, 6, 0); // scope on DAC output
  StatusLEDs_Init();
  __enable_irq();
  uint8_t states = Switch_In();

  while (1) {
    if(Switch_In() == SW_FIRE) led_toggle(X_LED1);
    Clock_Delay1ms(100);
  }
}

int main4(void) {
  __disable_irq();
  PLL_Init();
  LaunchPad_Init();
  GFX_Init();
  Switch_Init();
  Sound_Init();
  TExaS_Init(ADC0, 6, 0); // scope on DAC output
  __enable_irq();

  // ---- Static UI ----
  GFX_FillScreen(COLOR_BLACK);
  GFX_DrawStringCentered(2, "LAYER 3: SOUND TEST", COLOR_YELLOW, COLOR_BLACK,
                         1);
  GFX_DrawLine(0, 12, SCREEN_W, 12, COLOR_DARKGRAY);
  GFX_DrawString(2, 90, "FIRE:  cycle sounds", COLOR_DARKGRAY, COLOR_BLACK, 1);
  GFX_DrawString(2, 100, "SHLD:  explosion", COLOR_DARKGRAY, COLOR_BLACK, 1);
  GFX_DrawString(2, 110, "LEFT:  victory", COLOR_DARKGRAY, COLOR_BLACK, 1);
  GFX_DrawString(2, 120, "RGHT:  stop", COLOR_DARKGRAY, COLOR_BLACK, 1);

  // Sound names for display
  const char *soundNames[] = {
      "SHOOT",    "BARRIER",   "EXPLOSION", "VICTORY", "DEFEAT",   "BEEP LOW",
      "BEEP MID", "BEEP HIGH", "FIGHT!",    "CLICK",   "INCOMING",
  };
  const uint8_t NUM_SOUNDS = 11;
  uint8_t currentSound = 0;

  // ---- Playing indicator bar ----
  // Shows a simple animation when audio is playing
  uint8_t animFrame = 0;
  const char *bars[] = {"[=      ]", "[==     ]", "[===    ]", "[====   ]",
                        "[=====  ]", "[====== ]", "[=======]", "[====== ]",
                        "[=====  ]", "[====   ]", "[===    ]", "[==     ]"};

  auto showCurrentSound = [&]() {
    GFX_FillRect(0, 18, SCREEN_W, 20, COLOR_BLACK);
    GFX_DrawStringCentered(20, soundNames[currentSound], COLOR_WHITE,
                           COLOR_BLACK, 2);
  };

  showCurrentSound();

  uint8_t prevButtons = 0;

  while (1) {
    Switch_Tick();
    uint8_t buttons = Switch_In();

    // FIRE button — cycle and play next sound
    if (Switch_Pressed(SW_FIRE)) {
      switch (currentSound) {
      case 0:
        Sound_Shoot();
        break;
      case 1:
        Sound_Barrier();
        break;
      case 2:
        Sound_Explosion();
        break;
      case 3:
        Sound_Victory();
        break;
      case 4:
        Sound_Defeat();
        break;
      case 5:
        Sound_Beep(0);
        break;
      case 6:
        Sound_Beep(1);
        break;
      case 7:
        Sound_Beep(2);
        break;
      case 8:
        Sound_Fight();
        break;
      case 9:
        Sound_Click();
        break;
      case 10:
        Sound_Incoming();
        break;
      }
      currentSound = (currentSound + 1) % NUM_SOUNDS;
      showCurrentSound();
    }

    // SHIELD button — explosion (priority interrupt test)
    if (Switch_Pressed(SW_SHIELD)) {
      Sound_Explosion();
      currentSound = 2;
      showCurrentSound();
    }

    // LEFT button — victory fanfare
    if (Switch_Pressed(SW_LEFT)) {
      Sound_Barrier();
      currentSound = 3;
      showCurrentSound();
    }

    // RIGHT button — stop
    if (Switch_Pressed(SW_RIGHT)) {
      Sound_Stop();
      GFX_FillRect(0, 18, SCREEN_W, 20, COLOR_BLACK);
      GFX_DrawStringCentered(20, "STOPPED", COLOR_RED, COLOR_BLACK, 2);
    }

    // Animate playback bar
    if (Sound_IsPlaying()) {
      GFX_FillRect(2, 70, SCREEN_W - 4, 10, COLOR_BLACK);
      GFX_DrawString(2, 72, bars[animFrame % 12], COLOR_GREEN, COLOR_BLACK, 1);
      animFrame++;
    } else {
      GFX_FillRect(2, 70, SCREEN_W - 4, 10, COLOR_BLACK);
      GFX_DrawString(2, 72, "[       ]", COLOR_DARKGRAY, COLOR_BLACK, 1);
    }

    prevButtons = buttons;
    Clock_Delay1ms(16); // ~60Hz poll
  }
}


int main5(void) { // uart test
    __disable_irq();
    PLL_Init();
    LaunchPad_Init();
    GFX_Init();
    Switch_Init();
    Sound_Init();
    StatusLEDs_Init();
    GameUART_Init();
    __enable_irq();

    // ---- Static UI ----
    GFX_FillScreen(COLOR_BLACK);
    GFX_DrawStringCentered(2, "UART TEST", COLOR_YELLOW, COLOR_BLACK, 1);
    GFX_DrawLine(0, 12, SCREEN_W, 12, COLOR_DARKGRAY);

    GFX_DrawString(2, 16, "TX", COLOR_CYAN, COLOR_BLACK, 1);
    GFX_DrawLine(0, 24, SCREEN_W, 24, COLOR_DARKGRAY);
    GFX_DrawString(2, 27, "TYPE:", COLOR_DARKGRAY, COLOR_BLACK, 1);
    GFX_DrawString(2, 37, "D1:",   COLOR_DARKGRAY, COLOR_BLACK, 1);
    GFX_DrawString(2, 47, "D2:",   COLOR_DARKGRAY, COLOR_BLACK, 1);
    GFX_DrawString(2, 57, "CNT:", COLOR_DARKGRAY, COLOR_BLACK, 1);
    GFX_DrawLine(0, 67, SCREEN_W, 67, COLOR_DARKGRAY);

    GFX_DrawString(2, 70, "RX", COLOR_GREEN, COLOR_BLACK, 1);
    GFX_DrawLine(0, 78, SCREEN_W, 78, COLOR_DARKGRAY);
    GFX_DrawString(2, 81,  "TYPE:", COLOR_DARKGRAY, COLOR_BLACK, 1);
    GFX_DrawString(2, 91,  "D1:",   COLOR_DARKGRAY, COLOR_BLACK, 1);
    GFX_DrawString(2, 101, "D2:",   COLOR_DARKGRAY, COLOR_BLACK, 1);
    GFX_DrawString(2, 111, "CNT:", COLOR_DARKGRAY, COLOR_BLACK, 1);

    GFX_DrawString(95, 27, "FIRE=FIRE",  COLOR_DARKGRAY, COLOR_BLACK, 1);
    GFX_DrawString(95, 37, "SHLD=HIT",   COLOR_DARKGRAY, COLOR_BLACK, 1);
    GFX_DrawString(95, 47, "LEFT=RDY",   COLOR_DARKGRAY, COLOR_BLACK, 1);
    GFX_DrawString(95, 57, "RGHT=WIN",   COLOR_DARKGRAY, COLOR_BLACK, 1);

    auto typeName = [](uint8_t t) -> const char* {
        switch (t) {
            case PKT_FIRE:    return "FIRE   ";
            case PKT_HIT:     return "HIT    ";
            case PKT_READY:   return "READY  ";
            case PKT_WIN:     return "WIN    ";
            case PKT_LANG:    return "LANG   ";
            case PKT_TANK:    return "TANK   ";
            case PKT_REMATCH: return "REMATCH";
            default:          return "???    ";
        }
    };

    uint32_t txCount = 0, rxCount = 0;
    uint8_t lastTxType = 0, lastTxD1 = 0, lastTxD2 = 0;
    uint8_t lastRxType = 0, lastRxD1 = 0, lastRxD2 = 0;
    uint8_t txFlashTimer = 0, rxFlashTimer = 0;

    auto updateTxDisplay = [&]() {
        GFX_FillRect(32, 27, 58, 8, COLOR_BLACK);
        GFX_FillRect(32, 37, 58, 8, COLOR_BLACK);
        GFX_FillRect(32, 47, 58, 8, COLOR_BLACK);
        GFX_FillRect(32, 57, 58, 8, COLOR_BLACK);
        GFX_DrawString(32, 27, typeName(lastTxType), COLOR_CYAN, COLOR_BLACK, 1);
        GFX_DrawInt   (32, 37, lastTxD1,             COLOR_CYAN, COLOR_BLACK, 1);
        GFX_DrawInt   (32, 47, lastTxD2,             COLOR_CYAN, COLOR_BLACK, 1);
        GFX_DrawInt   (32, 57, txCount,              COLOR_CYAN, COLOR_BLACK, 1);
    };

    auto updateRxDisplay = [&]() {
        GFX_FillRect(32, 81,  58, 8, COLOR_BLACK);
        GFX_FillRect(32, 91,  58, 8, COLOR_BLACK);
        GFX_FillRect(32, 101, 58, 8, COLOR_BLACK);
        GFX_FillRect(32, 111, 58, 8, COLOR_BLACK);
        GFX_DrawString(32, 81,  typeName(lastRxType), COLOR_GREEN, COLOR_BLACK, 1);
        GFX_DrawInt   (32, 91,  lastRxD1,             COLOR_GREEN, COLOR_BLACK, 1);
        GFX_DrawInt   (32, 101, lastRxD2,             COLOR_GREEN, COLOR_BLACK, 1);
        GFX_DrawInt   (32, 111, rxCount,              COLOR_GREEN, COLOR_BLACK, 1);
    };

    updateTxDisplay();
    updateRxDisplay();

    while (1) {
        Switch_Tick();

        uint32_t raw   = Sensor.In();
        uint8_t  angle = (uint8_t)((raw * 255) / 4095);
        uint8_t  speed = (uint8_t)((raw * 15)  / 4095) + 1;

        if (Switch_Pressed(SW_FIRE)) {
            lastTxType = PKT_FIRE; lastTxD1 = angle; lastTxD2 = speed;
            // 3-arg: y_entry=angle (test value), vx_enc=speed, vy_enc=8 (vy=0)
            GameUART_SendFire(angle, speed, 8);
            txCount++; txFlashTimer = 5;
            SND(Sound_Click());
            updateTxDisplay();
        }
        if (Switch_Pressed(SW_SHIELD)) {
            lastTxType = PKT_HIT; lastTxD1 = 2; lastTxD2 = 0;
            GameUART_SendHit(2);
            txCount++; txFlashTimer = 5;
            SND(Sound_Click());
            updateTxDisplay();
        }
        if (Switch_Pressed(SW_LEFT)) {
            lastTxType = PKT_READY; lastTxD1 = 1; lastTxD2 = 0;
            GameUART_SendReady();
            txCount++; txFlashTimer = 5;
            SND(Sound_Click());
            updateTxDisplay();
        }
        if (Switch_Pressed(SW_RIGHT)) {
            lastTxType = PKT_WIN; lastTxD1 = 1; lastTxD2 = 0;
            GameUART_SendWin(1);
            txCount++; txFlashTimer = 5;
            SND(Sound_Click());
            updateTxDisplay();
        }

        if (txFlashTimer > 0) {
            GFX_FillRect(80, 16, 8, 8, COLOR_CYAN); txFlashTimer--;
        } else {
            GFX_FillRect(80, 16, 8, 8, COLOR_BLACK);
        }

        GamePacket_t pkt;
        while (GameUART_Receive(&pkt)) {
            lastRxType = pkt.type;
            lastRxD1   = pkt.data1;
            lastRxD2   = pkt.data2;
            rxCount++;
            rxFlashTimer = 5;
            SND(Sound_Click());
            updateRxDisplay();
        }

        if (rxFlashTimer > 0) {
            GFX_FillRect(80, 70, 8, 8, COLOR_GREEN); rxFlashTimer--;
        } else {
            GFX_FillRect(80, 70, 8, 8, COLOR_BLACK);
        }

        static uint8_t lastAngle = 255;
        if (angle != lastAngle) {
            GFX_FillRect(120, 37, 38, 8, COLOR_BLACK);
            GFX_DrawString(120, 37, "A:", COLOR_DARKGRAY, COLOR_BLACK, 1);
            GFX_DrawInt   (132, 37, angle, COLOR_ORANGE,  COLOR_BLACK, 1);
            lastAngle = angle;
        }

        Clock_Delay1ms(16);
    }
}






// // use main1 to observe special characters
// int main1(void) { // main1
//   char l;
//   __disable_irq();
//   PLL_Init(); // set bus speed
//   LaunchPad_Init();
//   GFX_Init();
//   for (int myPhrase = 0; myPhrase <= 2; myPhrase++) {
//     for (int myL = 0; myL <= 3; myL++) {
//       ST7735_OutString((char *)Phrases[LANGUAGE][myL]);
//       ST7735_OutChar(' ');
//       ST7735_OutString((char *)Phrases[myPhrase][myL]);
//       ST7735_OutChar(13);
//     }
//   }
//   Clock_Delay1ms(3000);
//   ST7735_FillScreen(0x0000); // set screen to black
//   l = 128;
//   while (1) {
//     Clock_Delay1ms(2000);
//     for (int j = 0; j < 3; j++) {
//       for (int i = 0; i < 16; i++) {
//         ST7735_SetCursor(7 * j + 0, i);
//         ST7735_OutUDec(l);
//         ST7735_OutChar(' ');
//         ST7735_OutChar(' ');
//         ST7735_SetCursor(7 * j + 4, i);
//         ST7735_OutChar(l);
//         l++;
//       }
//     }
//   }
// }

// // use main2 to observe graphics
// int main2(void) { // main2
//   __disable_irq();
//   PLL_Init(); // set bus speed
//   LaunchPad_Init();
//   GFX_Init();
//   ST7735_DrawBitmap(22, 159, PlayerShip0, 18, 8); // player ship bottom
//   ST7735_DrawBitmap(53, 151, Bunker0, 18, 5);
//   ST7735_DrawBitmap(42, 159, PlayerShip1, 18, 8); // player ship bottom
//   ST7735_DrawBitmap(62, 159, PlayerShip2, 18, 8); // player ship bottom
//   ST7735_DrawBitmap(82, 159, PlayerShip3, 18, 8); // player ship bottom
//   ST7735_DrawBitmap(0, 9, SmallEnemy10pointA, 16, 10);
//   ST7735_DrawBitmap(20, 9, SmallEnemy10pointB, 16, 10);
//   ST7735_DrawBitmap(40, 9, SmallEnemy20pointA, 16, 10);
//   ST7735_DrawBitmap(60, 9, SmallEnemy20pointB, 16, 10);
//   ST7735_DrawBitmap(80, 9, SmallEnemy30pointA, 16, 10);

//   for (uint32_t t = 500; t > 0; t = t - 5) {
//     SmallFont_OutVertical(t, 104, 6); // top left
//     Clock_Delay1ms(50);               // delay 50 msec
//   }
//   ST7735_FillScreen(0x0000); // set screen to black
//   ST7735_SetCursor(1, 1);
//   ST7735_OutString((char *)"GAME OVER");
//   ST7735_SetCursor(1, 2);
//   ST7735_OutString((char *)"Nice try,");
//   ST7735_SetCursor(1, 3);
//   ST7735_OutString((char *)"Earthling!");
//   ST7735_SetCursor(2, 4);
//   ST7735_OutUDec(1234);
//   while (1) {
//   }
// }

// // use main3 to test switches and LEDs
// int main3(void) { // main3
//   __disable_irq();
//   PLL_Init(); // set bus speed
//   LaunchPad_Init();
//   Switch_Init(); // initialize switches
//   LED_Init();    // initialize LED
//   while (1) {
//     // write code to test switches and LEDs
//   }
// }
// // use main4 to test sound outputs
// int main4(void) {
//   uint32_t last = 0, now;
//   __disable_irq();
//   PLL_Init(); // set bus speed
//   LaunchPad_Init();
//   Switch_Init();          // initialize switches
//   LED_Init();             // initialize LED
//   Sound_Init();           // initialize sound
//   TExaS_Init(ADC0, 6, 0); // ADC1 channel 6 is PB20, TExaS scope
//   __enable_irq();
//   while (1) {
//     now = Switch_In(); // one of your buttons
//     if ((last == 0) && (now == 1)) {
//       Sound_Shoot(); // call one of your sounds
//     }
//     if ((last == 0) && (now == 2)) {
//       Sound_Killed(); // call one of your sounds
//     }
//     if ((last == 0) && (now == 4)) {
//       Sound_Explosion(); // call one of your sounds
//     }
//     if ((last == 0) && (now == 8)) {
//       Sound_Fastinvader1(); // call one of your sounds
//     }
//     // modify this to test all your sounds
//   }
// }
// // ALL ST7735 OUTPUT MUST OCCUR IN MAIN
// int main5(void) { // final main
//   __disable_irq();
//   PLL_Init(); // set bus speed
//   LaunchPad_Init();
//   ST7735_InitPrintf(
//       INITR_REDTAB); // INITR_REDTAB for AdaFruit, INITR_BLACKTAB for HiLetGo
//   ST7735_FillScreen(ST7735_BLACK);
//   Sensor.Init(); // PB18 = ADC1 channel 5, slidepot
//   Switch_Init(); // initialize switches
//   LED_Init();    // initialize LED
//   Sound_Init();  // initialize sound
//   TExaS_Init(0, 0, &TExaS_LaunchPadLogicPB27PB26); // PB27 and PB26
//                                                    // initialize interrupts
//                                                    on
//                                                    // TimerG12 at 30 Hz

//   // initialize all data structures
//   __enable_irq();

//   while (1) {
//     // wait for semaphore
//     // clear semaphore
//     // update ST7735R
//     // check for end game or level switch
//   }
// }
