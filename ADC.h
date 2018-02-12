#include "stdint.h"
extern uint8_t ADC_index;
extern uint8_t ADCflag;

void ADC_Init_AC(uint32_t period);
void ADC_Init_DC(uint32_t period);
void ADC0Seq0_Handler(void);
void ADC1Seq0_Handler(void);
void Timer1A_Handler(void);
