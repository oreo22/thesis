#include <stdint.h>

#define PF1       (*((volatile uint32_t *)0x40025008))
#define PF2       (*((volatile uint32_t *)0x40025010))
#define PF3       (*((volatile uint32_t *)0x40025020))	
#define PF4				(*((volatile uint32_t *)0x40025040))	

//PortA	0x40004000  
//PortB	0x40005000
//PortC	0x40006000
//PortD	0x40007000
//PortE	0x40024000
//PortF	0x40025000

/*
7 0x0200
6 0x0100
5 0x0080
4 0x0040
3 0x0020
2 0x0010
1 0x0008
0 0x0004
*/


void GPIO_Init(void);
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode

void DelayWait10ms(uint32_t n);
void DelayWait1ms(uint32_t n);
void heartbeat(void);
void EdgeInterrupt_Init(void);
