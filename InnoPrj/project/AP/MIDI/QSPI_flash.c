/*----------------------------------------------------------------------------------------------------------*/
/* Holtek Semiconductor Inc.                                                                                */
/*                                                                                                          */
/* Copyright (c) 2010-2012 by Holtek Semiconductor Inc.                                                          */
/* All rights reserved.                                                                                     */
/*                                                                                                          */
/*------------------------------------------------------------------------------------------------------------
  File Name        : QSPI_flash.c
  Version          : V0.1
  Date[mm/dd/yyyy] : 08/03/2010
  Description      : QSPI_flash.c.
------------------------------------------------------------------------------------------------------------*/

/* Includes ------------------------------------------------------------------------------------------------*/
#include "QSPI_flash.h"
#include "ht32f5xxxx_spi_midi.h"
#include <stdio.h>

/* Private typedef -----------------------------------------------------------------------------------------*/
/* Private define ------------------------------------------------------------------------------------------*/
#define QSPI_FLASH_PageSize 256

#define WRITE      0x02  /* Page Program instruction */
#define EWSR       0x50  /* Enable-Write-Status-Register instruction */
#define WRSR       0x01  /* Write Status Register instruction */ 
#define WREN       0x06  /* Write enable instruction */
#define WRR				 0x01

#define READ       0x03  /* Read from Memory instruction */
#define RDSR       0x05  /* Read Status Register instruction  */
#define RDID       0x9F  /* Read identification */
#define ReadID     0x90  /* Read identification */
#define F_READ     0x0B  /* Read date bytes at high speed */

#define FRDO       0x3B  /* Read Memory with Dual Output */
#define DIOR       0xBB  /* Read Memory with Dual Output */
#define QIOR			 0xEB
#define QOR 			 0x6B
#define QPP 			 0x32

#define SE         0xD8  /* Sector Erase instruction */
#define BE         0x60  /* Sector Erase instruction */
#define P4E		     0x20
#define P8E        0x40  /* Sector Erase instruction */
#define CE         0xC7  /* Chip Erase instruction */
#define EQPI			 0x35	 /* QPI mode enable */
#define RSTQPI		 0xF5  /* QPI mode disable */

#define WIP_Flag   0x01  /* Write In Progress (WIP) flag */

#define Dummy_Byte 0x00

u16 Rx_data_16[256];
u16 Tx_data_16[256];
u16 Tx, Rx;
vu8 tmpflag;
u16 SEND_NUM = 0;

SPI_InitTypeDef QSPI_InitStructure;

/* Private macro -------------------------------------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------------------------------------*/
/* Private functions ---------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------------------------------------
  Function Name  : QSPI_FLASH_Init
  Description    : Initializes peripherals used by the QSPI flash driver.
  Input          : None
  Output         : None
  Return         : None
------------------------------------------------------------------------------------------------------------*/
void QSPI_FLASH_Init(void)
{
  //GPIO_InitTypeDef GPIO_InitStructure;

 	QSPI_InitStructure.SPI_Mode = SPI_MASTER;
  QSPI_InitStructure.SPI_FIFO = SPI_FIFO_DISABLE;
 	QSPI_InitStructure.SPI_DataLength =SPI_DATALENGTH_8;
  QSPI_InitStructure.SPI_SELMode = SPI_SEL_SOFTWARE;
  QSPI_InitStructure.SPI_SELPolarity = SPI_SELPOLARITY_LOW;
  QSPI_InitStructure.SPI_FirstBit = SPI_FIRSTBIT_MSB;
  //QSPI_InitStructure.QSPI_CPOL = QSPI_CPOL_HIGH;
  //QSPI_InitStructure.QSPI_CPHA = QSPI_CPHA_SECOND;
  QSPI_InitStructure.SPI_CPOL = SPI_CPOL_LOW;
  QSPI_InitStructure.SPI_CPHA = SPI_CPHA_FIRST;
  QSPI_InitStructure.SPI_RxFIFOTriggerLevel = 1;
  QSPI_InitStructure.SPI_TxFIFOTriggerLevel = 0;
  QSPI_InitStructure.SPI_ClockPrescaler = 10;
	SPI_Init(QSPIx, &QSPI_InitStructure);	

	SPI_SELOutputCmd(QSPIx, ENABLE);

	SPI_Cmd(QSPIx, ENABLE);
}

/*********************************************************************************************************//**
  * @brief  Configure the SEL state by software.
  * @param  QSPIx: where QSPIx is the selected QSPI from the QSPI peripherals.
  * @param  QSPI_SoftwareSEL: specify if the QSPI SEL to be active or inactive.
  *   This parameter can be one of the following values:
  *     @arg SPI_SEL_ACTIVE     : activate SEL signal
  *     @arg SPI_SEL_INACTIVE   : deactivate SEL signal
  * @retval None
  ***********************************************************************************************************/
void QSPI_SoftwareSELCmd(HT_SPI_TypeDef* SPIx, u32 QSPI_SoftwareSEL)
{
  /* Check the parameters                                                                                   */
  Assert_Param(IS_QSPI(SPIx));
  Assert_Param(IS_QSPI_SOFTWARE_SEL(QSPI_SoftwareSEL));

  if (QSPI_SoftwareSEL != SPI_SEL_INACTIVE)
  {
    SPIx->CR0 |= SPI_SEL_ACTIVE;
  }
  else
  {
    SPIx->CR0 &= SPI_SEL_INACTIVE;
  }
}


/*------------------------------------------------------------------------------------------------------------
  Function Name  : QSPI_FLASH_P4E
  Description    : Erases the specified FLASH sector.
  Input          : SectorAddr: address of the sector to erase.
  Output         : None
  Return         : None
------------------------------------------------------------------------------------------------------------*/
void QSPI_FLASH_P4E(u32 SectorAddr)
{
  /* Send write enable instruction */
  QSPI_FLASH_WriteEnable();

  /* Sector Erase */ 
  /* Select the FLASH: Chip Select low */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_ACTIVE);
  /* Send Sector Erase instruction */
  QSPI_FLASH_SendByte(P4E);
  /* Send SectorAddr high nibble address byte */
  QSPI_FLASH_SendByte((SectorAddr & 0xFF0000) >> 16);
  /* Send SectorAddr medium nibble address byte */
  QSPI_FLASH_SendByte((SectorAddr & 0xFF00) >> 8);
  /* Send SectorAddr low nibble address byte */
  QSPI_FLASH_SendByte(SectorAddr & 0xFF);
  /* Deselect the FLASH: Chip Select high */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_INACTIVE);

  /* Wait the end of Flash writing */
  QSPI_FLASH_WaitForWriteEnd();
}

/*------------------------------------------------------------------------------------------------------------
  Function Name  : QSPI_FLASH_ChipErase
  Description    : Erases the entire FLASH.
  Input          : None
  Output         : None
  Return         : None
------------------------------------------------------------------------------------------------------------*/
void QSPI_FLASH_ChipErase(void)
{
  /* Send write enable instruction */
  QSPI_FLASH_WriteEnable();

  /* Bulk Erase */ 
  /* Select the FLASH: Chip Select low */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_ACTIVE);
  /* Send Chip Erase instruction  */
  QSPI_FLASH_SendByte(CE);
  /* Deselect the FLASH: Chip Select high */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_INACTIVE);

  /* Wait the end of Flash writing */
  QSPI_FLASH_WaitForWriteEnd();
}

/*------------------------------------------------------------------------------------------------------------
  Function Name  : QSPI_FLASH_PageWrite
  Description    : Writes more than one byte to the FLASH with a single WRITE cycle(Page WRITE sequence). 
                   The number of byte can't exceed the FLASH page size.
  Input          : - pBuffer : pointer to the buffer  containing the data to be written to the FLASH.
                   - WriteAddr : FLASH's internal address to write to.
                   - NumByteToWrite : number of bytes to write to the FLASH, must be equal or less 
                     than "QSPI_FLASH_PageSize" value.
  Output         : None
  Return         : None
------------------------------------------------------------------------------------------------------------*/
void QSPI_FLASH_PageWrite(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite)
{
  /* Enable the write access to the FLASH */
  QSPI_FLASH_WriteEnable();
  
  /* Select the FLASH: Chip Select low */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_ACTIVE);
  /* Send "Write to Memory " instruction */
  QSPI_FLASH_SendByte(WRITE);
  /* Send WriteAddr high nibble address byte to write to */
  QSPI_FLASH_SendByte((WriteAddr & 0xFF0000) >> 16);
  /* Send WriteAddr medium nibble address byte to write to */
  QSPI_FLASH_SendByte((WriteAddr & 0xFF00) >> 8);  
  /* Send WriteAddr low nibble address byte to write to */
  QSPI_FLASH_SendByte(WriteAddr & 0xFF);
  
  /* while there is data to be written on the FLASH */
  while(NumByteToWrite--) 
  {
    /* Send the current byte */
    QSPI_FLASH_SendByte(*pBuffer);
    /* Point on the next byte to be written */
    pBuffer++; 
  }
  
  /* Deselect the FLASH: Chip Select high */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_INACTIVE);
  
  /* Wait the end of Flash writing */
  QSPI_FLASH_WaitForWriteEnd();
}

/*------------------------------------------------------------------------------------------------------------
  Function Name  : QSPI_FLASH_BufferWrite
  Description    : Writes block of data to the FLASH. In this function, the number of WRITE cycles are reduced, 
                   using Page WRITE sequence.
  Input          : - pBuffer : pointer to the buffer  containing the data to be written to the FLASH.
                   - WriteAddr : FLASH's internal address to write to.
                   - NumByteToWrite : number of bytes to write to the FLASH.
  Output         : None
  Return         : None
------------------------------------------------------------------------------------------------------------*/
void QSPI_FLASH_BufferWrite(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite)
{
	u8 NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0, temp = 0;

  Addr = WriteAddr % QSPI_FLASH_PageSize;
  count = QSPI_FLASH_PageSize - Addr;
  NumOfPage =  NumByteToWrite / QSPI_FLASH_PageSize;
  NumOfSingle = NumByteToWrite % QSPI_FLASH_PageSize;
  
  if(Addr == 0) /* WriteAddr is QSPI_FLASH_PageSize aligned  */
  {
    if(NumOfPage == 0) /* NumByteToWrite < QSPI_FLASH_PageSize */
    {
      QSPI_FLASH_PageWrite(pBuffer, WriteAddr, NumByteToWrite);
    }
    else /* NumByteToWrite > QSPI_FLASH_PageSize */ 
    {
      while(NumOfPage--)
      {
        QSPI_FLASH_PageWrite(pBuffer, WriteAddr, QSPI_FLASH_PageSize);
        WriteAddr +=  QSPI_FLASH_PageSize;
        pBuffer += QSPI_FLASH_PageSize;  
      }    
     
      QSPI_FLASH_PageWrite(pBuffer, WriteAddr, NumOfSingle);
   }
  }
  else /* WriteAddr is not QSPI_FLASH_PageSize aligned  */
  {
    if(NumOfPage== 0) /* NumByteToWrite < QSPI_FLASH_PageSize */
    {
      if(NumOfSingle > count) /* (NumByteToWrite + WriteAddr) > QSPI_FLASH_PageSize */
      {
        temp = NumOfSingle - count;
      
        QSPI_FLASH_PageWrite(pBuffer, WriteAddr, count);
        WriteAddr +=  count;
        pBuffer += count; 
        
        QSPI_FLASH_PageWrite(pBuffer, WriteAddr, temp);
      }
      else
      {
        QSPI_FLASH_PageWrite(pBuffer, WriteAddr, NumByteToWrite);
      }
    }
    else /* NumByteToWrite > QSPI_FLASH_PageSize */
    {
      NumByteToWrite -= count;
      NumOfPage =  NumByteToWrite / QSPI_FLASH_PageSize;
      NumOfSingle = NumByteToWrite % QSPI_FLASH_PageSize;
      
      QSPI_FLASH_PageWrite(pBuffer, WriteAddr, count);
      WriteAddr +=  count;
      pBuffer += count;  
     
      while(NumOfPage--)
      {
        QSPI_FLASH_PageWrite(pBuffer, WriteAddr, QSPI_FLASH_PageSize);
        WriteAddr +=  QSPI_FLASH_PageSize;
        pBuffer += QSPI_FLASH_PageSize;
      }
      
      if(NumOfSingle != 0)
      {
        QSPI_FLASH_PageWrite(pBuffer, WriteAddr, NumOfSingle);
      }
    }
  }
}

