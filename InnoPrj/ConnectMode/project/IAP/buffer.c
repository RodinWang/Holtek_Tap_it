#include "buffer.h"

/* Private typedef -----------------------------------------------------------------------------------------*/
/* Private constants ---------------------------------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------------------------------------*/
#define IS_BUFFER_EMPTY(Read, Write)              (Write == Read)
#define IS_BUFFER_FULL(Read, Write, Size)         (((Write + 1) % Size) == Read)
#define BUFF_FERR_LENGTH(Read, Write, Size)       ((Write >= Read) ? (Size - Write + Read) : (Read - Write))

/* Global variables ----------------------------------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------------------------------------*/
/* Global functions ----------------------------------------------------------------------------------------*/

/*********************************************************************************************************//**
  * @brief  Buffer initialization.
  * @param  pBuffer: Pointer of Buffer_TypeDef
  * @param  puMemory: Memory allocate by application
  * @param  uSize: Memory Size
  * @retval None
  ***********************************************************************************************************/
void Buffer_Init(Buffer_TypeDef *pBuffer, u8 *puMemory, u32 uSize)
{
  pBuffer->uRead = 0;
  pBuffer->uWrite = 0;
  pBuffer->puMemory = puMemory;
  pBuffer->uSize = uSize;

  return;
}

/*********************************************************************************************************//**
  * @brief  Read byte from buffer.
  * @param  pBuffer: Pointer of Buffer_TypeDef
  * @param  puData: Pointer of returned data
  * @retval uResult: TRUE for read success, FALSE for no DATA
  ***********************************************************************************************************/
u32 Buffer_ReadByte(Buffer_TypeDef *pBuffer, u8 *puData)
{
  u32 uResult = FALSE;
  if (!IS_BUFFER_EMPTY(pBuffer->uRead, pBuffer->uWrite))
  {
    *puData = pBuffer->puMemory[pBuffer->uRead];
    pBuffer->uRead = (pBuffer->uRead + 1) % pBuffer->uSize;
    uResult = TRUE;
  }

  return uResult;
}

/*********************************************************************************************************//**
  * @brief  Write byte to buffer.
  * @param  pBuffer: Pointer of Buffer_TypeDef
  * @param  uData: Data to write
  * @retval uResult: TRUE for write success, FALSE for buffer full
  ***********************************************************************************************************/
u32 Buffer_WriteByte(Buffer_TypeDef *pBuffer, u32 uData)
{
  u32 uResult = FALSE;
  u32 uDataLength;

  if (!IS_BUFFER_FULL(pBuffer->uRead, pBuffer->uWrite, pBuffer->uSize))
  {
    uDataLength = pBuffer->uWrite;
    pBuffer->puMemory[uDataLength] = uData;
    pBuffer->uWrite = (uDataLength + 1) % pBuffer->uSize;
    uResult = TRUE;
  }

  return uResult;
}

/*********************************************************************************************************//**
  * @brief  Read buffer.
  * @param  pBuffer: Pointer of Buffer_TypeDef
  * @param  puData: Pointer of dada buffer for read
  * @param  uLength: Read length
  * @retval uCount: Data length of read operation
  ***********************************************************************************************************/
u32 Buffer_Read(Buffer_TypeDef *pBuffer, u8 *puData, u32 uLength)
{
  u32 uCount;
  u32 uResult;
  for (uCount = 0; uCount < uLength; uCount++)
  {
    uResult = Buffer_ReadByte(pBuffer, (u8 *)(puData + uCount));
    if (!uResult)
    {
      break;
    }
  }

  return uCount;
}

/*********************************************************************************************************//**
  * @brief  Write Buffer.
  * @param  pBuffer: Pointer of Buffer_TypeDef
  * @param  puData: Pointer of dada buffer for write
  * @param  uLength: Write length
  * @retval uCount: Data length of write operation
  ***********************************************************************************************************/
u32 Buffer_Write(Buffer_TypeDef *pBuffer, u8 *puData, u32 uLength)
{
  u32 uCount = 0;
  u32 uResult;
  for (uCount = 0; uCount < uLength; uCount++)
  {
    uResult = Buffer_WriteByte(pBuffer, *(u8 *)(puData + uCount));
    if (!uResult)
    {
      break;
    }
  }

  return uCount;
}

/*********************************************************************************************************//**
  * @brief  Check if buffer is empty.
  * @param  pBuffer: Pointer of Buffer_TypeDef
  * @retval TRUE for buffer is empty, FALSE for buffer is not empty
  ***********************************************************************************************************/
u32 Buffer_isEmpty(Buffer_TypeDef *pBuffer)
{
  return(IS_BUFFER_EMPTY(pBuffer->uRead, pBuffer->uWrite));
}

/*********************************************************************************************************//**
  * @brief  Check if buffer is full.
  * @param  pBuffer: Pointer of Buffer_TypeDef
  * @retval TRUE for buffer is full, FALSE for buffer is not full
  ***********************************************************************************************************/
u32 Buffer_isFull(Buffer_TypeDef *pBuffer)
{
  return(IS_BUFFER_FULL(pBuffer->uRead, pBuffer->uWrite, pBuffer->uSize));
}

/*********************************************************************************************************//**
  * @brief  Discard Buffer data.
  * @param  pBuffer: Pointer of Buffer_TypeDef
  * @retval None
  ***********************************************************************************************************/
void Buffer_Discard(Buffer_TypeDef *pBuffer)
{
  pBuffer->uRead = pBuffer->uWrite = 0;

  return;
}

/*********************************************************************************************************//**
  * @brief  Get valid data length.
  * @param  None
  * @retval Length
  ***********************************************************************************************************/
u32 Get_Valid_Lenth(Buffer_TypeDef *pBuffer)
{
  return pBuffer->uSize - (BUFF_FERR_LENGTH(pBuffer->uRead, pBuffer->uWrite, pBuffer->uSize));
}

/* Private functions ---------------------------------------------------------------------------------------*/


