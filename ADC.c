#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "utilities.h"

#define NVIC_EN0_INT17          0x00020000  // Interrupt 17 enable

#define TIMER_CFG_16_BIT        0x00000004  // 16-bit timer configuration,
                                            // function is controlled by bits
                                            // 1:0 of GPTMTAMR and GPTMTBMR
#define TIMER_TAMR_TACDIR       0x00000010  // GPTM Timer A Count Direction
#define TIMER_TAMR_TAMR_PERIOD  0x00000002  // Periodic Timer mode
#define TIMER_CTL_TAOTE         0x00000020  // GPTM TimerA Output Trigger
                                            // Enable
#define TIMER_CTL_TAEN          0x00000001  // GPTM TimerA Enable
#define TIMER_IMR_TATOIM        0x00000001  // GPTM TimerA Time-Out Interrupt
                                            // Mask
#define TIMER_TAILR_TAILRL_M    0x0000FFFF  // GPTM TimerA Interval Load
                                            // Register Low

#define ADC_ACTSS_ASEN3         0x00000008  // ADC SS3 Enable
#define ADC_RIS_INR3            0x00000008  // SS3 Raw Interrupt Status
#define ADC_IM_MASK3            0x00000008  // SS3 Interrupt Mask
#define ADC_ISC_IN3             0x00000008  // SS3 Interrupt Status and Clear
#define ADC_EMUX_EM3_M          0x0000F000  // SS3 Trigger Select mask
#define ADC_EMUX_EM3_TIMER      0x00005000  // Timer
#define ADC_SSPRI_SS3_4TH       0x00003000  // fourth priority
#define ADC_SSPRI_SS2_3RD       0x00000200  // third priority
#define ADC_SSPRI_SS1_2ND       0x00000010  // second priority
#define ADC_SSPRI_SS0_1ST       0x00000000  // first priority
#define ADC_PSSI_SS3            0x00000008  // SS3 Initiate
#define ADC_SSCTL3_TS0          0x00000008  // 1st Sample Temp Sensor Select
#define ADC_SSCTL3_IE0          0x00000004  // 1st Sample Interrupt Enable
#define ADC_SSCTL3_END0         0x00000002  // 1st Sample is End of Sequence
#define ADC_SSCTL3_D0           0x00000001  // 1st Sample Diff Input Select
#define ADC_SSFIFO3_DATA_M      0x00000FFF  // Conversion Result Data mask
#define ADC_PC_SR_M             0x0000000F  // ADC Sample Rate
#define ADC_PC_SR_125K          0x00000001  // 125 ksps
#define SYSCTL_RCGCGPIO_R4      0x00000010  // GPIO Port E Run Mode Clock
                                            // Gating Control
#define SYSCTL_RCGCGPIO_R3      0x00000008  // GPIO Port D Run Mode Clock
                                            // Gating Control
#define SYSCTL_RCGCGPIO_R1      0x00000002  // GPIO Port B Run Mode Clock
                                            // Gating Control


// There are many choices to make when using the ADC, and many
// different combinations of settings will all do basically the
// same thing.  For simplicity, this function makes some choices
// for you.  When calling this function, be sure that it does
// not conflict with any other software that may be running on
// the microcontroller.  Particularly, ADC0 sample sequencer 3
// is used here because it only takes one sample, and only one
// sample is absolutely needed.  Sample sequencer 3 generates a
// raw interrupt when the conversion is complete, and it is then
// promoted to an ADC0 controller interrupt.  Hardware Timer0A
// triggers the ADC0 conversion at the programmed interval, and
// software handles the interrupt to process the measurement
// when it is complete.
//
// A simpler approach would be to use software to trigger the
// ADC0 conversion, wait for it to complete, and then process the
// measurement.
//
// This initialization function sets up the ADC according to the
// following parameters.  Any parameters not explicitly listed
// below are not modified:
// Timer0A: enabled
// Mode: 32-bit, down counting
// One-shot or periodic: periodic
// Interval value: programmable using 32-bit period
// Sample time is busPeriod*period
// Max sample rate: <=125,000 samples/second
// Sequencer 0 priority: 1st (highest)
// Sequencer 1 priority: 2nd
// Sequencer 2 priority: 3rd
// Sequencer 3 priority: 4th (lowest)
// SS3 triggering event: Timer0A
// SS3 1st sample source: programmable using variable 'channelNum' [0:11]
// SS3 interrupts: enabled and promoted to controller

