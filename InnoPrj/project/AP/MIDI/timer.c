/**********************************************
文件名称：	timer.c
功能描述：	
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
*@file 基本功能定时器 16位
*@2ms定时	
***********************/
void BFTM_Config()
{
//	CKCU_APBPerip1ClockConfig(CKCU_APBEN1_BFTM0, ENABLE);	//CKCU_APBEN1_BFTM1 | 
	NVIC_EnableIRQ(BFTM0_IRQn);	

	BFTM_SetCompare(HT_BFTM0,(SystemCoreClock/2000*4));//2ms定时		BFTMx->CMP 32bit   sysclk 12000000UL 
 
	BFTM_SetCounter(HT_BFTM0, 0);	//  BFTMx->CNTR
	BFTM_IntConfig(HT_BFTM0, ENABLE);
	BFTM_EnaCmd(HT_BFTM0, ENABLE);
	 
	BFTM1_TEMPO_Config(__R_Tempo);
}

/*************************************************************
	根据速度设定定时器的时间，48分之一beat
	t=1分钟/tempo/48 = 60000ms/48/tempo = 1250/tempo  (ms)
	CMP =t/T = f/1000*1250*tempo 	(/1000是us换成ms  数字大先除后乘防止溢出)
						 f/4*5*tempo
**************************************************************/
void BFTM1_TEMPO_Config(u8 	tempo)
{
//	CKCU_APBPerip1ClockConfig(CKCU_APBEN1_BFTM1, ENABLE);	
//	NVIC_EnableIRQ(BFTM1_IRQn);

//	BFTM_SetCompare(HT_BFTM1,(SystemCoreClock/4*5/tempo) );  // 1/48beat 定时
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
//函数名：	User_TimerInterrupt_2ms
//功能描述： 
//timer 2ms定时 计数
//中断函数调用 BFTM0_IRQHandler 
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
//函数名称：	User_TimerIRQ_Beat_48
//功能描述：
//				节拍 beat timer 定时 计数
//				中断函数调用 BFTM1_IRQHandler 
//=============================================
void User_TimerIRQ_Beat_48()
{
	Midi_Beat48_Counter();
}









