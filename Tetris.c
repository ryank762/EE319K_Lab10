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
#include "UART.h"
#include "FiFo.h"

// Slide pot used for x-position of Tetris block
// Button0 (PE0) : Rotates block
// Button1 (PE1) : Fast-drops block

void DisableInterrupts(void);			// Disable interrupts
void EnableInterrupts(void);			// Enable interrupts
void Delay100ms(uint32_t count);	// time delay in 0.1 seconds
void Wait10ms(uint32_t n);				// time delay in 0.01 seconds
void Wait1ms(uint32_t count);		// time delay in 0.001 seconds
void Game_Over(void);							// displays "GAME OVER" screen
void FastDrop_Block(void);				// shortens time interval between block drop
void Board_Init(void);						// initializes buffer
void Timer2A_Handler(void);
void Display_Init(void);
void SysTick_Init(void);
void PortE_Init(void);
void PortF_Init(void);
void Display_Board(void);
void Game_Win(void);

int16_t Coordinate_ConversionX(int16_t);
int16_t Coordinate_ConversionY(int16_t);
uint32_t Convert(uint32_t input);	// converts ADC data into a floating point value from 0 to 10

#define PF1       (*((volatile uint32_t *)0x40025008))
#define PF2       (*((volatile uint32_t *)0x40025010))
#define PF3       (*((volatile uint32_t *)0x40025020))			
#define BGColor				0xA2AE
#define BGColor2			0x0000
#define textColor1		0xFFFF
#define textColor2		0x07FF

const uint16_t IBlock[49] = {
	0xFFE0,0xFFE0,0xFFE0,0xFFE0,0xFFE0,0xFFE0,0xFFE0,
	0xFFE0,0xFFE0,0xFFE0,0xFFE0,0xFFE0,0xFFE0,0xFFE0,
	0xFFE0,0xFFE0,0xFFE0,0xFFE0,0xFFE0,0xFFE0,0xFFE0,
	0xFFE0,0xFFE0,0xFFE0,0xFFE0,0xFFE0,0xFFE0,0xFFE0,
	0xFFE0,0xFFE0,0xFFE0,0xFFE0,0xFFE0,0xFFE0,0xFFE0,
	0xFFE0,0xFFE0,0xFFE0,0xFFE0,0xFFE0,0xFFE0,0xFFE0,
	0xFFE0,0xFFE0,0xFFE0,0xFFE0,0xFFE0,0xFFE0,0xFFE0
};

const uint16_t JBlock[49] = {
	0x04FF,0x04FF,0x04FF,0x04FF,0x04FF,0x04FF,0x04FF,
	0x04FF,0x04FF,0x04FF,0x04FF,0x04FF,0x04FF,0x04FF,
	0x04FF,0x04FF,0x04FF,0x04FF,0x04FF,0x04FF,0x04FF,
	0x04FF,0x04FF,0x04FF,0x04FF,0x04FF,0x04FF,0x04FF,
	0x04FF,0x04FF,0x04FF,0x04FF,0x04FF,0x04FF,0x04FF,
	0x04FF,0x04FF,0x04FF,0x04FF,0x04FF,0x04FF,0x04FF,
	0x04FF,0x04FF,0x04FF,0x04FF,0x04FF,0x04FF,0x04FF
};

const uint16_t LBlock[49] = {
	0xF800,0xF800,0xF800,0xF800,0xF800,0xF800,0xF800,
	0xF800,0xF800,0xF800,0xF800,0xF800,0xF800,0xF800,
	0xF800,0xF800,0xF800,0xF800,0xF800,0xF800,0xF800,
	0xF800,0xF800,0xF800,0xF800,0xF800,0xF800,0xF800,
	0xF800,0xF800,0xF800,0xF800,0xF800,0xF800,0xF800,
	0xF800,0xF800,0xF800,0xF800,0xF800,0xF800,0xF800,
	0xF800,0xF800,0xF800,0xF800,0xF800,0xF800,0xF800
};

