#include "bt_uart.h"

volatile char rx_buffer[RXBUFF_SIZE];
volatile char tx_buffer[TXBUFF_SIZE];
volatile unsigned char rx_head, rx_position;
volatile unsigned char tx_head, tx_position;

void __attribute__((interrupt, no_auto_psv)) _U1RXInterrupt(void) {
    char data, tmp;
    data = U1RXREG;
    tmp = (rx_head + 1) & RX_MAX_POS;
    if (tmp != rx_position){
        rx_head = tmp;
        rx_buffer[rx_head] = data;
    }
    IFS0bits.U1RXIF = 0;
}
void __attribute__((interrupt, no_auto_psv)) _U1TXInterrupt(void) {
    if (tx_position != tx_head){
        tx_position = (tx_position + 1) & TX_MAX_POS;
        U1TXREG = tx_buffer[tx_position];
    }
    IFS0bits.U1TXIF = 0;
}
