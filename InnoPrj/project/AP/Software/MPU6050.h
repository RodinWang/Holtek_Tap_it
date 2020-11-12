/*********************************************************************************************************//**
 * @file    MPU6050.h
 * @date    $Date:: 2019-11-05 #$
 *************************************************************************************************************/
#ifndef __MPU6050_H
#define __MPU6050_H


void fun_DMPData_Processing(volatile unsigned short	*dmpdata);
float fun_cal_gyro_data(volatile unsigned char Gyro_L,volatile unsigned char Gyro_H);
float fun_cal_accel_data(volatile unsigned char Accel_L,volatile unsigned char Accel_H);
float fun_cal_temp_data(unsigned char Temp_L,unsigned char Temp_H);

#endif







