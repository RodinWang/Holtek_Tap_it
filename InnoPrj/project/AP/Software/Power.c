#include "Power.h"
#include "key_func.h"

static void Wakeup_Delay(u32 count)
{
  __disable_irq();
  for(u32 i = 0;i < count;i++)
  {
    __nop();
  }
  __enable_irq();
}

/*********************************************************************************************************//**
 * @brief 進入低功耗模式（deepSleep1）前初始化，關閉耗電設備並將三條bus的RX配置為EXTI喚醒功能
 * @param None
 * @retval None
 ************************************************************************************************************/
void LowPower_Init(void)
{
  CKCU_PeripClockConfig_TypeDef CKCUClock = {{0}};
  CKCUClock.Bit.EXTI         = 1;
  CKCU_PeripClockConfig(CKCUClock, ENABLE);

  /*HV RX pin配置為輸入上拉模式並且使能輸入功能*/
  AFIO_GPxConfig(HV_BUS_RX_GPIO_ID, HV_BUS_RX_AFIO_PIN, AFIO_FUN_GPIO);
  GPIO_DirectionConfig(HV_BUS_RX_PORT, HV_BUS_RX_GPIO_PIN, GPIO_DIR_IN);
  GPIO_PullResistorConfig(HV_BUS_RX_PORT, HV_BUS_RX_GPIO_PIN, GPIO_PR_UP);
  GPIO_InputConfig(HV_BUS_RX_PORT, HV_BUS_RX_GPIO_PIN, ENABLE);
  /*HV RX pin配置為EXTI喚醒（低電平有效）*/
  AFIO_EXTISourceConfig(HV_BUS_RX_AFIO_EXTI_CH, HV_BUS_RX_EXTI_ESS);
  EXTI_WakeupEventConfig(HV_BUS_RX_EXTI_CH, EXTI_WAKEUP_LOW_LEVEL, ENABLE);    
  /*HS RX pin配置為輸入上拉模式並且使能輸入功能*/
  AFIO_GPxConfig(HS_BUS_RX_GPIO_ID, HS_BUS_RX_AFIO_PIN, AFIO_FUN_GPIO);
  GPIO_DirectionConfig(HS_BUS_RX_PORT, HS_BUS_RX_GPIO_PIN, GPIO_DIR_IN);
  GPIO_PullResistorConfig(HS_BUS_RX_PORT, HS_BUS_RX_GPIO_PIN, GPIO_PR_UP);
  GPIO_InputConfig(HS_BUS_RX_PORT, HS_BUS_RX_GPIO_PIN, ENABLE);
  /*HS RX pin配置為EXTI喚醒（低電平有效）*/
  AFIO_EXTISourceConfig(HS_BUS_RX_AFIO_EXTI_CH, HS_BUS_RX_EXTI_ESS);
  EXTI_WakeupEventConfig(HS_BUS_RX_EXTI_CH, EXTI_WAKEUP_LOW_LEVEL, ENABLE);
  /*LS RX pin配置為輸入上拉模式並且使能輸入功能*/
  AFIO_GPxConfig(LS_BUS_RX_GPIO_ID, LS_BUS_RX_AFIO_PIN, AFIO_FUN_GPIO);
  GPIO_DirectionConfig(LS_BUS_RX_PORT, LS_BUS_RX_GPIO_PIN, GPIO_DIR_IN);
  GPIO_PullResistorConfig(LS_BUS_RX_PORT, LS_BUS_RX_GPIO_PIN, GPIO_PR_UP);
  GPIO_InputConfig(LS_BUS_RX_PORT, LS_BUS_RX_GPIO_PIN, ENABLE);
  /*LS RX pin配置為EXTI喚醒（低電平有效）*/
  AFIO_EXTISourceConfig(LS_BUS_RX_AFIO_EXTI_CH, LS_BUS_RX_EXTI_ESS);
  EXTI_WakeupEventConfig(LS_BUS_RX_EXTI_CH, EXTI_WAKEUP_LOW_LEVEL, ENABLE);

  /*使能EXTI事件喚醒功能，并使能喚醒中斷*/
  HT_EXTI->WAKUPFLG = 0xFFFF;
  EXTI_WakeupEventIntConfig(ENABLE);
  NVIC_SetPriority(EVWUP_IRQn, 3);
  NVIC_EnableIRQ(EVWUP_IRQn);
  
  /*關閉其他板載設備*/
  Led_OFF;//LED1關閉
  SPI_SoftwareSELCmd(FLASH_QSPI, SPI_SEL_INACTIVE);//Flash失能
  L_Audio_Stop();//停止一切聲音
  BLE_Power_Down();//BLE進入掉電模式
  
  /*進入DS1低功耗模式*/
  ADC_Cmd(HT_ADC, DISABLE);//進入低功耗前應關閉ADC,否則會產生多餘功耗
}

