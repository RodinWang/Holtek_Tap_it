//此文件用于定义自己定义的变量

#ifndef	__DEF_H
#define	__DEF_H

#include	"ht32.h"
typedef	enum 
{			
	c_enum_midi_play_middle=0,			//主节奏播放模式		从主节奏播放
	c_enum_midi_play_begin 	,			//前奏、歌曲播放模式	从头播放
}E_Midi_Play_Begin_Middle_Typedf	;


typedef	enum 
{		
	c_enum_midi_not_pause=0,	//合奏
	c_enum_midi_set_pause 	,	//单键 跟随								 
}E_Midi_Pause_Typedf	;


//typedef enum
//{
//	c_func_disable=0,
//	c_func_enable,
//}e_func_switch;

typedef enum
{
	c_midi_set=0,	//midi设置tempo
	c_human_set,	//人为设置tempo
}e_tempo_set;

 


typedef	struct {
		
	bool	F_Play				;	//正在播放 		F_ 大写 这是全局变量
	ControlStatus 	F_Mode_Song	;
	ControlStatus 	F_Play_Head	;		//1:ENABLE 从头播放 ；DISABLE 从主节奏开始播放
	bool		F_Get_Main_Note	;		//取得主轨音符
	
	bool	F_Pause					;	//暂停标志
	E_Midi_Pause_Typedf	F_set_pause	;	//设置暂停
	bool	F_Speed					;	//加速
	bool	F_Beat					;	//一拍标志
	bool	F_Half_Beat				;	//半拍标志

	bool	F_Midi_Note_Mute 		;	//音符关音:1, 放音:0
		
	bool	F_Cntns_Event 		;	//连续事件
	bool	F_Ready				;	//歌曲播放准备标志  先设为1 再设为0 然后开始MIDI解码
	bool	F_Get_Next_Value	;	//取得下一个音符 c_flag_true c_flag_false
 
	bool	F_Rhy_Play_End			;	//1:播放尾奏 0：播放主节奏

//	u8	F_1_24_beat			 	:1	;	//24分之一拍计时标志
//	u8	F_get_tone_vol_pan		:1	;	//取得轨道音色 音量 声相等基本信息
	
}MIDI_StatusTypedef;	

typedef	struct {
	u8					Value;//MIDI 速度 +40
	ControlStatus		Flag ;
}S_Tempo_Typedef;

#endif

