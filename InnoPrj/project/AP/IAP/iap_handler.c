/* Includes ------------------------------------------------------------------------------------------------*/
#include "ht32.h"
#include "string.h"
#include "crc16.h"
#include "iap_handler.h"
#include "iap_uart.h"
#include "ht32_iap.h"
#include "urbus.h"
#include "qspi_flash_MX25L12835F.h"
//-----------------------------------------------------------------------------
typedef struct {
  vu32 MDID;                             /* offset  0x180 Flash Manufacturer and Device ID Register (MDID)  */
  vu32 PNSR;                             /* offset  0x184 Flash Page Number Status Register (PNSR)          */
  vu32 PSSR;                             /* offset  0x188 Flash Page Size Status Register (PSSR)            */
  vu32 DID;
} FMCInfo_TypeDef;

typedef u32 (*pFunction)();


//-----------------------------------------------------------------------------
#define HEADER_LEN              (5)
#define MAX_RETURN_PAYLOAD      (12)
#define CRC_LEN                 (2)
#define MAX_RETURN_LEN          (HEADER_LEN + MAX_RETURN_PAYLOAD + CRC_LEN) // Header + MaxPayload + crc

#define MAX_CMD_LEN             (HEADER_LEN + 4 + 4 + CMD_PAYLOAD_LEN + CRC_LEN)
#define MAX_TOKENS              (3)
#define IAP_CMD_COUNT           (10)
#define CMD_COUNT               (IAP_CMD_COUNT)
#define CMD_SUCCESS             ('O')
#define CMD_FAILED              ('F')
#define USER_CMD_START          (0x50)
#define CMD_PAYLOAD_ADDR        (HEADER_LEN + 4 + 4)
#define CMD_PAYLOAD_LEN         (64)

#define PREFIX                  (0x55)

#define CMD_PAGE_ERASE          (1)
#define CMD_MASS_ERASE          (0)

#define CMD_VERIFY              (0)
#define CMD_PROGRAM             (1)
#define CMD_READ                (2)
#define CMD_BLANK               (3)

#define FMCINFO_BASE            (0x40080180)
#define FMCINFO                 ((FMCInfo_TypeDef*) FMCINFO_BASE)

#if defined(LIBCFG_FLASH_2PAGE_PER_WPBIT)
  #define PPBIT_PRE_PAGE         2
#else
  #define PPBIT_PRE_PAGE         1
#endif

#define AP_MODE                 (0x0)
#define AP_VERSION              (0x202)
#define AP_INFO                 ((AP_MODE << 12) | AP_VERSION)
#define AP_INFO_REVERSE         (~AP_INFO)

#define LOADER_CHIP_NAME        (0)
#define LOADER_PAGE_SIZE        (FMCINFO->PSSR)
#define LOADER_PPBIT_NUM        ((FMCINFO->PNSR) - (IAP_CODE_SIZE/LIBCFG_FLASH_PAGESIZE/PPBIT_PRE_PAGE) - 1)
#define LOADER_FLASH_NUM        ((LIBCFG_FLASH_SIZE - IAP_CODE_SIZE - IAP_VERSION_SIZE)/LIBCFG_FLASH_PAGESIZE)
#define LOADER_FULL_CHIP_NAME   ((FMCINFO->DID) & 0x0FFFFFFF)

#define LOADER_INFO0            (u32)((LOADER_VERSION   << 16) | (AP_MODE << 0))
#define LOADER_INFO1            (u32)((LOADER_PAGE_SIZE << 16) | (LOADER_FLASH_START << 0))
#define LOADER_INFO2            (u32)((LOADER_FLASH_NUM << 16) | (LOADER_PPBIT_NUM   << 0))
#define LOADER_INFO3            (u32)(IAP_CODE_SIZE)
#define LOADER_INFO4            (u32)(LOADER_FULL_CHIP_NAME)
#define LOADER_INFO5            (u32)(FLASH_SECTOR_SIZE << 16) | (FLASH_SECTOR_NUM << 16)

#define FLASH_CMD_PROGRAM       ((u32)0x00000004)
#define FLASH_SEND_MAIN         ((u32)0x00000014)

#define IAP_PAGE_ERASE          (0x8)
#define IAP_MASS_ERASE          (0xA)


