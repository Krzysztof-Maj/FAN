/* 
 * File:   IR.h
 * Author: Krzysiek
 *
 * Created on 31 sierpie? 2016, 19:01
 */

#ifndef IR_H
#define	IR_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <xc.h>
#include "sysSwLed.h"

#define bitOneMax       1700
#define bitZeroMin      370

#define frame_restart   0
#define frame_ok        1
#define frame_end       2
#define frame_err       3

#define TURN_OFF    0
#define TURN_ON     1
#define SPEED_ONE   49///////////////////
#define SPEED_TWO   72
#define SPEED_THREE 91/////////////////////
#define SPEED_FOUR  109
#define SPEED_FIFE  128
#define SPEED_SIX   151
#define SPEED_SEVEN 200//////////////////

#define IR_POWER        0x0D00BCB1
#define IR_SPEED_UP     0x0D00111C
#define IR_SPEED_DOWN   0x0D00E1EC
#define IR_SLEEP_UP     0x0D00818C
#define IR_SLEEP_DOWN   0x0D004944

void IR_function(void);

#ifdef	__cplusplus
}
#endif

#endif	/* IR_H */

