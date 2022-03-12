/*
 * openEL_MotorPCA9685.c
 *
 *  Created on: 2021/12/24
 *      Author: OpenEL-WG
 */

/*
#define OPENEL_SW_SURFACE_FRIEND 0
*/
#include <stdio.h>
#include "openEL.h"
#include "openEL_registry.h"
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>

#ifndef M_PI
#define M_PI           3.14159265358979323846  /* pi */
#endif

#define PCA9685_SUBADR1 0x2
#define PCA9685_SUBADR2 0x3
#define PCA9685_SUBADR3 0x4

#define PCA9685_MODE1 0x0
#define PCA9685_PRESCALE 0xFE

#define LED0_ON_L 0x6
#define LED0_ON_H 0x7
#define LED0_OFF_L 0x8
#define LED0_OFF_H 0x9

#define ALLLED_ON_L 0xFA
#define ALLLED_ON_H 0xFB
#define ALLLED_OFF_L 0xFC
#define ALLLED_OFF_H 0xFD

#define PWM_FREQUENCY 1526                    //Min:24Hz, Max:1526Hz

static void PCA9685_init(float freq);
static uint8_t PCA9685_read(uint8_t adr);
static void PCA9685_write(uint8_t adr, uint8_t dat);
static void PCA9685_pwmWrite(uint8_t ch, double pulseWidth_usec);
static void PCA9685_setPWM(uint8_t ch, uint16_t onTime, uint16_t offTime);

static int32_t i2c = -1;

static const char strName[] = "MOTOR_PCA9685";
static const char *strFncLst[] = {
	"HalInit",
	"HalReInit",
	"HalFinalize",
	"HalGetProperty",
	"HalActuatorSetValue",
	"HalActuatorGetValue"
};
static const HALPROPERTY_T mot1_property = {
	(char *)strName,
	(char **)strFncLst,
	sizeof(strFncLst)/sizeof(char *)
};

#define MAX_AXIS	2
#define NUMBER_OF_VALUE 16

static double velCmdAr[MAX_AXIS+1],velSenAr[MAX_AXIS+1];
static HALFLOAT_T valueList[MAX_AXIS+1][NUMBER_OF_VALUE];

static HALRETURNCODE_T fncInit(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) {
#ifdef DEBUG
	printf("%s:%s:HAL-ID:0x%x 0x%x 0x%x 0x%x\n", __FILE__, __FUNCTION__,
			pHalComponent->halId.deviceKindId,
			pHalComponent->halId.vendorId,
			pHalComponent->halId.productId,
			pHalComponent->halId.instanceId );
#endif
	if (pHalComponent->halId.instanceId < 1 || pHalComponent->halId.instanceId > MAX_AXIS) {
		printf("instanceId should be 1 or 2.");
		return HAL_ERROR;
	}

	((HALACTUATOR_T *)pHalComponent)->valueList = valueList[pHalComponent->halId.instanceId];

	if (i2c == -1) {
		char i2cFileName[] = "/dev/i2c-1";
		int driverAddress = 0x60;

		if((i2c = open(i2cFileName, O_RDWR)) < 0){
			printf("I2C open err\n");
			return HAL_ERROR;
		}

		if(ioctl(i2c, I2C_SLAVE, driverAddress) < 0){
			printf("ioctl err\n");
			close(i2c);
			i2c = -1;
			return HAL_ERROR;
		}

		PCA9685_init(PWM_FREQUENCY);
	}

	return HAL_OK;
}

static HALRETURNCODE_T fncReInit(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) {
#ifdef DEBUG
	printf("%s:%s\n", __FILE__, __FUNCTION__);
#endif
	return HAL_OK;
}

static HALRETURNCODE_T fncFinalize(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) {
#ifdef DEBUG
	printf("%s:%s\n", __FILE__, __FUNCTION__);
#endif
	if (i2c != -1) {
		close(i2c);
		i2c = -1;
	}
	return HAL_OK;
}

