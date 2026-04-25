/* ECE319K_Lab4main.c
 * Traffic light FSM
 * ECE319H students must use pointers for next state
 * ECE319K students can use indices or pointers for next state
 * Put your names here or look silly
 Sathvik Yechuri
  */

#include <ti/devices/msp/msp.h>
#include "../inc/LaunchPad.h"
#include "../inc/Clock.h"
#include "../inc/UART.h"
#include "../inc/Timer.h"
#include "../inc/Dump.h"  // student's Lab 3
#include <stdio.h>
#include <string.h>
// put your EID in the next line
const char EID1[] = "sy24742"; //  ;replace abc123 with your EID
// Hint implement Traffic_Out before creating the struct, make struct match your Traffic_Out

#define RED_LIGHT   4
#define YELLOW_LIGHT 2
#define GREEN_LIGHT 1
#define WALK_RED (1<< 26)
#define WALK_WHITE ((1<<22) | (1 << 26) | (1 << 27))
#define WALK_OFF 0

#define ALL8(x) { x, x, x, x, x, x, x, x }

typedef struct State {
    uint32_t delay;
    uint32_t south;
    uint32_t west;
    uint32_t walk;
    uint32_t stateNum;
    const struct State *next[8];
} State;

//21 states
const State greenSouth, delayGreenSouth, yellowSouth, delayYellowSouth, redSouth, delayRedSouth;
const State greenWest, delayGreenWest, yellowWest, delayYellowWest, redWest, delayRedWest;
const State delayWalk, whiteWalk, delayFlashWalk, delayWalk5, delayRedWest2;
const State redWalk1, delayWalk1, offWalk1, delayWalk2, redWalk2, delayWalk3, offWalk2, delayWalk4, redWalk3;


const State greenSouth = {
  .delay = 1,
  .south = GREEN_LIGHT,
  .west  = RED_LIGHT,
  .walk  = WALK_RED,
  .stateNum = 0,
  .next = {
    &greenSouth,
    &delayGreenSouth,
    &delayGreenSouth,
    &delayGreenSouth,
    &greenSouth,
    &delayGreenSouth,
    &delayGreenSouth,
    &delayGreenSouth
  }
};

const State delayGreenSouth = {
  .delay = 1500,
  .south = GREEN_LIGHT,
  .west  = RED_LIGHT,
  .walk  = WALK_RED,
  .stateNum = 1,
  .next = ALL8(&yellowSouth)
};

const State yellowSouth = {
  .delay = 1,
  .south = YELLOW_LIGHT,
  .west  = RED_LIGHT,
  .walk  = WALK_RED,
  .stateNum = 2,
  .next = ALL8(&delayYellowSouth)
};

const State delayYellowSouth = {
  .delay = 1500,
  .south = YELLOW_LIGHT,
  .west  = RED_LIGHT,
  .walk  = WALK_RED,
  .stateNum = 3,
  .next = ALL8(&redSouth)
};

const State redSouth = {
  .delay = 1,
  .south = RED_LIGHT,
  .west = RED_LIGHT,
  .walk = WALK_RED,
  .stateNum = 4,
  .next = {
    &greenSouth,
    &delayWalk, //delayWalk
    &delayRedWest,
    &delayWalk, //delayWalk
    &delayRedSouth,
    &delayWalk, //delayWalk
    &delayRedWest,
    &delayWalk, //delayWalk
  }

};

const State delayRedSouth = {
  .delay = 1500,
  .south = RED_LIGHT,
  .west = RED_LIGHT,
  .walk = WALK_RED,
  .stateNum = 5,
  .next = ALL8(&greenSouth)
};

const State delayRedWest = {
  .delay = 1500,
  .south = RED_LIGHT,
  .west = RED_LIGHT,
  .walk = WALK_RED,
  .stateNum = 6,
  .next = ALL8(&greenWest)
};

const State delayWalk = {
  .delay = 1500,
  .south = RED_LIGHT,
  .west = RED_LIGHT,
  .walk = WALK_RED,
  .stateNum = 7,
  .next = ALL8(&whiteWalk)
};

