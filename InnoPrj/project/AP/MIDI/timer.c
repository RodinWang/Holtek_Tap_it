/**********************************************
�ļ����ƣ�	timer.c
����������	
**********************************************/

#include	"ht32.h"
#include	"ht32f5xxxx_ckcu.h"
#include	"ht32f5xxxx_bftm.h"
#include	"timer.h"
#include	"define.h"
#include	"key_func.h"
#include	"channel_dispose.h"
void	Midi_Beat48_Counter(void);


/***********************
*@file �������ܶ�ʱ�� 16λ
*@2ms��ʱ	
***********************/
void BFTM_Config()
{
//	CKCU_APBPerip1ClockConfig(CKCU_APBEN1_BFTM0, ENABLE);	//CKCU_APBEN1_BFTM1 | 
	NVIC_EnableIRQ(BFTM0_IRQn);	

	BFTM_SetCompare(HT_BFTM0,(SystemCoreClock/2000*4));//2ms��ʱ		BFTMx->CMP 32bit   sysclk 12000000UL 
 
	BFTM_SetCounter(HT_BFTM0, 0);	//  BFTMx->CNTR
	BFTM_IntConfig(HT_BFTM0, ENABLE);
	BFTM_EnaCmd(HT_BFTM0, ENABLE);
	 
	BFTM1_TEMPO_Config(__R_Tempo);
}

/*************************************************************
	�����ٶ��趨��ʱ����ʱ�䣬48��֮һbeat
	t=1����/tempo/48 = 60000ms/48/tempo = 1250/tempo  (ms)
	CMP =t/T = f/1000*1250*tempo 	(/1000��us����ms  ���ִ��ȳ���˷�ֹ���)
						 f/4*5*tempo
**************************************************************/
void BFTM1_TEMPO_Config(u8 	tempo)
{
//	CKCU_APBPerip1ClockConfig(CKCU_APBEN1_BFTM1, ENABLE);	
//	NVIC_EnableIRQ(BFTM1_IRQn);

//	BFTM_SetCompare(HT_BFTM1,(SystemCoreClock/4*5/tempo) );  // 1/48beat ��ʱ
//	BFTM_SetCounter(HT_BFTM1, 0);
//	BFTM_IntConfig(HT_BFTM1, ENABLE);
// 
//	BFTM_EnaCmd(HT_BFTM1, ENABLE);
  
  SYSTICK_CounterCmd(DISABLE);
  SYSTICK_CounterCmd(SYSTICK_COUNTER_CLEAR);
  SYSTICK_SetReloadValue(SystemCoreClock / 4 * 5 / tempo); // (CK_SYS/8) = 1s on chip
  SYSTICK_CounterCmd(ENABLE);  
}


//=============================================
//��������	User_TimerInterrupt_2ms
//���������� 
//timer 2ms��ʱ ����
//�жϺ������� BFTM0_IRQHandler 
//=============================================
void User_TimerInterrupt_2ms()
{
	__R_Time_Count++;

	__R_2ms_Count++;

	if(R_Func_Key_Dbnc &&(R_Func_Key_Dbnc<c_time_dbnc_max))
		R_Func_Key_Dbnc++;
	
//	if(R_Time_Delay_Demo_Replay!=c_time_init_delay_demo_replay)
//		R_Time_Delay_Demo_Replay++;
}

//=============================================
//�������ƣ�	User_TimerIRQ_Beat_48
//����������
//				���� beat timer ��ʱ ����
//				�жϺ������� BFTM1_IRQHandler 
//=============================================
void User_TimerIRQ_Beat_48()
{
	Midi_Beat48_Counter();
}









