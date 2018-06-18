/*
 * openEL_SensorMPU9250.c
 *
 *  Created on: 2018/05/19
 *      Author: OpenEL-WG
 */

#include <HAL4RT.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

#include "openEL.h"
#include "openEL_registry.h"
#include "openEL_SensorMPU9250.h"

#include <stdlib.h>
#include <strings.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

#include <getopt.h>

/* 9軸センサモジュールに対応するデバイスファイル */
//static char serial_port[128] = "/dev/ttyACM0" ;
static char serial_port[128] = "/dev/cu.usbmodem9AXIS_01" ;

#define BAUDRATE B115200 //ボーレートの設定

int32_t fd; /* シリアル通信ファイルディスクリプタ */
char *com;
struct termios newtio, oldtio;    /* シリアル通信設定 */
char *c,buff[256];

static const char strName[] = "RT-USB-9AXIS";
static const char *strFncLst[] = {
	"HalInit",
	"HalReInit",
	"HalFinalize",
	"HalAddObserver",
	"HalRemoveObserver",
	"HalGetProperty",
	"HalGetTime",
	"HalSensorGetTimedValue",
	"HalSensorGetTimedValueList"
};
static const HALPROPERTY_T sen1_property = {
	(char *)strName,
	(char **)strFncLst,
	sizeof(strFncLst)/sizeof(char *)
};

static int32_t timeOrg;
static HALFLOAT_T sensorValueAr[16];
static HALFLOAT_T simSensor1_x,simSensor1_y;

int16_t acc_ref_x;
int16_t acc_ref_y;
int16_t acc_ref_z;
int16_t omega_ref_x;
int16_t omega_ref_y;
int16_t omega_ref_z;

static HALRETURNCODE_T fncInit3(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) {
	time_t timeWk;

	timeWk = time(&timeWk);
	timeOrg = (int32_t)timeWk;
	((HALSENSOR_T *)pHalComponent)->valueList = sensorValueAr;

	printf("HalInit Sensor1 HAL-ID %d %d %d %d\n",
			pHalComponent->halId.deviceKindId,
			pHalComponent->halId.vendorId,
			pHalComponent->halId.productId,
			pHalComponent->halId.instanceId );

	if(!(fd = open(serial_port, O_RDWR ))) return HAL_ERROR; /* デバイスをオープンする */
//	ioctl(fd, TCGETS, &oldtio);       /* 現在のシリアルポートの設定を待避 */
	tcgetattr(fd, &oldtio);
	bzero(&newtio, sizeof(newtio));
	newtio = oldtio;                  /* ポートの設定をコピー */
	cfmakeraw(&newtio);
	cfsetispeed(&newtio, BAUDRATE);
	cfsetospeed(&newtio, BAUDRATE);
//	ioctl(fd, TCSETS, &newtio);       /* ポートの設定を有効にする */
	tcsetattr(fd, TCSANOW, &newtio);

	int32_t concatenation, i, j=0, k=0, value[100][10];
	uint8_t data_H;
	uint8_t data_L;

  for(k=0;k<100;k++) {
		i=read(fd,buff,sizeof(buff)-1);
//	  printf("=====len=%d byte====== %c",i,0x0a);

		for(i=8;i<28;i=i+2) {
			data_L = (0xff&buff[i]);
			data_H = (0xff&buff[i+1]);
			concatenation = data_L + (data_H<<8);
			if(concatenation >= 32767)
				concatenation = concatenation - 65535;
//			printf("%d:%d %c",i, concatenation,0x0a);
			value[k][j] = concatenation;
			j++;
		}
		j=0;
	}
	for(i=0;i<10;i++) {
		int32_t total = 0;
		for(k=0;k<100;k++) {
			total = total + value[k][i];
		}
		printf("%d:%d\n", i, total/100);
		if(i == 0)
			acc_ref_x = total/100;
		if(i == 1)
			acc_ref_y = total/100;
		if(i == 2)
			acc_ref_z = total/100;
		if(i == 4)
			omega_ref_x = total/100;
		if(i == 5)
			omega_ref_y = total/100;
		if(i == 6)
			omega_ref_z = total/100;
	}
	return HAL_OK;
}

static HALRETURNCODE_T fncReInit(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) {
	return HAL_OK;
}

static HALRETURNCODE_T fncFinalize(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) {
//	ioctl(fd, TCSETS, &oldtio);       /* ポートの設定を元に戻す */
	tcsetattr(fd, TCSANOW, &oldtio);
	close(fd);                        /* デバイスのクローズ */
	return HAL_OK;
}

static HALRETURNCODE_T fncAddObserver(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) {
	return HAL_OK;
}

static HALRETURNCODE_T fncRemoveObserver(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) {
	return HAL_OK;
}

static HALRETURNCODE_T fncGetProperty(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) {
	pHalComponent->property = (HALPROPERTY_T *)&sen1_property;
	return HAL_OK;
}

