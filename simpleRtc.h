/* 
 * File:   simpleRtc.h
 * Author: Krzysiek
 *
 * Created on 30 sierpie? 2016, 21:19
 */

#ifndef SIMPLERTC_H
#define	SIMPLERTC_H

#ifdef	__cplusplus
extern "C" {
#endif

enum wDay {Monday = 1, Tuesday, Wednesday, Thursday, Friday, Saturday, Sunday };
typedef enum _BOOL { FALSE = 0, TRUE } BOOL;

typedef struct {
    unsigned char second;
    unsigned char minute;
    unsigned char hour;
    unsigned char wday;
}rtcTimeWDay;

typedef struct {
    unsigned char gosleep:1;
    unsigned char checksleep:1;
    unsigned char wakeup:1;
    unsigned char checkwakeup:1;
    unsigned char rsvd:4;
}timeFlags;

void time_Event(void);

#ifdef	__cplusplus
}
#endif

#endif	/* SIMPLERTC_H */

