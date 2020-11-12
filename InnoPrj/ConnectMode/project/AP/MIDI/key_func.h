

#ifndef	_KEY_FUNC_H
#define	_KEY_FUNC_H

#include "ht32.h"  
#include "ht32_usbd_core.h"

#define		c_sntnc_wav_id	0x8000
#define		c_sntnc_stop_id	0
#define		c_index_init	1

extern	u32	R_Index;
extern	EventStatus		F_Func_Play_Stop;
extern u16		R_Time_Delay_Demo_Replay;

extern	unsigned short const c_song_max;
extern	unsigned short const c_voice_max;
extern	unsigned short const c_sound_effect_max;
extern	unsigned const short	c_sentence_offset[];
extern	unsigned const short	c_sentence_content[];
extern	u16		R_Time_Mute;


extern	u8	R_Song;
extern	u8	R_Tone;
extern	u8	R_Time_Count;

extern	u32	R_Wave_Count;
extern	u8	const	c_time_dbnc_init;
extern	u8	const	c_time_dbnc_max;		//消抖时间延长，便于组合键检测.
extern	u8	const	c_time_dbnc_min;	
extern	u8	R_Func_Key_Dbnc;
extern	u8	R_FuncKeyStatus;
extern	u8	R_Vol;
extern	u8	R_Actl_Note_Vol;									//实际音符音量.
extern	u8	R_Actl_Midi_Vol;

extern	u32 RXDataBuffer[16];	
extern	u32 TXDataBuffer[16];	


extern	u32 R_TxLen;
extern	__ALIGN4 USBDCore_TypeDef gUSBCore;
extern	USBD_Driver_TypeDef gUSBDriver;

extern	PDMACH_InitTypeDef PDMACH_InitStructure;

void  	FuncKey_DemoAll(void);
void	L_Audio_Id_Inc(void);
void	L_Audio_Play(u8 audio_type,u32 index);
void	L_Sntnc_Play(u32 index);
void	L_Sntnc_Stop(void);
EventStatus	L_Midi_Finish(void);
EventStatus	L_Audio_Finish(void);
void	L_Vol_Inc(void);
void	L_Vol_Dec(void);
void	L_Audio_Vol(u8);
void	L_Audio_Stop(void);
void	L_Audio_Init(void);
void 	DAC_RAMP_UP(u8 Sel);
void	L_Sntnc_Main_Loop(void);
void	L_Midi_Stop(void);
void MIDI_FuncInit(void);
void MIDI_Process(void);

#endif	

