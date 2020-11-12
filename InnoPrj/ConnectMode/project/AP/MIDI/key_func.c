
#include	"define.h"
#include	"define_type.h"
#include	"key_func.h"
#include	"user.h"
#include 	"QSPI_flash.h"
#include 	"QSPI_MIDICTRL.h"
#include 	"MIDI_ENGINE.h"
#include  "ht32f5xxxx_dac_dual16.h"
#include	"channel_dispose.h"
#include	"ht32.h"
#include 	"timer.h"

#ifdef _24M		
		u8	const __r_max_chan=16;
#elif _48M
		u8	const __r_max_chan=32;
#endif

extern	u8	const	__t_midi_vol[];
extern	unsigned long  const R_DOT_Wav[];
EventStatus	F_Func_Tremble=ENABLE;
EventStatus		F_Func_Play_Stop=DISABLE;
EventStatus		F_Func_Sntnc=DISABLE;
u16		R_Sntnc_Order;
u16		R_Time_Mute;


u32	R_Index=c_index_init;
u8	R_Song=0;
u8	R_Tone=0;
u32	R_Wave_Count=0;
u8	const	c_time_dbnc_init=0;
u8	const	c_time_dbnc_max=20;		//消抖时间延长，便于组合键检测.
u8	const	c_time_dbnc_min=1;	
u8	R_Func_Key_Dbnc=c_time_dbnc_init;//功能键 debounce.
u8	R_FuncKeyStatus=0xff;

u8	R_Vol=c_vol_level_init;

PDMACH_InitTypeDef PDMACH_InitStructure;

											
E_Mode_Typedef	R_Mode=c_mode_normal;

//=============================================
//函数名：	Func_DemoCom
//功能描述：	demo.song.播放.
//=============================================
void Func_DemoCom(u8 r_num)
{

		
	if(c_mode_demoall==R_Mode)
	{		
		__MIDI.F_Mode_Song=ENABLE;
		__L_Play_Midi(r_num, c_enum_midi_play_begin,c_enum_midi_not_pause);
		
	}

}

//=============================================
//函数名：	FuncKey_DemoAll
//功能描述：
//=============================================
void  FuncKey_DemoAll(void)
{
	__L_Stop_Midi();
	R_Mode = c_mode_demoall;			
	Func_DemoCom(R_Song);
}

//
//***********非按键的 其他应用程序********************************************************************************
//=============================================
//函数名：	Play_NextMIDI
//功能描述：
//=============================================
void	Play_NextMIDI()
{
	if(__MIDI.F_Play == c_u8_false)
	{
		if(R_Mode == c_mode_demoall)
		{
			R_Song++ ;
			if(R_Song > c_song_max)
				R_Song = 0 ;
			
			Func_DemoCom(R_Song);
		}
		else if(R_Mode == c_mode_demoone)
		{
			Func_DemoCom(R_Song);
		}	
		else if(R_Mode == c_mode_onekey)
		{
			Func_DemoCom(R_Song);
		}	
	}
}



EventStatus	L_Audio_Finish(void)
{
	EventStatus	f_finish=DISABLE;
	if(0==R_Wave_Count && L_Midi_Finish())
	{
		f_finish=ENABLE;
		L_Audio_Stop();
	}

	return f_finish;
}

EventStatus	L_Midi_Finish(void)
{
	EventStatus	f_finish=DISABLE;
	if(__MIDI.F_Play)
		;
	else
		f_finish=ENABLE;

	return f_finish;
}


void	L_Sntnc_Start(u32 index)
{
	u16	r_data;
	r_data=c_sentence_content[index];
	if(r_data >= c_sntnc_wav_id)
	{
		u16	r_num;
		r_num=r_data&(~c_sntnc_wav_id);
		L_Audio_Play(c_voice_type,r_num+1);
	}
	else if(c_sntnc_stop_id==r_data)
		F_Func_Sntnc=DISABLE;
	else
	{
		R_Time_Mute=r_data/2;
	}
		
	
	
}


void	L_Sntnc_Play(u32 index)
{
	if(index!=0)
		index--;
	R_Sntnc_Order=c_sentence_offset[index];
	F_Func_Sntnc=ENABLE;
	R_Time_Mute=0;
	L_Sntnc_Start(R_Sntnc_Order);
}

void	L_Sntnc_Stop(void)
{
	if(F_Func_Sntnc)
	{
		F_Func_Sntnc=DISABLE;
		L_Audio_Stop();
	}
}


