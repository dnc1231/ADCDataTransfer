// FiFo.c
// Runs on LM4F120/TM4C123
// Provide functions that implement the Software FiFo Buffer
// Last Modified: 4/10/2017 
// Student names: change this to your names or look very silly
// Last modification date: change this to the last modification date or look very silly

#include <stdint.h>
#include "UART.h"//we added this
#define FIFO_SIZE 10
int get;
int put;
char static Fifo[FIFO_SIZE];

// --UUU-- Declare state variables for FiFo
//        size, buffer, put and get indexes

// *********** FiFo_Init**********
// Initializes a software FIFO of a
// fixed size and sets up indexes for
// put and get operations
void FiFo_Init() {
// --UUU-- Complete this
	put = get = 0;
}

// *********** FiFo_Put**********
// Adds an element to the FIFO
// Input: Character to be inserted
// Output: 1 for success and 0 for failure
//         failure is when the buffer is full
int FiFo_Put(char data) {
	// --UUU-- Complete this routine
	if (((put+1)%10)==get){
		return (0);
	}
  else{
		Fifo[put]=data;
		put=(put+1)%10;
		return (1);
	}
}

// *********** FiFo_Get**********
// Gets an element from the FIFO
// Input: Pointer to a character that will get the character read from the buffer
// Output: 1 for success and 0 for failure
//         failure is when the buffer is empty
int FiFo_Get(char *datapt)
{
	if (put==get){
		return (0);
		}
		*datapt=Fifo[get];
		get=(get+1)%10;
		return (1);
	}

