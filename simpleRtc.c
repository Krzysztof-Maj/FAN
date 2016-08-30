#include "simpleRtc.h"

rtcTimeWDay RtcTimeWDay, sleepTime, wupTime;
volatile unsigned int usecond;


void updateTime(void){
    if (usecond >=20000){
        usecond -= 20000;
        ++RtcTimeWDay.second;
        if (RtcTimeWDay.second >=60){
            RtcTimeWDay.second -= 60;
            ++RtcTimeWDay.minute;
            if (RtcTimeWDay.minute >= 60){
                RtcTimeWDay.minute -= 60;
                ++RtcTimeWDay.hour;
                if (RtcTimeWDay.hour >= 24){
                    RtcTimeWDay.hour -= 24;
                    ++RtcTimeWDay.wday;
                    if (RtcTimeWDay.wday >= 8) RtcTimeWDay.wday = Monday;
                }
            }
        }
    }
}
