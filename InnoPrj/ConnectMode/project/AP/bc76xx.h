#ifndef _BC760X_H_
#define _BC760X_H_

#include "ht32.h"
// <<< Use Configuration Wizard in Context Menu >>>

//  <h> BC760x Interface Configuration
//    <o> Interface Configuration
//       <0=> USART
//       <1=> SPI
#define _BC76XX_SPI_UART_  (1)

//    <o> UART Baud Rate
//       <2400=>2400Bps   
//       <9600=>9600Bps 
//       <14400=>14400Bps
//       <19200=>19200Bps
//       <38400=>38400Bps
//       <57600=>57600Bps
//       <115200=>115200Bps
#define _BC76XX_UART_BAUD_RATE_  (115200)

//    <o> SPI Speed<1000-10000000>
#define _BC76XX_SPI_SPEED_  (2000000UL)
//  </h>

#define  USART_MODE        0
#define  SPI_MODE          1
#define	BC76XX_USART		HT_USART0
#define	BC76XX_USART_IRQn	USART0_IRQn
#define  BC76XX_SPI     	HT_SPI0			/* SPI port */

#if (_BC76XX_SPI_UART_ == USART_MODE)
	#define	_BC76XX_RTS_		0					/*PA0*/
	#define	_BC76XX_CTS_		1					/*PA1*/
	#define  _BC76XX_TXD_    	2              /*PA2*/
	#define  _BC76XX_RXD_     	3              /*PA3*/
	#define	_BC76XX_STATE_		4					/*PA4*/
	#define	_BC76XX_PDN_		5					/*PA5*/
	#define	_BC76XX_ESCL_		6					/*PA6*/
	#define	_BC76XX_ESDA_		7					/*PA7*/
	#define	_BC76XX_SPIUR_		4					/*PC4*/
	#define	_BC76XX_WAKEUP_	5					/*PC5*/
	#define	_BC76XX_EXTINT_	6					/*PC6*/
	#define	_BC76XX_EWP_		7					/*PC7*/
	#define  _BC76XX_RES_     	9              /*PB9*/
	
	#define	BC76XX_RTS			   (1UL << _BC76XX_RTS_)
	#define	BC76XX_CTS			   (1UL << _BC76XX_CTS_)
	#define  BC76XX_TXD      		(1UL << _BC76XX_TXD_)
	#define  BC76XX_RXD      	   (1UL << _BC76XX_RXD_)
	
#else
	#define	_BC76XX_PDN_		  11				/*PA11*/
  #define	_BC76XX_EXTINT_	  15				/*PA15*/
  #define	_BC76XX_WAKEUP_ 	1					/*PD1*/
  #define _BC76XX_RES_     	14        /*PA14*/
  #define _BC76XX_STATE_    2         /*PD2*/
  
	#define	_BC76XX_SCK_		  3					/*PB3*/
	#define	_BC76XX_MOSI_		  4					/*PB4*/
	#define _BC76XX_MISO_    	5         /*PB5*/
	#define  _BC76XX_CSN_     2         /*PB2*/	

	#define	BC76XX_SCK			  (1UL << _BC76XX_SCK_)
	#define	BC76XX_MOSI			  (1UL << _BC76XX_MOSI_)
	#define BC76XX_MISO      	(1UL << _BC76XX_MISO_)
	#define BC76XX_CSN        (1UL << _BC76XX_CSN_)

#endif

#define	BC76XX_PDN			    (1UL << _BC76XX_PDN_)
#define	BC76XX_EXTINT		    (1UL << _BC76XX_EXTINT_)
#define	BC76XX_WAKEUP		    (1UL << _BC76XX_WAKEUP_)
#define BC76XX_RES      	  (1UL << _BC76XX_RES_)
#define BC76XX_STATE      	(1UL << _BC76XX_STATE_)

#define BC76XX_PDN_HIGH   	(HT_GPIOA->DOUTR |= BC76XX_PDN)
#define BC76XX_PDN_LOW    	(HT_GPIOA->DOUTR &= ~BC76XX_PDN)
#define BC76XX_PDN_STS      (HT_GPIOA->DOUTR & BC76XX_PDN)

#define	BC76XX_EXTINT_IN		(HT_GPIOA->DINR & BC76XX_EXTINT)
#define BC76XX_STATE_IN     (HT_GPIOD->DINR & BC76XX_STATE)

#define	BC76XX_WAKEUP_HIGH  (HT_GPIOD->DOUTR |= BC76XX_WAKEUP)
#define	BC76XX_WAKEUP_LOW	  (HT_GPIOD->DOUTR &= ~BC76XX_WAKEUP)
#define	BC76XX_WAKEUP_STS	  (HT_GPIOD->DOUTR & BC76XX_WAKEUP)

