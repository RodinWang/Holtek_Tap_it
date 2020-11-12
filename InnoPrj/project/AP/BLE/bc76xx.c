/*----------------------------------------------------------------------------
 * Name     : BC76xx.c
 * Purpose  : 
 * Note(s)  : 
 *----------------------------------------------------------------------------
 * This file is part of the uVision/ARM development tools.
 * This software may only be used under the terms of a valid, current,
 * end user licence from KEIL for a compatible version of KEIL software
 * development tools. Nothing else gives you the right to use this software.
 *
 * This software is supplied "AS IS" without warranties of any kind.
 *
 * Copyright (c) 2012 Keil - An ARM Company. All rights reserved.
 *----------------------------------------------------------------------------*/

#include "ht32f5xxxx_usart.h"
#include "string.h"
#include "bc76xx.h"
#include "urbus.h"
#include "bleprocess.h"
#include "bleuser.h"

__align(4) u8 transmit_data[PACKET_MAX_SIZE];
u8 transmit_data_len;
timer_operate_t timer_operate;
vu32 BLE_delay;
/*****************************************************************************************************
* @brief  delay n*10us
* @retval None
*******************************************************************************************************/
void delay_10us(u32 n)
{
  BLE_delay = n*10/BFTM_TIME_BASE;
  while(BLE_delay);			
}

/*****************************************************************************************************
* @brief  delay nms
* @retval None
*******************************************************************************************************/
void delay_ms(u16 n)
{
  BLE_delay = n*1000/BFTM_TIME_BASE;
  while(BLE_delay);			
}

void BC76XX_Init(void)
{
  CKCU_PeripClockConfig_TypeDef cken;
  cken.Bit.SPI0 = 1;
  CKCU_PeripClockConfig(cken,ENABLE);  
  
  BC76xx_interface_configure();
  BLE_param_configure();
  BLE_enter_power_save();
  delay_ms(100);
  BLE_enter_power_up();
  Buffer_Init(&BLE_rx_index, BLE_rx_buff, BLE_RX_Size + 1);
}

