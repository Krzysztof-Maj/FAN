/* 
 * File:   main.h
 * Author: Krzysiek
 *
 * Created on 22 sierpie? 2016, 19:36
 */

#ifndef MAIN_H
#define	MAIN_H

#ifdef	__cplusplus
extern "C" {
#endif

typedef unsigned char u08;
typedef unsigned int u16;


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
#define SPEED_ONE   40///////////////////
#define SPEED_TWO   60
#define SPEED_THREE 90/////////////////////
#define SPEED_FOUR  110
#define SPEED_FIFE  140
#define SPEED_SIX   170
#define SPEED_SEVEN 200//////////////////

typedef struct {
    volatile unsigned int *latch1;
    volatile unsigned int *latch2;
    volatile unsigned int *latch3;
    unsigned int led1_pin;
    unsigned int led2_pin;
    unsigned int led3_pin;
} sdiode;

sdiode dSpeed = {&LATB, &LATB, &LATB, 2, 4, 5};
sdiode dSleep = {&LATB, &LATB, &LATB, 7, 6, 0};

volatile unsigned char frameStatus, IrPulseCoun, ir_licznik, uchPowerON;
volatile unsigned int uiPreData, IrData;
volatile unsigned long ulKeyCodeTmp, ulKeyCode;
volatile unsigned int uiSWDelay, uiIRDelay,uiTriac, uiKroki, uiNastawa, uiLedUpdate;
u08 key_State_Speed;


#ifdef	__cplusplus
}
#endif

#endif	/* MAIN_H */

