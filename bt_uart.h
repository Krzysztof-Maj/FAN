/* 
 * File:   bt_uart.h
 * Author: Krzysiek
 *
 * Created on 12 wrzesie? 2016, 18:16
 */

#ifndef BT_UART_H
#define	BT_UART_H

#ifdef	__cplusplus
extern "C" {
#endif
#include "simpleRtc.h"

#define RXBUFF_SIZE 32
#define TXBUFF_SIZE 32
#define RX_MAX_POS  (RXBUFF_SIZE-1)
#define TX_MAX_POS  (TXBUFF_SIZE-1)

#define BT_MAX_CMD	16

typedef struct {
	unsigned char received_data:1;
	unsigned char recived_command:1;
	unsigned char rewrite:1;
	unsigned char rsvd:5;
}uartFlags;
#define TRUE 1
#define FALSE 0

void BT_uart(void);

#ifdef	__cplusplus
}
#endif

#endif	/* BT_UART_H */

