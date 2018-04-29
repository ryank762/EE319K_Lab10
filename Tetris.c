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
#include "Timer2.h"
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
void Board_Init(void);						// initializes buffer
void Timer2A_Handler(void);
void Display_Init(void);
void SysTick_Init(void);
void PortE_Init(void);
void PortF_Init(void);
int16_t Coordinate_ConversionX(int16_t);
int16_t Coordinate_ConversionY(int16_t);
uint32_t Convert(uint32_t input);	// converts ADC data into a floating point value from 0 to 10

#define PF1       (*((volatile uint32_t *)0x40025008))
#define PF2       (*((volatile uint32_t *)0x40025010))
#define PF3       (*((volatile uint32_t *)0x40025020))			


const uint16_t IBlock[49] = {
	0x8410,0x8410,0x8410,0x8410,0x8410,0x8410,0x8410,
	0x8410,0x8410,0x8410,0x8410,0x8410,0x8410,0x8410,
	0x8410,0x8410,0x8410,0x8410,0x8410,0x8410,0x8410,
	0x8410,0x8410,0x8410,0x8410,0x8410,0x8410,0x8410,
	0x8410,0x8410,0x8410,0x8410,0x8410,0x8410,0x8410,
	0x8410,0x8410,0x8410,0x8410,0x8410,0x8410,0x8410,
	0x8410,0x8410,0x8410,0x8410,0x8410,0x8410,0x8410
};

const uint16_t JBlock[49] = {
	0xF800,0xF800,0xF800,0xF800,0xF800,0xF800,0xF800,
	0xF800,0xF800,0xF800,0xF800,0xF800,0xF800,0xF800,
	0xF800,0xF800,0xF800,0xF800,0xF800,0xF800,0xF800,
	0xF800,0xF800,0xF800,0xF800,0xF800,0xF800,0xF800,
	0xF800,0xF800,0xF800,0xF800,0xF800,0xF800,0xF800,
	0xF800,0xF800,0xF800,0xF800,0xF800,0xF800,0xF800,
	0xF800,0xF800,0xF800,0xF800,0xF800,0xF800,0xF800
};

const uint16_t LBlock[49] = {
	0x07FF,0x07FF,0x07FF,0x07FF,0x07FF,0x07FF,0x07FF,
	0x07FF,0x07FF,0x07FF,0x07FF,0x07FF,0x07FF,0x07FF,
	0x07FF,0x07FF,0x07FF,0x07FF,0x07FF,0x07FF,0x07FF,
	0x07FF,0x07FF,0x07FF,0x07FF,0x07FF,0x07FF,0x07FF,
	0x07FF,0x07FF,0x07FF,0x07FF,0x07FF,0x07FF,0x07FF,
	0x07FF,0x07FF,0x07FF,0x07FF,0x07FF,0x07FF,0x07FF,
	0x07FF,0x07FF,0x07FF,0x07FF,0x07FF,0x07FF,0x07FF
};

const uint16_t OBlock[64] = {
	0x07E0,0x07E0,0x07E0,0x07E0,0x07E0,0x07E0,0x07E0,
	0x07E0,0x07E0,0x07E0,0x07E0,0x07E0,0x07E0,0x07E0,
	0x07E0,0x07E0,0x07E0,0x07E0,0x07E0,0x07E0,0x07E0,
	0x07E0,0x07E0,0x07E0,0x07E0,0x07E0,0x07E0,0x07E0,
	0x07E0,0x07E0,0x07E0,0x07E0,0x07E0,0x07E0,0x07E0,
	0x07E0,0x07E0,0x07E0,0x07E0,0x07E0,0x07E0,0x07E0,
	0x07E0,0x07E0,0x07E0,0x07E0,0x07E0,0x07E0,0x07E0
};

const uint16_t SBlock[49] = {
	0x04FF,0x04FF,0x04FF,0x04FF,0x04FF,0x04FF,0x04FF,
	0x04FF,0x04FF,0x04FF,0x04FF,0x04FF,0x04FF,0x04FF,
	0x04FF,0x04FF,0x04FF,0x04FF,0x04FF,0x04FF,0x04FF,
	0x04FF,0x04FF,0x04FF,0x04FF,0x04FF,0x04FF,0x04FF,
	0x04FF,0x04FF,0x04FF,0x04FF,0x04FF,0x04FF,0x04FF,
	0x04FF,0x04FF,0x04FF,0x04FF,0x04FF,0x04FF,0x04FF,
	0x04FF,0x04FF,0x04FF,0x04FF,0x04FF,0x04FF,0x04FF
};

