#pragma config WDTEN = OFF              // Watchdog Timer Enable bits (WDT disabled in hardware (SWDTEN ignored))
#pragma config CPUDIV = 00              //post scalar /1.
#pragma config FOSC = HSH

#include <stdio.h>
#include <stdlib.h>
#include <xc.h>
#include <math.h>

volatile unsigned short start = 0;//stores the value of the push button on/off 1/0
volatile unsigned short led = 1;//LED value for Mhz/Hz 1/2
volatile unsigned long int t1 = 0; //timer 1
volatile unsigned long int t2 = 0; //timer 2
volatile unsigned long int t3 = 0; //timer 3
volatile unsigned long int overflowt1 = 0; //timer 1 overflow
volatile unsigned long int overflowt3 = 0; //timer 3 overflow
volatile unsigned long int DFCval = 0; //freq result 1
volatile unsigned long int RFCval = 0; //freq result 2
volatile unsigned long int counter = 0; //stores the rising edges
volatile unsigned long int count = 0; //Decimal counter
volatile unsigned long int decimal = 0; //stores the decimal place
volatile unsigned int i = 0; //digit counter TMR0 IMP not to use
volatile long double Freq = 1000; // stores the Frequency value
unsigned char BCD[7] = {0,0,0,0,0,0,0,0}; //BCD values
unsigned char sevSeg[10] = {0b11111100,0b01100000,0b11011010,0b11110010,0b01100110,0b10110110,0b10111110,0b11100100,0b11111110,0b11100110}; //display numbers 0-9


void indicator(){
    //sets the LED to Hz/MHz
    switch(led)
    {
        case 1:
            LATBbits.LB4 = 1; //Hz
            LATBbits.LB5 = 0;
            
            break;
            
        case 2:
            LATBbits.LB4 = 0; //MHz
            LATBbits.LB5 = 1;
            
            break;
        
        }
    return;
}

void RFC(){
    //starts timer 3 
    INTCON3bits.INT1IE = 1; //enables external interrupt pin (INT1)(TMR3) 
    //will count the rising edges in (TMR3) in 1 input pulse (LOW FREQ)
    //TMR 3 running on internal clock FOSC/4 -> 16MHz/4
    
    if(T3CONbits.TMR3ON == 0){
        
        t3 = TMR3;
        
        //16MHz/4 = 4MHz Internal clock
        RFCval = (4000000/t3) + overflowt3*(65536); //frequency calculated from RFC internal clock*4
        
        t3 = 0;
        TMR3 = 0;
        overflowt3 = 0;
        
    } 
    
    INTCON3bits.INT1IE = 0; //disables external interrupt pin (INT1)(TMR3)
    
    return;
}

void Compare(){
    
    if(DFCval >= 100){
        
        Freq = DFCval;
    }
    else{
        RFC();
        Freq = RFCval;
    }
    
    return;
}

void DFC(){
    //will count the amount of pulses of the input in a given time (using TMR2) (HIGH FREQ)
    //tMR 1 will count all the rising edges for 1 second (1 second is set using timer 2)

    if(t2 == 0){

        TMR1 = 0;
        //TMR 1 overflow at 2^16 so cater with it by counting the number of over flows using the interrupt.
        t2 = 0;
        
        T2CONbits.TMR2ON = 1; // starts timer 2 (1s timer)
        T1CONbits.TMR1ON = 1; // starts timer 1 (counter)
    }
    
    //double checks that t2 counted to 375 (1s)
    if(t2 >= 375){
        
        DFCval = TMR1 + (65536)*overflowt1; //number of rising edges counted by TMR1
        TMR1 = 0;   //resets counter
        overflowt1 = 0;
        t2 = 0; //resets timer (1s)
    }
    
    return;
}