uint8_t ADC_index;
uint8_t ADCflag;
void ADC_Init_DC(uint32_t period){ //sequencer 0
  volatile uint32_t delay;
  // **** GPIO pin initialization
	ADC_index = 0;
	ADCflag = 0;
	SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R4; //    these are on GPIO_PORTE
  
  delay = SYSCTL_RCGCGPIO_R;      // 2) allow time for clock to stabilize
  delay = SYSCTL_RCGCGPIO_R;
	 //      Ain9 is on PE4
      GPIO_PORTE_DIR_R &= ~0x10;  // 3.9) make PE4 input
      GPIO_PORTE_AFSEL_R |= 0x10; // 4.9) enable alternate function on PE4
      GPIO_PORTE_DEN_R &= ~0x10;  // 5.9) disable digital I/O on PE4
      GPIO_PORTE_AMSEL_R |= 0x10; // 6.9) enable analog functionality on PE4
		//      Ain8 is on PE5
      GPIO_PORTE_DIR_R &= ~0x20;  // 3.8) make PE5 input
      GPIO_PORTE_AFSEL_R |= 0x20; // 4.8) enable alternate function on PE5
      GPIO_PORTE_DEN_R &= ~0x20;  // 5.8) disable digital I/O on PE5
      GPIO_PORTE_AMSEL_R |= 0x20; // 6.8) enable analog functionality on PE5
    //      Ain0 is on PE3
      GPIO_PORTE_DIR_R &= ~0x08;  // 3.0) make PE3 input
      GPIO_PORTE_AFSEL_R |= 0x08; // 4.0) enable alternate function on PE3
      GPIO_PORTE_DEN_R &= ~0x08;  // 5.0) disable digital I/O on PE3
      GPIO_PORTE_AMSEL_R |= 0x08; // 6.0) enable analog functionality on PE3
    //      Ain1 is on PE2
      GPIO_PORTE_DIR_R &= ~0x04;  // 3.1) make PE2 input
      GPIO_PORTE_AFSEL_R |= 0x04; // 4.1) enable alternate function on PE2
      GPIO_PORTE_DEN_R &= ~0x04;  // 5.1) disable digital I/O on PE2
      GPIO_PORTE_AMSEL_R |= 0x04; // 6.1) enable analog functionality on PE2

  DisableInterrupts();
  SYSCTL_RCGCADC_R |= SYSCTL_RCGCTIMER_R0;     // activate ADC0 
  SYSCTL_RCGCTIMER_R |= SYSCTL_RCGCTIMER_R0;   // activate timer0 
  delay = SYSCTL_RCGCTIMER_R;   // allow time to finish activating
  TIMER0_CTL_R = 0x00000000;    // disable timer0A during setup
  TIMER0_CTL_R |= 0x00000020;   // enable timer0A trigger to ADC
  TIMER0_CFG_R = 0;             // configure for 32-bit timer mode
  TIMER0_TAMR_R = 0x00000002;   // configure for periodic mode, default down-count settings
  TIMER0_TAPR_R = 0;            // prescale value for trigger
  TIMER0_TAILR_R = period-1;    // start value for trigger
  TIMER0_IMR_R = 0x00000000;    // disable all interrupts
  TIMER0_CTL_R |= 0x00000001;   // enable timer0A 32-b, periodic, no interrupts
	
  ADC0_PC_R = 0x01;         // configure for 125K samples/sec
  ADC0_SSPRI_R =0x1230;    // seq 0 highest with 3 being second highest;   0x3210 sequencer 0 is highest, sequencer 3 is lowest, //0x0123 has sequencer 3 is the highest with sequencer 0 being the lowest
	ADC0_ACTSS_R &= ~0x01;  //disable seq 0 //~0x09;     //disable sample sequencer 0 and 3
  ADC0_EMUX_R = (ADC0_EMUX_R&0xFFFFFFF0)+0x0005; // timer trigger event //ADC0_EMUX_R = (ADC0_EMUX_R&0xFFFF0FF0)+0x5005; // timer trigger event for sequencer 0 and 3 
	ADC0_SAC_R = ADC_SAC_AVG_4X; // 4x hardware oversampling // ADC_SAC_AVG_2X; // 2x hardware oversampling  //  ADC0_SAC_R = ADC_SAC_AVG_2X;
	ADC0_SSMUX0_R = 0x0089;               //sample channels 2,3,and  8 and 9
  ADC0_SSCTL0_R = 0x07;  // no TS0, yes IE0 END0, D0

	ADC0_IM_R |=  0x01;  //enable SS0 and SS3 interrupts
	ADC0_ACTSS_R |=0x01;  //enable sample sequencer 0 and 3
	NVIC_PRI3_R = (NVIC_PRI3_R&0xFF00FFFF)|0x00400000; //priority 2 b/c priority 1 is 2, plaed in right bits
	NVIC_EN0_R = 1<<14;
}

