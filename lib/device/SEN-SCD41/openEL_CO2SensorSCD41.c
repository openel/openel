/*
 * openEL_CO2Sensorscd41.c
 *
 *  Created on: 2022/02/01
 *      Author: OpenEL-WG
 */

#include <HAL4RT.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#include "openEL.h"
#include "openEL_registry.h"
#include "openEL_CO2SensorSCD41.h"

static int32_t i2c;
static int32_t once = 1;

static const char strName[] = "CO2SENSOR_SCD41";
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

typedef struct simSen1_st {
	HALCOMPONENT_T *hC;
	HALOBSERVER_T *obs;
	HALFLOAT_T valueList[4];
	int32_t numObs;
	int32_t errCode;
} SIM_SEN_T;
static SIM_SEN_T simSenAr[16];

static int32_t timeOrg;
static uint16_t interval_in_seconds = 2;
static uint16_t co2_ppm;
static int32_t temperature;
static int32_t relative_humidity;

static HALRETURNCODE_T measure(int32_t idx);
void check_measured_value(int32_t idx);
void send_error(int32_t idx, int32_t error_code);

//#define DEBUG

static HALRETURNCODE_T fncInit(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) {
	time_t timeWk;
	timeWk = time(&timeWk);
	timeOrg = (int32_t)timeWk;

	int32_t idx = pHalComponent->halId.instanceId;
	simSenAr[idx].hC = pHalComponent;
	((HALSENSOR_T *)pHalComponent)->valueList = simSenAr[idx].valueList;
#ifdef DEBUG
	printf("%s:%s:HAL-ID:0x%x 0x%x 0x%x 0x%x\n", __FILE__, __FUNCTION__,
			pHalComponent->halId.deviceKindId,
			pHalComponent->halId.vendorId,
			pHalComponent->halId.productId,
			pHalComponent->halId.instanceId );
#endif

	if (once) {
		int16_t error = 0;
		sensirion_i2c_hal_init();
		scd4x_wake_up();
		scd4x_stop_periodic_measurement();
		scd4x_reinit();
		uint16_t serial_0;
		uint16_t serial_1;
		uint16_t serial_2;
		error = scd4x_get_serial_number(&serial_0, &serial_1, &serial_2);
		if (error) {
			printf("Error executing scd4x_get_serial_number(): %i\n", error);
		} else {
			printf("serial: 0x%04x%04x%04x\n", serial_0, serial_1, serial_2);
		}
		error = scd4x_start_periodic_measurement();
		if (error) {
			printf("Error executing scd4x_start_periodic_measurement(): %i\n", error);
		}
		printf("Waiting for first measurement... (5 sec)\n");
		once = 0;
	}

	return HAL_OK;
}

static HALRETURNCODE_T fncReInit(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) {
#ifdef DEBUG
	printf("%s:%s\n", __FILE__, __FUNCTION__);
#endif
	int32_t idx = pHalComponent->halId.instanceId;
	SIM_SEN_T *simSen = &simSenAr[idx];

	simSen->errCode = 0;
	return HAL_OK;
}

static HALRETURNCODE_T fncFinalize(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) {
#ifdef DEBUG
	printf("%s:%s\n", __FILE__, __FUNCTION__);
#endif
	int32_t idx = pHalComponent->halId.instanceId;
	SIM_SEN_T *simSen = &simSenAr[idx];

	simSen->hC = 0;
	simSen->obs = 0;

	if (once == 0) {
		scd4x_stop_periodic_measurement();
		sensirion_i2c_hal_free();
		once = 1;
	}
	return HAL_OK;
}

static HALRETURNCODE_T fncAddObserver(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) {
#ifdef DEBUG
	printf("%s:%s\n", __FILE__, __FUNCTION__);
#endif
	int32_t idx = pHalComponent->halId.instanceId;
	SIM_SEN_T *simSen = &simSenAr[idx];

	simSen->numObs++;
	simSen->obs = pHalComponent->observerList;

	return HAL_OK;
}

static HALRETURNCODE_T fncRemoveObserver(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) {
#ifdef DEBUG
	printf("%s:%s\n", __FILE__, __FUNCTION__);
#endif
	int32_t idx = pHalComponent->halId.instanceId;
	SIM_SEN_T *simSen = &simSenAr[idx];

	simSen->numObs--;
	simSen->obs = pHalComponent->observerList;

	return HAL_OK;
}

static HALRETURNCODE_T fncGetProperty(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) {
#ifdef DEBUG
	printf("%s:%s\n", __FILE__, __FUNCTION__);
#endif
	pHalComponent->property = (HALPROPERTY_T *)&sen1_property;
	return HAL_OK;
}

static HALRETURNCODE_T fncHalGetTime(HALCOMPONENT_T *halComponent,HAL_ARGUMENT_T *pCmd) {
#ifdef DEBUG
	printf("%s:%s\n", __FILE__, __FUNCTION__);
#endif
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
	printf("%s:%s:HalSetValue is not supported.\n", __FILE__, __FUNCTION__);
	return HAL_ERROR;
}

static HALRETURNCODE_T fncGetVal(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) {
	printf("%s:%s:HalGetValue is not supported.\n", __FILE__, __FUNCTION__);
	return HAL_ERROR;
}

