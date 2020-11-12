#include "GPIO.h"
#include "i2c.h"
#include "ADC.h"

void GPIO_PortInit(void)
{
  CKCU_PeripClockConfig_TypeDef CKCUClock = {{0}};
  CKCUClock.Bit.PA     = 1;
  CKCUClock.Bit.PB     = 1;
  CKCUClock.Bit.PC     = 1;
  CKCUClock.Bit.PD     = 1;
  CKCUClock.Bit.AFIO   = 1;
  CKCU_PeripClockConfig(CKCUClock, ENABLE);
  
  GPIO_DirectionConfig(LED1_PORT, LED1_PIN, GPIO_DIR_OUT);
  Led_ON;
  
  GPIO_DirectionConfig(LGPIO1_PORT, LGPIO1_PIN, GPIO_DIR_OUT);
  GPIO_DirectionConfig(LGPIO2_PORT, LGPIO2_PIN, GPIO_DIR_OUT);
  GPIO_DirectionConfig(LGPIO3_PORT, LGPIO3_PIN, GPIO_DIR_OUT);
  GPIO_DirectionConfig(LGPIO4_PORT, LGPIO4_PIN, GPIO_DIR_OUT);
  GPIO_DirectionConfig(LGPIO5_PORT, LGPIO5_PIN, GPIO_DIR_OUT);
  GPIO_DirectionConfig(LGPIO6_PORT, LGPIO6_PIN, GPIO_DIR_OUT);
  
  GPIO_DirectionConfig(RGPIO1_PORT, RGPIO1_PIN, GPIO_DIR_OUT);
  GPIO_DirectionConfig(RGPIO2_PORT, RGPIO2_PIN, GPIO_DIR_OUT);
  GPIO_DirectionConfig(RGPIO3_PORT, RGPIO3_PIN, GPIO_DIR_OUT);
  GPIO_DirectionConfig(RGPIO4_PORT, RGPIO4_PIN, GPIO_DIR_OUT);
  GPIO_DirectionConfig(RGPIO5_PORT, RGPIO5_PIN, GPIO_DIR_OUT);
  GPIO_DirectionConfig(RGPIO6_PORT, RGPIO6_PIN, GPIO_DIR_OUT);

  AFIO_GPxConfig(I2C0_SCL_GPIO_ID, I2C0_SCL_AFIO_PIN, I2C0_SCL_AFIO_MODE);
  AFIO_GPxConfig(I2C0_SDA_GPIO_ID, I2C0_SDA_AFIO_PIN, I2C0_SDA_AFIO_MODE);
  
  AFIO_GPxConfig(AN11_GPIO_ID, AN11_AFIO_PIN, AN11_AFIO_MODE);
  AFIO_GPxConfig(AN10_GPIO_ID, AN10_AFIO_PIN, AN10_AFIO_MODE);
  AFIO_GPxConfig(AN5_GPIO_ID, AN5_AFIO_PIN, AN5_AFIO_MODE);
  AFIO_GPxConfig(AN4_GPIO_ID, AN4_AFIO_PIN, AN4_AFIO_MODE);

  I2C0_Init();
  InterfaceADC_Init();
}

//vu16 USB_insert_delay;
//u8 USB_insert_flag = FALSE;
//void USB_Detect_Handler(void)
//{
//  CKCU_PeripClockConfig_TypeDef CKCUClock = {{0}};
//  static u8 stage = 0;
//  
//  switch(stage)
//  {
//    case 0:
//      if(USB_DETECT_PORT->DINR & USB_DETECT_PIN)
//      {
//        USB_insert_delay = USB_INSERT_DELAY*1000/BFTM_TIME_BASE;
//        stage = 1;
//      }
//      break;
//    
//    case 1:
//      if(USB_insert_delay == 0)
//      {
//        if(USB_DETECT_PORT->DINR & USB_DETECT_PIN)
//        {
//          RETARGET_Configuration();
//          USB_insert_flag = TRUE;
//          stage = 2;
//        }
//        else stage = 0;
//      }
//      break;
//      
//    case 2:
//      if(!(USB_DETECT_PORT->DINR & USB_DETECT_PIN))
//      {
//        USB_insert_delay = USB_INSERT_DELAY*1000/BFTM_TIME_BASE;
//        stage = 3;
//      }
//      break;
//      
//    case 3:
//      if(USB_insert_delay == 0)
//      {
//        if(!(USB_DETECT_PORT->DINR & USB_DETECT_PIN))
//        {
//          USBD_DPpullupCmd(DISABLE);
//          NVIC_DisableIRQ(USB_IRQn);
//          CKCUClock.Bit.USBD = 1;
//          CKCU_PeripClockConfig(CKCUClock, DISABLE);
//          USB_insert_flag = FALSE;
//          stage = 0;
//        }
//        else stage = 2;        
//      }
//  }
//}
