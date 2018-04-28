// IO.c
// This software configures the switch and LED
// You are allowed to use any switch and any LED, 
// although the Lab suggests the SW1 switch PF4 and Red LED PF1
// Runs on LM4F120 or TM4C123
// Program written by: put your names here
// Date Created: March 30, 2018
// Last Modified:  change this or look silly
// Lab number: 7


#include "../inc/tm4c123gh6pm.h"
#include <stdint.h>

void Wait10ms(uint32_t n){
	uint32_t volatile time;
  while(n){
    time = 72724*20/91;  // 10msec, tuned at 80 MHz
    while(time){
	  	time--;
    }
    n--;
  }
}

//------------IO_Init------------
// Initialize GPIO Port for a switch and an LED
// Input: none
// Output: none
void IO_Init(void) {
 // --UUU-- Code to initialize PF4 and PF2
	SYSCTL_RCGCGPIO_R |= 0x20;						// enable PF clock
	volatile int clockDelay;
	clockDelay = SYSCTL_RCGCGPIO_R;				// wait until stable
	GPIO_PORTF_LOCK_R = GPIO_LOCK_KEY;		// unlock PF
	GPIO_PORTF_CR_R |= 0x1F;							// allow access
	GPIO_PORTF_DEN_R |= 0x14;							// enable digital logic on PF2,4
	GPIO_PORTF_DIR_R |= 0x04;							// make PF2 output
	GPIO_PORTF_AFSEL_R = 0x00;
	GPIO_PORTF_AMSEL_R = 0x00;						// disable AF and AM for PF
	GPIO_PORTF_PUR_R = 0x00;							// disable negative logic on PF
}

//------------IO_HeartBeat------------
// Toggle the output state of the  LED.
// Input: none
// Output: none
void IO_HeartBeat(void) {
 // --UUU-- PF2 is heartbeat
	GPIO_PORTF_DATA_R ^= 0x04;						// toggle PF2 (LED)
}

//------------IO_Touch------------
// wait for release and press of the switch
// Delay to debounce the switch
// Input: none
// Output: none
void IO_Touch(void) {
 // --UUU-- wait for release; delay for 20ms; and then wait for press
//	while((GPIO_PORTE_DATA_R & 0x10) != 0);		// do nothing until unpressed
//	Wait10ms(2);															// wait 20ms after unpressed
//	while((GPIO_PORTE_DATA_R & 0x10) == 0);		// do nothing until pressed
	while((GPIO_PORTF_DATA_R & 0x10) != 0);		// do nothing until unpressed
	Wait10ms(2);															// wait 20ms after unpressed
	while((GPIO_PORTF_DATA_R & 0x10) == 0);		// do nothing until pressed
}  

