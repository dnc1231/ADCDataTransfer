// UART1.c
// Runs on LM4F120/TM4C123
// Use UART1 to implement bidirectional data transfer to and from 
// another microcontroller in Lab 9.  This time, interrupts and FIFOs
// are used.
// Daniel Valvano
// November 17, 2014
// Modified by EE345L students Charlie Gough && Matt Hawk
// Modified by EE345M students Agustinus Darmawan && Mingjie Qiu

/* Lab solution, Do not post
 http://users.ece.utexas.edu/~valvano/
*/

// U1Rx (VCP receive) connected to PC4
// U1Tx (VCP transmit) connected to PC5
#include <stdint.h>
#include "FiFo.h"
#include "UART.h"
#include "tm4c123gh6pm.h"

int RxCounter=0;
uint32_t DataLost; 
volatile int delay;
// Initialize UART1
// Baud rate is 115200 bits/sec
// Make sure to turn ON UART1 Receiver Interrupt (Interrupt 6 in NVIC)
// Write UART1_Handler
void UART_Init(void){
   // --UUU-- complete with your code
	SYSCTL_RCGCUART_R |= 0x0002;//uart1
	delay=100;
	delay=100;
	SYSCTL_RCGCGPIO_R |= 0x0004;//port c
	delay=100;
	delay=100;
	
	UART1_CTL_R &= ~0x0001;
	UART1_IBRD_R = 43;
	UART1_FBRD_R = 26;
	UART1_LCRH_R = 0x0070;
	UART1_CTL_R = 0x0301;	
	
	UART1_IM_R |= 0x10;
	UART1_IFLS_R |= 0x10;
	UART1_IFLS_R &= ~0x28;
	NVIC_PRI1_R |= 0xE00000;
	NVIC_EN0_R |=0x40;
	
	GPIO_PORTC_PCTL_R = (GPIO_PORTC_PCTL_R&0xFF00FFFF) +0x00220000;
	GPIO_PORTC_AMSEL_R &= ~0x30;
	GPIO_PORTC_AFSEL_R |= 0x30;
	GPIO_PORTC_DEN_R |= 0x30;

}

// input ASCII character from UART
// spin if RxFifo is empty
char UART_InChar(void){
	while ((UART1_FR_R&0x0010) != 0){};
  return ((char)(UART1_DR_R&UART1_DR_R&0xFF)); // --UUU-- remove this, replace with real code
}
//------------UART1_OutChar------------
// Output 8-bit to serial port
// Input: letter is an 8-bit ASCII character to be transferred
// Output: none
void UART_OutChar(char data){
  // --UUU-- complete with your code
	while((UART1_FR_R&0x0020) !=0){};
	UART1_DR_R = data;
}

// hardware RX FIFO goes from 7 to 8 or more items
// UART receiver Interrupt is triggered; This is the ISR
void UART1_Handler(void){
  // --UUU-- complete with your code
	char temp;
	GPIO_PORTF_DATA_R^=0x02;//toggles pf1
	GPIO_PORTF_DATA_R^=0x04;//toggles pf2
	while ((UART1_FR_R&0x10)==0){
		temp=(UART1_DR_R&0xFF);
		FiFo_Put(temp);
	}
	RxCounter++;
	UART1_ICR_R=0x10;
	GPIO_PORTF_DATA_R^=0x08;
	
	
}
