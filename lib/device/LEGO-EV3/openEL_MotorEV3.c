/*
 * openEL_MotorEV3.c
 *
 *  Created on: 2021/04/17
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
/*
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
*/

#include "ev3.h"
#include "ev3_port.h"
#include "ev3_tacho.h"

static int32_t once = 1;
static int32_t i;
static uint8_t sn;
static FLAGS_T state;
static char s[256];

static const char strName[] = "MOTOR_LEGO_EV3";
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

static const HALPROPERTY_T motor_ev3_property = {
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
		if ( ev3_init() == -1 )
			return HAL_ERROR;
		while ( ev3_tacho_init() < 1)
			usleep( 1000 * 1000 );
		for ( i = 0; i < DESC_LIMIT; i++ ) {
			if ( ev3_tacho[ i ].type_inx != TACHO_TYPE__NONE_ ) {
				printf( "  type = %s\n", ev3_tacho_type( ev3_tacho[ i ].type_inx ));
				printf( "  port = %s\n", ev3_tacho_port_name( i, s ));
			}
		}
		once = 0;
	}
	if ( ev3_search_tacho( LEGO_EV3_L_MOTOR, &sn, (uint8_t)pHalComponent->halId.instanceId-1 ) == false ) {
		printf("LEGO_EV3_MOTOR was found and initialized. sn=%d\n", sn);
	} else if( ev3_search_tacho( LEGO_EV3_M_MOTOR, &sn, (uint8_t)pHalComponent->halId.instanceId-1 ) == false ) {
		printf("LEGO_EV3_MOTOR was found and initialized. sn=%d\n", sn);
	} else {
		return HAL_ERROR;
	}		
	set_tacho_command_inx( sn, TACHO_RESET );

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
		set_tacho_stop_action_inx( sn, TACHO_COAST );
		ev3_uninit();
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
	pHalComponent->property = (HALPROPERTY_T *)&motor_ev3_property;
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
	sn = idx-1;

	switch ( pCmd->FI.num ) {
	default:
		retCode = HAL_ERROR;
		break;
	case HAL_REQUEST_POSITION_CONTROL:
#if DEBUG
		printf("%s:%s:pCmd->FI.value=%f\n", __FILE__, __FUNCTION__, pCmd->FI.value);
#endif
		posCmdAr[idx] = pCmd->FI.value;
		set_tacho_stop_action_inx( sn, TACHO_HOLD );
		set_tacho_speed_sp( sn, 100 );
		set_tacho_ramp_up_sp( sn, 0 );
		set_tacho_ramp_down_sp( sn, 0 );
		set_tacho_position_sp( sn, posCmdAr[idx] );
		set_tacho_command_inx( sn, TACHO_RUN_TO_ABS_POS );
		retCode = HAL_OK;
		break;
	case HAL_REQUEST_VELOCITY_CONTROL:
#if DEBUG
		printf("%s:%s:pCmd->FI.value=%f\n", __FILE__, __FUNCTION__, pCmd->FI.value);
#endif
		velCmdAr[idx] = pCmd->FI.value;
		set_tacho_stop_action_inx( sn, TACHO_COAST );
		set_tacho_speed_sp( sn, velCmdAr[idx] );
		set_tacho_ramp_up_sp( sn, 0 );
		set_tacho_ramp_down_sp( sn, 0 );
		set_tacho_command_inx( sn, TACHO_RUN_FOREVER );
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
	sn = idx-1;

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
		get_tacho_position(sn, &buf);
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
		get_tacho_speed(sn, &buf);
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

HAL_FNCTBL_T HalMotorEV3Tbl = {
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

