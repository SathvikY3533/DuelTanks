// Lab8Main.c
// Runs on MSPM0G3507
// Lab 8
// Your name
// Last Modified: January 12, 2026
// PA8 GPIO output to IR LED, to other microcontroller IR sensor, PA22 UART2 Rx
// Spring 2026: message format {0x3C,digit1,digit2,digit3}
//   where each digit is an ASCII character '0' to '9'
#include <stdio.h>
#include <stdint.h>
#include <ti/devices/msp/msp.h>
#include "../inc/ST7735.h"
#include "../inc/Clock.h"
#include "../inc/LaunchPad.h"
#include "../inc/Timer.h"
#include "../inc/SlidePot.h"
#include "../inc/FIFO2.h"
#include "IRxmt.h"
#include "UART2.h"
extern "C" void __disable_irq(void);
extern "C" void __enable_irq(void);
extern "C" void TIMG12_IRQHandler(void);
// ****note to students****
// the data sheet says the ADC does not work when clock is 80 MHz
// however, the ADC seems to work on my boards at 80 MHz
// I suggest you try 80MHz, but if it doesn't work, switch to 40MHz
void PLL_Init(void){ // set phase lock loop (PLL)
  // Clock_Init40MHz(); // run this line for 40MHz
  Clock_Init80MHz(0);   // run this line for 80MHz
}
uint32_t ReceiveCount,TransmitCount;
uint32_t Position; // fixed point 0.001cm, in receiver
uint32_t Data;     // 12-bit raw ADC data in transmitter

char TxPayload[5];

SlidePot Sensor(1500,0); // copy calibration from Lab 7

// Implement the FIFO class
//   FIFO2.h is prototype
//   FIFO2.cpp is implementation
// run main1R program on receiver to test FIFO2
// use real scope on PB27 to measure execution time of Put
// use real scope on PB26 to measure execution time of Get
// Each FIFO.print should be 10 sequential characters
// err should be 0
Queue FIFO;
int err,count, y;
int main1R(void){ // main1R
  char data = 0; char out;
  bool result;
  __disable_irq();
  PLL_Init();     // set system clock to 80 MHz
  LaunchPad_Init();
  ST7735_InitPrintf(INITR_REDTAB); // INITR_REDTAB for AdaFruit, INITR_BLACKTAB for HiLetGo
  y = 0;
  while(1){
    ST7735_SetCursor(0, y);
    count=0;
    for(int i=0; i<10; i++){
      GPIOB->DOUTSET31_0 = GREEN;
      result = FIFO.Put(data);
      GPIOB->DOUTCLR31_0 = GREEN;
      if(result){
        count++;
        data++;
      }else{
        err++;
      }
    }
    FIFO.Print();
    for(int i=0; i<count; i++){
      GPIOB->DOUTSET31_0 = RED;
      result = FIFO.Get(&out);
      GPIOB->DOUTCLR31_0 = RED;
      if(!result){
        err++;
      }
    }
    Clock_Delay1ms(1000);
    y += 1;
  }
}
// Implement IRxmt_Init and IRxmt_SendBit
//   IRxmt.h is prototype
//   IRxmt.cpp is implementation
// run main1T on transmitter to test IRxmt_SendBit(0);
// Connect scope to PA8 on transmitter
//   verify continuous wave is 38 kHz
//   low time is about 13.158us
//   high time is about 13.158us
int main1T(void){ // main1T
  __disable_irq();
  PLL_Init();     // set system clock to 80 MHz
  LaunchPad_Init();
  IRxmt_Init();    // initialize transmitter, baud rate is 2375 bits/sec
  while(1){
    IRxmt_SendBit(0); // continuous 38 kHz squarewave
  }
}

