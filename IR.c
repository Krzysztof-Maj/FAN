#include "IR.h"

volatile unsigned int uiPreData, IrData, uiIRDelay;
volatile unsigned char frameStatus, IrPulseCoun, ir_licznik;
volatile unsigned long ulKeyCodeTmp, ulKeyCode;
extern volatile unsigned char uchPowerON;
extern unsigned int uiNastawa;


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
