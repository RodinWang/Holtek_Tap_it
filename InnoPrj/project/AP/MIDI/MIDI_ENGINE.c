/*----------------------------------------------------------------------------------------------------------*/
/* Holtek Semiconductor Inc.                                                                                */
/*                                                                                                          */
/* Copyright (c) 2010-2012 by Holtek Semiconductor Inc.                                                          */
/* All rights reserved.                                                                                     */
/*                                                                                                          */
/*------------------------------------------------------------------------------------------------------------
  File Name        : MIDI_engine.c
  Version          : V0.1
  Date[mm/dd/yyyy] : 02/28/2017
  Description      : MIDI_engine.c.
------------------------------------------------------------------------------------------------------------*/

/* Includes ------------------------------------------------------------------------------------------------*/
#include "ht32f0006_midi.h"
#include "SYS\MIDI_ENGINE.h"

/* Private typedef -----------------------------------------------------------------------------------------*/
/* Private define ------------------------------------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------------------------------------*/
/* Private functions ---------------------------------------------------------------------------------------*/

extern u8 R_Capture_Start;

/*------------------------------------------------------------------------------------------------------------
  Function Name  : MIDI_ENGINE_Init
  Description    : Initializes peripherals used by the MIDI engine driver.
  Input          : None
  Output         : None
  Return         : None
------------------------------------------------------------------------------------------------------------*/
void MIDI_ENGINE_Init(void)
{
	MIDI_InitTypeDef MIDI_InitStructure;
	
  MIDI_InitStructure.MIDI_CTRL_DACDS = 0;
  MIDI_InitStructure.MIDI_CTRL_MUSICEN = ENABLE;
  MIDI_InitStructure.MIDI_CTRL_SPIDISLOOP = DISABLE;
 // MIDI_InitStructure.MIDI_CTRL_CHS = CHS16;
	
  MIDI_Init(MIDIx, &MIDI_InitStructure);
}

//--Definition: MIDI_CHx_NOTE
void MIDI_CHx_NOTE(u8 CHx, MIDI_CHAN_ST_Enum STx, MIDI_CHAN_VM_Enum VMx, MIDI_CHAN_FR_Enum FRx,
									 MIDI_FREQ_BL_Enum BL, u16 FR,
									 MIDI_VOL_AR_Enum A_R, MIDI_VOL_ENV_Enum ENV, u16 VL, u16 VR,
									 u32 ST_ADDR,
									 MIDI_RENUM_WBS_Enum WBS, u16 RE_NUM,
									 u32 END_ADDR)
{
	MIDI_FREQ(MIDIx, BL, FR);
	MIDI_VOL(MIDIx, A_R, ENV, VL, VR);
	MIDI_STADDR(MIDIx, ST_ADDR);
	MIDI_RENUM(MIDIx, WBS, RE_NUM);
	MIDI_ENDADDR(MIDIx, END_ADDR);
	R_Capture_Start = 0;
	MIDI_CHAN(MIDIx, STx, VMx, FRx, CHx);
}




