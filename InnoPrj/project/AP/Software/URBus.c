#include "urbus.h"
#include "iap_handler.h"
#include "bleApp.h"
#include "key_func.h"

HT_USART_TypeDef* BUS_Port_table[3] = {LS_BUS_PORT, HS_BUS_PORT, HV_BUS_PORT};
/* public buffer defination */
u8 BUS_RX_Buffer[3][BUS_RX_BUFFER_SIZE + BUFFER_ADDITIONAL_SIZE];
Buffer_TypeDef BUS_RX_Index[3];

u16 busRXCount[3];

static vu16 Bus_Timeout_Value[3];
/* private function */
#define Bus_Set_Timeout(bus, nms) (Bus_Timeout_Value[bus] = nms*1000/BFTM_TIME_BASE)
#define Check_Bus_Timeout(bus)    (Bus_Timeout_Value[bus] == 0)
static bool Parse_Retval(u8 bus, u16* uid, u8* len, u8* par);

/*********************************************************************************************************//**
 * @brief Initialize UR Bus,include COLOCK/BFTM1/RX_BUFFER/IO and UART
 * @param None
 * @retval None
 ************************************************************************************************************/
void URBus_Init(void)
{
  USART_InitTypeDef USART_InitStructure;
  CKCU_PeripClockConfig_TypeDef CKCUClock = {{0}};
  CKCUClock.Bit.BFTM1    = 1;
  HS_BUS_CLK(CKCUClock)  = 1;
  LS_BUS_CLK(CKCUClock)  = 1;
  HV_BUS_CLK(CKCUClock)  = 1;
  CKCU_PeripClockConfig(CKCUClock, ENABLE);
  
  Buffer_Init(&BUS_RX_Index[LS_BUS_NUM], BUS_RX_Buffer[LS_BUS_NUM], BUS_RX_BUFFER_SIZE+BUFFER_ADDITIONAL_SIZE);
  Buffer_Init(&BUS_RX_Index[HS_BUS_NUM], BUS_RX_Buffer[HS_BUS_NUM], BUS_RX_BUFFER_SIZE+BUFFER_ADDITIONAL_SIZE);
  Buffer_Init(&BUS_RX_Index[HV_BUS_NUM], BUS_RX_Buffer[HV_BUS_NUM], BUS_RX_BUFFER_SIZE+BUFFER_ADDITIONAL_SIZE);
  
  BFTM_SetCompare(HT_BFTM1, SystemCoreClock / 1000000 * BFTM_TIME_BASE);
  BFTM_OneShotModeCmd(HT_BFTM1, DISABLE);
  BFTM_IntConfig(HT_BFTM1, ENABLE);
  BFTM_EnaCmd(HT_BFTM1, ENABLE);
  NVIC_EnableIRQ(BFTM1_IRQn);
  
//  AFIO_GPxConfig(HS_BUS_TX_GPIO_ID, HS_BUS_TX_AFIO_PIN, HS_BUS_TX_AFIO_MODE);
//  AFIO_GPxConfig(HS_BUS_RX_GPIO_ID, HS_BUS_RX_AFIO_PIN, HS_BUS_RX_AFIO_MODE);
  GPIO_OpenDrainConfig(HS_BUS_TX_PORT, HS_BUS_TX_GPIO_PIN, ENABLE);
  AFIO_GPxConfig(LS_BUS_TX_GPIO_ID, LS_BUS_TX_AFIO_PIN, LS_BUS_TX_AFIO_MODE);
  AFIO_GPxConfig(LS_BUS_RX_GPIO_ID, LS_BUS_RX_AFIO_PIN, LS_BUS_RX_AFIO_MODE);
  GPIO_OpenDrainConfig(LS_BUS_TX_PORT, LS_BUS_TX_GPIO_PIN, ENABLE); 
//  AFIO_GPxConfig(HV_BUS_TX_GPIO_ID, HV_BUS_TX_AFIO_PIN, HV_BUS_TX_AFIO_MODE);
//  AFIO_GPxConfig(HV_BUS_RX_GPIO_ID, HV_BUS_RX_AFIO_PIN, HV_BUS_RX_AFIO_MODE); 
  GPIO_OpenDrainConfig(HV_BUS_TX_PORT, HV_BUS_TX_GPIO_PIN, ENABLE);
  
  USART_InitStructure.USART_BaudRate = 115200;
  USART_InitStructure.USART_WordLength = USART_WORDLENGTH_9B;
  USART_InitStructure.USART_StopBits = USART_STOPBITS_1;
  USART_InitStructure.USART_Parity = USART_PARITY_NO;
  USART_InitStructure.USART_Mode = USART_MODE_NORMAL;

  USART_Init(HS_BUS_PORT, &USART_InitStructure);
  USART_Init(HV_BUS_PORT, &USART_InitStructure);
  USART_InitStructure.USART_BaudRate = 9600;
  USART_Init(LS_BUS_PORT, &USART_InitStructure);
  
  USART_IntConfig(HS_BUS_PORT, USART_INT_RXDR, ENABLE);
  NVIC_EnableIRQ(HS_BUS_IRQn);
  USART_IntConfig(LS_BUS_PORT, USART_INT_RXDR, ENABLE);
  NVIC_EnableIRQ(LS_BUS_IRQn);
  USART_IntConfig(HV_BUS_PORT, USART_INT_RXDR, ENABLE);
  NVIC_EnableIRQ(HV_BUS_IRQn);   
}

