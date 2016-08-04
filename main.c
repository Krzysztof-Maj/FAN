/* 
 * File:   main.c
 * Author: Krzysiek
 *
 * Created on 21 lipiec 2016, 17:20
 */
#define FCY 16000000UL
#include <stdio.h>
#include <stdlib.h>
#include <libpic30.h>
#include <Rtcc.h>

#include <xc.h>

// CONFIG3
#pragma config WPFP = WPFP511           // Write Protection Flash Page Segment Boundary (Highest Page (same as page 170))
#pragma config WPDIS = WPDIS            // Segment Write Protection Disable bit (Segmented code protection disabled)
#pragma config WPCFG = WPCFGDIS         // Configuration Word Code Page Protection Select bit (Last page(at the top of program memory) and Flash configuration words are not protected)
#pragma config WPEND = WPENDMEM         // Segment Write Protection End Page Select bit (Write Protect from WPFP to the last page of memory)

// CONFIG2
#pragma config POSCMOD = NONE           // Primary Oscillator Select (Primary oscillator disabled)
#pragma config DISUVREG = OFF           // Internal USB 3.3V Regulator Disable bit (Regulator is disabled)
#pragma config IOL1WAY = ON             // IOLOCK One-Way Set Enable bit (Write RP Registers Once)
#pragma config OSCIOFNC = OFF           // Primary Oscillator Output Function (OSCO functions as CLKO (FOSC/2))
#pragma config FCKSM = CSDCMD           // Clock Switching and Monitor (Both Clock Switching and Fail-safe Clock Monitor are disabled)
#pragma config FNOSC = FRCPLL           // Oscillator Select (Fast RC oscillator with Postscaler and PLL module (FRCPLL))
#pragma config PLL_96MHZ = ON           // 96MHz PLL Disable (Enabled)
#pragma config PLLDIV = NODIV           // USB 96 MHz PLL Prescaler Select bits (Oscillator input used directly (4MHz input))
#pragma config IESO = ON                // Internal External Switch Over Mode (IESO mode (Two-speed start-up) enabled)

// CONFIG1
#pragma config WDTPS = PS32768          // Watchdog Timer Postscaler (1:32,768)
#pragma config FWPSA = PR128            // WDT Prescaler (Prescaler ratio of 1:128)
#pragma config WINDIS = OFF             // Watchdog Timer Window (Standard Watchdog Timer enabled,(Windowed-mode is disabled))
#pragma config FWDTEN = OFF             // Watchdog Timer Enable (Watchdog Timer is disabled)
#pragma config ICS = PGx1               // Comm Channel Select (Emulator functions are shared with PGEC1/PGED1)
#pragma config GWRP = OFF               // General Code Segment Write Protect (Writes to program memory are allowed)
#pragma config GCP = OFF                // General Code Segment Code Protect (Code protection is disabled)
#pragma config JTAGEN = OFF              // JTAG Port Enable (JTAG port is enabled)

#define LICZBA_KROKOW   200
#define TRIAC_OFF       LATFbits.LATF5 = 0;
#define TRIAC_ON        LATFbits.LATF5 = 1;

#define bitOneMax       1700
#define bitZeroMin      370

#define frame_restart   0
#define frame_ok        1
#define frame_end       2
#define frame_err       3

#define TURN_OFF    0
#define TURN_ON     1
#define SPEED_ONE   40
#define SPEED_TWO   100
#define SPEED_THREE 200

volatile unsigned char frameStatus, IrPulseCoun, ir_licznik, uchPowerON;
volatile unsigned int uiPreData, IrData;
volatile unsigned long ulKeyCodeTmp, ulKeyCode;
volatile unsigned int uiPowerSWDelay, uiSleepSWDelay, uiSpeedSWDelay, uiIRDelay,uiTriac, uiKroki, uiNastawa;

void systemInit(void);
void turnOffAllDiode(void);
/*
 * 
 */
