/*
 * openEL_SensorSeeedRotaryAngle.c
 *
 *  Created on: 2025/05/05
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
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>

uint16_t ADC_read(uint8_t adr);
HALFLOAT_T getRotaryAngle();

static int32_t i2c;
static int32_t once = 1;

static const char strName[] = "SENSOR_SEEED_ROTARY_ANGLE";
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

static const HALPROPERTY_T sensor_seeed_rotary_angle_property = {
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
		printf("Sensor On\n");
		char i2cFileName[] = "/dev/i2c-1";
		int driverAddress = 0x08;

		if((i2c = open(i2cFileName, O_RDWR)) < 0){
			printf("I2C open err\n");
			return HAL_ERROR;
		}

		if(ioctl(i2c, I2C_SLAVE, driverAddress) < 0){
			printf("ioctl err\n");
			return HAL_ERROR;
		}

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
		printf("Sensor Off\n");
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
	pHalComponent->property = (HALPROPERTY_T *)&sensor_seeed_rotary_angle_property;
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
	pCmd->num = 1;
	halSensor->valueList[0] = getRotaryAngle();
	
	return HAL_OK;
}

static HALRETURNCODE_T fncGetTmValLst(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) { //uint32_t *pOutSize,HALFLOAT_T *pOutValLst,int32_t *pOutTime) {
	#if DEBUG
	printf("%s:%s\n", __FILE__, __FUNCTION__);
	#endif

	time_t timeWk;
	HALSENSOR_T *halSensor = (HALSENSOR_T *)pHalComponent;
	pCmd->num = 1;

	return HAL_OK;
}

static HALRETURNCODE_T fncNop(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) {
	return HAL_ERROR;
}

static HALRETURNCODE_T fncDeviceVensorSpec(HALCOMPONENT_T* pHalComponent,HAL_ARGUMENT_T *pCmd,HAL_ARGUMENT_DEVICE_T *pCmdDev) {
	return HAL_ERROR;
}

HAL_FNCTBL_T HalSensorSeeedRotaryAngleTbl = {
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

uint16_t ADC_read(uint8_t adr)
{
    uint8_t readData[2];
	int32_t ret;

    ret = read(i2c, &readData, 2);
    if(ret == -1){
        perror("ADC_read() err1 ");
		return -1;
    } else if(ret != 2){
            printf("ADC_read() err2\n");
    }
//	printf("%s:readData[0]=%x readData[1]=%x\n", __FUNCTION__, readData[0], readData[1]);
//	printf("%s:%d\n", __FUNCTION__, (readData[1] * 256 ) + readData[0]);

	return ((readData[1] * 256 ) + readData[0]);
}

HALFLOAT_T getRotaryAngle(){
	uint16_t data;
	data = ADC_read(0x20); /* Range:0x0 to 0x3E7(999)*/
	return (HALFLOAT_T)(300 * data / 999); /* Range: 0 to 300 degrees */
}

#ifdef __cplusplus
} /* extern "C" */
} /* namespace el */
#endif /* __cplusplus */