//displaying the digit  
void DisplayNum(unsigned int num /*0 - 9*/, unsigned int digit){
    //The BCD number and the digit to be displayed are passed
    unsigned Numdis = sevSeg[num];
    
    //Pull decimal pin high
    LATAbits.LA1 = 1;
    
    //Pull digits high of DISPLAY
    LATDbits.LD0 = 1;//Digit 1, FIRST DISPLAY
    LATDbits.LD1 = 1;//Digit 2
    LATDbits.LD2 = 1;//Digit 3
    LATDbits.LD3 = 1;//Digit 4
    
    LATDbits.LD4 = 1;//Digit 5, SECOND DISPLAY
    LATDbits.LD5 = 1;//Digit 6
    LATDbits.LD6 = 1;//Digit 7
    LATDbits.LD7 = 1;//Digit 8
    
    //selects segments to turn on
    LATAbits.LA2 = (unsigned)((Numdis&0x80) == 0x80); //A
    LATAbits.LA3 = (unsigned)((Numdis&0x40) == 0x40); //B
    LATAbits.LA4 = (unsigned)((Numdis&0x20) == 0x20); //C
    LATAbits.LA5 = (unsigned)((Numdis&0x10) == 0x10); //D
    LATEbits.LE0 = (unsigned)((Numdis&0x08) == 0x08); //E
    LATEbits.LE1 = (unsigned)((Numdis&0x04) == 0x04); //F
    LATEbits.LE2 = (unsigned)((Numdis&0x02) == 0x02); //G
    
    
    //Set decimal point
    if(decimal == digit){
    LATAbits.LA1 = 1;
    }
    else
        {
        LATAbits.LA1 = 0;
            }
    
    
    //Display number
    switch(digit){
        //switch to output on RD
        case 0:
            LATDbits.LD0 = 0; //DISPLAY 1
            break;
        case 1:
            LATDbits.LD1 = 0;
            break;
        case 2:
            LATDbits.LD2 = 0;
            break;
        case 3:
            LATDbits.LD3 = 0;
            break;
            
        case 4: 
            LATDbits.LD4 = 0; //DISPLAY 2
            break;
        case 5:
            LATDbits.LD5 = 0;
            break;
        case 6:
            LATDbits.LD6 = 0;
            break;
        case 7:
            LATDbits.LD7 = 0;
            break;
    }
    
    //__delay_ms(20);      // To see values on display.
   //don't use software delay use timer 0 instead (low priority interrupts)
    
    return;
}

void decimal_to_bcd() {
    //converts the frequency decimal to BCD
    unsigned long int FreqUSE;
        
    count = 0;
    
    //checking if Frequency is > than 100
    if(Freq >= 100){
        //dividing Frequency from Hz to MHz 
        Freq = Freq/1000000;
        //count = count + 1;
        led = 2;    //setting MHz LED
    }
    else{
        led = 1;    //setting Hz LED
    }
    
    FreqUSE = Freq;
    indicator(); // update LED
    
    //finding the decimal place
    while(FreqUSE >= 10){
        //dividing the Frequency until its less than 1.
        FreqUSE = FreqUSE/10;
        //increment the decimal place
        count = count + 1; //counting number of decimal shifts
    }
    
    decimal = count;//setting global decimal place variable
    count = 0;
    
    //removes the decimal point to convert to BCD
    FreqUSE = Freq*pow(10,(7-decimal));
    
    //changing Decimal to BCD
    for (int counterr = 7; counterr >= 0; counterr--) {
        BCD[counterr] = FreqUSE % 10;
        FreqUSE = FreqUSE/10;
    }
    return;
}

//low priority interrupts
void __interrupt(low_priority) isrLo (void){
    //external switch interrupt (make sure to account for bouncing)
    if(INTCON3bits.INT2IF == 1){
        //if button is pressed, switch on/off system
        if(start == 0){
            start = 1;  //turn on function
        }
        else{
            start = 0;  //turn off function
        }
        
        INTCON3bits.INT2IF = 0;  //clear flag
    }
    
    //setting TMR0 to update the display at 72.5HZ
    if(INTCONbits.TMR0IF == 1){
        
            if(i == 0){
                i = 8; //resets to the first digit
            }
            
            unsigned int BCDnum;
            i = i - 1;//decrements the digits from 8
            BCDnum = BCD[i];
            DisplayNum(BCDnum,i);
            
            //72.5Hz
            TMR0H = 0xFF; // Set the high byte of Timer0
            TMR0L = 0xB0; // Set the low byte of Timer0
            
        INTCONbits.TMR0IF = 0;//clears the interrupt flag
    }    
    return;
}