static HALRETURNCODE_T fncAddObserver(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) {
	printf("%s:%s:HalAddObserver is not supported.\n", __FILE__, __FUNCTION__);
	return HAL_ERROR;
}

static HALRETURNCODE_T fncRemoveObserver(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) {
	printf("%s:%s:HalRemoveObserver is not supported.\n", __FILE__, __FUNCTION__);
	return HAL_ERROR;
}

static HALRETURNCODE_T fncGetProperty(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) {
#ifdef DEBUG
	printf("%s:%s\n", __FILE__, __FUNCTION__);
#endif
	pHalComponent->property = (HALPROPERTY_T *)&mot1_property;
	return HAL_OK;
}

static HALRETURNCODE_T fncHalGetTime(HALCOMPONENT_T *halComponent,HAL_ARGUMENT_T *pCmd) {
	printf("%s:%s:HalGetTime is not supported.\n", __FILE__, __FUNCTION__);
	return HAL_ERROR;
}

static HALRETURNCODE_T fncSetVal(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) {
#ifdef DEBUG
	printf("%s:%s\n", __FILE__, __FUNCTION__);
#endif
	HALRETURNCODE_T retCode = HAL_ERROR;
	int32_t idx = pHalComponent->halId.instanceId;

	HALFLOAT_T velocity = 0;
	uint16_t pulseLen = 0;

	switch ( pCmd->FI.num ) {
	default:
		break;
	case HAL_REQUEST_NO_EXCITE:
		velSenAr[idx] = 0;
		velCmdAr[idx] = 0;
		if (idx == 1) { // Left Motor
			PCA9685_setPWM(10, 0, 4096);
			PCA9685_setPWM(9, 0, 4096);
			PCA9685_setPWM(8, 0, 4096);
		} else if (idx == 2) { // Right Motor
			PCA9685_setPWM(13, 0, 4096);
			PCA9685_setPWM(12, 0, 4096);
			PCA9685_setPWM(11, 0, 4096);
		} else {
			printf("%s:instanceId must be 1 or 2!\n", __FUNCTION__);
		}
		retCode = HAL_OK;
		break;
	case HAL_REQUEST_POSITION_CONTROL:
		printf("%s:HAL_REQUEST_POSITION_CONTROL is not supported.\n", __FUNCTION__);
		retCode = HAL_ERROR;
		break;
	case HAL_REQUEST_VELOCITY_CONTROL:
		velSenAr[idx] = velCmdAr[idx];
		velCmdAr[idx] = pCmd->FI.value; // Between -15 * M_PI and 15 * M_PI
		if (velCmdAr[idx]>0){
			velocity = velCmdAr[idx];
		} else {
			velocity = -velCmdAr[idx];
		}

#ifdef DEBUG
		printf("%s:velSenAr[%d]=%f\n", __FUNCTION__, idx, velSenAr[idx]);
		printf("%s:velCmdAr[%d]=%f\n", __FUNCTION__, idx, velCmdAr[idx]);
#endif
		if (velocity < 1 * M_PI){
			pulseLen = 0;
		}
		if (velocity >= 1 * M_PI && velocity < 2 * M_PI ){
			pulseLen = (uint16_t)(86 * (velocity - (1 * M_PI)) + 330);
		}
		if (velocity >= 2 * M_PI && velocity < 4 * M_PI ){
			pulseLen = (uint16_t)(68 * (velocity - (2 * M_PI)) + 600);
		}
		if (velocity >= 4 * M_PI && velocity < 8 * M_PI ){
			pulseLen = (uint16_t)(61 * (velocity - (4 * M_PI)) + 1030);
		}
		if (velocity >= 8 * M_PI && velocity < 15 * M_PI ){
			pulseLen = (uint16_t)(104 * (velocity - (8 * M_PI)) + 1800);
		}
		if (velocity >= 15 * M_PI){
			pulseLen = 4095;
		}

		if (idx == 1) { // Left Motor
			if (velCmdAr[idx]>0) {
				PCA9685_setPWM(10, 0, 4096);
				PCA9685_setPWM(9, 4096, 0); //Forward
			} else if (velCmdAr[idx]<0){
				PCA9685_setPWM(10, 4096, 0); //Backward
				PCA9685_setPWM(9, 0, 4096);
			} else {
				PCA9685_setPWM(10, 0, 0); //Release
				PCA9685_setPWM(9, 0, 0);
			}
			PCA9685_setPWM(8, 0, pulseLen);
		} else if (idx == 2) { // Right Motor
			if (velCmdAr[idx]>0) {
				PCA9685_setPWM(11, 0, 4096);
				PCA9685_setPWM(12, 4096, 0); //Forward
			} else if (velCmdAr[idx]<0){
				PCA9685_setPWM(11, 4096, 0); //Backward
				PCA9685_setPWM(12, 0, 4096);
			} else {
				PCA9685_setPWM(13, 0, 0); //Release
				PCA9685_setPWM(12, 0, 0);
			}
			PCA9685_setPWM(13, 0, pulseLen);
		} else {
			printf("%s:instanceId must be 1 or 2!\n", __FUNCTION__);
		}
		retCode = HAL_OK;
		break;
	case HAL_REQUEST_TORQUE_CONTROL:
		printf("%s:HAL_REQUEST_TORQUE_CONTROL is not supported.\n", __FUNCTION__);
		retCode = HAL_ERROR;
		break;
	}
	return retCode;
}

