//-----------------------------------------------------------------------------
#include "ht32.h"
#include "iap_handler.h"
#include "usb_detect.h"
#include "qspi_flash_MX25L12835F.h"

//-----------------------------------------------------------------------------
int main(void)
{
#if 0   //GPIO select iap or app mode
  u32 i;
  CKCU_PeripClockConfig_TypeDef CKCUClock = {{ 0 }}; 
  CKCUClock.Bit.BOOT_SEL_GPIO_CLK = 1;
  CKCU_PeripClockConfig(CKCUClock, ENABLE);

  GPIO_PullResistorConfig(BOOT_SEL_GPIO_PORT, BOOT_SEL_GPIO_PIN, GPIO_PR_UP);
  GPIO_InputConfig       (BOOT_SEL_GPIO_PORT, BOOT_SEL_GPIO_PIN, ENABLE);

  for(i=0; i<1000; i++);  //delay
  if (GPIO_ReadInBit(BOOT_SEL_GPIO_PORT, BOOT_SEL_GPIO_PIN) == SET)
#endif
  {
    if (rw(BOOT_MODE_ID_ADDR) != BOOT_MODE_IAP)
    {
      if (IAP_isAPValid() == TRUE)
      {
        WDT_DeInit();
        NVIC_SetVectorTable(NVIC_VECTTABLE_FLASH, IAP_APFLASH_START);   /* Set the Vector Table Offset      */
        IAP_GoCMD(IAP_APFLASH_START);
      }
    }
  }

  RETARGET_Configuration();
  USB_Detect_Init();
  IAP_Init();
  
  while (1)
  {
    if(HT_CKCU->APBCCR1 & (1 << 4))
      WDT_Restart();
    IAP_Handler();
    #if USB_DETECT_EN
    USB_Detect_Handler();
    #endif
  }
}