/*********************************************************************************************************//**
 * @brief UR BUS bottom driver send function
 * @param data: u16 only low 9bit is valid
 * @param buffer: the buffer to be send, only low 8bit is valid, the 9st bit fixed 0
 * @retval None
 ************************************************************************************************************/
void Uart_Send_Data(u8 num, u16 data)
{
  USART_TxCmd(BUS_Port_table[num], ENABLE);//enable send function
  while ((BUS_Port_table[num]->SR & USART_FLAG_TXDE) == 0); 
  BUS_Port_table[num]->DR = data;
  while ((BUS_Port_table[num]->SR & USART_FLAG_TXC) == 0);  
  USART_TxCmd(BUS_Port_table[num], DISABLE);//disable send function
}
void Uart_Send_Buffer(u8 num, u8* buffer, u32 length)
{
  USART_TxCmd(BUS_Port_table[num], ENABLE);////enable send function
  while(length--)
  {
    while ((BUS_Port_table[num]->SR & USART_FLAG_TXDE) == 0); 
    BUS_Port_table[num]->DR = *buffer;
    while ((BUS_Port_table[num]->SR & USART_FLAG_TXC) == 0); 
    buffer++;
  }
  USART_TxCmd(BUS_Port_table[num], DISABLE);////disable send function
}
/* end of driver function*/

/*********************************************************************************************************//**
 * @brief Parse the retval packet from module used UR_BUS, and fill VCP_send_buffer
 * @param which UR_BUS number
 * @retval Check pass or fail
 ************************************************************************************************************/
