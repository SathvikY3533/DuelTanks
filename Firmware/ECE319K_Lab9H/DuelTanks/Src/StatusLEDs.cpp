// Switch.h
// Duel Tanks - LED driver

// ECE319H Spring 2026

#ifndef STATUS_LEDS_H
#define STATUS_LEDS_H

#include <stdint.h>
#include <stdbool.h>
#include "../DuelTanks/Inc/StatusLEDs.h"
#include <ti/devices/msp/msp.h>
#include "../inc/LaunchPad.h"


#define LED_RX   (1U << 16)  // PA16
#define LED_TX   (1U << 15)  // PA15
#define X_LED1   (1U << 24)  // PA24
#define X_LED2   (1U << 18)  // PA18
#define X_LED3   (1U << 19)  // PB19
#define X_LED4   (1U << 13)  // PB13


void StatusLEDs_Init(void) {

    // --- Port A LEDs ---
    IOMUX->SECCFG.PINCM[PA16INDEX] = 0x00000081;
    IOMUX->SECCFG.PINCM[PA15INDEX] = 0x00000081;
    IOMUX->SECCFG.PINCM[PA24INDEX] = 0x00000081;
    IOMUX->SECCFG.PINCM[PA18INDEX] = 0x00000081;

    GPIOA->DOE31_0 |= (LED_RX | LED_TX | X_LED1 | X_LED2);

    // Set LOW
    GPIOA->DOUT31_0 &= ~(LED_RX | LED_TX | X_LED1 | X_LED2);


    // --- Port B LEDs ---
    IOMUX->SECCFG.PINCM[PB19INDEX] = 0x00000081;
    IOMUX->SECCFG.PINCM[PB13INDEX] = 0x00000081;

    GPIOB->DOE31_0 |= (X_LED3 | X_LED4);

    // Set LOW
    GPIOB->DOUT31_0 &= ~(X_LED3 | X_LED4);
}

void led_toggle(uint32_t led_mask) {

    uint32_t portA_mask = led_mask & (LED_RX | LED_TX | X_LED1 | X_LED2);
    uint32_t portB_mask = led_mask & (X_LED3 | X_LED4);

    if (portA_mask) {
        GPIOA->DOUT31_0 ^= portA_mask;
    }

    if (portB_mask) {
        GPIOB->DOUT31_0 ^= portB_mask;
    }
}

#endif