static HALRETURNCODE_T fncGetValLst(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) { //uint32_t *pOutSize,HALFLOAT_T *pOutValLst) {
#ifdef DEBUG
	printf("%s:%s\n", __FILE__, __FUNCTION__);
#endif
	int32_t idx = pHalComponent->halId.instanceId;
	SIM_SEN_T *simSen = &simSenAr[idx];

	if(measure(idx)== HAL_ERROR)
		return HAL_ERROR;

	simSen->valueList[0] = (HALFLOAT_T)co2_ppm;
	simSen->valueList[1] = (HALFLOAT_T)temperature/1000;
	simSen->valueList[2] = (HALFLOAT_T)relative_humidity/1000;

	HALSENSOR_T *halSensor = (HALSENSOR_T *)pHalComponent;

	pCmd->num = 3;
	halSensor->valueList[0] = simSen->valueList[0];
	halSensor->valueList[1] = simSen->valueList[1];
	halSensor->valueList[2] = simSen->valueList[2];

	check_measured_value(idx);

	return HAL_OK;
}

static HALRETURNCODE_T fncGetTmValLst(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) { //uint32_t *pOutSize,HALFLOAT_T *pOutValLst,int32_t *pOutTime) {
#ifdef DEBUG
	printf("%s:%s\n", __FILE__, __FUNCTION__);
#endif
	time_t timeWk;
	int32_t idx = pHalComponent->halId.instanceId;
	SIM_SEN_T *simSen = &simSenAr[idx];

	if(measure(idx)== HAL_ERROR)
		return HAL_ERROR;

	simSen->valueList[0] = (HALFLOAT_T)co2_ppm;
	simSen->valueList[1] = (HALFLOAT_T)temperature;
	simSen->valueList[2] = (HALFLOAT_T)relative_humidity;

	HALSENSOR_T *halSensor = (HALSENSOR_T *)pHalComponent;

	pCmd->num = 4;
	halSensor->valueList[0] = simSen->valueList[0];
	halSensor->valueList[1] = simSen->valueList[1];
	halSensor->valueList[2] = simSen->valueList[2];
	halSensor->valueList[3] = simSen->valueList[3];

	timeWk = time(&timeWk);
	halSensor->time = (int32_t)timeWk - timeOrg;

	check_measured_value(idx);

	return HAL_OK;
}

static HALRETURNCODE_T fncNop(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) {
	return HAL_ERROR;
}

static HALRETURNCODE_T fncDeviceVensorSpec(HALCOMPONENT_T* pHalComponent,HAL_ARGUMENT_T *pCmd,HAL_ARGUMENT_DEVICE_T *pCmdDev) {
	return HAL_ERROR;
}

HAL_FNCTBL_T HalSensorSCD41Tbl = {
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

static HALRETURNCODE_T measure(int32_t idx){
	SIM_SEN_T *simSen = &simSenAr[idx];
	HALOBSERVER_T *obsWk;
	int16_t error = 0;
	bool data_ready_flag = false;
    uint16_t timeout = 0;
    for (timeout = 0; (100000 * timeout) < (interval_in_seconds * 1200000);
         ++timeout) {
        error = scd4x_get_data_ready_flag(&data_ready_flag);
        if (error) {
#ifdef DEBUG
            printf("Error reading data_ready flag: %i\n", err);
#endif
			send_error(idx, 1);
        }
        if (data_ready_flag) {
            break;
        }
        usleep(100000);
    }
    if (!data_ready_flag) {
#ifdef DEBUG
        printf("Timeout waiting for data_ready flag\n");
#endif
		send_error(idx, 2);
		return HAL_ERROR;
    }
    error = scd4x_read_measurement(&co2_ppm, &temperature, &relative_humidity);
    if (error) {
#ifdef DEBUG
        printf("error reading measurement\n");
#endif
		send_error(idx, 3);
		return HAL_ERROR;
    }
#ifdef DEBUG 
	else {
        printf("measured co2 concentration: %u ppm, "
               "measured temperature: %d millidegreeCelsius, "
               "measured humidity: %d %%mRH\n",
               co2_ppm, temperature, relative_humidity);
    }
#endif
}

void check_measured_value(int32_t idx){
	SIM_SEN_T *simSen = &simSenAr[idx];
	HALOBSERVER_T *obsWk;

 	if ( simSen->valueList[0] > 1000 ) {
		obsWk = simSen->obs;
		while ( 0 != obsWk ) {
			obsWk->notify_event(simSen->hC,1);
			obsWk = HalLinkedList_getNext(obsWk);
		}
	}

 	if ( simSen->valueList[0] > 1500 ) {
		obsWk = simSen->obs;
		while ( 0 != obsWk ) {
			obsWk->notify_event(simSen->hC,2);
			obsWk = HalLinkedList_getNext(obsWk);
		}
	}
}

void send_error(int32_t idx, int32_t error_code){
	SIM_SEN_T *simSen = &simSenAr[idx];
	HALOBSERVER_T *obsWk;
	simSen->errCode = error_code;
	obsWk = simSen->obs;
	while ( 0 != obsWk ) {
		obsWk->notify_error(simSen->hC, simSen->errCode);
		obsWk = HalLinkedList_getNext(obsWk);
	}

}