/* Private function prototypes -----------------------------------------------------------------------------*/
static u32 _IAP_CMD0(void);
static u32 _IAP_CMD_ERR(void);
static u32 _IAP_Info(void);
static u32 _IAP_Reset(u32 uMode);
static u32 _IAP_GetBootMode(void);


/* Private macro -------------------------------------------------------------------------------------------*/
#define WriteByte(data)         IAP_UART_Send((u8*)&data, 1)
#define WriteBytes(pData, len)  IAP_UART_Send(pData, len)
#define ReadByte(pData)         IAP_UART_Get(pData, 1, 100)

/* Private variables ---------------------------------------------------------------------------------------*/
u32 gu32Infotable[6];
const u16 AP_Info[2] __attribute__((at(LOADER_FLASH_START + 0x400)))= {AP_INFO_REVERSE, AP_INFO};

__ALIGN4 u8 gu8CmdBuffer[MAX_CMD_LEN];
static u8 gu8ReturnBuffer[MAX_RETURN_LEN];
u32 u32BufferIndex;
static u32 u32ReturnBufferIndex;
static vu16 gRxCmdTimeoutCnt = 0;
static vu16 gHeartbeatTimeoutCnt = 0;

static u8 CalculateHeaderCheckSum(u8* pCmdBuffer);
static void AdjustReturnPackage(u8* pBuf, u8 length);


static const pFunction pFComHandlerTable[CMD_COUNT] =
{
  (pFunction)_IAP_CMD_ERR,
  (pFunction)_IAP_CMD_ERR,
  (pFunction)_IAP_CMD_ERR,
  (pFunction)_IAP_CMD_ERR,
  (pFunction)_IAP_CMD_ERR,
  (pFunction)_IAP_Info,
  (pFunction)_IAP_CMD0,
  (pFunction)_IAP_Reset,
  (pFunction)_IAP_CMD_ERR,
  (pFunction)_IAP_GetBootMode,
};




/*********************************************************************************************************//**
  * @brief  IAP mode initialization.
  * @retval None
  ***********************************************************************************************************/
void IAP_Init(void)
{
  gu32Infotable[0] = rhw(0x400);
  gu32Infotable[0] <<= 16;
  gu32Infotable[0] |= AP_INFO;
  gu32Infotable[1] = LOADER_INFO1;
  gu32Infotable[2] = LOADER_INFO2;
  gu32Infotable[3] = LOADER_INFO3;
  gu32Infotable[4] = LOADER_INFO4;
  gu32Infotable[5] = LOADER_INFO5;
//  IAP_Init_sub();
//  iap_uart_Configuration();
}


/*********************************************************************************************************//**
  ***********************************************************************************************************/
//void IAP_Handler(void)
//{ 
//  static u8 gRxCmdState = 0;
//  u8 c;
//  
//  
//  
//  while(ReadByte(&c))
//  {
//    switch (gRxCmdState)
//    {
//      case 0:
//      {
//        if(c == 0x55)
//        {
//          u32BufferIndex = 0;
//          gu8CmdBuffer[u32BufferIndex++] = c;
//          gRxCmdState++;
//          gRxCmdTimeoutCnt = 10;
//          continue;
//        }
//        return;
//      }
//      case 1:
//      {
//        gu8CmdBuffer[u32BufferIndex++] = c;
//        gRxCmdState++;
//        gRxCmdTimeoutCnt = 10;
//        continue;
//      }
//      case 2:
//      {
//        gRxCmdTimeoutCnt = 500;

//        gu8CmdBuffer[u32BufferIndex++] = c;
//        
//        if(u32BufferIndex >= gu8CmdBuffer[1])
//        {
//          ParseCmd();
//          gRxCmdState = 0;
//          gHeartbeatTimeoutCnt = 100;
//          return;
//        }
//        
//        continue;
//      }
//      default:
//      {
//        gRxCmdState = 0;

