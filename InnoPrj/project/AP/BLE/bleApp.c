#include "bleApp.h"
#include <string.h>

const u16 Module_Info[MODULE_NUM] __attribute__((at(CONFIGURATION_START))) = 
{
	0x0111,
	0x0112,
	0x0113,
	0x0114,
	0x0115,
	0x0116,
	0x01D1,
	0x0211,
	0x0212,
	0x0213,
	0x0214,
};//用戶添加的模塊資訊，由軟體在編譯時修改本數據
vu32 BLE_APPConnect;
bool APP_FirstConnect=FALSE;
u8 AppUserLength;
u8 AppUserBuffer[APP_USER_BUFFER_SIZE];

/* private function */
#define Set_BLEConnect(nms) (BLE_APPConnect = nms*1000/BFTM_TIME_BASE)
#define Check_BLEConnect()    (BLE_APPConnect == 0)

/*********************************************************************************************************//**
 * @brief 主控板向APP發送命令
 * @param type: 命令類型，由ENUM_CMDTYPE枚舉類型定義
 * @param uid: 模塊的uid = MID << 4 | EID
 * @param CMD_len: CMD的長度
 * @param CMD_buffer: 包括命令字節+參數
 * @param RES_len: RES的長度
 * @param RES_buffer: 包括狀態字節+參數
 * @retval true:執行成功 false:執行失敗
 * @example   HS_Transmit(0x123, 10, cmd);
              HS_Receive(&id, &len, buf);
              BLE_SendAPPCMD(CMD_RES, 0x123, 10, cmd, len, buf);
 ************************************************************************************************************/
bool BLE_SendAPPCMD(ENUM_CMDTYPE type, u16 uid, u8 CMD_len, u8* CMD_buffer, u8 RES_len, u8* RES_buffer)
{
#if USE_BLEAPP_ENABLE
  u8 temp_buff[128];
  u8 len;
  u8 send_index=0;
  
  if(BLEoperate.connect == FALSE)return FALSE;
  
  switch(type)
  {
    case CONFIGURATION:
      len = 2*MODULE_NUM+1;
      temp_buff[send_index++] = (u8)type;
      temp_buff[send_index++] = len;
      temp_buff[send_index++] = MODULE_NUM;
      for(u8 i = 0;i < MODULE_NUM;i++)
      {
        temp_buff[send_index++] = *((u16*)(CONFIGURATION_START)+i) >> 4;
        temp_buff[send_index++] = *((u16*)(CONFIGURATION_START)+i) & 0x000F;
      }
      break;
    
    case BOARD:
      len = 1;
      temp_buff[send_index++] = (u8)type;
      temp_buff[send_index++] = len;
      temp_buff[send_index++] = BOARD_INF;
      break;
    
    case BUS_CMD_RES:    
      len = CMD_len+RES_len+2;
      temp_buff[send_index++] = (u8)type;
      temp_buff[send_index++] = len;
      temp_buff[send_index++] = uid >> 4;
      temp_buff[send_index++] = uid & 0x000F;
      memcpy(&temp_buff[send_index], CMD_buffer, CMD_len);
      send_index += CMD_len;
      memcpy(&temp_buff[send_index], RES_buffer, RES_len);
      send_index += RES_len;
      break;
    
    case BUS_CMD:      
      len = CMD_len+2;
      temp_buff[send_index++] = (u8)type;
      temp_buff[send_index++] = len;
      temp_buff[send_index++] = uid >> 4;
      temp_buff[send_index++] = uid & 0x000F;
      memcpy(&temp_buff[send_index], CMD_buffer, CMD_len);
      send_index += CMD_len;
    default:
      break;
  }
  if(BLE_Write_Buffer(temp_buff, send_index) == send_index)
    return TRUE;
  else
#endif
    return FALSE;
}

/*********************************************************************************************************//**
 * @brief 主控板解析APP返回的數據
 * @param uid: 儲存uid的地址
 * @param len: 儲存長度的地址
 * @param buffer: 包括狀態字節+參數
 * @retval 當前接收到數據的類型，由ENUM_CMDTYPE枚舉類型定義
 * @note 對APP虛擬輸入模塊(0x10)的數據處理由BLE_CheckInput函數執行
 * @example   HS_Transmit(0x123, 10, cmd);
              HS_Receive(&id, &len, buf);
              BLE_SendAPPCMD(CMD_RES, 0x123, 10, cmd, len, buf);
              if(BLE_ParseData(&id, &len, buf) == BUS_RES){//輸入模塊User code}
              or if(BLE_ParseData(&id, &len, buf) == BUS_RES)
 ************************************************************************************************************/
