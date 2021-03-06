#include "usb_detect.h"
#include "urbus.h"

vu16 USB_insert_delay;
u8 USB_insert_flag = FALSE;

void USB_Detect_Init(void)
{
  CKCU_PeripClockConfig_TypeDef CKCUClock = {{0}};
  CKCUClock.Bit.PA                 = 1;
  CKCUClock.Bit.PB                 = 1;
  CKCUClock.Bit.PC                 = 1;
  CKCUClock.Bit.PD                 = 1;
  CKCUClock.Bit.BFTM0              = 1;
  CKCU_PeripClockConfig(CKCUClock, ENABLE);
  
  GPIO_DirectionConfig(USB_DETECT_PORT, USB_DETECT_PIN, GPIO_DIR_IN);
  GPIO_PullResistorConfig(USB_DETECT_PORT, USB_DETECT_PIN, GPIO_PR_DOWN);
  GPIO_InputConfig(USB_DETECT_PORT, USB_DETECT_PIN, ENABLE);
  
  GPIO_DirectionConfig(LED1_PORT, LED1_PIN, GPIO_DIR_OUT);
  
  GPIO_DirectionConfig(GPIO1_PORT, GPIO1_PIN, GPIO_DIR_OUT);
  GPIO_DirectionConfig(GPIO2_PORT, GPIO2_PIN, GPIO_DIR_OUT);
  GPIO_DirectionConfig(GPIO3_PORT, GPIO3_PIN, GPIO_DIR_OUT);
  GPIO_DirectionConfig(GPIO4_PORT, GPIO4_PIN, GPIO_DIR_OUT);
  GPIO_DirectionConfig(GPIO5_PORT, GPIO5_PIN, GPIO_DIR_OUT);
  GPIO_DirectionConfig(GPIO6_PORT, GPIO6_PIN, GPIO_DIR_OUT);
  GPIO_DirectionConfig(GPIO7_PORT, GPIO7_PIN, GPIO_DIR_OUT);
  GPIO_DirectionConfig(GPIO8_PORT, GPIO8_PIN, GPIO_DIR_OUT);
  GPIO_DirectionConfig(GPIO9_PORT, GPIO9_PIN, GPIO_DIR_OUT);
  GPIO_DirectionConfig(GPIO10_PORT, GPIO10_PIN, GPIO_DIR_OUT);
}

void USB_Detect_Handler(void)
{
  CKCU_PeripClockConfig_TypeDef CKCUClock = {{0}};
  static u8 stage = 0;
  
  switch(stage)
  {
    case 0:
      if(USB_DETECT_PORT->DINR & USB_DETECT_PIN)
      {
        USB_insert_delay = USB_INSERT_DELAY*1000/BFTM_TIME_BASE;
        stage = 1;
      }
      break;
    
    case 1:
      if(USB_insert_delay == 0)
      {
        if(USB_DETECT_PORT->DINR & USB_DETECT_PIN)
        {
          RETARGET_Configuration();
          USB_insert_flag = TRUE;
          stage = 2;
        }
        else stage = 0;
      }
      break;
      
    case 2:
      if(!(USB_DETECT_PORT->DINR & USB_DETECT_PIN))
      {
        USB_insert_delay = USB_INSERT_DELAY*1000/BFTM_TIME_BASE;
        stage = 3;
      }
      break;
      
    case 3:
      if(USB_insert_delay == 0)
      {
        if(!(USB_DETECT_PORT->DINR & USB_DETECT_PIN))
        {
          USBD_DPpullupCmd(DISABLE);
          NVIC_DisableIRQ(USB_IRQn);
          CKCUClock.Bit.USBD = 1;
          CKCU_PeripClockConfig(CKCUClock, DISABLE);
          USB_insert_flag = FALSE;
          stage = 0;
        }
        else stage = 2;        
      }
  }
}
