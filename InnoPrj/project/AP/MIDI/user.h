/**********************************************
�ļ����ƣ�	user.h
����������
**********************************************/
#include "key_func.h"
//�û�Ӧ�ò㶨��ĳ��� �¶���ı�����
typedef unsigned char  u8;

#define	c_actl_rhy_max		158

//#define	c_song_min		0

#define	c_vol_level_init	13
#define	c_vol_level_max	15
#define	c_vol_level_min	0
#define	c_num_null		0xff	//��Ч������
#define	c_tempo_max	240
#define	c_tempo_min	30
#define	c_tempo_init	120

#define	c_time_temp_disp_null	0
#define	c_midiin_led_indctr	48
#define	c_buffer_midiin_max	256*4//midiin buffer ָ��
#define	c_beat_max	4
#define		c_midi_type		1
#define		c_voice_type	2
#define		c_sound_type	3




typedef enum
{
	c_mode_record=0,	
	c_mode_program,
	c_mode_normal,
	
	c_mode_rhy_play,
	c_mode_demoone ,	
	c_mode_demoall  ,	
	c_mode_kalaok ,	
	c_mode_onekey ,	
	c_mode_follow ,	 
}E_Mode_Typedef;


typedef enum
{
	c_rhy_main=0,
	c_rhy_intro,
	c_rhy_end,
}e_rhy_mode_typedef;


	