// Implement IRxmt_Init and IRxmt_SendBit
//   IRxmt.h is prototype
//   IRxmt.cpp is implementation
// run main2T on transmitter to test low-level IR transmission
// Connect scope to PA8 on transmitter
//   verify wave is 38 kHz
//   verify there are 16 pulses when sending 0
//   verify each bit time is 421.05us, both 0 and 1
int main2T(void){ // main2T
  __disable_irq();
  PLL_Init();     // set system clock to 80 MHz
  LaunchPad_Init();
  IRxmt_Init();    // initialize transmitter, baud rate is 2375 bits/sec
  while(1){
    IRxmt_SendBit(1);
    IRxmt_SendBit(0);
  }
}

// Implement IRtranmission_OutChar
// Implement UART2_Init and UART2_InChar
// interface IR LED using ULN2003B on transmitter
// interface IR sensor on receiver
// Connect dual channel scope to transmitter sensor PA8, sensor output, receiver PA22
//   run main3T on transmitter
//   run main3R on receiver
//   verify pulses on transmitter convert to UART protocol on sensor output, receiver PA22
//   verify waveforms similar to Figure 8.9
int main3T(void){ // main3T
  __disable_irq();
  PLL_Init();     // set system clock to 80 MHz
  LaunchPad_Init();
  IRxmt_Init();    // initialize transmitter, baud rate is 2375 bits/sec
  while(1){ // sends any character over and over
    IRxmt_OutChar(0x5A);
    Clock_Delay1ms(500); // about 2Hz
  }
}

char data1,data2,data3,data4;
int main3R(void){ // main3R
  __disable_irq();
  PLL_Init();   // set bus speed
  LaunchPad_Init();
  ST7735_InitPrintf(INITR_REDTAB); // INITR_REDTAB for AdaFruit, INITR_BLACKTAB for HiLetGo
  UART2_Init(); // just receive, PA22, receiver timeout interrupt synchronization
  __enable_irq();       // interrupts for UART2
  while(1){ // receive a single character
    data1 = UART2_InChar(); // should match main3T
    ST7735_OutChar(data1);
    ST7735_OutChar('\n');
  }
}


// Connect dual channel scope to transmitter sensor PA8, sensor output, receiver PA22
//   run main4T on transmitter
//   run main4R on receiver, start receiver first
//   verify pulses on transmitter convert to UART protocol on sensor output, receiver PA22
int main4T(void){ // main4T
  __disable_irq();
  PLL_Init();     // set system clock to 80 MHz
  LaunchPad_Init();
  IRxmt_Init();    // initialize transmitter, baud rate is 2375 bits/sec
  while(1){ // message is 1.547 cm
    IRxmt_OutChar('<'); // 0x3C
    IRxmt_OutChar('1'); // 0x31
    IRxmt_OutChar('5'); // 0x35
    IRxmt_OutChar('4'); // 0x34
    Clock_Delay1ms(500); // about 2Hz
  }
}
int main4R(void){ // main4R
  __disable_irq();
  PLL_Init();     // set bus speed
  LaunchPad_Init();
  ST7735_InitPrintf(INITR_REDTAB); // INITR_REDTAB for AdaFruit, INITR_BLACKTAB for HiLetGo
  UART2_Init();   // just receive, PA22, receiver timeout interrupt synchronization
  __enable_irq(); // interrupts for UART2
  while(1){ // message is 1.234 cm
    data1 = UART2_InChar(); // should be 0x3C
    data2 = UART2_InChar(); // should be 31
    ST7735_OutChar(data2);
    ST7735_OutChar('.');    // implied
    data3 = UART2_InChar(); // should be 35
    ST7735_OutChar(data3);
    data4 = UART2_InChar(); // should be 34
    ST7735_OutChar(data4);
    ST7735_OutChar('\n');
  }
}


uint32_t dist = 1234;