const uint16_t TBlock[49] = {
	0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,
	0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,
	0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,
	0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,
	0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,
	0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,
	0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF
};

const uint16_t ZBlock[49] = {
	0x0F8F,0x0F8F,0x0F8F,0x0F8F,0x0F8F,0x0F8F,0x0F8F,
	0x0F8F,0x0F8F,0x0F8F,0x0F8F,0x0F8F,0x0F8F,0x0F8F,
	0x0F8F,0x0F8F,0x0F8F,0x0F8F,0x0F8F,0x0F8F,0x0F8F,
	0x0F8F,0x0F8F,0x0F8F,0x0F8F,0x0F8F,0x0F8F,0x0F8F,
	0x0F8F,0x0F8F,0x0F8F,0x0F8F,0x0F8F,0x0F8F,0x0F8F,
	0x0F8F,0x0F8F,0x0F8F,0x0F8F,0x0F8F,0x0F8F,0x0F8F,
	0x0F8F,0x0F8F,0x0F8F,0x0F8F,0x0F8F,0x0F8F,0x0F8
};

const uint16_t nBlock[49] = {
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000
};

struct board {
	uint8_t state;
	uint16_t color;
};

struct board Buffer[10][16];

int16_t Data;        						// 12-bit ADC
uint32_t blocksPlaced = 0;
uint8_t currentType, currentRot, tempRot;
int8_t currentx, tempx, currenty, xSave, ySave;


int main(void) {
	PLL_Init(Bus80MHz);							// Bus clock is 80 MHz 
	Output_Init();
	Random_Init(NVIC_ST_CURRENT_R);		// seed
	Board_Init();
	Tetris_Init();
	ADC_Init();											// initialize slide pot
	ST7735_FillScreen(0xA2AE);
	Display_Init();
	PortE_Init();
	PortF_Init();
	Generate_Block();
	Timer0_Init();
//	Timer1_Init();
//	Timer2_Init();
	SysTick_Init();
//	Sound_Init();										// initialize sound
	EnableInterrupts();
	while (1) {
		if ((GPIO_PORTE_DATA_R & 0x01) == 1) {
			Wait10ms(2);
			while ((GPIO_PORTE_DATA_R & 0x01) == 1) {
				
			}
			Rotate_Block();
		}
		if ((GPIO_PORTE_DATA_R & 0x02) == 1) {
			TIMER0_TAILR_R = (80000000/240)-1;
			while ((GPIO_PORTE_DATA_R & 0x02) == 1) {
				
			}
			TIMER0_TAILR_R = (80000000/30-1);
		}
	}
}

/*
uint32_t Convert(uint32_t input) {
	uint32_t result;
	result = (input*(10/4.096));	
  return result;
}
*/

int16_t Floor(uint32_t in) {
	int16_t floor;
	in &= 0x00000FFF;
/*
	if (in < 1*409) {
		floor = 0;
	}
	else if ((1*409 <= in) && (in < 2*409)) {
		floor = 1;
	}
	else if ((2*409 <= in) && (in < 3*409)) {
		floor = 2;
	}
	else if ((3*409 <= in) && (in < 4*409)) {
		floor = 3;
	}
	else if ((4*409 <= in) && (in < 5*409)) {
		floor = 4;
	}
	else if ((5*409 <= in) && (in < 6*409)) {
		floor = 5;
	}
	else if ((6*409 <= in) && (in < 7*409)) {
		floor = 6;
	}
	else if ((7*409 <= in) && (in < 8*409)) {
		floor = 7;
	}
	else if ((8*409 <= in) && (in < 9*409)) {
		floor = 8;
	}
	else if (9*409 <= in) {
		floor = 9;
	}
*/
	if (in < 600) {
		floor = -1;
	}
	else if ((600 <= in) && (in < 1200)) {
		floor = 0;
	}
	else if ((1200 <= in) && (in < 2896)) {
		floor = 0;
	}
	else if ((2896 <= in) && (in < 3496)) {
		floor = 0;
		}
	else if ((3496 <= in) && (in < 4096)) {
		floor = 1;
	}
	return floor;
}

