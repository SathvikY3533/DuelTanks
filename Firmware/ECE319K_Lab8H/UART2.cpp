/* UART2.cpp
 * sathvik Yechuri
 * Data: 4/8/26
 * PA22 UART2 Rx from other microcontroller PA8 IR output<br>
 */


#include <ti/devices/msp/msp.h>
#include "UART2.h"
#include "../inc/Clock.h"
#include "../inc/LaunchPad.h"
#include "../inc/FIFO2.h"

uint32_t LostData;
uint32_t temp;
Queue FIFO2;

// power Domain PD0
// for 80MHz bus clock, UART2 clock is ULPCLK 40MHz
// initialize UART2 for 2375 baud rate
// no transmit, interrupt on receive timeout
void UART2_Init(void){
  UART2->GPRCM.RSTCTL = 0xB1000003;  // reset UART2
  UART2->GPRCM.PWREN  = 0x26000001;  // activate power for UART2
  Clock_Delay(24); // time for UART to power up

  // PA22 = RX
  IOMUX->SECCFG.PINCM[PA22INDEX] = 0x00040082; // INENA, PC=1, Mode=2

  UART2->CLKSEL = 0x08;
  UART2->CLKDIV = 0x00;

  UART2->CTL0 &= ~0x01;          // disable UART2
  UART2->CTL0  = 0x00020018;     // enable FEN, Rx only (no Tx bit)

  // 40MHz / (16 * 2375) = 1054.21
  // IBRD = 1054, FBRD = round(0.21 * 64) = 13
  UART2->IBRD = 1052; //1054
  UART2->FBRD = 40;   //11

  UART2->LCRH = 0x00000030;  // 8-bit, 1 stop, no parity

  // interrupt on receive timeout only (bit 0 = RTOUT)
  UART2->CPU_INT.IMASK = 0x0001;
  UART2->IFLS = 0x0400;

  // UART2 IRQ number — check your device header for the correct IRQ
  NVIC->ICPR[0] = 1 << UART2_INT_IRQn;
  NVIC->ISER[0] = 1 << UART2_INT_IRQn;
  NVIC->IP[3]   = (NVIC->IP[3] & (~0xFF000000)) | (2 << 23); // priority 2

  UART2->CTL0 |= 0x01;  // enable UART2
}

//------------UART2_InChar------------
// Get new serial port receive data from FIFO2
// Input: none
// Output: Return 0 if the FIFO2 is empty
//         Return nonzero data from the FIFO1 if available
// your version spins looking for nonzero, which is wrong
// 0 is a valid return meaning FIFO is empty, not a valid char
char UART2_InChar(void){
    char in;
    while(FIFO2.Get(&in) == false){} // spin until data available
    return in;
}

extern "C" void UART2_IRQHandler(void);
void UART2_IRQHandler(void){
    uint32_t status; char letter;
    status = UART2->CPU_INT.IIDX;
    if(status == 0x01 || status == 0x0B){
        GPIOB->DOUTTGL31_0 = BLUE; // heartbeat 1
        GPIOB->DOUTTGL31_0 = BLUE; // heartbeat 2

        // read while RXFE (bit 2) in STAT is 0 (data available)
        while((UART2->STAT & 0x04) == 0){
            letter = UART2->RXDATA;
            if(letter == 0x3C) {
              temp++;
            }
            if(!FIFO2.Put(letter)) LostData++; // FIFO full = error
        }
        // IIDX read already acknowledged, or:
        // UART2->ICR = 1;             // explicitly clear if needed

        GPIOB->DOUTTGL31_0 = BLUE; // heartbeat 3
    }
}