static HALRETURNCODE_T fncHalGetTime(HALCOMPONENT_T *halComponent,HAL_ARGUMENT_T *pCmd) {
#ifdef HAL_SW_NOT_COMPATIBLE_REL20180524_B
	time_t timeWk;

	timeWk = time(&timeWk);
	halComponent->time = (int32_t)timeWk - timeOrg;
	return HAL_OK;
#else /* HAL_SW_NOT_COMPATIBLE_REL20180524 */
	time_t timeWk;

	timeWk = time(&timeWk);
	pCmd->num = (int32_t)timeWk - timeOrg;
	return HAL_OK;
#endif
}

/** アクチュエーター用API エラー返信 */
static HALRETURNCODE_T fncSetVal(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) {
	return HAL_ERROR;
}

/** アクチュエーター用API エラー返信 */
static HALRETURNCODE_T fncGetVal(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) {
	return HAL_ERROR;
}

static HALRETURNCODE_T fncGetValLst(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) { //uint32_t *pOutSize,HALFLOAT_T *pOutValLst) {
	HALSENSOR_T *halSensor = (HALSENSOR_T *)pHalComponent;
	int32_t i,j,k;
//	halSensor->valueList[0] = atan2(simSensor1_y,simSensor1_x);
//	i=read(fd,buff,sizeof(buff)-1);
	i=read(fd,buff,28);
	pCmd->num = i;
//  printf("=====len=%d byte====== %c",i,0x0a);
  for(j=0;j<i;j++){
//    printf("|%u|%x ||  %c",j,(0xff&buff[j]),0x0a);
//		halSensor->valueList[j] = (0xff&buff[j]);
		if( (0xff&buff[j]) == 0xff && (0xff&buff[j+1]) == 0xff && (0xff&buff[j+2]) == 0x52 && (0xff&buff[j+3]) == 0x54) {
			k = j;
			break;
		}
  }
//	for(j=0;j<28;j++){
//		halSensor->valueList[j] = (0xff&buff[k]);
//		k++;
//  }
	int32_t concatenation;
	uint8_t data_H;
	uint8_t data_L;

	for(i=0;i<10;i++) {
		data_L = (0xff&buff[k+8]);
		data_H = (0xff&buff[k+9]);
		concatenation = data_L + (data_H<<8);
		if(concatenation >= 32767)
			concatenation = concatenation - 65535;
		if(i == 0)
//			halSensor->valueList[i] = (HALFLOAT_T)(concatenation - acc_ref_x) / 2048; // [g]
			halSensor->valueList[i] = (HALFLOAT_T)(concatenation - acc_ref_x) / 2048 / 9.8; // [m/s^2]
		if(i == 1)
//			halSensor->valueList[i] = (HALFLOAT_T)(concatenation - acc_ref_y) / 2048; // [g]
			halSensor->valueList[i] = (HALFLOAT_T)(concatenation - acc_ref_y) / 2048 / 9.8; // [m/s^2]
		if(i == 2)
//			halSensor->valueList[i] = (HALFLOAT_T)((concatenation) - (acc_ref_z - 1)) / 2048; // [g]
			halSensor->valueList[i] = (HALFLOAT_T)((concatenation) - (acc_ref_z - 1)) / 2048 / 9.8; // [m/s^2]
		if(i == 3)
//			halSensor->valueList[i] = (HALFLOAT_T)concatenation / 340 + 35; // [degrees Celsius]
			halSensor->valueList[i] = (HALFLOAT_T)concatenation / 340 + 35 + 273.15; // [Kelbin]
//			halSensor->valueList[i] = ((HALFLOAT_T)concatenation / 340 + 35) * 1.8 + 32; // [degrees Fahrenheit]
		if(i == 4)
//			halSensor->valueList[i] = (HALFLOAT_T)(concatenation - omega_ref_x) / 16.4; // [deg/sec]
			halSensor->valueList[i] = (HALFLOAT_T)(concatenation - omega_ref_x) / 16.4 / 180 * M_PI; // [rad/sec]
		if(i == 5)
//			halSensor->valueList[i] = (HALFLOAT_T)(concatenation - omega_ref_y) / 16.4; // [deg/sec]
			halSensor->valueList[i] = (HALFLOAT_T)(concatenation - omega_ref_y) / 16.4 / 180 * M_PI; // [rad/sec]
		if(i == 6)
//			halSensor->valueList[i] = (HALFLOAT_T)(concatenation - omega_ref_z) / 16.4; // [deg/sec]
			halSensor->valueList[i] = (HALFLOAT_T)(concatenation - omega_ref_z) / 16.4 / 180 * M_PI; // [rad/sec]
		if(i == 7 | i == 8 | i == 9)
			halSensor->valueList[i] = (HALFLOAT_T)concatenation * 0.3;
		k = k + 2;
  }

//	for(j=0;j<16;j++){
//		halSensor->valueList[j] = 1.234;//(0xff&buff[j]);
//		halSensor->valueList[j] = (0xff&buff[j]);
//		printf("halSensor->valueList[%d]=%f\n", j, halSensor->valueList[j]);
//}
  fflush(stdout);

	return HAL_OK;
}

