// Timer2.h
// Runs on LM4F120 or TM4C123
// Program written by: Ryan Kim
// Lab number: 10

#include "tm4c123gh6pm.h"

#ifndef __TIMER2INTS_H__ // do not include more than once
#define __TIMER2INTS_H__

// ***************** Timer2_Init ****************
// Activate Timer2 interrupts to run user task periodically
// Inputs:  task is a pointer to a user function
//          period in units (1/clockfreq)
// Outputs: none
void Timer2_Init(uint32_t period);

#endif // __TIMER2INTS_H__
