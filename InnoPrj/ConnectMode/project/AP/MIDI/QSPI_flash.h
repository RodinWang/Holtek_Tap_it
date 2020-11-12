/*----------------------------------------------------------------------------------------------------------*/
/* Holtek Semiconductor Inc.                                                                                */
/*                                                                                                          */
/* Copyright (c) 2010-2012 by Holtek Semiconductor Inc.                                                          */
/* All rights reserved.                                                                                     */
/*                                                                                                          */
/*------------------------------------------------------------------------------------------------------------
  File Name        : QSPI_flash.h
  Version          : V0.1
  Date[mm/dd/yyyy] : 08/03/2010
  Description      : The header file of QSPI_flash.c module.
------------------------------------------------------------------------------------------------------------*/
/* Define to prevent recursive inclusion -------------------------------------------------------------------*/
#ifndef __QSPI_FLASH_H
#define __QSPI_FLASH_H


/* Includes ------------------------------------------------------------------------------------------------*/
//#include "ht32f165x.h"
#include "ht32.h"

#define QSPIx       HT_QSPI
/* Exported types ------------------------------------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------------------------------------*/
/* Exported functions --------------------------------------------------------------------------------------*/

extern u16 Tx, Rx;
extern vu8 tmpflag;
extern u16 Rx_data_16[];
extern u16 Tx_data_16[];
extern u16 SEND_NUM;


/*----- High layer function -----*/
void QSPI_FLASH_Init(void);
void QSPI_FLASH_P4E(u32 SectorAddr);
void QSPI_FLASH_ChipErase(void);
void QSPI_FLASH_PageWrite(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite);
void QSPI_FLASH_BufferWrite(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite);
void QSPI_FLASH_BufferRead(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead);
void QSPI_FLASH_BufferFastRead(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead);
u32 QSPI_FLASH_RDID(void);
u32 QSPI_FLASH_ReadID(void);
void QSPI_FLASH_StartReadSequence(u32 ReadAddr);
void QSPI_FLASH_DUAL_READ(u16* pBuffer, u32 ReadAddr, u16 NumByteToRead);

//2016/12/06 added functions for S25FL032P
void QSPI_FLASH_P8E(u32 SectorAddr);
void QSPI_FLASH_DIOR(u16* pBuffer, u32 ReadAddr, u16 NumByteToRead);
//void QSPI_FLASH_Send2Bytes(u16 data);
//u16 QSPI_FLASH_SendByte_TXBE(u16 byte); //--void QSPI_FLASH_SendByte_TXBE(u8 byte);

//2016/12/08 added functions 
u32 QSPI_FLASH_FIFO_ReadID(void);
u32 QSPI_FLASH_FIFO_RDID(void);
void QSPI_FLASH_FIFO_BufferRead(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead);
void QSPI_FLASH_FIFO_PageWrite(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite);
void QSPI_FLASH_FIFO_BufferFastRead(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead);
void QSPI_FLASH_FIFO_DUAL_READ(u16* pBuffer, u32 ReadAddr, u16 NumByteToRead);
void QSPI_FLASH_FIFO_DIOR(u16* pBuffer, u32 ReadAddr, u16 NumByteToRead);

//2016/12/12 added functions for S25FL032P
void QSPI_FLASH_WREN(void);
void QSPI_FLASH_WRR(u8 Status_data, u8 Configuration_data);
void QSPI_FLASH_QOR(u16* pBuffer, u32 ReadAddr, u16 NumByteToRead);
//u16 QSPI_FLASH_SendWord(u16 word);
void QSPI_FLASH_QPP(u16* pBuffer, u32 WriteAddr, u16 NumByteToWrite);
void QSPI_FLASH_QIOR(u16* pBuffer, u32 ReadAddr, u16 NumByteToRead);
void QSPI_FLASH_SE(u32 SectorAddr);
void QSPI_FLASH_BE(void); //--QSPI_FLASH_BE(u32 SectorAddr)
	
//2016/12/15	added functions for S25FL032P
void QSPI_FLASH_FIFO_QOR(u16* pBuffer, u32 ReadAddr, u16 NumByteToRead);
void QSPI_FLASH_FIFO_QIOR(u16* pBuffer, u32 ReadAddr, u16 NumByteToRead);	
void QSPI_FLASH_FIFO_QPP(u16* pBuffer, u32 WriteAddr, u16 NumByteToWrite);

//2016/12/27 added function (以byte為單位傳送)
void QSPI_FLASH_BYTE_DUAL_READ(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead);
void QSPI_FLASH_BYTE_QOR(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead);
void QSPI_FLASH_BYTE_DIOR(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead);
void QSPI_FLASH_BYTE_QIOR(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead);
void QSPI_FLASH_BYTE_QPP(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite);
void QSPI_FLASH_FIFO_BYTE_DUAL_READ(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead);
void QSPI_FLASH_FIFO_BYTE_QOR(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead);
void QSPI_FLASH_FIFO_BYTE_DIOR(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead);
void QSPI_FLASH_FIFO_BYTE_QIOR(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead);
void QSPI_FLASH_FIFO_BYTE_QPP(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite);
void QSPI_FLASH_FIFO_BYTE_QIOR_QPI(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead);

//2016/12/29	added functions for MX66L1G45G
void QSPI_FLASH_BYTE_QIOR_QPI(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead);

void QSPI_FLASH_EQPI(void);
void QSPI_FLASH_RSTQPI(void);

/*----- Low layer function -----*/
u8 QSPI_FLASH_ReadByte(void);
u8 QSPI_FLASH_SendByte(u8 byte); //--u8 QSPI_FLASH_SendByte(u8 byte);
u16 QSPI_FLASH_Send(u16 data);
//u16 QSPI_FLASH_SendHalfWord(u16 HalfWord);
void QSPI_FLASH_WriteEnable(void);
void QSPI_FLASH_WaitForWriteEnd(void);


#endif /* __QSPI_FLASH_H ---------------------------------------------------------------------------------*/
