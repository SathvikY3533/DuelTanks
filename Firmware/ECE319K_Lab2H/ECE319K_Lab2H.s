// ****************** ECE319K_Lab2H.s ***************
// Your solution to Lab 2 in assembly code
// Author: Sathvik Yechuri
// Last Modified: 2/3/2026
// ECE319H Spring 2026 (ECE319K students do Lab2)

    .include "../inc/msp.s"

        .data
        .align 2
// Declare global variables here if needed
// with the .space assembly directive

        .text
        .thumb
        .align 2
        .global EID
EID:    .string "SY24742" // replace ZZZ123 with your EID here
        .align 2
  .equ dot,1 //.equ dot,100
  .equ dash,(3*dot)
  .equ shortgap,(2*dot)  // because it will send an interelement too
  .equ interelement,dot
  Morse:
  .long  dot,  dash,    0,    0, 0 // A
  .long  dash,  dot,  dot,  dot, 0 // B
  .long  dash,  dot, dash,  dot, 0 // C
  .long  dash,  dot,  dot,    0, 0 // D
  .long  dot,     0,    0,    0, 0 // E
  .long  dot,   dot, dash,  dot, 0 // F
  .long  dash, dash,  dot,    0, 0 // G
  .long  dot,   dot,  dot,  dot, 0 // H
  .long  dot,   dot,    0,    0, 0 // I
  .long  dot,  dash, dash, dash, 0 // J
  .long  dash,  dot, dash,    0, 0 // K
  .long  dot,  dash,  dot,  dot, 0 // L
  .long  dash, dash,    0,    0, 0 // M
  .long  dash,  dot,    0,    0, 0 // N
  .long  dash, dash, dash,    0, 0 // O
  .long  dot,  dash, dash,  dot, 0 // P
  .long  dash, dash,  dot, dash, 0 // Q
  .long  dot,  dash,  dot,    0, 0 // R
  .long  dot,   dot,  dot,    0, 0 // S
  .long  dash,    0,    0,    0, 0 // T
  .long  dot,   dot, dash,    0, 0 // U
  .long  dot,   dot,  dot, dash, 0 // V
  .long  dot,  dash, dash,    0, 0 // W
  .long  dash,  dot,  dot, dash, 0 // X
  .long  dash,  dot, dash, dash, 0 // Y
  .long  dash, dash,  dot,  dot, 0 // Z

  .align 2
  .global Lab2Grader
  .global Lab2
  .global Debug_Init
  .global Dump
  .global Debug_Period
  .global Debug_Duty

// Switch input: PB1
// LED output: PB16
// logic analyzer pins PB18 PB17 PB16 PB2 PB1 PB0
// analog scope pin PB20
Lab2:
// Initially the main program will
//   set bus clock at 80 MHz,
//   reset and power enable both Port A and Port B
// Lab2Grader will
//   configure interrupts  on TIMERG0 for grader or TIMERG7 for TExaS
//   initialize ADC0 PB20 for scope,
//   initialize UART0 for grader or TExaS
    //MOVS R0,#0  // 0 for info, 1 debug with logic analyzer, 2 debug with scope, 10 for grade
    BL   Lab2Init   // you initialize input pin and output pin
    BL   Debug_Init
    MOVS R0,#3
    BL Lab2Grader //random letter input in R0
loop:
    BL Delay1s
    //BL checkSWInput
    //MOVS R0,#10
    //BL Lab2Grader //random letter input in R0
    //BL MorseOutput
    BL Dot
    B  loop
    //BL Delay1s
    //BL checkSWInput
    //H
    BL Dot
    //BL Dot
    //BL Dot
    //BL Dot

    //O
    //BL Dash
    //BL Dash
    //BL Dash

    //MOVS R0, #1
    //LSLS R0, 16
    //BL Debug_Period
    //BL Debug_Duty
    //B loop

MorseOutput:
    PUSH{R1-R7, LR}
    //find letter in Morse Array
    LDR R1, =Morse
    LDR R2, =0x41 //load ascii of 'A'
    SUBS R3, R0, R2
    MOVS R2, #20
    MULS R3, R3, R2
    ADDS R1, R1, R3

    MOVS R4, #0
