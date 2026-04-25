// Sound.cpp
// Duel Tanks
// SysTick runs at 11025 Hz, outputs one DAC sample per tick

#include <stdint.h>
#include <ti/devices/msp/msp.h>
#include "../DuelTanks/Inc/Sound.h"
#include "../DuelTanks/Inc/SoundData.h"
#include "../inc/DAC5.h"

static const uint8_t* soundPt = nullptr;
static volatile uint32_t soundCount = 0;

// ================================================================
// SysTick setup
// ================================================================
void SysTick_IntArm(uint32_t period, uint32_t priority) {
    SysTick->CTRL = 0;              // disable SysTick
    SysTick->LOAD = period - 1;     // reload value
    SysTick->VAL  = 0;              // reset current value

    NVIC_SetPriority(SysTick_IRQn, priority);

    SysTick->CTRL = 0x07;           // enable + interrupt + CPU clock
}

// ================================================================
// Sound_Init
// ================================================================
void Sound_Init(void) {
    DAC5_Init();

    soundPt = nullptr;
    soundCount = 0;

    DAC5_Out(15);                   // silence output

    SysTick_IntArm(7256, 2);        // 80MHz / 11025Hz
}

// ================================================================
// SysTick ISR
// ================================================================
extern "C" void SysTick_Handler(void) {

    if (soundCount > 0 && soundPt != nullptr) {
        DAC5_Out(*soundPt);
        soundPt++;
        soundCount--;
    } else {
        DAC5_Out(15);   // silence when no sound
        soundCount = 0;
    }
}

void Sound_Start(const uint8_t* pt, uint32_t count) {
    __disable_irq();

    soundPt = pt;
    soundCount = count;

    // restart SysTick cleanly
    SysTick->VAL = 0;
    SysTick->CTRL = 0x07;

    __enable_irq();
}

void Sound_Stop(void) {
    __disable_irq();

    soundPt = nullptr;
    soundCount = 0;

    SysTick->CTRL = 0;      // fully stop interrupt
    DAC5_Out(15);           // silence DAC

    __enable_irq();
}

bool Sound_IsPlaying(void) {
    return soundCount > 0;
}

void Sound_Shoot(void) {
    Sound_Start(Sound_ShootData, Sound_ShootLen);
}

void Sound_Killed(void) {
    Sound_Start(Sound_DefeatData, Sound_DefeatLen);
}

void Sound_Explosion(void) {
    Sound_Start(Sound_ExplosionData, Sound_ExplosionLen);
}

void Sound_Barrier(void) {
    if (Sound_IsPlaying()) return;
    Sound_Start(iso_shield, Sound_iso_shield);
}

void Sound_Victory(void) {
    Sound_Start(Sound_VictoryData, Sound_VictoryLen);
}

void Sound_Defeat(void) {
    Sound_Start(Sound_DefeatData, Sound_DefeatLen);
}

void Sound_Beep(uint8_t pitch) {
    if (Sound_IsPlaying()) return;

    switch (pitch) {
        case 0:  Sound_Start(Sound_BeepLowData,  Sound_BeepLowLen);  break;
        case 1:  Sound_Start(Sound_BeepMidData,  Sound_BeepMidLen);  break;
        default: Sound_Start(Sound_BeepHighData, Sound_BeepHighLen); break;
    }
}

void Sound_Fight(void) {
    Sound_Start(Sound_FightData, Sound_FightLen);
}

void Sound_Click(void) {
    if (Sound_IsPlaying()) return;
    Sound_Start(Sound_ClickData, Sound_ClickLen);
}

void Sound_Incoming(void) {
    if (Sound_IsPlaying()) return;
    Sound_Start(Sound_IncomingData, Sound_IncomingLen);
}

// old shi
void Sound_Fastinvader1(void) { Sound_Click(); }
void Sound_Fastinvader2(void) { Sound_Click(); }
void Sound_Fastinvader3(void) { Sound_Click(); }
void Sound_Fastinvader4(void) { Sound_Click(); }
void Sound_Highpitch(void)    { Sound_Beep(2); }