static bool Parse_Retval(u8 bus, u16* uid, u8* len, u8* par)
{
  u8 MID[2];
  u8 Ten_EID;
  u8 Length;
  u8 Length_backup = Get_Valid_Lenth(&BUS_RX_Index[bus]);
  u32 check_sum = 0;
  u8 CS_backup;
  
  Buffer_Read(&BUS_RX_Index[bus], MID, 2);//mid
  if(MID[1] != 0x01)
    return FALSE;
  *uid = (u16)MID[0] << 4;
  check_sum += MID[0];

  Buffer_Read(&BUS_RX_Index[bus], &Ten_EID, 1);//EID
  if(Ten_EID & 0xF0)
  {
    Length = Ten_EID >> 4;
    if(Length != Length_backup-3)
      return FALSE;
    *uid |= Ten_EID & 0xF;
    check_sum += Ten_EID;
  }else
  {
    Buffer_Read(&BUS_RX_Index[bus], &Length, 1);
    if(Length != Length_backup-4)
      return FALSE;
    *uid |= Ten_EID;  
    check_sum += Ten_EID + Length;  
  }
  Length--;//cs
  Buffer_Read(&BUS_RX_Index[bus], par, Length);//read instru+para  
  for(u8 i = 0;i < Length;i++)
  {
    check_sum += par[i];
  }
  Buffer_Read(&BUS_RX_Index[bus], &CS_backup, 1);//read cs  
  
  check_sum = ~check_sum;
  check_sum &= 0xFF;
  if(check_sum != CS_backup)
    return FALSE;
  
  *len = Length;
  return TRUE;
  
  /*
  v2.0 Module-->mcu mid+eid_len+(len2)+status+retval+cs
  mcu需要做以下工作：
  1.判斷MID最高位是否為1，如果是則回傳上位機全部內容，否則丟棄該筆數據
  2.其餘數據不做任何解析直接bypass
  3.BUS_RX_buffer數據copy至VCP_send_buffer，VCP_send_index遞增
  
  u8 MID[2];
  Buffer_Read(&BUS_RX_Index[num], MID, 2);
  if(MID[1] != 0x01)
    return FALSE;
  VCP_send_index = Get_Valid_Lenth(&BUS_RX_Index[num]);
  Buffer_Read(&BUS_RX_Index[num], &VCP_send_buffer[1], VCP_send_index);  
  VCP_send_buffer[0] = MID[0];
  VCP_send_index++;
  return TRUE;
  */  
}

/*********************************************************************************************************//**
 * @brief UART_BUS Transmit function
 * @param bus LS_BUS_NUM; HS_BUS_NUM; HV_BUS_NUM
 * @param uid = MID << 4 | EID
 * @param len = length of struction and paramenter
 * @param buffer of put struction and paramenter
 * @retval all count of send byte succeed(MID+Tlen&EID+/Tlen2/+Instrution+Para+CS)
 ************************************************************************************************************/
static u32 Bus_Transmit(u8 bus, u16 uid, u8 len, u8* par)
{
  u32 ret_num = 0;
  u32 check_sum = 0;
  u8 Tlen[2];
  //MID
  Uart_Send_Data(bus, (uid >> 4) | 0x100);
  check_sum += uid >> 4;
  ret_num++;  
  //EID and length
  if(len+1 > 0xF)
  {
    Tlen[0] = uid & 0x0F;
    Tlen[1] = len+1;
    ret_num += 2;
    check_sum += Tlen[0]+Tlen[1];
    Uart_Send_Buffer(bus, Tlen, 2);
  }
  else
  {
    Tlen[0] = ((len+1) << 4) | (uid & 0x0F);
    ret_num++;
    check_sum += Tlen[0];
    Uart_Send_Buffer(bus, Tlen, 1);
  }  
  //struction and paramenter
  ret_num += len;
  for(u16 i = 0;i < len;i++)
  {
    check_sum += par[i];
  }
  Uart_Send_Buffer(bus, par, len);
  //check_sum
  check_sum = ~check_sum;
  check_sum = check_sum & 0xFF;
  Uart_Send_Buffer(bus, (u8*)&check_sum, 1);
  ret_num++;
  
  return ret_num;  
}

/*********************************************************************************************************//**
 * @brief UART_BUS Transmit API interface
 * @param uid = MID << 4 | EID
 * @param len = length of struction and paramenter
 * @param buffer of put struction and paramenter
 * @retval all count of send byte succeed(MID+Tlen&EID+/Tlen2/+Instrution+Para+CS)
 ************************************************************************************************************/
int LS_Transmit(u16 uid, u8 len, u8 *par)
{
  return Bus_Transmit(LS_BUS_NUM, uid, len, par);
}

int HS_Transmit(u16 uid, u8 len, u8 *par)
{
  AFIO_GPxConfig(HV_BUS_TX_GPIO_ID, HV_BUS_TX_AFIO_PIN, AFIO_FUN_DEFAULT);
  AFIO_GPxConfig(HV_BUS_RX_GPIO_ID, HV_BUS_RX_AFIO_PIN, AFIO_FUN_DEFAULT);         
  AFIO_GPxConfig(HS_BUS_TX_GPIO_ID, HS_BUS_TX_AFIO_PIN, HS_BUS_TX_AFIO_MODE);
  AFIO_GPxConfig(HS_BUS_RX_GPIO_ID, HS_BUS_RX_AFIO_PIN, HS_BUS_RX_AFIO_MODE);
  
  return Bus_Transmit(HS_BUS_NUM, uid, len, par);
}

