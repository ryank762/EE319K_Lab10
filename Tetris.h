// Tetris.h
// 4/23/18
// Ryan Kim

#include <stdint.h>

extern const uint16_t IBlock, JBlock, LBlock, OBlock, SBlock, TBlock, ZBlock, nBlock, gBlock;

struct board {
	uint8_t state;
	uint16_t color;
};

extern struct board Buffer[10][16];

extern uint8_t currentType, currentRot, tempRot, nextBlock, level, startLine;

extern int8_t currentx, tempx, currenty, xSave, ySave;

extern uint32_t linesCleared, blocksPlaced, fdropCount, score;

int16_t Floor(uint32_t in);

void Game_Over(void); 

void Drop_Block(void);

void Check_Board(void);

void Display_Board(void);
