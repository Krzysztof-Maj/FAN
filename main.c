/* 
 * File:   main.c
 * Author: Krzysiek
 *
 * Created on 21 lipiec 2016, 17:20
 */
#include"main.h"

int main(void) {
    systemInit();

    while (1) {
        IR_function();
        update_Diode();
        time_Event();
    }
}



