/*
 * openEL_CO2SensorSCD30.c
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
#include "openEL_CO2SensorSCD30.h"

static int32_t i2c;
static int32_t once = 1;

static const char strName[] = "CO2SENSOR_SCD30";
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
static float co2_ppm, temperature, relative_humidity;
void measure(int32_t idx);
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
		sensirion_i2c_init();
		while (scd30_probe() != NO_ERROR) {
			printf("SCD30 sensor probing failed.\n");
			usleep(1000000);
		}
		printf("SCD30 sensor probing successful.\n");
		scd30_set_measurement_interval(interval_in_seconds);
		usleep(20000);
		scd30_start_periodic_measurement(0);
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
		scd30_stop_periodic_measurement();
		sensirion_i2c_release();
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

	measure(idx);

	simSen->valueList[0] = (HALFLOAT_T)co2_ppm;
	simSen->valueList[1] = (HALFLOAT_T)temperature;
	simSen->valueList[2] = (HALFLOAT_T)relative_humidity;

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

	measure(idx);

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

HAL_FNCTBL_T HalSensorSCD30Tbl = {
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

void measure(int32_t idx){
	SIM_SEN_T *simSen = &simSenAr[idx];
	HALOBSERVER_T *obsWk;
    uint16_t data_ready = 0;
    uint16_t timeout = 0;
	int16_t err;
    for (timeout = 0; (100000 * timeout) < (interval_in_seconds * 1200000);
         ++timeout) {
        err = scd30_get_data_ready(&data_ready);
        if (err != NO_ERROR) {
#ifdef DEBUG
            printf("Error reading data_ready flag: %i\n", err);
#endif
			send_error(idx, 1);
        }
        if (data_ready) {
            break;
        }
        usleep(100000);
    }
    if (!data_ready) {
#ifdef DEBUG
        printf("Timeout waiting for data_ready flag\n");
#endif
		send_error(idx, 2);
    }
    err = scd30_read_measurement(&co2_ppm, &temperature, &relative_humidity);
    if (err != NO_ERROR) {
#ifdef DEBUG
        printf("error reading measurement\n");
#endif
		send_error(idx, 3);
    }
#ifdef DEBUG 
	else {
        printf("measured co2 concentration: %0.2f ppm, "
               "measured temperature: %0.2f degreeCelsius, "
               "measured humidity: %0.2f %%RH\n",
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
