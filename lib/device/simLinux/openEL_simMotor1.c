/*
 * openEL_simMotor1.c
 *
 *  Created on: 2018/05/19
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
#include "openEL_simMotor1.h"
#include "openEL_simSensor_mot.h"


static const char strName[] = "MOTOR_100a";
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
static const HALPROPERTY_T mot1_property = {
	(char *)strName,
	(char **)strFncLst,
	sizeof(strFncLst)/sizeof(char *)
};

#define MAX_AXIS	8

static double posCmdAr[MAX_AXIS],posSenAr[MAX_AXIS];
static HALFLOAT_T valueList[MAX_AXIS][16];


static HALRETURNCODE_T fncInit(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) {
	printf("HalInit Motor1 HAL-ID %d %d %d %d\n",
			pHalComponent->halId.deviceKindId,
			pHalComponent->halId.vendorId,
			pHalComponent->halId.productId,
			pHalComponent->halId.instanceId );
	((HALACTUATOR_T *)pHalComponent)->valueList = valueList[pHalComponent->halId.instanceId];
	return HAL_OK;
}

static HALRETURNCODE_T fncReInit(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) {
	return HAL_OK;
}

static HALRETURNCODE_T fncFinalize(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) {
	return HAL_OK;
}

static HALRETURNCODE_T fncAddObserver(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) {
	return HAL_OK;
}

static HALRETURNCODE_T fncRemoveObserver(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) {
	return HAL_OK;
}

static HALRETURNCODE_T fncGetProperty(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) {
	pHalComponent->property = (HALPROPERTY_T *)&mot1_property;
	return HAL_OK;
}

static HALRETURNCODE_T fncHalGetTime(HALCOMPONENT_T *halComponent,HAL_ARGUMENT_T *pCmd) {
	return HAL_ERROR;
}

static HALRETURNCODE_T fncSetVal(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) { //uint32_t id,HALFLOAT_T val) {
	HALRETURNCODE_T retCode = HAL_ERROR;
	int32_t idx = pHalComponent->halId.instanceId;

	switch ( pCmd->FI.num ) {
	default:
		break;
	case HAL_REQUEST_POSITION_CONTROL:
		posSenAr[idx] = posCmdAr[idx];
		posCmdAr[idx] = pCmd->FI.value;
		simSensor1_setX(posSenAr[idx]);
		retCode = HAL_OK;
		break;
	}
	return retCode;
}

static HALRETURNCODE_T fncGetVal(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) { //uint32_t id,HALFLOAT_T *pOutVal) {
	HALRETURNCODE_T retCode = HAL_ERROR;
	int32_t idx = pHalComponent->halId.instanceId;

	switch ( pCmd->FI.num ) {
	default:
		break;
	case HAL_REQUEST_POSITION_COMMAND:
		pCmd->FI.value = posSenAr[idx];
		retCode = HAL_OK;
		break;
	case HAL_REQUEST_POSITION_ACTUAL:
		pCmd->FI.value = posSenAr[idx];
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

HAL_FNCTBL_T HalMotor001Tbl = {
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