//high priority interrupts
void __interrupt(high_priority) isrHi(void) {
    //timer 1 overflow for when frequency goes above 2^16
    if(PIR1bits.TMR1IF == 1){
        TMR1 = 0;
        overflowt1 = overflowt1 + 1;
        
        PIR1bits.TMR1IF = 0;
    }
    
    //timer 3 overflow for when frequency goes low
    if(PIR2bits.TMR3IF == 1){
         TMR3 = 0;
        overflowt3 = overflowt3 + 1;
        
        PIR2bits.TMR3IF = 0;
    }
    
    //timer2 interrupt flag -- DFC --
    if(PIR1bits.TMR2IF == 1){
        //1 second counter
        if(t2 <= 375){
        t2 = t2 +1;
        }
        else{
            //timer stops counting 
            T1CONbits.TMR1ON = 0; //stops timer 1
            T2CONbits.TMR2ON = 0; //stops timer 2
            
            //the timer stops and the system resets
        }
        
        PIR1bits.TMR2IF = 0;   //clears flag
    }
    
    //timer3 interrupt start/stop (physically connect TMR1 input (RC0) pin with INT1 (RB1))
    if(INTCON3bits.INT1IF == 1){
        //triggers on every rising edge of the input signal
        //if DFC value is less than 100Hz it will start RFC (TMR3)
        if(T3CONbits.TMR3ON == 0){
            //start counter (TMR3)
            TMR3 = 0; //reset counter
            overflowt3 = 0;
            T3CONbits.TMR3ON = 1;
        }
        else{
            //stop counter
            T3CONbits.TMR3ON = 0;
        }
        
        INTCON3bits.INT1IF = 0; //clear flag
    }
    
    return; 
}

void setextosc()
{   
    //secondary oscillator turned OFF/ON
    OSCCON2bits.SOSCGO = 0;
    //OSC1-2 pins
    OSCCONbits.SCS=0b00;
    //ENABLE PLL
    OSCCON2bits.PRISD=1;
    //EN primary clock
    OSCCON2bits.PLLEN=1;
    //PLL x4 (12MHz * 4 = 48MHz)
    OSCTUNEbits.SPLLMULT= 0;
    
    return;
}

//start/stop between 2 input pulses (RFC)
void settmr3(){
//setting up timer 3    
    T3CONbits.TMR3ON = 0;   //disables timer3
    IPR2bits.TMR3IP = 1;      //timer 3 interrupt is high priority
    T3CONbits.T3CKPS = 0b00;    //setting timer3 pre scaler 1
    PIE2bits.TMR3IE = 1;    //enables overflow interrupt pin (for very low frequencies)
    T3CONbits.TMR3CS = 0;   // clock source is Fosc/4 
    
    TMR3 = 0;
    t3 = 0;
    
    return;
}

//counts to 1 second (DFC interval)
void settmr2(){
//setting up timer 2
    T2CONbits.TMR2ON = 0;   //disables timer2
    IPR1bits.TMR2IP=1;      //timer 2 interrupt is high priority
    T2CONbits.T2OUTPS = 0b1111; //setting timer2 post scaler to 1:16
    T2CONbits.T2CKPS = 0b10; //setting timer2 pre scaler 1:16
    PR2 = 125; //setting period of timer 2 to 125. (0.0026s) 
    PIE1bits.TMR2IE=1;      //enables TMR2 interrupt
    
    TMR2 = 0;
    t2 = 0;
    return;
}

//counts input pulses
void settmr1(){
    //setting up timer 1
    T1CONbits.TMR1ON = 0; // disable Timer1
    T1CONbits.TMR1CS = 0b10; // Set Timer1 to use external clock source
    T1CONbits.SOSCEN = 0;   //sets the input to T1CKI
    T1CONbits.T1CKPS = 0b00; // Set Timer1 pre scaler to 1:1
    T1CONbits.T1SYNC = 0b1; // Set Timer1 to not synchronise external clock
    
    //Enable timer1 interrupts
    PIE1bits.TMR1IE = 1; // Enable Timer1 interrupt
    
    TMR1 = 0;   //resets count
    t1 = 0;
    return;
}

