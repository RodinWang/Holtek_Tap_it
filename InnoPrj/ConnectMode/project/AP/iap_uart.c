//-----------------------------------------------------------------------------
#include "iap_uart.h"
#include "buffer.h"

//-----------------------------------------------------------------------------
//u8 IAP_UART_RxBufferMem[IAP_UART_RX_BUFFER_SIZE];
//Buffer_TypeDef IAP_UART_RxBuffer;

//-----------------------------------------------------------------------------
void iap_uart_Configuration(void)
{
  USART_InitTypeDef USART_InitStruct;
  CKCU_PeripClockConfig_TypeDef CKCUClock = {{0}};
  CKCUClock.Bit.IAP_UART_NAME         = 1;
  CKCUClock.Bit.AFIO                  = 1;
  CKCUClock.Bit.IAP_UART_RX_GPIO_CLK  = 1;
  CKCU_PeripClockConfig(CKCUClock, ENABLE);

  AFIO_GPxConfig(IAP_UART_TX_GPIO_PORT_ID, AFIO_PIN_0 << IAP_UART_TX_GPIO_PIN_ID, AFIO_FUN_USART_UART);
  AFIO_GPxConfig(IAP_UART_RX_GPIO_PORT_ID, AFIO_PIN_0 << IAP_UART_RX_GPIO_PIN_ID, AFIO_FUN_USART_UART);
  
  GPIO_PullResistorConfig(IAP_UART_RX_GPIO_PORT, IAP_UART_RX_GPIO_PIN, GPIO_PR_UP);

  Buffer_Init(&IAP_UART_RxBuffer, IAP_UART_RxBufferMem, IAP_UART_RX_BUFFER_SIZE);   
  USART_DeInit(IAP_UART);

  USART_InitStruct.USART_BaudRate = 115200;
  USART_InitStruct.USART_WordLength = USART_WORDLENGTH_8B;
  USART_InitStruct.USART_StopBits = USART_STOPBITS_1;
  USART_InitStruct.USART_Parity = USART_PARITY_NO;
  USART_InitStruct.USART_Mode = USART_MODE_NORMAL;
  USART_Init(IAP_UART, &USART_InitStruct);
  USART_RxCmd(IAP_UART, ENABLE);
  USART_TxCmd(IAP_UART, ENABLE);

  USART_IntConfig(IAP_UART, USART_INT_RXDR, ENABLE);

  NVIC_EnableIRQ(IAP_UART_IRQn);
}


//-----------------------------------------------------------------------------
ErrStatus IAP_UART_PutChar(u8 c)
{
  u16 timeout = 10000;
  
  IAP_UART->DR = c;
  while (timeout--) 
  {
    if(((IAP_UART->SR & USART_FLAG_TXC) != 0))
    {
      return SUCCESS;
    }
  }
  
  return ERROR;
}
  
//-----------------------------------------------------------------------------
u16 IAP_UART_Send(u8 *puData, u16 length)
{
  u16 cnt;
  
  for(cnt = 0 ; cnt < length ; cnt++)
  {
    if(IAP_UART_PutChar(puData[cnt]) == ERROR)
    {
      break;
    }
  }
  
  return cnt;
}

//-----------------------------------------------------------------------------
static void delayXuS(u32 ct)
{
  while(ct--)
  {
    
  }
}
  

//-----------------------------------------------------------------------------
u16 IAP_UART_Get(u8 *puData, u16 length, u16 timeout_ms)
{
  u16 cnt = 0;
  
  do
  {
    cnt += Buffer_Read(&IAP_UART_RxBuffer, puData, length-cnt);
    if(cnt >= length)
      break;
    
    delayXuS(1000);
  } while(timeout_ms--);
  
  return cnt;
}

//-----------------------------------------------------------------------------
void IAP_UART_IRQHandler(void)
{
  /*--------------------------------------------------------------------------------------------------------*/
  /* Move data from USART FIFO to buffer when Rx Data Received                                              */
  /*--------------------------------------------------------------------------------------------------------*/
  if ((IAP_UART->IER & USART_INT_RXDR) && (IAP_UART->SR & USART_FLAG_RXDR))
  {
    #if (DBG_EN != 0)
    if (Buffer_IsFull(&IAP_UART_RxBuffer))
    {
      /*----------------------------------------------------------------------------------------------------*/
      /* Should not reach here! It means the buffer for USART is full.                                      */
      /*----------------------------------------------------------------------------------------------------*/
      while (1);
    }
    #endif
    Buffer_WriteByte(&IAP_UART_RxBuffer, (IAP_UART->DR & (u32)0x000000FF));
  }
}


//-----------------------------------------------------------------------------





