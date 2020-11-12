//���ļ����ڶ����Լ�����ı���

#ifndef	__DEF_H
#define	__DEF_H

#include	"ht32.h"
typedef	enum 
{			
	c_enum_midi_play_middle=0,			//�����ಥ��ģʽ		�������ಥ��
	c_enum_midi_play_begin 	,			//ǰ�ࡢ��������ģʽ	��ͷ����
}E_Midi_Play_Begin_Middle_Typedf	;


typedef	enum 
{		
	c_enum_midi_not_pause=0,	//����
	c_enum_midi_set_pause 	,	//���� ����								 
}E_Midi_Pause_Typedf	;


//typedef enum
//{
//	c_func_disable=0,
//	c_func_enable,
//}e_func_switch;

typedef enum
{
	c_midi_set=0,	//midi����tempo
	c_human_set,	//��Ϊ����tempo
}e_tempo_set;

 


typedef	struct {
		
	bool	F_Play				;	//���ڲ��� 		F_ ��д ����ȫ�ֱ���
	ControlStatus 	F_Mode_Song	;
	ControlStatus 	F_Play_Head	;		//1:ENABLE ��ͷ���� ��DISABLE �������࿪ʼ����
	bool		F_Get_Main_Note	;		//ȡ����������
	
	bool	F_Pause					;	//��ͣ��־
	E_Midi_Pause_Typedf	F_set_pause	;	//������ͣ
	bool	F_Speed					;	//����
	bool	F_Beat					;	//һ�ı�־
	bool	F_Half_Beat				;	//���ı�־

	bool	F_Midi_Note_Mute 		;	//��������:1, ����:0
		
	bool	F_Cntns_Event 		;	//�����¼�
	bool	F_Ready				;	//��������׼����־  ����Ϊ1 ����Ϊ0 Ȼ��ʼMIDI����
	bool	F_Get_Next_Value	;	//ȡ����һ������ c_flag_true c_flag_false
 
	bool	F_Rhy_Play_End			;	//1:����β�� 0������������

//	u8	F_1_24_beat			 	:1	;	//24��֮һ�ļ�ʱ��־
//	u8	F_get_tone_vol_pan		:1	;	//ȡ�ù����ɫ ���� ����Ȼ�����Ϣ
	
}MIDI_StatusTypedef;	

typedef	struct {
	u8					Value;//MIDI �ٶ� +40
	ControlStatus		Flag ;
}S_Tempo_Typedef;

#endif

