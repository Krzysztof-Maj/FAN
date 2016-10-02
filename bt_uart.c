#include "bt_uart.h"
#include <xc.h>

volatile char rx_buffer[RXBUFF_SIZE];
volatile char tx_buffer[TXBUFF_SIZE];
volatile unsigned char rx_head, rx_position;
volatile unsigned char tx_head, tx_position;

extern rtcTimeWDay RtcTimeWDay, sleepTime, wupTime;
extern timeFlags TimeFlags;

char BT_command[BT_MAX_CMD];
unsigned char start_cmd;
volatile uartFlags UartFlags;
extern volatile unsigned int uiNastawa;

static rtcTimeWDay ascToTime( char * str);
static unsigned char ascToSpeed (char *str);
static void sendTime(rtcTimeWDay * time, char tmp);
static void sendSpeed(unsigned char speed);
static void sendUART(unsigned char length, char* tab);
static void sendFlags(timeFlags * flags);
void uart_checkReceived(void);
void Bt_command(void);

void BT_uart(void){
    uart_checkReceived();
    Bt_command();
}

void __attribute__((interrupt, no_auto_psv)) _U1RXInterrupt(void) {
    char data, tmp;
    data = U1RXREG;
    tmp = (rx_head + 1) & RX_MAX_POS;
    if (tmp != rx_position){
        rx_head = tmp;
        rx_buffer[rx_head] = data;
    }
    if (data == '\n') UartFlags.received_data = TRUE;
    IFS0bits.U1RXIF = 0;
}
void __attribute__((interrupt, no_auto_psv)) _U1TXInterrupt(void) {
    if (tx_position != tx_head){
        tx_position = (tx_position + 1) & TX_MAX_POS;
        U1TXREG = tx_buffer[tx_position];
    }
    IFS0bits.U1TXIF = 0;
}

