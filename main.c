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
#include"main.h"
#define DEBUG__



void systemInit(void);
void turnOffAllDiode(void);
void key_SPEED();
void update_Diode();
void IR_function();

int main(void) {
    systemInit();

    while (1) {
        IR_function();
        update_Diode();
    }
}

void IR_function() {
    if (!uiIRDelay) {
        if (ulKeyCode) {
            switch (ulKeyCode) {
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
        if (uiPreData == 0x4014) {
            uiPreData = 0;
            ulKeyCode = ulKeyCodeTmp;
        }
    } else uiPreData &= ~0x0010;
}

void __attribute__((interrupt,no_auto_psv)) _INT1Interrupt(void) // PowerSW
{
    uiSWDelay = 100;
    while (uiSWDelay);
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
    IFS1bits.INT1IF = 0; //wyczy?? flag? przerwania
}
#ifndef DEBUG__
void __attribute__((interrupt,no_auto_psv)) _INT2Interrupt(void)    // SpeedSW
{
    uiSWDelay = 20;
    while (uiSWDelay);
    if (!PORTBbits.RB1) key_SPEED();
    IFS1bits.INT2IF = 0; //wyczy?? flag? przerwania
}
#endif
void __attribute__((interrupt,no_auto_psv)) _INT3Interrupt(void)    // SleepSW
{
    uiSWDelay = 100;
    while (uiSWDelay);
    if (!PORTBbits.RB15) {
        key_SPEED();
    }
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
    if (uiSWDelay) --uiSWDelay;
    if (uiIRDelay) --uiIRDelay;
    if (uiLedUpdate) --uiLedUpdate;
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
    IPC5bits.INT1IP = 3;
    IFS1bits.INT1IF = 0;
    IEC1bits.INT1IE = 1;
#ifndef DEBUG__
    //////     INT2    //////
    RPINR1bits.INT2R = 1;   // RP1
    INTCON2bits.INT2EP = 1;
    IEC1bits.INT2IE = 1;
#endif
    //////     INT3    //////
    RPINR1bits.INT3R = 29;  // RP29
    INTCON2bits.INT3EP = 1; // negative edge
    IPC13bits.INT3IP = 3;
    IFS3bits.INT3IF = 0;
//    INTCON1bits.NSTDIS = 1;
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
void key_SPEED() {
    if (uchPowerON == TURN_ON) {
        switch (uiNastawa) {
            case SPEED_ONE:
                uiNastawa = SPEED_THREE;
//                LATBbits.LATB2 = 1;
//                LATBbits.LATB4 = 1;
//                LATBbits.LATB5 = 0;
                break;
            case SPEED_THREE:
                uiNastawa = SPEED_SEVEN;
//                LATBbits.LATB2 = 1;
//                LATBbits.LATB4 = 1;
//                LATBbits.LATB5 = 1;
                break;
            default:
                uiNastawa = SPEED_ONE;
//                LATBbits.LATB2 = 1;
//                LATBbits.LATB4 = 0;
//                LATBbits.LATB5 = 0;
                break;
        }
    }
}
void diode_u(u08 tmp, sdiode *tmpDiode ){
    if (tmp & 0x01) *(tmpDiode->latch1) |= 1<<tmpDiode->led1_pin;
    else *(tmpDiode->latch1) &= ~(1<<tmpDiode->led1_pin);
    if (tmp & 0x02) *(tmpDiode->latch2) |= 1<<tmpDiode->led2_pin;
    else *(tmpDiode->latch2) &= ~(1<<tmpDiode->led2_pin);
    if (tmp & 0x04) *(tmpDiode->latch3) |= 1<<tmpDiode->led3_pin;
    else *(tmpDiode->latch3) &= ~(1<<tmpDiode->led3_pin);
}
void diode_speed(void){
    unsigned int tmp;
    if (uiNastawa<SPEED_ONE) tmp = 0;
    else if (uiNastawa<SPEED_TWO) tmp = 1;
    else if (uiNastawa<SPEED_THREE) tmp = 2;
    else if (uiNastawa<SPEED_FOUR) tmp = 3;
    else if (uiNastawa<SPEED_FIFE) tmp = 4;
    else if (uiNastawa<SPEED_SIX) tmp = 5;
    else if (uiNastawa<SPEED_SEVEN) tmp = 6;
    else tmp = 7;
    diode_u(tmp, &dSpeed);
}
void diode_slepp(void){
    
}
void update_Diode(){
    if (!uiLedUpdate) {
        diode_speed();
        diode_slepp();
        uiLedUpdate = 1000; // 50 ms
    }
}