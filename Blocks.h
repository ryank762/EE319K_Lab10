// Blocks.h
// 4/23/18
// Ryan Kim, Rafay Khurram

struct block {
	int16_t p0[2], p1[2], p2[2], p3[2];
};

typedef struct block block_t;

extern block_t Tetris[7][4];

void Tetris_Init(void);

void Rotate_Block(void);

void Drop_Block(void);

void FastDrop_Block(void);

void Generate_Block(void);

void Add_Line(void);

void Place_Block(uint8_t type, uint8_t rot, int16_t x, int16_t y);

void Erase_Block(uint8_t type, uint8_t rot, int16_t x, int16_t y);

uint8_t Check_Collision(uint8_t type, uint8_t rot, int16_t x2, int16_t y2);

// extern const uint16_t IBlock0, IBlock1, JBlock0, JBlock1, JBlock2, JBlock3, LBlock0, LBlock1, LBlock2, LBlock3, OBlock1, SBlock0, SBlock1, TBlock0, TBlock1, TBlock2, TBlock3, ZBlock0, ZBlock1;
