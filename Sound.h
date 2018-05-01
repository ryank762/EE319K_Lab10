// Sound.h
// Runs on TM4C123 or LM4F120
// Prototypes for basic functions to play sounds from the
// original Space Invaders.
// Jonathan Valvano
// November 17, 2014
#include <stdint.h>
#include "tm4c123gh6pm.h"

void Sound_Init(void);
//void Play(void);
//void Sound_Play(const unsigned char *pt, unsigned long count);
//void StopMusic(void);
void Sound_Rotate(void);
void Sound_Blockhit(void);
void Sound_Gameover(void);
void Sound_Song(void);
void Sound_Play1(const unsigned char *sound, uint32_t length);
void Sound_Fail(void);

void Sound_Start(void);
void Sound_NewLine(void);

void Sound_Explosion(void);


void Sound_Killed(void);

void NextSound(void);

void Sound_Play2(const unsigned char *sound, uint32_t length);

void ButtonSound(void);