ENUM_RESTYPE BLE_ParseData(u16* uid, u8* len, u8* buffer)
{
  u8 cmd = 0x5A;
#if USE_BLEAPP_ENABLE
  u8 temp_buff[128];
	
	if(BLE_CheckInput(*uid, len, buffer) == TRUE)
	return _BUS_RES;
  
  BLE_Read_Buffer(&cmd, 1);
  switch(cmd)
  {
    case _CONFIGURATION:
    case _BOARD:      
      BLE_Read_Buffer(len, 1);
      break;
    
    case _LS_BUS_CMD:
    case _HS_BUS_CMD:
    case _HV_BUS_CMD:
    case _LS_BUS_ADV:
    case _HS_BUS_ADV:
    case _HV_BUS_ADV:       
      BLE_Read_Buffer(len, 1);
      if(Get_BLE_RX_Length() < *len){Buffer_Discard(&BLE_rx_index); return _APP_ERROR;}
      BLE_Read_Buffer(temp_buff, 2);
      *uid = (u16)temp_buff[0] << 4 | temp_buff[1];
      BLE_Read_Buffer(buffer, *len-2);
      break;
		
		case _USER_DEFINE:
			BLE_Read_Buffer(len, 1);
			if(Get_BLE_RX_Length() < *len){Buffer_Discard(&BLE_rx_index); return _APP_ERROR;}
			BLE_Read_Buffer(buffer, *len);
			break;
    
    case _BUS_RES://虛擬輸入模塊的value，應該重新還原數據，等待下次調用BLE_CheckInput去解析
      temp_buff[0] = cmd;
      BLE_Read_Buffer(&temp_buff[1], 1);
      BLE_Read_Buffer(&temp_buff[2], temp_buff[1]);
      Buffer_WriteExt(&BLE_rx_index, temp_buff, temp_buff[1]+2);
		
			cmd = 0x5A;
      
    default:
      break;
  }
#endif
  return (ENUM_RESTYPE)cmd;
}

/*********************************************************************************************************//**
 * @brief 在BLE接收緩存區中查詢指定虛擬模組的數據(僅針對輸入類型模組有效)
 * @param uid:模組ID
 * @param len:存放長度的地址
 * @param buffer:存放數據的緩存
 * @retval TRUE:查詢到 FALSE:未查詢到
 * @note  此函數從BLE_ParseData獨立出來，為查詢虛擬按鍵而編寫;
          如果緩存區同時存在多組數據，取最前者;
          查詢到並且讀出來之後，會將本組數據刪除
 ************************************************************************************************************/
bool BLE_CheckInput(u16 uid, u8* len, u8* buffer)
{
#if USE_BLEAPP_ENABLE
  u8 temp_buffer[128] = {0};
  u32 index = 0,valid_num = Get_BLE_RX_Length();
  u16 id_res;
  
	if(uid == 0x00)return FALSE;
  while(index < valid_num)
  {
    BLE_Read_Buffer(&temp_buffer[0], 2);
    index += 2;
    BLE_Read_Buffer(&temp_buffer[2], temp_buffer[1]);
    index += temp_buffer[1];
    if(temp_buffer[0] == (u8)_BUS_RES)
    {
      id_res = (u16)temp_buffer[2] << 4 | temp_buffer[3];
      if(id_res == uid)
      {
        *len = temp_buffer[1] - 2;
        memcpy(buffer, &temp_buffer[4], *len);
        return TRUE;
      }
    }
    Buffer_WriteExt(&BLE_rx_index, temp_buffer, temp_buffer[1]+2);//將不匹配的數據還原
  }
#endif
  return FALSE;
}

/*********************************************************************************************************//**
 * @brief 對APP傳過來的請求進行響應
 * @param None
 * @retval None
 * @note  建議放在main loop中進行輪詢，
          如果APP無命令要傳輸，則直接退出該函數，
          如果有對模塊的命令，則調用BUS_Transmit進行傳遞。
 ************************************************************************************************************/
