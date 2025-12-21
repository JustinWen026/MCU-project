#include "main.h"

#include <stdlib.h>

#include "setting_hardaware/setting.h"
#include "stdio.h"
#include "string.h"

int axis = 0;  // 0 for x, 1 for y

char str[20];

#define _XTAL_FREQ 1000000

void ANPinReset() {
    TRISAbits.RA0 = 1;
    TRISAbits.RA1 = 1;
    TRISAbits.RA2 = 1;
    TRISAbits.RA3 = 1;
    TRISAbits.RA4 = 1;
    LATAbits.LATA0 = 0;
    LATAbits.LATA1 = 0;
    LATAbits.LATA2 = 0;
    LATAbits.LATA3 = 0;
    LATAbits.LATA4 = 0;
}

void adc0Init() {
    // reset

    ANPinReset();

    /*
        x axis
        pin1 - , AN3, **Digital** Output LOW
        pin3 + , AN4, **Digital** Output HIGH

        AN2 read PIN2,  **Analog** Input
    */

    TRISAbits.RA2 = 1;  // Set as input port

    TRISAbits.RA3 = 0;  // Set as output port
    TRISAbits.RA4 = 0;  // Set as output port

    LATAbits.LATA3 = 0;  // output low
    LATAbits.LATA4 = 1;  // output high

    ADCON1bits.PCFG = 0b1100;  // AN0-2 analog input, other digital
    ADCON0bits.CHS = 0b0010;   // AN2 analog input
    __delay_us(5);             // Acquisition time
}

void adc1Init() {
    // reset
    ANPinReset();

    /*
        y axis
        pin2 - , AN2 Digital Output LOW
        pin4 +, AN1, Digital Output HIGH

        AN0 read AN3(will not go in),  AN3(will not go in) read PIN1, Analog
       Input
    */

    TRISAbits.RA0 = 1;  // Set as input port

    TRISAbits.RA1 = 0;  // Set as output port
    TRISAbits.RA2 = 0;  // Set as output port

    LATAbits.LATA2 = 0;  // output low
    LATAbits.LATA1 = 1;  // output high

    ADCON1bits.PCFG = 0b1110;  // AN0-2 analog input, other digital
    ADCON0bits.CHS = 0b0000;   // AN0 analog input
    __delay_us(5);             // Acquisition time
}

void LEDInit() {
    // LED output
    TRISBbits.TRISB3 = 0;
    TRISBbits.TRISB2 = 0;
    TRISBbits.TRISB1 = 0;
    TRISBbits.TRISB0 = 0;
    LATBbits.LATB3 = 0;
    LATBbits.LATB2 = 0;
    LATBbits.LATB1 = 0;
    LATBbits.LATB0 = 0;
}

void main(void) {
    SYSTEM_Initialize();

    LEDInit();

    // step3
    if (axis == 0) adc0Init();

    if (axis == 1) adc1Init();

    ADCON0bits.GO = 1;

    while (1) {
        /*
        rotateTo(20);
        __delay_ms(1000);
        rotateTo(40);
        __delay_ms(1000);
         * */
    }

    return;
}

void readX() {
    int adcRead = (ADRESH << 8) | ADRESL;  // 0~1023
    char x_high = (adcRead >> 8) & 0xFF;   // 8 bit
    char x_low = adcRead & 0xFF;           // 8 bit

    UART_Write(0xAA);
    UART_Write(0x55);
    UART_Write(x_high);
    UART_Write(x_low);
}

void readY() {
    int adcRead = (ADRESH << 8) | ADRESL;  // 0~1023

    char y_high = (adcRead >> 8) & 0xFF;  // 8 bit
    char y_low = adcRead & 0xFF;          // 8 bit

    UART_Write(y_high);
    UART_Write(y_low);
}

void __interrupt(high_priority) Hi_ISR(void) {
    if (PIR1bits.ADIF) {
        // step4

        // 0 for x, 1 for y
        if (axis == 0) readX();
        if (axis == 1) readY();

        // switch axis
        axis ^= 1;

        if (axis == 0) adc0Init();
        if (axis == 1) adc1Init();

        // clear flag bit
        PIR1bits.ADIF = 0;

        __delay_us(10);
        ADCON0bits.GO = 1;
    }

    if (RCIF) {
        if (RCSTAbits.OERR) {
            CREN = 0;
            Nop();
            CREN = 1;
        }

        MyusartRead();
    }
    return;
}
