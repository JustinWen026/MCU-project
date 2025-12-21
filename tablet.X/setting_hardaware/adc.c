#include <xc.h>

void ADC_Initialize(void) {
    TRISAbits.RA0 = 1;       		// Set as input port
    
    ADCON1bits.VCFG0 = 0;
    ADCON1bits.VCFG1 = 0;
    ADCON1bits.PCFG = 0b1110; //AN0 ?analog input, other digital
    ADCON0bits.CHS = 0b0000;  //AN0 ?? analog input
    ADCON2bits.ADCS = 0b001;  // T_AD must > 0.7 us, For 8MHZ, TAD = 8 * TOSC = 8 * 1 / 8000000 = 1 us
    ADCON2bits.ACQT = 0b010;  // T_ACQ must > 2.4 us, we choose T_ACQ = 4 T_AD = 4 us
    ADCON0bits.ADON = 1;
    ADCON2bits.ADFM = 1 ;          // Right Justifie
    
    PIE1bits.ADIE = 1;
    PIR1bits.ADIF = 0;
   
    ADRESH=0;  			// Flush ADC output Register
    ADRESL=0;  
}