void	L_Sntnc_Main_Loop(void)
{
	if(F_Func_Sntnc)
	{
		if((0==R_Wave_Count)&&(0==R_Time_Mute))
		{
			R_Sntnc_Order++;
			L_Sntnc_Start(R_Sntnc_Order);
		}
	}
}

 


void	L_Audio_Play(u8 audio_type,u32 index)
{
	u8	keycode,tone=129;
	u32 r_count;
	
	L_Audio_Stop();
	if((c_midi_type == audio_type)  || (index >(c_voice_max + c_sound_effect_max)))
	{
		if((index >(c_voice_max + c_sound_effect_max))  && (c_midi_type != audio_type))
			index-=(c_voice_max + c_sound_effect_max);
		if(index>c_song_max)
			index=c_song_max;
		
		R_Song=(u16)index;
		if(R_Song!=0)
			R_Song--;
		FuncKey_DemoAll();
	}
	else
	{
		r_count=index;

		if(c_sound_type==audio_type)
		{//c_sound_type
			if(r_count>c_sound_effect_max)
				r_count=c_sound_effect_max;
			r_count+=c_voice_max;
		}
		if(r_count!=0)
			r_count--;
		
		R_Wave_Count=R_DOT_Wav[r_count];
		while(1)
		{
			if(r_count<61)
				break;
			r_count-=61;
			tone++;
		}
		keycode=(u8)(r_count);
		__L_Note_On(c_notekey_play,tone,keycode,__R_Actl_Note_Vol,keycode,0,0);
	}
	
	
}

void	L_Audio_Vol(u8	vol)
{
	if(vol <= c_vol_level_max)
	{
		R_Vol=vol;
		__R_Actl_Note_Vol=__t_midi_vol[R_Vol];	
		__R_Actl_Midi_Vol=__t_midi_vol[R_Vol];
	}
}

void	L_Midi_Stop(void)
{
	if(R_Mode == c_mode_demoall)
	{	
		R_Mode = c_mode_normal ;
		__L_Stop_Midi();
	}
}


void	L_Audio_Stop(void)
{
	L_Midi_Stop();
	
	__L_Off_Note_Play();
	if(R_Wave_Count!=0)
		R_Wave_Count=0;
}

void	L_Audio_Init(void)
{
	QSPI_FLASH_Init();
	QSPI_xFLHx_Init();
	QSPI_MIDICTRL_Init();

	CKCU_SetMIDIPrescaler(CKCU_MIDIPRE_DIV8);
	MIDI_DeInit();
	 
	MIDI_ENGINE_Init();
	
	#ifdef _48M	
		MIDI_CTRL_CHS(MIDIx, CHS32); 
	#endif
	
	#ifdef _24M	
		MIDI_CTRL_CHS(MIDIx, CHS16);
	#endif
	
	
	MIDI_ClearFlag(MIDIx, MIDI_FLAG_INTF);
	MIDI_IntConfig(MIDIx, MIDI_INT_MIDIO_DMAEN|MIDI_INT_MIDII_DMAEN|MIDI_INT_INTEN, ENABLE);
	
	BFTM_Config(); 
	Midi_DecodeInit();
	
	DAC_RAMP_UP(0);	
	R_Vol=c_vol_level_init;
	__R_Actl_Note_Vol=__t_midi_vol[R_Vol];	
	__R_Actl_Midi_Vol=__t_midi_vol[R_Vol];
}

void	L_Vol_Inc(void)
{
	if(R_Vol<c_vol_level_max)
		R_Vol++;
	__R_Actl_Note_Vol=__t_midi_vol[R_Vol];	
	__R_Actl_Midi_Vol=__t_midi_vol[R_Vol];
}

void	L_Vol_Dec(void)
{
	if(R_Vol!=c_vol_level_min)
		R_Vol--;
	__R_Actl_Note_Vol=__t_midi_vol[R_Vol];	
	__R_Actl_Midi_Vol=__t_midi_vol[R_Vol];
}

void	L_Audio_Id_Inc(void)
{
	
	if(R_Index<(c_song_max + c_voice_max + c_sound_effect_max))
		R_Index++;
	else
		R_Index=c_index_init;
	
	if(F_Func_Play_Stop)
		L_Audio_Play(c_voice_type,R_Index);		
	
}


