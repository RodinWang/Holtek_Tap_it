#ifndef __BUFFER_H
#define __BUFFER_H

/* Includes ------------------------------------------------------------------------------------------------*/
#include "ht32.h"

#define BUFFER_ADDITIONAL_SIZE  (1)

typedef struct
{
  u32 uRead;
  vu32 uWrite;
  u8 *puMemory;
  u32 uSize;
} Buffer_TypeDef;

/* Exported functions --------------------------------------------------------------------------------------*/
void Buffer_Init(Buffer_TypeDef *pBuffer, u8 *puMemory, u32 uSize);
u32 Buffer_ReadByte(Buffer_TypeDef *pBuffer, u8 *puData);
u32 Buffer_WriteByte(Buffer_TypeDef *pBuffer, u32 uData);
u32 Buffer_Read(Buffer_TypeDef *pBuffer, u8 *puData, u32 uLength);
u32 Buffer_Write(Buffer_TypeDef *pBuffer, u8 *puData, u32 uLength);
u32 Buffer_isEmpty(Buffer_TypeDef *pBuffer);
u32 Buffer_isFull(Buffer_TypeDef *pBuffer);
void Buffer_Discard(Buffer_TypeDef *pBuffer);
u32 Get_Valid_Lenth(Buffer_TypeDef *pBuffer);
#endif