/*********************************************************************************************************//**
 * @brief MCU被喚醒後，對其他兩條bus發送low pulse喚醒模組
 * @param bus: MCU當前是被哪條bus喚醒的
 * @retval None
 * @note 僅在本文件內使用
 ************************************************************************************************************/
static void Wakeup_OtherBUS(u8 bus)
{
  switch(bus)
  {
    case HS_BUS_NUM:
      /*其他兩條bus輸出對應1bit baudrate的low pulse*/
      GPIO_DirectionConfig(LS_BUS_RX_PORT, LS_BUS_RX_GPIO_PIN, GPIO_DIR_OUT);
      GPIO_ClearOutBits(LS_BUS_RX_PORT, LS_BUS_RX_GPIO_PIN);
      Wakeup_Delay(500);
      GPIO_SetOutBits(LS_BUS_RX_PORT, LS_BUS_RX_GPIO_PIN);
    
      GPIO_DirectionConfig(HV_BUS_RX_PORT, HV_BUS_RX_GPIO_PIN, GPIO_DIR_OUT);
      GPIO_ClearOutBits(HV_BUS_RX_PORT, HV_BUS_RX_GPIO_PIN);
      Wakeup_Delay(40);
      GPIO_SetOutBits(HV_BUS_RX_PORT, HV_BUS_RX_GPIO_PIN);   
      break;
    case LS_BUS_NUM:
      /*其他兩條bus輸出對應1bit baudrate的low pulse*/
      GPIO_DirectionConfig(HS_BUS_RX_PORT, HS_BUS_RX_GPIO_PIN, GPIO_DIR_OUT);
      GPIO_ClearOutBits(HS_BUS_RX_PORT, HS_BUS_RX_GPIO_PIN);
      Wakeup_Delay(40);
      GPIO_SetOutBits(HS_BUS_RX_PORT, HS_BUS_RX_GPIO_PIN);
      GPIO_DirectionConfig(HV_BUS_RX_PORT, HV_BUS_RX_GPIO_PIN, GPIO_DIR_OUT);
      GPIO_ClearOutBits(HV_BUS_RX_PORT, HV_BUS_RX_GPIO_PIN);
      Wakeup_Delay(40);
      GPIO_SetOutBits(HV_BUS_RX_PORT, HV_BUS_RX_GPIO_PIN);    
      break;
    case HV_BUS_NUM:
      /*其他兩條bus輸出對應1bit baudrate的low pulse*/
      GPIO_DirectionConfig(LS_BUS_RX_PORT, LS_BUS_RX_GPIO_PIN, GPIO_DIR_OUT);
      GPIO_ClearOutBits(LS_BUS_RX_PORT, LS_BUS_RX_GPIO_PIN);
      Wakeup_Delay(500);
      GPIO_SetOutBits(LS_BUS_RX_PORT, LS_BUS_RX_GPIO_PIN);
      GPIO_DirectionConfig(HS_BUS_RX_PORT, HS_BUS_RX_GPIO_PIN, GPIO_DIR_OUT);
      GPIO_ClearOutBits(HS_BUS_RX_PORT, HS_BUS_RX_GPIO_PIN);
      Wakeup_Delay(40);
      GPIO_SetOutBits(HS_BUS_RX_PORT, HS_BUS_RX_GPIO_PIN);
    default:
      break;
  }
}

