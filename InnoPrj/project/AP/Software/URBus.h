#ifndef __URBUS_H
#define __URBUS_H

#include "ht32.h"
#include "buffer.h"

#define Printf_Debug                  (1)

/* high speed UART bus configuration */
#define HS_BUS_CLK(CK)                (CK.Bit.USART0)
#define HS_BUS_PORT                   (HT_USART0)
#define HS_BUS_IRQn                   (USART0_IRQn)
#define HS_BUS_IRQHandler             (USART0_IRQHandler)

#define HS_BUS_TX_GPIO_ID             (GPIO_PB)
#define HS_BUS_TX_AFIO_PIN            (AFIO_PIN_0)
#define HS_BUS_TX_AFIO_MODE           (AFIO_FUN_USART_UART)
#define HS_BUS_TX_PORT                (HT_GPIOB)
#define HS_BUS_TX_GPIO_PIN            (GPIO_PIN_0)

#define HS_BUS_RX_GPIO_ID             (GPIO_PB)
#define HS_BUS_RX_AFIO_PIN            (AFIO_PIN_1)
#define HS_BUS_RX_AFIO_MODE           (AFIO_FUN_USART_UART)
#define HS_BUS_RX_PORT                (HT_GPIOB)
#define HS_BUS_RX_GPIO_PIN            (GPIO_PIN_1)

/* low speed UART bus configuration */
#define LS_BUS_CLK(CK)                (CK.Bit.UART0)
#define LS_BUS_PORT                   (HT_UART0)
#define LS_BUS_IRQn                   (UART0_IRQn)
#define LS_BUS_IRQHandler             (UART0_IRQHandler)

#define LS_BUS_TX_GPIO_ID             (GPIO_PC)
#define LS_BUS_TX_AFIO_PIN            (AFIO_PIN_4)
#define LS_BUS_TX_AFIO_MODE           (AFIO_FUN_USART_UART)
#define LS_BUS_TX_PORT                (HT_GPIOC)
#define LS_BUS_TX_GPIO_PIN            (GPIO_PIN_4)

#define LS_BUS_RX_GPIO_ID             (GPIO_PC)
#define LS_BUS_RX_AFIO_PIN            (AFIO_PIN_5)
#define LS_BUS_RX_AFIO_MODE           (AFIO_FUN_USART_UART)
#define LS_BUS_RX_PORT                (HT_GPIOC)
#define LS_BUS_RX_GPIO_PIN            (GPIO_PIN_5)

/* high voltage(same high speed) UART bus configuration */
#define HV_BUS_CLK(CK)                (CK.Bit.USART0)
#define HV_BUS_PORT                   (HT_USART0)
#define HV_BUS_IRQn                   (USART0_IRQn)
#define HV_BUS_IRQHandler             (USART0_IRQHandler)

#define HV_BUS_TX_GPIO_ID             (GPIO_PA)
#define HV_BUS_TX_AFIO_PIN            (AFIO_PIN_2)
#define HV_BUS_TX_AFIO_MODE           (AFIO_FUN_USART_UART)
#define HV_BUS_TX_PORT                (HT_GPIOA)
#define HV_BUS_TX_GPIO_PIN            (GPIO_PIN_2)

#define HV_BUS_RX_GPIO_ID             (GPIO_PA)
#define HV_BUS_RX_AFIO_PIN            (AFIO_PIN_3)
#define HV_BUS_RX_AFIO_MODE           (AFIO_FUN_USART_UART)
#define HV_BUS_RX_PORT                (HT_GPIOA)
#define HV_BUS_RX_GPIO_PIN            (GPIO_PIN_3)

#define LS_BUS_NUM            (0)
#define HS_BUS_NUM            (1)
#define HV_BUS_NUM            (2)

#define CMD_SIZE              (1)
#define MID_SIZE              (1)
#define EID_LEN_SIZE          (1)
#define LEN2_SIZE             (1)
#define INSTRU_PARA_CS_SIZE   (255)
#define STATUS_PARA_CS_SIZE   (255)

#define VCP_RX_BUFFER_SIZE    (CMD_SIZE+MID_SIZE+EID_LEN_SIZE+LEN2_SIZE+INSTRU_PARA_CS_SIZE)
#define BUS_RX_BUFFER_SIZE    (1+MID_SIZE+EID_LEN_SIZE+LEN2_SIZE+STATUS_PARA_CS_SIZE)
#define VCP_TX_BUFFER_SIZE    (BUS_RX_BUFFER_SIZE-1)
#define BUS_TX_BUFFER_SIZE    (VCP_RX_BUFFER_SIZE-CMD_SIZE)

#define BFTM_TIME_BASE        (10)  //us
#define BUS_WAITING_TIMEOUT   (5)  //ms
#define BUS_INTERVAL_TIMEOUT  (2)   //ms

typedef enum
{
  RX_OK=0x00,
  NO_RESPONSE,
  CHECK_ERR,//The three results means the end of the receive process
  WAITING,
  RECEIVING,
  INTERVAL,
}RX_STATUS_ENUM;

//extern u8 VCP_RX_Buffer[VCP_RX_BUFFER_SIZE + BUFFER_ADDITIONAL_SIZE];
//extern Buffer_TypeDef VCP_RX_Index;

void URBus_Init(void);
int LS_Transmit(u16 uid, u8 len, u8 *par);
int HS_Transmit(u16 uid, u8 len, u8 *par);
int HV_Transmit(u16 uid, u8 len, u8 *par);
int LS_Receive(u16* uid, u8* len, u8* par);
int HS_Receive(u16* uid, u8* len, u8* par);
int HV_Receive(u16* uid, u8* len, u8* par);
#endif
