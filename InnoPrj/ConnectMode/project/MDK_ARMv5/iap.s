;********************************************************************************
;* @file        iap.s
;* @version     $Rev:: 328          $
;* @date        $Date:: 2016-04-14 #$
;* @brief       Include IAP image
;*
;* @note        Copyright (C) Holtek Semiconductor Inc. All rights reserved.
;*
;* <h2><center>&copy; COPYRIGHT Holtek</center></h2>
;*
;********************************************************************************

;// <q> Include IAP image into user's application
INCLUDE_IAP      EQU     1

        AREA    IAP, DATA, READONLY

        INCBIN  HT32\IAP\IAP.axf.bin

        END


