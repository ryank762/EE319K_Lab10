// FiFo.c
// Runs on LM4F120/TM4C123
// Provide functions that implement the Software FiFo Buffer
// Last Modified: 4/10/2017 
// Student names: Ryan Kim, Rafay Khurram
// Last modification date: 4/17/18

#include <stdint.h>
// --UUU-- Declare state variables for FiFo
//        size, buffer, put and get indexes
#define SIZE 8
uint8_t static PutI; // should be 0 to SIZE-1
uint8_t static GetI; // should be 0 to SIZE-1
char FIFO[SIZE];
// *********** FiFo_Init**********
// Initializes a software FIFO of a
// fixed size and sets up indexes for
// put and get operations
void FiFo_Init(){
	PutI = GetI = 0; // empty
}

// *********** FiFo_Put**********
// Adds an element to the FIFO
// Input: Character to be inserted
// Output: 1 for success and 0 for failure
//         failure is when the buffer is full
uint32_t FiFo_Put(char data){
	if(GetI == ((PutI + 1)%SIZE)){
		return 0;
	}
	else{
		FIFO[PutI] = data;
		PutI = (PutI+1)%SIZE;
		return 1;
	}
}

// *********** FiFo_Get**********
// Gets an element from the FIFO
// Input: Pointer to a character that will get the character read from the buffer
// Output: 1 for success and 0 for failure
//         failure is when the buffer is empty
uint32_t FiFo_Get(char *datapt){
	if(GetI == PutI){
		return 0;
	}
	*datapt = FIFO[GetI];
	GetI = (GetI + 1)%SIZE;
	return 1;
}