/*********************************************************************************************************//**
参数Sel：	当参数为0是，执行DAC RAMP UP
          当参数为1时，直接输出0X8000
  ***********************************************************************************************************/
void DAC_RAMP_UP(u8 Sel)
{
//	u16 cnt;
	
//	CKCU_APBPerip1ClockConfig(CKCU_APBEN1_DAC, ENABLE); 	//enable DAC clock

	DACD16_DataSourceConfig(HT_DACDUAL16, DAC_CH_R, DATA_FROM_MIDI );//DATA_FROM_MIDI DATA_FROM_UC
	DACD16_DataSourceConfig(HT_DACDUAL16, DAC_CH_L, DATA_FROM_MIDI );
	
//	if (Sel)
//	{
//		HT_DAC->LH =0;
//		HT_DAC->RH =0;
//		HT_DAC->TG = 0x101;
//		HT_DAC->TG = 0;
//		
//		for (cnt=0; cnt<2048; cnt++)
//		{
//			HT_DAC->LH =HT_DAC->LH + 16;
//			HT_DAC->RH =HT_DAC->RH + 16;
//			HT_DAC->TG = 0x101;
//			HT_DAC->TG = 0;
//			dly(140);
////			dly(340);
//		}
//	}
//	else
//	{
//		HT_DAC->LH =0x8000;
//		HT_DAC->RH =0x8000;
//		HT_DAC->TG = 0x101;
//		HT_DAC->TG = 0;
//		
//	}
}

void NVIC_Configuration(void)
{
  NVIC_EnableIRQ(QSPI_IRQn);
  NVIC_EnableIRQ(MIDI_IRQn);
  NVIC_SetPriority(MIDI_IRQn,0);
	NVIC_SetPriority(BFTM0_IRQn,1);
  
  SYSTICK_ClockSourceConfig(SYSTICK_SRC_FCLK);       // Default : CK_SYS/8
  SYSTICK_IntConfig(ENABLE);                          // Enable SYSTICK Interrupt
}

void CKCU_Configuration(void)
{
  CKCU_PeripClockConfig_TypeDef CKCUClock = {{ 0 }};	
	
	CKCUClock.Bit.BKP        = 1;
	CKCUClock.Bit.QSPI       = 1;
	CKCUClock.Bit.PDMA       = 1;
	CKCUClock.Bit.MIDI       = 1;
	CKCUClock.Bit.BFTM1      = 1;
	CKCUClock.Bit.BFTM0      = 1;
	CKCUClock.Bit.DAC        = 1;

  CKCU_PeripClockConfig(CKCUClock, ENABLE);
}

void MIDI_GPIOConfiguration(void)
{
	//---------------------------------------------QSPI	
//	AFIO_GPxConfig(GPIO_PA, AFIO_PIN_8| AFIO_PIN_14 | AFIO_PIN_10 | AFIO_PIN_15, AFIO_MODE_5);
//	AFIO_GPxConfig(GPIO_PB, AFIO_PIN_0| AFIO_PIN_1, AFIO_MODE_5);
    AFIO_GPxConfig(GPIO_PA, AFIO_PIN_8 | AFIO_PIN_10, AFIO_MODE_5);
  AFIO_GPxConfig(GPIO_PC, AFIO_PIN_10 | AFIO_PIN_11 | AFIO_PIN_12 | AFIO_PIN_13, AFIO_MODE_5);  
	//---------------------------------------------<<DAC咏迏狮吱.
	AFIO_GPxConfig(GPIO_PC,AFIO_PIN_1 | AFIO_PIN_2, AFIO_MODE_2);//GPC1|GPC2胤为DAC爻話通謤咏迏.
}

void	L_Func_Init(void)
{
	__L_Set_Vib(0);
	__L_Set_Trans(0);
	__L_Set_Sus(0);
	
//	FuncKey_DemoAll();
	
	__R_Actl_Note_Vol=__t_midi_vol[R_Vol];	
	__R_Actl_Midi_Vol=__t_midi_vol[R_Vol];
	
}

void MIDI_FuncInit(void)
{
  NVIC_Configuration();
  CKCU_Configuration();
  MIDI_GPIOConfiguration();
  
  L_Audio_Init(); 
	L_Func_Init();
}

void MIDI_Process(void)
{
  __L_Midi_Decode();
  __l_channel_dispose();
  L_Sntnc_Main_Loop();
  L_Midi_In_Dispose();
}



