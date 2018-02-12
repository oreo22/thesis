#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "PWM.h"
#include "math.h"
#include "utilities.h"

//PWM0A is PB4 and top switch signal
//PWM0B2 is PB5 and bottom switch signal //check whether the output GPIO pins are correct

//----------CONVERTER VARIABLES---
//uint8_t batteryMode=0; 	//0=buck/charging   1=boost/discharging
//float duty_trial[]={0,0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9};
double pwm_duty;
uint32_t reload_value=0;  //the value you put into the PWM to make the duty cycle you need to do
uint32_t pwm_frequency=20000; //Frequency of PWM= bus_clock(80MHz)/desired_frequency=80000000/20000
float oldOutput=0; //the former output to boost 
float oldInput=0; //the former input to buck

//----------ADC VARIABLES--------
extern uint8_t ADC_flag;
extern volatile uint32_t ADCdata[8]; //ADC data stored here
uint32_t dcScaleFactor=6; //conversion factor from board measurement to actual voltage (Vout=Vin/6)
uint32_t dcSamplingFreq=800000;//Frequency of ADC= bus_clock(80MHz)/desired_frequency=80000000/100


void buckMode(float buck_Vin){ //batteryMode=0 when Vhigh>Vlow
	//-----Parameters-----
	float chargeVoltage=14.7; 	//Charge battery at 14.7, number in charging range of 14.4-15V
	PWM0A2_Duty(0); //--------------Set the output of PB5 (lower switch) to 0-------
	
	//		if((invOutput-invOutput_old) > 1) { //only change the duty cycle if the incoming voltage is less than 1V difference from previous 

	if(buck_Vin<chargeVoltage){
		PWM0A_Duty(0); //if the voltage has dropped below the battery's accetable charging voltage, turn off the charging 
	}
	else if(buck_Vin >= chargeVoltage){ //abs(oldInput-buck_Vin)>1			//otherwise, Vin > chargeVoltage, start charging battery 
		DelayWait1ms(1); 	//---------Wait 1 ms so the MOSFETS can turn off-------
		//------------CALCULATE DUTY CYCLE TO CHARGE THE BATTERY--------
		pwm_duty=(chargeVoltage/buck_Vin); //D=Vout/Vin 
		reload_value=pwm_duty*pwm_frequency; 
		PWM0A_Duty(reload_value); 
		PF2 |= 0x04; //turn on blue light
		PF3 = 0x00; //turn off green light
		oldInput=buck_Vin; 
	}
}

void boostMode(float boost_Vout,float boost_Vin){ //batteryMode=1 when Vhigh>Vlow
		PWM0A_Duty(500); //--------------Set the output of PWM0A or PB6 to 0------- //PWM requires a vale that's (duty - 1) so make it 1 
	if(boost_Vout==0){ //RP3 says not to discharge
		PWM0A2_Duty(1); //stop discharging
	} //what if Vgrid >> boost_Vout, can we disconnect the dc bus? you have to ensure the voltage difference is too much
	else if(boost_Vout>boost_Vin){//abs(oldOutput-boost_Vout)>1{ //discharge battery
		DelayWait1ms(1);//---------Wait 1 ms so the MOSFETS can turn off-------
	
		//------------CALCULATE DUTY CYCLE TO DISCHARGE THE BATTERY--------
		pwm_duty=1-(boost_Vin/boost_Vout); //buck_Vin); //1-Vin/Vout
		if(pwm_duty>=0.8){ //upper limit on the duty cycle of boost converter
			pwm_duty=0.8;
		}
		reload_value=pwm_duty*pwm_frequency;  //round the reload value up? 
		PWM0A2_Duty(reload_value); 
		PF2  = 0x00; //turn off blue light
		PF3 |= 0x08; //turn on green light
		oldOutput=boost_Vout;
	}
}	
	  /*//----------TESTING CODE----------
		for(char x=0; x<10; x++){
			pwm_duty=duty_trial[x];
			if(pwm_duty>=0.8){ //upper limit on the duty cycle of boost converter
				pwm_duty=0.8;
			}
			reload_value=pwm_duty*pwm_frequency; 
			PWM0A2_Duty(reload_value); 
			DelayWait10ms(3000); //do this for 30 s }
		 		//------RP3 asks to discharge the battery----- 
	
	//---------Scenario we need to calculate the voltage to boost to------
		if(SPI_flag==1){
			Vhigh=SPIdata[0]; //read the voltage you want to discharge from RP3
			//Vhigh=Vhigh=ADCdata[3]*dcScaleFactor; Measure Vhigh (PV voltage) to determine voltage you want to boost up to
			boostMode(Vhigh);
		} */			

