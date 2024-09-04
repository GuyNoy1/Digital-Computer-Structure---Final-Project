//#include <header/bsp_msp430x2xx.h>    // private library - BSP layer
#include "../header/bsp.h" //pesronal kit

//-----------------------------------------------------------------------------  
//           GPIO congiguration
//-----------------------------------------------------------------------------
void GPIOconfig(void){
  
  WDTCTL = WDTHOLD | WDTPW;		                  // Stop WDT
   
  // LCD Setup
    LCD_CTL_SEL &= ~0xE0;                         // LCD control connected to P1.5-P1.7
    LCD_DATA_SEL &= ~0xF0;                        // LCD Data connected to p2.4-p2.7
    LCD_DATA_DIR |= 0xF0;                         // LCD 2.0-p2.3 as output
    LCD_DATA_WRITE &= ~0xF0;                      // Clear output

// Joystick Setup

     Joys_PB_Sel  &= ~BIT0;                    // I/O
     Joys_PB_Dir  &= ~BIT0;                    // Input
     Joys_PB_REN  |= BIT0;                     // Set as Resistor
     Joys_PB_IEN  |= BIT0;                    // Enanble interrupts
     Joys_PB_OUT  |= BIT0;                    // Set as Pull up
     Joys_PB_INTPEND &= ~BIT0;                // clear pending interrupts
     Joys_PB_EDGESEL |=  BIT0;                 // Set interrupt on falling edge

// motor setup

    Motor_Sel &= ~0x0F;    // I/O
    Motor_Dir |= 0x0F;     // Output
    Motor_arr &= ~0x0F;    // Clear output
     
     _BIS_SR(GIE);
}

//-------------------------------------------------------------------------------------
//            ADC congiguration
//-------------------------------------------------------------------------------------
void ADCconfig(void){

               ADC10CTL0 = ADC10SHT_3 + ADC10ON + ADC10IE + MSC;  // 16*ADCLK+ Turn on, and Enable Interrupt
               ADC10CTL1 = INCH_4 + ADC10SSEL_3 + CONSEQ_1;     // Select A4 as the highest channel for sequence of conversions, select SMCLK.
               ADC10AE0 |= BIT3 + BIT4;                         // P1.3 ADC option select
               ADC10DTC1 = 2;                                   // 2 conversions (samples)
     }

//-------------------------------------------------------------------------------------
//            TIMERS congiguration
//-------------------------------------------------------------------------------------
void TIMER_0_config(void){
    DCOCTL = 0;  // Select lowest DCOx and MODx settings
    BCSCTL1 = CALBC1_1MHZ;  // Set DCO to 1 MHz using calibration data
    DCOCTL = CALDCO_1MHZ;  // Set DCO step and modulation
    TA0CCTL0 = CCIE;
    TA0CTL = TASSEL_2 + MC_0 + ID_3 + TACLR; //select: 2 - SMCLK ; control: 0 - STOP  ; divider: 3 - /8
}

//----------------------------------------------------------------------------------
//            UART congiguration
//-------------------------------------------------------------------------------------
void UART_config(void) {
    // Check if calibration constant is erased
    if (CALBC1_1MHZ == 0xFF) {
        while (1);  // Trap CPU in infinite loop if calibration constant is erased
    }

    // Configure DCO to 1 MHz
    DCOCTL = 0;  // Select lowest DCOx and MODx settings
    BCSCTL1 = CALBC1_1MHZ;  // Set DCO to 1 MHz using calibration data
    DCOCTL = CALDCO_1MHZ;  // Set DCO step and modulation

    // Select UART function for P1.1 (RX) and P1.2 (TX)
    P1SEL |= BIT1 + BIT2;  // Set P1.1 and P1.2 to function as UART
    P1SEL2 |= BIT1 + BIT2;  // Secondary function select for P1.1 and P1.2

    // Configure USCI_A0 for UART mode
    UCA0CTL1 |= UCSSEL_2;  // Select SMCLK as the clock source
    UCA0BR0 = 104;  // Set baud rate to 9600 (1 MHz / 9600 = 104)
    UCA0BR1 = 0x00;  // Upper byte of baud rate setting
    UCA0MCTL = UCBRS0;  // Set modulation
    UCA0CTL1 &= ~UCSWRST;  // Initialize USCI state machine
}