//        break;
//      }
//    }
//  }        
//  
//  if(gRxCmdTimeoutCnt == 0)
//  {
//    gRxCmdState = 0;
//  }
//}
void IAP_Handler(void)
{
  extern vu8 VCP_RX_finish;
  extern Buffer_TypeDef VCP_RX_Index;
  u8 cmd;
  
  if(VCP_RX_finish)
  {
    VCP_RX_finish = FALSE;
    Buffer_Read(&VCP_RX_Index, &cmd, 1);
    if(cmd == 0x55)
    {
      u32BufferIndex = 0;
      gu8CmdBuffer[u32BufferIndex++] = 0x55;
      u16 len_temp = Get_Valid_Lenth(&VCP_RX_Index);
      Buffer_Read(&VCP_RX_Index, &gu8CmdBuffer[1], len_temp);
      u32BufferIndex += len_temp;
      ParseCmd();
    }
    else
      Buffer_Discard(&VCP_RX_Index);
  }
}
/*********************************************************************************************************//**
  * @brief  Reset Command.
  * @param  uMode: Mode after reset
  * @retval FALSE or TRUE
  ***********************************************************************************************************/
u32 _IAP_Reset(u32 uMode)
{
  if (uMode == 0)
  {
    ww(BOOT_MODE_ID_ADDR, BOOT_MODE_AP);
  }
  else
  {
    ww(BOOT_MODE_ID_ADDR, BOOT_MODE_IAP);
  }

  gu8ReturnBuffer[2] = CMD_SUCCESS;
  AdjustReturnPackage(gu8ReturnBuffer, 5);
  WriteBytes(gu8ReturnBuffer, gu8ReturnBuffer[1]);
  
  USBD_DPpullupCmd(DISABLE);
  NVIC_SystemReset();

  return TRUE;
}


/*********************************************************************************************************//**
  * @brief  TBD
  * @retval None
  ***********************************************************************************************************/
u8 CalculateHeaderCheckSum(u8* pCmdBuffer)
{
  u8 i;
  u8 sum = 0;
  
  for(i=0 ; i < 4 ; i++)
  {
    sum += pCmdBuffer[i];
  }
  sum = (~sum ) + 1;
  return sum;
}

/*********************************************************************************************************//**
  * @brief  TBD
  * @retval None
  ***********************************************************************************************************/
void AdjustReturnPackage(u8* pBuf, u8 length)
{
  u16 crc;
  
  pBuf[0] = 0x55;
  pBuf[1] = length + 2; // Add CRC length
  pBuf[4] = CalculateHeaderCheckSum(pBuf);
  
  crc = CRC16(0, (u8 *)(&pBuf[0]), length);
  memcpy((u8*)&pBuf[length], (u8*)&crc, 2);
}

/*********************************************************************************************************//**
  * @brief  TBD
  * @retval None
  ***********************************************************************************************************/
void ParseCmd(void)
{
  u32 u32Parameter[MAX_TOKENS];
  u16 crc;
  u16 crcValue;
  u8 len;
  
  u32ReturnBufferIndex = 5;
  gu8ReturnBuffer[3] = 0x0;
  
  /*------------------------------------------------------------------------------------------------------*/
  /* Check CRC value of command packet                                                                    */
  /*------------------------------------------------------------------------------------------------------*/
  len = gu8CmdBuffer[1];
  crc = gu8CmdBuffer[len-2] | ((u16)gu8CmdBuffer[len-1] << 8);
  crcValue = CRC16(0, (u8 *)(&gu8CmdBuffer[0]), len-2);

  if (gu8CmdBuffer[2] >= USER_CMD_START)
  {
    gu8CmdBuffer[2] = gu8CmdBuffer[2] - USER_CMD_START + IAP_CMD_COUNT;
  }
  
  /*------------------------------------------------------------------------------------------------------*/
  /* Check command is valid and CRC is correct                                                            */
  /*------------------------------------------------------------------------------------------------------*/
  if (gu8CmdBuffer[2] > CMD_COUNT)
  {
    /*----------------------------------------------------------------------------------------------------*/
    /* Command invalid or CRC error. Return 'F' and clear command buffer                                  */
    /*----------------------------------------------------------------------------------------------------*/
    gu8ReturnBuffer[2] = CMD_FAILED;
    gu8ReturnBuffer[3] = 0x1;
  }
  else if (crc != crcValue)
  {
    gu8ReturnBuffer[2] = CMD_FAILED;
    gu8ReturnBuffer[3] = 0x1;
  }
  else
  {
    /*----------------------------------------------------------------------------------------------------*/
    /* Prepare parameter and execution command                                                            */
    /*----------------------------------------------------------------------------------------------------*/
    u32Parameter[0] = gu8CmdBuffer[3];
    memcpy((u8*)&u32Parameter[1], (u8*)&gu8CmdBuffer[5], 4);
    memcpy((u8*)&u32Parameter[2], (u8*)&gu8CmdBuffer[9], 4);
    
    gu8ReturnBuffer[2] = (*pFComHandlerTable[gu8CmdBuffer[2]])(u32Parameter[0], u32Parameter[1], u32Parameter[2]);
  }

  /*------------------------------------------------------------------------------------------------------*/
  /* Send Result to Host                                                                                  */
  /*------------------------------------------------------------------------------------------------------*/  
  AdjustReturnPackage(gu8ReturnBuffer, u32ReturnBufferIndex);
  WriteBytes(gu8ReturnBuffer, gu8ReturnBuffer[1]);
}

