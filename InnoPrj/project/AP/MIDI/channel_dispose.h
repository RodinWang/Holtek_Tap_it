/************************************
	@file HT MCU channel dispoase.h
	@
************************************/

#ifndef	_CHANNEL_DISPASOE_H
#define	_CHANNEL_DISPASOE_H

#include "ht32.h"
#include	"define.h"
#include	"midi_decode.h"
#include	"timer.h"
#define		c_attack_dot_min	0

#define		c_channel_read_data		0	//设定读取资料的channel为0
#define		c_channel_error		0xff
#define		c_envelope_min		0x3ff
#define		c_envelope_end		0x3ff	//判断包络可以关闭的数值 
#define		c_envelope_start		0
#define		c_length_wav_def	9	//音色样本长度 9个字


#define		c_vib_count_min	0
#define		c_vib_count_max	15
#define		c_vib_intrvl_min	0
#define		c_vib_intrvl_max	8	// 单位2ms

#define		c_attack_a			0xa	

#define		c_zoom_unit	0x40
#define		c_zoom_max	0xC0
#define		c_zoom_min	0x00

#define		c_beat_min	2
#define		c_beat_max	4

#define		c_tempo_max	240
#define		c_tempo_min	30
#define		c_tempo_init	120

#define		c_value_offset				18
#define		c_wave_sample_max			250
#define		c_mask_low_low_nibble		0xf
#define		c_mask_low_high_nibble		0xf0
#define		c_mask_high_low_nibble		0xf00
#define		c_mask_high_high_nibble		0xf000
#define		c_mask_freq_low				0xfff
#define		c_mask_freq_high			0xf000
#define		c_uint_freq_high			0x1000
#define		c_freq_low_max				0x4b0

#define		c_enable_channel_freq		(1<<8)
#define		c_enable_channel_volume		(1<<9)
#define		c_enable_channel_st			(1<<10)

#define		c_envelope_slope_1			(3<<29)	
#define		c_envelope_slope_2			(2<<29)
#define		c_envelope_slope_4			(1<<29)
#define		c_envelope_slope_8			(0<<29)
#define		c_envelope_a_r_attack		((uint32_t)1<<31)
#define		c_envelope_a_r_release		(0<<31)	

#define		c_adsr_slow					0x80
#define		c_adsr_soon_1				0x81
#define		c_adsr_soon_2				0x82
#define		c_adsr_soon_3				0x84	
	
#define		c_envelope_no_attack	0x0f
#define		c_envelope_attack		0		
#define		c_envelope_decay		1
#define		c_envelope_sustain		2
#define		c_envelope_release		3





#define		c_def_freq_offset		1
#define		c_def_st_h_offset		2		//起始地址高位
#define		c_def_bits_offset		2		//wavebits
#define		c_mask_def_wav_bit		3		//音色头样本bit所在位置

#define		c_def_zoom_offset		2
#define		c_def_st_offset			3		//起始地址低位
#define		c_def_re_offset			4
#define		c_def_ea_offset			5
#define		c_def_ea_h_offset		6
#define		c_def_sus_level_offset	7
#define		c_def_adsr_offset		8


#define		c_note_freq				0x4900	//	0x4f00	//		
#define		c_st_addr				0				//  0x65  	//		
#define		c_re_num				0x7f03	//	0x7f5f	//		
#define		c_end_addr				0x67c		// 	0x180d	//	

#define		c_attack_dot_min		0
#define		c_attack_dot_max		67				
#define		c_channel_vol_l_offset	16
#define		c_mask_low_byte 		0x00ff;
#define		c_mask_high_byte 		0xff00;

  
#define		TRACK_MAX			16
#define		c_data_buf_track	16			//MIDI解码 track 数据缓存字节数 32字节
#define		c_max_chan			32

