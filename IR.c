#include "IR.h"

volatile unsigned int uiPreData, IrData, uiIRDelay;
volatile unsigned char frameStatus, IrPulseCoun, ir_licznik;
volatile unsigned long ulKeyCodeTmp, ulKeyCode;
extern volatile unsigned int uiNastawa;


void IR_function() {
    if (!uiIRDelay) {
        if (ulKeyCode) {
            switch (ulKeyCode) {
                case IR_POWER:
                    if (!uiNastawa) {
                        uiNastawa = SPEED_ONE;
                    } else {
                        uiNastawa = TURN_OFF;
                    }
                    ulKeyCode = 0;
                    uiIRDelay = 5000;
                    break;
                case IR_SPEED_UP:
                    if (!uiNastawa) {
                        uiNastawa = SPEED_ONE;
                    } else {
                        uiNastawa += 10;
                        if (uiNastawa > 200) uiNastawa = 200;
                    }
                    ulKeyCode = 0;
                    uiIRDelay = 2500;
                    break;
                case IR_SPEED_DOWN:
                    if (uiNastawa) {
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