/*------------------------------------------------------------------------------------------------------------
  Function Name  : QSPI_FLASH_BufferRead
  Description    : Reads a block of data from the FLASH.
  Input          : - pBuffer : pointer to the buffer that receives the data read from the FLASH.
                   - ReadAddr : FLASH's internal address to read from.
                   - NumByteToRead : number of bytes to read from the FLASH.
  Output         : None
  Return         : None
------------------------------------------------------------------------------------------------------------*/
void QSPI_FLASH_BufferRead(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead)
{
  /* Select the FLASH: Chip Select low */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_ACTIVE);
  
  /* Send "Read from Memory " instruction */
  QSPI_FLASH_SendByte(READ);
  
  /* Send ReadAddr high nibble address byte to read from */
  QSPI_FLASH_SendByte((ReadAddr & 0xFF0000) >> 16);
  /* Send ReadAddr medium nibble address byte to read from */
  QSPI_FLASH_SendByte((ReadAddr& 0xFF00) >> 8);
  /* Send ReadAddr low nibble address byte to read from */
  QSPI_FLASH_SendByte(ReadAddr & 0xFF);

  //QSPI_FIFOReset(QSPIx, QSPI_FIFO_RX);

  while(NumByteToRead--) /* while there is data to be read */
  {
    /* Read a byte from the FLASH */
    *pBuffer = QSPI_FLASH_SendByte(Dummy_Byte);
    /* Point to the next location where the byte read will be saved */
    pBuffer++;
  }
  
  /* Deselect the FLASH: Chip Select high */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_INACTIVE);
}

/*------------------------------------------------------------------------------------------------------------
  Function Name  : QSPI_FLASH_BufferFastRead
  Description    : Reads a block of data from the FLASH.
  Input          : - pBuffer : pointer to the buffer that receives the data read from the FLASH.
                   - ReadAddr : FLASH's internal address to read from.
                   - NumByteToRead : number of bytes to read from the FLASH.
  Output         : None
  Return         : None
------------------------------------------------------------------------------------------------------------*/
void QSPI_FLASH_BufferFastRead(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead)
{
  /* Select the FLASH: Chip Select low */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_ACTIVE);
  
  /* Send "Read from Memory " instruction */
  QSPI_FLASH_SendByte(F_READ);
  
  /* Send ReadAddr high nibble address byte to read from */
  QSPI_FLASH_SendByte((ReadAddr & 0xFF0000) >> 16);
  /* Send ReadAddr medium nibble address byte to read from */
  QSPI_FLASH_SendByte((ReadAddr& 0xFF00) >> 8);
  /* Send ReadAddr low nibble address byte to read from */
  QSPI_FLASH_SendByte(ReadAddr & 0xFF);

  QSPI_FLASH_SendByte(Dummy_Byte);

  //QSPI_FIFOReset(QSPIx, QSPI_FIFO_RX);

  while(NumByteToRead--) /* while there is data to be read */
  {
    /* Read a byte from the FLASH */
    *pBuffer = QSPI_FLASH_SendByte(Dummy_Byte);
    /* Point to the next location where the byte read will be saved */
    pBuffer++;
  }
  
  /* Deselect the FLASH: Chip Select high */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_INACTIVE);
}

/*------------------------------------------------------------------------------------------------------------
  Function Name  : QSPI_FLASH_ReadID
  Description    : Reads FLASH identification.
  Input          : None
  Output         : None
  Return         : FLASH identification
------------------------------------------------------------------------------------------------------------*/
u32 QSPI_FLASH_ReadID(void)
{
  vu16 Temp = 0;
	vu8 Temp0 = 0, Temp1 = 0;

  /* Select the FLASH: Chip Select low */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_ACTIVE);
  
  /* Send "READ ID " instruction */
  QSPI_FLASH_SendByte(ReadID);

  QSPI_FLASH_SendByte(Dummy_Byte);
  QSPI_FLASH_SendByte(Dummy_Byte);
  QSPI_FLASH_SendByte(0);

  /* Read a byte from the FLASH */
  Temp0 = QSPI_FLASH_SendByte(Dummy_Byte);

  /* Read a byte from the FLASH */
  Temp1 = QSPI_FLASH_SendByte(Dummy_Byte);

  /* Deselect the FLASH: Chip Select high */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_INACTIVE);

  Temp = (Temp0 << 8) | (Temp1);
  return Temp;

}

/*------------------------------------------------------------------------------------------------------------
  Function Name  : QSPI_FLASH_RDID
  Description    : Reads FLASH identification.
  Input          : None
  Output         : None
  Return         : FLASH identification
------------------------------------------------------------------------------------------------------------*/
u32 QSPI_FLASH_RDID(void)
{
  vu32 Temp = 0;
	vu8 Temp0 = 0, Temp1 = 0, Temp2 = 0, Temp3 = 0;

  /* Select the FLASH: Chip Select low */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_ACTIVE);
  
  /* Send "READ ID " instruction */
  QSPI_FLASH_SendByte(RDID);

  /* Read a byte from the FLASH */
  Temp0 = QSPI_FLASH_SendByte(Dummy_Byte);

  /* Read a byte from the FLASH */
  Temp1 = QSPI_FLASH_SendByte(Dummy_Byte);
  
  /* Read a byte from the FLASH */
  Temp2 = QSPI_FLASH_SendByte(Dummy_Byte);
	
	/* Read a byte from the FLASH */
  Temp3 = QSPI_FLASH_SendByte(Dummy_Byte);

  /* Deselect the FLASH: Chip Select high */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_INACTIVE);

  Temp = (Temp0 << 24) | (Temp1 << 16) | (Temp2 << 8) | Temp3;
  return Temp;

}

/*------------------------------------------------------------------------------------------------------------
  Function Name  : QSPI_FLASH_SendByte
  Description    : Sends a byte through the QSPI interface and return 2 bytes received from the QSPI bus in Dual mode.
  Input          : byte : byte to send.
  Output         : None
  Return         : The value of the received byte.
------------------------------------------------------------------------------------------------------------*/
u8 QSPI_FLASH_SendByte(u8 byte)
{
  /* Loop while DR register in not emplty */
  while(!SPI_GetFlagStatus(QSPIx, SPI_FLAG_TXBE));

  /* Send byte through the QSPIx peripheral */
  SPI_SendData(QSPIx, byte);

  /* Wait to receive a byte */
  while(!SPI_GetFlagStatus(QSPIx, SPI_FLAG_RXBNE));

  /* Return the byte read from the QSPI bus */
  return ((u8)SPI_ReceiveData(QSPIx));
}

/*------------------------------------------------------------------------------------------------------------
  Function Name  : QSPI_FLASH_Send
  Description    : Sends data through the QSPI interface and return 2 bytes received from the QSPI bus.
  Input          : byte : byte to send.
  Output         : None
  Return         : The value of the received byte.
------------------------------------------------------------------------------------------------------------*/
u16 QSPI_FLASH_Send(u16 data)
{
  /* Loop while DR register in not emplty */
  while(!SPI_GetFlagStatus(QSPIx, SPI_FLAG_TXBE));

  /* Send byte through the QSPIx peripheral */
  SPI_SendData(QSPIx, data);

  /* Wait to receive data */
  while(!SPI_GetFlagStatus(QSPIx, SPI_FLAG_RXBNE));

  /* Return the data from the QSPI bus */
  return SPI_ReceiveData(QSPIx);
}

/*------------------------------------------------------------------------------------------------------------
  Function Name  : QSPI_FLASH_SendByte_TXBE
  Description    : Sends a byte through the QSPI interface and return the byte received from the QSPI bus.
  Input          : byte : byte to send.
  Output         : None
  Return         : None
------------------------------------------------------------------------------------------------------------*/
u16 QSPI_FLASH_SendByte_TXBE(u16 byte)
{
  /* Loop while DR register in not emplty */
  while(!SPI_GetFlagStatus(QSPIx, SPI_FLAG_TXBE));

  /* Send byte through the QSPIx peripheral */
  SPI_SendData(QSPIx, byte);

  while(!SPI_GetFlagStatus(QSPIx, SPI_FLAG_TXBE));

	/* Return the byte read from the QSPI bus */
  return SPI_ReceiveData(QSPIx);
}

/*------------------------------------------------------------------------------------------------------------
  Function Name  : QSPI_FLASH_WriteEnable
  Description    : Enables the write access to the FLASH.
  Input          : None.
  Output         : None
  Return         : None
------------------------------------------------------------------------------------------------------------*/
void QSPI_FLASH_WriteEnable(void)
{
  /* Select the FLASH: Chip Select low */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_ACTIVE);
  
  /* Send "Write Enable" instruction */
  QSPI_FLASH_SendByte(WREN);
  
  /* Deselect the FLASH: Chip Select high */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_INACTIVE);
}

/*------------------------------------------------------------------------------------------------------------
  Function Name  : QSPI_FLASH_WaitForWriteEnd
  Description    : Polls the status of the Write In Progress (WIP) flag in the FLASH's status register and 
                   loop until write  opertaion has completed.
  Input          : None.
  Output         : None
  Return         : None
------------------------------------------------------------------------------------------------------------*/
void QSPI_FLASH_WaitForWriteEnd(void)
{
  u8 FLASH_Status = 0;
  
  /* Select the FLASH: Chip Select low */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_ACTIVE);
  
  /* Send "Read Status Register" instruction */
  QSPI_FLASH_SendByte(RDSR);
   //QSPI_FIFOReset(QSPIx, QSPI_FIFO_RX);
  /* Loop as long as the memory is busy with a write cycle */
  do
  {
    /* Send a dummy byte to generate the clock needed by the FLASH 
    and put the value of the status register in FLASH_Status variable */
    FLASH_Status = QSPI_FLASH_SendByte(Dummy_Byte);

  } while((FLASH_Status & WIP_Flag) == SET); /* Write in progress */

  /* Deselect the FLASH: Chip Select high */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_INACTIVE);
}
/*------------------------------------------------------------------------------------------------------------
  Function Name  : QSPI_FLASH_DUAL_READ
  Input          : None.
  Output         : None
  Return         : None
------------------------------------------------------------------------------------------------------------*/
void QSPI_FLASH_DUAL_READ(u16* pBuffer, u32 ReadAddr, u16 NumByteToRead)
{
  /* Select the FLASH: Chip Select low */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_ACTIVE);
  
  /* Send "Read Memory with Dual Output" instruction */
  QSPI_FLASH_SendByte(FRDO);
  
  /* Send ReadAddr high nibble address byte to read from */
  QSPI_FLASH_SendByte((ReadAddr & 0xFF0000) >> 16);
  /* Send ReadAddr medium nibble address byte to read from */
  QSPI_FLASH_SendByte((ReadAddr& 0xFF00) >> 8);
  /* Send ReadAddr low nibble address byte to read from */
  QSPI_FLASH_SendByte(ReadAddr & 0xFF);
	
	/////////////////////////////////////////
	QSPIx->CR0 |= 0x40;  // enable QSPI dual
	/////////////////////////////////////////
	
	QSPI_FLASH_SendByte(Dummy_Byte);
	
	NumByteToRead = (NumByteToRead+1)/2;
	
  while(NumByteToRead--) /* while there is data to be read */
  {
    /* Read 2 bytes from the FLASH */
    *pBuffer = QSPI_FLASH_Send(Dummy_Byte);
    /* Point to the next location where the byte read will be saved */
    pBuffer++;
  }
  
  /* Deselect the FLASH: Chip Select high */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_INACTIVE);
	
	/////////////////////////////////////////
	QSPIx->CR0 &= ~0x40; // disable QSPI dual
	/////////////////////////////////////////
}

