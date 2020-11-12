#ifndef __IAP_HANDLER_H
#define __IAP_HANDLER_H


/* Exported macro ------------------------------------------------------------------------------------------*/
#define IAP_CODE_PAGE           (8)
#define IAP_CODE_SIZE           (IAP_CODE_PAGE * 1024)       /* IAP code size in Bytes             */
#define IAP_VERSION_SIZE        (0)
#define IAP_APINFO_SIZE         (0) // The VectorTable must align 128 bytes


#define BOOT_MODE_AP            (0x55AAFAF0)
#define BOOT_MODE_IAP           (0x55AAFAF5)
#define BOOT_MODE_ID_ADDR       (HT_SRAM_BASE)

#define IAP_APFLASH_START       (IAP_CODE_SIZE + IAP_VERSION_SIZE)
#define IAP_APFLASH_END         (LIBCFG_FLASH_SIZE - 1)
#define IAP_APSRAM_START        (HT_SRAM_BASE)
#define IAP_SRAM_END            (IAP_APSRAM_START + LIBCFG_RAM_SIZE - 1)

/* Global variables ----------------------------------------------------------------------------------------*/

/* Exported functions --------------------------------------------------------------------------------------*/
void IAP_Init(void);
void IAP_Handler(void);
u32 IAP_Reset(u32 uMode);
u32 IAP_isAPValid(void);
void IAP_GoCMD(u32 address);
void IAP_TimebaseHandler(void);

#endif /* __IAP_HANDLER_H ----------------------------------------------------------------------------------*/


