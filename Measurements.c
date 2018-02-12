#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "math.h"

extern double ADCvalue[100];
uint32_t maxV=0;
uint8_t maxVdone=0;

float calculateV(void){
	double ADCcopy[100];
	for (int i = 0; i<100; i++){ //create a local copy of the 
		ADCcopy[i] = ADCvalue[i];
		if (i>1 && (ADCcopy[i] > ADCvalue[i-1])  ){
			maxV=ADCcopy[i];
		}
	}
	maxVdone=1;
	float scaledV[100];
	//float SCBscale = 2.66666;  //4V input
	float SCBscale = 3.73333;  //5.6V input
	//float xfmrScale=5; //120:24 step-down xfmr
	
/*	for (int i = 0; i<100; i++){
		scaledV[i] = (ADCcopy[i] *3.3)/4096; //To get the non-level shifted value
		//scaledV[i] = scaledV[i] - 1.65; 	 //Account for the level-shift by subtracting the offset 
	}*/
	
	float sum = 0; //uint32_t?
	float ui2 = 0;
	for (int i = 0; i<100; i++){
		ui2=scaledV[i] * scaledV[i];
		sum = sum + ui2;
	}
	sum = sum /100;
	sum = sqrt(sum);
	sum = sum * SCBscale; //Scale up the voltage value by the SCB's reduction factor 
	//sum	= sum* xfmrScale; //Scale up the voltage value by the step-down factor of the xfmr
	return sum;
}

float calculateI(void){ //DETERMINE THE RIGHT ARRAY AND FACTORS BEFORE USING THIS 
	uint32_t ADCcopy[100];
	for (int i = 0; i<100; i++){ //create a local copy of the 
		ADCcopy[i] = ADCvalue[i];
	}
	float scaledV[100];
	//float SCBscale = 2.66666;  //4V input
	float SCBscale = 3.73333;  //5.6V input
	float xfmrScale=5; //120:24 step-down xfmr
	
	for (int i = 0; i<100; i++){
		scaledV[i] = (ADCcopy[i] *3.3)/4096; //To get the non-level shifted value
		//scaledV[i] = scaledV[i] - 1.65; 	 //Account for the level-shift by subtracting the offset 
	}
	
	float sum = 0; //uint32_t?
	float ui2 = 0;
	for (int i = 0; i<100; i++){
		ui2=scaledV[i] * scaledV[i];
		sum = sum + ui2;
	}
	sum = sum /100;
	sum = sqrt(sum);
	sum = sum * SCBscale; //Scale up the voltage value by the SCB's reduction factor 
	sum	= sum* xfmrScale; //Scale up the voltage value by the step-down factor of the xfmr
	return sum;
}

float calculateP(void){
	uint32_t Vcopy[100];
	uint32_t Icopy[100];
	//float SCBscale = 2.66666;  //4V input
	float SCBscaleV = 3.73333;  //5.6V input
	float xfmrScale=5; //120:24 step-down xfmr
	float SCBscaleI=1;
	for (int i = 0; i<100; i++){ //create a local copy of the 
		Vcopy[i] = ADCvalue[i];
		Vcopy[i] = (Vcopy[i] *3.3)/4096;
	}
	for (int i = 0; i<100; i++){ //create a local copy of the 
		Icopy[i] = ADCvalue[i]; //PLACE HOLDER
		Icopy[i] = (Icopy[i] *3.3)/4096;
	}
	float scaledV[100];
	float scaledI[100];
	float sum = 0; //uint32_t?
	float ui2 = 0;
	for (int i = 0; i<100; i++){
		ui2=scaledV[i] * scaledI[i];
		sum = sum + ui2;
	}
	sum = sum /100;
	sum = sqrt(sum);
	sum = sum * SCBscaleV; //Scale up the voltage value by the SCB's reduction factor 
	sum = sum * SCBscaleI;
	sum	= sum * xfmrScale; //Scale up the voltage value by the step-down factor of the xfmr
	return sum;
}