/*------------------------------------------------------------------------------------------------------------
  Function Name  : QSPI_FLASH_P8E
  Description    : Erases the specified FLASH sector.
  Input          : SectorAddr: address of the sector to erase.
  Output         : None
  Return         : None
------------------------------------------------------------------------------------------------------------*/
void QSPI_FLASH_P8E(u32 SectorAddr)
{

	/* Send write enable instruction */
  QSPI_FLASH_WriteEnable();

  /* Sector Erase */ 
  /* Select the FLASH: Chip Select low */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_ACTIVE);
  /* Send Sector Erase instruction */
  QSPI_FLASH_SendByte(P8E);
  /* Send SectorAddr high nibble address byte */
  QSPI_FLASH_SendByte((SectorAddr & 0xFF0000) >> 16);
  /* Send SectorAddr medium nibble address byte */
  QSPI_FLASH_SendByte((SectorAddr & 0xFF00) >> 8);
  /* Send SectorAddr low nibble address byte */
  QSPI_FLASH_SendByte(SectorAddr & 0xFF);
  /* Deselect the FLASH: Chip Select high */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_INACTIVE);

  /* Wait the end of Flash writing */
  QSPI_FLASH_WaitForWriteEnd();
}

/*------------------------------------------------------------------------------------------------------------
  Function Name  : QSPI_FLASH_DIOR
  Input          : None.
  Output         : None
  Return         : None
------------------------------------------------------------------------------------------------------------*/
void QSPI_FLASH_DIOR(u16* pBuffer, u32 ReadAddr, u16 NumByteToRead)
{
	QSPIx->CR0 |= 0x10000;  // enable QDIOR
	
	/* Select the FLASH: Chip Select low */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_ACTIVE);
  
  /* Send "Read Memory with Dual Output" instruction */
  QSPI_FLASH_SendByte(DIOR);
  
	QSPIx->CR0 |= 0x40;  // enable QSPI dual
	
	ReadAddr = (ReadAddr<<8)|(ReadAddr&0xff);
	QSPI_FLASH_Send((ReadAddr & 0xFFFF0000)>>16);
	
	QSPI_InitStructure.SPI_DataLength = SPI_DATALENGTH_4;
	SPI_Init(QSPIx, &QSPI_InitStructure);
	QSPI_FLASH_SendByte((ReadAddr & 0xFF00)>>8);
	
	QSPIx->CR0 &= ~0x10000;

	QSPI_FLASH_SendByte(0);
	
	QSPI_InitStructure.SPI_DataLength = SPI_DATALENGTH_8;
	SPI_Init(QSPIx, &QSPI_InitStructure);
	
	NumByteToRead = (NumByteToRead+1)/2;
	
  while(NumByteToRead--) /* while there is data to be read */
  {
    /* Read 2 bytes from the FLASH */
    *pBuffer = QSPI_FLASH_Send(Dummy_Byte);
    /* Point to the next location where the byte read will be saved */
    pBuffer++;
  }
  
  /* Deselect the FLASH: Chip Select high */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_INACTIVE);
	
	/////////////////////////////////////////
	QSPIx->CR0 &= ~0x40; // disable QSPI dual
	/////////////////////////////////////////
}

/*------------------------------------------------------------------------------------------------------------
  Function Name  : QSPI_FLASH_QOR
  Input          : None.
  Output         : None
  Return         : None
------------------------------------------------------------------------------------------------------------*/
void QSPI_FLASH_QOR(u16* pBuffer, u32 ReadAddr, u16 NumByteToRead)
{
	/* Select the FLASH: Chip Select low */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_ACTIVE);
  
  /* Send instruction */
  QSPI_FLASH_SendByte(QOR);
  
  /* Send ReadAddr high nibble address byte to read from */
  QSPI_FLASH_SendByte((ReadAddr & 0xFF0000) >> 16);
  /* Send ReadAddr medium nibble address byte to read from */
  QSPI_FLASH_SendByte((ReadAddr& 0xFF00) >> 8);
  /* Send ReadAddr low nibble address byte to read from */
  QSPI_FLASH_SendByte(ReadAddr & 0xFF);
	
	HT_QSPI->CR1 &= 0xfffffff0;
	HT_QSPI->CR1 |= 4;
	HT_QSPI->CR0 |= 0x20000;
	
	QSPI_FLASH_SendByte(Dummy_Byte);
	QSPI_FLASH_SendByte(Dummy_Byte);
	
	//Quad mode: 一次送 4 clocks => receive 16 bits data = 2 bytes
	NumByteToRead = (NumByteToRead+1)/2; 
	
  while(NumByteToRead--) /* while there is data to be read */
  {
    /* Read 2 bytes from the FLASH */
    *pBuffer = QSPI_FLASH_Send(Dummy_Byte);
    /* Point to the next location where the byte read will be saved */
    pBuffer++;
  }
  
  /* Deselect the FLASH: Chip Select high */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_INACTIVE);
	
	HT_QSPI->CR0 &= ~0x20000;
	HT_QSPI->CR1 &= 0xfffffff0;
	HT_QSPI->CR1 |= 8;
}

/*------------------------------------------------------------------------------------------------------------
  Function Name  : QSPI_FLASH_WREN
  Input          : None.
  Output         : None
  Return         : None
------------------------------------------------------------------------------------------------------------*/
void QSPI_FLASH_WREN()
{
	/* Select the FLASH: Chip Select low */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_ACTIVE);
	
	/* Send instruction */
  QSPI_FLASH_SendByte(WREN);
	
	/* Deselect the FLASH: Chip Select high */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_INACTIVE);
}

/*------------------------------------------------------------------------------------------------------------
  Function Name  : QSPI_FLASH_WRR
  Input          : None.
  Output         : None
  Return         : None
------------------------------------------------------------------------------------------------------------*/
void QSPI_FLASH_WRR(u8 Status_data, u8 Configuration_data)
{
	QSPI_FLASH_WREN();
	
	/* Select the FLASH: Chip Select low */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_ACTIVE);
	
	/* Send instruction */
  QSPI_FLASH_SendByte(WRR);
  
	QSPI_FLASH_SendByte(Status_data);
	
	QSPI_FLASH_SendByte(Configuration_data);
	
	/* Deselect the FLASH: Chip Select high */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_INACTIVE);
	
	/* Wait the end of Flash writing */
	QSPI_FLASH_WaitForWriteEnd();
}

/*------------------------------------------------------------------------------------------------------------
  Function Name  : QSPI_FLASH_QPP
  Input          : None.
  Output         : None
  Return         : None
------------------------------------------------------------------------------------------------------------*/
void QSPI_FLASH_QPP(u16* pBuffer, u32 WriteAddr, u16 NumByteToWrite)
{
	/* Enable the write access to the FLASH */
  QSPI_FLASH_WriteEnable();
  
  QSPIx->CR0 |= 0x10000;  // enable QDIOR
  
  /* Select the FLASH: Chip Select low */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_ACTIVE);
	
  /* Send instruction */
  QSPI_FLASH_SendByte(QPP);
	
  /* Send WriteAddr high nibble address byte to write to */
  QSPI_FLASH_SendByte((WriteAddr & 0xFF0000) >> 16);
  /* Send WriteAddr medium nibble address byte to write to */
  QSPI_FLASH_SendByte((WriteAddr & 0xFF00) >> 8);  
  /* Send WriteAddr low nibble address byte to write to */
  QSPI_FLASH_SendByte(WriteAddr & 0xFF);
  
	HT_QSPI->CR1 &= 0xfffffff0;
	HT_QSPI->CR1 |= 4;
	HT_QSPI->CR0 |= 0x20000;
	
	NumByteToWrite = (NumByteToWrite+1)/2;
	
  /* while there is data to be written on the FLASH */
  while(NumByteToWrite--) 
  {
    /* Send the current word */
    QSPI_FLASH_Send(*pBuffer); //一次打4個clock，送 16 bits = 1 word
    /* Point on the next byte to be written */
    pBuffer++; 
  }
  
  /* Deselect the FLASH: Chip Select high */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_INACTIVE);
	
	HT_QSPI->CR0 &= ~0x30000;
	HT_QSPI->CR1 &= 0xfffffff0;
	HT_QSPI->CR1 |= 8;
  
  /* Wait the end of Flash writing */
  QSPI_FLASH_WaitForWriteEnd();
}

/*------------------------------------------------------------------------------------------------------------
  Function Name  : QSPI_FLASH_QIOR
  Input          : None.
  Output         : None
  Return         : None
------------------------------------------------------------------------------------------------------------*/
void QSPI_FLASH_QIOR(u16* pBuffer, u32 ReadAddr, u16 NumByteToRead)
{
	u16 SendAddr_ModeBits;
	
	QSPIx->CR0 |= 0x10000;  // enable QDIOR
		
	/* Select the FLASH: Chip Select low */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_ACTIVE);
  
	/* Send "Read Memory with Dual Output" instruction */
  QSPI_FLASH_SendByte(QIOR);
  
	HT_QSPI->CR1 &= 0xfffffff0;
	HT_QSPI->CR1 |= 4;
	HT_QSPI->CR0 |= 0x20000;
	
	QSPI_FLASH_Send((ReadAddr & 0xFFFF00) >> 8); //addr bit 23~8
	
	SendAddr_ModeBits = ((ReadAddr & 0xFF) << 8) | (ReadAddr & 0xFF); //(addr bit 7~0 << 8) | (mode bits 7~0)
	QSPI_FLASH_Send(SendAddr_ModeBits);
	
	HT_QSPI->CR0 &= ~0x10000;

	QSPI_FLASH_Send(Dummy_Byte);
	
	NumByteToRead = (NumByteToRead+1)/2;
	
  while(NumByteToRead--) /* while there is data to be read */
  {
    /* Read a byte from the FLASH */
    *pBuffer = QSPI_FLASH_Send(Dummy_Byte);
    /* Point to the next location where the byte read will be saved */
    pBuffer++;
  }
  
  /* Deselect the FLASH: Chip Select high */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_INACTIVE);
	
	HT_QSPI->CR0 &= ~0x20000;
	HT_QSPI->CR1 &= 0xfffffff0;
	HT_QSPI->CR1 |= 8;
}

/*------------------------------------------------------------------------------------------------------------
  Function Name  : QSPI_FLASH_SE
  Description    : Erases the specified FLASH sector.
  Input          : SectorAddr: address of the sector to erase.
  Output         : None
  Return         : None
------------------------------------------------------------------------------------------------------------*/
void QSPI_FLASH_SE(u32 SectorAddr)
{
	/* Send write enable instruction */
  QSPI_FLASH_WriteEnable();

  /* Sector Erase */ 
  /* Select the FLASH: Chip Select low */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_ACTIVE);
  /* Send Sector Erase instruction */
  QSPI_FLASH_SendByte(SE);
  /* Send SectorAddr high nibble address byte */
  QSPI_FLASH_SendByte((SectorAddr & 0xFF0000) >> 16);
  /* Send SectorAddr medium nibble address byte */
  QSPI_FLASH_SendByte((SectorAddr & 0xFF00) >> 8);
  /* Send SectorAddr low nibble address byte */
  QSPI_FLASH_SendByte(SectorAddr & 0xFF);
  /* Deselect the FLASH: Chip Select high */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_INACTIVE);

  /* Wait the end of Flash writing */
  QSPI_FLASH_WaitForWriteEnd();
}

/*------------------------------------------------------------------------------------------------------------
  Function Name  : QSPI_FLASH_BE
  Description    : Erases the specified FLASH sector.
  Input          : SectorAddr: address of the sector to erase.
  Output         : None
  Return         : None
------------------------------------------------------------------------------------------------------------*/
void QSPI_FLASH_BE() //--QSPI_FLASH_BE(u32 SectorAddr)
{
	/* Send write enable instruction */
  QSPI_FLASH_WriteEnable();

  /* Sector Erase */ 
  /* Select the FLASH: Chip Select low */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_ACTIVE);
  /* Send Sector Erase instruction */
  QSPI_FLASH_SendByte(BE);
  //--/* Send SectorAddr high nibble address byte */
  //--QSPI_FLASH_SendByte((SectorAddr & 0xFF0000) >> 16);
  //--/* Send SectorAddr medium nibble address byte */
  //--QSPI_FLASH_SendByte((SectorAddr & 0xFF00) >> 8);
  //--/* Send SectorAddr low nibble address byte */
  //--QSPI_FLASH_SendByte(SectorAddr & 0xFF);
  //--/* Deselect the FLASH: Chip Select high */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_INACTIVE);

  /* Wait the end of Flash writing */
  QSPI_FLASH_WaitForWriteEnd();
}

