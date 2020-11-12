/*********************************************************************************************************//**
 * @file    ADC/AnalogWatchdog/ht32f5xxxx_01_it.c
 * @version $Rev:: 2970         $
 * @date    $Date:: 2018-08-03 #$
 * @brief   This file provides all interrupt service routine.
 *************************************************************************************************************
 * @attention
 *
 * Firmware Disclaimer Information
 *
 * 1. The customer hereby acknowledges and agrees that the program technical documentation, including the
 *    code, which is supplied by Holtek Semiconductor Inc., (hereinafter referred to as "HOLTEK") is the
 *    proprietary and confidential intellectual property of HOLTEK, and is protected by copyright law and
 *    other intellectual property laws.
 *
 * 2. The customer hereby acknowledges and agrees that the program technical documentation, including the
 *    code, is confidential information belonging to HOLTEK, and must not be disclosed to any third parties
 *    other than HOLTEK and the customer.
 *
 * 3. The program technical documentation, including the code, is provided "as is" and for customer reference
 *    only. After delivery by HOLTEK, the customer shall use the program technical documentation, including
 *    the code, at their own risk. HOLTEK disclaims any expressed, implied or statutory warranties, including
 *    the warranties of merchantability, satisfactory quality and fitness for a particular purpose.
 *
 * <h2><center>Copyright (C) Holtek Semiconductor Inc. All rights reserved</center></h2>
 ************************************************************************************************************/

/* Includes ------------------------------------------------------------------------------------------------*/
#include "ht32.h"
#include "ht32_board.h"
#include	"timer.h"
#include	"channel_dispose.h"
#include	"key_func.h"
#include	"QSPI_flash.h"

/** @addtogroup HT32_Series_Peripheral_Examples HT32 Peripheral Examples
  * @{
  */

/** @addtogroup ADC_Examples ADC
  * @{
  */

/** @addtogroup AnalogWatchdog
  * @{
  */




/* Global functions ----------------------------------------------------------------------------------------*/
/*********************************************************************************************************//**
 * @brief   This function handles NMI exception.
 * @retval  None
 ************************************************************************************************************/
void NMI_Handler(void)
{
}

/*********************************************************************************************************//**
 * @brief   This function handles Hard Fault exception.
 * @retval  None
 ************************************************************************************************************/
void HardFault_Handler(void)
{
  #if 1

  static vu32 gIsContinue = 0;
  /*--------------------------------------------------------------------------------------------------------*/
  /* For development FW, MCU run into the while loop when the hardfault occurred.                           */
  /* 1. Stack Checking                                                                                      */
  /*    When a hard fault exception occurs, MCU push following register into the stack (main or process     */
  /*    stack). Confirm R13(SP) value in the Register Window and typing it to the Memory Windows, you can   */
  /*    check following register, especially the PC value (indicate the last instruction before hard fault).*/
  /*    SP + 0x00    0x04    0x08    0x0C    0x10    0x14    0x18    0x1C                                   */
  /*           R0      R1      R2      R3     R12      LR      PC    xPSR                                   */
  while (gIsContinue == 0)
  {
  }
  /* 2. Step Out to Find the Clue                                                                           */
  /*    Change the variable "gIsContinue" to any other value than zero in a Local or Watch Window, then     */
  /*    step out the HardFault_Handler to reach the first instruction after the instruction which caused    */
  /*    the hard fault.                                                                                     */
  /*--------------------------------------------------------------------------------------------------------*/

  #else

  /*--------------------------------------------------------------------------------------------------------*/
  /* For production FW, you shall consider to reboot the system when hardfault occurred.                    */
  /*--------------------------------------------------------------------------------------------------------*/
  NVIC_SystemReset();

  #endif
}

/*********************************************************************************************************//**
 * @brief   This function handles SVCall exception.
 * @retval  None
 ************************************************************************************************************/
void SVC_Handler(void)
{
}

/*********************************************************************************************************//**
 * @brief   This function handles PendSVC exception.
 * @retval  None
 ************************************************************************************************************/
void PendSV_Handler(void)
{
}

/*********************************************************************************************************//**
 * @brief   This function handles SysTick Handler.
 * @retval  None
 ************************************************************************************************************/
void SysTick_Handler(void)
{
  Midi_Beat48_Counter();
}

/* Private functions ---------------------------------------------------------------------------------------*/



/*********************************************************************************************************//**
 * @brief   This function handles BFTM0 interrupt.
 * @retval  None
 ************************************************************************************************************/
void BFTM0_IRQHandler(void)
{
	User_TimerInterrupt_2ms();
	BFTM_ClearFlag(HT_BFTM0);	
}


/************************************************************************************************************
 * @brief   This function handles QSPI interrupt.
 * @retval  None
************************************************************************************************************/
void QSPI_IRQHandler(void)
{
	u8 i, tmp;

  while((HT_QSPI->FSR & 0xf0) && (Rx != SEND_NUM))
  {
    Rx_data_16[Rx++] = HT_QSPI->DR;
  }

  tmp = 8 - (HT_QSPI->FSR & 0xF);
  for(i = 0; i < tmp; i++)
  {
		if(Tx == SEND_NUM)
			break;

		HT_QSPI->DR = Tx_data_16[Tx++];
  }
  
  if( (Tx == SEND_NUM) && (Rx == SEND_NUM))
  {
    SPI_IntConfig(HT_QSPI, SPI_INT_RXBNE | SPI_INT_TXBE, DISABLE);	
    tmpflag += 100;
  }	
}

/*********************************************************************************************************//**
 * @brief   This function handles MIDI interrupt.
 * @retval  None
 ************************************************************************************************************/
void MIDI_IRQHandler(void)
{
	
	__R_Intf_Count++;
	__L_It_Spi_Data_Judge();
	
if(R_Wave_Count!=0)
{	
	R_Wave_Count--;	
}	

	MIDI_ClearFlag(MIDIx, MIDI_FLAG_INTF);
}

void PDMA_CH0_1_IRQHandler(void)
{
}

/*********************************************************************************************************//**
 * @brief   This function handles PDMA_CH2_5 interrupt.
 * @retval  None
 ************************************************************************************************************/
void PDMA_CH2_5_IRQHandler(void)
{
  
}




/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */
