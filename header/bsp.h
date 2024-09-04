#ifndef _bsp_msp430x2xx_H_
#define _bsp_msp430x2xx_H_

#include  <msp430g2553.h>          // MSP430x2xx

#define  debounceVal      250

// Joystick cursor P1.3 x, P1.4 y
#define Joys_Sel     P1SEL
#define Joys_DIR     P1DIR
#define Joys_IN      P1OUT
//JOISTICK PB P1.0
#define Joys_PB_Sel     P1SEL
#define Joys_PB_Dir     P1DIR
#define Joys_PB_REN     P1REN
#define Joys_PB_IEN      P1IE
#define Joys_PB_INTPEND P1IFG
#define Joys_PB_EDGESEL P1IES
#define Joys_PB_OUT     P1OUT  // control pull-up/down

// MOTOR CONTROL // phase A - phase D P2.0-P2.3
#define Motor_Dir P2DIR
#define Motor_Sel P2SEL
#define Motor_arr P2OUT

// LCD - LCD control: P1.5-P1.7
// LCD - LCD data: P2.4-P2.7
#define LCD_DATA_WRITE     P2OUT
#define LCD_DATA_DIR       P2DIR
#define LCD_DATA_SEL       P2SEL
#define LCD_CTL_SEL        P1SEL

extern void GPIOconfig(void);
extern void ADCconfig(void);
extern void TIMER_0_config(void);
extern void UART_config(void);

#endif
