// Tetris.h
// 4/23/18
// Ryan Kim

#include <stdint.h>

struct board {
	uint8_t state;
	uint16_t color;
};

extern struct board Buffer[16][10];

extern uint8_t currentType, currentRot;

extern int8_t currentx, currenty;

void Game_Over(void); 