int HV_Transmit(u16 uid, u8 len, u8 *par)
{
  AFIO_GPxConfig(HS_BUS_TX_GPIO_ID, HS_BUS_TX_AFIO_PIN, AFIO_FUN_DEFAULT);
  AFIO_GPxConfig(HS_BUS_RX_GPIO_ID, HS_BUS_RX_AFIO_PIN, AFIO_FUN_DEFAULT);         
  AFIO_GPxConfig(HV_BUS_TX_GPIO_ID, HV_BUS_TX_AFIO_PIN, HV_BUS_TX_AFIO_MODE);
  AFIO_GPxConfig(HV_BUS_RX_GPIO_ID, HV_BUS_RX_AFIO_PIN, HV_BUS_RX_AFIO_MODE);  
  
  return Bus_Transmit(HV_BUS_NUM, uid, len, par);
}

/*********************************************************************************************************//**
 * @brief Wait for UR_BUS receive packet from module
 * @param None
 * @retval Receive status
 ************************************************************************************************************/
static RX_STATUS_ENUM UR_Bus_Receive(u8 bus, u16* uid, u8* len, u8* par)
{
  static u8 stage[3] = {0};
  static u16 receive_num[3];
  u16 len_temp = 0; 
  
  switch(stage[bus])
  {
    case 0://start to receive packet
			*len = 0;
      Bus_Set_Timeout(bus, BUS_WAITING_TIMEOUT);
      USART_RxCmd(BUS_Port_table[bus], ENABLE);//enable receive function
      stage[bus]++;
      busRXCount[bus] = 0;
      return WAITING;

    case 1://wait for the first data
      if(Get_Valid_Lenth(&BUS_RX_Index[bus]))
      {
        Bus_Set_Timeout(bus, BUS_INTERVAL_TIMEOUT);
        receive_num[bus] = Get_Valid_Lenth(&BUS_RX_Index[bus]);
        stage[bus]++;
        return RECEIVING;
      }
      else if(Check_Bus_Timeout(bus))
      {
        USART_RxCmd(BUS_Port_table[bus], DISABLE);//disable receive function
        stage[bus] = 0;
        return NO_RESPONSE;//timeout no response
      }
      else return WAITING; 

    case 2://wait for timeout to indicate receive complete
      //initialize for expect length
      if(busRXCount[bus] == 0 && receive_num[bus] >= 3)
      {
        Buffer_ReadByteExt(&BUS_RX_Index[bus], (u8*)&len_temp, 2);
        if(len_temp & 0xF0)
        {
          busRXCount[bus] = (len_temp >> 4) + 2 + 1;
        }
      }
      if(busRXCount[bus] == 0 && receive_num[bus] >= 4)
      {
        Buffer_ReadByteExt(&BUS_RX_Index[bus], (u8*)&len, 3);
        busRXCount[bus] = len_temp + 2 + 1;
      }
 
      if(Get_Valid_Lenth(&BUS_RX_Index[bus]) > receive_num[bus])//data update
      {
        receive_num[bus] = Get_Valid_Lenth(&BUS_RX_Index[bus]);
        Bus_Set_Timeout(bus, BUS_INTERVAL_TIMEOUT);
        return RECEIVING;        
      }
      else if(Check_Bus_Timeout(bus) || receive_num[bus] == busRXCount[bus])
      {
        //data receive complete
        USART_RxCmd(BUS_Port_table[bus], DISABLE);//disable receive function
        stage[bus] = 0;
        if(Parse_Retval(bus, uid, len, par))
         return RX_OK; 
        else
        {
          Buffer_Discard(&BUS_RX_Index[bus]);
          return CHECK_ERR;
        }
      }
      else return INTERVAL;   
  }
  return CHECK_ERR;
}

