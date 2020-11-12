#include "ADC.h"

u16 ADC_Result[4];

void InterfaceADC_Init(void)
{
  CKCU_PeripClockConfig_TypeDef CKCUClock = {{0}};
  INTERFACE_ADC_CLOCK(CKCUClock) = 1;
  CKCU_PeripClockConfig(CKCUClock, ENABLE);
    
  /* ADCLK frequency is set to CK_AHB                                                                   */
  CKCU_SetADCPrescaler(CKCU_ADCPRE_DIV1);
  
  /* ONE_SHOT_MODE, Length 4                                                             */
  ADC_RegularGroupConfig(HT_ADC, ONE_SHOT_MODE, 4, 0);

  ADC_RegularChannelConfig(HT_ADC, ADC_CH_4, 0);
  ADC_RegularChannelConfig(HT_ADC, ADC_CH_5, 1);
  ADC_RegularChannelConfig(HT_ADC, ADC_CH_10, 2);
  ADC_RegularChannelConfig(HT_ADC, ADC_CH_11, 3);

  /* Set sampling time as 1.5 + 36 ADCCLK, Conversion = 12.5 + 1.5 + 36 = 50 ADCCLK                         */
  ADC_SamplingTimeConfig(HT_ADC, 36);

  /* Use software as ADC trigger source                                                               */
  ADC_RegularTrigConfig(HT_ADC, ADC_TRIG_SOFTWARE);
  
  /* Enable ADC cycle end of conversion interrupt                                                           */
  ADC_IntConfig(HT_ADC, ADC_INT_CYCLE_EOC, ENABLE);
  NVIC_EnableIRQ(ADC_IRQn);

  ADC_Cmd(HT_ADC, ENABLE);
}

void InterfaceADC_StartConversion(void)
{
  ADC_SoftwareStartConvCmd(HT_ADC, ENABLE);
}

void ADC_IRQHandler(void)
{
  if(ADC_GetFlagStatus(HT_ADC, ADC_FLAG_CYCLE_EOC))
  {
    ADC_ClearIntPendingBit(HT_ADC, ADC_INT_CYCLE_EOC);
    ADC_Result[0] = HT_ADC->DR[0];
    ADC_Result[1] = HT_ADC->DR[1];
    ADC_Result[2] = HT_ADC->DR[2];
    ADC_Result[3] = HT_ADC->DR[3];
  }
}