/*------------------------------------------------------------------------------------------------------------
  Function Name  : QSPI_FLASH_FIFO_QOR
  Input          : None.
  Output         : None
  Return         : None
------------------------------------------------------------------------------------------------------------*/
void QSPI_FLASH_FIFO_QOR(u16* pBuffer, u32 ReadAddr, u16 NumByteToRead)
{
	u16 i;
	
	/* Select the FLASH: Chip Select low */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_ACTIVE);
  
  /* Send instruction */
  QSPI_FLASH_SendByte(QOR);
  
  /* Send ReadAddr high nibble address byte to read from */
  QSPI_FLASH_SendByte((ReadAddr & 0xFF0000) >> 16);
  /* Send ReadAddr medium nibble address byte to read from */
  QSPI_FLASH_SendByte((ReadAddr& 0xFF00) >> 8);
  /* Send ReadAddr low nibble address byte to read from */
  QSPI_FLASH_SendByte(ReadAddr & 0xFF);
	
	HT_QSPI->CR1 &= 0xfffffff0;
	HT_QSPI->CR1 |= 4;
	HT_QSPI->CR0 |= 0x20000;

	QSPI_FLASH_SendByte(Dummy_Byte);
	QSPI_FLASH_SendByte(Dummy_Byte);
	
	//開FIFO傳送
	QSPI_InitStructure.SPI_FIFO = SPI_FIFO_ENABLE;
	QSPI_InitStructure.SPI_RxFIFOTriggerLevel = 3;
  QSPI_InitStructure.SPI_TxFIFOTriggerLevel = 3;
	QSPI_InitStructure.SPI_DataLength = SPI_DATALENGTH_4;
	SPI_Init(QSPIx, &QSPI_InitStructure);	
//	QSPIx->FCR = 0x444;
	Tx = 0;
	Rx = 0;
	//呼叫一次QSPI_FLASH_SendByte會送出4個clock，收到16 bits data = 2 bytes
	NumByteToRead = (NumByteToRead+1)/2; 
	SEND_NUM = NumByteToRead; //送的byte數
	
	//initial TX FIFO
	for(i=0;i<NumByteToRead;i++)
	{
		Tx_data_16[i] = Dummy_Byte;
	}
	
	SPI_IntConfig(QSPIx, SPI_INT_RXBNE | SPI_INT_TXBE, ENABLE);
	
	while(tmpflag != 100);
	
	HT_QSPI->CR0 &= ~0x20000;
	HT_QSPI->CR1 &= 0xfffffff0;
	HT_QSPI->CR1 |= 8; //bit length回復成8
	
  for(i=0;i<SEND_NUM;i++) //送一次讀回2bytes
	{
		*pBuffer = Rx_data_16[i];
		pBuffer++;
	}
	
	Rx = 0;
  Tx = 0;
  tmpflag = 0;

	//關掉FIFO功能
	QSPI_InitStructure.SPI_FIFO = SPI_FIFO_DISABLE;
	QSPI_InitStructure.SPI_RxFIFOTriggerLevel = 1;
  QSPI_InitStructure.SPI_TxFIFOTriggerLevel = 0;
	QSPI_InitStructure.SPI_DataLength = SPI_DATALENGTH_8;
	SPI_Init(QSPIx, &QSPI_InitStructure);	
	
  /* Deselect the FLASH: Chip Select high */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_INACTIVE);	
}

/*------------------------------------------------------------------------------------------------------------
  Function Name  : QSPI_FLASH_FIFO_QIOR
  Input          : None.
  Output         : None
  Return         : None
------------------------------------------------------------------------------------------------------------*/
void QSPI_FLASH_FIFO_QIOR(u16* pBuffer, u32 ReadAddr, u16 NumByteToRead)
{
	u16 i;
	u16 SendAddr_ModeBits;
	
	QSPIx->CR0 |= 0x10000;  // enable QDIOR
		
	/* Select the FLASH: Chip Select low */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_ACTIVE);
  
  /* Send "Read Memory with Dual Output" instruction */
  QSPI_FLASH_SendByte(QIOR);
  
	HT_QSPI->CR1 &= 0xfffffff0;
	HT_QSPI->CR1 |= 4;
	HT_QSPI->CR0 |= 0x20000;
	
	QSPI_FLASH_Send((ReadAddr & 0xFFFF00) >> 8); //addr bit 23~8
	
	SendAddr_ModeBits = ((ReadAddr & 0xFF) << 8) | (ReadAddr & 0xFF); //(addr bit 7~0 << 8) | (mode bits 7~0)
	QSPI_FLASH_Send(SendAddr_ModeBits);
	
	HT_QSPI->CR0 &= ~0x10000;
	
	QSPI_FLASH_Send(Dummy_Byte);
	
	//開FIFO傳送
	QSPI_InitStructure.SPI_FIFO = SPI_FIFO_ENABLE;
	QSPI_InitStructure.SPI_RxFIFOTriggerLevel = 3;
  QSPI_InitStructure.SPI_TxFIFOTriggerLevel = 3;
	QSPI_InitStructure.SPI_DataLength = SPI_DATALENGTH_4;
	SPI_Init(QSPIx, &QSPI_InitStructure);	
//	QSPIx->FCR = 0x444;
	Tx = 0;
	Rx = 0;
	SEND_NUM = (NumByteToRead+1)/2; //送一次4個clock，讀回1word=2bytes
	
	//initial TX FIFO
	for(i=0;i<SEND_NUM;i++)
	{
		Tx_data_16[i] = Dummy_Byte;
	}
	
	SPI_IntConfig(QSPIx, SPI_INT_RXBNE | SPI_INT_TXBE, ENABLE);
	
	while(tmpflag != 100);

	HT_QSPI->CR0 &= ~0x20000;
	HT_QSPI->CR1 &= 0xfffffff0;
	HT_QSPI->CR1 |= 8;
  
	for(i=0;i<SEND_NUM;i++)
	{
		*pBuffer = Rx_data_16[i];
		pBuffer++;
	}

	Rx = 0;
  Tx = 0;
  tmpflag = 0;
	
  //關掉FIFO功能
	QSPI_InitStructure.SPI_FIFO = SPI_FIFO_DISABLE;
	QSPI_InitStructure.SPI_RxFIFOTriggerLevel = 1;
  QSPI_InitStructure.SPI_TxFIFOTriggerLevel = 0;
	QSPI_InitStructure.SPI_DataLength = SPI_DATALENGTH_8;
	SPI_Init(QSPIx, &QSPI_InitStructure);	
	
  /* Deselect the FLASH: Chip Select high */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_INACTIVE);	
}

/*------------------------------------------------------------------------------------------------------------
  Function Name  : QSPI_FLASH_FIFO_ReadID
  Description    : Reads FLASH identification.
  Input          : None
  Output         : None
  Return         : FLASH identification
------------------------------------------------------------------------------------------------------------*/
u32 QSPI_FLASH_FIFO_ReadID(void)
{
  vu32 Temp;

  /* Select the FLASH: Chip Select low */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_ACTIVE);
  
  /* Send "READ ID " instruction */
  QSPI_FLASH_SendByte(ReadID);

  QSPI_FLASH_SendByte(Dummy_Byte);
  QSPI_FLASH_SendByte(Dummy_Byte);
  QSPI_FLASH_SendByte(0);
	
	//開FIFO傳送
	QSPI_InitStructure.SPI_FIFO = SPI_FIFO_ENABLE;
	QSPI_InitStructure.SPI_RxFIFOTriggerLevel = 3;
  QSPI_InitStructure.SPI_TxFIFOTriggerLevel = 3;
	SPI_Init(QSPIx, &QSPI_InitStructure);	
	Tx = 0;
	Rx = 0;
	SEND_NUM = 4; //--SEND_NUM = 8; //送8 byte
	Tx_data_16[0] = Dummy_Byte;
	Tx_data_16[1] = Dummy_Byte;
	Tx_data_16[2] = Dummy_Byte;
	Tx_data_16[3] = Dummy_Byte;
	//--Tx_Buffer_16[4] = Dummy_Byte;
	//--Tx_Buffer_16[5] = Dummy_Byte;
	//--Tx_Buffer_16[6] = Dummy_Byte;
	//--Tx_Buffer_16[7] = Dummy_Byte;
	
	SPI_IntConfig(QSPIx, SPI_INT_RXBNE | SPI_INT_TXBE, ENABLE);
	
	while(tmpflag != 100);
	
	Temp = (Rx_data_16[0]<< 24)|(Rx_data_16[1]<<16)|(Rx_data_16[2]<<8)|(Rx_data_16[3]);
	
	Rx = 0;
  Tx = 0;
  tmpflag = 0;

	//關掉FIFO功能
	QSPI_InitStructure.SPI_FIFO = SPI_FIFO_DISABLE;
	QSPI_InitStructure.SPI_RxFIFOTriggerLevel = 1;
  QSPI_InitStructure.SPI_TxFIFOTriggerLevel = 0;
	SPI_Init(QSPIx, &QSPI_InitStructure);	
	
	/* Deselect the FLASH: Chip Select high */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_INACTIVE);
	
	return Temp;
}

/*------------------------------------------------------------------------------------------------------------
  Function Name  : QSPI_FLASH_FIFO_RDID
  Description    : Reads FLASH identification.
  Input          : None
  Output         : None
  Return         : FLASH identification
------------------------------------------------------------------------------------------------------------*/
u32 QSPI_FLASH_FIFO_RDID(void)
{
  vu32 Temp = 0;

  /* Select the FLASH: Chip Select low */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_ACTIVE);
  
  /* Send "READ ID " instruction */
  QSPI_FLASH_SendByte(RDID);

	//開FIFO傳送
	QSPI_InitStructure.SPI_FIFO = SPI_FIFO_ENABLE;
	QSPI_InitStructure.SPI_RxFIFOTriggerLevel = 3;
  QSPI_InitStructure.SPI_TxFIFOTriggerLevel = 3;
	SPI_Init(QSPIx, &QSPI_InitStructure);	
	Tx = 0;
	Rx = 0;
	SEND_NUM = 4; //--SEND_NUM = 8; //送8 byte
	Tx_data_16[0] = Dummy_Byte;
	Tx_data_16[1] = Dummy_Byte;
	Tx_data_16[2] = Dummy_Byte;
	Tx_data_16[3] = Dummy_Byte;
	//--Tx_Buffer_16[4] = Dummy_Byte;
	//--Tx_Buffer_16[5] = Dummy_Byte;
	//--Tx_Buffer_16[6] = Dummy_Byte;
	//--Tx_Buffer_16[7] = Dummy_Byte;
	
	SPI_IntConfig(QSPIx, SPI_INT_RXBNE | SPI_INT_TXBE, ENABLE);
	
	while(tmpflag != 100);
	
	Temp = (Rx_data_16[0]<< 24)|(Rx_data_16[1]<<16)|(Rx_data_16[2]<<8)|(Rx_data_16[3]);
	
	Rx = 0;
  Tx = 0;
  tmpflag = 0;

	//關掉FIFO功能
	QSPI_InitStructure.SPI_FIFO = SPI_FIFO_DISABLE;
	QSPI_InitStructure.SPI_RxFIFOTriggerLevel = 1;
  QSPI_InitStructure.SPI_TxFIFOTriggerLevel = 0;
	SPI_Init(QSPIx, &QSPI_InitStructure);	
	
  /* Deselect the FLASH: Chip Select high */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_INACTIVE);

	return Temp;
}