int main(void) {
    systemInit();
    uiNastawa = 50;

    while (1){
        if (!uiIRDelay){
            if (ulKeyCode){
                switch(ulKeyCode){
                    case 0x0D00BCB1:
                        if (!uchPowerON) {
                            uchPowerON = TURN_ON;
                            uiNastawa = SPEED_ONE;
                            LATBbits.LATB14 = 1;
                            LATBbits.LATB2 = 1;
                        } else {
                            uchPowerON = TURN_OFF;
                            uiNastawa = 0;
                            turnOffAllDiode();
                        }
                        ulKeyCode = 0;
                        uiIRDelay = 5000;
                        break;
                    case 0x0D00111C:
                        if (!uchPowerON) {
                            uchPowerON = TURN_ON;
                            uiNastawa = SPEED_ONE;
                            LATBbits.LATB14 = 1;
                            LATBbits.LATB2 = 1;
                        } else {
                            uiNastawa += 10;
                            if (uiNastawa > 200) uiNastawa = 200;
                        }
                        ulKeyCode = 0;
                        uiIRDelay = 2500;
                        break;
                    case 0x0D00E1EC:
                        if (uchPowerON) {
                            uiNastawa -= 10;
                            if (uiNastawa < 10) uiNastawa = 10;
                        }
                        ulKeyCode = 0;
                        uiIRDelay = 2500;
                        break;
                }
            }
            if (uiPreData == 0x4014){
                uiPreData = 0;
                ulKeyCode = ulKeyCodeTmp;
            }
        } else uiPreData = 0;
    }
}

void __attribute__((interrupt,no_auto_psv)) _INT1Interrupt(void) // PowerSW
{
    if (!uiPowerSWDelay){
        if (!PORTDbits.RD1){
            if (!uchPowerON){
                uchPowerON = TURN_ON;
                uiNastawa = SPEED_ONE;
                LATBbits.LATB14 = 1;
                LATBbits.LATB2 = 1;
            } else {
                uchPowerON = TURN_OFF;
                uiNastawa = 0;
                turnOffAllDiode();
            }
        }
    }
    uiPowerSWDelay = 2000; // 100 ms Delay
    IFS1bits.INT1IF = 0; //wyczy?? flag? przerwania
}

void __attribute__((interrupt,no_auto_psv)) _INT2Interrupt(void)    // SpeedSW
{
    if (!uiSpeedSWDelay){
        if (!PORTBbits.RB1){
            if (uchPowerON == TURN_ON){
                switch (uiNastawa){
                    case SPEED_ONE:
                        uiNastawa = SPEED_TWO;
                        LATBbits.LATB2 = 1;
                        LATBbits.LATB4 = 1;
                        LATBbits.LATB5 = 0;
                        break;
                    case SPEED_TWO:
                        uiNastawa = SPEED_THREE;
                        LATBbits.LATB2 = 1;
                        LATBbits.LATB4 = 1;
                        LATBbits.LATB5 = 1;
                        break;
                    default:
                        uiNastawa = SPEED_ONE;
                        LATBbits.LATB2 = 1;
                        LATBbits.LATB4 = 0;
                        LATBbits.LATB5 = 0;
                        break;
                }
            }
        }
    }
    uiSpeedSWDelay = 2000;   // 100 ms Delay
    IFS1bits.INT2IF = 0; //wyczy?? flag? przerwania
}

void __attribute__((interrupt,no_auto_psv)) _INT3Interrupt(void)    // SleepSW
{
    if (!uiSleepSWDelay){
        if (!PORTBbits.RB15){
            
        }
    }
    uiSleepSWDelay = 2000;   // 100 ms Delay
    IFS3bits.INT3IF = 0; //wyczy?? flag? przerwania
}

void __attribute__((interrupt,no_auto_psv)) _INT4Interrupt(void)
{
    TRIAC_OFF;     // wy??czenie triaka
    uiTriac = uiNastawa;
    uiKroki = LICZBA_KROKOW;
    IFS3bits.INT4IF = 0; //wyczy?? flag? przerwania
}