void uart_checkReceived(void) {
    if (UartFlags.received_data) {
        unsigned char counter_a,  i;
        if (!UartFlags.rewrite) {
            for (i = 0; rx_position != rx_head; ++i) {
                rx_position = (rx_position + 1) & RX_MAX_POS;
                BT_command[i] = rx_buffer[rx_position];
            }
            for (counter_a = 0; counter_a < 16; ++counter_a) {
                if (BT_command[counter_a] == 'S') {
                    if (counter_a > 5) break;
                    if (BT_command[++counter_a] == 'D') {
                        for (i = 0; i < 3; ++i) {
                            if (BT_command[counter_a + i] > '9' && BT_command[counter_a + i] < '0') break;
                        }
                        if (i == 3) UartFlags.rewrite = 1;
                        break;
                    } else if (BT_command[counter_a] == 'F') {
                        if (BT_command[++counter_a] != 'P') break;
                        else if (BT_command[++counter_a] != '0' && BT_command[counter_a] != '1') break;
                        else if (BT_command[++counter_a] != 'W') break;
                        else if (BT_command[++counter_a] != '0' && BT_command[counter_a] != '1') break;
                        else {
                            UartFlags.rewrite = 1;
                            break;
                        }
                    } else if (BT_command[counter_a] < 88 && BT_command[counter_a] > 79) {
                        for (i = 0; i < 7; ++i) {
                            if (BT_command[counter_a + i] > '9' && BT_command[counter_a + i] < '0') break;
                        }
                        if (i == 7) UartFlags.rewrite = 1;
                        break;
                    }
                }
                else if (BT_command[counter_a] == 'G') {
                    if (BT_command[++counter_a] < 88 && BT_command[counter_a] > 67) {
                        UartFlags.rewrite = 1;
                        break;
                    }
                }
            }
        }
        UartFlags.received_data = 0;
    }
}
void Bt_command(void) {
    if (UartFlags.rewrite) {
        unsigned char tmp = 0;
        switch (BT_command[tmp++]) {
            case 'S':
                switch (BT_command[tmp++]) {
                    case 'T':
                        RtcTimeWDay = ascToTime(&BT_command[tmp]);
                        break;
                    case 'W':
                        wupTime = ascToTime(&BT_command[tmp]);
                        TimeFlags.wakeup = 1;
                        break;
                    case 'P':
                        sleepTime = ascToTime(&BT_command[tmp]);
                        TimeFlags.gosleep = 1;
                            break;
                    case 'D':
                        uiNastawa = ascToSpeed(&BT_command[tmp]);
                        break;
                    case 'F':
                        if (BT_command[tmp++] == 'P') TimeFlags.gosleep = BT_command[tmp++] - '0';
                        if (BT_command[tmp++] == 'W') TimeFlags.wakeup = BT_command[tmp] - '0';
                        break;
                    default:
                        break;
                } break;
            case 'G':
                switch (BT_command[tmp]){
                    case 'D':
                        sendSpeed(uiNastawa);
                        break;
                    case 'F':
                        sendFlags(&TimeFlags);
                        break;
                    case 'P':
                        sendTime(&sleepTime,'P');
                        break;
                    case 'T':
                        sendTime(&RtcTimeWDay, 'T');
                        break;
                    case 'W':
                        sendTime(&wupTime,'W');
                        break;
                    default:
                        break;
                }
            default:
                break;
        }
        UartFlags.rewrite = 0;
    }
}
static unsigned int ascToDec (unsigned int x, char * str){
    unsigned int tmp, i;
    for (i = 0, tmp = 0; i < x; ++i){
        tmp *= 10;
        tmp += str[i] - '0';
    }
    return tmp;
}
static char * decToAsc (unsigned int dec, unsigned char length, char *buff){
    unsigned char i;
    length -=1;
    for (i = 0; i <= length; ++i){
        buff[length - i] = dec%10 + '0';
        dec /= 10;
    }
    buff[i] = 0;
    return buff;
}
static void sendUART(unsigned char length, char* tab){
    unsigned char i, tmp, good = 1;
    tmp = tx_head;
    for (i = 0; i<(length - 1); ++i){  
        tmp = (tmp + 1) & TX_MAX_POS;
        if (tmp == tx_position){
            good = 0;
            break;
        }
    }
    if (good){
        for (i = 0; i < length; ++i){
            tx_head = (tx_head + 1) & TX_MAX_POS;
            tx_buffer[tx_head] = tab[i];
        }
        if (U1STAbits.TRMT){
            tx_position = (tx_position + 1) & TX_MAX_POS;
            U1TXREG = tx_buffer[tx_position];
        }
    }
}
static rtcTimeWDay ascToTime( char * str){
    rtcTimeWDay tmpTime;
    tmpTime.hour = ascToDec(2,str);
    tmpTime.minute = ascToDec(2,&str[2]);
    tmpTime.second = ascToDec(2,&str[4]);
    tmpTime.wday = ascToDec(1,&str[6]);
    return tmpTime;
}
static unsigned char ascToSpeed (char *str){
    return ascToDec(3,str);
}
static void sendSpeed(unsigned char speed){
    char buff[10];
    buff[0] = 'D';
    decToAsc(speed,3,&buff[1]);
    buff[4] = '\n';
    buff[5] = '\r';
    sendUART(6,buff);
}
static void sendTime(rtcTimeWDay * time, char tmp){
    char buff[12];
    buff[0] = tmp;
    decToAsc(time->hour,2,&buff[1]);
    decToAsc(time->minute,2,&buff[3]);
    decToAsc(time->second,2,&buff[5]);
    decToAsc(time->wday,1,&buff[7]);
    buff[8] = '\n';
    buff[9] = '\r';
    sendUART(10,buff);
}
static void sendFlags(timeFlags * flags){
    char buff[8];
    buff[0] = 'F';
    buff[1] = 'P';
    buff[2] = '0' + flags->gosleep;
    buff[3] = 'W';
    buff[4] = '0' + flags->wakeup;
    buff[5] = '\n';
    buff[6] = '\r';
    sendUART(7,buff);
}