/*------------------------------------------------------------------------------------------------------------
  Function Name  : QSPI_FLASH_FIFO_BufferRead
  Description    : Reads a block of data from the FLASH.
  Input          : - pBuffer : pointer to the buffer that receives the data read from the FLASH.
                   - ReadAddr : FLASH's internal address to read from.
                   - NumByteToRead : number of bytes to read from the FLASH.
  Output         : None
  Return         : None
------------------------------------------------------------------------------------------------------------*/
void QSPI_FLASH_FIFO_BufferRead(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead)
{
	u16 i;
	
  /* Select the FLASH: Chip Select low */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_ACTIVE);
  
  /* Send "Read from Memory " instruction */
  QSPI_FLASH_SendByte(READ);
  
  /* Send ReadAddr high nibble address byte to read from */
  QSPI_FLASH_SendByte((ReadAddr & 0xFF0000) >> 16);
  /* Send ReadAddr medium nibble address byte to read from */
  QSPI_FLASH_SendByte((ReadAddr& 0xFF00) >> 8);
  /* Send ReadAddr low nibble address byte to read from */
  QSPI_FLASH_SendByte(ReadAddr & 0xFF);

	//開FIFO傳送
	QSPI_InitStructure.SPI_FIFO = SPI_FIFO_ENABLE;
	QSPI_InitStructure.SPI_RxFIFOTriggerLevel = 3;
  QSPI_InitStructure.SPI_TxFIFOTriggerLevel = 3;
	SPI_Init(QSPIx, &QSPI_InitStructure);	
	Tx = 0;
	Rx = 0;
	SEND_NUM = NumByteToRead; //送的byte數
	
	//initial TX FIFO
	for(i=0;i<NumByteToRead;i++)
	{
		Tx_data_16[i] = Dummy_Byte;
	}
	
	SPI_IntConfig(QSPIx, SPI_INT_RXBNE | SPI_INT_TXBE, ENABLE);
	
	while(tmpflag != 100);
	
	for(i=0;i<NumByteToRead;i++)
	{
		*pBuffer = Rx_data_16[i];
		pBuffer++;
	}

	Rx = 0;
  Tx = 0;
  tmpflag = 0;

	//關掉FIFO功能
	QSPI_InitStructure.SPI_FIFO = SPI_FIFO_DISABLE;
	QSPI_InitStructure.SPI_RxFIFOTriggerLevel = 1;
  QSPI_InitStructure.SPI_TxFIFOTriggerLevel = 0;
	SPI_Init(QSPIx, &QSPI_InitStructure);	
	
  /* Deselect the FLASH: Chip Select high */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_INACTIVE);
}

/*------------------------------------------------------------------------------------------------------------
  Function Name  : QSPI_FLASH_FIFO_PageWrite
  Description    : Writes more than one byte to the FLASH with a single WRITE cycle(Page WRITE sequence). 
                   The number of byte can't exceed the FLASH page size.
  Input          : - pBuffer : pointer to the buffer  containing the data to be written to the FLASH.
                   - WriteAddr : FLASH's internal address to write to.
                   - NumByteToWrite : number of bytes to write to the FLASH, must be equal or less 
                     than "QSPI_FLASH_PageSize" value.
  Output         : None
  Return         : None
------------------------------------------------------------------------------------------------------------*/
void QSPI_FLASH_FIFO_PageWrite(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite)
{
	u16 i;
	
  /* Enable the write access to the FLASH */
  QSPI_FLASH_WriteEnable();
  
  /* Select the FLASH: Chip Select low */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_ACTIVE);
  /* Send "Write to Memory " instruction */
  QSPI_FLASH_SendByte(WRITE);
  /* Send WriteAddr high nibble address byte to write to */
  QSPI_FLASH_SendByte((WriteAddr & 0xFF0000) >> 16);
  /* Send WriteAddr medium nibble address byte to write to */
  QSPI_FLASH_SendByte((WriteAddr & 0xFF00) >> 8);  
  /* Send WriteAddr low nibble address byte to write to */
  QSPI_FLASH_SendByte(WriteAddr & 0xFF);
	
	//開FIFO傳送
	QSPI_InitStructure.SPI_FIFO = SPI_FIFO_ENABLE;
	QSPI_InitStructure.SPI_RxFIFOTriggerLevel = 3;
  QSPI_InitStructure.SPI_TxFIFOTriggerLevel = 3;
	SPI_Init(QSPIx, &QSPI_InitStructure);	
	Tx = 0;
	Rx = 0;
	SEND_NUM = NumByteToWrite; //送的byte數
	
	//initial TX FIFO
	for(i=0;i<NumByteToWrite;i++)
	{
		Tx_data_16[i] = *pBuffer;
		pBuffer++; 
	}
	
	SPI_IntConfig(QSPIx, SPI_INT_RXBNE | SPI_INT_TXBE, ENABLE);
	
	while(tmpflag != 100);

	Rx = 0;
  Tx = 0;
  tmpflag = 0;

	//關掉FIFO功能
	QSPI_InitStructure.SPI_FIFO = SPI_FIFO_DISABLE;
	QSPI_InitStructure.SPI_RxFIFOTriggerLevel = 1;
  QSPI_InitStructure.SPI_TxFIFOTriggerLevel = 0;
	SPI_Init(QSPIx, &QSPI_InitStructure);	
	
  /* Deselect the FLASH: Chip Select high */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_INACTIVE);
	
	/* Wait the end of Flash writing */
  QSPI_FLASH_WaitForWriteEnd();
}

/*------------------------------------------------------------------------------------------------------------
  Function Name  : QSPI_FLASH_FIFO_BufferFastRead
  Description    : Reads a block of data from the FLASH.
  Input          : - pBuffer : pointer to the buffer that receives the data read from the FLASH.
                   - ReadAddr : FLASH's internal address to read from.
                   - NumByteToRead : number of bytes to read from the FLASH.
  Output         : None
  Return         : None
------------------------------------------------------------------------------------------------------------*/
void QSPI_FLASH_FIFO_BufferFastRead(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead)
{
	u16 i;
	
  /* Select the FLASH: Chip Select low */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_ACTIVE);
  
  /* Send "Read from Memory " instruction */
  QSPI_FLASH_SendByte(F_READ);
  
  /* Send ReadAddr high nibble address byte to read from */
  QSPI_FLASH_SendByte((ReadAddr & 0xFF0000) >> 16);
  /* Send ReadAddr medium nibble address byte to read from */
  QSPI_FLASH_SendByte((ReadAddr& 0xFF00) >> 8);
  /* Send ReadAddr low nibble address byte to read from */
  QSPI_FLASH_SendByte(ReadAddr & 0xFF);

  QSPI_FLASH_SendByte(Dummy_Byte);
	
	//開FIFO傳送
	QSPI_InitStructure.SPI_FIFO = SPI_FIFO_ENABLE;
	QSPI_InitStructure.SPI_RxFIFOTriggerLevel = 3;
  QSPI_InitStructure.SPI_TxFIFOTriggerLevel = 3;
	SPI_Init(QSPIx, &QSPI_InitStructure);	
	Tx = 0;
	Rx = 0;
	SEND_NUM = NumByteToRead; //送的byte數
	
	//initial TX FIFO
	for(i=0;i<NumByteToRead;i++)
	{
		Tx_data_16[i] = Dummy_Byte;
	}
	
	SPI_IntConfig(QSPIx, SPI_INT_RXBNE | SPI_INT_TXBE, ENABLE);
	
	while(tmpflag != 100);
	
	for(i=0;i<NumByteToRead;i++)
	{
		*pBuffer = Rx_data_16[i];
		pBuffer++;
	}

	Rx = 0;
  Tx = 0;
  tmpflag = 0;

	//關掉FIFO功能
	QSPI_InitStructure.SPI_FIFO = SPI_FIFO_DISABLE;
	QSPI_InitStructure.SPI_RxFIFOTriggerLevel = 1;
  QSPI_InitStructure.SPI_TxFIFOTriggerLevel = 0;
	SPI_Init(QSPIx, &QSPI_InitStructure);	
	
  /* Deselect the FLASH: Chip Select high */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_INACTIVE);	
}

/*------------------------------------------------------------------------------------------------------------
  Function Name  : QSPI_FLASH_FIFO_DUAL_READ
  Input          : None.
  Output         : None
  Return         : None
------------------------------------------------------------------------------------------------------------*/
void QSPI_FLASH_FIFO_DUAL_READ(u16* pBuffer, u32 ReadAddr, u16 NumByteToRead)
{
	u16 i;
	
  /* Select the FLASH: Chip Select low */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_ACTIVE);
  
  /* Send "Read Memory with Dual Output" instruction */
  QSPI_FLASH_SendByte(FRDO);
  
  /* Send ReadAddr high nibble address byte to read from */
  QSPI_FLASH_SendByte((ReadAddr & 0xFF0000) >> 16);
  /* Send ReadAddr medium nibble address byte to read from */
  QSPI_FLASH_SendByte((ReadAddr& 0xFF00) >> 8);
  /* Send ReadAddr low nibble address byte to read from */
  QSPI_FLASH_SendByte(ReadAddr & 0xFF);

	NumByteToRead = (NumByteToRead+1)/2;
	
	/////////////////////////////////////////
	QSPIx->CR0 |= 0x40;  // enable QSPI dual
	/////////////////////////////////////////
	
	QSPI_FLASH_SendByte(Dummy_Byte);
	
	//開FIFO傳送
	QSPI_InitStructure.SPI_FIFO = SPI_FIFO_ENABLE;
	QSPI_InitStructure.SPI_RxFIFOTriggerLevel = 3;
  QSPI_InitStructure.SPI_TxFIFOTriggerLevel = 3;
	SPI_Init(QSPIx, &QSPI_InitStructure);	
	Tx = 0;
	Rx = 0;
	SEND_NUM = NumByteToRead; //送的byte數
	
	//initial TX FIFO
	for(i=0;i<NumByteToRead;i++)
	{
		Tx_data_16[i] = Dummy_Byte;
	}
	
	SPI_IntConfig(QSPIx, SPI_INT_RXBNE | SPI_INT_TXBE, ENABLE);
	
	while(tmpflag != 100);
	
	/////////////////////////////////////////
	QSPIx->CR0 &= ~0x40; // disable QSPI dual
	/////////////////////////////////////////
	
	for(i=0;i<NumByteToRead;i++)
	{
		*pBuffer = Rx_data_16[i];
		pBuffer++;
	}

	Rx = 0;
  Tx = 0;
  tmpflag = 0;

	//關掉FIFO功能
	QSPI_InitStructure.SPI_FIFO = SPI_FIFO_DISABLE;
	QSPI_InitStructure.SPI_RxFIFOTriggerLevel = 1;
  QSPI_InitStructure.SPI_TxFIFOTriggerLevel = 0;
	SPI_Init(QSPIx, &QSPI_InitStructure);	
	
  /* Deselect the FLASH: Chip Select high */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_INACTIVE);	
}

/*------------------------------------------------------------------------------------------------------------
  Function Name  : QSPI_FLASH_FIFO_DIOR
  Input          : None.
  Output         : None
  Return         : None
------------------------------------------------------------------------------------------------------------*/
void QSPI_FLASH_FIFO_DIOR(u16* pBuffer, u32 ReadAddr, u16 NumByteToRead)
{
	u16 i;
	
	QSPIx->CR0 |= 0x10000;  // enable QDIOR
		
	/* Select the FLASH: Chip Select low */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_ACTIVE);
  
  /* Send "Read Memory with Dual Output" instruction */
  QSPI_FLASH_SendByte(DIOR);
  
	QSPIx->CR0 |= 0x40;  // enable QSPI dual

	ReadAddr = (ReadAddr<<8)|(ReadAddr&0xff);
	QSPI_FLASH_Send((ReadAddr & 0xFFFF0000)>>16);
	
	QSPI_InitStructure.SPI_DataLength = 4;
	SPI_Init(QSPIx, &QSPI_InitStructure);	
	QSPI_FLASH_SendByte((ReadAddr & 0xFF00)>>8);
	
	QSPIx->CR0 &= ~0x10000;
	
	QSPI_FLASH_SendByte(0);
	
	//開FIFO傳送
	QSPI_InitStructure.SPI_FIFO = SPI_FIFO_ENABLE;
	QSPI_InitStructure.SPI_DataLength = 8;
	QSPI_InitStructure.SPI_RxFIFOTriggerLevel = 3;
  QSPI_InitStructure.SPI_TxFIFOTriggerLevel = 3;
	SPI_Init(QSPIx, &QSPI_InitStructure);	
	Tx = 0;
	Rx = 0;
	SEND_NUM = (NumByteToRead+1)/2; //送的byte數
	
	//initial TX FIFO
	for(i=0;i<NumByteToRead;i++)
	{
		Tx_data_16[i] = Dummy_Byte;
	}
	
	SPI_IntConfig(QSPIx, SPI_INT_RXBNE | SPI_INT_TXBE, ENABLE);
	
	while(tmpflag != 100);
	
	/////////////////////////////////////////
	QSPIx->CR0 &= ~0x40; // disable QSPI dual
	/////////////////////////////////////////
	
	for(i=0;i<SEND_NUM;i++)
	{
		*pBuffer = Rx_data_16[i];
		pBuffer++;
	}

	Rx = 0;
  Tx = 0;
  tmpflag = 0;

	//關掉FIFO功能
	QSPI_InitStructure.SPI_FIFO = SPI_FIFO_DISABLE;
	QSPI_InitStructure.SPI_RxFIFOTriggerLevel = 1;
  QSPI_InitStructure.SPI_TxFIFOTriggerLevel = 0;
	SPI_Init(QSPIx, &QSPI_InitStructure);	
	
  /* Deselect the FLASH: Chip Select high */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_INACTIVE);	
}