void __attribute__((interrupt,no_auto_psv)) _T3Interrupt(void)
{
    if (uiPowerSWDelay) --uiPowerSWDelay;
    if (uiSpeedSWDelay) --uiSpeedSWDelay;
    if (uiSleepSWDelay) --uiSleepSWDelay;
    if (uiIRDelay) --uiIRDelay;
    if (uiKroki == uiTriac) TRIAC_ON;
    --uiKroki;
    IFS0bits.T3IF = 0; //wyczy?? flag? przerwania
}
void __attribute__((interrupt,no_auto_psv)) _IC1Interrupt(void) {

    unsigned int Tmp = TMR1>>1;
    TMR1 = 0;
//    unsigned int Tmp = IC1TMR>>1;
//    IC1CON2bits.TRIGSTAT = 0;
    if (Tmp > 1700) ir_licznik = 0;
    if (ir_licznik > 1) frameStatus = frame_ok;
    if (ir_licznik == 1) ++ir_licznik;
//    IC1CON2bits.TRIGSTAT = 1;
    if (ir_licznik == 0) {
        IrData = 0;
        IrPulseCoun = 0;
        ++ir_licznik;
        frameStatus = frame_end;
    }

    if (frameStatus == frame_ok) {
        if (Tmp < bitZeroMin) frameStatus = frame_restart;
        if (Tmp > bitOneMax) frameStatus = frame_restart;

        if (frameStatus == frame_ok) {
            if (ir_licznik > 1) {
                if ((ir_licznik % 2) == 0) {
                    IrData = IrData << 1;
                    if (Tmp > 1000) ++IrData;
//                    uiCaptureValue[IrPulseCoun] = Tmp;
                    IrPulseCoun++;
                    if (IrPulseCoun == 16) {
                        uiPreData = IrData;
                    }
                    if (IrPulseCoun == 32) {
                        ulKeyCodeTmp = IrData;
                    }
                    if (IrPulseCoun == 48) {
                        ulKeyCodeTmp <<= 16;
                        ulKeyCodeTmp |= IrData;
                        uiPreData |= 0x0010;
                        frameStatus = frame_restart;
                    }
                }
            }
            ir_licznik++;
        }
    }
    if (frameStatus == frame_restart) {
        ir_licznik = 0;
    }
    IFS0bits.IC1IF = 0;
}
void systemInit(void){
    //    CLKDIVbits.RCDIV = 1;
    AD1PCFGL = 0xFFFF; // all as digital
    TRISB = ~(0b0100000011110101);  // diody
    TRISFbits.TRISF5 = 0;   // triac kierunek wyjsciowy

    //////     INT1    //////
    RPINR0bits.INT1R = 24;  // RP24
    INTCON2bits.INT1EP = 1;
    IEC1bits.INT1IE = 1;
    //////     INT2    //////
    RPINR1bits.INT2R = 1;   // RP1
    INTCON2bits.INT2EP = 1;
    IEC1bits.INT2IE = 1;
    //////     INT3    //////
    RPINR1bits.INT3R = 29;  // RP29
    INTCON2bits.INT3EP = 1; // negative edge
    IEC3bits.INT3IE = 1;    // enable
    //////     INT4    //////
    RPINR2bits.INT4R = 10;  // RP10
    INTCON2bits.INT4EP = 0; // positive edge
//    IPC13bits.INT4IP = 5;   // priorytet przerwania, domyslnie jest 4
    IEC3bits.INT4IE = 1;
    //////////////////////////


    //////     TIMER 2/3    //////
    T2CONbits.T32 = 1;      // 32 bits mode
    T2CONbits.TCKPS = 0;    // prescaler 1:1
    T2CONbits.TCS = 0; //TCS = 0/1 taktowanie wewn?trzne/zewn?trzne
    T2CONbits.TGATE = 0; //TGATE = 0/1 Gated Time Accumulation Mode wy??czony/w??czony
    PR3 = 0x00;   // przerwanie co 50 us
    PR2 = 0x320;
    IEC0bits.T3IE = 1; //w??cz przerwanie
    IFS0bits.T3IF = 0; //wyczy?? flag? przerwania
    T2CONbits.TON = 1;
    ///////////////////////////////

    ////////    Timer 1    ////////
    T1CONbits.TCKPS = 1;    // preskaler 1:8
    T1CONbits.TCS = 0;      // taktowanie wewnetrzne
    T1CONbits.TGATE = 0;    // gated disable
    IEC0bits.T1IE = 0;      // wylacz przerwanie
    IFS0bits.T1IF = 0;      // wyczysc flage
    T1CONbits.TON = 1;      // wlacz zliczanie
    //////// Input Capture ////////
    RPINR7bits.IC1R = 16;       // RP16
    IC1CON2bits.SYNCSEL = 0;    // no synchronization
//    while (IC1CON1bits.ICBNE) uiCaptureValueICP[0] = IC1BUF;   // cleared buff
    IC1CON1bits.ICM = 1;        // capture every edge
    IC1CON1bits.ICTSEL = 4;     // Timer1 as select timer
//    IC1CON1bits.ICI = 0;        // interrupt on every fourth capture event //// is not used
    IC1CON2bits.SYNCSEL = 0;   // synchronization ICP1
    IC1CON2bits.ICTRIG = 1;
    IC1CON2bits.TRIGSTAT = 1;
    IEC0bits.IC1IE = 1;         // turn on interrupt
    IFS0bits.IC1IF = 0;
    ///////////////////////////////
    turnOffAllDiode();
}
void turnOffAllDiode(void){
    LATBbits.LATB14 = 0;    // power on diode
    LATBbits.LATB2 = 0;     // first speed diode
    LATBbits.LATB4 = 0;    // second speed diode
    LATBbits.LATB5 = 0;    // third speed diode
    LATBbits.LATB7 = 0;    // first sleep diode
    LATBbits.LATB6 = 0;    // second sleep diode
    LATBbits.LATB0 = 0;    // third sleep diode
}