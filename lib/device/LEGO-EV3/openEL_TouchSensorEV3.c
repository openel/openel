/*
 * openEL_TouchSensorEV3.c
 *
 *  Created on: 2021/08/21
 *      Author: OpenEL-WG
 */

#include <HAL4RT.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

#include "openEL.h"
#include "openEL_registry.h"
#include "openEL_TouchSensorEV3.h"

#include "ev3.h"
#include "ev3_port.h"
#include "ev3_sensor.h"

static int32_t once = 1;
static int32_t n,i,j,val;
static char s[256];
static const char strName[] = "TOUCHSENSOR_EV3";
static uint8_t sn_touch;

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
static const HALPROPERTY_T touchsensor_ev3_property = {
	(char *)strName,
	(char **)strFncLst,
	sizeof(strFncLst)/sizeof(char *)
};

static int32_t timeOrg;
static HALFLOAT_T sensorValueAr[16];

static HALRETURNCODE_T fncInit(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) {
#if DEBUG
	printf("%s:%s HAL-ID %d %d %d %d\n", __FILE__, __FUNCTION__
			pHalComponent->halId.deviceKindId,
			pHalComponent->halId.vendorId,
			pHalComponent->halId.productId,
			pHalComponent->halId.instanceId );
#endif
	time_t timeWk;
	timeWk = time(&timeWk);
	timeOrg = (int32_t)timeWk;
	((HALSENSOR_T *)pHalComponent)->valueList = sensorValueAr;

	if (once) {
		if ( ev3_init() == -1 )
			return HAL_ERROR;
		ev3_sensor_init();
		for ( i = 0; i < DESC_LIMIT; i++ ) {
			if ( ev3_sensor[ i ].type_inx != SENSOR_TYPE__NONE_ ) {
				printf( "  type = %s\n", ev3_sensor_type( ev3_sensor[ i ].type_inx ));
				printf( "  port = %s\n", ev3_sensor_port_name( i, s ));
				if ( get_sensor_mode( i, s, sizeof( s ))) {
					printf( "  mode = %s\n", s );
				}
				if ( get_sensor_num_values( i, &n )) {
					for ( j = 0; j < n; j++ ) {
						if ( get_sensor_value( j, i, &val )) {
							printf( "  value%d = %d\n", j, val );
						}
					}
				}
			}
		}
		once = 0;
	}

	if ( ev3_search_sensor( LEGO_EV3_TOUCH, &sn_touch, 0 )) {
		printf( "TOUCH sensor is found.\n" );
	} else {
		printf( "TOUCH sensor is NOT found.\n" );
	}

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
	pHalComponent->property = (HALPROPERTY_T *)&touchsensor_ev3_property;
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

static HALRETURNCODE_T fncSetVal(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) {
	return HAL_ERROR;
}

static HALRETURNCODE_T fncGetVal(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) {
	return HAL_ERROR;
}

static HALRETURNCODE_T fncGetValLst(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) { //uint32_t *pOutSize,HALFLOAT_T *pOutValLst) {
	//printf( "%s:%d:%s\n", __FILE__, __LINE__, __func__ );
	HALSENSOR_T *halSensor = (HALSENSOR_T *)pHalComponent;
	int32_t i=sn_touch,j,n;
	if ( get_sensor_mode( i, s, sizeof( s ))) {
		//printf( "  mode = %s\n", s );
	}
	if ( get_sensor_num_values( i, &n )) {
		for ( j = 0; j < n; j++ ) {
			if ( get_sensor_value( j, i, &val )) {
				//printf( "  value%d = %d\n", 0, val );
				halSensor->valueList[j] = val; 
			}
		}
	}
	pCmd->num = n;

	return HAL_OK;
}

static HALRETURNCODE_T fncGetTmValLst(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) { //uint32_t *pOutSize,HALFLOAT_T *pOutValLst,int32_t *pOutTime) {
	//printf( "%s:%d:%s\n", __FILE__, __LINE__, __func__ );
	time_t timeWk;
	HALSENSOR_T *halSensor = (HALSENSOR_T *)pHalComponent;
	int32_t i=sn_touch,j,n;
	if ( get_sensor_mode( i, s, sizeof( s ))) {
		//printf( "  mode = %s\n", s );
	}
	if ( get_sensor_num_values( i, &n )) {
		for ( j = 0; j < n; j++ ) {
			if ( get_sensor_value( j, i, &val )) {
				//printf( "  value%d = %d\n", 0, val );
				halSensor->valueList[j] = val; 
			}
		}
	}
	pCmd->num = n;

	timeWk = time(&timeWk);
	halSensor->time = (int32_t)timeWk - timeOrg;
	return HAL_OK;
}

static HALRETURNCODE_T fncNop(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) {
	return HAL_ERROR;
}

static HALRETURNCODE_T fncDeviceVensorSpec(HALCOMPONENT_T* pHalComponent,HAL_ARGUMENT_T *pCmd,HAL_ARGUMENT_DEVICE_T *pCmdDev) {
	return HAL_ERROR;
}

HAL_FNCTBL_T HalTouchSensorEV3Tbl = {
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
		fncSetVal,fncGetVal,fncGetValLst,fncGetTmValLst,
		fncDeviceVensorSpec,fncDeviceVensorSpec,fncDeviceVensorSpec,fncDeviceVensorSpec
};

