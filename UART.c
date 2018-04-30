// UART1.c
// Runs on LM4F120/TM4C123
// Use UART1 to implement bidirectional data transfer to and from 
// another microcontroller in Lab 9.  This time, interrupts and FIFOs
// are used.
// Daniel Valvano
// November 17, 2014
// Modified by EE345L students Charlie Gough && Matt Hawk
// Modified by EE345M students Agustinus Darmawan && Mingjie Qiu

/* Lab solution, Do not post
 http://users.ece.utexas.edu/~valvano/
*/

// U1Rx (VCP receive) connected to PC4
// U1Tx (VCP transmit) connected to PC5
#include <stdint.h>
#include "FiFo.h"
#include "UART.h"
#include "tm4c123gh6pm.h"
#define PF1       (*((volatile uint32_t *)0x40025008))
#define PF2       (*((volatile uint32_t *)0x40025010))
#define PF3       (*((volatile uint32_t *)0x40025020))
uint32_t DataLost; 
uint32_t RxCounter = 0;
// Initialize UART1
// Baud rate is 115200 bits/sec
// Make sure to turn ON UART1 Receiver Interrupt (Interrupt 6 in NVIC)
// Write UART1_Handler
void UART_Init(void){
	volatile uint32_t delay;
	SYSCTL_RCGCUART_R |= 0x0002;				// activate UART1
	delay = SYSCTL_RCGCUART_R;					// wait for clock to stabilize
	SYSCTL_RCGCGPIO_R |= 0x0004;				// activate port C
	UART1_CTL_R &= ~0x0001;							// disable UART1
	UART1_IBRD_R = 43;									// IBRD=int(80000000/(16*115,200)) = int(43.4027)
	UART1_FBRD_R = 26;									// FBRD = round(0.4027 * 64) = 26
	UART1_LCRH_R  = 0x0070;							// 8-bit word length, enable FIFO
	UART1_IM_R |= 0x10;									// arm RXRIS
	UART1_IFLS_R |= 0x10;								
	UART1_IFLS_R &= ~0x28;		
	// set bits 5,4,3 to 010 (1/2 full)
	NVIC_PRI1_R = (NVIC_PRI1_R&0xFF0FFFFF)+0x00600000;	// set bits 21-23
	NVIC_EN0_R	|= 0x0040;							// enable interrupt 6 in NVIC
	UART1_CTL_R = 0x0301;								// enable RXE, TXE and UART
	GPIO_PORTC_AFSEL_R |= 0x30;					// enable alt funct on PC5-4
	GPIO_PORTC_PCTL_R = (GPIO_PORTC_PCTL_R&0xFF00FFFF)+0x00220000; // UART
	GPIO_PORTC_DEN_R |= 0x30;						// enable digital I/O on PC5-4
	GPIO_PORTC_AMSEL_R &= ~0x30;				// disable analog function on PC5-4
	FiFo_Init();
}

// input ASCII character from UART
// spin if RxFifo is empty
char UART_InChar(void){
  while((UART1_FR_R&0x0010) != 0);		// wait until RXFE is 0
	return((char)(UART1_DR_R&0xFF));
}
//------------UART1_OutChar------------
// Output 8-bit to serial port
// Input: letter is an 8-bit ASCII character to be transferred
// Output: none
void UART_OutChar(char data){
  while((UART1_FR_R&0x0020) != 0); // wait until TXFFis 0
	UART1_DR_R = data;
}

// hardware RX FIFO goes from 7 to 8 or more items
// UART receiver Interrupt is triggered; This is the ISR
void UART1_Handler(void){
	//PF1 ^= 0x02;												// toggle PF1
	//PF1 ^= 0x02;												// toggle PF1
	while((UART1_FR_R & 0x0010) == 0){	// read while not empty
		FiFo_Put(UART_InChar());
	}
	RxCounter++;												// increment receiving counter
	UART1_ICR_R = 0x10;									// recognize that ISR was serviced
	//PF1 ^= 0x02;												// toggle PF1
}
