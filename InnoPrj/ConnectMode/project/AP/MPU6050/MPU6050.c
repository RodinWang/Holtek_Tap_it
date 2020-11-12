/*********************************************************************************************************//**
 * @file    MPU6050.c
 * @date    $Date:: 2019-11-05 #$
 *************************************************************************************************************
 * @attention
 *
 * Firmware Disclaimer Information
 *
 * 1. The customer hereby acknowledges and agrees that the program technical documentation, including the
 *    code, which is supplied by Holtek Semiconductor Inc., (hereinafter referred to as "HOLTEK") is the
 *    proprietary and confidential intellectual property of HOLTEK, and is protected by copyright law and
 *    other intellectual property laws.
 *
 * 2. The customer hereby acknowledges and agrees that the program technical documentation, including the
 *    code, is confidential information belonging to HOLTEK, and must not be disclosed to any third parties
 *    other than HOLTEK and the customer.
 *
 * 3. The program technical documentation, including the code, is provided "as is" and for customer reference
 *    only. After delivery by HOLTEK, the customer shall use the program technical documentation, including
 *    the code, at their own risk. HOLTEK disclaims any expressed, implied or statutory warranties, including
 *    the warranties of merchantability, satisfactory quality and fitness for a particular purpose.
 *
 * <h2><center>Copyright (C) Holtek Semiconductor Inc. All rights reserved</center></h2>
 ************************************************************************************************************/
/* Includes ------------------------------------------------------------------------------------------------*/
#include "ht32.h"
#include "ht32_board.h"
#include "MPU6050.h"
#include "math.h"


volatile float Yaw=0.00,Roll,Pitch;
volatile float q0,q1,q2,q3;


/*********************************************************************************************************//**
  * @brief  Calculate Euler angle
	* @param	dmpdata:8 Bytes DMP data
  * @retval None
  ***********************************************************************************************************/
void fun_DMPData_Processing(volatile unsigned short	*dmpdata)
{
	volatile short quat[4];
	
	quat[0] = (short)((dmpdata[1]<<8) + dmpdata[0]);	//QW
	quat[1] = (short)((dmpdata[3]<<8) + dmpdata[2]);	//QX
	quat[2] = (short)((dmpdata[5]<<8) + dmpdata[4]);	//QY
	quat[3] = (short)((dmpdata[7]<<8) + dmpdata[6]);	//QZ
	
	q0 = quat[0]/16383.0f;
	q1 = quat[1]/16383.0f;
	q2 = quat[2]/16383.0f;
	q3 = quat[3]/16383.0f;
	
	Roll = asin(-2 * q1 * q3 + 2 * q0* q2)* 57.3; 	// roll
	Pitch = atan2(2 * q2 * q3 + 2 * q0 * q1, -2 * q1 * q1 - 2 * q2* q2 + 1)* 57.3; // pitch
	Yaw = atan2(2*(q1*q2 + q0*q3),q0*q0+q1*q1-q2*q2-q3*q3) * 57.3;	// yaw

}


/*********************************************************************************************************//**
  * @brief  Calculate Gyroscope  data
	* @param	Gyro_L:low byte of the Gyroscope data
	* @param	Gyro_H:high byte of the Gyroscope data

* @retval gyro_data unit:degree/s
  ***********************************************************************************************************/
float fun_cal_gyro_data(volatile unsigned char Gyro_L,volatile unsigned char Gyro_H)
{
	volatile float sens,gyro_data;
	volatile short temp;
	
        sens = 16.4f;
 
	
	temp = (Gyro_H << 8) | Gyro_L;
	gyro_data = temp / sens;

	return gyro_data;
}

/*********************************************************************************************************//**
  * @brief  Calculate Accelerometer data
	* @param	Accel_L:low byte of the Accelerometer data
	* @param	Accel_H:high byte of the Accelerometer data

* @retval accel_data unit:g(1g=9.8m/s^2)
  ***********************************************************************************************************/
float fun_cal_accel_data(volatile unsigned char Accel_L,volatile unsigned char Accel_H)
{
	volatile float sens,accel_data;
	volatile short temp;
	

        sens = 16384.0f;


	temp = (Accel_H << 8) | Accel_L;
	accel_data = temp / sens;

	return accel_data;
}

/*********************************************************************************************************//**
  * @brief  Calculate Temperature data
	* @param	Temp_L:low byte of the Temperture data
	* @param	Temp_H:high byte of the Temperture data
	* @retval temp_data unit: degrees C
  ***********************************************************************************************************/
float fun_cal_temp_data(unsigned char Temp_L,unsigned char Temp_H)
{
	volatile float temp_data;
	volatile float temp;
	
	temp = (float)((Temp_H << 8) | Temp_L);
	if(temp >= 0x8000)		// Negative number
	{
		temp_data = -(65535 - temp + 1)/340.0f + 36.53f;
	}
	else				// Positive number
	{
		temp_data = (temp/340.0f) + 36.53f;
	}
	
	
	return temp_data;
}