void ADC_Init_AC(uint32_t period){ //sequencer 3
	volatile uint32_t delay;
  // **** GPIO pin initialization 
	
	SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R4; //    these are on GPIO_PORTE
  delay = SYSCTL_RCGCGPIO_R;      // 2) allow time for clock to stabilize
  delay = SYSCTL_RCGCGPIO_R;
    //      Ain2 is on PE1
      GPIO_PORTE_DIR_R &= ~0x02;  // 3.2) make PE1 input
      GPIO_PORTE_AFSEL_R |= 0x02; // 4.2) enable alternate function on PE1
      GPIO_PORTE_DEN_R &= ~0x02;  // 5.2) disable digital I/O on PE1
      GPIO_PORTE_AMSEL_R |= 0x02; // 6.2) enable analog functionality on PE1
    //      Ain3 is on PE0
      GPIO_PORTE_DIR_R &= ~0x01;  // 3.3) make PE0 input
      GPIO_PORTE_AFSEL_R |= 0x01; // 4.3) enable alternate function on PE0
      GPIO_PORTE_DEN_R &= ~0x01;  // 5.3) disable digital I/O on PE0
      GPIO_PORTE_AMSEL_R |= 0x01; // 6.3) enable analog functionality on PE0
    //      Ain9 is on PE4
      GPIO_PORTE_DIR_R &= ~0x10;  // 3.9) make PE4 input
      GPIO_PORTE_AFSEL_R |= 0x10; // 4.9) enable alternate function on PE4
      GPIO_PORTE_DEN_R &= ~0x10;  // 5.9) disable digital I/O on PE4
      GPIO_PORTE_AMSEL_R |= 0x10; // 6.9) enable analog functionality on PE4
		//      Ain8 is on PE5
      GPIO_PORTE_DIR_R &= ~0x20;  // 3.8) make PE5 input
      GPIO_PORTE_AFSEL_R |= 0x20; // 4.8) enable alternate function on PE5
      GPIO_PORTE_DEN_R &= ~0x20;  // 5.8) disable digital I/O on PE5
      GPIO_PORTE_AMSEL_R |= 0x20; // 6.8) enable analog functionality on PE5

  DisableInterrupts();
	SYSCTL_RCGCADC_R |= SYSCTL_RCGCTIMER_R1;     // activate ADC1 
	SYSCTL_RCGCTIMER_R |= SYSCTL_RCGCTIMER_R1;   // activate timer1 
	TIMER1_CTL_R = 0x00000000;    // 1) disable TIMER1A during setup
	TIMER1_CTL_R |= 0x00000020;   // enable timer1A trigger to ADC
  TIMER1_CFG_R =0;    // 2) configure for 32-bit mode
  TIMER1_TAMR_R = 0x00000002;   // 3) configure for periodic mode, down-count 
  TIMER1_TAILR_R = period-1;  // 4) reload value
  TIMER1_TAPR_R = 0;            // 5) bus clock resolution
 // TIMER1_ICR_R = 0x00000001;    // 6) clear TIMER1A timeout flag
  TIMER1_IMR_R = 0x00000000;    // 7) disarm timeout interrupt
	TIMER1_CTL_R = 0x00000001;    // 10) enable TIMER1A
	


	ADC1_PC_R = 0x01;         // configure for 125K samples/sec
  ADC1_SSPRI_R =0x1230;    // seq 0 highest with 3 being second highest;   0x3210 sequencer 0 is highest, sequencer 3 is lowest, //0x0123 has sequencer 3 is the highest with sequencer 0 being the lowest
	ADC1_ACTSS_R &= ~0x01;  //disable seq 0 //~0x09;     //disable sample sequencer 0 and 3
	ADC1_EMUX_R = (ADC1_EMUX_R&0xFFFFFFF0)+0x0005; // timer trigger event for sequencer 0 and 3 
	ADC1_SAC_R = ADC_SAC_AVG_4X; // 4x hardware oversampling // ADC_SAC_AVG_2X; // 2x hardware oversampling  //  ADC0_SAC_R = ADC_SAC_AVG_2X;
	ADC1_SSMUX0_R = 0x00239;        //sample channels 2,3,and  8 and 9
  ADC1_SSCTL0_R = 0x07;  // no TS0 D0, yes IE0 

	ADC1_IM_R |=  0x01;  //enable SS0 and SS3 interrupts
	ADC1_ACTSS_R |=0x01;  //enable sample sequencer 0 
	NVIC_PRI3_R = (NVIC_PRI3_R&0x0FFFFFFF)|0x80000000; //priority 1
		//NVIC_PRI3_R = (NVIC_PRI3_R&0xFF00FFFF)|0x00400000;
	NVIC_EN0_R = 1<<15;              // enable interrupt 15 in NVIC b/c IRQ is 15
}