/*------------------------------------------------------------------------------------------------------------
  Function Name  : QSPI_FLASH_FIFO_QPP
  Input          : None.
  Output         : None
  Return         : None
------------------------------------------------------------------------------------------------------------*/
void QSPI_FLASH_FIFO_QPP(u16* pBuffer, u32 WriteAddr, u16 NumByteToWrite)
{
	u16 i;
	
	/* Enable the write access to the FLASH */
  QSPI_FLASH_WriteEnable();
  
  QSPIx->CR0 |= 0x10000;  // enable QDIOR
	
  /* Select the FLASH: Chip Select low */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_ACTIVE);
	
  /* Send instruction */
  QSPI_FLASH_SendByte(QPP);
	
  /* Send WriteAddr high nibble address byte to write to */
  QSPI_FLASH_SendByte((WriteAddr & 0xFF0000) >> 16);
  /* Send WriteAddr medium nibble address byte to write to */
  QSPI_FLASH_SendByte((WriteAddr & 0xFF00) >> 8);  
  /* Send WriteAddr low nibble address byte to write to */
  QSPI_FLASH_SendByte(WriteAddr & 0xFF);
  
	HT_QSPI->CR1 &= 0xfffffff0;
	HT_QSPI->CR1 |= 4;
	HT_QSPI->CR0 |= 0x20000;
	
	//開FIFO傳送
	QSPI_InitStructure.SPI_FIFO = SPI_FIFO_ENABLE;
	QSPI_InitStructure.SPI_RxFIFOTriggerLevel = 3;
  QSPI_InitStructure.SPI_TxFIFOTriggerLevel = 3;
	QSPI_InitStructure.SPI_DataLength = SPI_DATALENGTH_4;
	SPI_Init(QSPIx, &QSPI_InitStructure);	
	Tx = 0;
	Rx = 0;
	SEND_NUM = (NumByteToWrite+1)/2; //送的byte數
	
	//initial TX FIFO
	for(i=0;i<SEND_NUM;i++)
	{
		Tx_data_16[i] = *pBuffer;
		pBuffer++; 
	}
	
	SPI_IntConfig(QSPIx, SPI_INT_RXBNE | SPI_INT_TXBE, ENABLE);
	
	while(tmpflag != 100);

  Rx = 0;
  Tx = 0;
  tmpflag = 0;

	//關掉FIFO功能
	QSPI_InitStructure.SPI_FIFO = SPI_FIFO_DISABLE;
	QSPI_InitStructure.SPI_RxFIFOTriggerLevel = 1;
  QSPI_InitStructure.SPI_TxFIFOTriggerLevel = 0;
	QSPI_InitStructure.SPI_DataLength = SPI_DATALENGTH_8;
	SPI_Init(QSPIx, &QSPI_InitStructure);	
	
  /* Deselect the FLASH: Chip Select high */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_INACTIVE);
	
	HT_QSPI->CR0 &= ~0x30000;
	HT_QSPI->CR1 &= 0xfffffff0;
	HT_QSPI->CR1 |= 8;
  
  /* Wait the end of Flash writing */
  QSPI_FLASH_WaitForWriteEnd();
}

/*------------------------------------------------------------------------------------------------------------
  Function Name  : QSPI_FLASH_FIFO_BYTE_DUAL_READ
  Input          : None.
  Output         : None
  Return         : None
------------------------------------------------------------------------------------------------------------*/
void QSPI_FLASH_FIFO_BYTE_DUAL_READ(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead)
{
	u16 i;
	
  /* Select the FLASH: Chip Select low */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_ACTIVE);
  
  /* Send "Read Memory with Dual Output" instruction */
  QSPI_FLASH_SendByte(FRDO);
  
  /* Send ReadAddr high nibble address byte to read from */
  QSPI_FLASH_SendByte((ReadAddr & 0xFF0000) >> 16);
  /* Send ReadAddr medium nibble address byte to read from */
  QSPI_FLASH_SendByte((ReadAddr& 0xFF00) >> 8);
  /* Send ReadAddr low nibble address byte to read from */
  QSPI_FLASH_SendByte(ReadAddr & 0xFF);
	
	/////////////////////////////////////////
	QSPIx->CR0 |= 0x40;  // enable QSPI dual
	/////////////////////////////////////////
	
	QSPI_FLASH_SendByte(Dummy_Byte);
	
	//開FIFO傳送
	QSPI_InitStructure.SPI_FIFO = SPI_FIFO_ENABLE;
	QSPI_InitStructure.SPI_RxFIFOTriggerLevel = 3;
  QSPI_InitStructure.SPI_TxFIFOTriggerLevel = 3;
	QSPI_InitStructure.SPI_DataLength = SPI_DATALENGTH_4; //dual mode下，一次送4個clock => 相當於1個byte
	SPI_Init(QSPIx, &QSPI_InitStructure);	
	Tx = 0;
	Rx = 0;
	SEND_NUM = NumByteToRead; //送的byte數
	
	//initial TX FIFO
	for(i=0;i<NumByteToRead;i++)
	{
		Tx_data_16[i] = Dummy_Byte;
	}
	
	SPI_IntConfig(QSPIx, SPI_INT_RXBNE | SPI_INT_TXBE, ENABLE);
	
	while(tmpflag != 100);
	
	/////////////////////////////////////////
	QSPIx->CR0 &= ~0x40; // disable QSPI dual
	/////////////////////////////////////////
	
	for(i=0;i<NumByteToRead;i++)
	{
		*pBuffer = Rx_data_16[i];
		pBuffer++;
	}

	Rx = 0;
  Tx = 0;
  tmpflag = 0;

	//關掉FIFO功能
	QSPI_InitStructure.SPI_FIFO = SPI_FIFO_DISABLE;
	QSPI_InitStructure.SPI_RxFIFOTriggerLevel = 1;
  QSPI_InitStructure.SPI_TxFIFOTriggerLevel = 0;
	QSPI_InitStructure.SPI_DataLength = SPI_DATALENGTH_8;
	SPI_Init(QSPIx, &QSPI_InitStructure);	
	
  /* Deselect the FLASH: Chip Select high */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_INACTIVE);	
}

/*------------------------------------------------------------------------------------------------------------
  Function Name  : QSPI_FLASH_FIFO_BYTE_DIOR
  Input          : None.
  Output         : None
  Return         : None
------------------------------------------------------------------------------------------------------------*/
void QSPI_FLASH_FIFO_BYTE_DIOR(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead)
{
	u16 i;
	
	QSPIx->CR0 |= 0x10000;  // enable QDIOR
		
	/* Select the FLASH: Chip Select low */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_ACTIVE);
  
  /* Send "Read Memory with Dual Output" instruction */
  QSPI_FLASH_SendByte(DIOR);
  
	QSPIx->CR0 |= 0x40;  // enable QSPI dual
	
	QSPI_InitStructure.SPI_DataLength = SPI_DATALENGTH_4; //dual mode下，一次送4個clock => 相當於1個byte
	SPI_Init(QSPIx, &QSPI_InitStructure);	

//	ReadAddr = (ReadAddr<<8)|(ReadAddr&0xff);
//	QSPI_FLASH_Send((ReadAddr & 0xFFFF0000)>>16);
//	QSPI_FLASH_Send(ReadAddr & 0xFFFF);
	
	QSPI_FLASH_SendByte((ReadAddr & 0xFF0000)>>16);
	QSPI_FLASH_SendByte((ReadAddr & 0xFF00)>>8);
	QSPI_FLASH_SendByte(ReadAddr & 0xFF);
	QSPI_FLASH_SendByte(ReadAddr & 0xFF); //mode bits
	
	QSPIx->CR0 &= ~0x10000;
	
	//開FIFO傳送
	QSPI_InitStructure.SPI_FIFO = SPI_FIFO_ENABLE;
	QSPI_InitStructure.SPI_RxFIFOTriggerLevel = 3;
  QSPI_InitStructure.SPI_TxFIFOTriggerLevel = 3;
	SPI_Init(QSPIx, &QSPI_InitStructure);	
	Tx = 0;
	Rx = 0;
	SEND_NUM = NumByteToRead; //送的byte數
	
	//initial TX FIFO
	for(i=0;i<NumByteToRead;i++)
	{
		Tx_data_16[i] = Dummy_Byte;
	}
	
	SPI_IntConfig(QSPIx, SPI_INT_RXBNE | SPI_INT_TXBE, ENABLE);
	
	while(tmpflag != 100);
	
	/////////////////////////////////////////
	QSPIx->CR0 &= ~0x40; // disable QSPI dual
	/////////////////////////////////////////
	
	for(i=0;i<SEND_NUM;i++)
	{
		*pBuffer = Rx_data_16[i];
		pBuffer++;
	}

	Rx = 0;
  Tx = 0;
  tmpflag = 0;

	//關掉FIFO功能
	QSPI_InitStructure.SPI_FIFO = SPI_FIFO_DISABLE;
	QSPI_InitStructure.SPI_RxFIFOTriggerLevel = 1;
  QSPI_InitStructure.SPI_TxFIFOTriggerLevel = 0;
	QSPI_InitStructure.SPI_DataLength = SPI_DATALENGTH_8;
	SPI_Init(QSPIx, &QSPI_InitStructure);	
	
  /* Deselect the FLASH: Chip Select high */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_INACTIVE);	
}

/*------------------------------------------------------------------------------------------------------------
  Function Name  : QSPI_FLASH_BYTE_QIOR
  Input          : None.
  Output         : None
  Return         : None
------------------------------------------------------------------------------------------------------------*/
void QSPI_FLASH_BYTE_QIOR(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead)
{
	QSPIx->CR0 |= 0x10000;  // enable QDIOR
		
	/* Select the FLASH: Chip Select low */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_ACTIVE);
  
	/* Send "Read Memory with Dual Output" instruction */
  QSPI_FLASH_SendByte(QIOR);
	
	HT_QSPI->CR1 &= 0xfffffff0;
	HT_QSPI->CR1 |= 2; //DFL=2, 一次送一個byte
	HT_QSPI->CR0 |= 0x20000;
	
//	QSPI_FLASH_Send((ReadAddr & 0xFFFF00) >> 8); //addr bit 23~8
//	
//	SendAddr_ModeBits = ((ReadAddr & 0xFF) << 8) | (ReadAddr & 0xFF); //(addr bit 7~0 << 8) | (mode bits 7~0)
//	QSPI_FLASH_Send(SendAddr_ModeBits);
//	
//	QSPI_FLASH_Send(Dummy_Byte);
	
	QSPI_FLASH_SendByte((ReadAddr & 0xFF0000) >> 16);
	QSPI_FLASH_SendByte((ReadAddr & 0xFF00) >> 8);
	QSPI_FLASH_SendByte(ReadAddr & 0xFF);
	QSPI_FLASH_SendByte(ReadAddr & 0xFF); //mode bits
	
	HT_QSPI->CR0 &= ~0x10000;
	QSPI_FLASH_SendByte(Dummy_Byte);
	QSPI_FLASH_SendByte(Dummy_Byte);

  while(NumByteToRead--) /* while there is data to be read */
  {
    /* Read a byte from the FLASH */
    *pBuffer = QSPI_FLASH_SendByte(Dummy_Byte);
    /* Point to the next location where the byte read will be saved */
    pBuffer++;
  }
  
  /* Deselect the FLASH: Chip Select high */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_INACTIVE);
	
	HT_QSPI->CR0 &= ~0x20000;
	HT_QSPI->CR1 &= 0xfffffff0;
	HT_QSPI->CR1 |= 8;
}