/*********************************************************************************************************//**
 * @brief 喚醒中斷服務函數，恢復睡眠前的功能
 * @param None
 * @retval None
 * @note 在被喚醒後第一時間進入該中斷服務函數
 ************************************************************************************************************/
void EVWUP_IRQHandler(void)
{  
  /*判斷是哪一種EXTI的中斷并清除中斷標誌位*/  
  EXTI_WakeupEventIntConfig(DISABLE);
  if(EXTI_GetWakeupFlagStatus(HS_BUS_RX_EXTI_CH))
  {
    EXTI_ClearWakeupFlag(HS_BUS_RX_EXTI_CH);
    Wakeup_OtherBUS(HS_BUS_NUM);
  }
  else if(EXTI_GetWakeupFlagStatus(LS_BUS_RX_EXTI_CH))
  {
    EXTI_ClearWakeupFlag(LS_BUS_RX_EXTI_CH);
    Wakeup_OtherBUS(LS_BUS_NUM);
  }
  else
  {
    EXTI_ClearWakeupFlag(HV_BUS_RX_EXTI_CH);
    Wakeup_OtherBUS(HV_BUS_NUM);
  }
  /*三條bus的RX pin恢復為UART RX功能*/
  GPIO_PullResistorConfig(HS_BUS_RX_PORT, HS_BUS_RX_GPIO_PIN, GPIO_PR_DISABLE);
  GPIO_PullResistorConfig(LS_BUS_RX_PORT, LS_BUS_RX_GPIO_PIN, GPIO_PR_DISABLE);
  GPIO_PullResistorConfig(HV_BUS_RX_PORT, HV_BUS_RX_GPIO_PIN, GPIO_PR_DISABLE);
  AFIO_GPxConfig(HS_BUS_RX_GPIO_ID, HS_BUS_RX_AFIO_PIN, HS_BUS_RX_AFIO_MODE);
  AFIO_GPxConfig(LS_BUS_RX_GPIO_ID, LS_BUS_RX_AFIO_PIN, LS_BUS_RX_AFIO_MODE);
  AFIO_GPxConfig(HV_BUS_RX_GPIO_ID, HV_BUS_RX_AFIO_PIN, HV_BUS_RX_AFIO_MODE);
  /*恢復其他板載設備*/
  Led_ON;
  BLE_Power_Up();
  ADC_Cmd(HT_ADC, ENABLE);
  /*延時12bit baudrate=9600後退出*/
  Wakeup_Delay(1500);
  SYSTICK_CounterCmd(SYSTICK_COUNTER_DISABLE);
  return;
}

/********************************************System Command**************************************************/

void Bus_SystemResetAll(void)
{
  u8 cmd = 0x00;//0x00為reset命令
  LS_Transmit(0x000, 1, &cmd);
  HS_Transmit(0x000, 1, &cmd);
  HV_Transmit(0x000, 1, &cmd);
}

void Bus_SystemStandby(void)
{
  u8 cmd = 0x01;//0x01為standby命令
  LS_Transmit(0x000, 1, &cmd);
  HS_Transmit(0x000, 1, &cmd);
  HV_Transmit(0x000, 1, &cmd);
  
  Enter_LowPower();
}

void Bus_SystemAction(void)
{
  u8 cmd = 0x02;//0x02為action命令
  LS_Transmit(0x000, 1, &cmd);
  HS_Transmit(0x000, 1, &cmd);
  HV_Transmit(0x000, 1, &cmd);  
}
