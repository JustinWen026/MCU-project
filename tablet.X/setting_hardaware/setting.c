// CONFIG1H
#pragma config OSC = INTIO67      // Oscillator Selection bits (HS oscillator)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enable bit (Fail-Safe Clock Monitor disabled)
#pragma config IESO = ON       // Internal/External Oscillator Switchover bit (Oscillator Switchover mode disabled)

// CONFIG2L
#pragma config PWRT = OFF       // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOREN = SBORDIS  // Brown-out Reset Enable bits (Brown-out Reset enabled in hardware only (SBOREN is disabled))
#pragma config BORV = 3         // Brown Out Reset Voltage bits (Minimum setting)

// CONFIG2H
#pragma config WDT = OFF        // Watchdog Timer Enable bit (WDT disabled (control is placed on the SWDTEN bit))
#pragma config WDTPS = 1        // Watchdog Timer Postscale Select bits (1:1)

// CONFIG3H
#pragma config CCP2MX = PORTC   // CCP2 MUX bit (CCP2 input/output is multiplexed with RC1)
#pragma config PBADEN = ON      // PORTB A/D Enable bit (PORTB<4:0> pins are configured as analog input channels on Reset)
#pragma config LPT1OSC = OFF    // Low-Power Timer1 Oscillator Enable bit (Timer1 configured for higher power operation)
#pragma config MCLRE = ON       // MCLR Pin Enable bit (MCLR pin enabled; RE3 input pin disabled)

// CONFIG4L
#pragma config STVREN = ON      // Stack Full/Underflow Reset Enable bit (Stack full/underflow will cause Reset)
#pragma config LVP = OFF         // Single-Supply ICSP Enable bit (Single-Supply ICSP enabled)
#pragma config XINST = OFF      // Extended Instruction Set Enable bit (Instruction set extension and Indexed Addressing mode disabled (Legacy mode))

// CONFIG5L
#pragma config CP0 = OFF        // Code Protection bit (Block 0 (000800-001FFFh) not code-protected)
#pragma config CP1 = OFF        // Code Protection bit (Block 1 (002000-003FFFh) not code-protected)
#pragma config CP2 = OFF        // Code Protection bit (Block 2 (004000-005FFFh) not code-protected)
#pragma config CP3 = OFF        // Code Protection bit (Block 3 (006000-007FFFh) not code-protected)

// CONFIG5H
#pragma config CPB = OFF        // Boot Block Code Protection bit (Boot block (000000-0007FFh) not code-protected)
#pragma config CPD = OFF        // Data EEPROM Code Protection bit (Data EEPROM not code-protected)

// CONFIG6L
#pragma config WRT0 = OFF       // Write Protection bit (Block 0 (000800-001FFFh) not write-protected)
#pragma config WRT1 = OFF       // Write Protection bit (Block 1 (002000-003FFFh) not write-protected)
#pragma config WRT2 = OFF       // Write Protection bit (Block 2 (004000-005FFFh) not write-protected)
#pragma config WRT3 = OFF       // Write Protection bit (Block 3 (006000-007FFFh) not write-protected)

// CONFIG6H
#pragma config WRTC = OFF       // Configuration Register Write Protection bit (Configuration registers (300000-3000FFh) not write-protected)
#pragma config WRTB = OFF       // Boot Block Write Protection bit (Boot block (000000-0007FFh) not write-protected)
#pragma config WRTD = OFF       // Data EEPROM Write Protection bit (Data EEPROM not write-protected)

// CONFIG7L
#pragma config EBTR0 = OFF      // Table Read Protection bit (Block 0 (000800-001FFFh) not protected from table reads executed in other blocks)
#pragma config EBTR1 = OFF      // Table Read Protection bit (Block 1 (002000-003FFFh) not protected from table reads executed in other blocks)
#pragma config EBTR2 = OFF      // Table Read Protection bit (Block 2 (004000-005FFFh) not protected from table reads executed in other blocks)
#pragma config EBTR3 = OFF      // Table Read Protection bit (Block 3 (006000-007FFFh) not protected from table reads executed in other blocks)

// CONFIG7H
#pragma config EBTRB = OFF      // Boot Block Table Read Protection bit (Boot block (000000-0007FFh) not protected from table reads executed in other blocks)



#include "setting.h"

void TMR2_Initialize(void){
    PIR1bits.TMR2IF = 0; // flag
    PIE1bits.TMR2IE = 1; // enable
    IPR1bits.TMR2IP = 1; // high priority
    T2CONbits.T2OUTPS = 0x0f;  // PostScale 1 : 16
    T2CONbits.T2CKPS  = 0x03;  // PreScale 1 : 16
    T2CONbits.TMR2ON = 1; // Enable Timer2
    PR2 = 255; // 0.1 sec interrupt, for 1MHZ,   PR2 MAX = 1M / 4 / 16 / 16 = 976, which is 1 sec
              //                    for 250KHZ, PR2 MAX = 250K / 4 / 16 / 16 = 244, which is 1 sec 
              //                    for 125KHZ, PR2 MAX = 125K / 4 / 16 / 16 = 125, which is 1 sec 
}

void TMR1_Initialize(void){
    // Set up timer1
    T1CONbits.TMR1CS = 0;    // Use Internal clock
    T1CONbits.T1CKPS = 0b10; // Prescalar 1:4

    // Set initial TMR1 variable, Interrupt will occur when TMR1 reachs 65535
    // To calculate TMR1, increment_in_1sec = Fosc / 4 / prescalar.
    // In this case, increment_in_1sec = 1000000 / 4 / 4 = 62500
    //               increment_in_20ms = 62500 / 50 = 1250
// Timer = 65536 - (10000 * 1000000) / (prescalar * _XTAL_FREQ)
//       = 65536 - (10000 * 1000000) / (16 * 1000000)
//       = 65536 - 625
//       = 64911 = 0xFDAF

TMR1H = 0xFD;
TMR1L = 0xAF;

    PIR1bits.TMR1IF = 0;     // interrupt flag bit, 1 => interrupt occured
    PIE1bits.TMR1IE = 1;     // Enable Timer1 Interrupt
    IPR1bits.TMR2IP = 1;     // Timer1 high priority
    T1CONbits.TMR1ON = 1;    // Timer1 on
}


void SYSTEM_Initialize(void)
{
    OSCILLATOR_Initialize(); //default 1Mhz
    TMR2_Initialize();
    //TMR1_Initialize();
    //TMR0_Initialize();
    INTERRUPT_Initialize();
    UART_Initialize();

    
    ADC_Initialize();
    //CCP1_Initialize();
    //CCP2_Initialize();
    return ;
}

void OSCILLATOR_Initialize(void)
{
    IRCF2 = 1; // default setting 1M Hz
    IRCF1 = 1;
    IRCF0 = 1;
    return ;
    // RCON = 0x0000;
}

