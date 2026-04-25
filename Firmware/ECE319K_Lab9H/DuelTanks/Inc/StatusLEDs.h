// Switch.h
// Duel Tanks - LED driver

// ECE319H Spring 2026

#ifndef STATUS_LEDS_H
#define STATUS_LEDS_H

#include <stdint.h>
#include <stdbool.h>
#include "../inc/LaunchPad.h"


#define LED_RX   (1U << 16)  // PA16
#define LED_TX   (1U << 15)  // PA15
#define X_LED1   (1U << 24)  // PA24
#define X_LED2   (1U << 18)  // PA18
#define X_LED3   (1U << 19)  // PB19
#define X_LED4   (1U << 13)  // PB13

void StatusLEDs_Init(void);

void led_toggle(uint32_t led_mask);

#endif