typedef struct
{
	u32		st_addr;		
	u32		re_num;			
	u32		end_addr;			
	u8		type; 		//发音类型
	u8		time;	
	u16		envelope;	//包络点	0~3ff
	u8		adsr;		//当前所在的A、D、S、R状态中的哪个
	u16		R_adsr_value;		//此样本的 adsr 值
	u8		R_sus_level		:4;
	u8		slope;		//ADSR实际值 A=? D=? S=? R=?
	u16		volume;
	u8		id_delay;
	u8		F_release		:1;			//音符抬起标志
	u8		F_first_dispose	:1;	//第一次处理包络
	u8		F_freq_send		:1;
	u8		value;
	u8		def_num;
	u16		freq;			//基础频率							
	u16		freq_total;				//加上 滑音轮、音分之后的频率  
	u8		attack_dot;		//attack的包络点 对应 __t_Attackchange1_7 0~67
	u8		zoom;
	u8		pan;
	u8		track; 
	u8		pitch;		//0x9C~0x64   每个单位表示2个音分	记录上次的滑音值
	u8		tune;		//0x9C~0x64   每个单位表示1个音分	记录上次的音分值
	u8		vib_count;	//vib进程计数 0~15
	u8		vib_intrvl;	//vib间隔 	单位2ms
	u8		F_tremble		:1;	//音量抖动标志 
	u8		R_time_intervl_tremble;//抖音间隔		单位：2ms
	u8		R_counter_tremble;//抖音计数
	u8		R_time_delay_tremble;//抖音启动延时时间 单位：2ms
	u8		R_ENV	:2;
	u8		R_time_tremble;
	
	
} HT_parameter_channel_TypeDef;



uint16_t __l_get_pan_left_vol(uint8_t r_pan);
uint16_t __l_get_pan_right_vol(uint8_t r_pan);
uint16_t __l_vol_overflow_judge(uint16_t r_env);


void	channel_init(void);
void	__l_channel_dispose(void);
void	__L_Note_On(u8 r_type,u8 r_tone,u8 r_keycode,u8 r_volume,u8 r_id_delay,u8 r_pan,u8 r_track) ; 
void	__L_Note_Off(u8);
void	__L_Off_Midi_Note_Play(void);
void	__L_Play_Midi(u16	number,E_Midi_Play_Begin_Middle_Typedf,E_Midi_Pause_Typedf);
void 	__L_Stop_Midi(void);
void	__L_Off_Note_Play(void);
void	__L_Channel_First_Dispose(u8,u8);
void	__L_Channel_Note_Release(u8,u8,u8);
void	__L_Channel_Note_Attack(u8,u8,u8 *,u32 *);
void	__L_Channel_Note_Decay(u8,u8,u8 *,u32 *,u8*);
void	__L_Channel_Delay(void);
void	__L_channel_envelope_total(u16 *,u16 *,u8,u8);
void	__L_Set_Vib(u8);
void	__L_Set_Trans(u8);
void	__L_Set_Sus(u8);
void		__L_Sys_Lrn_Next_Value(void);
void	L_Midi_In_Dispose(void);
void	L_Sys_Exclusive_Tx(void);
void	L_Midi_In_Data_Adjust_1(void);


extern 	uint16_t	const	__t_tone_def[];
extern	u8	__R_Vib;			//系统颤音值	范围 （0~4）最大上下1个半音
extern	u8	__R_Trans;	
extern	u8	__R_Sus;
extern	u8	__R_Pitch_Bend;		//系统滑音轮值	范围（9C~64）上下两个半音，一个单位表示两个音分
extern	u8	__R_Tune;			//系统音分值	范围（9C~64）上下1个半音，一个单位表示1个音分		
extern	u8	__R_Bt_Cnt_1_24;
extern	u8	__R_Time_Count;
extern	u8	__R_Bt_Cnt_1_48;
extern	u8	__R_Last_1_48Beat;
extern	EventStatus	__F_Func_Tremble;
extern	u8	__R_Actl_Note_Vol;
extern	u8	__R_Actl_Midi_Vol;
extern	u8	const __r_max_chan;

extern	u32	__R_Midi_Addr;
extern	u16	__R_Track_Data[TRACK_MAX][c_data_buf_track];
extern	HT_parameter_channel_TypeDef	__R_Chan_Para_Struct[c_max_chan];
extern	TRACK_InfTypeDef		__MIDI_Track_Struct[];
extern	MIDI_StatusTypedef		__MIDI;
extern	u8 __R_Ch_Note_Play;



extern	u8	__R_Intf_Count;


#endif







