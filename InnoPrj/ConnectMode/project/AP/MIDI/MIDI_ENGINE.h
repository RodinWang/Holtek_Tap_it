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
/* Define to prevent recursive inclusion -------------------------------------------------------------------*/
#ifndef __MIDI_ENGINE_H
#define __MIDI_ENGINE_H


/* Includes ------------------------------------------------------------------------------------------------*/
#include "ht32.h"

#define MIDIx       HT_MIDI

/* Exported types ------------------------------------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------------------------------------*/
#define SIN_8BIT_START    0x0
#define SIN_12BIT_START   0x60000
#define SIN_8BIT_START_2  0x2AAA0
#define SIN_8BIT_START_3  0x55540
#define SIN_16BIT_START   0xFFFE80

#define SIN_8BIT_START_A  0x200
#define SIN_12BIT_START_A 0x600
#define SIN_16BIT_START_A 0xA00

#define SIN_8BIT_START_B  0x2000
#define SIN_12BIT_START_B 0x6000
#define SIN_16BIT_START_B 0xA000

#define SIN_8BIT_START_C  0x20000
#define SIN_12BIT_START_C 0x60000
#define SIN_16BIT_START_C 0xA0000


#define SIN_8BIT_END (SIN_8BIT_START+(128*1))
#define SIN_12BIT_END (SIN_12BIT_START+(192))
#define SIN_8BIT_END_2 (SIN_8BIT_START_2+(128*1))
#define SIN_8BIT_END_3 (SIN_8BIT_START_3+(128*1))

#define SIN_16BIT_END (SIN_16BIT_START+(128*2))

#define SIN_8BIT_END_A (SIN_8BIT_START_A+(128*1))
#define SIN_12BIT_END_A (SIN_12BIT_START_A+(192))
#define SIN_16BIT_END_A (SIN_16BIT_START_A+(128*2))

#define SIN_8BIT_END_B (SIN_8BIT_START_B+(128*1))
#define SIN_12BIT_END_B (SIN_12BIT_START_B+(192))
#define SIN_16BIT_END_B (SIN_16BIT_START_B+(128*2))

#define SIN_8BIT_END_C (SIN_8BIT_START_C+(128*1))
#define SIN_12BIT_END_C (SIN_12BIT_START_C+(192))
#define SIN_16BIT_END_C (SIN_16BIT_START_C+(128*2))



#define SIN_8BIT_ST      (SIN_8BIT_START   )>>5
#define SIN_8BIT_ST_2    (SIN_8BIT_START_2 )>>5 
#define SIN_8BIT_ST_3    (SIN_8BIT_START_3 )>>5 
#define SIN_8BIT_ST_A    (SIN_8BIT_START_A )>>5 
#define SIN_8BIT_ST_B    (SIN_8BIT_START_B )>>5 
#define SIN_8BIT_ST_C    (SIN_8BIT_START_C )>>5 
#define SIN_12BIT_ST     (SIN_12BIT_START   /3*2)>>5    
#define SIN_12BIT_ST_A   (SIN_12BIT_START_A /3*2)>>5
#define SIN_12BIT_ST_B   (SIN_12BIT_START_B /3*2)>>5 
#define SIN_12BIT_ST_C   (SIN_12BIT_START_C /3*2)>>5 
#define SIN_16BIT_ST     (SIN_16BIT_START   /2)  >>5
#define SIN_16BIT_ST_A   (SIN_16BIT_START_A /2)  >>5 
#define SIN_16BIT_ST_B   (SIN_16BIT_START_B /2)  >>5
#define SIN_16BIT_ST_C   (SIN_16BIT_START_C /2)  >>5

#define SIN_8BIT_EA       SIN_8BIT_END    
#define SIN_8BIT_EA_2     SIN_8BIT_END_2 
#define SIN_8BIT_EA_3     SIN_8BIT_END_3 
#define SIN_8BIT_EA_A     SIN_8BIT_END_A 
#define SIN_8BIT_EA_B     SIN_8BIT_END_B 
#define SIN_8BIT_EA_C     SIN_8BIT_END_C 
#define SIN_12BIT_EA      (SIN_12BIT_END   /3*2)
#define SIN_12BIT_EA_A    (SIN_12BIT_END_A /3*2)
#define SIN_12BIT_EA_B    (SIN_12BIT_END_B /3*2)
#define SIN_12BIT_EA_C    (SIN_12BIT_END_C /3*2)
#define SIN_16BIT_EA      (SIN_16BIT_END   /2)
#define SIN_16BIT_EA_A    (SIN_16BIT_END_A /2)
#define SIN_16BIT_EA_B    (SIN_16BIT_END_B /2)
#define SIN_16BIT_EA_C    (SIN_16BIT_END_C /2)

#define NOTE2_3 0x285
#define NOTE2_4 0x2AB
#define NOTE2_4U 0x2D4
#define NOTE2_5 0x2FF
#define NOTE2_5U 0x32C
#define NOTE2_6 0x35D
#define NOTE2_6U 0x390
#define NOTE2_7 0x3C6
#define NOTE3_1 0x3FF
#define NOTE3_1U 0x43C
#define NOTE3_2 0x47D
#define NOTE3_2U 0x4C1
#define NOTE3_3 0x50A
#define NOTE3_4 0x556
#define NOTE3_4U 0x5A8
#define NOTE3_5 0x5FE
#define NOTE3_5U 0x659
#define NOTE3_6 0x6BA
#define NOTE3_6U 0x720
#define NOTE3_7 0x78D
#define NOTE4_1 0x800
#define NOTE4_1U 0x879
#define NOTE4_2 0x8FA
#define NOTE4_2U 0x983
#define NOTE4_3 0xA14
#define NOTE4_4 0xAAD
#define NOTE4_4U 0xB50
#define NOTE4_5 0xBFC
#define NOTE4_5U 0xCB2
#define NOTE4_6 0xD74
#define NOTE4_6U 0xE41
#define NOTE4_7 0xF1A

/* Exported macro ------------------------------------------------------------------------------------------*/
/* Exported functions --------------------------------------------------------------------------------------*/
void MIDI_ENGINE_Init(void);

void MIDI_CHx_NOTE(u8 CHx, MIDI_CHAN_ST_Enum STx, MIDI_CHAN_VM_Enum VMx, MIDI_CHAN_FR_Enum FRx,
									 MIDI_FREQ_BL_Enum BL, u16 FR,
									 MIDI_VOL_AR_Enum A_R, MIDI_VOL_ENV_Enum ENV, u16 VL, u16 VR,
									 u32 ST_ADDR,
									 MIDI_RENUM_WBS_Enum WBS, u16 RE_NUM,
									 u32 END_ADDR);


#endif /* __MIDI_ENGINE_H ---------------------------------------------------------------------------------*/




