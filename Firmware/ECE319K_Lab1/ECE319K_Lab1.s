// ****************** ECE319K_Lab1.s ***************
// Your solution to Lab 1 in assembly code
// Author: Sathvik Yechuri
// Last Modified: 1/27/2026
// Spring 2026
        .data
        .align 2
// Declare global variables here if needed
// with the .space assembly directive

        .text
        .thumb
        .align 2
        .global EID
EID:    .string "SY24742" // replace ZZZ123 with your EID here

        .global Phase
        .align 2
Phase:  .long 10
// Phase= 0 will display your objective and some of the test cases, 
// Phase= 1 to 5 will run one test case (the ones you have been given)
// Phase= 6 to 7 will run one test case (the inputs you have not been given)
// Phase=10 will run the grader (all cases 1 to 7)
        .global Lab1
// Input: R0 points to the list
// Return: R0 as specified in Lab 1 assignment and terminal window
// According to AAPCS, you must save/restore R4-R7
// If your function calls another function, you must save/restore LR
Lab1: 
        PUSH {R4-R7,LR}
        
        // your solution goes here
        LDR R1, =EID // addr of eid label
Loop:
        LDR R2, [R0, #0]
        CMP R2, #0
        BEQ Done

        BL compareString // outputs in R3 a 1 for same, 0 for not same
        CMP R3, #1
        BEQ getScore
        ADDS R0, R0, #8
        B Loop

getScore:
        ADDS R0, R0, #4
        LDR R0, [R0, #0]
        B Return
Done:
        LDR R0, =-1
Return:
        POP  {R4-R7,PC} // return

compareString: // R1 = addr of string 1, R2 = addr of string 2, R3 = 1 for yes, 0 for no
        PUSH {R1-R2, R4-R7,LR}
csLoop:        
        LDRB R4, [R1, #0]
        LDRB R5, [R2, #0]
        MOVS R3, #1

        // for same null terminations
        CMP R4, #0
        BEQ sameTermination
        CMP R5, #0
        BEQ notSame


        CMP R4, R5
        BNE notSame
        ADDS R1, R1, #1
        ADDS R2, R2, #1
        B csLoop


notSame:
        MOVS R3, #0
        B csReturn

sameTermination:
        EORS R4, R4, R5
        BEQ csReturn
        B notSame

csReturn:
        POP {R1-R2, R4-R7, PC}

        .align 2
        .global myClass
myClass: .long pAB123  // pointer to EID
         .long 95      // Score
         .long pXYZ1   // pointer to EID
         .long 96      // Score
         .long pAB5549 // pointer to EID
         .long 94      // Score
         .long 0       // null pointer means end of list
         .long 0
pAB123:  .string "AB123"
pXYZ1:   .string "XYZ1"
pAB5549: .string "AB5549"
        .end