void BLE_CheckAPP(void)
{
  u16 uid = 0;
  u8 CMD_len, RES_len;
  u8 CMD_buffer[128], RES_buffer[128];
  ENUM_RESTYPE ret_type;
  
  ret_type = BLE_ParseData(&uid, &CMD_len, CMD_buffer);
  switch(ret_type)
  {
    case _CONFIGURATION:
      BLE_SendAPPCMD(CONFIGURATION, uid, CMD_len, CMD_buffer, RES_len, RES_buffer);
      break;
    
    case _BOARD:
      BLE_SendAPPCMD(BOARD, uid, CMD_len, CMD_buffer, RES_len, RES_buffer);
      break;
    
    case _LS_BUS_CMD:
      LS_Transmit(uid, CMD_len-2, CMD_buffer);
      LS_Receive(&uid, &RES_len, RES_buffer);
      BLE_SendAPPCMD(BUS_CMD_RES, uid, CMD_len-2, CMD_buffer, RES_len, RES_buffer);
      break;
    
    case _LS_BUS_ADV:
      LS_Transmit(uid, CMD_len-2, CMD_buffer);
      break;
    
    case _HS_BUS_CMD:
      HS_Transmit(uid, CMD_len-2, CMD_buffer);
      HS_Receive(&uid, &RES_len, RES_buffer);
      BLE_SendAPPCMD(BUS_CMD_RES, uid, CMD_len-2, CMD_buffer, RES_len, RES_buffer);
      break;
    
    case _HS_BUS_ADV:
      HS_Transmit(uid, CMD_len-2, CMD_buffer);
      break;

    case _HV_BUS_CMD:
      HV_Transmit(uid, CMD_len-2, CMD_buffer);
      HV_Receive(&uid, &RES_len, RES_buffer);
      BLE_SendAPPCMD(BUS_CMD_RES, uid, CMD_len-2, CMD_buffer, RES_len, RES_buffer);
      break;
    
    case _HV_BUS_ADV:
      HV_Transmit(uid, CMD_len-2, CMD_buffer);
		
		case _USER_DEFINE:
			AppUserLength = CMD_len;
			memcpy(AppUserBuffer, CMD_buffer, AppUserLength);
    
    default:
      break;      
  }
}

/*********************************************************************************************************//**
 * @brief APP過程處理函數
 * @param None
 * @retval None
 * @note  建議放在main loop中進行輪詢，
          如果APP無命令要傳輸，則直接退出該函數，
          如果有對模塊的命令，則調用BUS_Transmit進行傳遞。
 ************************************************************************************************************/
void APP_Process(void)
{
#if USE_BLEAPP_ENABLE
  static bool start = FALSE;
  u8 temp_buffer[32];
  
  //輪詢解析APP的請求
  BLE_CheckAPP();//暫時不需要
  
  //第一次連接時發送資訊
  if(APP_FirstConnect)
  {
    APP_FirstConnect = FALSE;
    Set_BLEConnect(1000);//第一次連接延時1s後向APP發送資訊
    start = TRUE;
  }
  else if(start == TRUE && Check_BLEConnect())
  {
    start = FALSE;
    BLE_SendAPPCMD(BOARD, 0x00, 0, temp_buffer, 0, temp_buffer);
    BLE_SendAPPCMD(CONFIGURATION, 0x00, 0, temp_buffer, 0, temp_buffer);
  }
#endif
}

/*********************************************************************************************************//**
 * @brief 讀取用戶自定義數據區
 * @param 儲存數據的buffer
 * @retval 讀取到的數量
 * @note  主板接收完畢自定義數據後，請及時調用該函數讀取走，否則會在下一筆數據到來時發生覆蓋；
					調用該函數後會自動將緩存區清空；
					請給參數buffer預留足夠的空間，否則有可能數組溢出導致程式死機
 ************************************************************************************************************/
u8 Read_UserDefineBuffer(u8* buffer)
{
	u8 len_temp = AppUserLength;
	if(len_temp == 0)
	{
		return 0;
	}
	else
	{
		memcpy(buffer, AppUserBuffer, len_temp);
		AppUserLength = 0;
		return len_temp;
	}
}

/*********************************************************************************************************//**
 * @brief 發送用戶自定義數據
 * @param 儲存數據的buffer
 * @param 發送的長度
 * @retval None
 * @note 
 ************************************************************************************************************/
void Send_UserDefineBuffer(u8* buffer, u8 len)
{
	u8 buffer_temp[APP_USER_BUFFER_SIZE + 2];
	
	buffer_temp[0] = (u8)USER_DEFINE;
	buffer_temp[1] = len;
	memcpy(&buffer_temp[2], buffer, len);
	BLE_Write_Buffer(buffer_temp, 2+len);
}
