#ifndef __BLEAPP_H
#define __BLEAPP_H

#include "ht32.h"
#include "bleuser.h"
#include "iap_handler.h"
#include "urbus.h"
#include "drv_ht82v73a.h"

#define MODULE_NUM            1       //用戶添加的模塊資訊，由軟體在編譯時修改本數據
#define BOARD_INF             0x01
#define CONFIGURATION_START   LOADER_FLASH_START + 0x404
#define TIMEOUT_VALUE         2       //ms
#define INTERVAL_VALUE        100      //ms
#define APP_USER_BUFFER_SIZE	128

typedef enum
{
  CONFIGURATION=0x01,   //發送配置資訊(BLE發送)
  BOARD,                //發送主板資訊(BLE發送)
  
  BUS_CMD_RES=0x10,     //發送主板命令+模組迴應(BLE發送)
  BUS_CMD=0x20,         //發送主板命令(BLE發送)
	
	USER_DEFINE = 0x40,		//發送用戶自定義數據的CMD	
  
  APP_ERROR=0x5A        //傳輸有誤
}ENUM_CMDTYPE;

typedef enum
{
  _CONFIGURATION=0x01,  //app請求發送配置資訊(BLE接收)
  _BOARD,               //app請求發送主板資訊(BLE接收)  
  
  _BUS_RES=0x10,        //app發送虛擬輸入模塊的值(BLE接收)  
  _LS_BUS_CMD=0x20,     //app請求向LS_BUS發送數據(BLE接收)
  _HS_BUS_CMD,          //app請求向HS_BUS發送數據(BLE接收)
  _HV_BUS_CMD,          //app請求向HV_BUS發送數據(BLE接收)
  
  _LS_BUS_ADV=0x30,     //app請求向LS_BUS廣播數據(BLE接收)
  _HS_BUS_ADV,          //app請求向HS_BUS廣播數據(BLE接收)
  _HV_BUS_ADV,          //app請求向HV_BUS廣播數據(BLE接收)
	
	_USER_DEFINE=0x40,		//接收app的用戶自定義數據
  
  _APP_ERROR=0x5A       //傳輸有誤
}ENUM_RESTYPE;

extern const u16 Module_Info[MODULE_NUM];
extern u8 len;
extern u8 cmd[256];
extern u8 buf[256];
extern u16 id;
extern u8 ret;
extern bool bResult;

bool BLE_SendAPPCMD(ENUM_CMDTYPE type, u16 uid, u8 CMD_len, u8* CMD_buffer, u8 RES_len, u8* RES_buffer);
ENUM_RESTYPE BLE_ParseData(u16* uid, u8* len, u8* buffer);
bool BLE_CheckInput(u16 uid, u8* len, u8* buffer);
void APP_Process(void);
#endif
