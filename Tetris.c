// Tetris.c
// Runs on LM4F120/TM4C123
// Ryan Kim

// Last Modified: 04/20/2018 

#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "ST7735.h"
#include "Random.h"
#include "PLL.h"
#include "ADC.h"
#include "IO.h"
#include "Sound.h"
#include "Timer0.h"
#include "Timer1.h"
#include "Blocks.h"

// Slide pot used for x-position of Tetris block
// Button0 (PE0) : Rotates block
// Button1 (PE1) : Fast-drops block

void DisableInterrupts(void);			// Disable interrupts
void EnableInterrupts(void);			// Enable interrupts
void Delay100ms(uint32_t count);	// time delay in 0.1 seconds
void Wait10ms(uint32_t n);				// time delay in 0.01 seconds
void Game_Over(void);							// displays "GAME OVER" screen
void FastDrop_Block(void);				// shortens time interval between block drop
uint32_t Convert(uint32_t input);	// converts ADC data into a floating point value from 0 to 10

#define PF1       (*((volatile uint32_t *)0x40025008))
#define PF2       (*((volatile uint32_t *)0x40025010))
#define PF3       (*((volatile uint32_t *)0x40025020))			

struct board {
	uint8_t state;
	uint16_t color;
};

struct board TetrisBoard[10][16];

int8_t i, j;
uint32_t Data;        						// 12-bit ADC
uint8_t gameOverFlag = 0;
uint32_t blocksPlaced = 0;

int main(void) {
	DisableInterrupts();
	PLL_Init(Bus80MHz);							// Bus clock is 80 MHz 
	Timer0_Init();
	Random_Init(1);									// seed
	Tetris_Init();
	ADC_Init();											// initialize slide pot
	Sound_Init();										// initialize sound
	Output_Init();									// clear display
	EnableInterrupts();
	while (1) {
		if (gameOverFlag == 1) {
			Game_Over();
		}
		if ((GPIO_PORTE_DATA_R & 0x01) == 1) {
			Wait10ms(2);
			while ((GPIO_PORTE_DATA_R & 0x01) == 1) {
				
			}
			Rotate_Block();
		}
		while ((GPIO_PORTE_DATA_R & 0x02) == 1) {
			FastDrop_Block();
		}
		TIMER0_TAILR_R = (80000000-1);
	}
}

uint32_t Convert(uint32_t input){
	uint32_t result;
	result = (input*(10/4.096));	
  return result;
}

void Board_Init(void) {
	for (i = 0; i < 16; i++) {
		for (j = 0; j < 10; j++) {
			TetrisBoard[i][j].state = 0;
			TetrisBoard[i][j].color = 0x0000;
		}
	}
}

void SysTick_Init(void) {
	NVIC_ST_CTRL_R = 0;									// disable SysTick during init
	NVIC_ST_RELOAD_R = (80000000/60)-1; // 60Hz interrupts
	NVIC_ST_CURRENT_R = 0;							// any write to CURRENT clears it
	NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R & 0x00FFFFFF)|0x20000000;
	NVIC_ST_CTRL_R = 0x00000007;				// enable interrupts, enable SysTick
}

void SysTick_Handler(void) {
	PF3 ^= 0x04;      // Heartbeat
	Data = ADC_In();  // sample 12-bit channel 5
	PF3 ^= 0x04;			// ADC execution time
}

/*
void Board_Check(void) {
	uint8_t i;
	while (i < 16) {
		if (currentBoard[i][15].status == 1) {
			gameOverFlag = 1;
			break;
		}
	}
	if (gameOverFlag == 1) {
		Game_Over();
	}
}
*/

void Game_Over(void) {
	ST7735_FillScreen(0x0000);
	ST7735_SetCursor(3, 1);
	ST7735_OutString("GAME OVER");
	ST7735_SetCursor(3, 2);
	ST7735_OutString("You placed");
	ST7735_SetCursor(3, 3);
	LCD_OutDec(blocksPlaced);
	ST7735_OutString(" blocks");
	while (1) {
		
	}
}

void FastDrop_Block(void) {
	TIMER0_TAILR_R = (80000000/8)-1;
}

void Timer0A_Handler(void) {
	TIMER0_ICR_R = TIMER_ICR_TATOCINT;
	Drop_Block();
}

void Delay100ms(uint32_t count) {
	uint32_t volatile time;
	while (count > 0) {
		time = 727240;  // 0.1sec at 80 MHz
		while (time) {
			time--;
		}
		count--;
	}
}
