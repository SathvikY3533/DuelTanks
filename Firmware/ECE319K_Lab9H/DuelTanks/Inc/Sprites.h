// Sprites.h
#ifndef SPRITES_H
#define SPRITES_H
#include <stdint.h>

extern const uint16_t Tank1Right[288];
#define TANK1RIGHT_W 24
#define TANK1RIGHT_H 12

extern const uint16_t Tank1Left[288];
#define TANK1LEFT_W 24
#define TANK1LEFT_H 12

extern const uint16_t Tank2Right[288];
#define TANK2RIGHT_W 24
#define TANK2RIGHT_H 12

extern const uint16_t Tank2Left[288];
#define TANK2LEFT_W 24
#define TANK2LEFT_H 12

extern const uint16_t Tank3Right[288];
#define TANK3RIGHT_W 24
#define TANK3RIGHT_H 12

extern const uint16_t Tank3Left[288];
#define TANK3LEFT_W 24
#define TANK3LEFT_H 12

extern const uint16_t MissileRight[50];
#define MISSILERIGHT_W 10
#define MISSILERIGHT_H 5

extern const uint16_t MissileLeft[50];
#define MISSILELEFT_W 10
#define MISSILELEFT_H 5

extern const uint16_t ExpF0[196];
#define EXPF0_W 14
#define EXPF0_H 14

extern const uint16_t ExpF1[196];
#define EXPF1_W 14
#define EXPF1_H 14

extern const uint16_t ExpF2[196];
#define EXPF2_W 14
#define EXPF2_H 14

extern const uint16_t ExpF3[196];
#define EXPF3_W 14
#define EXPF3_H 14

extern const uint16_t HeartFull[64];
#define HEARTFULL_W 8
#define HEARTFULL_H 8

extern const uint16_t HeartEmpty[64];
#define HEARTEMPTY_W 8
#define HEARTEMPTY_H 8

extern const uint16_t* const TankSprites[3][2];
extern const uint16_t* const ExpFrames[4];
extern const uint8_t TerrainY[160];
#endif