//delay for display
void settmr0(){
    //calling a low priority interrupt 72.5 times a second (each digit)
    
    T0CONbits.TMR0ON = 1;//Turns off TMR0
    T0CONbits.T08BIT = 0;//sets timer to 16 bits
    T0CONbits.T0CS = 0;//sets source to Fosc/4
    T0CONbits.PSA = 0;//turn on pre scale
    T0CONbits.T0PS = 0b111;// 1:256 pre scale value
    TMR0H = 0xFF; // Set the high byte of Timer0
    TMR0L = 0xB0; // Set the low byte of Timer0
    
    INTCONbits.INT0IE = 0; //disables external interrupt
    INTCONbits.TMR0IE = 1; //enable overflow interrupt
    INTCON2bits.TMR0IP = 0; //setting TMR0 to Low Priority
    
    return;
}

void setInterrupts(){
    RCONbits.IPEN = 1; //Enable interrupt priorities
    INTCONbits.GIE_GIEH = 1; //enables all unmasked interrupts 
    INTCONbits.GIE = 1;//Global enable interrupts
    INTCONbits.PEIE_GIEL=1; //enables peripheral interrupts
    
    //setting interrupt for start button INT2 (RB2)
    ANSELBbits.ANSB0=0;//digital input function
    PORTBbits.INT2 = 1; //enables External interrupt on PortB for button
    //TRISBbits.RB2 = 1;//setting RB2 to input for button
    INTCON3bits.INT2IE = 1; //enables external interrupt pin (INT2)
    INTCON2bits.INTEDG2 = 1;    //sets interrupt to falling edges (INT2)
    INTCON3bits.INT2IP = 0; //sets INT2 to Low priority
    
    //setting counter interrupt (INT1) RFC
    INTCON2bits.INTEDG1 = 1; //sets the interrupt to rising edge (INT1)
    INTCON3bits.INT1IE = 0; //disables external interrupt pin (INT1)
    INTCON3bits.INT1IP = 1; //sets INT1 to high priority

    return;
}

void Initiliase(){
    //set all pins to digital function
    ANSELA = 0;
    ANSELB = 0;
    ANSELC = 0;
    ANSELD = 0;
    ANSELE = 0;
    
    //1 = input, 0 = output //CHECK THIS
    TRISA &= ~0x3E;  //0011 1110 (numbers 1 + decimal place)
    TRISE &= ~0x07;  //0000 0111 (numbers 2)
    TRISB &= ~0x30; //0011 0000 (2 output Hz/MHz LEDs)
    TRISC &= ~0x00;  //0000 0000 (unchanged so inputs)
    TRISD &= ~0xFF; //1111 1111 (digits outputs)
    
    //outputs all to 0
    PORTA = 0;
    PORTB = 0;
    PORTC = 0;
    PORTD = 0;
    PORTE = 0;
    
    //Pull digits high of DISPLAY
    LATDbits.LD0 = 1;//Digit 1, FIRST DISPLAY
    LATDbits.LD1 = 1;//Digit 2
    LATDbits.LD2 = 1;//Digit 3
    LATDbits.LD3 = 1;//Digit 4
    LATDbits.LD4 = 1;//Digit 5, SECOND DISPLAY
    LATDbits.LD5 = 1;//Digit 6
    LATDbits.LD6 = 1;//Digit 7
    LATDbits.LD7 = 1;//Digit 8
    
    return;
}

void main(void) 
{
  Initiliase();//startup settings  
  setextosc();//Primary oscillator configuration
  settmr3();//TMR3 configuration
  settmr2();//TMR2 configuration
  settmr1();//TMR1 configuration
  settmr0();//TMR0 configuration
  setInterrupts();//Interrupts Configuration
  
   
 while(1)
    {
        switch(start)
        {   
            case(0):
                Freq = 0;   //OFF
                decimal_to_bcd();
                break;
            
            case(1):
                
                DFC();  //frequency calculator
                Compare(); // checks if the value of Frequency is more than 100Hz
                decimal_to_bcd(); //converts final value to BCD to send to display
                break;
        }
    }            
    return;
}

