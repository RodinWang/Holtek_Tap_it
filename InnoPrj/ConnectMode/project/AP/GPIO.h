#ifndef __GPIO_H
#define __GPIO_H

#include "ht32.h"

/*GPIO port define*/
#define CFG_LED1_PORT             D
#define CFG_LED1_PIN              0

#define CFG_LGPIO1_PORT           B
#define CFG_LGPIO1_PIN            8
#define CFG_LGPIO2_PORT           B
#define CFG_LGPIO2_PIN            7
#define CFG_LGPIO3_PORT           B
#define CFG_LGPIO3_PIN            6
#define CFG_LGPIO4_PORT           C
#define CFG_LGPIO4_PIN            3
#define CFG_LGPIO5_PORT           C
#define CFG_LGPIO5_PIN            15
#define CFG_LGPIO6_PORT           C
#define CFG_LGPIO6_PIN            14

#define CFG_RGPIO1_PORT           C
#define CFG_RGPIO1_PIN            0
#define CFG_RGPIO2_PORT           C
#define CFG_RGPIO2_PIN            8
#define CFG_RGPIO3_PORT           C
#define CFG_RGPIO3_PIN            9
#define CFG_RGPIO4_PORT           B
#define CFG_RGPIO4_PIN            15
#define CFG_RGPIO5_PORT           B
#define CFG_RGPIO5_PIN            12
#define CFG_RGPIO6_PORT           B
#define CFG_RGPIO6_PIN            9

#define LED1_PORT                 STRCAT2(HT_GPIO,        CFG_LED1_PORT)
#define LED1_PIN                  STRCAT2(GPIO_PIN_,      CFG_LED1_PIN)

#define LGPIO1_PORT               STRCAT2(HT_GPIO,        CFG_LGPIO1_PORT)
#define LGPIO1_PIN                STRCAT2(GPIO_PIN_,      CFG_LGPIO1_PIN)
#define LGPIO2_PORT               STRCAT2(HT_GPIO,        CFG_LGPIO2_PORT)
#define LGPIO2_PIN                STRCAT2(GPIO_PIN_,      CFG_LGPIO2_PIN)
#define LGPIO3_PORT               STRCAT2(HT_GPIO,        CFG_LGPIO3_PORT)
#define LGPIO3_PIN                STRCAT2(GPIO_PIN_,      CFG_LGPIO3_PIN)
#define LGPIO4_PORT               STRCAT2(HT_GPIO,        CFG_LGPIO4_PORT)
#define LGPIO4_PIN                STRCAT2(GPIO_PIN_,      CFG_LGPIO4_PIN)
#define LGPIO5_PORT               STRCAT2(HT_GPIO,        CFG_LGPIO5_PORT)
#define LGPIO5_PIN                STRCAT2(GPIO_PIN_,      CFG_LGPIO5_PIN)
#define LGPIO6_PORT               STRCAT2(HT_GPIO,        CFG_LGPIO6_PORT)
#define LGPIO6_PIN                STRCAT2(GPIO_PIN_,      CFG_LGPIO6_PIN)

#define RGPIO1_PORT               STRCAT2(HT_GPIO,        CFG_RGPIO1_PORT)
#define RGPIO1_PIN                STRCAT2(GPIO_PIN_,      CFG_RGPIO1_PIN)
#define RGPIO2_PORT               STRCAT2(HT_GPIO,        CFG_RGPIO2_PORT)
#define RGPIO2_PIN                STRCAT2(GPIO_PIN_,      CFG_RGPIO2_PIN)
#define RGPIO3_PORT               STRCAT2(HT_GPIO,        CFG_RGPIO3_PORT)
#define RGPIO3_PIN                STRCAT2(GPIO_PIN_,      CFG_RGPIO3_PIN)
#define RGPIO4_PORT               STRCAT2(HT_GPIO,        CFG_RGPIO4_PORT)
#define RGPIO4_PIN                STRCAT2(GPIO_PIN_,      CFG_RGPIO4_PIN)
#define RGPIO5_PORT               STRCAT2(HT_GPIO,        CFG_RGPIO5_PORT)
#define RGPIO5_PIN                STRCAT2(GPIO_PIN_,      CFG_RGPIO5_PIN)
#define RGPIO6_PORT               STRCAT2(HT_GPIO,        CFG_RGPIO6_PORT)
#define RGPIO6_PIN                STRCAT2(GPIO_PIN_,      CFG_RGPIO6_PIN)

#define Led_ON                    LED1_PORT->DOUTR &= ~LED1_PIN
#define Led_OFF                   LED1_PORT->DOUTR |= LED1_PIN
#define Led_Toggle                LED1_PORT->DOUTR ^= LED1_PIN

/*I2C port define*/
#define I2C0_SCL_GPIO_ID              (GPIO_PA)
#define I2C0_SCL_AFIO_PIN             (AFIO_PIN_4)
#define I2C0_SCL_AFIO_MODE            (AFIO_FUN_I2C)

#define I2C0_SDA_GPIO_ID              (GPIO_PA)
#define I2C0_SDA_AFIO_PIN             (AFIO_PIN_5)
#define I2C0_SDA_AFIO_MODE            (AFIO_FUN_I2C)

/*ADC port define*/
#define AN11_GPIO_ID                  (GPIO_PA)
#define AN11_AFIO_PIN                 (AFIO_PIN_7)
#define AN11_AFIO_MODE                (AFIO_FUN_ADC)

#define AN10_GPIO_ID                  (GPIO_PA)
#define AN10_AFIO_PIN                 (AFIO_PIN_6)
#define AN10_AFIO_MODE                (AFIO_FUN_ADC)

#define AN5_GPIO_ID                   (GPIO_PA)
#define AN5_AFIO_PIN                  (AFIO_PIN_1)
#define AN5_AFIO_MODE                 (AFIO_FUN_ADC)

#define AN4_GPIO_ID                   (GPIO_PA)
#define AN4_AFIO_PIN                  (AFIO_PIN_0)
#define AN4_AFIO_MODE                 (AFIO_FUN_ADC)

void GPIO_PortInit(void);
#endif
