#include <xc.h>

void CCP1_Initialize() {
    TRISCbits.TRISC2=0;	// RC2 pin is output.
    //CCP1CON=9;		// Compare mode, initialize CCP1 pin high, clear output on compare match
    CCP1CON=12;         // PWM mode, P1A, P1C active-high; P1B, P1D active-high
    //PIR1bits.CCP1IF=0;    // maybe for capture mode
    //IPR1bits.CCP1IP = 1;  // maybe for capture mode

    
    PIE1bits.TMR2IE = 0; // we don't need interrupt for pwm
    IPR1bits.TMR2IP = 0; // we don't need interrupt for pwm
    return ;
}

void CCP2_Initialize() {
    TRISCbits.TRISC1=0;	// RC1 pin is output.
    //CCP1CON=9;		// Compare mode, initialize CCP1 pin high, clear output on compare match
    CCP2CON=12;         // PWM mode, P1A, P1C active-high; P1B, P1D active-high
    //PIR1bits.CCP1IF=0;    // maybe for capture mode
    //IPR1bits.CCP1IP = 1;  // maybe for capture mode

    return ;
}
