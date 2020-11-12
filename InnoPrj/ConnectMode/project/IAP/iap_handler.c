/* Includes ------------------------------------------------------------------------------------------------*/
#include "ht32.h"
#include "string.h"
#include "crc16.h"
#include "iap_handler.h"
#include "iap_uart.h"
#include "ht32_iap.h"
#include "buffer.h"
#include "qspi_flash_MX25L12835F.h"

/* Private typedef -----------------------------------------------------------------------------------------*/
#define LOADER_VERSION          (0x100)
typedef struct {
  vu32 MDID;                             /* offset  0x180 Flash Manufacturer and Device ID Register (MDID)  */
  vu32 PNSR;                             /* offset  0x184 Flash Page Number Status Register (PNSR)          */
  vu32 PSSR;                             /* offset  0x188 Flash Page Size Status Register (PSSR)            */
  vu32 DID;
} FMCInfo_TypeDef;

typedef u32 (*pFunction)();


/*  User Command count                                                                                      */
#define HEADER_LEN              (5)
#define MAX_RETURN_PAYLOAD      (64)
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

#define CMD_VERIFY              (0x00)
#define CMD_PROGRAM             (0x01)
#define CMD_READ                (0x02)
#define CMD_BLANK               (0x03)

#define FMCINFO_BASE            (0x40080180)
#define FMCINFO                 ((FMCInfo_TypeDef*) FMCINFO_BASE)

#if defined(LIBCFG_FLASH_2PAGE_PER_WPBIT)
  #define PPBIT_PRE_PAGE         2
#else
  #define PPBIT_PRE_PAGE         1
#endif

#define LOADER_FLASH_START      (IAP_CODE_SIZE + IAP_APINFO_SIZE)
#define LOADER_CHIP_NAME        (0)
#define LOADER_PAGE_SIZE        (FMCINFO->PSSR)
#define LOADER_PPBIT_NUM        ((FMCINFO->PNSR) - (IAP_CODE_SIZE/LIBCFG_FLASH_PAGESIZE/PPBIT_PRE_PAGE) - 1)
#define LOADER_FLASH_NUM        ((LIBCFG_FLASH_SIZE - IAP_CODE_SIZE - IAP_VERSION_SIZE)/LIBCFG_FLASH_PAGESIZE)
#define LOADER_FULL_CHIP_NAME   ((FMCINFO->DID) & 0x0FFFFFFF)

#define LOADER_INFO0            (u32)(LOADER_VERSION << 16)
#define LOADER_INFO1            (u32)((LOADER_PAGE_SIZE << 16) | (LOADER_FLASH_START << 0))
#define LOADER_INFO2            (u32)((LOADER_FLASH_NUM << 16) | (LOADER_PPBIT_NUM   << 0))
#define LOADER_INFO3            (u32)(IAP_CODE_SIZE)
#define LOADER_INFO4            (u32)(LOADER_FULL_CHIP_NAME)
#define LOADER_INFO5            (u32)(FLASH_SECTOR_SIZE << 16 | FLASH_SECTOR_NUM)

#define FLASH_CMD_PROGRAM       ((u32)0x00000004)
#define FLASH_SEND_MAIN         ((u32)0x00000014)

#define IAP_PAGE_ERASE          (0x00)
#define IAP_MASS_ERASE          (0x01)
#define IAP_SECT_ERASE          (0x02)
#define IAP_CHIP_ERASE          (0x03)


/* Private function prototypes -----------------------------------------------------------------------------*/
static u32 _IAP_CMD0(void);
static u32 _IAP_Erase(u32 type, u32 saddr, u32 eaddr);
static u32 _IAP_Flash(u32 type, u32 saddr, u32 eaddr);
static u32 _IAP_CRC(u32 crc, u32 saddr, u32 length);
static u32 _IAP_Info(void);
static void _IAP_Exit(void);
static u32 _IAP_Reset(u32 uMode);
static u32 _IAP_GetBootMode(void);
static u32 _IAP_CRC_FLASH(u32 crc, u32 saddr, u32 length);
static u32 _IAP_External_Flash(u32 type, u32 saddr, u32 eaddr);

/* Private macro -------------------------------------------------------------------------------------------*/
#define WriteByte(data)         IAP_UART_Send((u8*)&data, 1)
#define WriteBytes(pData, len)  IAP_UART_Send(pData, len)
#define ReadByte(pData)         IAP_UART_Get(pData, 1, 100)

