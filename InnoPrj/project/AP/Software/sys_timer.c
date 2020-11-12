#include "sys_timer.h"

vu32 systemDelayCounter;

extern u8 GameMode;
extern vu32 BeatCounter;
u8 TinyTimeCnt = 0;
vu16 RandCnt = 0;

void Timer_Init(void)
{
  CKCU_PeripClockConfig_TypeDef CKCUClock = {{0}}; 
	TM_TimeBaseInitTypeDef 	SCTM_TimeBaseInitStructure;
  
	CKCUClock.Bit.AFIO              = 1;
  CKCUClock.Bit.CFG_SYSTEM_TM0		= 1;	
  CKCUClock.Bit.CFG_SYSTEM_TM1 		= 1;
  CKCU_PeripClockConfig(CKCUClock, ENABLE);  
  
	TM_DeInit(SYSTEM_TM0);
  TM_DeInit(SYSTEM_TM1);	
	SCTM_TimeBaseInitStructure.CounterMode   = TM_CNT_MODE_UP;
	SCTM_TimeBaseInitStructure.CounterReload = SystemCoreClock / 1000000 * SYSTEM_TIMER_SET - 1;
	SCTM_TimeBaseInitStructure.Prescaler		 = 1 - 1;//Prescaler DIV = 1
	SCTM_TimeBaseInitStructure.PSCReloadTime = TM_PSC_RLD_UPDATE;
	TM_TimeBaseInit(SYSTEM_TM0, &SCTM_TimeBaseInitStructure);  
  TM_TimeBaseInit(SYSTEM_TM1, &SCTM_TimeBaseInitStructure); 
  
  TM_IntConfig(SYSTEM_TM0, TM_INT_UEV, ENABLE);
  TM_IntConfig(SYSTEM_TM1, TM_INT_UEV, ENABLE);
  NVIC_EnableIRQ(SYSTEM_TM0_IRQn);
  NVIC_EnableIRQ(SYSTEM_TM1_IRQn);
//  TM_Cmd(SYSTEM_TM0, ENABLE);
  TM_Cmd(SYSTEM_TM1, ENABLE);
}

void SYSTEM_TM0_IRQHandler(void)
{
  if(TM_GetFlagStatus(SYSTEM_TM0, TM_FLAG_UEV))
  {                                           
    TM_ClearFlag(SYSTEM_TM0, TM_FLAG_UEV);
  }
}
void SYSTEM_TM1_IRQHandler(void)
{
  if(TM_GetFlagStatus(SYSTEM_TM1, TM_FLAG_UEV))
  {
    TM_ClearFlag(SYSTEM_TM1, TM_FLAG_UEV);
    if(systemDelayCounter) systemDelayCounter--;
		if (GameMode == 3 || GameMode == 1)
		{
			// is in Game mode 2
			if (TinyTimeCnt >= 9)
			{
				// plus 1 every 10 ms
				BeatCounter++;
				TinyTimeCnt = 0;
			}	
			else
			{
				TinyTimeCnt++;
			}
		}
		else
		{
			TinyTimeCnt = 0;
		}
		RandCnt = (RandCnt + 1) % 60000;
  }
}

void SystemDelay(u32 nms)
{
  TM_SetCounter(SYSTEM_TM1, 0);
  systemDelayCounter = nms;
  while(systemDelayCounter != 0);
}
