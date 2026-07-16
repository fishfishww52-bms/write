#ifndef __uart_h
#define __uart_h

#include "gpm32f13xx_gpio.h"
#include "gpm32f13xx_uart.h"
#include "type.h"


#define		Rs485Tx_En()			GPIO0->P10 = 1
#define		Rs485Rx_En()			GPIO0->P10 = 0
#define		Rs485Txing				GPIO0->ODR_b.P10

extern u8 Uart1TxArray[],Uart0TxArray[];
extern void uart_receive(void);

#endif