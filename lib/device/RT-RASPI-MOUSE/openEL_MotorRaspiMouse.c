/*
 * openEL_MotorRaspiMouse.c
 *
 *  Created on: 2025/03/30
 *      Author: OpenEL-WG
 */

#ifdef __cplusplus
namespace el {
extern "C" {
#endif /* __cplusplus */
/*
#define OPENEL_SW_SURFACE_FRIEND 0
*/
#include <stdio.h>
#include "openEL.h"
#include "openEL_registry.h"

#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>

#ifndef M_PI
#define M_PI           3.14159265358979323846  /* pi */
#endif

static int32_t once = 1;
static int motoren;
static int motor_l;
static int motor_r;
static int motor;

static const char strName[] = "MOTOR_RT_RASPI_MOUSE";
static const char *strFncLst[] = {
	"HalInit",
	"HalReInit",
	"HalFinalize",
	"HalAddObserver",
	"HalRemoveObserver",
	"HalGetProperty",
	"HalGetTime",
	"HalActuatorSetValue",
	"HalActuatorGetValue"
};

static const HALPROPERTY_T motor_rt_raspi_mouse_property = {
	(char *)strName,
	(char **)strFncLst,
	sizeof(strFncLst)/sizeof(char *)
};

typedef struct simMot_st {
	HALCOMPONENT_T *hC;
	HALOBSERVER_T *obs;
	double posCmd,posSen,velCmd,velSen;
	HALFLOAT_T valueList[16];
	int32_t numObs;
	int32_t errCode;
	uint8_t inPos;
} SIM_MOT_T;
SIM_MOT_T simMotAr[16];

#define MAX_AXIS	4

static double posCmdAr[MAX_AXIS],posSenAr[MAX_AXIS];
static double velCmdAr[MAX_AXIS],velSenAr[MAX_AXIS];
static HALFLOAT_T valueList[MAX_AXIS][16];

#define DEBUG 0

static HALRETURNCODE_T fncInit(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) {
#if DEBUG
	printf("%s:%s HAL-ID %d %d %d %d\n", __FILE__, __FUNCTION__, 
			pHalComponent->halId.deviceKindId,
			pHalComponent->halId.vendorId,
			pHalComponent->halId.productId,
			pHalComponent->halId.instanceId );
#endif
	((HALACTUATOR_T *)pHalComponent)->valueList = valueList[pHalComponent->halId.instanceId];

	if (once) {
		motoren = open("/dev/rtmotoren0", O_WRONLY);
		motor_l = open("/dev/rtmotor_raw_l0", O_WRONLY);
		motor_r = open("/dev/rtmotor_raw_r0", O_WRONLY);
		motor = open("/dev/rtmotor0", O_WRONLY);
	
		printf("Motor On\n");
		write(motoren, "1", 1);
		usleep(500 * 1000);
		write(motor_l, "0", 5);
		write(motor_r, "0", 5);
		usleep(500 * 1000);

		once = 0;
	}

	return HAL_OK;
}

static HALRETURNCODE_T fncReInit(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) {
#if DEBUG
	printf("%s:%s\n", __FILE__, __FUNCTION__);
#endif
	return HAL_OK;
}

static HALRETURNCODE_T fncFinalize(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) {
#if DEBUG
	printf("%s:%s\n", __FILE__, __FUNCTION__);
#endif
	if (once == 0) {
		printf("Motor Off\n");
		write(motoren, "0", 1);
		close(motoren);
		close(motor_l);
		close(motor_r);
		close(motor);
		once = 1;
	}
	return HAL_OK;
}

static HALRETURNCODE_T fncAddObserver(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) {
	int32_t idx = pHalComponent->halId.instanceId;
	SIM_MOT_T *simMot = &simMotAr[idx];

	simMot->numObs++;
	simMot->obs = pHalComponent->observerList;
	return HAL_OK;
}

static HALRETURNCODE_T fncRemoveObserver(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) {
	int32_t idx = pHalComponent->halId.instanceId;
	SIM_MOT_T *simMot = &simMotAr[idx];

	simMot->numObs--;
	simMot->obs = pHalComponent->observerList;
	return HAL_OK;
}

static HALRETURNCODE_T fncGetProperty(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) {
	pHalComponent->property = (HALPROPERTY_T *)&motor_rt_raspi_mouse_property;
	return HAL_OK;
}

static HALRETURNCODE_T fncHalGetTime(HALCOMPONENT_T *halComponent,HAL_ARGUMENT_T *pCmd) {
	return HAL_ERROR;
}

static HALRETURNCODE_T fncSetVal(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) { //uint32_t id,HALFLOAT_T val) {
#if DEBUG
	printf("%s:%s\n", __FILE__, __FUNCTION__);
#endif
	HALRETURNCODE_T retCode = HAL_ERROR;
	int32_t idx = pHalComponent->halId.instanceId;
	HALFLOAT_T velocity = 0;
	int32_t pulseLen = 0;

	switch ( pCmd->FI.num ) {
	default:
		retCode = HAL_ERROR;
		break;
	case HAL_REQUEST_POSITION_CONTROL:
#if DEBUG
		printf("%s:%s:pCmd->FI.value=%f\n", __FILE__, __FUNCTION__, pCmd->FI.value);
#endif
		posCmdAr[idx] = pCmd->FI.value;
		int value = (int)posCmdAr[idx];
		if (idx == 1) { // Left Motor
			write(motor_l, (const void *)value, 5);
		} else if (idx == 2) { // Right Motor
			write(motor_r, (const void *)value, 5);
		} else {
			printf("%s:instanceId must be 1 or 2!\n", __FUNCTION__);
		}
		retCode = HAL_OK;
		break;
	case HAL_REQUEST_VELOCITY_CONTROL:
#if DEBUG
		printf("%s:%s:pCmd->FI.value=%f\n", __FILE__, __FUNCTION__, pCmd->FI.value);
#endif
		velSenAr[idx] = velCmdAr[idx];
		velCmdAr[idx] = pCmd->FI.value; // Between -20 * M_PI and 20 * M_PI
		char str[5];
		if (velCmdAr[idx]>0){
			velocity = velCmdAr[idx];
//			pulseLen = (int32_t) (velocity / (2 * M_PI)) * 400;
//			printf("pulseLen = %d\n", pulseLen);
//			printf("size = %d\n", sizeof(pulseLen));

			if (velocity < 1 * M_PI){
				pulseLen = 0;
			}
			if (velocity >= 1 * M_PI && velocity < 50 * M_PI ){
				pulseLen =  (velocity / (2 * M_PI)) * 400;
			}
			if (velocity >= 20 * M_PI){
				pulseLen = 10000;
			}

		} else {
			velocity = -velCmdAr[idx];
//			pulseLen =  (int32_t) -(velocity / (2 * M_PI)) * 400;

			if (velocity < 1 * M_PI){
				pulseLen = 0;
			}
			if (velocity >= 1 * M_PI && velocity < 50 * M_PI ){
				pulseLen =  -(velocity / (2 * M_PI)) * 400;
			}
			if (velocity >= 20 * M_PI){
				pulseLen = -10000;
			}

		}
		
		sprintf(str, "%d", pulseLen);
		printf("str = %s\n", str);
//		printf("size = %d\n", sizeof(str));

		if (idx == 1) { // Left Motor
			write(motor_l, str, sizeof(pulseLen));
//			write(motor_l, (const void *)pulseLen, sizeof(pulseLen));
		} else if (idx == 2) { // Right Motor
			write(motor_r, str, sizeof(pulseLen));
//			write(motor_r, (const void *)pulseLen, sizeof(pulseLen));
		} else {
			printf("%s:instanceId must be 1 or 2!\n", __FUNCTION__);
		}
		retCode = HAL_OK;
		break;
	case HAL_REQUEST_TORQUE_CONTROL:
		retCode = HAL_ERROR;
		break;
	}
	return retCode;
}

static HALRETURNCODE_T fncGetVal(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) { //uint32_t id,HALFLOAT_T *pOutVal) {
#if DEBUG
	printf("%s:%s\n", __FILE__, __FUNCTION__);
#endif
	HALRETURNCODE_T retCode = HAL_ERROR;
	int32_t idx = pHalComponent->halId.instanceId;
	int32_t buf;
	if (idx < 1)
		return HAL_ERROR;

	switch ( pCmd->FI.num ) {
	default:
		break;
	case HAL_REQUEST_POSITION_COMMAND:
		pCmd->FI.value = posCmdAr[idx];
#if DEBUG
		printf("%s:%f\n", __FUNCTION__, posCmdAr[idx]);
#endif
		retCode = HAL_OK;
		break;
	case HAL_REQUEST_POSITION_ACTUAL:
		posSenAr[idx] = (double)buf;
		pCmd->FI.value = posSenAr[idx];
#if DEBUG
		printf("%s:%f\n", __FUNCTION__, posSenAr[idx]);
#endif
		retCode = HAL_OK;
		break;
	case HAL_REQUEST_VELOCITY_COMMAND:
		pCmd->FI.value = velCmdAr[idx];
#if DEBUG
		printf("%s:%f\n", __FUNCTION__, velCmdAr[idx]);
#endif
		retCode = HAL_OK;
		break;
	case HAL_REQUEST_VELOCITY_ACTUAL:
		velSenAr[idx] = (double)buf;
		pCmd->FI.value = velSenAr[idx];
#if DEBUG
		printf("%s:%f\n", __FUNCTION__, velSenAr[idx]);
#endif
		retCode = HAL_OK;
		break;
	case HAL_REQUEST_TORQUE_COMMAND:
		pCmd->FI.value = posCmdAr[idx];
#if DEBUG
		printf("%s:%f\n", __FUNCTION__, posCmdAr[idx]);
#endif
		retCode = HAL_OK;
		break;
	case HAL_REQUEST_TORQUE_ACTUAL:
		pCmd->FI.value = posSenAr[idx];
#if DEBUG
		printf("%s:%f\n", __FUNCTION__, posSenAr[idx]);
#endif
		retCode = HAL_OK;
		break;
	}
	return retCode;
}

/** センサー用API , エラー返信 */
static HALRETURNCODE_T fncGetValLst(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) { //uint32_t *pOutSize,HALFLOAT_T *pOutValLst) {
	return HAL_ERROR;
}

/** センサー用API , エラー返信 */
static HALRETURNCODE_T fncGetTmValLst(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) { //uint32_t *pOutSize,HALFLOAT_T *pOutValLst,int32_t *pOutTime) {
	return HAL_ERROR;
}

static HALRETURNCODE_T fncNop(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) {
	return HAL_ERROR;
}

static HALRETURNCODE_T fncDeviceVensorSpec(HALCOMPONENT_T* pHalComponent,HAL_ARGUMENT_T *pCmd,HAL_ARGUMENT_DEVICE_T *pCmdDev) {
	return HAL_ERROR;
}

HAL_FNCTBL_T HalMotorRaspiMouseTbl = {
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

#ifdef __cplusplus
} /* extern "C" */
} /* namespace el */
#endif /* __cplusplus */