/* Private variables ---------------------------------------------------------------------------------------*/
u32 gu32Infotable[6];
const u16 IAP_Info __attribute__((at(0x400)))= LOADER_VERSION;
const u8 Err_Code[7] = {0x55, 0x07, 'F', 0x01, 0x5D, 0x2C, 0x70};

__ALIGN4 static u8 gu8CmdBuffer[MAX_CMD_LEN];
static u8 gu8ReturnBuffer[MAX_RETURN_LEN];
static u32 u32BufferIndex;
static u32 u32ReturnBufferIndex;
static vu16 gRxCmdTimeoutCnt = 0;
static vu16 gHeartbeatTimeoutCnt = 0;

u32 sp_backup,sp_address_backup;

static u8 CalculateHeaderCheckSum(u8* pCmdBuffer);
static void AdjustReturnPackage(u8* pBuf, u8 length);
static void ParseCmd(void);

static const pFunction pFComHandlerTable[CMD_COUNT] =
{
  (pFunction)_IAP_Erase,
  (pFunction)_IAP_Flash,
  (pFunction)_IAP_External_Flash,
  (pFunction)_IAP_CRC,
  (pFunction)_IAP_CRC_FLASH,
  (pFunction)_IAP_Info,
  (pFunction)_IAP_CMD0,
  (pFunction)_IAP_Reset,
  (pFunction)_IAP_Exit,
  (pFunction)_IAP_GetBootMode,
};



/*********************************************************************************************************//**
  * @brief  IAP mode initialization.
  * @retval None
  ***********************************************************************************************************/
void IAP_Init(void)
{ 
  u32 temp;
  temp = rw(LOADER_FLASH_START+0x400);
  rw(BOOT_MODE_ID_ADDR) = BOOT_MODE_IAP;
  if((temp >> 16) != (~temp & 0xFFFF))
  {
    temp = 0x2000 | LOADER_VERSION << 16;
  }
  else
  {
    temp >>= 16;
    temp |= LOADER_VERSION << 16;
  }
  gu32Infotable[0] = temp;;
  gu32Infotable[1] = LOADER_INFO1;
  gu32Infotable[2] = LOADER_INFO2;
  gu32Infotable[3] = LOADER_INFO3;
  gu32Infotable[4] = LOADER_INFO4;
  gu32Infotable[5] = LOADER_INFO5;

  //IAP_Init_sub();
  QSPI_FLASH_Init();
  QSPI_FLASH_RDID();
  QSPI_FLASH_WRR(0x42, 0x7);
  iap_uart_Configuration();
}


/*********************************************************************************************************//**
  ***********************************************************************************************************/
void IAP_Handler(void)
{ 
//  extern vu8 VCP_RX_finish;
//  extern Buffer_TypeDef IAP_UART_RxBuffer;
//  u8 data;
//  u8 len;
//  if(VCP_RX_finish)
//  {
//    VCP_RX_finish = FALSE;
//    //Check head
//    Buffer_ReadByte(&IAP_UART_RxBuffer, &data);
//    if(data != 0x55)
//    {
//      goto Format_err;
//    }
//    u32BufferIndex = 0;
//    gu8CmdBuffer[u32BufferIndex++] = data;
//    
//    //Check length
//    Buffer_ReadByte(&IAP_UART_RxBuffer, &data);
//    len = Get_Valid_Lenth(&IAP_UART_RxBuffer);
//    if((data - 2) != len)
//    {
//      goto Format_err;
//    }
//    gu8CmdBuffer[u32BufferIndex++] = data;
//    
//    //Copy other data(comamnd+type+...)
//    Buffer_Read(&IAP_UART_RxBuffer, &gu8CmdBuffer[u32BufferIndex], len);
//    u32BufferIndex += len;
//    
//    //Parse command
//    ParseCmd();
//  }
//  return;
//  Format_err:  
//  WriteBytes((u8*)Err_Code, 7);
//  Buffer_Discard(&IAP_UART_RxBuffer);
  static u8 gRxCmdState = 0;
  u8 c;

  while(ReadByte(&c))
  {
    switch (gRxCmdState)
    {
      case 0:
      {
        if(c == 0x55)
        {
          u32BufferIndex = 0;
          gu8CmdBuffer[u32BufferIndex++] = c;
          gRxCmdState++;
          gRxCmdTimeoutCnt = 10;
          continue;
        }
        return;
      }
      case 1:
      {
        gu8CmdBuffer[u32BufferIndex++] = c;
        gRxCmdState++;
        gRxCmdTimeoutCnt = 10;
        continue;
      }
      case 2:
      {
        gRxCmdTimeoutCnt = 500;

        gu8CmdBuffer[u32BufferIndex++] = c;
        
        if(u32BufferIndex >= gu8CmdBuffer[1])
        {
          ParseCmd();
          gRxCmdState = 0;
          gHeartbeatTimeoutCnt = 100;
          return;
        }
        
        continue;
      }
      default:
      {
        gRxCmdState = 0;

        break;
      }
    }
  }        
  
  if(gRxCmdTimeoutCnt == 0)
  {
    gRxCmdState = 0;
  }
}