static HALRETURNCODE_T fncGetVal(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) { //uint32_t id,HALFLOAT_T *pOutVal) {
#ifdef DEBUG
	printf("%s:%s\n", __FILE__, __FUNCTION__);
#endif
	HALRETURNCODE_T retCode = HAL_ERROR;
	int32_t idx = pHalComponent->halId.instanceId;

	switch ( pCmd->FI.num ) {
	default:
		break;
	case HAL_REQUEST_POSITION_COMMAND:
		pCmd->FI.value = 0;
		printf("%s:HAL_REQUEST_POSITION_COMMAND is not supported.\n", __FUNCTION__);
		retCode = HAL_ERROR;
		break;
	case HAL_REQUEST_POSITION_ACTUAL:
		pCmd->FI.value = 0;
		printf("%s:HAL_REQUEST_POSITION_ACTUAL is not supported.\n", __FUNCTION__);
		retCode = HAL_ERROR;
		break;
	case HAL_REQUEST_VELOCITY_COMMAND:
		pCmd->FI.value = velSenAr[idx];
#ifdef DEBUG
		printf("%s:%f\n", __FUNCTION__, posSenAr[idx]);
#endif
		retCode = HAL_OK;
		break;
	case HAL_REQUEST_VELOCITY_ACTUAL:
		pCmd->FI.value = velSenAr[idx];
#ifdef DEBUG
		printf("%s:%f\n", __FUNCTION__, posSenAr[idx]);
#endif
		retCode = HAL_OK;
		break;
	}
	return retCode;
}

static HALRETURNCODE_T fncGetValLst(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) { //uint32_t *pOutSize,HALFLOAT_T *pOutValLst) {
	printf("%s:%s:HalGetValueList is not supported.\n", __FILE__, __FUNCTION__);
	return HAL_ERROR;
}

static HALRETURNCODE_T fncGetTmValLst(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) { //uint32_t *pOutSize,HALFLOAT_T *pOutValLst,int32_t *pOutTime) {
	printf("%s:%s:HalGetTimedValueList is not supported.\n", __FILE__, __FUNCTION__);
	return HAL_ERROR;
}

static HALRETURNCODE_T fncNop(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) {
	return HAL_ERROR;
}

static HALRETURNCODE_T fncDeviceVensorSpec(HALCOMPONENT_T* pHalComponent,HAL_ARGUMENT_T *pCmd,HAL_ARGUMENT_DEVICE_T *pCmdDev) {
	return HAL_ERROR;
}

