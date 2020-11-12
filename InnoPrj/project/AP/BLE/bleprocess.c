/*----------------------------------------------------------------------------
 * Name     : bleprocess.c
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
#include "ht32f5xxxx_flash.h" 
#include "string.h"
#include "bc76xx.h"
#include "bleprocess.h"
#include "bleuser.h"
#include "bleApp.h"
// <<< Use Configuration Wizard in Context Menu >>>

//	<e> BLE UUID modify Enable
#define	_BLE_UUID_ENABLE_   (0)
//		<o> BLE Service UUID<0X0000-0XFFFF>
#define 	_BLE_SERVICE_UUID_	(0xFF00)
//		<o> BLE Read UUID<0X0000-0XFFFF>
#define 	_BLE_READ_UUID_	(0xFF01)
//		<o.11> BLE Read Property
//			<0=> Notifying
//			<1=> Indicate      
#define 	_BLE_READ_PROPERTY_	(0x0800)
//		<o> BLE Write UUID<0X0000-0XFFFF>
#define 	_BLE_WRITE_UUID_	(0xFF02)
//		<o.9> BLE Write Property
//			<0=> Without Response
//			<1=> Write 					           
#define 	_BLE_WRITE_PROPERTY_	(0x0200)
//	</e>

//	<e> BLE Parameter modify Enable
#define	_BLE_PARAMETER_MODIFY_EN_   (1)
//		<o.3> Sync to EEPROM
//			<0=> No  
//			<1=> Yes  
//		<o.2> Sync to Back EEPROM
//			<0=> No  
//			<1=> Yes  
#define 	_BLE_SYNC_EEPROM_	(0x08)
//	</e>

//  <h> EXTINT/STATUS Active Configuration
//    <o.1> EXTINT Active Configuration
//       <0=> Low Active	
//       <1=> High Active	
//    <o.2> STATUS Active Configuration
//       <0=> Low Active	
//       <1=> High Active	
#define _BLE_IO_ACTIVE_SET_  (0x04)
//  </h>


const u16 _UUID_Table_[] = 
{
	_BLE_SERVICE_UUID_,					/* Service UUID */
	_BLE_READ_UUID_,                 /* Read UUID */
	_BLE_WRITE_UUID_                 /* Write UUID */
};

const u16 _Property_Table_[] = 
{
	CHARACTER_S_UUID,                                                     /* Change Service UUID */
	CHARACTER_R_UUID + CHARACTER_R_PROPERTY + _BLE_READ_PROPERTY_,  /* Change Read UUID & property */
	CHARACTER_W_UUID + CHARACTER_W_PROPERTY + _BLE_WRITE_PROPERTY_  /* Change Write UUID & property */	
};

u8	BLE_BD_Name[16] = { "BCTX" };
uc8	BLE_ADV_Interval[] = { 0xE8,0x03,0x00,0x00 };
uc8	BLE_TX_Power[] = { TX_POWER_LEVEL0 };
uc8	BLE_BD_Address[] = { 0x44,0x55,0x66,0x77,0x88,0x99 };

uc8	BLE_ParameterModifyCmd[] = 
{	
	MAC_NAME,sizeof(BLE_BD_Name)-1,
//	MAC_ADDRESS,sizeof(BLE_BD_Address),
	ADV_INTERVAL,sizeof(BLE_ADV_Interval),
	TX_POWER_CTRL,sizeof(BLE_TX_Power),
};

uc8	*BLE_ParameterModifyValue[]=
{
//	BLE_BD_Name,BLE_BD_Address,BLE_ADV_Interval,BLE_TX_Power
	BLE_BD_Name,BLE_ADV_Interval,BLE_TX_Power	
};