const uint16_t OBlock[49] = {
	0x07FF,0x07FF,0x07FF,0x07FF,0x07FF,0x07FF,0x07FF,
	0x07FF,0x07FF,0x07FF,0x07FF,0x07FF,0x07FF,0x07FF,
	0x07FF,0x07FF,0x07FF,0x07FF,0x07FF,0x07FF,0x07FF,
	0x07FF,0x07FF,0x07FF,0x07FF,0x07FF,0x07FF,0x07FF,
	0x07FF,0x07FF,0x07FF,0x07FF,0x07FF,0x07FF,0x07FF,
	0x07FF,0x07FF,0x07FF,0x07FF,0x07FF,0x07FF,0x07FF,
	0x07FF,0x07FF,0x07FF,0x07FF,0x07FF,0x07FF,0x07FF
};

const uint16_t SBlock[49] = {
	0x001F,0x001F,0x001F,0x001F,0x001F,0x001F,0x001F,
	0x001F,0x001F,0x001F,0x001F,0x001F,0x001F,0x001F,
	0x001F,0x001F,0x001F,0x001F,0x001F,0x001F,0x001F,
	0x001F,0x001F,0x001F,0x001F,0x001F,0x001F,0x001F,
	0x001F,0x001F,0x001F,0x001F,0x001F,0x001F,0x001F,
	0x001F,0x001F,0x001F,0x001F,0x001F,0x001F,0x001F,
	0x001F,0x001F,0x001F,0x001F,0x001F,0x001F,0x001F
};

const uint16_t TBlock[49] = {
	0xF81F,0xF81F,0xF81F,0xF81F,0xF81F,0xF81F,0xF81F,
	0xF81F,0xF81F,0xF81F,0xF81F,0xF81F,0xF81F,0xF81F,
	0xF81F,0xF81F,0xF81F,0xF81F,0xF81F,0xF81F,0xF81F,
	0xF81F,0xF81F,0xF81F,0xF81F,0xF81F,0xF81F,0xF81F,
	0xF81F,0xF81F,0xF81F,0xF81F,0xF81F,0xF81F,0xF81F,
	0xF81F,0xF81F,0xF81F,0xF81F,0xF81F,0xF81F,0xF81F,
	0xF81F,0xF81F,0xF81F,0xF81F,0xF81F,0xF81F,0xF81F
};

