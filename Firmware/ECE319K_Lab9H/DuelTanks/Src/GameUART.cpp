// GameUART.cpp
// Duel Tanks — board-to-board UART packet protocol
// PA8  = UART1 Tx (out to other board's PA22)
// PA22 = UART2 Rx (in from other board's PA8)
// 115200 baud, 3-byte packets, polling RX (no IRQ)
// ECE319H Spring 2026

#include "../DuelTanks/Inc/GameUART.h"
#include <ti/devices/msp/msp.h>
#include "../inc/LaunchPad.h"
#include "../inc/Clock.h"
#include "../DuelTanks/Inc/StatusLEDs.h"

// RX buffer
#define RX_BUF_SIZE  64
#define RX_BUF_MASK  (RX_BUF_SIZE - 1)

static uint8_t  rxBuf[RX_BUF_SIZE];
static uint32_t rxHead = 0;
static uint32_t rxTail = 0;

// Packet assembly — 5 states for 5-byte packet
// [0xAA][0x55][type][data1][data2]
static uint8_t pktBuf[3];   // stores [type][d1][d2]
static uint8_t pktIdx = 0;  // 0=wait AA, 1=wait 55, 2=type, 3=d1, 4=d2
static bool msgRead = false;

// UART1 Init — PA8 TX only, 115200 baud
static void UART1_GameInit(void) {
    UART1->GPRCM.RSTCTL = 0xB1000003;
    UART1->GPRCM.PWREN  = 0x26000001;
    Clock_Delay1ms(24);

    // PA8 = UART1_TX, function=2, PC connected
    IOMUX->SECCFG.PINCM[PA8INDEX] = 0x00000082;

    UART1->CLKSEL        = 0x08;
    UART1->CLKDIV        = 0x00;
    UART1->CTL0         &= ~0x01;       // disable
    UART1->CTL0          = 0x00020018;  // FEN, 16x, TX+RX
    UART1->IBRD          = 21;          // 80MHz bus → ULPCLK=40MHz
    UART1->FBRD          = 45;          // 40M/16/21.703 = 115191 baud
    UART1->LCRH          = 0x00000030;  // 8N1
    UART1->CTL0         |= 0x01;        // enable
}

// UART2 Init — PA22 RX only, 115200 baud, no IRQ :()
static void UART2_GameInit(void) {
    UART2->GPRCM.RSTCTL = 0xB1000003;
    UART2->GPRCM.PWREN  = 0x26000001;
    Clock_Delay1ms(24);

    // PA22 = UART2_RX
    // bit 18 = INENA (input enable), bit 7 = PC connected
    IOMUX->SECCFG.PINCM[PA22INDEX] = 0x00040082;

    UART2->CLKSEL        = 0x08;
    UART2->CLKDIV        = 0x00;
    UART2->CTL0         &= ~0x01;
    UART2->CTL0          = 0x00020018;
    UART2->IBRD          = 21;
    UART2->FBRD          = 45;
    UART2->LCRH          = 0x00000030;
    UART2->CPU_INT.IMASK = 0x00;        // no interrupts
    UART2->CTL0         |= 0x01;
}

void GameUART_Init(void) {
    UART1_GameInit();
    UART2_GameInit();
    rxHead = 0;
    rxTail = 0;
    pktIdx = 0;
}

//helper funcs ---------------------------

static bool uart2Available(void) {
    return (UART2->STAT & 0x10) == 0;  // RXFE=0 means data waiting
}

static void rxPush(uint8_t b) {
    uint32_t next = (rxHead + 1) & RX_BUF_MASK;
    if (next != rxTail) {
        rxBuf[rxHead] = b;
        rxHead = next;
    }
}

static bool rxPop(uint8_t *b) {
    if (rxTail == rxHead) return false;
    *b = rxBuf[rxTail];
    rxTail = (rxTail + 1) & RX_BUF_MASK;
    return true;
}


bool GameUART_Receive(GamePacket_t *pkt) {
    uint8_t drain = 16;
    while (drain-- && uart2Available()) {
        rxPush((uint8_t)(UART2->RXDATA & 0xFF));
    }

    // Assemble packet — state machine
    // State: 0=need 0xAA, 1=need 0x55, 2=type, 3=d1, 4=d2
    uint8_t b;
    while (rxPop(&b)) {
        // If we see 0xAA at any point, restart — new packet beginning
        if (b == 0xAA && pktIdx != 0 && msgRead) {
            msgRead = false;
            pktIdx = 1;  // treat this 0xAA as the header start
            continue;
        }

        switch (pktIdx) {
            case 0:
                pktIdx = (b == 0xAA) ? 1 : 0;
                break;
            case 1:
                pktIdx = (b == 0x55) ? 2 : 0;
                break;
            case 2:
                pktBuf[0] = b;
                pktIdx = 3;
                break;
            case 3:
                pktBuf[1] = b;
                pktIdx = 4;
                break;
            case 4:
                pktBuf[2] = b;
                pktIdx = 0;
                pkt->type  = pktBuf[0];
                pkt->data1 = pktBuf[1];
                pkt->data2 = pktBuf[2];
                msgRead = true;
                led_toggle(LED_RX);
                return true;
        }
    }
    return false;
}

// sendPacket — 5 bytes: [0xAA][0x55][type][d1][d2]
static void sendPacket(uint8_t type, uint8_t d1, uint8_t d2) {
    while (UART1->STAT & 0x08); UART1->TXDATA = 0xAA;
    while (UART1->STAT & 0x08); UART1->TXDATA = 0x55;
    while (UART1->STAT & 0x08); UART1->TXDATA = type;
    while (UART1->STAT & 0x08); UART1->TXDATA = d1;
    while (UART1->STAT & 0x08); UART1->TXDATA = d2;
    led_toggle(LED_TX);
}

//functions used in main.cpp
void GameUART_SendFire(uint8_t y_entry, uint8_t vx_enc, uint8_t vy_enc) { sendPacket(PKT_FIRE, y_entry, (uint8_t)((vx_enc << 4) | (vy_enc & 0x0F))); }
void GameUART_SendHit    (uint8_t lives)                { sendPacket(PKT_HIT,     lives,    0x00);  }
void GameUART_SendReady  (void)                         { sendPacket(PKT_READY,   0x01,     0x00);  }
void GameUART_SendWin    (uint8_t winnerId)             { sendPacket(PKT_WIN,     winnerId, 0x00);  }
void GameUART_SendLang   (uint8_t lang)                 { sendPacket(PKT_LANG,    lang,     0x00);  }
void GameUART_SendTank   (uint8_t tankId)               { sendPacket(PKT_TANK,    tankId,   0x00);  }
void GameUART_SendRematch(uint8_t vote)                 { sendPacket(PKT_REMATCH, vote,     0x00);  }

void UART1_OutChar(char data) {
    while (UART1->STAT & 0x08);
    UART1->TXDATA = data;
}

// debugging stuff
bool    GameUART_ByteAvailable(void) { return uart2Available(); }
uint8_t GameUART_ReadByte(void)      { return (uint8_t)(UART2->RXDATA & 0xFF); }
