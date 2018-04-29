// ADC.c
// Runs on LM4F120/TM4C123
// Provide functions that initialize ADC0
// Last Modified: 3/29/2018 
// Student names: Ryan Kim

#include <stdint.h>
#include "Tetris.h"
#include "../inc/tm4c123gh6pm.h"

// ADC initialization function 
// Input: none
// Output: none
// measures from PD2, analog channel 5
void ADC_Init(void) { 
	volatile long delay;
	SYSCTL_RCGCGPIO_R |= 0x08;			// turn PD clock on
	SYSCTL_RCGCADC_R |= 0x0001;			// activate ADC0
	delay = SYSCTL_RCGCGPIO_R;			// wait until clock stabilizes
	GPIO_PORTD_DIR_R &= 0x04;				// make PD2 input
	GPIO_PORTD_AFSEL_R |= 0x04;			// enable alternate function on PD2
	GPIO_PORTD_DEN_R |= ~0x04;			// disable digital logic on PD2
	GPIO_PORTD_AMSEL_R |= 0x04;			// enable analog function on PD2
//
	ADC0_PC_R &= 0;									// clear sample rate
	ADC0_PC_R |= 0x1;								// set sample rate to 125K samples/sec
	ADC0_SSPRI_R = 0x0123;					// sequencer 3 is highest priority
	ADC0_ACTSS_R &= ~0x0008;				// disable sample sequencer 3 during init
	ADC0_EMUX_R &= ~0xF000;					// seuence 3 is software trigger
	ADC0_SSMUX3_R &= ~0x000F;				// clear SS3 field
	ADC0_SSMUX3_R += 5;							// set analog channel to 5
	ADC0_SSCTL3_R = 0x0006;					// no TS0 D0, yes IE0 END0
	ADC0_IM_R &= ~0x0008;						// disable SS3 interrupts
	ADC0_ACTSS_R |= 0x0008;					// enable sample sequencer 3
}

//------------ADC_In------------
// Busy-wait Analog to digital conversion
// Input: none
// Output: 12-bit result of ADC conversion
// measures from PD2, analog channel 5
uint32_t ADC_In(void) {  
	uint32_t result;
	ADC0_PSSI_R = 0x0008;							// init SS3
	while((ADC0_RIS_R & 0x08) == 0);	// wait for conversion done
	result = ADC0_SSFIFO3_R & 0xFFF;	// read result
	ADC0_ISC_R = 0x0008;							// acknowledge
  return result; 										// remove this, replace with real code
}