/*********************************************************************************************************//**
  * @brief  Reset Command.
  * @param  uMode: Mode after reset
  * @retval FALSE or TRUE
  ***********************************************************************************************************/
u32 _IAP_Reset(u32 uMode)
{
  u32 time_out = 10000;
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
  while(time_out--)
  {
    __nop();
  }
  USBD_DPpullupCmd(DISABLE);
  NVIC_SystemReset();

  return TRUE;
}

/*********************************************************************************************************//**
  * @brief  Check AP is valid or not.
  * @retval FALSE or TRUE
  ***********************************************************************************************************/
u32 IAP_isAPValid(void)
{
  vu32 SP, PC;

  /* Check Stack Point in range                                                                             */
  SP = rw(IAP_APFLASH_START);
  if (SP < IAP_APSRAM_START || SP > IAP_SRAM_END)
  {
    return FALSE;
  }

  /* Check PC in range                                                                                      */
  PC = rw(IAP_APFLASH_START + 0x4);
  if (PC < IAP_APFLASH_START || PC > IAP_APFLASH_END)
  {
    return FALSE;
  }
  
  return TRUE;
}


#if defined (__CC_ARM)
/*********************************************************************************************************//**
  * @brief  Jump to user application by change PC.
  * @param  address: Start address of user application
  * @retval None
  ***********************************************************************************************************/