/*********************************************************************************************************//**
 * @brief UR_BUS receive API interface
 * @param None
 * @retval Receive status
 * @note The retval 0--OK,1--No Res,2--Check Error means the end of receive process, others should not care.
          Only when retval is 0--OK, paramenter is valid.
 ************************************************************************************************************/
int LS_Receive(u16* uid, u8* len, u8* par)
{
  RX_STATUS_ENUM ret;
  do{
    ret = UR_Bus_Receive(LS_BUS_NUM, uid, len, par);
    IAP_Handler();
    BLE_process();
    MIDI_Procrss();
  }while(ret > CHECK_ERR);
  return ret;
}

int HS_Receive(u16* uid, u8* len, u8* par)
{
  RX_STATUS_ENUM ret;
  do{
    ret = UR_Bus_Receive(HS_BUS_NUM, uid, len, par);
    IAP_Handler();
    BLE_process();
    MIDI_Procrss();
  }while(ret > CHECK_ERR);
  return ret;
}

int HV_Receive(u16* uid, u8* len, u8* par)
{
  RX_STATUS_ENUM ret;
  do{
    ret = UR_Bus_Receive(HV_BUS_NUM, uid, len, par);
    IAP_Handler();
    BLE_process();
    MIDI_Procrss();
  }while(ret > CHECK_ERR);
  return ret;
}

/*********************************************************************************************************//**
 * @brief   This function handles HS_BUS & HV_BUS interrupt.
 * @retval  None
 ************************************************************************************************************/
void HS_BUS_IRQHandler(void)//HV_BUS_IRQHandler共用
{
  u16 temp;
  if(USART_GetFlagStatus(HS_BUS_PORT, USART_FLAG_RXDR))
  {
    temp = USART_ReceiveData(HS_BUS_PORT);
    if(temp & 0x100)
      Buffer_Write(&BUS_RX_Index[HS_BUS_NUM], (u8*)&temp, 2);
    else
      Buffer_Write(&BUS_RX_Index[HS_BUS_NUM], (u8*)&temp, 1);
  }
  if(USART_GetFlagStatus(HV_BUS_PORT, USART_FLAG_RXDR))
  {
    temp = USART_ReceiveData(HV_BUS_PORT);
    if(temp & 0x100)
      Buffer_Write(&BUS_RX_Index[HV_BUS_NUM], (u8*)&temp, 2);
    else
      Buffer_Write(&BUS_RX_Index[HV_BUS_NUM], (u8*)&temp, 1);
  }
}
/*********************************************************************************************************//**
 * @brief   This function handles LS_BUS interrupt.
 * @retval  None
 ************************************************************************************************************/
void LS_BUS_IRQHandler(void)
{
  u16 temp;
  if(USART_GetFlagStatus(LS_BUS_PORT, USART_FLAG_RXDR))
  {
    temp = USART_ReceiveData(LS_BUS_PORT);
    if(temp & 0x100)
      Buffer_Write(&BUS_RX_Index[LS_BUS_NUM], (u8*)&temp, 2);
    else
      Buffer_Write(&BUS_RX_Index[LS_BUS_NUM], (u8*)&temp, 1);
  }
}

void BFTM1_IRQHandler(void)
{
  BFTM_ClearFlag(HT_BFTM1);
  //uart bus timer related
  if(Bus_Timeout_Value[LS_BUS_NUM]) Bus_Timeout_Value[LS_BUS_NUM]--;
  if(Bus_Timeout_Value[HS_BUS_NUM]) Bus_Timeout_Value[HS_BUS_NUM]--;
  if(Bus_Timeout_Value[HV_BUS_NUM]) Bus_Timeout_Value[HV_BUS_NUM]--;
#if USE_BLEAPP_ENABLE
  /***************BLE & APP related***************/
  extern vu32 BLE_APPConnect;
  if(BLE_APPConnect) BLE_APPConnect--;
#endif  
  /***************BLE related***************/
  static u16 tm_cnt = 0;
  if(BLE_delay) BLE_delay--;
  if(++tm_cnt == 1000/BFTM_TIME_BASE) //1ms
  {
    tm_cnt = 0;
    timer_operate.bits.t1ms = TRUE;
  }
}
