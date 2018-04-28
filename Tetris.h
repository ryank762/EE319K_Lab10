// Tetris.h
// 4/23/18
// Ryan Kim

#include <stdint.h>

extern const uint16_t IBlock;

extern const uint16_t JBlock;

extern const uint16_t LBlock;

extern const uint16_t OBlock;

extern const uint16_t SBlock;

extern const uint16_t TBlock;

extern const uint16_t ZBlock;

extern const uint16_t nBlock;

struct board {
	uint8_t state;
	uint16_t color;
};

extern struct board Buffer[16][10];

extern uint8_t currentType, currentRot;

extern int8_t currentx, currenty;

void Game_Over(void); 
