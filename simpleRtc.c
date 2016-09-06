#include "simpleRtc.h"
#include "IR.h"

rtcTimeWDay RtcTimeWDay, sleepTime, wupTime;
volatile unsigned int usecond;
timeFlags TimeFlags;
extern volatile unsigned int uiNastawa;

void update_Time(void){
    if (usecond >=20000){
        usecond -= 20000;
        ++RtcTimeWDay.second;
        TimeFlags.checksleep = 1;
        TimeFlags.checkwakeup = 1;
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
static BOOL second_equal(rtcTimeWDay *base, rtcTimeWDay *checked){
    if (base->second == checked->second) return TRUE;
    else return FALSE;
}
static BOOL minute_equal(rtcTimeWDay *base, rtcTimeWDay *checked){
    if (base->minute == checked->minute) return TRUE;
    else return FALSE;
}
static BOOL hour_equal (rtcTimeWDay *base, rtcTimeWDay *checked){
    if (base->hour == checked->hour) return TRUE;
    else return FALSE;
}
static BOOL wday_equal (rtcTimeWDay *base, rtcTimeWDay *checked){
    if (base->wday == checked->wday) return TRUE;
    else return FALSE;
}
void go_sleep (rtcTimeWDay *actTime, rtcTimeWDay *sleepTime){
    if(TimeFlags.gosleep && TimeFlags.checksleep){
        TimeFlags.checksleep = 0;
        if (second_equal(actTime, sleepTime))
            if (minute_equal(actTime, sleepTime))
                if (hour_equal(actTime, sleepTime))
                    if (wday_equal(actTime, sleepTime)){
                        uiNastawa = 0;
                        TimeFlags.gosleep = 0;
                    }
    }
}
void wake_up (rtcTimeWDay *actTime, rtcTimeWDay *wakeupTime){
    if(TimeFlags.wakeup && TimeFlags.checkwakeup){
        TimeFlags.checkwakeup = 0;
        if (second_equal(actTime, wakeupTime))
            if (minute_equal(actTime, wakeupTime))
                if (hour_equal(actTime, wakeupTime))
                    if (wday_equal(actTime, wakeupTime)){
                        uiNastawa = SPEED_TWO;
                        TimeFlags.wakeup = 0;
                    }
    }
}
void time_Event(void){
    update_Time();
    go_sleep(&RtcTimeWDay, &sleepTime);
    wake_up(&RtcTimeWDay, &wupTime);
}