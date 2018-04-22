// Lab9.c
// Runs on LM4F120 or TM4C123
// Student names: change this to your names or look very silly
// Last modification date: change this to the last modification date or look very silly
// Last Modified: 4/10/2017 

// Analog Input connected to PE2=ADC1
// displays on Sitronox ST7735
// PF3, PF2, PF1 are heartbeats
// UART1 on PC4-5
// * Start with where you left off in Lab8. 
// * Get Lab8 code working in this project.
// * Understand what parts of your main have to move into the UART1_Handler ISR
// * Rewrite the SysTickHandler
// * Implement the s/w FiFo on the receiver end 
//    (we suggest implementing and testing this first)

#include <stdint.h>

#include "ST7735.h"
#include "TExaS.h"
#include "ADC.h"
#include "print.h"
#include "tm4c123gh6pm.h"
#include "UART.h"
#include "FiFo.h"

//*****the first three main programs are for debugging *****
// main1 tests just the ADC and slide pot, use debugger to see data
// main2 adds the LCD to the ADC and slide pot, ADC data is on Nokia
// main3 adds your convert function, position data is no Nokia

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts

#define PF1       (*((volatile uint32_t *)0x40025008))
#define PF2       (*((volatile uint32_t *)0x40025010))
#define PF3       (*((volatile uint32_t *)0x40025020))
uint32_t Data;      // 12-bit ADC
uint32_t Position;  // 32-bit fixed-point 0.001 cm
int TxCounter=0;
int array[8];
int error_count;
volatile int delay1;

// Initialize Port F so PF1, PF2 and PF3 are heartbeats
void PortF_Init(void){
	SYSCTL_RCGCGPIO_R|=0x20;
	delay1=100;
	delay1=100;
	GPIO_PORTF_DIR_R|=0x0E;	
	GPIO_PORTF_LOCK_R=GPIO_LOCK_KEY;
	GPIO_PORTF_CR_R|=0x0E;

	GPIO_PORTF_DEN_R|=0x0E;
	GPIO_PORTF_AFSEL_R&=~(0x0E);//turn on pf123 for heartbeats, no inputs, only outputs, disable af and am
	GPIO_PORTF_AMSEL_R&=~(0x0E);
	GPIO_PORTF_PCTL_R&=~(0x0E);
// Intialize PortF for hearbeat
}

// Get fit from excel and code the convert routine with the constants
// from the curve-fit
uint32_t Convert(uint32_t input){
  input=((4373*input)+1984300)/10000;						
  return input; //replace with your calibration code
}

	void SysTick_Init(uint32_t period){
	NVIC_ST_CTRL_R = 0; //disable the interrupt during setup
	NVIC_ST_RELOAD_R = period - 1; //reload value (2*10^6)
	NVIC_ST_CURRENT_R = 0; //clears data in current
	NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R&0x00FFFFFF)|0x20000000; //priority 2
	NVIC_ST_CTRL_R = 0x00000007; //Enable the interrupts
	//SYSCTL_RCGC2_R |=0x20;

}
// final main program for bidirectional communication
// Sender sends using SysTick Interrupt
// Receiver receives using RX
int main(void){
  TExaS_Init();       // Bus clock is 80 MHz 
	DisableInterrupts();
	ADC_Init();    // initialize to sample ADC  
	PortF_Init();
	SysTick_Init(1160000);  
	UART_Init();       // initialize UART
	FiFo_Init();	
	ST7735_InitR(INITR_REDTAB);
	ST7735_SetCursor(0,0);
  LCD_OutFix(0);
  ST7735_OutString(" cm");
	//Enable SysTick Interrupt by calling SysTick_Init()
  EnableInterrupts();


	char data=0;
	char digit1=0;
	char digit2=0;
	char digit3=0;
	char digit4=0;
	char dot=0;
	int result=0;
  while(1){

		FiFo_Get(&data);
		if (data==0x02){
			ST7735_SetCursor(0,0);
			result=FiFo_Get(&digit1);
				if(result==0){
					error_count++;
				}
			ST7735_OutChar(digit1);
			result=FiFo_Get(&dot);
				if(result==0){
					error_count++;
				}
			ST7735_OutChar(dot);
			result=FiFo_Get(&digit2);
				if(result==0){
					error_count++;
				}
			ST7735_OutChar(digit2);
			result=FiFo_Get(&digit3);
				if(result==0){
					error_count++;
				}
			ST7735_OutChar(digit3);
			result=FiFo_Get(&digit4);
				if(result==0){
					error_count++;
				}
			ST7735_OutChar(digit4);
			FiFo_Get(&data);
			FiFo_Get(&data);
		}
		
		
//		ST7735_SetCursor(0, 1);
//		LCD_OutDec(error_count);
		//--UUU--Complete this  - see lab manual
		
	}
}