volatile double ADCvalue[100];
uint32_t ADCindex=0;
double inputValue;
//float dcScaleFactor=0.5;
//float adcInput;

void ADC0Seq0_Handler(void){ //used to sample a bunch of stuff
	volatile uint32_t V0; //PE3
	volatile uint32_t V1; //PE2
	double inputCheck1; //PE2
	double inputCheck2;
  ADC0_ISC_R = 0x01;          // acknowledge ADC sequence 0 completion
  V0 = ADC0_SSFIFO0_R&0x0FFF;  // 3A) read third result
	V1 = ADC0_SSFIFO0_R&0x0FFF;  // 12-bit result //ADC0_SSFIFO3_R&0xFFF //channel 8, PE5
//	data = (ADC0_SSFIFO0_R&0x0FFF);  // 3A) read first result   //channel 9, PE4
	inputCheck1=(V0*3.3); //-1.65 //to get rid of the level shift
	inputCheck2=(V1*3.3);
	
	inputCheck1=(inputCheck1)/4096;
	inputCheck2=inputCheck2/4096;
	inputCheck1=inputCheck1-inputCheck2;
	if(inputCheck1<4){
		inputValue=inputCheck1;
		ADCvalue[ADCindex]=inputValue;
		ADCindex= (ADCindex %100)+1;
	}
	//	PF3 |= 0x08; //green light
	//PF3 |= 0x08; //green light
  //GPIO_PORTD_DATA_R ^=0x04; // PD2 PWM at 3kHz
	
}

void ADC1Seq0_Handler(void){	
	volatile uint32_t data;
  ADC1_ISC_R = 0x01;          // acknowledge ADC sequence 3 completion
	PF3 |= 0x08; //green light
  //GPIO_PORTD_DATA_R ^=0x04; // PD2 PWM at 3kHz

}

void Timer1A_Handler(void){
	 TIMER1_ICR_R = TIMER_ICR_TATOCINT;// acknowledge TIMER1A timeout
	PF3 |= 0x08; //green light
}



