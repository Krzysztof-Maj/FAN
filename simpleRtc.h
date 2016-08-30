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
typedef struct {
    unsigned char second;
    unsigned char minute;
    unsigned char hour;
    unsigned char wday;
}rtcTimeWDay;

void updateTime(void);

#ifdef	__cplusplus
}
#endif

#endif	/* SIMPLERTC_H */

