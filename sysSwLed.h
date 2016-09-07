/* 
 * File:   swLed.h
 * Author: Krzysiek
 *
 * Created on 31 sierpie? 2016, 19:38
 */

#ifndef SWLED_H
#define	SWLED_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "IR.h"
typedef unsigned char u08;
typedef unsigned int u16;
typedef struct {
    volatile unsigned int *latch1;
    volatile unsigned int *latch2;
    volatile unsigned int *latch3;
    unsigned int led1_pin;
    unsigned int led2_pin;
    unsigned int led3_pin;
} sdiode;

#define DEBUG__

#define LICZBA_KROKOW   200
#define TRIAC_OFF       LATFbits.LATF5 = 0
#define TRIAC_ON        LATFbits.LATF5 = 1
#define POWER_LED       LATBbits.LATB14

#define POWER_SW        PORTDbits.RD1
#define SPEED_SW        PORTBbits.RB1
#define SLEEP_SW        PORTBbits.RB15

void update_Diode(void);
void systemInit(void);

#ifdef	__cplusplus
}
#endif

#endif	/* SWLED_H */