HAL_FNCTBL_T HalMotorPCA9685Tbl = {
		fncInit,
		fncReInit,
		fncFinalize,
		fncAddObserver,
		fncRemoveObserver,
		fncGetProperty,
		fncHalGetTime,
		fncNop,
		fncNop,fncNop,fncNop,fncNop,fncNop,fncNop,fncNop,fncNop,
		fncNop,fncNop,fncNop,fncNop,fncNop,fncNop,fncNop,fncNop,
		fncSetVal,
		fncGetVal,
		fncGetValLst,
		fncGetTmValLst,
		fncDeviceVensorSpec,
		fncDeviceVensorSpec,
		fncDeviceVensorSpec,
		fncDeviceVensorSpec
};

static void PCA9685_init(float freq)
{
    PCA9685_write(PCA9685_MODE1, 0x0);
    usleep(100000);//100ms
    uint8_t prescale = 0x03; // 0x03:1526 Hz, 0xFF:24 Hz
    uint8_t oldmode = PCA9685_read(PCA9685_MODE1);
    uint8_t newmode = (oldmode&0x7F) | 0x10;
    PCA9685_write(PCA9685_MODE1, newmode);
    PCA9685_write(PCA9685_PRESCALE, prescale);
    PCA9685_write(PCA9685_MODE1, oldmode);
    sleep(1);
    PCA9685_write(PCA9685_MODE1, oldmode | 0xa1);
}

static void PCA9685_pwmWrite(uint8_t ch, double pulseWidth_usec)
{
    double pulselength;
    double pulseWidth;

    pulselength = 1000000 / PWM_FREQUENCY;
    if(pulseWidth_usec > pulselength || pulseWidth_usec < 0){
        printf("PCA9685_pwmWrite() err : pulseWidth_usec %f is invalid.\n", pulseWidth_usec);
        return;
    }
    pulselength /= 4096;
    pulseWidth = pulseWidth_usec / pulselength;
    PCA9685_setPWM(ch, 0, (uint16_t)pulseWidth);
}

static void PCA9685_setPWM(uint8_t ch, uint16_t onTime, uint16_t offTime)
{
    uint8_t sendData[5];
	if (ch > 15){
        printf("PCA9685_setPWM() err : ch %d is invalid.\n", ch);
		return;
	}
    sendData[0] = LED0_ON_L + 4 * ch;
    sendData[1] = (uint8_t)(0x00ff & onTime);
    sendData[2] = (uint8_t)((0xff00 & onTime) >> 8);
    sendData[3] = (uint8_t)(0x00ff & offTime);
    sendData[4] = (uint8_t)((0xff00 & offTime) >> 8);

    if(write(i2c, sendData, 5) != 5){
        printf("PCA9685_setPWM() err\n");
    }
}

static uint8_t PCA9685_read(uint8_t adr)
{
    uint8_t sendData;
    uint8_t readData;
	int32_t ret;

    sendData = adr;
    ret = write(i2c, &sendData, 1);
    if(ret == -1){
        perror("PCA9685_read() err ");
		return ret;
	} else if(ret != 1){
        printf("PCA9685_read() err1\n");
		return ret;
    }
    ret = read(i2c, &readData, 1);
    if( ret == -1){
        perror("PCA9685_read() err ");
		return ret;
	} else if (ret != 1){
        printf("PCA9685_read() err2\n");
		return ret;
    }

    return readData;
}

static void PCA9685_write(uint8_t adr, uint8_t dat)
{
    uint8_t buf[2];
	int32_t ret;

    buf[0] = adr;
    buf[1] = dat;
    ret = write(i2c, buf, 2);
    if( ret == -1){
        perror("PCA9685_write() err ");
		return;
	} else if(ret != 2){
        printf("PCA9685_write() err\n");
    }
}