// sampling frequency is 30 Hz
// should take 4*10*421.06us = 16.84ms
void TIMG12_IRQHandler(void){
  uint32_t pos;
  if((TIMG12->CPU_INT.IIDX) == 1){ // this will acknowledge
    GPIOB->DOUTTGL31_0 = GREEN; // toggle PB27 (minimally intrusive debugging)
    TransmitCount++; // increment TransmitCount
    pos = Sensor.In(); // sample 12-bit ADC0 channel 5, slidepot
    GPIOB->DOUTTGL31_0 = GREEN; // toggle PB27 (minimally intrusive debugging)
    dist = Sensor.Convert(pos); // convert to fixed point distance

    TxPayload[0] = '<';
    TxPayload[1] = '0' + dist/1000;
    dist %= 1000;
    TxPayload[2] = '0' + (dist)/100;
    dist %= 100;
    TxPayload[3] = '0' + dist/10;

    IRxmt_OutChar(TxPayload[0]);  // '<'
    IRxmt_OutChar(TxPayload[1]);  // '1'
    IRxmt_OutChar(TxPayload[2]);  // '2'
    IRxmt_OutChar(TxPayload[3]);  // '3'
    // IRxmt_OutChar(TxPayload[4]);


    GPIOB->DOUTTGL31_0 = GREEN; // toggle PB27 (minimally intrusive debugging)
  }
}

// Connect dual channel scope to transmitter sensor PA8, sensor output, receiver PA22
//   run main5T on transmitter
//   run main5R on receiver, should work regardless of which is started first
//   verify pulses on transmitter convert to UART protocol on sensor output, receiver PA22
// Data should go from 0 to 4095 in transmitter
// Position should go from 0 to 2000 in receiver
// LCD should show 0.000cm to 2.000 cm
int maint(void){ // main5T
  __disable_irq();
  PLL_Init(); // set bus speed
  LaunchPad_Init();
  TransmitCount=0;
  Sensor.Init(); //PB18 = ADC1 channel 5, slidepot
  IRxmt_Init(); // just transmit, PA8, blind synchronization
  TimerG12_Init();
  TimerG12_IntArm(80000000/30, 3); // initialize interrupts on TimerG12 at 30 Hz
  
  __enable_irq();

  while(1){
	  // nothing to do here
  }
}

char b0, b1, b2, b3, b4;

int main(void){ // main5R
  __disable_irq();
  PLL_Init(); // set bus speed
  LaunchPad_Init();
  ReceiveCount=0;
  UART2_Init(); // just receive, PA22, receiver timeout synchronization
  ST7735_InitPrintf(INITR_REDTAB); // INITR_REDTAB for AdaFruit, INITR_BLACKTAB for HiLetGo
  ST7735_FillScreen(ST7735_BLACK);
  ST7735_PlotClear(0,2000);
  __enable_irq();

  while(1){
    uint32_t PositionReceive;
    ST7735_SetCursor(0, 0);  // move cursor to top left
    do {
        b0 = UART2_InChar();
    } while(b0 != '<');// wait for first frame

    ReceiveCount++; // increment ReceiveCount

    GPIOB->DOUTTGL31_0 = RED; // toggle PB26 (minimally intrusive debugging)
    
    b1 = UART2_InChar(); // receive next three bytes of message
    b2 = UART2_InChar();
    b3 = UART2_InChar();
    b4 = UART2_InChar();
    
    __disable_irq();
    PositionReceive = (b1 - '0')*1000 + (b2-'0')*100 + (b3-'0')*10 + (b4-'0');  // calculate PositionReceive from the message

    ST7735_OutUDec(PositionReceive/1000);        // ones
    ST7735_OutChar('.');
    PositionReceive %= 1000;
    ST7735_OutUDec(PositionReceive/100);    // tenths
    PositionReceive %= 100;
    ST7735_OutUDec(PositionReceive/10);         // hundredths
    PositionReceive %= 10;
    ST7735_OutUDec(PositionReceive);  
    ST7735_OutString(" cm");
    
    if((ReceiveCount%15)==0){
      ST7735_PlotPoint(PositionReceive);
      ST7735_PlotNextErase(); // data plotted at about 2 Hz
    }
    __enable_irq();
  }
}




