#include <xc.h>
    //setting TX/RX
char mystring[20];
int lenStr = 0;
int stringReady = 0;


#define _XTAL_FREQ 1000000
#define PULSE_WIDTHH 2400
#define PULSE_WIDTHL 400
#define T_osc 1
#define prescalar 16

void rotateTo(int deg) {
	long pulse_width = (long)deg * (PULSE_WIDTHH - PULSE_WIDTHL) / 180 + PULSE_WIDTHL;
	int duty_cycle = pulse_width / (T_osc * prescalar);
	CCPR1L = (duty_cycle >> 2) & 0xFF;
	CCP1CONbits.DC1B = duty_cycle & 0x03;
}

void UART_Initialize() {

    /*       TODObasic   
           Serial Setting      
        1.   Setting Baud rate
        2.   choose sync/async mode 
        3.   enable Serial port (configures RX/DT and TX/CK pins as serial port pins)
        3.5  enable Tx, Rx Interrupt(optional)
        4.   Enable Tx & RX
     *  */   

    // Set to 1 to enable RX/TX, It's not simply RC6 RC7
    TRISCbits.TRISC6 = 1;            
    TRISCbits.TRISC7 = 1;            

    //  Setting baud rate 2400
    TXSTAbits.SYNC = 0;                     
    TXSTAbits.BRGH = 1;
    BAUDCONbits.BRG16 = 0;  
  
    SPBRG = 25; //for 1MHZ     

   //   Serial enable
    RCSTAbits.SPEN = 1;     // Enable Serial Port, namely Setting RC6, 7 as Uart pin           
    PIR1bits.TXIF = 0;      // When TXREG is empty, TXIF is set
    PIR1bits.RCIF = 1;      // When the current data finish processing, RCIF is set, data will be sent to RCREG 
    TXSTAbits.TXEN = 1;     // Enable transimission      
    RCSTAbits.CREN = 1;     // Enalbe receiver        
    PIE1bits.TXIE = 0;      // Disable TX interrupt !!!!!!!!!! Otherwise it will keep sending interrupt
    IPR1bits.TXIP = 1;      // TX , low priority interrupt       
    PIE1bits.RCIE = 1;      // enable RX interrupt        
    IPR1bits.RCIP = 1;      // RX , low priority interrupt      

    }
void UART_Write(unsigned char data)  // Output on Terminal
{
        while(!TXSTAbits.TRMT);
    TXREG = data;              //write to TXREG will send data 
}
void UART_Write_Text(char* text) { // Output on Terminal, limit:10 chars
    for(int i=0;text[i]!='\0';i++)
        UART_Write(text[i]);
}
void ClearBuffer(){
    for(int i = 0; i < 20 ; i++)
        mystring[i] = '\0';
    lenStr = 0;
    stringReady = 0;
}
void MyusartRead()
{
    /* TODObasic: try to use UART_Write to finish this function */
    unsigned char ch = RCREG; // read from RCREG
    UART_Write(ch);
    if(stringReady == 0) {
        if(ch == '\n' || ch == '\r'){
            mystring[lenStr] = '\0';
            stringReady = 1;
        }
        else if(ch == 'W'){
                    LATBbits.LATB3 = 1;
                    LATBbits.LATB2 = 1;
                    LATBbits.LATB1 = 1;
                    LATBbits.LATB0 = 1;
                    rotateTo(90);
        }
        else if(ch == 'L'){
                    LATBbits.LATB3 = 0;
                    LATBbits.LATB2 = 0;
                    LATBbits.LATB1 = 0;
                    LATBbits.LATB0 = 0;
                    rotateTo(5);
        }
        else{
            mystring[lenStr++] = ch;
        }
        
    }


    return ;
}
char *GetString(){
    return mystring;
}

int isStringReady(){
    return stringReady;
}
// void interrupt low_priority Lo_ISR(void)
void __interrupt(low_priority)  Lo_ISR(void)
{


   // process other interrupt sources here, if required
    return;
}