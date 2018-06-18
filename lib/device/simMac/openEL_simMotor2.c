/*
 * openEL_simMotor2.c
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
#include "openEL_simMotor2.h"
#include "openEL_simSensor_mot.h"

static const char strName[] = "MOTOR_200";
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
static const HALPROPERTY_T mot2_property = {
	(char *)strName,
	(char **)strFncLst,
	sizeof(strFncLst)/sizeof(char *)
};

typedef struct simMot1_st {
	HALCOMPONENT_T *hC;
	HALOBSERVER_T *obs;
	double posCmd,pos1,posSen;
	HALFLOAT_T valueList[16];
	int32_t numObs;
	int32_t errCode;
	uint8_t inPos;
} SIM_MOT_T;
SIM_MOT_T simMotAr[16];

static HALRETURNCODE_T fncInit2(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) {
	int32_t idx = pHalComponent->halId.instanceId;
	printf("HalInit Motor2 HAL-ID %d %d %d %d\n",
			pHalComponent->halId.deviceKindId,
			pHalComponent->halId.vendorId,
			pHalComponent->halId.productId,
			pHalComponent->halId.instanceId );
	simMotAr[idx].hC = pHalComponent;
	((HALSENSOR_T *)pHalComponent)->valueList = simMotAr[idx].valueList;
	return HAL_OK;
}

static HALRETURNCODE_T fncReInit(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) {
	int32_t idx = pHalComponent->halId.instanceId;
	SIM_MOT_T *simMot = &simMotAr[idx];

	simMot->errCode = 0;
	return HAL_OK;
}

static HALRETURNCODE_T fncFinalize(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) {
	int32_t idx = pHalComponent->halId.instanceId;
	SIM_MOT_T *simMot = &simMotAr[idx];
	simMot->hC = 0;
	simMot->obs = 0;
	return HAL_OK;
}

static HALRETURNCODE_T fncAddObserver(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) { //HALOBSERVER_T *pObs) {
	int32_t idx = pHalComponent->halId.instanceId;
	SIM_MOT_T *simMot = &simMotAr[idx];

	simMot->numObs++;
	simMot->obs = pHalComponent->observerList;
	return HAL_OK;
}

static HALRETURNCODE_T fncRemoveObserver(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) { //HALOBSERVER_T *pObs) {
	int32_t idx = pHalComponent->halId.instanceId;
	SIM_MOT_T *simMot = &simMotAr[idx];

	simMot->numObs--;
	simMot->obs = pHalComponent->observerList;
	return HAL_OK;
}

void simMotExe(int32_t idx) {
	SIM_MOT_T *simMot = &simMotAr[idx];
	HALOBSERVER_T *obsWk;
	static int32_t cnt;
	uint8_t inPosWk;

	inPosWk = ( simMot->posSen == simMot->posCmd )? 1: 0;
	if ( 1==inPosWk && 0==simMot->inPos ) {
		obsWk = simMot->obs;
		while ( 0 != obsWk ) {
			obsWk->notify_event(simMot->hC,1);
			obsWk = HalLinkedList_getNext(obsWk);
		}
	}
	simMot->inPos = inPosWk;
	simMot->pos1 = simMot->posCmd;
	simMot->posSen = simMot->pos1;
	simSensor1_setY(simMot->posSen);

	/* Error */
	if ( 120 == (++cnt) ) {
		simMot->errCode = 200+idx;
		obsWk = simMot->obs;
		while ( 0 != obsWk ) {
			obsWk->notify_error(simMot->hC,simMot->errCode);
			obsWk = HalLinkedList_getNext(obsWk);
		}
	}
}

static HALRETURNCODE_T fncGetProperty(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) {
	pHalComponent->property = (HALPROPERTY_T *)&mot2_property;
	return HAL_OK;
}

static HALRETURNCODE_T fncHalGetTime(HALCOMPONENT_T *halComponent,HAL_ARGUMENT_T *pCmd) { // int32_t *pTime_value) {
	return HAL_ERROR;
}

static HALRETURNCODE_T fncSetVal(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) { //uint32_t id,HALFLOAT_T val) {
	HALRETURNCODE_T retCode = HAL_ERROR;
	int32_t idx = pHalComponent->halId.instanceId;

	switch ( pCmd->FI.num ) {
	default:
		break;
	case HAL_REQUEST_POSITIONE_CONTROL:
		simMotAr[idx].posCmd = pCmd->FI.value;
		retCode = HAL_OK;
		break;
	}
	simMotExe(idx);
	return retCode;
}

static HALRETURNCODE_T fncGetVal(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) { //uint32_t id,HALFLOAT_T *pOutVal) {
	HALRETURNCODE_T retCode = HAL_ERROR;
	int32_t idx = pHalComponent->halId.instanceId;

	switch ( pCmd->FI.num ) {
	default:
		break;
	case HAL_REQUEST_POSITIONE_COMMAND:
		pCmd->FI.value = simMotAr[idx].posCmd;
		retCode = HAL_OK;
		break;
	case HAL_REQUEST_POSITIONE_ACUTUAL:
		pCmd->FI.value = simMotAr[idx].posSen;
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

static HALRETURNCODE_T fncDeviceVensorSpec(HALCOMPONENT_T* pHalComponent,HAL_ARGUMENT_T *pCmd,HAL_ARGUMENT_DEVICE_T *pCndDev) {
	return HAL_ERROR;
}

HAL_FNCTBL_T HalMotor002Tbl = {
		fncInit2,
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
		fncDeviceVensorSpec,
		fncDeviceVensorSpec,
		fncDeviceVensorSpec,
		fncDeviceVensorSpec
};

#ifdef __cplusplus
} /* extern "C" */
} /* namespace el */
#endif /* __cplusplus */