/*********************************************************************************************************//**
  * @brief  
  * @retval 
  ***********************************************************************************************************/
static u32 _IAP_CMD0(void)
{
  return CMD_SUCCESS;
}
static u32 _IAP_CMD_ERR(void)
{
  return CMD_FAILED;
}

/*********************************************************************************************************//**
  * @brief  
  * @retval 
  ***********************************************************************************************************/
static u32 _IAP_GetBootMode(void)
{
  gu8ReturnBuffer[u32ReturnBufferIndex++] = 0xF0; // AP mode
  
  return CMD_SUCCESS;
}

/*********************************************************************************************************//**
  * @brief  Send information to Host.
  * @retval Always success (CMD_SUCCESS)
  ***********************************************************************************************************/
static u32 _IAP_Info(void)
{
  gu8ReturnBuffer[2] = CMD_SUCCESS;

  memcpy((u8*)&gu8ReturnBuffer[5], (u8*)&gu32Infotable[0], 24);
//  AdjustReturnPackage(gu8ReturnBuffer, 5+24);
//  WriteBytes(gu8ReturnBuffer, gu8ReturnBuffer[1]);

//  memcpy((u8*)&gu8ReturnBuffer[u32ReturnBufferIndex], (u8*)&gu32Infotable[3], 8);
  u32ReturnBufferIndex += 24;
  
  return CMD_SUCCESS;
}


/*********************************************************************************************************//**
  * @brief  Download image for program or verify.
  * @param  type: Program or verify
  *         @arg CMD_PROGRAM: Program mode
  *         @arg CMD_VERIFY: Verify mode
  *         @arg CMD_BLANK: Blank check mode
  *         @arg CMD_READ: Read mode
  * @param  saddr: Start address
  * @param  eaddr: End address
  * @param  buffer: point of data buffer
  * @retval CMD_SUCCESS or CMD_FAILED
  ***********************************************************************************************************/
//static u32 _IAP_Flash(u32 type, u32 saddr, u32 eaddr)
//{
//  u32 data;
//  u8 *buffer = (u8 *)(&gu8CmdBuffer[13]);

//  while (saddr <= eaddr)
//  {
//    if (saddr > 0x1FF003FC)
//    {
//      data = 0;
//    }
//    else
//    {
//      data = rw(saddr);
//    }

//    if (type == CMD_READ)
//    {
//    /*--------------------------------------------------------------------------------------------------------*/
//    /* Read                                                                                                   */
//    /*--------------------------------------------------------------------------------------------------------*/   
//      if ( (saddr >= IAP_CODE_SIZE + DEV_SVER_OFFSET) 
//        && (saddr < (IAP_CODE_SIZE + DEV_SVER_OFFSET + 48)) )
//      {
//        memcpy((u8*)&gu8ReturnBuffer[u32ReturnBufferIndex], (u8*)&data, 4);
//        u32ReturnBufferIndex += 4;
//      }

//      else
//      {
//        data = 0;
//        memcpy((u8*)&gu8ReturnBuffer[u32ReturnBufferIndex], (u8*)&data, 4);
//        u32ReturnBufferIndex += 4;
//      }
//    }
//    saddr += 4;
//    buffer += 4;
//  }

//  return CMD_SUCCESS;
//}

u16 IAP_UART_Send(u8 *puData, u16 length)
{
  u8 i;
  u16 current_cnt = length > 64 ? 64 : length;
  while(current_cnt)
  {
    for(i = 0;i < current_cnt;i++)
    {
      SERIAL_PutChar(puData[i]);
    }
    SERIAL_Flush();
    puData += current_cnt;
    length -= current_cnt;
    current_cnt = length > 64 ? 64 : length;
  }
  
  return length;
}


