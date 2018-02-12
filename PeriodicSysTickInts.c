// PeriodicSysTickInts.c
// Runs on LM4F120
// Use the SysTick timer to request interrupts at a particular period.
// Daniel Valvano
// October 11, 2012

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2014

   Program 5.12, section 5.7

 Copyright 2014 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */

// oscilloscope or LED connected to PF2 for period measurement
#include <stdint.h>
#include "tm4c123gh6pm.h"

#include "SysTickInts.h"
#include "PLL.h"


#define PF2     (*((volatile uint32_t *)0x40025010))


double sawtooth[18] = {1.65, 1.98, 2.31, 2.64, 2.97, 2.64, 2.31, 1.98, 1.65, 1.32,0.99,0.66,0.33,0,0.33,0.66,0.99,1.32};
double VCtrlIn[18]={1,0.7,0.8,0.9,0.1,0.7,1.1,1.5,1.8,12,13,14,15,17,18,21,2.3,2.4}; //make this a sine wave
int saw_index=0;

// Interrupt service routine
// Executed every 12.5ns*(period)
void SysTick_Handler(void){
  PF2 ^= 0x04;                // toggle PF2
	saw_index= (saw_index %18 )+1;
	
		if(sawtooth[saw_index-1] > VCtrlIn[saw_index-1]){ //how do I test the two? Is this too many functions to do in a handler?
			GPIO_PORTD_DATA_R =0x00; //PD0 //GPIO_PORTD_DATA_R |=0x04; //PD2 on, PD3 off
		}
		else {
			GPIO_PORTD_DATA_R =0x01; 
			//GPIO_PORTD_DATA_R |=0x08;  //PD2 off, PD3 on
		}
}
