/*
 * openEL_SensorRaspiMouse.c
 *
 *  Created on: 2025/04/17
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
#include <time.h>
#include <stdlib.h>
#include <string.h>

static int32_t once = 1;
static FILE *fp;

static const char strName[] = "SENSOR_RT_RASPI_MOUSE";
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

static const HALPROPERTY_T sensor_rt_raspi_mouse_property = {
	(char *)strName,
	(char **)strFncLst,
	sizeof(strFncLst)/sizeof(char *)
};

static int32_t timeOrg;
static HALFLOAT_T sensorValueAr[16];

#define DEBUG 0

static HALRETURNCODE_T fncInit(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) {
	time_t timeWk;
#if DEBUG
	printf("%s:%s HAL-ID %d %d %d %d\n", __FILE__, __FUNCTION__, 
			pHalComponent->halId.deviceKindId,
			pHalComponent->halId.vendorId,
			pHalComponent->halId.productId,
			pHalComponent->halId.instanceId );
#endif
timeWk = time(&timeWk);
timeOrg = (int32_t)timeWk;

((HALSENSOR_T *)pHalComponent)->valueList = sensorValueAr;

	if (once) {
		printf("Light Sensor On\n");
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
		printf("Light Sensor Off\n");
		once = 1;
	}
	return HAL_OK;
}

static HALRETURNCODE_T fncAddObserver(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) {
	return HAL_OK;
}

static HALRETURNCODE_T fncRemoveObserver(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) {
	return HAL_OK;
}

static HALRETURNCODE_T fncGetProperty(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) {
	pHalComponent->property = (HALPROPERTY_T *)&sensor_rt_raspi_mouse_property;
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

static HALRETURNCODE_T fncSetVal(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) { //uint32_t id,HALFLOAT_T val) {
	return HAL_ERROR;
}

static HALRETURNCODE_T fncGetVal(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) { //uint32_t id,HALFLOAT_T *pOutVal) {
	return HAL_ERROR;
}

static HALRETURNCODE_T fncGetValLst(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) { //uint32_t *pOutSize,HALFLOAT_T *pOutValLst) {
	#if DEBUG
	printf("%s:%s\n", __FILE__, __FUNCTION__);
	#endif

	HALSENSOR_T *halSensor = (HALSENSOR_T *)pHalComponent;
	int32_t buff_size = 256;
	char buff[buff_size];
	char s2[8] = " ";
	char *tok;
	pCmd->num = 4;

	fp = fopen("/dev/rtlightsensor0", "r");

	if (fgets(buff, buff_size, fp) != NULL) {
		tok = strtok(buff, s2);
		halSensor->valueList[0] = (HALFLOAT_T)atoi(tok);
		tok = strtok(NULL, s2);
		halSensor->valueList[1] = (HALFLOAT_T)atoi(tok);
		tok = strtok(NULL, s2);
		halSensor->valueList[2] = (HALFLOAT_T)atoi(tok);
		tok = strtok(NULL, s2);
		halSensor->valueList[3] = (HALFLOAT_T)atoi(tok);
		fclose(fp);
		return HAL_OK;
	} else {
		fclose(fp);
		return HAL_ERROR;
	}
}

static HALRETURNCODE_T fncGetTmValLst(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) { //uint32_t *pOutSize,HALFLOAT_T *pOutValLst,int32_t *pOutTime) {
	#if DEBUG
	printf("%s:%s\n", __FILE__, __FUNCTION__);
	#endif

	time_t timeWk;
	HALSENSOR_T *halSensor = (HALSENSOR_T *)pHalComponent;
    int32_t buff_size = 256;
    char buff[buff_size];
	char s2[8] = " ";
	char *tok;
	pCmd->num = 4;

	fp = fopen("/dev/rtlightsensor0", "r");

	if (fgets(buff, buff_size, fp) != NULL) {
		tok = strtok(buff, s2);
		halSensor->valueList[0] = (HALFLOAT_T)atoi(tok);
		tok = strtok(NULL, s2);
		halSensor->valueList[1] = (HALFLOAT_T)atoi(tok);
		tok = strtok(NULL, s2);
		halSensor->valueList[2] = (HALFLOAT_T)atoi(tok);
		tok = strtok(NULL, s2);
		halSensor->valueList[3] = (HALFLOAT_T)atoi(tok);
		timeWk = time(&timeWk);
		halSensor->time = (int32_t)timeWk - timeOrg;
		fclose(fp);
		return HAL_OK;
	} else {
		fclose(fp);
		return HAL_ERROR;
	}
}

static HALRETURNCODE_T fncNop(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) {
	return HAL_ERROR;
}

static HALRETURNCODE_T fncDeviceVensorSpec(HALCOMPONENT_T* pHalComponent,HAL_ARGUMENT_T *pCmd,HAL_ARGUMENT_DEVICE_T *pCmdDev) {
	return HAL_ERROR;
}

HAL_FNCTBL_T HalSensorRaspiMouseTbl = {
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

