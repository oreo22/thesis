#include <stdint.h>
#include "PLL.h"
#include "PWM.h"
#include "tm4c123gh6pm.h"
#include "utilities.h"
#include "ADC.h"
#include "SysTickInts.h"
#include "Measurements.h"
#include "VoltageConverter.h"

//#include "SPI.h"
	
//extern uint8_t ADCflag;
//extern uint32_t ADCvalue[100];
	uint8_t condition=0;  //Master Controller Command 
	uint8_t shutDown=0; 

	/*DC ADC: PE5, PE4, PE1, PE0 
	AC ADC: PE3, PE2
	MOSFET Switches (PWM): PB5 for top switch, PB6 for bottom switch 
	INVERTER CTRLS (Output GPIO): PD2 (+Vcont), PD3 (-Vcont) for inverter
	LIGHTS: PF2 AND PF3 
	*/

void Init(void){
	DisableInterrupts();
  PLL_Init();                 // bus clock at 80 MHz
  SYSCTL_RCGCGPIO_R |= 0x00000020;         // activate port F
	GPIO_Init();
	PWM0B2_Init(2000, 500); //25% duty cycle, change second number to % of first
	PWM0A_Init(2000, 500); //1st number is frequency and 2nd # is duty cycle
	ADC_Init_DC(800); //800   1 kHz sampling, PE5, PE4, PE1, PE0  bus_freq/desired_freq
	//ADC_Init_AC(13333); // 13333   6 kHz sampling, PE2 and PE3; bus_freq/desired_freq
	EnableInterrupts();
	condition =2;
}

float invOutput=0;
float dcVin=0;
void chargeBat(void){
	DisableInterrupts();
	NVIC_ST_CTRL_R = 0;         // disable SysTick 
 	NVIC_ST_RELOAD_R = 0;// disarm the systick by setting reload value to 0 //check if systick is still interrupting...
	GPIO_PORTD_DATA_R |=0x00;  //PD2 off, PD3 off
	EnableInterrupts();
	while(condition==1){
		invOutput=calculateV();
		buckMode(invOutput); //tell the converter about the new voltage
	}
}

void dischargeBat(void){
	DisableInterrupts(); 
	SysTick_Init(1067);        // initialize SysTick timer, sawtooth has 25 pts, so to get a 30kHz signal, you have to interrupt 30kHz *25	
	EnableInterrupts();
	while(condition==2){
	//	invOutput=calculateV(); //voltage level it should boost to
		dcVin=12; //the input to the boost 
	//	boostMode(invOutput,dcVin);
	}
}
	
void neutral(void){ //turn off everything except SPI to get readings 
	DisableInterrupts();
	NVIC_ST_CTRL_R = 0;         // disable SysTick 
 	NVIC_ST_RELOAD_R = 0;// disarm the systick by setting reload value to 0 //check if systick is still interrupting...
	GPIO_PORTD_DATA_R =0x00;  //PD2 off, PD3 off
	GPIO_PORTB_DATA_R =0x00;
	EnableInterrupts();
}

volatile uint32_t Counts = 0;
int main(void){
	Init();
//	heartbeat();
  while(shutDown != 1){              
		if (condition==1){
			chargeBat();
		}
		if (condition==2){
			dischargeBat();
		}
		if (condition==0){
			neutral();
		}
		else{
			WaitForInterrupt();
		}
  }
}

// Interrupt service routine
// Executed every 12.5ns*(period)
