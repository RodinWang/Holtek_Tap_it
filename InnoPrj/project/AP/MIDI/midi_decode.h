

#ifndef	__MIDI_DECODE_H
#define	__MIDI_DECODE_H



#include	"ht32.h"
#include	"define.h"


#include 	"define_type.h"

#define MIDIx       HT_MIDI
#define NOTE4_1 0x800
#define		c_low_byte		0		//���ݵĸ��ֽ�
#define		c_high_byte		1

#define		c_main_rhy		0		//����������  ǰ������
#define		c_intro_rhy		1

#define		c_time		0
#define		c_event		1
#define		c_midi_note_delay_max	240

#define		ADDR_NULL	0x0000	

#define		MIDI_TRACK	0
#define		c_main_track	1
#define		RHY_TRACK		2
#define		RHY_TRACK_2	9


#define		C_48BEAT_MAX		48
#define		C_HALF_BEAT 	24

//extern	const	u16	c_addr_table_midi;
//extern	const	u32	*Midi_Pointer;
//extern	const	u8	c_unit_addr_midi;



//*******�������*******************************
#define		C_DRUM_TONE			128 //�������ɫ
#define		C_DRUM_DELAY		10			



/*******�������ָ��******
*		������ö�ٽṹ
*********************/




/****************************
*midiÿ�����������	
****************************/
typedef struct 	
{
	u16	ADDR;		//addr ���ڽ���ĵ�ַ
	u16	REPLAY_ADDR;		////����ѭ�� λ��
	u16	DATA;	
	u8	TONE;		//tone ��ɫ
	u8	NOTE;		// note value ����ֵVAL
	u8	BEAT;		// �������ʱ�� ���Ǹ������ĺ�
	u8	VOL;		//volume ����
	u8	PAN;		//pan ���� ��������ƫ��
	u8	PITCH;	
	u8	VIB;	
	u8	F_Byte_Low_High	:1;		//HIGH :1,	LOW:0 ���������ֽڵĸߵ�
	u8	F_Time_Event	:1;		//event:1, time :0  
	u8	F_End			:1;		//���������־	
	ControlStatus	F_Rhy_Jump;	//������Ծ ENABLE ������������λ��
	
} TRACK_InfTypeDef;

/**********************
midi��������Ϣ MIDI_Data
�ٶ�	����	
******************/
typedef struct {

//	u8	MIDI_TEMPO	;
	u8	Midi_Beat	;		//midi ������ ��MIDI��Ϣת������
	u8	MIDI_PITCH	;
	u8	MIDI_PAN	;
	u8	Midi_Current_Beat;	//midi ʵʱ������
//	u8	TRACK_NUM  ;
//	u8	MIDI_MEASURE	;
//	u8	MIDI_	;

}MIDI_DataTypedef;

/**********************************
MIDI����״̬����ر�־  MIDI
***********************************/
//typedef	struct {
//		
//	u8	F_play					:1	;	//���ڲ��� 		F_ ��д ����ȫ�ֱ���
//	u8	F_mode_song_rhy			:1	;	//���� or ���ࣨ���죩
//	u8	F_pause					:1	;	//��ͣ��־
//	E_Midi_Pause_Typedf	F_set_pause	;	//������ͣ
//	u8	F_speed					:1	;	//����
//	u8	F_beat					:1	;	//һ�ı�־
//	u8	F_half_beat				:1	;	//���ı�־
//	u8	F_next_track			:1	;	//��һ��������־ 
//	u8	F_1_24_beat			 	:1	;	//24��֮һ�ļ�ʱ��־
//	u8	F_midi_note_mute 		:1	;	//��������:1, ����:0
////	u8	F_rhy_main_intro:1		:1	;	//������������������ѭ����־
//	E_Midi_Play_Begin_Middle_Typedf	F_play_begin_middle			;
//	u8	F_continuous_event 		:1	;	//�����¼�
//	u8	F_ready					:1	;  //��������׼����־  ����Ϊ1 ����Ϊ0 Ȼ��ʼMIDI����
//
//}MIDI_StatusTypedef;

typedef enum
{
	MIDI_TRACK_0 = 0,
	MIDI_TRACK_1		,
	MIDI_TRACK_2		,
	MIDI_TRACK_3		,
	MIDI_TRACK_4		,
	MIDI_TRACK_5		,
	MIDI_TRACK_6		,
	MIDI_TRACK_7 		,
	MIDI_TRACK_8		,
	MIDI_TRACK_9		,
	MIDI_TRACK_10		,
	MIDI_TRACK_11 	,
	MIDI_TRACK_12		,
	MIDI_TRACK_13 	,	
	MIDI_TRACK_14		,
	MIDI_TRACK_15	

}MIDI_TRACK_TypeDef;


typedef enum{
	
	MIDI_CMD_VOL 		= 0X41,
	MIDI_CMD_PAN 		= 0X42,
	MIDI_CMD_PITCH 		= 0X43,
	MIDI_CMD_VIB 		= 0X44,
	MIDI_CMD_REPLAY 	= 0X45,
	MIDI_CMD_TEMPO 		= 0X46,
	MIDI_CMD_TONE 		= 0X47,
	MIDI_CMD_BEAT 		= 0X48,
	MIDI_CMD_NULL 		= 0X4C,
	MIDI_CMD_END	 	= 0X4F
	
}MIDI_Cammamdypedef;





extern	u8 		__R_Track;
extern	u8	__R_Tempo;
extern	u16	__R_2ms_Count;
extern	u16 __R_2ms_Last;
extern	u32 RXDataBuffer[];
extern	u32 TXDataBuffer[];
extern	u8 R_Len_Buffer_Tx;



/************��������***���ⲿ����******************/
void	Midi_DecodeInit(void);
void	Midi_TrackDataInit(void);
void 	Midi_PlayStop(void);
void	__L_It_Spi_Data_Judge(void);
void	__L_Midi_Decode(void);
void	Midi_Beat48_Counter(void);
void	Midi_GetNextMainNote(void);
void	MIDI_NotePlay(u8 t_note) ;
void	__L_User_Measure_Finish(void);
void	__L_User_Beat_Finish(void);
void	__L_User_Half_Beat_Finish(void);
void	__L_Sys_Beat_Count(void);
void	__L_User_Before_Note_Play(void);

void	__L_User_1_48Beat(u8);	
void	__L_User_2ms_Count(u16);
void	__L_Sys_Time_Count(void);
void	__L_User_Rhy_Main_Start(void);
void	__L_User_Rhy_Replay(void);
void	__L_User_Midi_End(void);
void	__L_Channel_Delay(void);
void 	MIDI_SPIDATA_Read(u32 Addr, u8 Numtoread);
u16		__L_Get_Flash_Single_2byte(u32	r_addr);
void	__L_Get_Flash_32byte(u32	r_addr,u8	r_track,u16 *);
void	L_Sys_Exclusive_Dispose(void);
void	L_Sys_Exclusive_The_TQB(void);
void	__L_Midi_In_All_Sound_Off(void);
void	__L_Set_Sus(u8);

void 	MIDI_CHx_NOTE(u8 CHx, MIDI_CHAN_ST_Enum STx, MIDI_CHAN_VM_Enum VMx, MIDI_CHAN_FR_Enum FRx,
									 MIDI_FREQ_BL_Enum BL, u16 FR,
									 MIDI_VOL_AR_Enum A_R, MIDI_VOL_ENV_Enum ENV, u16 VL, u16 VR,
									 u32 ST_ADDR,
									 MIDI_RENUM_WBS_Enum WBS, u16 RE_NUM,
									 u32 END_ADDR);

#endif