#define BC76XX_RES_HIGH   	(HT_GPIOA->DOUTR |= BC76XX_RES)
#define BC76XX_RES_LOW    	(HT_GPIOA->DOUTR &= ~BC76XX_RES)
#define BC76XX_RES_STS     	(HT_GPIOA->DOUTR & BC76XX_RES)

typedef enum {POWER_DOWN = FALSE, POWER_UP = !POWER_DOWN} PowerStatus;
typedef enum {LOW = 0, HIGH = !LOW} IOStatus;

/* BC76XX SPI register */
enum 
{
   SPI_REGS_CR0 = 0,
   SPI_REGS_CR1,
   SPI_REGS_CR2,
   SPI_REGS_CR3,
   SPI_REGS_CR4,
   SPI_REGS_CR5,
   SPI_REGS_CR6,
   SPI_REGS_CR7,   
   SPI_REGS_CR8   
};

typedef union
{
	struct
	{
		u16 rx_fifo_hold : 6;	/* SPI Rx FIFO threshold */
		u16 tx_fifo_hold : 6;	/* SPI Tx FIFO threshold */
		u16 : 4;
	}b;
	u16	w;
}CR0_TypeDef;

typedef union
{
	struct
	{
		u16 rx_undef_hold : 1;		/* BLE Rx FIFO under threshold */
		u16 rx_fifo_empty : 1;		/* BLE Rx FIFO empty */		
		u16 tx_over_hold : 1;		/* BLE Tx FIFO over threshold */
		u16 tx_fifo_over : 1;		/* BLE Tx FIFO overflow */
		u16 tx_fifo_noempty : 1;	/* BLE Tx FIFO not empty */		
		u16 : 11;
	}b;
	u16	w;
}CR13_TypeDef;

typedef union
{
	struct
	{
		u16 rx_fifo_count : 6;	/* SPI Rx FIFO counter */
		u16 tx_fifo_count : 6;	/* SPI Tx FIFO counter */
		u16 : 4;
	}b;
	u16	w;
}CR4_TypeDef;

typedef union
{
	struct
	{
		u16 analysing : 1;		/*Status_BLE_Cmd_Analy sing,0: No analy sis,1: Analy sing */
		u16 connected : 1;		/*Status_BLE_Connection,0: Not connected,1: Connected */
		u16 uuid_notify : 1;		/*Status_BLE_UUID_0xFFF1,0: Disable Notify,1: Enable Notify */
		u16 : 3;
		u16 wait_patch : 1;		/*Status_Wait_For_Patch,0: Not necessary to download the patch,1: Wait for patch download*/
		u16 buffer_reset : 1;	/*Status_Buffer_Reset */
		u16 soft_reset : 1;		/*Status_Soft_Reset */
		u16 : 1;	
		u16 auto_sleep : 1;		/*Status_BLE_Can_Sleep */
		u16 int_occurs : 1;		/*Status INT_EXT_STATUS*/
		u16 : 2;		
		u16 sleep_state : 1;		/*Status_BLE_Sleep,0: Active,1: Sleep*/
		u16 rom_init_sts : 1;	/*Status_BLE_ROM_Init,0: Not ready,1: Ready */
	}b;
	u16	w;
}CR5_TypeDef;


typedef union
{
	struct
	{
		u16 : 7;		
		u16 buffer_reset : 1;	/* Set_Buffer_Reset/Clr_Buffer_Reset */
		u16 soft_reset : 1;		/* Set_Soft_Reset/Clr_Soft_Reset */
		u16 : 1;				
		u16 auto_sleep : 1;		/* Set_BLE_Can_Sleep/Clr_BLE_Can_Sleep */
		u16 : 6;
	}b;
	u16	w;
}CR78_TypeDef;

/* HCI packet type */
typedef union 
{
   u8 value;
	struct 
   {
	   u8 t1ms : 1;
	   u8 t10ms : 1;
	   const u8 : 5;
		u8 tc_finish : 1;
	} bits;
} timer_operate_t;

enum
{
   HCI_CMD_PKG = 0x01,
   HCI_ACL_PKG = 0x02,
   HCI_SCL_PKG = 0x03,
   HCI_EVENT_PKG = 0x04
};

#define  EVENT_CMD_COMPLETE   0x0E

#define  HCI_RESET_OPCODE     0x0C03	
#define  HCI_RX_TEST_OPCODE   0x201D
#define  HCI_TX_TEST_OPCODE   0x201E
#define  HCI_END_TEST_OPCODE  0x201F

/* DTM data format */
enum
{
	DTM_FORMAT_PRBS9 = 0,
	DTM_FORMAT_11110000,
	DTM_FORMAT_10101010,
	DTM_FORMAT_PRBS15,
	DTM_FORMAT_11111111,
	DTM_FORMAT_00000000,	
	DTM_FORMAT_00001111,
	DTM_FORMAT_01010101
};

