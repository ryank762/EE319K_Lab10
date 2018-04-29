// Blocks.h
// 4/23/18
// Ryan Kim

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
