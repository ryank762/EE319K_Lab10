// Timer2.c
// Runs on LM4F120 or TM4C123
// Program written by: Ryan Kim
// Lab number: 10

#include <stdint.h>
#include "Timer2.h"
#include "tm4c123gh6pm.h"
#include "ADC.h"
#include "Tetris.h"

// ***************** TIMER2_Init ****************
// Activate TIMER2 interrupts to run user task periodically
// Inputs:  task is a pointer to a user function
//          period in units (1/clockfreq)
// Outputs: none
void Timer2_Init(void){ 
  unsigned long volatile delay;
  SYSCTL_RCGCTIMER_R |= 0x04;   // 0) activate timer2
  delay = SYSCTL_RCGCTIMER_R;
  TIMER2_CTL_R = 0x00000000;   // 1) disable timer2A
  TIMER2_CFG_R = 0x00000000;   // 2) 32-bit mode
  TIMER2_TAMR_R = 0x00000002;  // 3) periodic mode
  TIMER2_TAILR_R = (80000000/10)-1;   // 4) reload value
  TIMER2_TAPR_R = 0;           // 5) clock resolution
  TIMER2_ICR_R = 0x00000001;   // 6) clear timeout flag
  TIMER2_IMR_R = 0x00000001;   // 7) arm timeout
  NVIC_PRI5_R = (NVIC_PRI5_R&0x00FFFFFF)|0x80000000; 
// 8) priority 4
  NVIC_EN0_R = 1<<23;          // 9) enable IRQ 23 in
  TIMER2_CTL_R = 0x00000001;   // 10) enable timer2A
}

void Timer2A_Handler(void) {
	TIMER2_ICR_R = TIMER_ICR_TATOCINT;
	TIMER0_CTL_R = 0x00000000;
	Drop_Block();
	Check_Board();
	Display_Board();
	TIMER0_CTL_R = 0x00000001;
}
