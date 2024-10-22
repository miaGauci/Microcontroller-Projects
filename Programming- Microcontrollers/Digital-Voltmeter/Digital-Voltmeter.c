#pragma config FOSC = HSH //sets the oscillator type to High-Speed Frequency 
#pragma config WDTEN = OFF
#pragma config LVP = ON //enables Low Voltage Programming

#define _XTAL_FREQ 4000000 
#include <xc.h>

    volatile unsigned short VolData = 0;
    unsigned char sevSeg[10] = {0b11111100, 0b01100000, 0b11011010, 0b11110010, 0b01100110,0b10110110, 0b10111110, 0b11100100, 0b11111110, 0b11100110};

    void __interrupt(high_priority) IntHi(void){
        //checking if CCP2 interrupt flag is set. If true, the flag is cleared and the device is put to sleep if(PIR2bits.CCP2IF == 1){
        PIR2bits.CCP2IF = 0; asm("SLEEP");
        }
        //if the AD interrupt flag is set, if true it reads the analog to digital conversion result and stores it in VolData and the AD interrupt flag is cleared.
        else if (PIR1bits.ADIF == 1){
        VolData = (0x0300&((unsigned short)ADRESH<<8))|(0x00FF&((unsigned short)ADRESL)); PIR1bits.ADIF = 0;
        }
    //ISR function returned
        return;
    }

    void UpClock(){
        OSCCONbits.SCS = 0x3; // Set the system clock source to Internal Oscillator (SCS = 11) 
        OSCCONbits.IRCF = 0x5; // Set the Internal Oscillator frequency to 4 MHz (IRCF = 101) 
        OSCCONbits.IDLEN = 0; // Disable the idle mode (IDLEN = 0)
        return;
    }
//display conversion
    unsigned short SwapDisplay(unsigned short data, unsigned short digit){ 
        unsigned short num = 0;
        for(unsigned int i = 4; i>0; i--){
            num = data%10; // Extract the least significant digit of 'data'
            data = data/10; // Update 'data' by removing the least significant digit 
            if(digit == i){
                return num; // Return the digit corresponding to the specified 'digit' 
            }
        } 
    }
    // If 'digit' is not in the range of 1 to 4, the function does not return a value.

    void NumDisplay(unsigned char num, unsigned char digit){
        unsigned NumRep = sevSeg[num]; // Get the corresponding 7-segment representation for the given 'num'
        // Setup Pins to display the 7-segment representation 
        PORTAbits.RA1 = (unsigned)((NumRep&0x80) == 0x80); 
        PORTAbits.RA2 = (unsigned)((NumRep&0x40) == 0x40); 
        PORTAbits.RA3 = (unsigned)((NumRep&0x20) == 0x20); 
        PORTAbits.RA4 = (unsigned)((NumRep&0x10) == 0x10); 
        PORTAbits.RA5 = (unsigned)((NumRep&0x08) == 0x08); 
        PORTEbits.RE0 = (unsigned)((NumRep&0x04) == 0x04); 
        PORTEbits.RE1 = (unsigned)((NumRep&0x02) == 0x02);

        if(integ == 1){
            PORTEbits.RE2 = 1; // Turn on the decimal point LED if 'integ' is 1
        } 
        else {
            PORTEbits.RE2 = 0; // Turn off the decimal point LED if 'integ' is not 1 
        }

        switch(integ){ 
            case 1:
                PORTAbits.RA7 = 0; // Enable the specific digit by driving its corresponding control pin low
                break; 
            case 2:
                PORTAbits.RA6 = 0;
                break; 
            case 3:
                PORTCbits.RC0 = 0;
                break; 
            case 4:
                PORTCbits.RC1 = 0;
                break; 
            }

    // Delay for a short period to allow the display to show the number 
        __delay_ms(2);
    // Turn off all digit control pins to disable all digits
        PORTAbits.RA7 = 1; 
        PORTAbits.RA6 = 1; 
        PORTCbits.RC0 = 1; 
        PORTCbits.RC1 = 1;
        return; 
    }

    void main(void){
        UpClock(); // configure clock settings

        unsigned short voltage = 0; // Initialise variables to store voltage values
        unsigned short n1 = 0; //Initialize variables for individual digits unsigned short n2 = 0;
        unsigned short n3 = 0;
        unsigned short n4 = 0;

        //Configuration of Analog Port
        ANSELA = 0; ANSELB = 0; ANSELC = 0; ANSELD = 0; ANSELE = 0;

        ANSELAbits.ANSA0 = 1; // Set RA0 as analog input 
        ANSELAbits.ANSA0 = 1; // Set RA0 as analog input (duplicate line)
        TRISA = 0; TRISB = 0; TRISC = 0; TRISD = 0; TRISE = 0;
        TRISAbits.RA0 = 1; // Set RAO as input
        
        //ADC Module Configuration 
        VREFCON0bits.FVRS = 3; // FVR = 4.096V
        VREFCON0bits.FVREN = 1; // Enable FVR 
        __delay_ms (0.2); // Delay to stabilize FVR

        ADCON0bits.CHS = 0;// Select Channel 0 for ADC conversion 
        ADCON0bits.ADON = 1;
        ADCON1bits. PVCFG = 2; //Use FVR
        ADCON1bits.NVCFG = 0;
        ADCON1bits. TRIGSEL = 0; //SET Source is CCP2 
        ADCON2bits.ADFM = 1; // Right-justify result 
        ADCON2bits.ACQT = 0b111; // Acquisition Timer = 8Tad 
        ADCON2bits.ADCS = 3; // Clock Conversion Select is FRC
        
        //ADC interrupt
        PIR1bits.ADIF = 0; // Clearing interrupt flag 
        IPR1bits.ADIP = 1; // Set ADC interrupt priority as high 
        PIE1bits.ADIE = 1; //Enable ADC interrupts 
        RCONbits.IPEN = 1; //Enable interrupt priorities 
        INTCONbits.GIEH = 1; // Enable high priority interrupt

        //Configure timer 1
        T1CONbits.TMR1CS = 0; // Fosc/4
        T1CONbits.SOSCEN = 0; // Disable sec oscillator 
        T1CONbits.T1CKPS = 2; // Prescaler of 1:4
        T1CONbits.RD16 = 1; // Enable register read/write in one operation 
        T1GCONbits.TMR1GE = 0; // Timer1 Gate function disabled 
        T1GCONbits.T1SYNC = 1; // Synchronize external clock input

        //Configure CCP2 to generate SET
        CCP2CONbits.CCP2M = 2;
        CCPR2H = 0xFF;
        CCPR2L = 0xFF;
        IPR2bits.CCP2IP = 1;
        PIR2bits.CCP2IF = 0;
        CCP2CONbits.CCP2M = 0b1011; // Generate SET on match 
        PIE2bits.CCP2IE = 1;
        T1CONbits.TMR1ON = 1;

        //Main Loop
        while (1) {
        voltage = (VoltageData * 4);
        num_1 = SwapDisplay(voltage, 1); // Obtain first digit 
        num_2 = SwapDisplay(voltage, 2); // Obtain second digit 
        num_3 = SwapDisplay(voltage, 3); // Obtain third digit 
        num_4 = SwapDisplay(voltage, 4); // Obtain fourth digit 
        NumDisplay (n1,1);
        NumDisplay (n2,2); NumDisplay (n3,3); NumDisplay (n4,4); }

        return; 
    }