__asm void IAP_GoCMD(u32 address)
{
  LDR R1, [R0]
  MOV SP, R1
  LDR R1, [R0, #4]
  BX R1
}
#elif defined (__ICCARM__)
void IAP_GoCMD(u32 address)
{
  __asm("LDR R1, [R0]");
  __asm("MOV SP, R1");
  __asm("LDR R1, [R0, #4]");
  __asm("BX R1");
}
#endif

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
static void ParseCmd(void)
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

/*********************************************************************************************************//**
  * @brief  
  * @retval 
  ***********************************************************************************************************/
static u32 _IAP_GetBootMode(void)
{
  gu8ReturnBuffer[u32ReturnBufferIndex++] = 0xF5; // IAP mode
  
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
static u32 _IAP_Flash(u32 type, u32 saddr, u32 eaddr)
{
  u32 i, data;
  u8 *buffer = (u8 *)(&gu8CmdBuffer[13]);
  FLASH_OptionByte Option;

  /*--------------------------------------------------------------------------------------------------------*/
  /* When Security protection is enabled, read operation is not allowed                                     */
  /*--------------------------------------------------------------------------------------------------------*/
  FLASH_GetOptionByteStatus(&Option);

  if (type == CMD_PROGRAM)
  {
    /* Blank Check before programming                                                                       */
    for (i = 0; i < (eaddr-saddr + 1); i += 4)
    {
      if (rw(saddr + i) != 0xFFFFFFFF )
      {
        gu8ReturnBuffer[3] = 0x4;
        return CMD_FAILED;
      }
    }
  }

  /*--------------------------------------------------------------------------------------------------------*/
  /* Program                                                                                                */
  /*--------------------------------------------------------------------------------------------------------*/
  if (type == CMD_PROGRAM)
  {
    if(saddr == sp_address_backup)
    {
      memcpy((u8*)&sp_backup, (u8*)buffer, 4);
      saddr += 4;
      buffer += 4;
    }

    while (saddr <= eaddr)
    {
      u32 tmp;
      memcpy((u8*)&tmp, (u8*)buffer, 4);
      FLASH_ProgramWordData(saddr, tmp);
      saddr += 4;
      buffer += 4;
    }
  }

  while (saddr <= eaddr)
  {
    if (saddr > 0x1FF003FC)
    {
      data = 0;
    }
    else
    {
      data = rw(saddr);
    }
    /*------------------------------------------------------------------------------------------------------*/
    /* Verify                                                                                               */
    /*------------------------------------------------------------------------------------------------------*/
    if (type == CMD_VERIFY)
    {
      u32 tmp;
      memcpy((u8*)&tmp, (u8*)buffer, 4);

      if (data != tmp)
      {
        gu8ReturnBuffer[3] = 0x4;
        return CMD_FAILED;
      }
    }
    /*------------------------------------------------------------------------------------------------------*/
    /* Blank                                                                                                */
    /*------------------------------------------------------------------------------------------------------*/
    else if (type == CMD_BLANK)
    {
      if (data != 0xFFFFFFFF)
      {
        gu8ReturnBuffer[3] = 0x4;
        return CMD_FAILED;
      }
    }
    /*--------------------------------------------------------------------------------------------------------*/
    /* Read                                                                                                   */
    /*--------------------------------------------------------------------------------------------------------*/   
    else
    {
      if (Option.MainSecurity == 1)
      {
        if ((saddr >= IAP_CODE_SIZE + 0x800) && (saddr < (IAP_CODE_SIZE + 0x800 + 48)) )
        {
          memcpy((u8*)&gu8ReturnBuffer[u32ReturnBufferIndex], (u8*)&data, 4);
          u32ReturnBufferIndex += 4;
        }

        else
        {
          data = 0;
          memcpy((u8*)&gu8ReturnBuffer[u32ReturnBufferIndex], (u8*)&data, 4);
          u32ReturnBufferIndex += 4;
        }
      } 
      else
      {     
        memcpy((u8*)&gu8ReturnBuffer[u32ReturnBufferIndex], (u8*)&data, 4);
        u32ReturnBufferIndex += 4;
      }
    }    

    saddr += 4;
    buffer += 4;
  }

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
static u32 _IAP_External_Flash(u32 type, u32 saddr, u32 eaddr)
{
  u16   cnt = 0;
  u8    u8TempBuffer[256];
  u8    *buffer = (u8 *)(&gu8CmdBuffer[13]);

  QSPI_FLASH_BYTE_QOR(u8TempBuffer, saddr, 256);
  if (type == CMD_PROGRAM)
  {
    /* Blank Check before programming */
    for (u16 i = 0; i < (eaddr-saddr+1); i++)
    {
      if(u8TempBuffer[i] != 0xFF)
      {
        gu8ReturnBuffer[3] = 0x4;
        return CMD_FAILED;
      }
    }
    QSPI_FLASH_BYTE_QPP(buffer, saddr, eaddr-saddr+1);
    return CMD_SUCCESS;
  }
  
  while (saddr <= eaddr)
  {
    /*------------------------------------------------------------------------------------------------------*/
    /* Verify                                                                                               */
    /*------------------------------------------------------------------------------------------------------*/
    if (type == CMD_VERIFY)
    {
      if (u8TempBuffer[cnt] != *buffer)
      {
        gu8ReturnBuffer[3] = 0x4;
        return CMD_FAILED;
      }
    }
    /*------------------------------------------------------------------------------------------------------*/
    /* Blank                                                                                                */
    /*------------------------------------------------------------------------------------------------------*/
    else if (type == CMD_BLANK)
    {
      if (u8TempBuffer[cnt] != 0xFF)
      {
        gu8ReturnBuffer[3] = 0x4;
        return CMD_FAILED;
      }
    }
    /*--------------------------------------------------------------------------------------------------------*/
    /* Read                                                                                                   */
    /*--------------------------------------------------------------------------------------------------------*/   
    else
    {
      memcpy((u8*)&gu8ReturnBuffer[u32ReturnBufferIndex], &u8TempBuffer[cnt], 1);
      u32ReturnBufferIndex ++;
    }    

    cnt ++;
    saddr ++;
    buffer ++;
  }

  return CMD_SUCCESS;
}


/*********************************************************************************************************//**
  * @brief  Mass/Page Erase.
  * @param  type: Erase type
  *         @arg IAP_MASS_ERASE: Mass Erase (Not support in IAP mode)
  *         @arg IAP_PAGE_ERASE: Page Erase
  * @param  saddr: Start address
  * @param  eaddr: End address
  * @retval CMD_SUCCESS or CMD_FAILED
  ***********************************************************************************************************/
static u32 _IAP_Erase(u32 type, u32 saddr, u32 eaddr)
{
  u32 i, j;
  
  if(type == IAP_PAGE_ERASE)
  {
    if((saddr < IAP_APFLASH_START) || ((eaddr-saddr) > (LIBCFG_FLASH_SIZE - IAP_APFLASH_START)))
    {
      gu8ReturnBuffer[3] = 0x3;
      return CMD_FAILED;
    }
    for (i = saddr, j = 0; i <= eaddr; i += FLASH_PAGE_SIZE, j++)
    {
      FLASH_ErasePage(i);
    }
    sp_address_backup = saddr;
  }
  else if(type == IAP_MASS_ERASE)
  {
    for (i = IAP_APFLASH_START, j = 0; i < LIBCFG_FLASH_SIZE; i += FLASH_PAGE_SIZE, j++)
    {
      FLASH_ErasePage(i);
    }
    sp_address_backup = IAP_APFLASH_START;
  }
  else if(type == IAP_SECT_ERASE)
  {
    if(eaddr >= FLASH_SECTOR_SIZE * FLASH_SECTOR_NUM)
    {
      gu8ReturnBuffer[3] = 0x3;
      return CMD_FAILED;      
    }
    for (i = saddr, j = 0; i <= eaddr; i += FLASH_SECTOR_SIZE, j++)
    {
      QSPI_FLASH_SE(i);
    }   
  }
  else if(type == IAP_CHIP_ERASE)
  {
    QSPI_FLASH_ChipErase();
  }
  else
  {
    gu8ReturnBuffer[3] = 0x2;
    return CMD_FAILED;     
  }
 
  return CMD_SUCCESS;
}


/*********************************************************************************************************//**
  * @brief  Calculate CRC value.
  * @param  crc: Iinitial value of CRC (Usually as 0)
  * @param  saddr: Start address
  * @param  length: Length for CRC calculation
  * @retval Always success (CMD_SUCCESS)
  ***********************************************************************************************************/
static u32 _IAP_CRC(u32 crc, u32 saddr, u32 length)
{
  FLASH_ProgramWordData(sp_address_backup, sp_backup); //write sp

  crc = CRC16(crc, (u8 *)saddr, length);

  memcpy((u8*)&gu8ReturnBuffer[u32ReturnBufferIndex], (u8*)&crc, 2);
  u32ReturnBufferIndex += 2;
  
  HT_GPIOC->DOUTR |= 0x1 << 14;
  return CMD_SUCCESS;
}


/*********************************************************************************************************//**
  * @brief  Calculate CRC value for external flash.
  * @param  crc: Iinitial value of CRC (Usually as 0)
  * @param  saddr: Start address
  * @param  length: Length for CRC calculation
  * @retval Always success (CMD_SUCCESS)
  ***********************************************************************************************************/
static u32 _IAP_CRC_FLASH(u32 crc, u32 saddr, u32 length)
{
  u8 u8TempBuffer[256];
  u32 u32CurrentNum;
  
  while(length)
  {
    u32CurrentNum = (length > 256 ? 256 : length);
    QSPI_FLASH_BYTE_QOR(u8TempBuffer, saddr, u32CurrentNum);
    crc = CRC16(crc, u8TempBuffer, u32CurrentNum);
    saddr += u32CurrentNum;
    length -= u32CurrentNum;
  }
  
  memcpy((u8*)&gu8ReturnBuffer[u32ReturnBufferIndex], (u8*)&crc, 2);
  u32ReturnBufferIndex += 2;
  
  return CMD_SUCCESS;
}
/*********************************************************************************************************//**
  * @brief  Exit Loader mode.
  * @retval None
  ***********************************************************************************************************/
static void _IAP_Exit(void)
{
  gu8ReturnBuffer[2] = CMD_SUCCESS;
  AdjustReturnPackage(gu8ReturnBuffer, 5);
  WriteBytes(gu8ReturnBuffer, gu8ReturnBuffer[1]);
  
  while (1);
}



