#include "IR.h"

volatile unsigned int uiPreData, IrData, uiIRDelay;
volatile unsigned char frameStatus, IrPulseCoun, ir_licznik;
volatile unsigned long ulKeyCodeTmp, ulKeyCode;
extern volatile unsigned int uiNastawa;
extern rtcTimeWDay RtcTimeWDay, sleepTime;
extern timeFlags TimeFlags;

static void ir_power(void);
static void ir_speed_up(void);
static void ir_speed_down(void);
static void ir_sleep_up(void);
static void ir_sleep_down(void);

void IR_function() {
    if (!uiIRDelay) {
        if (ulKeyCode) {
            switch (ulKeyCode) {
                case IR_POWER:
                    ir_power();
                    ulKeyCode = 0;
                    uiIRDelay = 10000;
                    break;
                case IR_SPEED_UP:
                    ir_speed_up();
                    ulKeyCode = 0;
                    uiIRDelay = 5000;
                    break;
                case IR_SPEED_DOWN:
                    ir_speed_down();
                    ulKeyCode = 0;
                    uiIRDelay = 5000;
                    break;
                case IR_SLEEP_UP:
                    ir_sleep_up();
                    ulKeyCode = 0;
                    uiIRDelay = 5000;
                    break;
                case IR_SLEEP_DOWN:
                    ir_sleep_down();
                    ulKeyCode = 0;
                    uiIRDelay = 5000;
                    break;
                default:
                    ulKeyCode = 0;
            }
        }
        if (uiPreData == 0x4014) {
            uiPreData = 0;
            ulKeyCode = ulKeyCodeTmp;
        }
    } else uiPreData &= ~0x0010;
}

void __attribute__((interrupt,no_auto_psv)) _IC1Interrupt(void) {

    unsigned int Tmp = TMR1>>1;
    TMR1 = 0;
    if (Tmp > 1700) ir_licznik = 0;
    if (ir_licznik > 1) frameStatus = frame_ok;
    if (ir_licznik == 1) ++ir_licznik;
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
static void ir_power(void){
    if (!uiNastawa) {
        uiNastawa = SPEED_ONE;
    } else {
        uiNastawa = TURN_OFF;
        TimeFlags.gosleep = TURN_OFF;
    }
}
static void ir_speed_up(void){
    if (uiNastawa) {
        if (uiNastawa < SPEED_ONE) uiNastawa = SPEED_ONE;
        else if (uiNastawa < SPEED_TWO) uiNastawa = SPEED_TWO;
        else if (uiNastawa < SPEED_THREE) uiNastawa = SPEED_THREE;
        else if (uiNastawa < SPEED_FOUR) uiNastawa = SPEED_FOUR;
        else if (uiNastawa < SPEED_FIFE) uiNastawa = SPEED_FIFE;
        else if (uiNastawa < SPEED_SIX) uiNastawa = SPEED_SIX;
        else uiNastawa = SPEED_SEVEN;
    }
}
static void ir_speed_down(void) {
    if (uiNastawa) {
        if (uiNastawa > SPEED_SIX) uiNastawa = SPEED_SIX;
        else if (uiNastawa > SPEED_FIFE) uiNastawa = SPEED_FIFE;
        else if (uiNastawa > SPEED_FOUR) uiNastawa = SPEED_FOUR;
        else if (uiNastawa > SPEED_THREE) uiNastawa = SPEED_THREE;
        else if (uiNastawa > SPEED_TWO) uiNastawa = SPEED_TWO;
        else uiNastawa = SPEED_ONE;
    }
}
unsigned int give_minutes (rtcTimeWDay *basic, rtcTimeWDay *checked){
    int minutes;
    if (basic->hour < checked->hour || basic->hour == checked->hour)
            minutes =  (checked->hour - basic->hour) * 60;
    else minutes = (24 - basic->hour + checked->hour) * 60;
        minutes += (checked->minute - basic->minute);       // if hour is eg basic->17:56 , 
        if (minutes > 0) return minutes;    // checked->17:34, value will be negative
        else return 0;
}
void add_minutes (unsigned int minutes, rtcTimeWDay *modify){
    unsigned char tmp, min;
    tmp = minutes / 60;
    min = minutes%60;
    modify->hour += tmp;
    modify->minute += min;
    if (modify->minute > 59) {
        modify->minute -= 60;
        ++(modify->hour);
    }
    if (modify->hour > 23) modify->hour -=24;
}
static void subtract_minutes (unsigned int minutes, rtcTimeWDay *modify){
    unsigned char tmp, min;
    tmp = minutes / 60;
    min = minutes%60;
    if (modify->minute >= min) modify->minute -=min;
    else {
        modify->minute += (60-min);
        ++tmp;
    }
    if (modify->hour >= tmp) modify->hour -= tmp;
    else    modify->hour += (24-tmp);
}
static void ir_sleep_up(void){
    unsigned int time;
    if (uiNastawa){
        if (TimeFlags.gosleep){
            time = give_minutes(&RtcTimeWDay,&sleepTime);
            if (time < 181) add_minutes(30, &sleepTime);
            else {
                time -=180;
                add_minutes(30 - time, &sleepTime);
            }
        } else {
            sleepTime = RtcTimeWDay;
            add_minutes(30, &sleepTime);
            TimeFlags.gosleep = 1;
        }
    }
}
static void ir_sleep_down(void){
    unsigned int time;
    if (TimeFlags.gosleep){
        time = give_minutes(&RtcTimeWDay, &sleepTime);
        if (time < 30) TimeFlags.gosleep = 0;
        else subtract_minutes(30, &sleepTime);
    }
}