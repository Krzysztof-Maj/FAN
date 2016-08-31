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
#define SPEED_ONE   40///////////////////
#define SPEED_TWO   60
#define SPEED_THREE 90/////////////////////
#define SPEED_FOUR  110
#define SPEED_FIFE  140
#define SPEED_SIX   170
#define SPEED_SEVEN 200//////////////////

void IR_function(void);

#ifdef	__cplusplus
}
#endif

#endif	/* IR_H */