/* BC76XX ACI packet type */
#define  CTRL_COMD         0x25
#define  CTRL_RETURN       0x26
#define  READ_INFO         0x20
#define  INFO_RETURN       0x21
#define  DATA_PACKET       0x22
#define  WRITE_PHY         0x55
#define  READ_PHY          0x56
#define  READ_PHY_RETURN   0x57
#define  COMD_PACKET       0x27

/* BC76XX PHY read/write address */
#define  PHY_TX_BUFF_SIZE_ADR 	0x00200480UL
#define  PHY_FIRMWARE_VER_ADR 	0x00200484UL
#define  PHY_PATCH_START_ADR  	0x00200500UL
#define  PHY_PATCH_READY_ADR  	0x00201F80UL
#define  PHY_PATCH_STATE_ADR  	0x00602020UL
#define  PHY_PATCH_SUCCESS    	0x00000000UL
#define  PHY_PARAM_INDX_ADR   	0x00200488UL
#define  PHY_PARAM_SIZE_ADR   	0x0020048CUL
#define  PHY_IO_ACTIVE_ADR   		0x00200494UL

#define	PHY_UUID_TRIGGER_ADR		0x002004ACUL
#define	PHY_UUID_CHARGE_ADR		0x002004B0UL
#define	PHY_EEPROM_FUN_ADR		0x002004C4UL

#define	PHY_CRYSTAL_ADR			0x002004A4UL
#define  PHY_CRYSTAL_CL_ADR   	0x00608038UL
#define  PHY_DTM_RX_CNT_ADR   	0x00200498UL
#define  PHY_DTM_TX_CNT_ADR   	0x0020049CUL

#define  PHY_E2P_CHECK_ADR 		0x00200134UL
#define  PHY_E2P_CHECK_DISABLE	0x00000001UL
#define  PHY_E2P_DISABLE_ADR 		0x00200120UL
#define  PHY_E2P_DISABLE 			0x00000000UL
#define	PHY_ISR_DISABLE_ADR		0xE000E180UL
#define	PHY_ISR_DISABLE			0x00000200UL
#define  PHY_GPIO_CTRL_ADR 		0x00608064UL
#define  PHY_GPIO_DIR_ADR  		0x00608048UL
#define  PHY_GPIO_PULL_ADR  		0x00608070UL
#define  PHY_GPIO_OUT_ADR  		0x0060804CUL
#define  PHY_GPIO_IN_ADR	   	0x00608050UL
#define  PHY_WAKEUP_VAL_ADR		0x00608058UL
#define	PHY_CLKOUT_EN_ADR			0x002004A8UL			/* for DTM */
#define	PHY_CLKOUT_ENABLE			0x00000001UL			/* for DTM */
#define	PHY_CLKOUT_PERIOD_ADR	0x002004ACUL         /* for DTM */

/* BC76XX command */
#define  CONNECT_INTERVAL  0x30
#define  MAC_NAME          0x31
#define  UART_BAUD_RATE    0x32
#define  MAC_ADDRESS       0x33
#define  ADV_RESET_CTRL    0x34
#define  ADV_INTERVAL      0x35
#define  ADV_DATA          0x36
#define  CONNECTABLE_ADDR  0x37
#define  TX_POWER_CTRL     0x38
#define  BATTERY_VALUE     0x3B
#define  ADV_WORK_CTRL     0x3E
#define  CHANGE_BAUDRATE   0x3F
#define  CONNECT_INTERVAL2 0x40
#define  ADV_DATA2   		0x50
#define	SCAN_RESPONSE	   0x51
#define  DISCONNECT_CTRL   0x5F


/* UUID & property Change */
#define	CHARACTER_S_UUID 				0x0002
#define	CHARACTER_W_UUID 				0x0004
#define	CHARACTER_W_PROPERTY 		0x0008
#define	CHARACTER_R_UUID 				0x0010
#define	CHARACTER_R_PROPERTY 		0x0020

#define 	W_PROPERTY_WRITENORES		0x0000
#define 	W_PROPERTY_WRITE        	0x0200

#define 	R_PROPERTY_NOTIFY       	0x0000
#define 	R_PROPERTY_INDICATE			0x0800


enum
{
   BAUD_RATE_2400 = 0,
   BAUD_RATE_9600,
   BAUD_RATE_14400,
   BAUD_RATE_19200,
   BAUD_RATE_38400,
   BAUD_RATE_57600,
   BAUD_RATE_115200,
   BAUD_RATE_256000   
};