const State whiteWalk = {
  .delay = 1,
  .south = RED_LIGHT,
  .west = RED_LIGHT,
  .walk = WALK_WHITE,
  .stateNum = 8,
  .next = {
    &delayFlashWalk,
    &whiteWalk,
    &delayFlashWalk,
    &delayFlashWalk,
    &delayFlashWalk,
    &delayFlashWalk,
    &delayFlashWalk,
    &delayFlashWalk
  }
};

const State delayFlashWalk = {
  .delay = 2500,
  .south = RED_LIGHT,
  .west = RED_LIGHT,
  .walk = WALK_WHITE,
  .stateNum = 9,
  .next = ALL8(&redWalk1)
};

const State greenWest = {
  .delay = 1,
  .south = RED_LIGHT,
  .west = GREEN_LIGHT,
  .walk = WALK_RED,
  .stateNum = 10,
  .next = {
    &delayGreenWest,
    &delayGreenWest,
    &greenWest,
    &delayGreenWest,
    &delayGreenWest,
    &delayGreenWest,
    &delayGreenWest,
    &delayGreenWest
  }
};

const State delayGreenWest = {
  .delay = 1500,
  .south = RED_LIGHT,
  .west = GREEN_LIGHT,
  .walk = WALK_RED,
  .stateNum = 11,
  .next = ALL8(&yellowWest)
};

const State yellowWest = {
  .delay = 1,
  .south = RED_LIGHT,
  .west = YELLOW_LIGHT,
  .walk = WALK_RED,
  .stateNum = 12,
  .next = ALL8(&delayYellowWest)
};

const State delayYellowWest = {
  .delay = 1500,
  .south = RED_LIGHT,
  .west = YELLOW_LIGHT,
  .walk = WALK_RED,
  .stateNum = 13,
  .next = ALL8(&redWest)
};

const State redWest = {
  .delay = 1,
  .south = RED_LIGHT,
  .west = RED_LIGHT,
  .walk = WALK_RED,
  .stateNum = 14,
  .next = {
    &delayRedSouth, //000
    &delayWalk, //001
    &delayRedWest2, //010
    &delayWalk, //011
    &delayRedSouth, //100
    &delayRedSouth, //101
    &delayRedSouth, //110
    &delayRedSouth, //111
  }
};

const State delayRedWest2 = {
  .delay = 1500,
  .south = RED_LIGHT,
  .west = RED_LIGHT,
  .walk = WALK_RED,
  .stateNum = 25,
  .next = ALL8(&greenWest)
};

const State redWalk1 = {
  .delay = 1,
  .south = RED_LIGHT,
  .west = RED_LIGHT,
  .walk = WALK_RED,
  .stateNum = 15,
  .next = ALL8(&delayWalk1)
};

const State delayWalk1 = {
  .delay = 1000,
  .south = RED_LIGHT,
  .west = RED_LIGHT,
  .walk = WALK_RED,
  .stateNum = 16,
  .next = ALL8(&offWalk1)
};

const State offWalk1 = {
  .delay = 1,
  .south = RED_LIGHT,
  .west = RED_LIGHT,
  .walk = WALK_OFF,
  .stateNum = 17,
  .next = ALL8(&delayWalk2)
};


const State delayWalk2 = {
  .delay = 1000,
  .south = RED_LIGHT,
  .west = RED_LIGHT,
  .walk = WALK_OFF,
  .stateNum = 18,
  .next = ALL8(&redWalk2)
};

const State redWalk2 = {
  .delay = 1,
  .south = RED_LIGHT,
  .west = RED_LIGHT,
  .walk = WALK_RED,
  .stateNum = 19,
  .next = ALL8(&delayWalk3)
};

const State delayWalk3 = {
  .delay = 1000,
  .south = RED_LIGHT,
  .west = RED_LIGHT,
  .walk = WALK_RED,
  .stateNum = 20,
  .next = ALL8(&offWalk2)
};

const State offWalk2 = {
  .delay = 1,
  .south = RED_LIGHT,
  .west = RED_LIGHT,
  .walk = WALK_OFF,
  .stateNum = 21,
  .next = ALL8(&delayWalk4)
};

