
#include "QSPI_flash.h"
#include "QSPI_MIDICTRL.h"

void SPANSION_FLASH_ReadID()
{
}

void SPANSION_FLASH_DOR()
{
}

void SPANSION_FLASH_DIOR()
{
}

void SPANSION_FLASH_QOR()
{
	//configure register: QUAD bit set 1
	QSPI_FLASH_WRR(0x2, 0x2); //WEL=1, QUAD=1
}

void SPANSION_FLASH_QIOR()
{
	//configure register: QUAD bit set 1
	QSPI_FLASH_WRR(0x2, 0x2); //WEL=1, QUAD=1
}

void MXIC_FLASH_DOR()
{
}

void MXIC_FLASH_DIOR()
{
}

void MXIC_FLASH_QOR()
{
	//configure register: QUAD bit set 1
	QSPI_FLASH_WRR(0x42, 0x7); //WEL=1, QUAD=1
	//--0QSPI_FLASH_EQPI();	//enter QPI mode (EQIO)
}

void MXIC_FLASH_QIOR()
{
	//configure register: QUAD bit set 1
	QSPI_FLASH_WRR(0x42, 0x7); //WEL=1, QUAD=1
	//--0QSPI_FLASH_EQPI();	//enter QPI mode (EQIO)
}

void MXIC_FLASH_QPI()
{
	//configure register: QUAD bit set 1
	QSPI_FLASH_WRR(0x42, 0x7); //WEL=1, QUAD=1
	QSPI_FLASH_EQPI();	//enter QPI mode (EQIO)
}

void WINBOND_FLASH_QIOR()
{
	//configure register: QUAD bit set 1
	QSPI_FLASH_WRR(0x2, 0x2); //WEL=1, QUAD=1
}

void QSPI_MIDICTRL_Init()
{
	SPI_InitTypeDef QSPI_InitStructure_MIDI;

	SPI_Cmd(QSPIx, DISABLE);
	
	QSPI_InitStructure_MIDI.SPI_Mode = SPI_MASTER;
	QSPI_InitStructure_MIDI.SPI_FIFO = SPI_FIFO_DISABLE;
 	QSPI_InitStructure_MIDI.SPI_DataLength = SPI_DATALENGTH_8;
	QSPI_InitStructure_MIDI.SPI_SELMode = SPI_SEL_HARDWARE;
	QSPI_InitStructure_MIDI.SPI_SELPolarity = SPI_SELPOLARITY_LOW;
	QSPI_InitStructure_MIDI.SPI_FirstBit = SPI_FIRSTBIT_MSB;
	QSPI_InitStructure_MIDI.SPI_CPOL = SPI_CPOL_LOW;
	QSPI_InitStructure_MIDI.SPI_CPHA = SPI_CPHA_FIRST;
	QSPI_InitStructure_MIDI.SPI_ClockPrescaler = 0;
	SPI_Init(QSPIx, &QSPI_InitStructure_MIDI);	

	SPI_SELOutputCmd(QSPIx, ENABLE);
	SPI_DUALCmd(QSPIx, DISABLE);
	HT_QSPI->CR0 &= 0xFFFCFFDF; //--Disable QUADEN, QDIODIR, LOOPBACK
	
	ww(0x400E0040, CMDFL|MIDICEN|QDIOEN|ADFL|MDFL|DMFL|SMDSEL|DATFL);
	ww(0x400E0044, CMDVALUE|MDVALUE);
}

