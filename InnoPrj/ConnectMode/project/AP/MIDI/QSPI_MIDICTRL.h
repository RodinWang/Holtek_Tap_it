#include "main.h"

#ifdef SPANSION_ReadID
	void SPANSION_FLASH_ReadID(void);
	#define QSPI_xFLHx_Init() SPANSION_FLASH_ReadID()
	
	#define CMD          0x9F 
	#define CMDVALUE     (CMD<<8)
	#define MDVALUE      0x0
	#define CMDFL        (8<<24)
	#define ADFL         (0<<16)
	#define MDFL         (0<<12)
	#define DMFL         (0<<8)
	#define DATFL        32
	
	#define SMDSEL       (0<<6)
	#define QDIOEN       (0<<22)
	#define MIDICEN      (1<<23)
	
#elif defined SPANSION_DOR
	void SPANSION_FLASH_DOR(void);
	#define QSPI_xFLHx_Init() SPANSION_FLASH_DOR()
	
	#define CMD          0x3B 
	#define CMDVALUE     (CMD<<8)
	#define MDVALUE      0x0
	#define CMDFL        (8<<24)
	#define ADFL         (24<<16)
	#define MDFL         (0<<12)
	#define DMFL         (8<<8)
	#define DATFL        16
	
	#define SMDSEL       (1<<6)
	#define QDIOEN       (0<<22)
	#define MIDICEN      (1<<23)
	
#elif defined SPANSION_DIOR
	void SPANSION_FLASH_DIOR(void);
	#define QSPI_xFLHx_Init() SPANSION_FLASH_DIOR()
	
	#define CMD          0xBB 
	#define CMDVALUE     (CMD<<8)
	#define MDVALUE      0x0
	#define CMDFL        (8<<24)
	#define ADFL         (12<<16)
	#define MDFL         (4<<12)
	#define DMFL         (0<<8)
	#define DATFL        16
	
	#define SMDSEL       (1<<6)
	#define QDIOEN       (1<<22)
	#define MIDICEN      (1<<23)
	
#elif defined SPANSION_QOR
	void SPANSION_FLASH_QOR(void);
	#define QSPI_xFLHx_Init() SPANSION_FLASH_QOR()
	
	#define CMD          0x6B 
	#define CMDVALUE     (CMD<<8)
	#define MDVALUE      0x0
	#define CMDFL        (8<<24)
	#define ADFL         (24<<16)
	#define MDFL         (0<<12)
	#define DMFL         (8<<8)
	#define DATFL        8
	
	#define SMDSEL       (2<<6)
	#define QDIOEN       (0<<22)
	#define MIDICEN      (1<<23)
	
#elif defined SPANSION_QIOR
	void SPANSION_FLASH_QIOR(void);
	#define QSPI_xFLHx_Init() SPANSION_FLASH_QIOR()
	
	#define CMD          0xEB 
	#define CMDVALUE     (CMD<<8)
	#define MDVALUE      0x0
	#define CMDFL        (8<<24)
	#define ADFL         (6<<16)
	#define MDFL         (2<<12)
	#define DMFL         (4<<8)
	#define DATFL        8
	
	#define SMDSEL       (2<<6)
	#define QDIOEN       (1<<22)
	#define MIDICEN      (1<<23)
	
#elif defined MXIC_DOR
	void MXIC_FLASH_DOR(void);
	#define QSPI_xFLHx_Init() MXIC_FLASH_DOR()
	
	#define CMD          0x3B 
	#define CMDVALUE     (CMD<<8)
	#define MDVALUE      0x0
	#define CMDFL        (8<<24)
	#define ADFL         (24<<16)
	#define MDFL         (0<<12)
	#define DMFL         (8<<8)
	#define DATFL        16
	
	#define SMDSEL       (1<<6)
	#define QDIOEN       (0<<22)
	#define MIDICEN      (1<<23)
	
#elif defined MXIC_DIOR
	void MXIC_FLASH_DIOR(void);
	#define QSPI_xFLHx_Init() MXIC_FLASH_DIOR()
	
	#define CMD          0xBB 
	#define CMDVALUE     (CMD<<8)
	#define MDVALUE      0x0
	#define CMDFL        (8<<24)
	#define ADFL         (12<<16)
	#define MDFL         (4<<12)
	#define DMFL         (0<<8)
	#define DATFL        16
	
	#define SMDSEL       (1<<6)
	#define QDIOEN       (1<<22)
	#define MIDICEN      (1<<23)
		
#elif defined MXIC_QOR
	void MXIC_FLASH_QOR(void);
	#define QSPI_xFLHx_Init() MXIC_FLASH_QOR()
	
	#define CMD          0x6B 
	#define CMDVALUE     (CMD<<8)
	#define MDVALUE      0x0
	#define CMDFL        (8<<24)
	#define ADFL         (24<<16)
	#define MDFL         (0<<12)
	#define DMFL         (8<<8)
	#define DATFL        8
	
	#define SMDSEL       (2<<6)
	#define QDIOEN       (0<<22)
	#define MIDICEN      (1<<23)
	
#elif defined MXIC_QIOR
	void MXIC_FLASH_QIOR(void);
	#define QSPI_xFLHx_Init() MXIC_FLASH_QIOR()
	
	#define CMD         0xEB
	#define CMDVALUE    (CMD<<8)
	#define MDVALUE     0x0
	#define CMDFL       (8<<24)
	#define ADFL        (6<<16)
	#define MDFL        (2<<12)
	#define DMFL        (4<<8)
	#define DATFL       8
	
	#define SMDSEL      (2<<6)
	#define QDIOEN      (1<<22)
	#define MIDICEN     (1<<23)
	
#elif defined MXIC_QPI
	void MXIC_FLASH_QPI(void);
	#define QSPI_xFLHx_Init() MXIC_FLASH_QPI()
	
	#define CMD         0xEB
	#define CMDVALUE    (CMD<<8)
	#define MDVALUE     0x0
	#define CMDFL       (2<<24)
	#define ADFL        (6<<16)
	#define MDFL        (2<<12)
	#define DMFL        (4<<8)
	#define DATFL       8
	
	#define SMDSEL      (3<<6)
	#define QDIOEN      (1<<22)
	#define MIDICEN     (1<<23)
	
#elif defined WINBOND_QIOR
	void WINBOND_FLASH_QIOR(void);
	#define QSPI_xFLHx_Init() WINBOND_FLASH_QIOR()
	
	#define CMD          0xEB 
	#define CMDVALUE     (CMD<<8)
	#define MDVALUE      0xF0
	#define CMDFL        (8<<24)
	#define ADFL         (6<<16)
	#define MDFL         (2<<12)
	#define DMFL         (4<<8)
	#define DATFL        8
	
	#define SMDSEL       (2<<6)
	#define QDIOEN       (1<<22)
	#define MIDICEN      (1<<23)
	
#else
	
#endif	

void QSPI_MIDICTRL_Init(void);




