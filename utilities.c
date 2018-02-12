#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "utilities.h"

void GPIO_Init(void){  
  SYSCTL_RCGCGPIO_R |= 0x08;        // 1) activate port D
  while((SYSCTL_PRGPIO_R&0x08)==0){};   // allow time for clock to stabilize
                                    // 2) no need to unlock PD3-0
  GPIO_PORTD_AMSEL_R &= ~0x0F;      // 3) disable analog functionality on PD3-0
  GPIO_PORTD_PCTL_R &= ~0x0000FFFF; // 4) GPIO
  GPIO_PORTD_DIR_R |= 0x0F;         // 5) make PD3-0 out
  GPIO_PORTD_AFSEL_R &= ~0x0F;      // 6) regular port function 
  GPIO_PORTD_DEN_R |= 0x0F;         // 7) enable digital I/O on PD3-0
	SYSCTL_RCGCGPIO_R |= 0x00000020;         // activate port F
  GPIO_PORTF_DIR_R |= 0x0C;                // make PF2 out (built-in LED)
  GPIO_PORTF_AFSEL_R &= ~0x0C;             // disable alt funct on PF2
  GPIO_PORTF_DEN_R |= 0x0C;                // enable digital I/O on PF2
                                           // configure PF2 as GPIO
  GPIO_PORTF_PCTL_R = (GPIO_PORTF_PCTL_R&0xFFFF00FF)+0x00000000;
  GPIO_PORTF_AMSEL_R = 0;                  // disable analog functionality on PF
}

void DelayWait10ms(uint32_t n){
	uint32_t volatile time;
  while(n){
    time = 727240*2/91;  // 10msec
    while(time){
      time--;
    }
    n--;
  }
}

void DelayWait1ms(uint32_t n){
	uint32_t volatile time;
  while(n){
    time = (727240*2/91)/10;  // 1 msec //ACTION ITEM: ASK VALVANO ABOUT THIS EQUATION
    while(time){
      time--;
    }
    n--;
  }
}

void heartbeat(void){
  PF2 |= 0x04;              // turn off LED
	PF3 |= 0x08;
}

volatile uint8_t run_flag;

void EdgeInterrupt_Init(void){                          
  SYSCTL_RCGCGPIO_R |= 0x00000020; // (a) activate clock for port 
  GPIO_PORTF_DIR_R &= ~0x10;    // (c) make PF4 in (built-in button)
  GPIO_PORTF_AFSEL_R &= ~0x10;  //     disable alt funct on PF4
  GPIO_PORTF_DEN_R |= 0x10;     //     enable digital I/O on PF4   
  GPIO_PORTF_PCTL_R &= ~0x000F0000; // configure PF4 as GPIO
  GPIO_PORTF_AMSEL_R = 0;       //     disable analog functionality on PF
  GPIO_PORTF_PUR_R |= 0x10;     //     enable weak pull-up on PF4
  GPIO_PORTF_IS_R &= ~0x10;     // (d) PF4 is edge-sensitive
  GPIO_PORTF_IBE_R |= 0x10;    //     PF4 is both edges

  GPIO_PORTF_ICR_R = 0x10;      // (e) clear flag4
  GPIO_PORTF_IM_R |= 0x10;      // (f) arm interrupt on PF4 *** No IME bit as mentioned in Book ***
  NVIC_PRI7_R = (NVIC_PRI7_R&0xFF00FFFF)|0x00A00000; // (g) priority 5
  NVIC_EN0_R = 0x40000000;      // (h) enable interrupt 30 in NVIC
 // EnableInterrupts();           // (i) Clears the I bit
	run_flag = 0;
}
void GPIOPortF_Handler(void){
  GPIO_PORTF_ICR_R = 0x10;      // acknowledge flag4
 //read Sharms signal
	if (PF4 == 0x10){
		run_flag = 1;
	} else{
	  run_flag = 0;
	}
}
