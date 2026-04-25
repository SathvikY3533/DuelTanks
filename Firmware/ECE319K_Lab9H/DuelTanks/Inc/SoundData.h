// SoundData.h
// Duel Tanks - Layer 3: Sound sample arrays
// All arrays are 5-bit DAC values (0-31), sample rate 11025 Hz
// Generated synthetically — replace with .wav-converted arrays later if desired
// ECE319H Spring 2026

#ifndef SOUNDDATA_H
#define SOUNDDATA_H

#include <stdint.h>

// ================================================================
// Helper macro for sample count
// ================================================================
#define SOUND_LEN(arr) (sizeof(arr) / sizeof((arr)[0]))

// shooting sound
extern const uint8_t Sound_ShootData[];
extern const uint32_t Sound_ShootLen;

//ability sound
extern const unsigned char iso_shield[];
extern const uint32_t Sound_iso_shield;

// explosion sound
extern const uint8_t Sound_ExplosionData[];
extern const uint32_t Sound_ExplosionLen;

// victory!!!
extern const uint8_t Sound_VictoryData[];
extern const uint32_t Sound_VictoryLen;

// you lose
extern const uint8_t Sound_DefeatData[];
extern const uint32_t Sound_DefeatLen;

// beep low
extern const uint8_t Sound_BeepLowData[];
extern const uint32_t Sound_BeepLowLen;

// beep mid
extern const uint8_t Sound_BeepMidData[];
extern const uint32_t Sound_BeepMidLen;

// beep high
extern const uint8_t Sound_BeepHighData[];
extern const uint32_t Sound_BeepHighLen;

// fight
extern const uint8_t Sound_FightData[];
extern const uint32_t Sound_FightLen;

// click sound fx
extern const uint8_t Sound_ClickData[];
extern const uint32_t Sound_ClickLen;

// projectile incoming
extern const uint8_t Sound_IncomingData[];
extern const uint32_t Sound_IncomingLen;

#endif // SOUNDDATA_H
