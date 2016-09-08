#include "sysSwLed.h"
#include "simpleRtc.h"

volatile unsigned int uiSWDelay,uiTriac, uiKroki, uiNastawa, uiLedUpdate;

extern volatile unsigned int uiIRDelay, usecond;
extern rtcTimeWDay RtcTimeWDay, sleepTime;
extern timeFlags TimeFlags;

sdiode dSpeed = {&LATB, &LATB, &LATB, 2, 4, 5};
sdiode dSleep = {&LATB, &LATB, &LATB, 7, 6, 0};

static void key_SPEED() {
    if (uiNastawa) {
        switch (uiNastawa) {
            case SPEED_ONE:
                uiNastawa = SPEED_THREE;
                break;
            case SPEED_THREE:
                uiNastawa = SPEED_SEVEN;
                break;
            default:
                uiNastawa = SPEED_ONE;
                break;
        }
    }
}
static void diode_u(u08 tmp, sdiode *tmpDiode ){
    if (tmp & 0x01) *(tmpDiode->latch1) |= 1<<tmpDiode->led1_pin;
    else *(tmpDiode->latch1) &= ~(1<<tmpDiode->led1_pin);
    if (tmp & 0x02) *(tmpDiode->latch2) |= 1<<tmpDiode->led2_pin;
    else *(tmpDiode->latch2) &= ~(1<<tmpDiode->led2_pin);
    if (tmp & 0x04) *(tmpDiode->latch3) |= 1<<tmpDiode->led3_pin;
    else *(tmpDiode->latch3) &= ~(1<<tmpDiode->led3_pin);
}
static void diode_Speed(void){
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
static void diode_Slepp(void){
    unsigned char tmp;
    if (TimeFlags.gosleep ){
        signed int minutes;
        if (RtcTimeWDay.hour < sleepTime.hour || RtcTimeWDay.hour == sleepTime.hour)
            minutes =  (sleepTime.hour - RtcTimeWDay.hour) * 60;
        else minutes = (24 - RtcTimeWDay.hour + sleepTime.hour) * 60;
        minutes += (sleepTime.minute - RtcTimeWDay.minute);
        if (minutes > 209)    tmp = 7;   // max time 210 min
        else tmp = (minutes / 30) + 1;;
    } else tmp = 0;
    diode_u(tmp, &dSleep);
}
void update_Diode(){
    if (!uiLedUpdate) {
        if (!uiNastawa) POWER_LED = TURN_OFF;
        else POWER_LED = TURN_ON;
        diode_Speed();
        diode_Slepp();
        uiLedUpdate = 500; // 25 ms
    }
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
    IPC7bits.INT2IP = 3;
    IFS1bits.INT2IF = 0;
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
    IC1CON1bits.ICM = 1;        // capture every edge
    IC1CON1bits.ICTSEL = 4;     // Timer1 as select timer
    IC1CON2bits.SYNCSEL = 0;   // synchronization ICP1
    IC1CON2bits.ICTRIG = 1;
    IC1CON2bits.TRIGSTAT = 1;
    IEC0bits.IC1IE = 1;         // turn on interrupt
    IFS0bits.IC1IF = 0;

}
void __attribute__((interrupt,no_auto_psv)) _INT1Interrupt(void) // PowerSW
{
    uiSWDelay = 20;
    while (uiSWDelay);
        if (!POWER_SW){
            if (!uiNastawa){
                uiNastawa = SPEED_ONE;
            } else {
                uiNastawa = TURN_OFF;
                TimeFlags.gosleep = TURN_OFF;
            }
        }
    IFS1bits.INT1IF = 0; //wyczy?? flag? przerwania
}
#ifndef DEBUG__
void __attribute__((interrupt,no_auto_psv)) _INT2Interrupt(void)    // SpeedSW
{
    uiSWDelay = 20;
    while (uiSWDelay);
    if (!SPEED_SW) key_SPEED();
    IFS1bits.INT2IF = 0; //wyczy?? flag? przerwania
}
#endif
void __attribute__((interrupt,no_auto_psv)) _INT3Interrupt(void)    // SleepSW
{
    uiSWDelay = 20;
    while (uiSWDelay);
    if (!SLEEP_SW) {
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
    ++usecond;
    IFS0bits.T3IF = 0; //wyczy?? flag? przerwania
}