static HALRETURNCODE_T fncGetTmValLst(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) { //uint32_t *pOutSize,HALFLOAT_T *pOutValLst,int32_t *pOutTime) {
	time_t timeWk;
	HALSENSOR_T *halSensor = (HALSENSOR_T *)pHalComponent;
	int32_t i,j,k;

	i=read(fd,buff,28);
	pCmd->num = i;
	for(j=0;j<i;j++){
		if( (0xff&buff[j]) == 0xff && (0xff&buff[j+1]) == 0xff && (0xff&buff[j+2]) == 0x52 && (0xff&buff[j+3]) == 0x54) {
			k = j;
			break;
		}
  }
	int32_t concatenation;
	uint8_t data_H;
	uint8_t data_L;

	for(i=0;i<10;i++) {
		data_L = (0xff&buff[k+8]);
		data_H = (0xff&buff[k+9]);
		concatenation = data_L + (data_H<<8);
		if(concatenation >= 32767)
			concatenation = concatenation - 65535;
		if(i == 0)
//			halSensor->valueList[i] = (HALFLOAT_T)(concatenation - acc_ref_x) / 2048; // [g]
			halSensor->valueList[i] = (HALFLOAT_T)(concatenation - acc_ref_x) / 2048 / 9.8; // [m/s^2]
		if(i == 1)
//			halSensor->valueList[i] = (HALFLOAT_T)(concatenation - acc_ref_y) / 2048; // [g]
			halSensor->valueList[i] = (HALFLOAT_T)(concatenation - acc_ref_y) / 2048 / 9.8; // [m/s^2]
		if(i == 2)
//			halSensor->valueList[i] = (HALFLOAT_T)((concatenation) - (acc_ref_z - 1)) / 2048; // [g]
			halSensor->valueList[i] = (HALFLOAT_T)((concatenation) - (acc_ref_z - 1)) / 2048 / 9.8; // [m/s^2]
		if(i == 3)
//			halSensor->valueList[i] = (HALFLOAT_T)concatenation / 340 + 35; // [degrees Celsius]
			halSensor->valueList[i] = (HALFLOAT_T)concatenation / 340 + 35 + 273.15; // [Kelbin]
//			halSensor->valueList[i] = ((HALFLOAT_T)concatenation / 340 + 35) * 1.8 + 32; // [degrees Fahrenheit]
		if(i == 4)
//			halSensor->valueList[i] = (HALFLOAT_T)(concatenation - omega_ref_x) / 16.4; // [deg/sec]
			halSensor->valueList[i] = (HALFLOAT_T)(concatenation - omega_ref_x) / 16.4 / 180 * M_PI; // [rad/sec]
		if(i == 5)
//			halSensor->valueList[i] = (HALFLOAT_T)(concatenation - omega_ref_y) / 16.4; // [deg/sec]
			halSensor->valueList[i] = (HALFLOAT_T)(concatenation - omega_ref_y) / 16.4 / 180 * M_PI; // [rad/sec]
		if(i == 6)
//			halSensor->valueList[i] = (HALFLOAT_T)(concatenation - omega_ref_z) / 16.4; // [deg/sec]
			halSensor->valueList[i] = (HALFLOAT_T)(concatenation - omega_ref_z) / 16.4 / 180 * M_PI; // [rad/sec]
		if(i == 7 | i == 8 | i == 9)
			halSensor->valueList[i] = (HALFLOAT_T)concatenation * 0.3;
		k = k + 2;
  }

//	halSensor->valueList[0] = atan2(simSensor1_y,simSensor1_x);
	timeWk = time(&timeWk);
	halSensor->time = (int32_t)timeWk - timeOrg;

	fflush(stdout);

	return HAL_OK;
}

static HALRETURNCODE_T fncNop(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) {
	return HAL_ERROR;
}

static HALRETURNCODE_T fncDeviceVensorSpec(HALCOMPONENT_T* pHalComponent,HAL_ARGUMENT_T *pCmd,HAL_ARGUMENT_DEVICE_T *pCmdDev) {
	return HAL_ERROR;
}

HAL_FNCTBL_T HalSensorRTUSB9AXISTbl = {
		fncInit3,
		fncReInit,
		fncFinalize,
		fncAddObserver,
		fncRemoveObserver,
		fncGetProperty,
		fncHalGetTime,
		fncNop,
		fncNop,fncNop,fncNop,fncNop,fncNop,fncNop,fncNop,fncNop,
		fncNop,fncNop,fncNop,fncNop,fncNop,fncNop,fncNop,fncNop,
		fncSetVal,fncGetVal,fncGetValLst,fncGetTmValLst,
		fncDeviceVensorSpec,fncDeviceVensorSpec,fncDeviceVensorSpec,fncDeviceVensorSpec
};


void simSensor1_setX(HALFLOAT_T x) {
	simSensor1_x = x-10.0;
}
void simSensor1_setY(HALFLOAT_T y) {
	simSensor1_y = y;
}
