// SysTickInts.c
// Runs on LM4F120/TM4C123
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

#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "SysTickInts.h"
#include "utilities.h"

#define NVIC_ST_CTRL_CLK_SRC    0x00000004  // Clock Source
#define NVIC_ST_CTRL_INTEN      0x00000002  // Interrupt enable
#define NVIC_ST_CTRL_ENABLE     0x00000001  // Counter mode
#define PD3 (*((volatile uint32_t *)0x40007020 ))
#define PD2 (*((volatile uint32_t *)0x40007010 ))

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode

extern uint32_t inputValue;
extern uint32_t inputNeg;

// **************SysTick_Init*********************
// Initialize SysTick periodic interrupts
// Input: interrupt period
//        Units of period are 12.5ns (assuming 50 MHz clock)
//        Maximum is 2^24-1
//        Minimum is determined by length of ISR
// Output: none
void SysTick_Init(uint32_t period){long sr; //sawtooth wave should be at 200kHz or 133 kHz
  sr = StartCritical();
  NVIC_ST_CTRL_R = 0;         // disable SysTick during setup
  NVIC_ST_RELOAD_R = period-1;// reload value
  NVIC_ST_CURRENT_R = 0;      // any write to current clears it
  NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R&0x00FFFFFF)|0x40000000; // priority 2
                              // enable SysTick with core clock and interrupts
  NVIC_ST_CTRL_R = NVIC_ST_CTRL_ENABLE+NVIC_ST_CTRL_CLK_SRC+NVIC_ST_CTRL_INTEN;
  EndCritical(sr);
}

float sFactor=0.715; //ma factor, since sawtooth is set to 3.3, it's unknown if the max of the incoming modulated sine wave is at 3.3. 
//If the peak isn't, change the scaling factor of the sawtooth to the ratio of the maxIncomingSignal/3.3, so it's a percentage of the full. Modulate the whole sawtooth to 
//(if incoming signal is greater than 3.3, ignore value), do this calculation elsewhere 

float sawtooth[25] = { 1.65, 1.86, 2.06,2.27,2.48,2.68,2.89,3.09, 3.30,3.09,2.89,2.68, 2.48, 2.27,2.06, 1.86 ,1.65 ,1.44, 1.24 ,1.03 ,0.83 ,0.62, 0.41, 0.21, 0   };
//float VCtrlIn[18]={1,0.7,0.8,0.9,0.1,0.7,1.1,1.5,1.8,12,13,14,15,17,18,21,2.3,2.4}; 
int saw_index=0;
// PD2 is the A branch, PD3 is the B branch 
//sFactor=1;
void SysTick_Handler(void){
	double inputNeg=(inputValue*-1)+3.3;
	saw_index= (saw_index %25 )+1;
		/*if(maxVdone==1){
			sFactor=maxV/3.3;
		}*/
	float temp=sawtooth[saw_index-1]*sFactor; //*sFactor;
		if(inputValue >= temp) {
		//	GPIO_PORTD_DATA_R =0x030; //PD2 on, PD3 on
			PD2 = 0x04; 
		}
	if(inputValue < temp  ){ 
			PD2 = 0x00; 
		}
		if (inputNeg > temp){
//			GPIO_PORTD_DATA_R =0x010;  //PD2 off, PD3 on
			PD3 = 0x08; 
		}
		if(inputNeg < temp){ //inputValue> (sawtooth[saw_index-1]*sFactor) && 
			//GPIO_PORTD_DATA_R =0x020;  //PD2 on, PD3 off 
			PD3 = 0x00; 
		}
}