BLE_OPERATE BLEoperate;
bool	BLE_opevent,BLE_eventok;
IOStatus	BLE_extint_sts;
__align(4) u8 BLE_receive_data[PACKET_MAX_SIZE];
u8		BLE_rcv_buffer[16];
/*----------------------------------------------------------------------------*/
/*	 BLE param configure                                              			*/
/*----------------------------------------------------------------------------*/
void BLE_param_configure(void)
{
	BLEoperate.state = _BLE_POWER_DOWN_;
	BLEoperate.connect = FALSE;
	BLEoperate.event_finish = FALSE;
	BLEoperate.rcvop.pkg_step = RCV_PKG_TYPE;
	BLEoperate.rcvop.pkg_buffer = BLE_receive_data;
	BLEoperate.rcvop.rcv_head = 0;
	BLEoperate.rcvop.rcv_tail = 0;
	BLEoperate.rcvop.rcv_buffer = BLE_rcv_buffer;
	BLEoperate.timer_counter = 0;
#if (_BC76XX_SPI_UART_ == USART_MODE)
	USART_IntConfig(BC76XX_USART, USART_INT_RXDR, ENABLE);
	NVIC_EnableIRQ(BC76XX_USART_IRQn);
#endif	
}
/*----------------------------------------------------------------------------*/
/*	 BLE power up mode                                                        	*/
/*----------------------------------------------------------------------------*/
void BLE_enter_power_up(void)
{
	BLEoperate.state = _BLE_START_;
}
/*----------------------------------------------------------------------------*/
/*	 BLE power save mode                                                     	*/
/*----------------------------------------------------------------------------*/
void BLE_enter_power_save(void)
{
	if(BC76XX_PDN_STS	&& BC76XX_RES_STS)
	{
		BC76xx_wakeup(HIGH);
		while(BC76xx_get_state() != LOW);
	}
	BC76xx_power_mode(POWER_DOWN);
	BC76xx_wakeup(LOW);
	BLEoperate.state = _BLE_POWER_DOWN_;
}
/*----------------------------------------------------------------------------*/
/*	 BLE timer process                                                         */
/*----------------------------------------------------------------------------*/
void BLE_timer_process(void)
{
   if(BLEoperate.timer_counter != 0) BLEoperate.timer_counter--;
}
/*----------------------------------------------------------------------------*/
/*	 generate interval packet                                                  */
/*----------------------------------------------------------------------------*/
void BLE_interval_packet(u16 min_intv,u16 max_intv,CTRL_PACKAGE *buf)
{
	buf->type = CTRL_COMD;
	buf->cmd = CONNECT_INTERVAL2;
	buf->leng = 8;
	buf->data[0] = min_intv;
	buf->data[1] = min_intv >> 8;
	buf->data[2] = max_intv;
	buf->data[3] = max_intv >> 8;
	buf->data[4] = 0;
	buf->data[5] = 0;
	buf->data[6] = (600 & 0xFF);
	buf->data[7] = (600 >> 8);
}
/*----------------------------------------------------------------------------*/
/*	 generate advertising data packet                                          */
/*----------------------------------------------------------------------------*/
void BLE_advertising_packet(u8 *adv,u8 len,CTRL_PACKAGE *buf)
{
	u8 x;
	
	buf->type = CTRL_COMD;
	buf->cmd = ADV_DATA2;
	buf->leng = len+5;
	buf->data[0] = 0x02;				/* length = 2 */
	buf->data[1] = 0x01;				/* type = flag */
	buf->data[2] = 0x06;				/* valu e=LE General Discoverable Mode */
	buf->data[3] = len+1;			/* length */
	buf->data[4] = 0xFF;          /* type=Manufacturer Specific Data */
	x = 0;
	while(len) 
	{ 
		buf->data[x+5] = adv[x];
		x++;
		len--;
	}
}
/*----------------------------------------------------------------------------*/
/*	 parser event packet                                                       */
/*----------------------------------------------------------------------------*/
void BLE_Test(void);
void BLE_event_parser(void)
{  
   extern bool APP_FirstConnect;
   if((((RETURN_PACKAGE *)BLE_receive_data)->type == CTRL_RETURN) && 
		(((RETURN_PACKAGE *)BLE_receive_data)->cmd == 0xFF))
   {
      switch(((RETURN_PACKAGE *)BLE_receive_data)->value)
      {
         case 0x10 :    /* BLE disconnected */
            BLEoperate.connect = FALSE;
            break;
         case 0x11 :    /* BLE connected */
            BLEoperate.connect = TRUE;
            BLE_Test();
            #if USE_BLEAPP_ENABLE
            APP_FirstConnect = TRUE;
            #endif         
            break;
      }
		BLEoperate.event_finish = FALSE;
   }
}
/*----------------------------------------------------------------------------*/
/*	 SPI FIFO process                                                          */
/*----------------------------------------------------------------------------*/
void BLE_SPI_process(void)
{
	u8	leng;
	
	if(BLEoperate.rcvop.rcv_head == BLEoperate.rcvop.rcv_tail)
	{
      leng = (BC76xxSPI_read_register(SPI_REGS_CR4) >> 6) & 0x3F;
      if(leng)
      {
         if(leng > 15) leng = 15;
         BC76xxSPI_read_fifo(BLEoperate.rcvop.rcv_buffer,leng);
			BLEoperate.rcvop.rcv_head = leng;
			BLEoperate.rcvop.rcv_tail = 0;
      }
	}	
}
/*----------------------------------------------------------------------------*/
/*	 BLE state process                                                         */
/*----------------------------------------------------------------------------*/
void BLE_process(void)
{
  static u8 st;
	u32	phy_data;
  
  if(timer_operate.bits.t1ms)
  {
     timer_operate.bits.t1ms = FALSE;
     BLE_timer_process();
  }		
    
	if(BLE_opevent)
	{
#if (_BC76XX_SPI_UART_ == SPI_MODE)			
		if(BC76xx_get_extint() == BLE_extint_sts) BLE_SPI_process();
#endif		
    if(BC76xx_rcv_packet_parser(&BLEoperate.rcvop))
		{
			BLE_eventok = TRUE;
			BLE_opevent = FALSE;
		}
		if(BLE_opevent && (BLEoperate.timer_counter == 0))
		{
			BLE_opevent = FALSE;
			BLEoperate.state = _BLE_PWR_ON_ERROR_;
		}
	}
   
   switch(BLEoperate.state)
   {
		case _BLE_POWER_DOWN_ :
			break;
      case _BLE_START_ :
			BLE_extint_sts = LOW;
         BLEoperate.state = _BLE_POWER_UP_;
      case _BLE_POWER_UP_ :
         /* reset BC76xx */
#if (_BC76XX_SPI_UART_ == USART_MODE)
			BC76XX_SPIUR_LOW;								/* BC76XX to UART mode */
#else		
      //Fixed pull high resistance
			//BC76XX_SPIUR_HIGH;							/* BC76XX to SPI mode */
#endif		
		   BC76xx_power_mode(POWER_UP);
			BC76xx_wakeup(HIGH);
         BLEoperate.timer_counter = PWRUP_DELAY_TIME;     /* delay 500ms */
         BLEoperate.state = _BLE_PWRUP_DELAY_L_;
         break;
      case _BLE_PWRUP_DELAY_L_ :
      case _BLE_PWRUP_DELAY_H_ :
         if(BLEoperate.timer_counter == 0 && BC76xx_get_extint()) 
           BLEoperate.state = _BLE_PWRUP_EXAMINE_;
         break;
		case _BLE_PWRUP_EXAMINE_ :
         /* read PHY_0x602020, if PHY_0x062020 bit[15]=1,[10]=1,[6]=0,BLE power on finish */
         BC76xx_send_read_phy_pkg(PHY_PATCH_STATE_ADR,1);
			BLE_opevent = TRUE;
         BLEoperate.timer_counter = PWRUP_DELAY_TIME;     /* wait time 500ms */
			BLEoperate.state = _BLE_EXAMINE_RESPOND_;			
			break;
		case _BLE_EXAMINE_RESPOND_ :
         if(BLE_eventok)
         {
				if((((PHY_PACKAGE *)BLE_receive_data)->type == READ_PHY_RETURN) && 
				  (((PHY_PACKAGE *)BLE_receive_data)->addr == PHY_PATCH_STATE_ADR)  )
				{
					/* check PHY_0x062020 bit[15]=1,[10]=1,[6]=0 */
					if((((PHY_PACKAGE *)BLE_receive_data)->data[0] & 0x8440) == 0x8400)
					{
						BLEoperate.state = _BLE_IO_ACTIVE_INIT_;
					}
				}
				BLE_eventok = FALSE;
			}
			break;
		case	_BLE_IO_ACTIVE_INIT_ :
			BC76xx_send_read_phy_pkg(PHY_IO_ACTIVE_ADR,1);
			BLE_opevent = TRUE;		
         BLEoperate.timer_counter = EVENT_RETURN_TIME;     /* wait time 5ms */		
			BLEoperate.state = _BLE_IO_ACTIVE_RESPOND_;
			break;
		case	_BLE_IO_ACTIVE_RESPOND_ :
         if(BLE_eventok)
         {
				if((((PHY_PACKAGE *)BLE_receive_data)->type == READ_PHY_RETURN) && 
				  (((PHY_PACKAGE *)BLE_receive_data)->addr == PHY_IO_ACTIVE_ADR)  )
				{					
					phy_data = ((PHY_PACKAGE *)BLE_receive_data)->data[0];
					phy_data &= ~0x06;
					phy_data |= _BLE_IO_ACTIVE_SET_;
					BC76xx_send_write_phy_pkg(PHY_IO_ACTIVE_ADR,1,(u8 *)&phy_data,FALSE);
					if(_BLE_IO_ACTIVE_SET_ & 0x02) BLE_extint_sts = HIGH; 
					st = 0;
					BLEoperate.state = _BLE_UUID_ENABLE_ ? _BLE_UUID_INIT_ : (_BLE_PARAMETER_MODIFY_EN_ ? _BLE_PARAM_MODIFY_1_ : _BLE_PWR_ON_FINISH_);
				}
				BLE_eventok = FALSE;
			}
			break;			
		case _BLE_UUID_INIT_ :
			phy_data = _UUID_Table_[st];
			BC76xx_send_write_phy_pkg(PHY_UUID_CHARGE_ADR,1,(u8 *)&phy_data,FALSE);
			BC76xx_send_read_phy_pkg(PHY_UUID_TRIGGER_ADR,1);
			BLE_opevent = TRUE;		
         BLEoperate.timer_counter = EVENT_RETURN_TIME;     /* wait time 5ms */		
			BLEoperate.state = _BLE_UUID_RESPOND_;
			break;
		case _BLE_UUID_RESPOND_ :
         if(BLE_eventok)
         {
				if((((PHY_PACKAGE *)BLE_receive_data)->type == READ_PHY_RETURN) && 
				  (((PHY_PACKAGE *)BLE_receive_data)->addr == PHY_UUID_TRIGGER_ADR)  )
				{					
					phy_data = ((PHY_PACKAGE *)BLE_receive_data)->data[0];
					phy_data &= 0xFFFFFF00;
					phy_data |= _Property_Table_[st];
					BC76xx_send_write_phy_pkg(PHY_UUID_TRIGGER_ADR,1,(u8 *)&phy_data,FALSE);
					if(++st >= 3) 
					{
						BLEoperate.state = _BLE_PARAMETER_MODIFY_EN_ ? _BLE_PARAM_MODIFY_1_ : _BLE_PWR_ON_FINISH_;
					}
					else
					{
						BLEoperate.state = _BLE_UUID_INIT_;
					}
				}
				BLE_eventok = FALSE;				
			}
			break;
		case	_BLE_PARAM_MODIFY_1_ :
			BC76xx_send_read_info_pkg(BLE_ParameterModifyCmd[0]);
			BLE_opevent = TRUE;		
         BLEoperate.timer_counter = EVENT_RETURN_TIME;     /* wait time 5ms */		
			BLEoperate.state = _BLE_PARAM_MODIFY_2_;		
			break;
		case	_BLE_PARAM_MODIFY_2_ :
         if(BLE_eventok)
         {
				if((((RETURN_INF_PACKAGE *)BLE_receive_data)->type == INFO_RETURN) && 
				  (((RETURN_INF_PACKAGE *)BLE_receive_data)->cmd == BLE_ParameterModifyCmd[0]) )
				{
					BLE_eventok = FALSE;
					if(((RETURN_INF_PACKAGE *)BLE_receive_data)->leng == BLE_ParameterModifyCmd[1])
					{
						for(st=0;st<BLE_ParameterModifyCmd[1];st++)
						{
							if(((RETURN_INF_PACKAGE *)BLE_receive_data)->data[st] != 
								*(BLE_ParameterModifyValue[0]+st) 								)
								BLE_eventok = TRUE;							
						}
					}
					else	BLE_eventok = TRUE;
					st = 0;					
					BLEoperate.state = BLE_eventok ? _BLE_PARAM_MODIFY_3_ : _BLE_PWR_ON_FINISH_;
				}
				BLE_eventok = FALSE;
			}
			break;		
		case	_BLE_PARAM_MODIFY_3_ :
			BC76xx_send_ctrl_cmd_pkg(BLE_ParameterModifyCmd[st],
											 BLE_ParameterModifyCmd[st+1],
											 (u8 *)BLE_ParameterModifyValue[st/2]);
			BLE_opevent = TRUE;		
         BLEoperate.timer_counter = EVENT_RETURN_TIME;     /* wait time 5ms */				
			BLEoperate.state = _BLE_PARAM_MODIFY_4_;
			break;
		case	_BLE_PARAM_MODIFY_4_ :
         if(BLE_eventok)
         {
				if((((RETURN_PACKAGE *)BLE_receive_data)->type == CTRL_RETURN) && 
				  (((RETURN_PACKAGE *)BLE_receive_data)->cmd == BLE_ParameterModifyCmd[st]) &&
				  (((RETURN_PACKAGE *)BLE_receive_data)->value == 00) )
				{
					st += 2;
					if( st == sizeof(BLE_ParameterModifyCmd))
					{
						st = 0;
						BLEoperate.timer_counter = 0;
						BLEoperate.state =  (_BLE_SYNC_EEPROM_ != 0) ? _BLE_PARAM_MODIFY_5_ : _BLE_PWR_ON_FINISH_;
					}
					else
					{
						BLEoperate.state = _BLE_PARAM_MODIFY_3_;
					}
				}
				BLE_eventok = FALSE;
			}
			break;
		case	_BLE_PARAM_MODIFY_5_ :
			if(BLEoperate.timer_counter == 0)
			{
				BC76xx_send_read_phy_pkg(PHY_EEPROM_FUN_ADR,1);
				BLE_opevent = TRUE;		
				BLEoperate.timer_counter = EVENT_RETURN_TIME;     /* wait time 5ms */		
				BLEoperate.state = _BLE_PARAM_MODIFY_6_;
			}
			break;
		case	_BLE_PARAM_MODIFY_6_ :
         if(BLE_eventok)
         {
				if((((PHY_PACKAGE *)BLE_receive_data)->type == READ_PHY_RETURN) && 
				  (((PHY_PACKAGE *)BLE_receive_data)->addr == PHY_EEPROM_FUN_ADR)  )
				{					
					phy_data = ((PHY_PACKAGE *)BLE_receive_data)->data[0];
					if( st == 0x5A)
						phy_data &= ~(_BLE_SYNC_EEPROM_ + 0x01);
					else						
						phy_data |= (_BLE_SYNC_EEPROM_ + 0x01);
					BC76xx_send_write_phy_pkg(PHY_EEPROM_FUN_ADR,1,(u8 *)&phy_data,FALSE);
					if( st == 0x5A)
					{
						BLEoperate.state = _BLE_PWR_ON_FINISH_;
					}
					else
					{
						st = 0x5A;
						BLEoperate.timer_counter = EPWR_DELAY_TIME;     /* wait time 50ms */
						if(_BLE_SYNC_EEPROM_ == 0x0C )	BLEoperate.timer_counter = EPWR_DELAY_TIME*2;     /* wait time 100ms */
						BLEoperate.state = _BLE_PARAM_MODIFY_5_;
					}
				}
				BLE_eventok = FALSE;				
			}			
			break;

		case _BLE_PWR_ON_FINISH_ :
			BLE_opevent = FALSE;
			//BC76xx_wakeup(LOW);
			BLEoperate.state = _BLE_PWR_ON_SUCCESS_;
			break;
      
      case _BLE_ADVERTISE_:      
      case _BLE_PWR_ON_SUCCESS_ :
#if (_BC76XX_SPI_UART_ == USART_MODE)
			if(BC76xx_get_extint() == LOW) BC76xx_wakeup(HIGH);
#else			
			if(BC76xx_get_extint() == BLE_extint_sts)BLE_SPI_process(); 
#endif		
       if(BC76xx_rcv_packet_parser(&BLEoperate.rcvop))
       {
          BLEoperate.event_finish = TRUE;
          BLE_event_parser();
       }
       /****************Advertising event*****************/
       switch(BLEoperate.adver_event.adver_stage)
       {
         case _ADVER_START_:
           BLEoperate.timer_counter = 1000;     /* delay 1s */
           BLEoperate.adver_event.adver_stage = _ADVER_DELAY_;
           break;
         
         case _ADVER_DELAY_:
           if(BLEoperate.timer_counter == 0 && !BC76XX_STATE_IN)
             BLEoperate.adver_event.adver_stage = _ADVER_WORKING_;
           break;
           
         case _ADVER_WORKING_:
           if(BC76XX_STATE_IN && BLEoperate.adver_event.updateAdver_flag == TRUE)
           {
              BC76xx_send_ctrl_cmd_pkg(ADV_DATA2, BLEoperate.adver_event.adver_len, BLEoperate.adver_event.adver_buffer);
              BLEoperate.adver_event.updateAdver_flag = FALSE;
           }
           
           if(BC76XX_STATE_IN && BLEoperate.adver_event.adverCount_flag == FALSE)
           {
              BLEoperate.adver_event.adver_cnt++;
              BLEoperate.adver_event.adverCount_flag = TRUE;
           }
           else if(!BC76XX_STATE_IN && BLEoperate.adver_event.adverCount_flag == TRUE)
           {
              BLEoperate.adver_event.adverCount_flag = FALSE;
           }
           break;
           
         default:
            break;
       }
       /*********************************/
      if(BLEoperate.connect && BLEoperate.event_finish)
      {
        BLEoperate.event_finish = FALSE;
        u8 t_len = ((MESSAGE_PACKAGE *)(BLEoperate.rcvop.pkg_buffer))->leng;
        if((((MESSAGE_PACKAGE *)(BLEoperate.rcvop.pkg_buffer))->type == DATA_PACKET) && t_len)
        {
          Buffer_WriteExt(&BLE_rx_index, ((MESSAGE_PACKAGE *)(BLEoperate.rcvop.pkg_buffer))->data, t_len);
        }     
      } 
      BC76xx_send_data_pkg();
       break;
      case _BLE_PWR_ON_ERROR_ :
			if(BC76XX_PDN_STS)
			{
				BC76xx_power_mode(POWER_DOWN);
				BC76xx_wakeup(LOW);				
			}
         break;         
   }
}
///*********************************************************************************************************//**
// * @brief   This function handles USART0 interrupt.
// * @retval  None
// ************************************************************************************************************/
//void USART0_IRQHandler(void)
//{
//	BC76XX_USART->SR &= USART_FLAG_RXDR;
//	BLE_rcv_buffer[BLEoperate.rcvop.rcv_head] = BC76XX_USART->DR;
//	BLEoperate.rcvop.rcv_head++;
//	BLEoperate.rcvop.rcv_head &= 0x0F;
//  /*--------------------------------------------------------------------------------------------------------*/
//  /* DSB instruction is added in this function to ensure the write operation which is for clearing interrupt*/
//  /* flag is actually completed before exiting ISR. It prevents the NVIC from detecting the interrupt again */
//  /* since the write register operation may be pended in the internal write buffer of Cortex-Mx when program*/
//  /* has exited interrupt routine. This DSB instruction may be masked if this function is called in the     */
//  /* beginning of ISR and there are still some instructions before exiting ISR.                             */
//  /*--------------------------------------------------------------------------------------------------------*/
//  __DSB();	
//}

