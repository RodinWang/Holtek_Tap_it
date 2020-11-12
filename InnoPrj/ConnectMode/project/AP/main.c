 /************************************************************************************************************
 * @attention
 *
 * Firmware Disclaimer Information
 *
 * 1. The customer hereby acknowledges and agrees that the program technical documentation, including the
 *    code, which is supplied by Holtek Semiconductor Inc., (hereinafter referred to as "HOLTEK") is the
 *    proprietary and confidential intellectual property of HOLTEK, and is protected by copyright law and
 *    other intellectual property laws.
 *
 * 2. The customer hereby acknowledges and agrees that the program technical documentation, including the
 *    code, is confidential information belonging to HOLTEK, and must not be disclosed to any third parties
 *    other than HOLTEK and the customer.
 *
 * 3. The program technical documentation, including the code, is provided "as is" and for customer reference
 *    only. After delivery by HOLTEK, the customer shall use the program technical documentation, including
 *    the code, at their own risk. HOLTEK disclaims any expressed, implied or statutory warranties, including
 *    the warranties of merchantability, satisfactory quality and fitness for a particular purpose.
 *
 * <h2><center>Copyright (C) Holtek Semiconductor Inc. All rights reserved</center></h2>
 ************************************************************************************************************/
//-----------------------------------------------------------------------------
#include "ht32.h"
#include "urbus.h"
#include "qspi_flash_MX25L12835F.h"
#include "iap_handler.h"
#include "GPIO.h"
#include "user.h"
#include "bleApp.h"
#include "Power.h"
#include "MPU6050.h"
#include <string.h>
//-----------------------------------------------------------------------------
int main(void)
{
  ////////////////////////////////////////////////////
  //System Initialization Area
  ////////////////////////////////////////////////////
  RETARGET_Configuration();   //USB Initialization
  GPIO_PortInit();            //GPIO pin & related basic function Initialization
  URBus_Init();               //UART Bus Initialization
  IAP_Init();                 //IAP flag & buffer Initialization
  BC76XX_Init();              //BLE Initialization
  MIDI_FuncInit();            //MIDI Initialization
  
  ////////////////////////////////////////////////////
  //User Initialization Area
  ////////////////////////////////////////////////////

  while (1)
  {
    ////////////////////////////////////////////////////
    //System Coding Area
    ////////////////////////////////////////////////////
    if(HT_CKCU->APBCCR1 & (1 << 4))
      WDT_Restart();
    Handler();
    BLE_process();
    MIDI_Process();
    APP_Process();
    
    ////////////////////////////////////////////////////
    //User Coding Area
    ////////////////////////////////////////////////////
		
  }
}
