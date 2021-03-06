// Timer0.c
// Runs on LM4F120/TM4C123
// Use TIMER0 in 32-bit periodic mode to request interrupts at a periodic rate
// Daniel Valvano
// Modified by: Ryan Kim, Rafay Khurram
// Last Modified: 3/6/2015 
// You can use this timer only if you learn how it works

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2015
  Program 7.5, example 7.6

 Copyright 2015 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */
#include <stdint.h>
#include "Tetris.h"
#include "Blocks.h"
#include "tm4c123gh6pm.h"

void DisableInterrupts(void);			// Disable interrupts
void EnableInterrupts(void);			// Enable interrupts

// ***************** Timer0_Init ****************
// Activate TIMER0 interrupts to run user task periodically
// Inputs:  task is a pointer to a user function
//          period in units (1/clockfreq)
// Outputs: none

uint32_t TimerCount, TimerCounter;

void Timer0_Init(void){
	TimerCount = 0;
	TimerCounter = 0;
	volatile unsigned long delay;
  SYSCTL_RCGCTIMER_R |= 0x01;   // 0) activate TIMER0
	delay = SYSCTL_RCGCTIMER_R;
  TIMER0_CTL_R = 0x00000000;    // 1) disable TIMER0A during setup
  TIMER0_CFG_R = 0x00000000;    // 2) configure for 32-bit mode
  TIMER0_TAMR_R = 0x00000002;   // 3) configure for periodic mode, default down-count settings
  TIMER0_TAILR_R = (80000000/15)-1;			// 4) reload value
//	 TIMER0_TAILR_R = (80000000/360)-1;			// 4) reload value
  TIMER0_TAPR_R = 0;            // 5) bus clock resolution
  TIMER0_ICR_R = 0x00000001;    // 6) clear TIMER0A timeout flag
  TIMER0_IMR_R = 0x00000001;    // 7) arm timeout interrupt
  NVIC_PRI4_R = (NVIC_PRI4_R&0x00FFFFFF)|0x80000000; // 8) priority 4
// interrupts enabled in the main program after all devices initialized
// vector number 35, interrupt number 19
  NVIC_EN0_R = 1<<19;           // 9) enable IRQ 19 in NVIC
  TIMER0_CTL_R = 0x00000001;    // 10) enable TIMER0A
}

void Timer0A_Handler(void) {
	uint8_t rem;
	TIMER0_ICR_R = TIMER_ICR_TATOCINT;
	Display_Board();
	if ((currentx != xSave) || (currenty != ySave)) {
		Erase_Block(currentType, currentRot, xSave, ySave);
		Place_Block(currentType, currentRot, currentx, currenty);
		xSave = currentx;
		ySave = currenty;
		Display_Board();
	}
	TimerCount++;
	if(TimerCount == (12 - level)) {
//		Add_Line();
//		Rotate_Block();
//		FastDrop_Block();
		Drop_Block();
		Display_Board();
//		Check_Board();
		TimerCount = 0;
		TimerCounter++;
		score = ((blocksPlaced * 10) + (TimerCounter * 1.2) + (linesCleared * 50) + (fdropCount * 10)) * 10;

		rem = score%10;
		if (rem != 0) {
			score -= rem;
		}
	}
}