void Board_Init(void) {
	uint8_t i,j;
	for (i = 0; i < 16; i++) {
		for (j = 0; j < 10; j++) {
			Buffer[i][j].state = 0;
			Buffer[i][j].color = 0x0000;
		}
	}
}

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
		DisableInterrupts();
	}
}

void Check_Board(void) {
	uint8_t i, j, jSave, iCount;
	for (j = 0; j < 16; j++) {
		iCount = 0;
		for (i = 0; i < 10; i++) {
			iCount += Buffer[i][j].state;
		}
		if (iCount == 10) {
			jSave = j;
			while (j < 14) {
				for (i = 0; i < 10; i++) {
					Buffer[i][j].state = Buffer[i][j+1].state;
					Buffer[i][j].color = Buffer[i][j+1].color;
				}
				j++;
			}
			for (i = 0; i < 10; i++) {
				Buffer[i][15].state = 0;
				Buffer[i][15].color = 0x0000;
			}
			j = jSave;
		}
	}
}

int16_t Coordinate_ConversionX(int16_t t) {
	t = 8*t + 1;
	return t;
}

int16_t Coordinate_ConversionY(int16_t t) {
	t = (128-8*t)+30;
	return t;
}

void Display_Init(void) {
	int16_t i, j;
	uint8_t t1, t2;
	for (i = 0; i < 10; i++) {
		for (j = 0; j < 16; j++) {
			t1 = Coordinate_ConversionX(i);
			t2 = Coordinate_ConversionY(j);
			ST7735_DrawBitmap(t1, t2, nBlock, 7, 7);
		}
	}
}

void Display_Board(void) {
	int16_t i, j;
	uint16_t t1, t2;
	DisableInterrupts();
	for (i = 0; i < 10; i++) {
		for (j = 0; j < 16; j++) {
			t1 = Coordinate_ConversionX(i);
			t2 = Coordinate_ConversionY(j);
			if (Buffer[i][j].state == 0) {
				ST7735_DrawBitmap(t1, t2, nBlock, 7, 7);
			}
			else {
				switch (Buffer[i][j].color) {
					case (0x8410) : {
						ST7735_DrawBitmap(t1, t2, IBlock, 7, 7);
						break;
					}
					case (0xF800) : {
						ST7735_DrawBitmap(t1, t2, JBlock, 7, 7);
						break;
					}
					case (0x7FFF) : {
						ST7735_DrawBitmap(t1, t2, LBlock, 7, 7);
						break;
					}
					case (0x07E0) : {
						ST7735_DrawBitmap(t1, t2, OBlock, 7, 7);
						break;
					}
					case (0x04FF) : {
						ST7735_DrawBitmap(t1, t2, SBlock, 7, 7);
						break;
					}
					case (0xFFFF) : {
						ST7735_DrawBitmap(t1, t2, TBlock, 7, 7);
						break;
					}
					case (0x0F8F) : {
						ST7735_DrawBitmap(t1, t2, ZBlock, 7, 7);
						break;
					}
				}
			}
		}
	}
	EnableInterrupts();
}

void PortE_Init(void) {
	volatile unsigned long delay;
	SYSCTL_RCGCGPIO_R |= 0x10;
	delay = SYSCTL_RCGCGPIO_R;
	GPIO_PORTE_DEN_R |= 0x03;
	GPIO_PORTE_DIR_R &= ~0x03;
	GPIO_PORTE_AFSEL_R = 0x00;
	GPIO_PORTE_AMSEL_R = 0x00;
	GPIO_PORTE_PUR_R = 0x00;
}

void PortF_Init(void) {
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

void SysTick_Init(void) {
	NVIC_ST_CTRL_R = 0;									// disable SysTick during init
	NVIC_ST_RELOAD_R = (80000000/5)-1; // 60Hz interrupts
	NVIC_ST_CURRENT_R = 0;							// any write to CURRENT clears it
	NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R & 0x00FFFFFF)|0x60000000;
	NVIC_ST_CTRL_R = 0x00000007;				// enable interrupts, enable SysTick
}

void SysTick_Handler(void) {
	DisableInterrupts();
	PF2 ^= 0x04;      // Heartbeat
	Data = ADC_In();  // sample 12-bit channel 5
	PF2 ^= 0x04;			// ADC execution time
	currentx += Floor(Data);
	if (currentx < 0) {
		currentx = 0;
	}
	if (currentx > 9) {
		currentx = 9;
	}
	EnableInterrupts();
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