enum
{
   TX_POWER_LEVEL0 = 0,       /* TX power = +3dBm */
   TX_POWER_LEVEL1,           /* TX power = 0dBm */
   TX_POWER_LEVEL2,           /* TX power = -6dBm */
   TX_POWER_LEVEL3,           /* TX power = -12dBm */
   TX_POWER_LEVEL4            /* TX power = -18dBm */
};


#define   PACKET_MAX_SIZE   202

typedef __packed struct 
{
   u8    type;                         /* ACI packet type */
	u8    cmd;                          /* ACI control command */
	u8    leng;                         /* data length */
   u8    data[16];                     /* control data */
}  NAME_PACKAGE;

typedef __packed struct 
{
   u8    type;                         /* ACI packet type */
	u8    cmd;                          /* ACI control command */
	u8    leng;                         /* data length */
   u8    data[];                       /* control data */
}  CTRL_PACKAGE;

typedef __packed struct 
{
   u8    type;                         /* ACI packet type */
	u8    cmd;                          /* ACI control command */
   u8    value;                        /* return value */
}  RETURN_PACKAGE;

typedef __packed struct 
{
   u8    type;                         /* ACI packet type */
	u8    cmd;                          /* read control command */ 
}  READ_INF_PACKAGE;

typedef __packed struct 
{
   u8    type;                         /* ACI packet type */
	u8    cmd;                          /* ACI control command */
	u8    leng;                         /* data length */
   u8    data[];                       /* control data */
}  RETURN_INF_PACKAGE;

typedef __packed struct 
{
   u8    type;                         /* ACI packet type */
	u8    leng;                         /* data length */ 
   u8    data[];                       /* data posint */
}  MESSAGE_PACKAGE;

typedef __packed struct 
{
   u8    type;                         /* ACI packet type */
	u8    leng;                         /* data length */
   u32   addr;
}  READ_PHY_PACKAGE;

typedef __packed struct 
{
   u8    type;                         /* ACI packet type */
	u8    leng;                         /* data length */
   u16   reserve;
   u32   addr;
   u32   data[];                       /* data posint */
}  PHY_PACKAGE;

typedef __packed struct 
{
   u8 pkg_type;
   u16 opcode;
   u8 leng;
   u8 param[];
}HCI_COMMAND_PACKAGE;

typedef __packed struct 
{
   u8 pkg_type;
   u8 eventcode;
   u8 leng;
   u8 param_num;
   u8 param[];
}HCI_EVENT_PACKAGE;


typedef __packed struct 
{	
   u8 	rcv_head;
   u8 	rcv_tail;
   u8 	*rcv_buffer;
	u8		pkg_step;
	u8 	pkg_index;
	u8		pkg_leng;
	u8		*pkg_buffer;
}PARSER_OPERATE;

/* BC76XX ACI packet parser step */ 
enum
{
   RCV_PKG_TYPE = 0x01,
   RCV_ACI_CMD = 0x40,
   RCV_ACI_LENG,
   RCV_ACI_LENG32,
   RCV_ACI_DATA,
   RCV_HCI_EVENT = 0x60,
   RCV_EVENT_LENG,
   RCV_EVENT_PARAM
};


extern u8 transmit_data[];
extern u8 transmit_data_len;
extern vu32 BLE_delay;
extern timer_operate_t timer_operate;

void delay_10us(u32 n); 
void delay_ms(u16 n); 

void  BC76XX_Init(void);
void 	BC76xx_interface_configure(void);
void 	BC76xx_power_mode(PowerStatus Mode);
void 	BC76xx_wakeup(IOStatus State);
IOStatus BC76xx_get_wakeup(void);
IOStatus BC76xx_get_state(void);
IOStatus BC76xx_get_extint(void);
void 	BC76xx_send_hci_cmd_pkg(u16 opcode,u8 len,u8 *pbuf);
void  BC76xx_send_ctrl_cmd_pkg(u8 cmd,u8 len,u8 *pbuf);
void  BC76xx_send_read_info_pkg(u8 cmd);
void  BC76xx_send_data_pkg(void);
void  BC76xx_send_read_phy_pkg(u32 adr,u8 len);
void  BC76xx_send_write_phy_pkg(u32 adr,u8 len,u8 *pbuf,bool dir);
void  BC76xx_send_packet(u8 *pkb,u8 *dkb);
bool 	BC76xx_rcv_packet_parser(PARSER_OPERATE *op);
void  BC76xx_multibyte_write(u8 *pbuf,u16 length);
u16 	BC76xxSPI_read_register(u8 reg);
void 	BC76xxSPI_write_register(u8 reg,u16 value);
void 	BC76xxSPI_read_fifo(u8 *pbuf,u8 length);
void 	BC76xxSPI_write_fifo(u8 *pbuf,u8 length);
#endif   /* _BC760X_H_ */
