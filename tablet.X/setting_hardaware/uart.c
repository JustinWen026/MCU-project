#include <xc.h>
    //setting TX/RX
char mystring[200];
int lenStr = 0;
int stringReady = 0;



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

    //  Setting baud rate 19200
    TXSTAbits.SYNC = 0;                     
    TXSTAbits.BRGH = 1;
    BAUDCONbits.BRG16 = 1;  
  
    SPBRG = 103; //for 8MHZ      

   //   Serial enable
    RCSTAbits.SPEN = 1;     // Enable Serial Port, namely Setting RC6, 7 as Uart pin           
    PIR1bits.TXIF = 0;      // When TXREG is empty, TXIF is set
    PIR1bits.RCIF = 1;      // When the current data finish processing, RCIF is set, data will be sent to RCREG 
    TXSTAbits.TXEN = 1;     // Enable transimission      
    RCSTAbits.CREN = 1;     // Enalbe receiver        
    PIE1bits.TXIE = 0;      // Disable TX interrupt !!!!!!!!!! Otherwise it will keep sending interrupt
    IPR1bits.TXIP = 0;      // TX , low priority interrupt       
    PIE1bits.RCIE = 1;      // enable RX interrupt        
    IPR1bits.RCIP = 0;      // RX , low priority interrupt      

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
    if(RCIF)
    {
        if(RCSTAbits.OERR)
        {
            CREN = 0;
            Nop();
            CREN = 1;
        }

        MyusartRead();
        

    }

   // process other interrupt sources here, if required
    return;
}