/* SysTick ISR
*/
void SysTick_Handler(void){ // every 25 ms
 //Similar to Lab9 except rather than grab sample and put in mailbox
 //        format message and transmit 
	int i=0;
	GPIO_PORTF_DATA_R^=0x02;//toggles pf1
	Data= ADC_In();//sets data equal to the adc input	
	GPIO_PORTF_DATA_R^=0x04;//toggles pf2
	Data=Convert(Data);
	array[7]=0x03;
	array[6]=0x0D;
	array[0]=0x02;
	while (i<3){		
//		if (i==4){
//			array[2]=0x2E;
//			i++;
//		}
		{array[5-i]=Data%10 +0x30;
		i++;
		Data=Data/10;
		}

	}
	i=0;
	array[2]=0x2E;
	array[1]=(Data%10) +0x30;
	while (i<8){
		UART_OutChar(array[i]);
		i++;
	}
	TxCounter++;
	GPIO_PORTF_DATA_R^=0x08;//toggles pf3
}


uint32_t Status[20];             // entries 0,7,12,19 should be false, others true
char GetData[10];  // entries 1 2 3 4 5 6 7 8 should be 1 2 3 4 5 6 7 8
int mainfifo(void){ // Make this main to test FiFo
  FiFo_Init(); // Assuming a buffer of size 6
  for(;;){
    Status[0]  = FiFo_Get(&GetData[0]);  // should fail,    empty
    Status[1]  = FiFo_Put(1);            // should succeed, 1 
    Status[2]  = FiFo_Put(2);            // should succeed, 1 2
    Status[3]  = FiFo_Put(3);            // should succeed, 1 2 3
    Status[4]  = FiFo_Put(4);            // should succeed, 1 2 3 4
    Status[5]  = FiFo_Put(5);            // should succeed, 1 2 3 4 5
    Status[6]  = FiFo_Put(6);            // should succeed, 1 2 3 4 5 6
    Status[7]  = FiFo_Put(7);            // should fail,    1 2 3 4 5 6 
    Status[8]  = FiFo_Get(&GetData[1]);  // should succeed, 2 3 4 5 6
    Status[9]  = FiFo_Get(&GetData[2]);  // should succeed, 3 4 5 6
    Status[10] = FiFo_Put(7);            // should succeed, 3 4 5 6 7
    Status[11] = FiFo_Put(8);            // should succeed, 3 4 5 6 7 8
    Status[12] = FiFo_Put(9);            // should fail,    3 4 5 6 7 8 
    Status[13] = FiFo_Get(&GetData[3]);  // should succeed, 4 5 6 7 8
    Status[14] = FiFo_Get(&GetData[4]);  // should succeed, 5 6 7 8
    Status[15] = FiFo_Get(&GetData[5]);  // should succeed, 6 7 8
    Status[16] = FiFo_Get(&GetData[6]);  // should succeed, 7 8
    Status[17] = FiFo_Get(&GetData[7]);  // should succeed, 8
    Status[18] = FiFo_Get(&GetData[8]);  // should succeed, empty
    Status[19] = FiFo_Get(&GetData[9]);  // should fail,    empty
  }
}