parseLetter:
    CMP R4, #5
    BEQ parseDone
    LDR R5, [R1]
    CMP R5, #1
    BEQ DotOut
    CMP R5, #3
    BEQ DashOut
MorseContinue:
    ADDS R1, R1, #4
    ADDS R4, R4, #1
    B parseLetter
parseDone:
    POP{R1-R7, PC}

DotOut:
    BL Dot
    B MorseContinue

DashOut:
    BL Dash
    B MorseContinue


checkSWInput:
    PUSH {R1-R7, LR}
checkRelease:
    LDR R0, =GPIOB_DIN31_0
    LDR R2, [R0]
    LDR R1, =0x00000002
    ANDS R1, R1, R2
    CMP R1, #0
    BNE checkRelease
checkPress:
    LDR R0, =GPIOB_DIN31_0
    LDR R2, [R0]
    LDR R1, =0x00000002
    ANDS R1, R1, R2
    CMP R1, #2
    BNE checkPress
    POP {R1-R7, PC}

Dot:
    PUSH{R0-R7,LR}
    BL ToggleLED_ON
    BL Dump
    BL Delay100ms
    BL ToggleLED_OFF
    BL Dump
    BL Delay100ms
    POP{R0-R7,PC}

Dash:
    PUSH{R0-R7, LR}
    BL ToggleLED_ON
    BL Dump
    BL Delay100ms
    BL Delay100ms
    BL Delay100ms
    BL ToggleLED_OFF
    BL Dump
    BL Delay100ms
    POP{R0-R7, PC}


ToggleLED_ON:
    PUSH {R1-R7, LR}
    LDR  R4, =GPIOB_DOUT31_0
    LDR  R5, [R4]
    LDR  R6, =0x00010000
    ORRS R5, R5, R6
    STR  R5, [R4]
    POP  {R1-R7, PC}

ToggleLED_OFF:
    PUSH {R1-R7,LR}
    LDR  R4, =GPIOB_DOUT31_0
    LDR  R5, [R4]
    LDR  R6, =0x00010000
    BICS R5, R5, R6
    STR  R5, [R4]
    POP  {R1-R7,PC}

Delay1s:
    PUSH{R1-R7, LR}
    BL Delay100ms
    BL Delay100ms
    BL Delay100ms
    BL Delay100ms
    BL Delay100ms
    BL Delay100ms
    BL Delay100ms
    BL Delay100ms
    BL Delay100ms
    BL Delay100ms
    POP{R1-R7, PC}


Delay100ms:
    PUSH{R1-R7, LR}
    MOVS R3,#41
Outer:
    MOVS R2,#255
Middle:
    MOVS R1,#255
Inner:
    SUBS R1,R1,#1
    BNE Inner
    SUBS R2,R2,#1
    BNE Middle
    SUBS R3,R3,#1
    BNE Outer
    POP{R1-R7, PC}


// make switch an input, LED an output
// PortB is already reset and powered
// Set IOMUX for your input and output
// Set GPIOB_DOE31_0 for your output (be friendly)
Lab2Init:
    PUSH{LR}
// ***do not reset/power Port A or Port B, already done****
    // configure iomux so pin is init to GPIO
    MOVS R1, #0x81 // x01 -> selects GPIO Function, x80 -> digital mode enable
    LDR R0, =IOMUXPB16
    STR R1, [R0] // sets iomux for PB16 aka PINCM

    // if output -> enable GPIOB_DOE31_0
    LDR R0, =GPIOB_DOE31_0
    LDR R1, [R0] //first read existing states to apply mask over
    LDR R2, =0x00010000 //0000 0000 0000 0001 0000 0000 0000 0000 -> bit 16 represents PB16
    ORRS R1, R1, R2 //applying mask to enable bit 16 as output on DOE
    STR R1, [R0]

    //if input -> no doe needed
    LDR R0, =IOMUXPB1
    LDR R1, =0x00040081 //init pincm for PB1 as general input
    STR R1, [R0]
    POP{PC}
   .end