const State delayWalk4 = {
  .delay = 1000,
  .south = RED_LIGHT,
  .west = RED_LIGHT,
  .walk = WALK_OFF,
  .stateNum = 22,
  .next = ALL8(&redWalk3)
};

const State redWalk3 = {
  .delay = 1,
  .south = RED_LIGHT,
  .west = RED_LIGHT,
  .walk = WALK_RED,
  .stateNum = 23,
  .next = ALL8(&delayWalk5)
};

const State delayWalk5 = {
  .delay = 1500,
  .south = RED_LIGHT,
  .west = RED_LIGHT,
  .walk = WALK_RED,
  .stateNum = 24,
  .next = {
    &greenSouth,
    &whiteWalk,
    &greenWest,
    &greenWest,
    &greenSouth,
    &greenSouth,
    &greenSouth,
    &greenWest
  }
};


// initialize all 6 LED outputs and 3 switch inputs
// assumes LaunchPad_Init resets and powers A and B
void Traffic_Init(void){ // assumes LaunchPad_Init resets and powers A and B

  // ---------- South LEDs ----------
  IOMUX->SECCFG.PINCM[PB2INDEX] = 0x00000081;
  IOMUX->SECCFG.PINCM[PB1INDEX] = 0x00000081;
  IOMUX->SECCFG.PINCM[PB0INDEX] = 0x00000081;

  // ---------- West LEDs ----------
  IOMUX->SECCFG.PINCM[PB8INDEX] = 0x00000081;
  IOMUX->SECCFG.PINCM[PB7INDEX] = 0x00000081;
  IOMUX->SECCFG.PINCM[PB6INDEX] = 0x00000081;

  // ---------- Walk RGB LED ----------
  IOMUX->SECCFG.PINCM[PB22INDEX] = 0x00000081;
  IOMUX->SECCFG.PINCM[PB26INDEX] = 0x00000081;
  IOMUX->SECCFG.PINCM[PB27INDEX] = 0x00000081;

  // ---------- Buttons ----------
  IOMUX->SECCFG.PINCM[PB16INDEX] = 0x00040081; // South Btn
  IOMUX->SECCFG.PINCM[PB15INDEX] = 0x00040081; // West Btn
  IOMUX->SECCFG.PINCM[PB17INDEX] = 0x00040081; // Walk Btn

  GPIOB->DOE31_0 |= 0x0C4001C7;
  

}
/* Activate LEDs
* Inputs: west is 3-bit value to three east/west LEDs
*         south is 3-bit value to three north/south LEDs
*         walk is 3-bit value to 3-color positive logic LED on PB22,PB26,PB27
* Output: none
* - west =1 sets west green
* - west =2 sets west yellow
* - west =4 sets west red
* - south =1 sets south green
* - south =2 sets south yellow
* - south =4 sets south red
* - walk=0 to turn off LED
* - walk bit 22 sets blue color
* - walk bit 26 sets red color
* - walk bit 27 sets green color
* Feel free to change this. But, if you change the way it works, change the test programs too
* Be friendly*/
void Traffic_Out(uint32_t west, uint32_t south, uint32_t walk){

  // --- West LEDs ---
  const uint32_t WEST_MASK = (1 << 8) | (1 << 7) | (1 << 6);
  GPIOB->DOUT31_0 &= ~WEST_MASK;
  if(west == 1)       GPIOB->DOUT31_0 |= (1 << 6);  // green
  else if(west == 2)  GPIOB->DOUT31_0 |= (1 << 7);  // yellow
  else if(west == 4)  GPIOB->DOUT31_0 |= (1 << 8);  // red

  // --- South LEDs ---
  const uint32_t SOUTH_MASK = (1 << 2) | (1 << 1) | (1 << 0);
  GPIOB->DOUT31_0 &= ~SOUTH_MASK; 
  if(south == 1)      GPIOB->DOUT31_0 |= (1 << 0);  // green
  else if(south == 2) GPIOB->DOUT31_0 |= (1 << 1);  // yellow
  else if(south == 4) GPIOB->DOUT31_0 |= (1 << 2);  // red

  // --- Walk RGB LED ---
  const uint32_t WALK_MASK = (1 << 22) | (1 << 26) | (1 << 27);
  GPIOB->DOUT31_0 &= ~WALK_MASK;                  
  GPIOB->DOUT31_0 |= (walk & WALK_MASK);
  
}
/* Read sensors
 * Input: none
 * Output: sensor values
 * - bit 0 is west car sensor
 * - bit 1 is south car sensor
 * - bit 2 is walk people sensor
* Feel free to change this. But, if you change the way it works, change the test programs too
 */
