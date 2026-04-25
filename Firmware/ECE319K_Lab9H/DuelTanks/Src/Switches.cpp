// Switch.cpp
// Duel Tanks - Layer 2: Button driver implementation
// ECE319H Spring 2026

#include "../DuelTanks/Inc/Switches.h"
#include <ti/devices/msp/msp.h>
#include "../inc/LaunchPad.h"

static uint8_t sw_current = 0;  // raw reading this tick
static uint8_t sw_previous = 0; // raw reading last tick
static uint8_t sw_rose = 0;     // buttons that went 0→1 this tick
static uint8_t sw_fell = 0;     // buttons that went 1→0 this tick
static uint8_t sw_held_ticks[4] = {0, 0, 0, 0}; // hold duration per button


static const uint8_t BIT[4] = {SW_FIRE, SW_SHIELD, SW_LEFT, SW_RIGHT};

void Switch_Init(void) {

  // --- PB12 (Shield / SW1) ---
  // --- PB16 (Left / SW3) ---
  // --- PB17 (Fire / SW2) ---
  // --- PA27 (Right / SW4) ---

  IOMUX->SECCFG.PINCM[PB12INDEX] = 0x00040081;
  IOMUX->SECCFG.PINCM[PB16INDEX] = 0x00040081;
  IOMUX->SECCFG.PINCM[PB17INDEX] = 0x00040081;
  IOMUX->SECCFG.PINCM[PA27INDEX] = 0x00040081;

  GPIOB->DOE31_0 &= ~(SW_FIRE | SW_SHIELD | SW_LEFT);
  GPIOA->DOE31_0 &= ~SW_RIGHT;

  // Clear all edge state
  sw_current = 0;
  sw_previous = 0;
  sw_rose = 0;
  sw_fell = 0;
  for (int i = 0; i < 4; i++)
    sw_held_ticks[i] = 0;
}

uint8_t Switch_In(void) {
  uint8_t result = 0;

  // Fire   — PB17
  // Shield — PB12
  // Left   — PB16
  // Right  — PA27

  result =
      ((GPIOB->DIN31_0 >> 17) & 0x1) | (((GPIOB->DIN31_0 >> 12) & 1) << 1) |
      (((GPIOB->DIN31_0 >> 16) & 1) << 2) | (((GPIOA->DIN31_0 >> 27) & 1) << 3);

  return result;
}

// ================================================================
// Switch_Tick
// Call ONCE per game tick (in TIMG12_IRQHandler, before game logic)
// Updates edge detection state
// ================================================================
void Switch_Tick(void) {
  sw_previous = sw_current;
  sw_current = Switch_In();

  sw_rose = (sw_current) & (~sw_previous); // bits that went 0→1
  sw_fell = (~sw_current) & (sw_previous); // bits that went 1→0

  // Update hold tick counters
  for (int i = 0; i < 4; i++) {
    if (sw_current & BIT[i]) {
      // Button held — increment counter (cap at 255)
      if (sw_held_ticks[i] < 255)
        sw_held_ticks[i]++;
    } else {
      // Button released — reset counter
      sw_held_ticks[i] = 0;
    }
  }
}


bool Switch_Pressed(uint8_t mask) { return (sw_rose & mask) != 0; }

bool Switch_Held(uint8_t mask) { return (sw_current & mask) != 0; }

bool Switch_Released(uint8_t mask) { return (sw_fell & mask) != 0; }

uint8_t Switch_HeldTicks(uint8_t mask) {
  // Return the tick count for the lowest set bit in mask
  for (int i = 0; i < 4; i++) {
    if (mask & BIT[i])
      return sw_held_ticks[i];
  }
  return 0;
}
   
uint8_t Switch_WaitForAny(void) {
  uint8_t prev = Switch_In();
  uint8_t curr;
  while (1) {
    // Small delay between polls to avoid hammering GPIO
    for (volatile uint32_t d = 0; d < 4000; d++)
      ;
    curr = Switch_In();
    // Look for rising edge (newly pressed)
    uint8_t pressed = curr & ~prev;
    if (pressed)
      return pressed;
    prev = curr;
  }
}
