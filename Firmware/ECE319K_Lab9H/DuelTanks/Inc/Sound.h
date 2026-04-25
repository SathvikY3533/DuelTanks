// Sound.h
// Duel Tanks - Sound driver
// ECE319H Spring 2026

#ifndef SOUND_H
#define SOUND_H

#include <stdint.h>
#include <stdbool.h>

// Initialize DAC and SysTick at 11025 Hz — call once at startup
void Sound_Init(void);

// Start playing an arbitrary sample array (non-blocking)
// Sound plays once and stops — calling again interrupts current sound
void Sound_Start(const uint8_t* pt, uint32_t count);

// Stop immediately and silence output
void Sound_Stop(void);

// True while a sound is currently playing
bool Sound_IsPlaying(void);

void Sound_Shoot(void);       // shell fired
void Sound_Barrier(void);     // barrier placed
void Sound_Explosion(void);   // tank hit — highest priority
void Sound_Victory(void);     // win screen
void Sound_Defeat(void);      // lose screen
void Sound_Beep(uint8_t pitch); // 0=low 1=mid 2=high (countdown)
void Sound_Fight(void);       // FIGHT! screen
void Sound_Click(void);       // menu navigation
void Sound_Incoming(void);    // UART shell arriving

// from valvano code - maybe use?? idek
void Sound_Killed(void);
void Sound_Fastinvader1(void);
void Sound_Fastinvader2(void);
void Sound_Fastinvader3(void);
void Sound_Fastinvader4(void);
void Sound_Highpitch(void);

#endif // SOUND_H