uint32_t Traffic_In(void){
    uint32_t west  = (GPIOB->DIN31_0 >> 15) & 0x01;
    uint32_t south = (GPIOB->DIN31_0 >> 16) & 0x01;
    uint32_t walk  = (GPIOB->DIN31_0 >> 17) & 0x01;

    return (walk | (west << 1) | (south << 2));
}

void DelayMs(uint32_t ms){
    uint32_t delay_units = ms * 80000;  
    Clock_Delay(delay_units);
}

// use main1 to determine Lab4 assignment
void Lab4Grader(int mode);
void Grader_Init(void);
int main1(void){ // main1
  Clock_Init80MHz(0);
  LaunchPad_Init();
  Lab4Grader(0); // print assignment, no grading
  while(1){
  }
}
// use main2 to debug LED outputs
// at this point in ECE319K you need to be writing your own test functions
// modify this program so it tests your Traffic_Out  function
int main2(void){ // main2
  Clock_Init80MHz(0);
  LaunchPad_Init();
  Grader_Init(); // execute this line before your code
  LaunchPad_LED1off();
  Traffic_Init(); // your Lab 4 initialization
  if((GPIOB->DOE31_0 & 0x20)==0){
    UART_OutString("access to GPIOB->DOE31_0 should be friendly.\n\r");
  }
  UART_Init();
  UART_OutString("Lab 4, Spring 2026, Step 1. Debug LEDs\n\r");
  UART_OutString("EID1= "); UART_OutString((char*)EID1); UART_OutString("\n\r");
  while(1){
    
    uint32_t delay = 1000;

    // --- WEST LED TESTS ---
    Traffic_Out(1, 0, 0);  // West green - PB6
    Debug_Dump(GPIOB->DOUT31_0);
    DelayMs(delay);
    Debug_Dump(GPIOB->DOUT31_0);
    Traffic_Out(2, 0, 0);  // West yellow - PB7
    DelayMs(delay);
    Debug_Dump(GPIOB->DOUT31_0);
    Traffic_Out(4, 0, 0);  // West red - PB8
    DelayMs(delay);
    Debug_Dump(GPIOB->DOUT31_0);


    //--- SOUTH LED TESTS ---
    Traffic_Out(0, 1, 0);  // South green - PB0
    Debug_Dump(GPIOB->DOUT31_0);
    DelayMs(delay);
    Traffic_Out(0, 2, 0);  // South yellow - PB1
    Debug_Dump(GPIOB->DOUT31_0);
    DelayMs(delay);
    Traffic_Out(0, 4, 0);  // South red - PB2
    Debug_Dump(GPIOB->DOUT31_0);
    DelayMs(delay);

    // --- WALK RGB LED TESTS ---
    Traffic_Out(0, 0, (1<<22));  // Blue
    DelayMs(delay);
    Traffic_Out(0, 0, (1<<26));  // Red
    DelayMs(delay);
    Traffic_Out(0, 0, (1<<27));  // Green
    DelayMs(delay);
    Traffic_Out(0, 0, (1<<22)|(1<<26)|(1<<27)); // White (all)
    DelayMs(delay);

    // // --- COMBINED TEST ---
    // Traffic_Out(1, 4, (1<<22)|(1<<26)|(1<<27));  // West green, South red, Walk white
    // DelayMs(delay);
    // Traffic_Out(4, 1, (1<<26)); // West red, South green, Walk red
    // DelayMs(delay);

    if((GPIOB->DOUT31_0&0x20) == 0){
      UART_OutString("DOUT not friendly\n\r");
    }
  }
}
// use main3 to debug the three input switches
// at this point in ECE319K you need to be writing your own test functions
// modify this program so it tests your Traffic_In  function
int main3(void){ // main3
  uint32_t last=0,now;
  Clock_Init80MHz(0);
  LaunchPad_Init();
  Traffic_Init(); // your Lab 4 initialization
  Debug_Init();   // Lab 3 debugging
  UART_Init();
  __enable_irq(); // UART uses interrupts
  UART_OutString("Lab 4, Spring 2026, Step 2. Debug switches\n\r");
  UART_OutString("EID1= "); UART_OutString((char*)EID1); UART_OutString("\n\r");
  while(1){
    now = Traffic_In(); // Your Lab4 input

    if(now != last){ // change detected
        UART_OutString("Switch= 0x"); 
        UART_OutUHex(now); 
        UART_OutString("\n\r");

        Debug_Dump(now);
    }
    last = now;

    Clock_Delay(800000); // 10ms, to debounce switch
  }
}