const uint16_t ZBlock[49] = {
	0x0F8F,0x0F8F,0x0F8F,0x0F8F,0x0F8F,0x0F8F,0x0F8F,
	0x0F8F,0x0F8F,0x0F8F,0x0F8F,0x0F8F,0x0F8F,0x0F8F,
	0x0F8F,0x0F8F,0x0F8F,0x0F8F,0x0F8F,0x0F8F,0x0F8F,
	0x0F8F,0x0F8F,0x0F8F,0x0F8F,0x0F8F,0x0F8F,0x0F8F,
	0x0F8F,0x0F8F,0x0F8F,0x0F8F,0x0F8F,0x0F8F,0x0F8F,
	0x0F8F,0x0F8F,0x0F8F,0x0F8F,0x0F8F,0x0F8F,0x0F8F,
	0x0F8F,0x0F8F,0x0F8F,0x0F8F,0x0F8F,0x0F8F,0x0F8F
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

const uint16_t gBlock[49] = {
	0x4227,0x4227,0x4227,0x4227,0x4227,0x4227,0x4227,
	0x4227,0x4227,0x4227,0x4227,0x4227,0x4227,0x4227,
	0x4227,0x4227,0x4227,0x4227,0x4227,0x4227,0x4227,
	0x4227,0x4227,0x4227,0x4227,0x4227,0x4227,0x4227,
	0x4227,0x4227,0x4227,0x4227,0x4227,0x4227,0x4227,
	0x4227,0x4227,0x4227,0x4227,0x4227,0x4227,0x4227,
	0x4227,0x4227,0x4227,0x4227,0x4227,0x4227,0x4227
};

const uint16_t nextb1[36*18] = {
	0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,
	0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,
	0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,
	0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,
	0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,
	0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,
	0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,
	0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,
	0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,
	0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,
	0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,
	0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,
	0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,
	0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,
	0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,
	0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,
	0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,
	0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,
};

/*
const uint16_t nextb1[36*27] = {
	0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,
	0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,
	0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,
	0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,
	0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,
	0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,
	0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,
	0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,
	0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,
	0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,
	0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,
	0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,
	0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,
	0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,
	0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,
	0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,
	0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,
	0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,
	0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,
	0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,
	0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,
	0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,
	0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,
	0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,
	0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,
	0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,
	0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,0xA2AE,
};
*/

struct board {
	uint8_t state;
	uint16_t color;
};

struct board Buffer[10][16];

int16_t Data;        						// 12-bit ADC
uint32_t Position = 0;
uint32_t TxCounter = 0;
char data;


uint32_t linesCleared = 0;
uint32_t blocksPlaced = 0;
uint32_t fdropCount = 0;
uint32_t score = 0;
uint8_t currentType, currentRot, tempRot, nextBlock;
uint8_t level = 0;
uint8_t startLine = 0;
uint8_t rotFlag = 0;
uint8_t cFlag = 0;
uint8_t uartFlag = 0;
uint8_t genFlag = 0;
int8_t currentx, tempx, currenty, tempy, xSave, ySave;


int main(void) {
	PLL_Init(Bus80MHz);							// Bus clock is 80 MHz 
	Output_Init();
	Random_Init(1);									// seed
	nextBlock = 3;
	Board_Init();
	Tetris_Init();
	PortE_Init();
	PortF_Init();
	UART_Init();
	ADC_Init();											// initialize slide pot
	ST7735_FillScreen(BGColor);
	ST7735_DrawBitmap(5, 11, SBlock, 5, 5);
	ST7735_DrawBitmap(11, 11, SBlock, 5, 5);
	ST7735_DrawBitmap(17, 11, SBlock, 5, 5);
	ST7735_DrawBitmap(11, 17, SBlock, 5, 5);
	ST7735_DrawBitmap(11, 23, SBlock, 5, 5);
	ST7735_DrawBitmap(11, 29, SBlock, 5, 5);
	ST7735_DrawBitmap(11, 35, SBlock, 5, 5);
//
	ST7735_DrawBitmap(26, 11, JBlock, 5, 5);
	ST7735_DrawBitmap(26, 17, JBlock, 5, 5);
	ST7735_DrawBitmap(26, 23, JBlock, 5, 5);
	ST7735_DrawBitmap(26, 29, JBlock, 5, 5);
	ST7735_DrawBitmap(26, 35, JBlock, 5, 5);
	ST7735_DrawBitmap(32, 11, JBlock, 5, 5);
	ST7735_DrawBitmap(38, 11, JBlock, 5, 5);
	ST7735_DrawBitmap(32, 23, JBlock, 5, 5);
	ST7735_DrawBitmap(38, 23, JBlock, 5, 5);
	ST7735_DrawBitmap(32, 35, JBlock, 5, 5);
	ST7735_DrawBitmap(38, 35, JBlock, 5, 5);
//
	ST7735_DrawBitmap(47, 11, OBlock, 5, 5);
	ST7735_DrawBitmap(53, 11, OBlock, 5, 5);
	ST7735_DrawBitmap(59, 11, OBlock, 5, 5);
	ST7735_DrawBitmap(53, 17, OBlock, 5, 5);
	ST7735_DrawBitmap(53, 23, OBlock, 5, 5);
	ST7735_DrawBitmap(53, 29, OBlock, 5, 5);
	ST7735_DrawBitmap(53, 35, OBlock, 5, 5);
//
	ST7735_DrawBitmap(68, 11, ZBlock, 5, 5);
	ST7735_DrawBitmap(68, 17, ZBlock, 5, 5);
	ST7735_DrawBitmap(68, 23, ZBlock, 5, 5);
	ST7735_DrawBitmap(68, 29, ZBlock, 5, 5);
	ST7735_DrawBitmap(68, 35, ZBlock, 5, 5);
	ST7735_DrawBitmap(74, 11, ZBlock, 5, 5);
	ST7735_DrawBitmap(74, 23, ZBlock, 5, 5);
	ST7735_DrawBitmap(74, 29, ZBlock, 5, 5);
	ST7735_DrawBitmap(80, 11, ZBlock, 5, 5);
	ST7735_DrawBitmap(80, 17, ZBlock, 5, 5);
	ST7735_DrawBitmap(80, 23, ZBlock, 5, 5);
	ST7735_DrawBitmap(80, 35, ZBlock, 5, 5);
//
	ST7735_DrawBitmap(89, 11, IBlock, 5, 5);
	ST7735_DrawBitmap(89, 17, IBlock, 5, 5);
	ST7735_DrawBitmap(89, 23, IBlock, 5, 5);
	ST7735_DrawBitmap(89, 29, IBlock, 5, 5);
	ST7735_DrawBitmap(89, 35, IBlock, 5, 5);
//
	ST7735_DrawBitmap(98, 11, TBlock, 5, 5);
	ST7735_DrawBitmap(104, 11, TBlock, 5, 5);
	ST7735_DrawBitmap(110, 11, TBlock, 5, 5);
	ST7735_DrawBitmap(98, 17, TBlock, 5, 5);
	ST7735_DrawBitmap(98, 23, TBlock, 5, 5);
	ST7735_DrawBitmap(104, 23, TBlock, 5, 5);
	ST7735_DrawBitmap(110, 23, TBlock, 5, 5);
	ST7735_DrawBitmap(110, 29, TBlock, 5, 5);
	ST7735_DrawBitmap(98, 35, TBlock, 5, 5);
	ST7735_DrawBitmap(104, 35, TBlock, 5, 5);
	ST7735_DrawBitmap(110, 35, TBlock, 5, 5);
//
	ST7735_DrawCharS(15, 75, 'B', textColor2, BGColor, 1);
	ST7735_DrawCharS(20, 75, 'u', textColor2, BGColor, 1);
	ST7735_DrawCharS(25, 75, 't', textColor2, BGColor, 1);
	ST7735_DrawCharS(30, 75, 't', textColor2, BGColor, 1);
	ST7735_DrawCharS(35, 75, 'o', textColor2, BGColor, 1);
	ST7735_DrawCharS(40, 75, 'n', textColor2, BGColor, 1);
	ST7735_DrawCharS(45, 75, ' ', textColor2, BGColor, 1);
	ST7735_DrawCharS(50, 75, '1', textColor2, BGColor, 1);
	ST7735_DrawCharS(55, 75, ':', textColor2, BGColor, 1);
	ST7735_DrawCharS(60, 75, ' ', textColor2, BGColor, 1);
	ST7735_DrawCharS(65, 75, 'R', textColor2, BGColor, 1);
	ST7735_DrawCharS(71, 75, 'O', textColor2, BGColor, 1);
	ST7735_DrawCharS(77, 75, 'T', textColor2, BGColor, 1);
	ST7735_DrawCharS(83, 75, 'A', textColor2, BGColor, 1);
	ST7735_DrawCharS(89, 75, 'T', textColor2, BGColor, 1);
	ST7735_DrawCharS(95, 75, 'E', textColor2, BGColor, 1);
//
	ST7735_DrawCharS(15, 85, 'B', textColor2, BGColor, 1);
	ST7735_DrawCharS(20, 85, 'u', textColor2, BGColor, 1);
	ST7735_DrawCharS(25, 85, 't', textColor2, BGColor, 1);
	ST7735_DrawCharS(30, 85, 't', textColor2, BGColor, 1);
	ST7735_DrawCharS(35, 85, 'o', textColor2, BGColor, 1);
	ST7735_DrawCharS(40, 85, 'n', textColor2, BGColor, 1);
	ST7735_DrawCharS(45, 85, ' ', textColor2, BGColor, 1);
	ST7735_DrawCharS(50, 85, '2', textColor2, BGColor, 1);
	ST7735_DrawCharS(55, 85, ':', textColor2, BGColor, 1);
	ST7735_DrawCharS(60, 85, ' ', textColor2, BGColor, 1);
	ST7735_DrawCharS(65, 85, 'F', textColor2, BGColor, 1);
	ST7735_DrawCharS(71, 85, 'A', textColor2, BGColor, 1);
	ST7735_DrawCharS(77, 85, 'S', textColor2, BGColor, 1);
	ST7735_DrawCharS(83, 85, 'T', textColor2, BGColor, 1);
	ST7735_DrawCharS(89, 85, 'D', textColor2, BGColor, 1);
	ST7735_DrawCharS(95, 85, 'R', textColor2, BGColor, 1);
	ST7735_DrawCharS(101, 85, 'O', textColor2, BGColor, 1);
	ST7735_DrawCharS(108, 85, 'P', textColor2, BGColor, 1);
//
	ST7735_DrawCharS(20, 115, 'P', textColor1, BGColor, 1);
	ST7735_DrawCharS(25, 115, 'r', textColor1, BGColor, 1);
	ST7735_DrawCharS(30, 115, 'e', textColor1, BGColor, 1);
	ST7735_DrawCharS(35, 115, 's', textColor1, BGColor, 1);
	ST7735_DrawCharS(40, 115, 's', textColor1, BGColor, 1);
	ST7735_DrawCharS(45, 115, ' ', textColor1, BGColor, 1);
	ST7735_DrawCharS(50, 115, 'a', textColor1, BGColor, 1);
	ST7735_DrawCharS(55, 115, 'n', textColor1, BGColor, 1);
	ST7735_DrawCharS(60, 115, 'y', textColor1, BGColor, 1);
	ST7735_DrawCharS(65, 115, ' ', textColor1, BGColor, 1);
	ST7735_DrawCharS(70, 115, 'b', textColor1, BGColor, 1);
	ST7735_DrawCharS(75, 115, 'u', textColor1, BGColor, 1);
	ST7735_DrawCharS(80, 115, 't', textColor1, BGColor, 1);
	ST7735_DrawCharS(85, 115, 't', textColor1, BGColor, 1);
	ST7735_DrawCharS(90, 115, 'o', textColor1, BGColor, 1);
	ST7735_DrawCharS(95, 115, 'n', textColor1, BGColor, 1);
//
	ST7735_DrawCharS(40, 125, 't', textColor1, BGColor, 1);
	ST7735_DrawCharS(45, 125, 'o', textColor1, BGColor, 1);
	ST7735_DrawCharS(50, 125, ' ', textColor1, BGColor, 1);
	ST7735_DrawCharS(55, 125, 's', textColor1, BGColor, 1);
	ST7735_DrawCharS(60, 125, 't', textColor1, BGColor, 1);
	ST7735_DrawCharS(65, 125, 'a', textColor1, BGColor, 1);
	ST7735_DrawCharS(70, 125, 'r', textColor1, BGColor, 1);
	ST7735_DrawCharS(75, 125, 't', textColor1, BGColor, 1);
	ST7735_DrawCharS(80, 125, '!', textColor1, BGColor, 1);
//	ST7735_OutString("Press any button");
//	ST7735_SetCursor(4, 2);
//	ST7735_OutString("to play!");
	while ((GPIO_PORTE_DATA_R & 0x03) == 0){
	}
	Wait10ms(2);
	while ((GPIO_PORTE_DATA_R & 0x03)) {
	}
	ST7735_FillScreen(0xA2AE);
	Display_Init();
	Generate_Block();
	Display_Board();
	Timer0_Init();
//	Timer1_Init();
//	Timer2_Init();
	SysTick_Init();
	Sound_Init();										// initialize sound
	EnableInterrupts();
	Sound_Start();
	while (1) {
		if ((GPIO_PORTE_DATA_R & 0x02)) {
			Wait10ms(2);
			while ((GPIO_PORTE_DATA_R & 0x02)) {
			}
			PF1 ^= 0x02;
			FastDrop_Block();
			PF1 ^= 0x02;
			Delay100ms(2);
		}
		if ((GPIO_PORTE_DATA_R & 0x01) == 1) {
			Wait10ms(2);
			while ((GPIO_PORTE_DATA_R & 0x01) == 1) {
			}
			PF3 ^= 0x08;
			Rotate_Block();
			PF3 ^= 0x08;
			Sound_Rotate();
			Delay100ms(2);
		}
		/*
		int t;													// index
		uint8_t input[]={0x00,0x2E,0x00,0x00,0x00};			// base output array
		if (FiFo_Get(&data) != 0) {
			for(t = 1; t < 7; t++){
				FiFo_Get(&data);							// update data
				input[t] = data;
			}
		}
		else {
			input[0] = 0x00;
			input[1] = 0x2E;
			input[2] = 0x00;
			input[3] = 0x00;
			input[4] = 0x00;
		}
		if ((input[0] == 0x31) ||
				(input[2] == 0x31) ||
				(input[3] == 0x31) ||
				(input[4] == 0x31) ) {
			Add_Line();
		}
		if ((input[0] == 0x31) ||
				(input[2] == 0x31) ||
				(input[3] == 0x31) ||
				(input[4] == 0x31) ) {
			Game_Win();
		}
		*/
	}
}

int16_t Floor(uint32_t in) {
	int16_t floor;
	in &= 0x00000FFF;
	if (in < 600) {
		floor = 1;
	}
	else if ((600 <= in) && (in < 1200)) {
		floor = 1;
	}
	else if ((1200 <= in) && (in < 2896)) {
		floor = 0;
	}
	else if ((2896 <= in) && (in < 3496)) {
		floor = 0;
		}
	else if ((3496 <= in) && (in < 4096)) {
		floor = -1;
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

void Game_Win(void) {
	ST7735_FillScreen(0x0000);
	ST7735_SetCursor(3, 1);
	ST7735_OutString("YOU WIN!");
	ST7735_SetCursor(3, 2);
	ST7735_OutString("Your score:");
	ST7735_SetCursor(3, 3);
	LCD_OutDec(score);
	while (1) {
		DisableInterrupts();
	}
}

void Game_Over(void) {
	DisableInterrupts();
	ST7735_FillScreen(BGColor2);
	ST7735_FillScreen(BGColor2);
	ST7735_DrawBitmap(5, 11, SBlock, 5, 5);
	ST7735_DrawBitmap(11, 11, SBlock, 5, 5);
	ST7735_DrawBitmap(17, 11, SBlock, 5, 5);
	ST7735_DrawBitmap(11, 17, SBlock, 5, 5);
	ST7735_DrawBitmap(11, 23, SBlock, 5, 5);
	ST7735_DrawBitmap(11, 29, SBlock, 5, 5);
	ST7735_DrawBitmap(11, 35, SBlock, 5, 5);
//
	ST7735_DrawBitmap(26, 11, JBlock, 5, 5);
	ST7735_DrawBitmap(26, 17, JBlock, 5, 5);
	ST7735_DrawBitmap(26, 23, JBlock, 5, 5);
	ST7735_DrawBitmap(26, 29, JBlock, 5, 5);
	ST7735_DrawBitmap(26, 35, JBlock, 5, 5);
	ST7735_DrawBitmap(32, 11, JBlock, 5, 5);
	ST7735_DrawBitmap(38, 11, JBlock, 5, 5);
	ST7735_DrawBitmap(32, 23, JBlock, 5, 5);
	ST7735_DrawBitmap(38, 23, JBlock, 5, 5);
	ST7735_DrawBitmap(32, 35, JBlock, 5, 5);
	ST7735_DrawBitmap(38, 35, JBlock, 5, 5);
//
	ST7735_DrawBitmap(47, 11, OBlock, 5, 5);
	ST7735_DrawBitmap(53, 11, OBlock, 5, 5);
	ST7735_DrawBitmap(59, 11, OBlock, 5, 5);
	ST7735_DrawBitmap(53, 17, OBlock, 5, 5);
	ST7735_DrawBitmap(53, 23, OBlock, 5, 5);
	ST7735_DrawBitmap(53, 29, OBlock, 5, 5);
	ST7735_DrawBitmap(53, 35, OBlock, 5, 5);
//
	ST7735_DrawBitmap(68, 11, ZBlock, 5, 5);
	ST7735_DrawBitmap(68, 17, ZBlock, 5, 5);
	ST7735_DrawBitmap(68, 23, ZBlock, 5, 5);
	ST7735_DrawBitmap(68, 29, ZBlock, 5, 5);
	ST7735_DrawBitmap(68, 35, ZBlock, 5, 5);
	ST7735_DrawBitmap(74, 11, ZBlock, 5, 5);
	ST7735_DrawBitmap(74, 23, ZBlock, 5, 5);
	ST7735_DrawBitmap(74, 29, ZBlock, 5, 5);
	ST7735_DrawBitmap(80, 11, ZBlock, 5, 5);
	ST7735_DrawBitmap(80, 17, ZBlock, 5, 5);
	ST7735_DrawBitmap(80, 23, ZBlock, 5, 5);
	ST7735_DrawBitmap(80, 35, ZBlock, 5, 5);
//
	ST7735_DrawBitmap(89, 11, IBlock, 5, 5);
	ST7735_DrawBitmap(89, 17, IBlock, 5, 5);
	ST7735_DrawBitmap(89, 23, IBlock, 5, 5);
	ST7735_DrawBitmap(89, 29, IBlock, 5, 5);
	ST7735_DrawBitmap(89, 35, IBlock, 5, 5);
//
	ST7735_DrawBitmap(98, 11, TBlock, 5, 5);
	ST7735_DrawBitmap(104, 11, TBlock, 5, 5);
	ST7735_DrawBitmap(110, 11, TBlock, 5, 5);
	ST7735_DrawBitmap(98, 17, TBlock, 5, 5);
	ST7735_DrawBitmap(98, 23, TBlock, 5, 5);
	ST7735_DrawBitmap(104, 23, TBlock, 5, 5);
	ST7735_DrawBitmap(110, 23, TBlock, 5, 5);
	ST7735_DrawBitmap(110, 29, TBlock, 5, 5);
	ST7735_DrawBitmap(98, 35, TBlock, 5, 5);
	ST7735_DrawBitmap(104, 35, TBlock, 5, 5);
	ST7735_DrawBitmap(110, 35, TBlock, 5, 5);
//
	ST7735_SetCursor(5, 5);
	ST7735_OutString("GAME OVER!");
	ST7735_SetCursor(2, 6);
	ST7735_OutString("Your score: ");
	LCD_OutDec(score);
	ST7735_SetCursor(2, 7);
	ST7735_OutString("Your level: ");
	LCD_OutDec(level+1);
	ST7735_SetCursor(1, 10);
	ST7735_OutString("Thanks for playing!");
//
	ST7735_SetCursor(5, 12);
	ST7735_OutString("Ryan Kim");
	ST7735_SetCursor(4, 13);
	ST7735_OutString("Rafay Khurram");
	while (1) {
		Sound_Gameover();
		DisableInterrupts();
	}
}

void Check_Board(void) {	
	int8_t i, j, jSave, iCount, ltemp, lev;
	for (j = startLine; j < 16; j++) {
		iCount = 1;
		for (i = 0; i < 10; i++) {
			if (Buffer[i][j].state == 0) {
				iCount = 0;
				break;
			}
		}
		if (iCount == 1) {
			cFlag = 2;
			linesCleared++;
			ltemp = 4;
			lev = 8;
			if (linesCleared >= ltemp) {
				uartFlag = 1;
				ltemp += 4;
			}
			if (linesCleared >= lev) {
				level++;
				TimerCount = 0;
				lev += 8;
			}
			for (jSave = j; jSave < 15; jSave++) {
				for (i = 0; i < 10; i++) {
//					Buffer[i][j].state = 0;
//					Buffer[i][j].color = 0x0000;
					Buffer[i][jSave].state = Buffer[i][jSave+1].state;
					Buffer[i][jSave].color = Buffer[i][jSave+1].color;
				}
			}
			for (i = 0; i < 10; i++) {
				Buffer[i][15].state = 0;
				Buffer[i][15].color = 0x0000;
			}
			Display_Board();	
			j--;
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
//	ST7735_DrawBitmap(85, 60, nextb1, 36, 27);
	ST7735_DrawBitmap(85, 60, nextb1, 36, 18);
	ST7735_DrawCharS(85, 33, 'N', textColor1, BGColor, 1);
	ST7735_DrawCharS(91, 33, 'E', textColor1, BGColor, 1);
	ST7735_DrawCharS(97, 33, 'X', textColor1, BGColor, 1);
	ST7735_DrawCharS(103, 33, 'T', textColor1, BGColor, 1);
}

void Display_Board(void) {
	int16_t i, j;
	uint16_t t1, t2;
	for (i = 0; i < 10; i++) {
		for (j = 0; j < 16; j++) {
			t1 = Coordinate_ConversionX(i);
			t2 = Coordinate_ConversionY(j);
			if (Buffer[i][j].color != 0x0000) {
				switch (Buffer[i][j].color) {
					case (0xFFE0) : {
						ST7735_DrawBitmap(t1, t2, IBlock, 7, 7);
						break;
					}
					case (0x04FF) : {
						ST7735_DrawBitmap(t1, t2, JBlock, 7, 7);
						break;
					}
					case (0xF800) : {
						ST7735_DrawBitmap(t1, t2, LBlock, 7, 7);
						break;
					}
					case (0x07FF) : {
						ST7735_DrawBitmap(t1, t2, OBlock, 7, 7);
						break;
					}
					case (0x001F) : {
						ST7735_DrawBitmap(t1, t2, SBlock, 7, 7);
						break;
					}
					case (0xF81F) : {
						ST7735_DrawBitmap(t1, t2, TBlock, 7, 7);
						break;
					}
					case (0x0F8F) : {
						ST7735_DrawBitmap(t1, t2, ZBlock, 7, 7);
						break;
					}
					case (0x4227) : {
						ST7735_DrawBitmap(t1, t2, gBlock, 7, 7);
						break;
					}
				}
			}
			else if (Buffer[i][j].state == 0) {
				ST7735_DrawBitmap(t1, t2, nBlock, 7, 7);
			}
		}
	}
}

void PortE_Init(void) {
	volatile unsigned long delay;
	SYSCTL_RCGCGPIO_R |= 0x10;
	delay = SYSCTL_RCGCGPIO_R;
	GPIO_PORTE_DEN_R |= 0x03;
	GPIO_PORTE_DIR_R &= ~0x03;
//	GPIO_PORTE_PCTL_R &= ~0x000F0000;
	GPIO_PORTE_PUR_R &= ~0x03;
	GPIO_PORTE_PDR_R |= 0x03;
}

void PortF_Init(void) {
	SYSCTL_RCGCGPIO_R |= 0x20;						// enable PF clock
	volatile int clockDelay;
	clockDelay = SYSCTL_RCGCGPIO_R;				// wait until stable
	GPIO_PORTF_LOCK_R = GPIO_LOCK_KEY;		// unlock PF
	GPIO_PORTF_CR_R |= 0x1F;							// allow access
	GPIO_PORTF_DEN_R |= 0x1E;							// enable digital logic on PF1-4
	GPIO_PORTF_DIR_R |= 0x0E;							// make PF1-3 output
	GPIO_PORTF_DIR_R &= ~0x11;
	GPIO_PORTF_AFSEL_R &= ~0x1E;
	GPIO_PORTF_AMSEL_R &= ~0x1E;					// disable AF and AM for PF
	GPIO_PORTF_PCTL_R &= ~0x000F0000;
	GPIO_PORTF_PUR_R &= ~0x1E;						// disable negative logic on PF
	GPIO_PORTF_IS_R &= ~0x10;
	GPIO_PORTF_IBE_R &= ~0x10;
	GPIO_PORTF_IEV_R &= ~0x10;
	GPIO_PORTF_ICR_R = 0x10;
	GPIO_PORTF_IM_R |= 0x10;
	NVIC_PRI7_R = (NVIC_PRI7_R & 0xFF00FFFF)|0x00A00000;
	NVIC_EN0_R = 0x40000000;
	EnableInterrupts();
}

void GPIOPortF_Handler(void) {
	GPIO_PORTF_ICR_R = 0x10;
	Rotate_Block();
}

void SysTick_Init(void) {
	NVIC_ST_CTRL_R = 0;									// disable SysTick during init
	NVIC_ST_RELOAD_R = (80000000/5)-1; // 5Hz interrupts
	NVIC_ST_CURRENT_R = 0;							// any write to CURRENT clears it
	NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R & 0x00FFFFFF)|0x60000000;
	NVIC_ST_CTRL_R = 0x00000007;				// enable interrupts, enable SysTick
}

void SysTick_Handler(void) {
	int16_t c, d;
	Data = ADC_In();  // sample 12-bit channel 5
	if (genFlag) {
		Random_Init(Data);
		genFlag = 0;
	}
	c = currentx + Floor(Data);
	if (c < 0) {
		c = 0;
	}
	if (c > 9) {
		c = 9;
	}
	d = tempx;
	Erase_Block(currentType, currentRot, tempx, currenty);
	if (Check_Collision(currentType, currentRot, c, currenty) == 1) {
		if (c != currentx) {
			Sound_Blockhit();
		}
		currentx = c;
	} 
	Place_Block(currentType, currentRot, currentx, currenty);
	tempx = d;
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

void Wait1ms(uint32_t count) {
	uint32_t volatile time;
	while (count > 0) {
		time = 7272;  // 0.1sec at 80 MHz
		while (time) {
			time--;
		}
		count--;
	}
}
