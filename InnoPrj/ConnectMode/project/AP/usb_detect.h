#ifndef __USB_DETECT_H
#define __USB_DETECT_H

#include "ht32.h"
//Must be compatible with hardware(MotherBoard) version 3.0 or above
#define USB_DETECT_EN             (0)
#define USB_INSERT_DELAY          (10)

#define CFG_USB_DETECT_PORT       D
#define CFG_USB_DETECT_PIN        3
#define CFG_LED1_PORT             D
#define CFG_LED1_PIN              0

#define CFG_GPIO1_PORT            C
#define CFG_GPIO1_PIN             14
#define CFG_GPIO2_PORT            C
#define CFG_GPIO2_PIN             15
#define CFG_GPIO3_PORT            C
#define CFG_GPIO3_PIN             3
#define CFG_GPIO4_PORT            B
#define CFG_GPIO4_PIN             6
#define CFG_GPIO5_PORT            B
#define CFG_GPIO5_PIN             7
#define CFG_GPIO6_PORT            B
#define CFG_GPIO6_PIN             8
#define CFG_GPIO7_PORT            B
#define CFG_GPIO7_PIN             9
#define CFG_GPIO8_PORT            B
#define CFG_GPIO8_PIN             12
#define CFG_GPIO9_PORT            C
#define CFG_GPIO9_PIN             8
#define CFG_GPIO10_PORT           C
#define CFG_GPIO10_PIN            9

#define USB_DETECT_PORT           STRCAT2(HT_GPIO,        CFG_USB_DETECT_PORT)
#define USB_DETECT_PIN            STRCAT2(GPIO_PIN_,      CFG_USB_DETECT_PIN)
#define LED1_PORT                 STRCAT2(HT_GPIO,        CFG_LED1_PORT)
#define LED1_PIN                  STRCAT2(GPIO_PIN_,      CFG_LED1_PIN)

#define GPIO1_PORT                STRCAT2(HT_GPIO,        CFG_GPIO1_PORT)
#define GPIO1_PIN                 STRCAT2(GPIO_PIN_,      CFG_GPIO1_PIN)
#define GPIO2_PORT                STRCAT2(HT_GPIO,        CFG_GPIO2_PORT)
#define GPIO2_PIN                 STRCAT2(GPIO_PIN_,      CFG_GPIO2_PIN)
#define GPIO3_PORT                STRCAT2(HT_GPIO,        CFG_GPIO3_PORT)
#define GPIO3_PIN                 STRCAT2(GPIO_PIN_,      CFG_GPIO3_PIN)
#define GPIO4_PORT                STRCAT2(HT_GPIO,        CFG_GPIO4_PORT)
#define GPIO4_PIN                 STRCAT2(GPIO_PIN_,      CFG_GPIO4_PIN)
#define GPIO5_PORT                STRCAT2(HT_GPIO,        CFG_GPIO5_PORT)
#define GPIO5_PIN                 STRCAT2(GPIO_PIN_,      CFG_GPIO5_PIN)
#define GPIO6_PORT                STRCAT2(HT_GPIO,        CFG_GPIO6_PORT)
#define GPIO6_PIN                 STRCAT2(GPIO_PIN_,      CFG_GPIO6_PIN)
#define GPIO7_PORT                STRCAT2(HT_GPIO,        CFG_GPIO7_PORT)
#define GPIO7_PIN                 STRCAT2(GPIO_PIN_,      CFG_GPIO7_PIN)
#define GPIO8_PORT                STRCAT2(HT_GPIO,        CFG_GPIO8_PORT)
#define GPIO8_PIN                 STRCAT2(GPIO_PIN_,      CFG_GPIO8_PIN)
#define GPIO9_PORT                STRCAT2(HT_GPIO,        CFG_GPIO9_PORT)
#define GPIO9_PIN                 STRCAT2(GPIO_PIN_,      CFG_GPIO9_PIN)
#define GPIO10_PORT               STRCAT2(HT_GPIO,        CFG_GPIO10_PORT)
#define GPIO10_PIN                STRCAT2(GPIO_PIN_,      CFG_GPIO10_PIN)

#define Led_ON                    LED1_PORT->DOUTR &= ~LED1_PIN
#define Led_OFF                   LED1_PORT->DOUTR |= LED1_PIN
#define Led_TOGGLE                LED1_PORT->DOUTR ^= LED1_PIN

void USB_Detect_Init(void);
void USB_Detect_Handler(void);
#endif