// use main4 to debug using your dump
// proving your machine cycles through all states
int main4(void){// main4
uint32_t input;
  Clock_Init80MHz(0);
  LaunchPad_Init();
  LaunchPad_LED1off();
  Traffic_Init(); // your Lab 4 initialization
 // set initial state
  Debug_Init();   // Lab 3 debugging
  UART_Init();
  __enable_irq(); // UART uses interrupts
  UART_OutString("Lab 4, Spring 2026, Step 3. Debug FSM cycle\n\r");
  UART_OutString("EID1= "); UART_OutString((char*)EID1); UART_OutString("\n\r");
// initialize your FSM
  SysTick_Init();   // Initialize SysTick for software waits
  State *S = &greenSouth; //starting state
  while(1){
    Traffic_Out(S->west, S->south, S->walk);
    uint32_t walkLSB =
      ((S->walk >> 22) & 1)        // bit 22 → LSB bit 0
    | (((S->walk >> 26) & 1) << 1) // bit 26 → LSB bit 1
    | (((S->walk >> 27) & 1) << 2); // bit 27 → LSB bit 2
    uint32_t dumpMessage = (walkLSB) | ((S->south & 0xFF) << 8) 
                            | ((S->west & 0xFF) << 16)
                            | ((S->stateNum & 0xFF) << 24);
    Debug_Dump(dumpMessage);
    uint32_t input = 7;
    S = S->next[input];
    DelayMs(S->delay);

  }
      // 1) output depending on state using Traffic_Out
      // call your Debug_Dump logging your state number and output
      // 2) wait depending on state
      // 3) hard code this so input always shows all switches pressed
      // 4) next depends on state and input
}

// use main5 to grade
int main(void){// main5
  Clock_Init80MHz(0);
  LaunchPad_Init();
  Grader_Init(); // execute this line before your code
  LaunchPad_LED1off();
  Traffic_Init(); // your Lab 4 initialization
// initialize your FSM
  SysTick_Init();   // Initialize SysTick for software waits
  // initialize your FSM
  State *S = &greenSouth; //starting state
  Lab4Grader(1); // activate UART, grader and interrupts
  while(1){

    Traffic_Out(S->west, S->south, S->walk);
    uint32_t walkLSB =
      ((S->walk >> 22) & 1)        // bit 22 → LSB bit 0
    | (((S->walk >> 26) & 1) << 1) // bit 26 → LSB bit 1
    | (((S->walk >> 27) & 1) << 2); // bit 27 → LSB bit 2
    uint32_t dumpMessage = (walkLSB) | ((S->south & 0xFF) << 8) 
                            | ((S->west & 0xFF) << 16)
                            | ((S->stateNum & 0xFF) << 24);
    //Debug_Dump(dumpMessage);
    if(S->stateNum == 10) Debug_Dump(Traffic_In());
    uint32_t input = Traffic_In();
    S = S->next[input];
    DelayMs(S->delay);
      // 1) output depending on state using Traffic_Out
      // call your Debug_Dump logging your state number and output
      // 2) wait depending on state
      // 3) input from switches
      // 4) next depends on state and input
  }
}

