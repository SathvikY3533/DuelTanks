// GameUART.h
// Duel Tanks — board-to-board UART packet protocol
// Uses UART1: PA8 = Tx out, PA9 = Rx in
// 3-byte packets, 115200 baud
// ECE319H Spring 2026

#ifndef GAMEUART_H
#define GAMEUART_H

#include <stdint.h>
#include <stdbool.h>
#include "../../ECE319K_Lab8/UART1.h"
#include "../../ECE319K_Lab8/UART2.h"

#define PKT_HEADER1  0xAA
#define PKT_HEADER2  0x55

//Payload being sent - after the 0xAA and 0x55 start bytes
#define PKT_FIRE        0xA1  // shell crossed screen edge
#define PKT_HIT         0xB1  // shell landed on opponent
#define PKT_READY       0xC0  // pre-match sync
#define PKT_WIN         0xD1  // game over
#define PKT_LANG        0xE1  // language selection sync
#define PKT_TANK        0xF1  // tank selection
#define PKT_REMATCH     0xF2  // rematch yes/no

//Payload helper struct
typedef struct {
    uint8_t type;   // PKT_xxx constant above
    uint8_t data1;  // meaning depends on type
    uint8_t data2;  // meaning depends on type
} GamePacket_t;

// Init both TX and RX on both baords
void GameUART_Init(void);

// ------------ Helper funcs to pack payload with inputs and send --------------

void GameUART_SendFire(uint8_t y_entry, uint8_t vx_enc, uint8_t vy_enc);

void GameUART_SendHit(uint8_t livesRemaining);

void GameUART_SendReady(void);

void GameUART_SendWin(uint8_t winnerId);

void GameUART_SendLang(uint8_t lang);

void GameUART_SendTank(uint8_t tankId);

void GameUART_SendRematch(uint8_t vote);

// RX polling hardware mailbox to push data into RX FIFO :)
bool GameUART_Receive(GamePacket_t* pkt);

// check RX UART1 FIFO for any data pushed from the RX polling
bool GameUART_ByteAvailable(void);

//read data!!!!
uint8_t GameUART_ReadByte(void);

/**
 * Output 8-bit to serial port UART1<br>
 * Uses no synchronization<br>
 * This function does not wait, if the transmitter is busy, data will be lost
 * @param data is an 8-bit ASCII character to be transferred
 * @return none
 * @brief output character to UART1
 */
void UART1_OutChar(char data);

#endif // GAMEUART_H