/*------------------------------------------------------------------------------------------------------------
  Function Name  : QSPI_FLASH_BYTE_DIOR
  Input          : None.
  Output         : None
  Return         : None
------------------------------------------------------------------------------------------------------------*/
void QSPI_FLASH_BYTE_DIOR(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead)
{
	QSPIx->CR0 |= 0x10000;  // enable QDIOR
		
	/* Select the FLASH: Chip Select low */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_ACTIVE);
  
  /* Send "Read Memory with Dual Output" instruction */
  QSPI_FLASH_SendByte(DIOR);
  
	QSPIx->CR0 |= 0x40;  // enable QSPI dual
	
	HT_QSPI->CR1 &= 0xfffffff0;
	HT_QSPI->CR1 |= 4; //DFL=4, 一次送一個byte
	
//	QSPI_FLASH_Send((ReadAddr & 0xFFFF0000) >> 16);
//	QSPI_FLASH_Send(ReadAddr & 0xFFFF);
	
	QSPI_FLASH_SendByte((ReadAddr & 0xFF0000) >> 16);
	QSPI_FLASH_SendByte((ReadAddr & 0xFF00) >> 8);
	QSPI_FLASH_SendByte(ReadAddr & 0xFF);
	
	QSPIx->CR0 &= ~0x10000;
	
	QSPI_FLASH_SendByte(ReadAddr & 0xFF); //mode bits
	
  while(NumByteToRead--) /* while there is data to be read */
  {
    /* Read 2 bytes from the FLASH */
    *pBuffer = QSPI_FLASH_SendByte(Dummy_Byte);
    /* Point to the next location where the byte read will be saved */
    pBuffer++;
  }
  
  /* Deselect the FLASH: Chip Select high */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_INACTIVE);
	
	/////////////////////////////////////////
	QSPIx->CR0 &= ~0x40; // disable QSPI dual
	
	HT_QSPI->CR1 &= 0xfffffff0;
	HT_QSPI->CR1 |= 8;
	/////////////////////////////////////////
}

/*------------------------------------------------------------------------------------------------------------
  Function Name  : QSPI_FLASH_FIFO_BYTE_QPP
  Input          : None.
  Output         : None
  Return         : None
------------------------------------------------------------------------------------------------------------*/
void QSPI_FLASH_FIFO_BYTE_QPP(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite)
{
	u16 i;
	
	/* Enable the write access to the FLASH */
  QSPI_FLASH_WriteEnable();
  
  QSPIx->CR0 |= 0x10000;  // enable QDIOR
	
  /* Select the FLASH: Chip Select low */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_ACTIVE);
	
  /* Send instruction */
  QSPI_FLASH_SendByte(QPP);
	
  /* Send WriteAddr high nibble address byte to write to */
  QSPI_FLASH_SendByte((WriteAddr & 0xFF0000) >> 16);
  /* Send WriteAddr medium nibble address byte to write to */
  QSPI_FLASH_SendByte((WriteAddr & 0xFF00) >> 8);  
  /* Send WriteAddr low nibble address byte to write to */
  QSPI_FLASH_SendByte(WriteAddr & 0xFF);
  
//	HT_QSPI->CR1 &= 0xfffffff0;
//	HT_QSPI->CR1 |= 2; //一次送一個byte
	HT_QSPI->CR0 |= 0x20000;
	
	//開FIFO傳送
	QSPI_InitStructure.SPI_FIFO = SPI_FIFO_ENABLE;
	QSPI_InitStructure.SPI_RxFIFOTriggerLevel = 3;
  QSPI_InitStructure.SPI_TxFIFOTriggerLevel = 3;
	QSPI_InitStructure.SPI_DataLength = SPI_DATALENGTH_2; //一次送一個byte
	SPI_Init(QSPIx, &QSPI_InitStructure);	
	Tx = 0;
	Rx = 0;
	SEND_NUM = NumByteToWrite; //送的byte數
	
	//initial TX FIFO
	for(i=0;i<SEND_NUM;i++)
	{
		Tx_data_16[i] = *pBuffer;
		pBuffer++; 
	}
	
	SPI_IntConfig(QSPIx, SPI_INT_RXBNE | SPI_INT_TXBE, ENABLE);
	
	while(tmpflag != 100);

  Rx = 0;
  Tx = 0;
  tmpflag = 0;

	//關掉FIFO功能
	QSPI_InitStructure.SPI_FIFO = SPI_FIFO_DISABLE;
	QSPI_InitStructure.SPI_RxFIFOTriggerLevel = 1;
  QSPI_InitStructure.SPI_TxFIFOTriggerLevel = 0;
	QSPI_InitStructure.SPI_DataLength = SPI_DATALENGTH_8;
	SPI_Init(QSPIx, &QSPI_InitStructure);	
	
  /* Deselect the FLASH: Chip Select high */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_INACTIVE);
	
	HT_QSPI->CR0 &= ~0x30000;
//	HT_QSPI->CR1 &= 0xfffffff0;
//	HT_QSPI->CR1 |= 8;
  
  /* Wait the end of Flash writing */
  QSPI_FLASH_WaitForWriteEnd();
}

/*------------------------------------------------------------------------------------------------------------
  Function Name  : QSPI_FLASH_FIFO_BYTE_QIOR
  Input          : None.
  Output         : None
  Return         : None
------------------------------------------------------------------------------------------------------------*/
void QSPI_FLASH_FIFO_BYTE_QIOR(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead)
{
	u16 i;
	
	QSPIx->CR0 |= 0x10000;  // enable QDIOR
	
	/* Select the FLASH: Chip Select low */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_ACTIVE);
  
  /* Send "Read Memory with Dual Output" instruction */
  QSPI_FLASH_SendByte(QIOR);
  
	HT_QSPI->CR1 &= 0xfffffff0;
	HT_QSPI->CR1 |= 2; //一次送一個byte
	HT_QSPI->CR0 |= 0x20000;
	
//	QSPI_FLASH_Send((ReadAddr & 0xFFFF00) >> 8); //addr bit 23~8
//	
//	SendAddr_ModeBits = ((ReadAddr & 0xFF) << 8) | (ReadAddr & 0xFF); //(addr bit 7~0 << 8) | (mode bits 7~0)
//	QSPI_FLASH_Send(SendAddr_ModeBits);
//	
//	QSPI_FLASH_Send(Dummy_Byte);

	QSPI_FLASH_SendByte((ReadAddr & 0xFF0000) >> 16);
	QSPI_FLASH_SendByte((ReadAddr & 0xFF00) >> 8);
	QSPI_FLASH_SendByte(ReadAddr & 0xFF);
	QSPI_FLASH_SendByte(ReadAddr & 0xFF); //mode bits
	HT_QSPI->CR0 &= ~0x10000;
	QSPI_FLASH_SendByte(Dummy_Byte);
	QSPI_FLASH_SendByte(Dummy_Byte);

	
	//開FIFO傳送
	QSPI_InitStructure.SPI_FIFO = SPI_FIFO_ENABLE;
	QSPI_InitStructure.SPI_RxFIFOTriggerLevel = 3;
  QSPI_InitStructure.SPI_TxFIFOTriggerLevel = 3;
	QSPI_InitStructure.SPI_DataLength = SPI_DATALENGTH_2; //一次送一個byte
	SPI_Init(QSPIx, &QSPI_InitStructure);	
//	QSPIx->FCR = 0x444;
	Tx = 0;
	Rx = 0;
	SEND_NUM = NumByteToRead; //送一次4個clock，讀回1word=2bytes
	
	//initial TX FIFO
	for(i=0;i<SEND_NUM;i++)
	{
		Tx_data_16[i] = Dummy_Byte;
	}
	
	SPI_IntConfig(QSPIx, SPI_INT_RXBNE | SPI_INT_TXBE, ENABLE);
	
	while(tmpflag != 100);

	HT_QSPI->CR0 &= ~0x20000;
//	HT_QSPI->CR1 &= 0xfffffff0;
//	HT_QSPI->CR1 |= 8;
  
	for(i=0;i<SEND_NUM;i++)
	{
		*pBuffer = Rx_data_16[i];
		pBuffer++;
	}

	Rx = 0;
  Tx = 0;
  tmpflag = 0;
	
  //關掉FIFO功能
	QSPI_InitStructure.SPI_FIFO = SPI_FIFO_DISABLE;
	QSPI_InitStructure.SPI_RxFIFOTriggerLevel = 1;
  QSPI_InitStructure.SPI_TxFIFOTriggerLevel = 0;
	QSPI_InitStructure.SPI_DataLength = SPI_DATALENGTH_8;
	SPI_Init(QSPIx, &QSPI_InitStructure);	
	
  /* Deselect the FLASH: Chip Select high */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_INACTIVE);	
}

/*------------------------------------------------------------------------------------------------------------
  Function Name  : QSPI_FLASH_BYTE_QPP
  Input          : None.
  Output         : None
  Return         : None
------------------------------------------------------------------------------------------------------------*/
void QSPI_FLASH_BYTE_QPP(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite)
{
	/* Enable the write access to the FLASH */
  QSPI_FLASH_WriteEnable();
  
  QSPIx->CR0 |= 0x10000;  // enable QDIOR
	
  /* Select the FLASH: Chip Select low */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_ACTIVE);
	
  /* Send instruction */
  QSPI_FLASH_SendByte(QPP);
	
  /* Send WriteAddr high nibble address byte to write to */
  QSPI_FLASH_SendByte((WriteAddr & 0xFF0000) >> 16);
  /* Send WriteAddr medium nibble address byte to write to */
  QSPI_FLASH_SendByte((WriteAddr & 0xFF00) >> 8);  
  /* Send WriteAddr low nibble address byte to write to */
  QSPI_FLASH_SendByte(WriteAddr & 0xFF);
  
	HT_QSPI->CR1 &= 0xfffffff0;
	HT_QSPI->CR1 |= 2; //一次送一個byte
	HT_QSPI->CR0 |= 0x20000;
	
  /* while there is data to be written on the FLASH */
  while(NumByteToWrite--) 
  {
    /* Send the current word */
    QSPI_FLASH_SendByte(*pBuffer); //一次打2個clock，送 8 bits = 1 byte
    /* Point on the next byte to be written */
    pBuffer++; 
  }
  
  /* Deselect the FLASH: Chip Select high */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_INACTIVE);
	
	HT_QSPI->CR0 &= ~0x30000;
	HT_QSPI->CR1 &= 0xfffffff0;
	HT_QSPI->CR1 |= 8;
  
  /* Wait the end of Flash writing */
  QSPI_FLASH_WaitForWriteEnd();
}

/*------------------------------------------------------------------------------------------------------------
  Function Name  : QSPI_FLASH_BYTE_DUAL_READ
  Input          : None.
  Output         : None
  Return         : None
------------------------------------------------------------------------------------------------------------*/
void QSPI_FLASH_BYTE_DUAL_READ(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead)
{
  /* Select the FLASH: Chip Select low */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_ACTIVE);
  
  /* Send "Read Memory with Dual Output" instruction */
  QSPI_FLASH_SendByte(FRDO);
  
  /* Send ReadAddr high nibble address byte to read from */
  QSPI_FLASH_SendByte((ReadAddr & 0xFF0000) >> 16);
  /* Send ReadAddr medium nibble address byte to read from */
  QSPI_FLASH_SendByte((ReadAddr& 0xFF00) >> 8);
  /* Send ReadAddr low nibble address byte to read from */
  QSPI_FLASH_SendByte(ReadAddr & 0xFF);
	
	/////////////////////////////////////////
	QSPIx->CR0 |= 0x40;  // enable QSPI dual
	
	QSPI_FLASH_SendByte(Dummy_Byte);
	
	HT_QSPI->CR1 &= 0xfffffff0;
	HT_QSPI->CR1 |= 4; //DFL=4, 一次送一個byte
	/////////////////////////////////////////
	
  while(NumByteToRead--) /* while there is data to be read */
  {
    /* Read 2 bytes from the FLASH */
    *pBuffer = QSPI_FLASH_SendByte(Dummy_Byte);
    /* Point to the next location where the byte read will be saved */
    pBuffer++;
  }
  
  /* Deselect the FLASH: Chip Select high */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_INACTIVE);
	
	/////////////////////////////////////////
	QSPIx->CR0 &= ~0x40; // disable QSPI dual
	
	HT_QSPI->CR1 &= 0xfffffff0;
	HT_QSPI->CR1 |= 8;
	/////////////////////////////////////////
}