/*----------------------------------------------------------------------------*/
/*	 BC76XX interface Configure                                                	*/
/*----------------------------------------------------------------------------*/
void BC76xx_interface_configure(void)
{
#if (_BC76XX_SPI_UART_ == USART_MODE)
#else
   /* configure PDN/INT/WAKEUP/RST as GPIO  */
   AFIO_GPxConfig(GPIO_PA, BC76XX_PDN | BC76XX_EXTINT | BC76XX_RES, AFIO_FUN_DEFAULT);
   AFIO_GPxConfig(GPIO_PD, BC76XX_WAKEUP | BC76XX_STATE, AFIO_FUN_DEFAULT);
   
   GPIO_DirectionConfig(HT_GPIOA, BC76XX_PDN | BC76XX_RES, GPIO_DIR_OUT);
   GPIO_DirectionConfig(HT_GPIOA, BC76XX_EXTINT, GPIO_DIR_IN);
   GPIO_InputConfig(HT_GPIOA, BC76XX_EXTINT, ENABLE);
   GPIO_PullResistorConfig(HT_GPIOA, BC76XX_EXTINT, GPIO_PR_UP);
  
   GPIO_DirectionConfig(HT_GPIOD, BC76XX_STATE, GPIO_DIR_IN);
   GPIO_InputConfig(HT_GPIOD, BC76XX_STATE, ENABLE);
   GPIO_PullResistorConfig(HT_GPIOD, BC76XX_STATE, GPIO_PR_UP);  
  
   GPIO_DirectionConfig(HT_GPIOD, BC76XX_WAKEUP, GPIO_DIR_OUT);
   /* configure SCK/MOSI/MISO/CSN as SPI  */
   GPIO_PullResistorConfig(HT_GPIOB, BC76XX_MISO, GPIO_PR_UP);
   AFIO_GPxConfig(GPIO_PB, BC76XX_SCK | BC76XX_MOSI | BC76XX_MISO | BC76XX_CSN , AFIO_FUN_SPI);	
                     
   /* SPI Configuration */	
   BC76XX_SPI->CR1 =	SPI_MASTER |  SPI_DATALENGTH_8 |
                  SPI_SEL_SOFTWARE | SPI_SELPOLARITY_LOW |
                  SPI_FIRSTBIT_MSB | 0x500;			/* CPOL=1, CPHA=1 */
   /*-- Enable or Disable the specified SPI interrupt --*/
   BC76XX_SPI->IER = 0x00000000;
   /*-- SPIx FIFO Control Register Configuration --*/
   BC76XX_SPI->FCR = SPI_FIFO_DISABLE;
   /*-- SPIx Clock Prescaler Register Configuration -*/
   BC76XX_SPI->CPR = ((CKCU_GetPeripFrequency(CKCU_PCLK_SPI0)/_BC76XX_SPI_SPEED_)-1)/2; /* fSCK=fPCLK/(2*(CP + 1)) */
   /* Enable or Disable the SEL output for the specified SPI peripheral.*/
   BC76XX_SPI->CR0 = 0x00000008;
   /* Enable the selected SPI peripheral */
   BC76XX_SPI->CR0 |= 0x00000001;
#endif	
	
}
/*----------------------------------------------------------------------------*/
/*	 BC76xx power mode                                                         */
/*----------------------------------------------------------------------------*/
void BC76xx_power_mode(PowerStatus Mode)
{
	if(Mode == POWER_UP)
	{
		BC76XX_PDN_HIGH;		
		delay_10us(15);
		BC76XX_RES_HIGH;
	}
	else
	{
		BC76XX_RES_LOW;
		delay_10us(15);
		BC76XX_PDN_LOW;
	}
	delay_10us(15);			
}
/*----------------------------------------------------------------------------*/
/*	 BC76xx wakeup mode                                                        */
/*----------------------------------------------------------------------------*/
void BC76xx_wakeup(IOStatus State)
{
   (State == HIGH) ? BC76XX_WAKEUP_HIGH:BC76XX_WAKEUP_LOW;
}
/*----------------------------------------------------------------------------*/
/*	 BC76xx wakeup mode                                                        */
/*----------------------------------------------------------------------------*/
IOStatus BC76xx_get_wakeup(void)
{
   return(BC76XX_WAKEUP_STS ? HIGH : LOW);
}
/*----------------------------------------------------------------------------*/
/*	 BC76xx state                                                         		*/
/*----------------------------------------------------------------------------*/
IOStatus BC76xx_get_state(void)
{
	return LOW;
}
/*----------------------------------------------------------------------------*/
/*	 BC76xx extint                                                         		*/
/*----------------------------------------------------------------------------*/
IOStatus BC76xx_get_extint(void)
{
	return( BC76XX_EXTINT_IN ? HIGH : LOW);
}
/*----------------------------------------------------------------------------*/
/*	 send HCI ctrl command packet                                              */
/*----------------------------------------------------------------------------*/
void BC76xx_send_hci_cmd_pkg(u16 opcode,u8 len,u8 *pbuf)
{     
   ((HCI_COMMAND_PACKAGE *)transmit_data)->pkg_type = HCI_CMD_PKG;   
   ((HCI_COMMAND_PACKAGE *)transmit_data)->opcode = opcode;
   ((HCI_COMMAND_PACKAGE *)transmit_data)->leng = len;   
   if((len != 0) && (pbuf != NULL)) 
      memcpy(((HCI_COMMAND_PACKAGE *)transmit_data)->param,pbuf,len);
   BC76xx_send_packet(transmit_data,NULL);
}
/*----------------------------------------------------------------------------*/
/*	 BC76XX send ctrl command packet                                           */
/*----------------------------------------------------------------------------*/
void BC76xx_send_ctrl_cmd_pkg(u8 cmd,u8 len,u8 *pbuf)
{   
   ((CTRL_PACKAGE *)transmit_data)->type = CTRL_COMD;
   ((CTRL_PACKAGE *)transmit_data)->cmd = cmd;
   ((CTRL_PACKAGE *)transmit_data)->leng = len;
   if((len != 0) && (pbuf != NULL)) 
      memcpy(((CTRL_PACKAGE *)transmit_data)->data,pbuf,len);
   BC76xx_send_packet(transmit_data,NULL);
}
/*----------------------------------------------------------------------------*/
/*	 BC76XX create read info packet                                            */
/*----------------------------------------------------------------------------*/
void BC76xx_send_read_info_pkg(u8 cmd)
{  
   ((READ_INF_PACKAGE *)transmit_data)->type = READ_INFO;
   ((READ_INF_PACKAGE *)transmit_data)->cmd = cmd;
   BC76xx_send_packet(transmit_data,NULL);
}
/*----------------------------------------------------------------------------*/
/*	 BC76XX create write ctrl packet                                           */
/*----------------------------------------------------------------------------*/
void BC76xx_send_data_pkg(void)
{   
  if(transmit_data_len == 0)return;
   ((MESSAGE_PACKAGE *)transmit_data)->type = DATA_PACKET;
   ((MESSAGE_PACKAGE *)transmit_data)->leng = transmit_data_len;
//   if((len != 0) && (pbuf != NULL) ) 
//      memcpy(((MESSAGE_PACKAGE *)transmit_data)->data,pbuf,len);
   BC76xx_send_packet(transmit_data,NULL);
  transmit_data_len = 0;
}
/*----------------------------------------------------------------------------*/
/*	 BC76XX create read phy packet                                             */
/*----------------------------------------------------------------------------*/
void BC76xx_send_read_phy_pkg(u32 adr,u8 len)
{
   ((READ_PHY_PACKAGE *)transmit_data)->type = READ_PHY;    /* read phy command */
   ((READ_PHY_PACKAGE *)transmit_data)->leng = len;         /* read phy length */
   ((READ_PHY_PACKAGE *)transmit_data)->addr = adr;        /* read phy start address */   
   BC76xx_send_packet(transmit_data,NULL);
}
/*----------------------------------------------------------------------------*/
/*	 BC76XX create write phy packet                                            */
/*----------------------------------------------------------------------------*/
void BC76xx_send_write_phy_pkg(u32 adr,u8 len,u8 *pbuf,bool dir)
{
   ((PHY_PACKAGE *)transmit_data)->type = WRITE_PHY;  /* write phy command */
   ((PHY_PACKAGE *)transmit_data)->leng = len;        /* write phy length */
   ((PHY_PACKAGE *)transmit_data)->reserve = 0x0000;  /* reserve */
   ((PHY_PACKAGE *)transmit_data)->addr = adr;        /* write phy start address */
   if(!dir)
   {
      if(pbuf != NULL) memcpy((u8 *)((PHY_PACKAGE *)transmit_data)->data,pbuf,len*4);
      BC76xx_send_packet(transmit_data,NULL);
   }
   else
   {
      BC76xx_send_packet(transmit_data,pbuf);
   }
}
/*----------------------------------------------------------------------------*/
/*	 BC76XX send ACI packet                                                    */
/*----------------------------------------------------------------------------*/
void BC76xx_send_packet(u8 *pkb,u8 *dkb)
{
   u16   p1len=0,p2len=0;
   
   switch(*pkb)
   {
      case HCI_CMD_PKG :
         p1len = 4;                 /* type+opcode+length = 4byte */
         p2len = ((HCI_COMMAND_PACKAGE *)pkb)->leng;      /* data length */
         if(dkb == NULL) p1len += p2len;
         break;
      case CTRL_COMD :              /* 0x25 */
         p1len = 3;                 /* type,CC8bit,len = 3byte */ 
         p2len = ((CTRL_PACKAGE *)pkb)->leng;      /* data length */
         if(dkb == NULL ) p1len += p2len;
         break;
      case READ_INFO :              /* 0x20 */
         p1len = sizeof(READ_INF_PACKAGE); /* packet length = type + CC8bit */
         dkb = NULL;
         break;
      case DATA_PACKET :            /* 0x22  */
         p1len = 2;                 /* type,len = 2byte */
         p2len = ((MESSAGE_PACKAGE *)pkb)->leng;      /* data length */
         if(dkb == NULL) p1len += p2len;
         break;
      case WRITE_PHY :              /* 0x55  */
         p1len = 8;                 /* packet lenght = type + len + address */
         p2len = ((PHY_PACKAGE *)pkb)->leng;
         p2len = p2len*4;           /* data length(32bit) */
         if(dkb == NULL) p1len += p2len;
         break;
      case READ_PHY :               /* 0x56  */
         p1len = sizeof(READ_PHY_PACKAGE); /* packet lenght = type + len + address */
         dkb = NULL;
         break;
   }
	BC76xx_multibyte_write(pkb,p1len);
	if(dkb != NULL) BC76xx_multibyte_write(dkb,p2len);
}
/*----------------------------------------------------------------------------*/
/*	 BC76XX multibyte write                                                    */
/*----------------------------------------------------------------------------*/
void BC76xx_multibyte_write(u8 *pbuf,u16 length)
{
#if (_BC76XX_SPI_UART_ == USART_MODE)
   while(length)   
   {
      BC76XX_USART->DR = *pbuf++;
      while(!(BC76XX_USART->SR & USART_FLAG_TXDE));
      length--;
   }
   while(!(BC76XX_USART->SR & USART_FLAG_TXC));   	
#else
	while(length)
	{
		if(length > 32)
		{
			BC76xxSPI_write_fifo(pbuf,32);
			delay_10us(7);
			pbuf += 32;
			length -= 32;
		}
		else
		{
			BC76xxSPI_write_fifo(pbuf,length);
			length = 0;
		}
	}
#endif	
}
/*----------------------------------------------------------------------------*/
/*	 BC76XX parser ACI packet                                                  */
/*----------------------------------------------------------------------------*/
bool BC76xx_rcv_packet_parser(PARSER_OPERATE *op)
{
   u8    rdata;
   bool  pkg_valid;
	
   pkg_valid = FALSE;	
   while(op->rcv_head != op->rcv_tail)
   {
		rdata = op->rcv_buffer[op->rcv_tail++];
		op->rcv_tail &= 0x0F;
		switch(op->pkg_step)
      {
			case RCV_ACI_CMD :    /* receive CC8Bit data */
				op->pkg_buffer[op->pkg_index++] = rdata;
            op->pkg_step = RCV_ACI_LENG;
            break;
         case RCV_ACI_LENG :
				op->pkg_buffer[op->pkg_index++] = rdata;
            op->pkg_leng = rdata;
            op->pkg_step = RCV_ACI_DATA;
            break;
			case RCV_ACI_LENG32 :
				op->pkg_buffer[op->pkg_index++] = rdata;
				op->pkg_leng = (rdata*4) + 6;
            op->pkg_step = RCV_ACI_DATA;
            break;
			case RCV_ACI_DATA :
         case RCV_EVENT_PARAM :
				op->pkg_buffer[op->pkg_index++] = rdata;
				if(--op->pkg_leng == 0)
            {
					pkg_valid = TRUE;
               op->pkg_step = RCV_PKG_TYPE;
            }
            break;
			case RCV_HCI_EVENT :
				op->pkg_buffer[op->pkg_index++] = rdata;
            op->pkg_step = RCV_EVENT_LENG;
            break;
			case RCV_EVENT_LENG :
				op->pkg_buffer[op->pkg_index++] = rdata;				
				if(rdata == 0)
            {
					pkg_valid = TRUE;
               op->pkg_step = RCV_PKG_TYPE;
				}
            else
            {
					op->pkg_leng = rdata;
               op->pkg_step = RCV_EVENT_PARAM;
            }
            break;
			case RCV_PKG_TYPE :
				op->pkg_index = 0;
            switch(rdata)
				{
					case HCI_EVENT_PKG :             	/* 0x04 */
                  op->pkg_buffer[op->pkg_index++] = HCI_EVENT_PKG;
						op->pkg_leng = 2;          		/* event code=1byte + data length=1byte */					
                  op->pkg_step = RCV_HCI_EVENT; 	/* to receive data step */
                  break;
					case CTRL_RETURN :               	/* 0x26  */
                  op->pkg_buffer[op->pkg_index++] = CTRL_RETURN;
						op->pkg_leng = 2;					
                  op->pkg_step = RCV_ACI_DATA;		/* to receive data step */
                  break;
					case INFO_RETURN :               	/* 0x21  */
						op->pkg_buffer[op->pkg_index++] = INFO_RETURN;
                  op->pkg_step = RCV_ACI_CMD;   	/* to receive CC8Bit step */
                  break;
					case DATA_PACKET :               	/* 0x22  */
						op->pkg_buffer[op->pkg_index++] = DATA_PACKET;
                  op->pkg_step = RCV_ACI_LENG;  	/* to receive length step */
                  break;
					case READ_PHY_RETURN :           	/* 0x57  */
						op->pkg_buffer[op->pkg_index++] = READ_PHY_RETURN;
                  op->pkg_leng = 6;                        
                  op->pkg_step = RCV_ACI_LENG32;	/* to receive phy length step */
                  break;
					case COMD_PACKET :               	/* 0x27  */
						op->pkg_buffer[op->pkg_index++] = COMD_PACKET;
                  op->pkg_step = RCV_ACI_CMD;   	/* to receive CC8Bit step */
                  break;
				}
            break;
		}
		if(pkg_valid) break;
   }
   return(pkg_valid);
}
/*----------------------------------------------------------------------------*/
/*	 BC76XX SPI read register                                                  */
/*----------------------------------------------------------------------------*/
u16 BC76xxSPI_read_register(u8 reg)
{
   u8 cmd;
   u16 value;
   
   cmd = ((reg & 0x0F) << 1) | 0x01;
   /* enable CS */
   BC76XX_SPI->CR0 |= SPI_SEL_ACTIVE;						/* SPI CS active */
   BC76XX_SPI->DR = cmd;
   while(!(BC76XX_SPI->SR & SPI_FLAG_TXE));           /* wait SPI TX finish */
   BC76XX_SPI->DR = 0xFF;
   while(!(BC76XX_SPI->SR & SPI_FLAG_RXBNE));   
   value = BC76XX_SPI->DR;
   while(!(BC76XX_SPI->SR & SPI_FLAG_TXE));
   BC76XX_SPI->DR = 0xFF;
   while(!(BC76XX_SPI->SR & SPI_FLAG_RXBNE));   
   value = BC76XX_SPI->DR;
   while(!(BC76XX_SPI->SR & SPI_FLAG_TXE));
   BC76XX_SPI->CR0 &= ~SPI_SEL_ACTIVE;   
   while(!(BC76XX_SPI->SR & SPI_FLAG_RXBNE));   
   cmd = BC76XX_SPI->DR;
   value = (value << 8) | cmd;
  
//  printf("RD_REG:0x%02X,0x%04X\r\n", reg, value);
   return(value);
}
/*----------------------------------------------------------------------------*/
/*	 BC76XX SPI write register                                                 */
/*----------------------------------------------------------------------------*/
void BC76xxSPI_write_register(u8 reg,u16 value)
{
   u8 cmd;
   
   cmd = ((reg & 0x0F) << 1) | 0x21;
   /* enable CS */
   BC76XX_SPI->CR0 |= SPI_SEL_ACTIVE;
   BC76XX_SPI->DR = cmd;
   while(!(BC76XX_SPI->SR & SPI_FLAG_TXE));
   BC76XX_SPI->DR = (value >> 8) & 0xFF;
   while(!(BC76XX_SPI->SR & SPI_FLAG_RXBNE));
   cmd = BC76XX_SPI->DR;
   while(!(BC76XX_SPI->SR & SPI_FLAG_TXE));
   BC76XX_SPI->DR = (value & 0xFF);
   while(!(BC76XX_SPI->SR & SPI_FLAG_RXBNE));   
   cmd = BC76XX_SPI->DR;
   while(!(BC76XX_SPI->SR & SPI_FLAG_TXE));
   BC76XX_SPI->CR0 &= ~SPI_SEL_ACTIVE;   
   while(!(BC76XX_SPI->SR & SPI_FLAG_RXBNE));   
   cmd = BC76XX_SPI->DR;
  
//  printf("WR_REG:0x%02X,0x%04X\r\n", reg, value);
}
/*----------------------------------------------------------------------------*/
/*	 BC76XX SPI read FIFO                                                      */
/*----------------------------------------------------------------------------*/
void BC76xxSPI_read_fifo(u8 *pbuf,u8 length)
{
   u8 cmd;
  
//  u8 i;
//  u8 *tbuf = pbuf;
//  u8 tlen = length;

   /* enable CS */
   BC76XX_SPI->CR0 |= SPI_SEL_ACTIVE;
   cmd = (length & 0x1F) | 0x60;
   BC76XX_SPI->DR = cmd;
   while(!(BC76XX_SPI->SR & SPI_FLAG_RXBNE));
   cmd = BC76XX_SPI->DR;
   while(length--)
   {
      BC76XX_SPI->DR = 0xFF;
      while(!(BC76XX_SPI->SR & SPI_FLAG_RXBNE));   
      *pbuf++ = BC76XX_SPI->DR;      
   }
   BC76XX_SPI->CR0 &= ~SPI_SEL_ACTIVE;
   
//  printf("RD_FIFO:");
//  for(i = 0;i < tlen;i++)
//  {
//    printf("0x%02X ", tbuf[i]);
//  }
//  printf("\r\n");
}
/*----------------------------------------------------------------------------*/
/*	 BC76XX SPI write FIFO	                                                   */
/*----------------------------------------------------------------------------*/
void BC76xxSPI_write_fifo(u8 *pbuf,u8 length)
{
   u8 cmd;
  
//  u8 i;
//  printf("WR_FIFO:");
//  for(i = 0;i < length;i++)
//  {
//    printf("0x%02X ", pbuf[i]);
//  }
//  printf("\r\n");
  
   /* enable CS */   
   BC76XX_SPI->CR0 |= SPI_SEL_ACTIVE;                     
   cmd = (length & 0x1F) | 0xA0;
   BC76XX_SPI->DR = cmd;
   while(length--)
   {
      while(!(BC76XX_SPI->SR & SPI_FLAG_TXE));
      BC76XX_SPI->DR = *pbuf++;
   }
   cmd = BC76XX_SPI->DR;
   while(!(BC76XX_SPI->SR & SPI_FLAG_TXE));   
   while(!(BC76XX_SPI->SR & SPI_FLAG_RXBNE));
   BC76XX_SPI->CR0 &= ~SPI_SEL_ACTIVE;                           
   while(BC76XX_SPI->SR & SPI_FLAG_RXBNE) cmd = BC76XX_SPI->DR;
}

