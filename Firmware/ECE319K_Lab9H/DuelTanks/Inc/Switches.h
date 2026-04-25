// Switch.h
// Duel Tanks - Layer 2: Button driver
// Pins from schematic:
//   SW1 Shield → PB12  (pull-down, pressed = HIGH)
//   SW2 Fire   → PB17  (pull-down, pressed = HIGH)
//   SW3 Left   → PB16  (pull-down, pressed = HIGH)
//   SW4 Right  → PA27  (pull-down, pressed = HIGH)
// ECE319H Spring 2026

#ifndef SWITCH_H
#define SWITCH_H

#include <stdint.h>
#include <stdbool.h>

#define SW_FIRE     (1U << 0)   // PB17
#define SW_SHIELD   (1U << 1)   // PB12
#define SW_LEFT     (1U << 2)   // PB16
#define SW_RIGHT    (1U << 3)   // PA27

// Initialize all 4 button GPIO pins as digital inputs
// Call once at startup before Switch_In()
void Switch_Init(void);

// Read all 4 buttons simultaneously
// Returns bitmask: bit0=Fire, bit1=Shield, bit2=Left, bit3=Right
// 1 = pressed (HIGH), 0 = not pressed
uint8_t Switch_In(void);

// Call once per game tick to update internal edge state
// high to low vs low to high
void Switch_Tick(void);

// Returns true ONCE on the frame the button was first pressed
bool Switch_Pressed(uint8_t mask);

// Returns true while button is currently held down
bool Switch_Held(uint8_t mask);

// Returns true ONCE on the frame the button was released
bool Switch_Released(uint8_t mask);

// How many consecutive ticks has this button been held?
// Useful for Artillery charged shot mechanic
uint8_t Switch_HeldTicks(uint8_t mask);  // max 255 ticks (~8.5 sec at 30Hz)

// wait for any button to be pressed (busy-wait)
uint8_t Switch_WaitForAny(void);

#endif