/*------------------------------------------------------------------------------------------------------------
  Function Name  : QSPI_FLASH_BYTE_QOR
  Input          : None.
  Output         : None
  Return         : None
------------------------------------------------------------------------------------------------------------*/
void QSPI_FLASH_BYTE_QOR(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead)
{
	/* Select the FLASH: Chip Select low */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_ACTIVE);
  
  /* Send instruction */
  QSPI_FLASH_SendByte(QOR);
  
  /* Send ReadAddr high nibble address byte to read from */
  QSPI_FLASH_SendByte((ReadAddr & 0xFF0000) >> 16);
  /* Send ReadAddr medium nibble address byte to read from */
  QSPI_FLASH_SendByte((ReadAddr& 0xFF00) >> 8);
  /* Send ReadAddr low nibble address byte to read from */
  QSPI_FLASH_SendByte(ReadAddr & 0xFF);
	
	HT_QSPI->CR1 &= 0xfffffff0;
	HT_QSPI->CR1 |= 2;
	HT_QSPI->CR0 |= 0x20000;
	
	QSPI_FLASH_SendByte(Dummy_Byte);
	QSPI_FLASH_SendByte(Dummy_Byte);
	QSPI_FLASH_SendByte(Dummy_Byte);
	QSPI_FLASH_SendByte(Dummy_Byte);
	
  while(NumByteToRead--) /* while there is data to be read */
  {
    /* Read 2 bytes from the FLASH */
    *pBuffer = QSPI_FLASH_SendByte(Dummy_Byte);
    /* Point to the next location where the byte read will be saved */
    pBuffer++;
  }
  
  /* Deselect the FLASH: Chip Select high */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_INACTIVE);
	
	HT_QSPI->CR0 &= ~0x20000;
	HT_QSPI->CR1 &= 0xfffffff0;
	HT_QSPI->CR1 |= 8;
}

/*------------------------------------------------------------------------------------------------------------
  Function Name  : QSPI_FLASH_FIFO_BYTE_QOR
  Input          : None.
  Output         : None
  Return         : None
------------------------------------------------------------------------------------------------------------*/
void QSPI_FLASH_FIFO_BYTE_QOR(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead)
{
	u16 i;
	
	/* Select the FLASH: Chip Select low */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_ACTIVE);
  
  /* Send instruction */
  QSPI_FLASH_SendByte(QOR);
  
  /* Send ReadAddr high nibble address byte to read from */
  QSPI_FLASH_SendByte((ReadAddr & 0xFF0000) >> 16);
  /* Send ReadAddr medium nibble address byte to read from */
  QSPI_FLASH_SendByte((ReadAddr& 0xFF00) >> 8);
  /* Send ReadAddr low nibble address byte to read from */
  QSPI_FLASH_SendByte(ReadAddr & 0xFF);
	
//	HT_QSPI->CR1 &= 0xfffffff0;
//	HT_QSPI->CR1 |= 2; //一次送一個byte
	HT_QSPI->CR0 |= 0x20000;

	QSPI_FLASH_SendByte(Dummy_Byte);
	
	//開FIFO傳送
	QSPI_InitStructure.SPI_FIFO = SPI_FIFO_ENABLE;
	QSPI_InitStructure.SPI_RxFIFOTriggerLevel = 3;
  QSPI_InitStructure.SPI_TxFIFOTriggerLevel = 3;
	QSPI_InitStructure.SPI_DataLength = SPI_DATALENGTH_2;
	SPI_Init(QSPIx, &QSPI_InitStructure);	
//	QSPIx->FCR = 0x444;
	Tx = 0;
	Rx = 0;

	SEND_NUM = NumByteToRead; //送的byte數
	
	//initial TX FIFO
	for(i=0;i<NumByteToRead;i++)
	{
		Tx_data_16[i] = Dummy_Byte;
	}
	
	SPI_IntConfig(QSPIx, SPI_INT_RXBNE | SPI_INT_TXBE, ENABLE);
	
	while(tmpflag != 100);
	
	HT_QSPI->CR0 &= ~0x20000;
//	HT_QSPI->CR1 &= 0xfffffff0;
//	HT_QSPI->CR1 |= 8; //bit length回復成8
	
  for(i=0;i<SEND_NUM;i++) //送一次讀回1byte
	{
		*pBuffer = Rx_data_16[i];
		pBuffer++;
	}
	
	Rx = 0;
  Tx = 0;
  tmpflag = 0;

	//關掉FIFO功能
	QSPI_InitStructure.SPI_FIFO = SPI_FIFO_DISABLE;
	QSPI_InitStructure.SPI_RxFIFOTriggerLevel = 1;
  QSPI_InitStructure.SPI_TxFIFOTriggerLevel = 0;
	QSPI_InitStructure.SPI_DataLength = SPI_DATALENGTH_8;
	SPI_Init(QSPIx, &QSPI_InitStructure);	
	
  /* Deselect the FLASH: Chip Select high */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_INACTIVE);	
}

/*------------------------------------------------------------------------------------------------------------
  Function Name  : QSPI_FLASH_BYTE_QIOR_QPI
  Input          : None.
  Output         : None
  Return         : None
------------------------------------------------------------------------------------------------------------*/
void QSPI_FLASH_BYTE_QIOR_QPI(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead)
{
	QSPIx->CR0 |= 0x10000;  // enable QDIOR
		
	/* Select the FLASH: Chip Select low */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_ACTIVE);
	
	HT_QSPI->CR1 &= 0xfffffff0;
	HT_QSPI->CR1 |= 2; //DFL=2, 一次送一個byte
	HT_QSPI->CR0 |= 0x20000;
	
	/* Send "Read Memory with Dual Output" instruction */
  QSPI_FLASH_SendByte(QIOR);
	
//	QSPI_FLASH_Send((ReadAddr & 0xFFFF00) >> 8); //addr bit 23~8
//	
//	SendAddr_ModeBits = ((ReadAddr & 0xFF) << 8) | (ReadAddr & 0xFF); //(addr bit 7~0 << 8) | (mode bits 7~0)
//	QSPI_FLASH_Send(SendAddr_ModeBits);
//	
//	QSPI_FLASH_Send(Dummy_Byte);
	
	QSPI_FLASH_SendByte((ReadAddr & 0xFF0000) >> 16);
	QSPI_FLASH_SendByte((ReadAddr & 0xFF00) >> 8);
	QSPI_FLASH_SendByte(ReadAddr & 0xFF);
	
	QSPI_FLASH_SendByte(Dummy_Byte); //MODE
	
	HT_QSPI->CR0 &= ~0x10000;
	
	QSPI_FLASH_SendByte(Dummy_Byte); //DUMMY
	QSPI_FLASH_SendByte(Dummy_Byte); //DUMMY
	
  while(NumByteToRead--) /* while there is data to be read */
  {
    /* Read a byte from the FLASH */
    *pBuffer = QSPI_FLASH_SendByte(Dummy_Byte);
    /* Point to the next location where the byte read will be saved */
    pBuffer++;
  }
  
  /* Deselect the FLASH: Chip Select high */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_INACTIVE);
	
	HT_QSPI->CR0 &= ~0x20000;
	HT_QSPI->CR1 &= 0xfffffff0;
	HT_QSPI->CR1 |= 8;
}

/*------------------------------------------------------------------------------------------------------------
  Function Name  : QSPI_FLASH_EQPI
  Description    : Enables the write access to the FLASH.
  Input          : None.
  Output         : None
  Return         : None
------------------------------------------------------------------------------------------------------------*/
void QSPI_FLASH_EQPI(void)
{
  /* Select the FLASH: Chip Select low */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_ACTIVE);
  
  /* Send "Write Enable" instruction */
  QSPI_FLASH_SendByte(EQPI);
  
  /* Deselect the FLASH: Chip Select high */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_INACTIVE);
}

/*------------------------------------------------------------------------------------------------------------
  Function Name  : QSPI_FLASH_RSTQPI
  Description    : Enables the write access to the FLASH.
  Input          : None.
  Output         : None
  Return         : None
------------------------------------------------------------------------------------------------------------*/
void QSPI_FLASH_RSTQPI(void)
{
	HT_QSPI->CR1 &= 0xfffffff0;
	HT_QSPI->CR1 |= 2; //DFL=2, 一次送一個byte
	HT_QSPI->CR0 |= 0x30000;
	
  /* Select the FLASH: Chip Select low */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_ACTIVE);
  
  /* Send "Write Enable" instruction */
  QSPI_FLASH_SendByte(RSTQPI);
  
  /* Deselect the FLASH: Chip Select high */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_INACTIVE);
	
	HT_QSPI->CR0 &= ~0x30000;
	HT_QSPI->CR1 &= 0xfffffff0;
	HT_QSPI->CR1 |= 8;
}

/*------------------------------------------------------------------------------------------------------------
  Function Name  : QSPI_FLASH_FIFO_BYTE_QIOR_QPI
  Input          : None.
  Output         : None
  Return         : None
------------------------------------------------------------------------------------------------------------*/
void QSPI_FLASH_FIFO_BYTE_QIOR_QPI(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead)
{
u16 i;
	
	QSPIx->CR0 |= 0x10000;  // enable QDIOR
		
	/* Select the FLASH: Chip Select low */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_ACTIVE);
  
	HT_QSPI->CR1 &= 0xfffffff0;
	HT_QSPI->CR1 |= 2; //一次送一個byte
	HT_QSPI->CR0 |= 0x20000;
	
	/* Send "Read Memory with Dual Output" instruction */
  QSPI_FLASH_SendByte(QIOR);

	QSPI_FLASH_SendByte((ReadAddr & 0xFF0000) >> 16);
	QSPI_FLASH_SendByte((ReadAddr & 0xFF00) >> 8);
	QSPI_FLASH_SendByte(ReadAddr & 0xFF);
	
	QSPI_FLASH_SendByte(Dummy_Byte); //MODE
	
	HT_QSPI->CR0 &= ~0x10000;
	
	QSPI_FLASH_SendByte(Dummy_Byte); //DUMMY
	QSPI_FLASH_SendByte(Dummy_Byte); //DUMMY
	
	//開FIFO傳送
	QSPI_InitStructure.SPI_FIFO = SPI_FIFO_ENABLE;
	QSPI_InitStructure.SPI_RxFIFOTriggerLevel = 3;
  QSPI_InitStructure.SPI_TxFIFOTriggerLevel = 3;
	QSPI_InitStructure.SPI_DataLength = SPI_DATALENGTH_2; //一次送一個byte
	SPI_Init(QSPIx, &QSPI_InitStructure);	
//	QSPIx->FCR = 0x444;
	Tx = 0;
	Rx = 0;
	SEND_NUM = NumByteToRead; //送一次4個clock，讀回1word=2bytes
	
	//initial TX FIFO
	for(i=0;i<SEND_NUM;i++)
	{
		Tx_data_16[i] = Dummy_Byte;
	}
	
	SPI_IntConfig(QSPIx, SPI_INT_RXBNE | SPI_INT_TXBE, ENABLE);
	
	while(tmpflag != 100);

	HT_QSPI->CR0 &= ~0x20000;
//	HT_QSPI->CR1 &= 0xfffffff0;
//	HT_QSPI->CR1 |= 8;
  
	for(i=0;i<SEND_NUM;i++)
	{
		*pBuffer = Rx_data_16[i];
		pBuffer++;
	}

	Rx = 0;
  Tx = 0;
  tmpflag = 0;
	
  //關掉FIFO功能
	QSPI_InitStructure.SPI_FIFO = SPI_FIFO_DISABLE;
	QSPI_InitStructure.SPI_RxFIFOTriggerLevel = 1;
  QSPI_InitStructure.SPI_TxFIFOTriggerLevel = 0;
	QSPI_InitStructure.SPI_DataLength = SPI_DATALENGTH_8;
	SPI_Init(QSPIx, &QSPI_InitStructure);	
	
  /* Deselect the FLASH: Chip Select high */
  QSPI_SoftwareSELCmd(QSPIx, SPI_SEL_INACTIVE);	
	
}
