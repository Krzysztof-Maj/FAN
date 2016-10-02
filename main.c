/* 
 * File:   main.c
 * Author: Krzysiek
 *
 * Created on 21 lipiec 2016, 17:20
 */
#include"main.h"


int main(void) {
    system_Init();

    while (1) {
        IR_function();
        update_Diode();
        time_Event();
        BT_uart();
    }
}



