//-----------------------------------------------------------------------------
#ifndef __IAP_UART_H
#define __IAP_UART_H


//-----------------------------------------------------------------------------
#include "ht32.h"

//-----------------------------------------------------------------------------
#define IAP_UART_TX_BUFFER_SIZE    (68)                         /* Tx Buffer size                */
#define IAP_UART_RX_BUFFER_SIZE    (512)                        /* Rx Buffer size                */

/* I/O */
#define IAP_UART_TX_GPIO_PORT_NAME  A
#define IAP_UART_TX_GPIO_PIN_ID     4
#define IAP_UART_RX_GPIO_PORT_NAME  A
#define IAP_UART_RX_GPIO_PIN_ID     5

#define IAP_UART_NAME               USART0
#define IAP_UART_IRQn               STRCAT2(IAP_UART_NAME,  _IRQn)
#define IAP_UART                    STRCAT2(HT_,            IAP_UART_NAME)
#define IAP_UART_IRQHandler         STRCAT2(IAP_UART_NAME,  _IRQHandler)

#define IAP_UART_TX_GPIO_PORT_ID    STRCAT2(GPIO_P,         IAP_UART_TX_GPIO_PORT_NAME)
#define IAP_UART_RX_GPIO_PORT_ID    STRCAT2(GPIO_P,         IAP_UART_RX_GPIO_PORT_NAME)

#define IAP_UART_RX_GPIO_CLK        STRCAT2(P,              IAP_UART_RX_GPIO_PORT_NAME)
#define IAP_UART_RX_GPIO_PORT       STRCAT2(HT_GPIO,        IAP_UART_RX_GPIO_PORT_NAME)
#define IAP_UART_RX_GPIO_PIN        STRCAT2(GPIO_PIN_,      IAP_UART_RX_GPIO_PIN_ID)

//-----------------------------------------------------------------------------
void iap_uart_Configuration(void);
u16 IAP_UART_Send(u8 *puData, u16 length);
u16 IAP_UART_Get(u8 *puData, u16 length, u16 timeout_ms);

//-----------------------------------------------------------------------------
#endif



