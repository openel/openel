/*
 * openEL_SensorINA219.c
 *
 *  Created on: 2022/01/21
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
#include <errno.h>

#include "openEL.h"
#include "openEL_registry.h"
#include "openEL_SensorINA219.h"

/* Register */
#define REG_CONFIG	     0x00 /* Config Register (R/W) */
#define REG_SHUNTVOLTAGE 0x01 /* SHUNT VOLTAGE REGISTER (R) */
#define REG_BUSVOLTAGE   0x02 /* BUS VOLTAGE REGISTER (R) */
#define REG_POWER        0x03 /* POWER REGISTER (R) */
#define REG_CURRENT      0x04 /* CURRENT REGISTER (R) */
#define REG_CALIBRATION  0x05 /* CALIBRATION REGISTER (R/W) */

/* BusVoltageRange */
#define RANGE_16V 0x00 /* set bus voltage range to 16V */
#define RANGE_32V 0x01 /* set bus voltage range to 32V (default) */

/* Gain */
#define DIV_1_40MV  0x00 /* shunt prog. gain set to  1, 40 mV range */
#define DIV_2_80MV  0x01 /* shunt prog. gain set to /2, 80 mV range */
#define DIV_4_160MV 0x02 /* shunt prog. gain set to /4, 160 mV range */
#define DIV_8_320MV 0x03 /* shunt prog. gain set to /8, 320 mV range */

/* ADCResolution */
#define ADCRES_9BIT_1S    0x00 /*  9bit,   1 sample,     84us */
#define ADCRES_10BIT_1S   0x01 /* 10bit,   1 sample,    148us */
#define ADCRES_11BIT_1S   0x02 /* 11 bit,  1 sample,    276us */
#define ADCRES_12BIT_1S   0x03 /* 12 bit,  1 sample,    532us */
#define ADCRES_12BIT_2S   0x09 /* 12 bit,  2 samples,  1.06ms */
#define ADCRES_12BIT_4S   0x0A /* 12 bit,  4 samples,  2.13ms */
#define ADCRES_12BIT_8S   0x0B /* 12bit,   8 samples,  4.26ms */
#define ADCRES_12BIT_16S  0x0C /* 12bit,  16 samples,  8.51ms */
#define ADCRES_12BIT_32S  0x0D /* 12bit,  32 samples, 17.02ms */
#define ADCRES_12BIT_64S  0x0E /* 12bit,  64 samples, 34.05ms */
#define ADCRES_12BIT_128S 0x0F /* 12bit, 128 samples, 68.10ms */

/* class Mode */
#define POWERDOW             0x00 /* power down */
#define SVOLT_TRIGGERED      0x01 /* shunt voltage triggered */
#define BVOLT_TRIGGERED      0x02 /* bus voltage triggered */
#define SANDBVOLT_TRIGGERED  0x03 /* shunt and bus voltage triggered */
#define ADCOFF               0x04 /* ADC off */
#define SVOLT_CONTINUOUS     0x05 /* shunt voltage continuous */
#define BVOLT_CONTINUOUS     0x06 /* bus voltage continuous */
#define SANDBVOLT_CONTINUOUS 0x07 /* shunt and bus voltage continuous */

void setCalibration_32V_2A();
void INA219_write(uint8_t adr, uint16_t dat);
uint16_t INA219_read(uint8_t adr);
HALFLOAT_T ina219_current_lsb = .1;
uint16_t ina219_cal_value = 4096;
HALFLOAT_T ina219_power_lsb = .002;
HALFLOAT_T getShuntVoltage_mV();
HALFLOAT_T getBusVoltage_V();
HALFLOAT_T getPower_W();
HALFLOAT_T getCurrent_mA();
void checkVoltage(int32_t idx);

static int32_t i2c;
static int32_t once = 1;

static const char strName[] = "SENSOR_INA219";
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
		char i2cFileName[] = "/dev/i2c-1";
		int driverAddress = 0x41;

		if((i2c = open(i2cFileName, O_RDWR)) < 0){
			printf("I2C open err\n");
			return HAL_ERROR;
		}

		if(ioctl(i2c, I2C_SLAVE, driverAddress) < 0){
			printf("ioctl err\n");
			return HAL_ERROR;
		}

		setCalibration_32V_2A();

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
		close(i2c);
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
	simSen->valueList[0] = getShuntVoltage_mV();
	simSen->valueList[1] = getBusVoltage_V();
	simSen->valueList[2] = getPower_W();
	simSen->valueList[3] = getCurrent_mA();

	HALSENSOR_T *halSensor = (HALSENSOR_T *)pHalComponent;

	pCmd->num = 4;
	halSensor->valueList[0] = simSen->valueList[0];
	halSensor->valueList[1] = simSen->valueList[1];
	halSensor->valueList[2] = simSen->valueList[2];
	halSensor->valueList[3] = simSen->valueList[3];

	checkVoltage(idx);

	return HAL_OK;
}

static HALRETURNCODE_T fncGetTmValLst(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) { //uint32_t *pOutSize,HALFLOAT_T *pOutValLst,int32_t *pOutTime) {
#ifdef DEBUG
	printf("%s:%s\n", __FILE__, __FUNCTION__);
#endif
	time_t timeWk;
	int32_t idx = pHalComponent->halId.instanceId;
	SIM_SEN_T *simSen = &simSenAr[idx];
	simSen->valueList[0] = getShuntVoltage_mV();
	simSen->valueList[1] = getBusVoltage_V();
	simSen->valueList[2] = getPower_W();
	simSen->valueList[3] = getCurrent_mA();

	HALSENSOR_T *halSensor = (HALSENSOR_T *)pHalComponent;

	pCmd->num = 4;
	halSensor->valueList[0] = simSen->valueList[0];
	halSensor->valueList[1] = simSen->valueList[1];
	halSensor->valueList[2] = simSen->valueList[2];
	halSensor->valueList[3] = simSen->valueList[3];

	checkVoltage(idx);

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

HAL_FNCTBL_T HalSensorINA219Tbl = {
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

void setCalibration_32V_2A(){
	uint8_t bus_voltage_range = RANGE_32V;
	uint8_t gain = DIV_8_320MV;
	uint8_t bus_adc_resolution = ADCRES_12BIT_32S;
	uint8_t shunt_adc_resolution = ADCRES_12BIT_32S;
	uint8_t mode = SANDBVOLT_CONTINUOUS;
	uint16_t config = (bus_voltage_range << 13 |
        			  gain << 11 |
        		      bus_adc_resolution << 7 |
    				  shunt_adc_resolution << 3 |
        		      mode);
	INA219_write(REG_CALIBRATION, ina219_cal_value);
    INA219_write(REG_CONFIG, config);
}

void INA219_write(uint8_t adr, uint16_t dat)
{
    uint8_t buf[3];
	int32_t ret;

    buf[0] = adr;
    buf[1] = (uint8_t)((0xff00 & dat) >> 8);
    buf[2] = (uint8_t)(0x00ff & dat);
//	printf("%s:buf[0]=%x buf[1]=%x buf[2]=%x \n",__FUNCTION__,buf[0],buf[1],buf[2]);
    ret = write(i2c, buf, 3);
    if(ret == -1){
        perror("INA219_write() err1 ");
		return;
    } else if(ret != 3){
        printf("INA219_write() err\n");
    }
}

uint16_t INA219_read(uint8_t adr)
{
    uint8_t sendData;
    uint8_t readData[2];
	int32_t ret;

    sendData = adr;
    ret = write(i2c, &sendData, 1);
    if(ret == -1){
        perror("INA219_read() err1 ");
		return -1;
    } else if(ret != 1){
        printf("INA219_read() err1\n");
		return -1;
    }
    ret = read(i2c, &readData, 2);
    if(ret == -1){
        perror("INA219_read() err1 ");
		return -1;
    } else if(ret != 2){
            printf("INA219_read() err2\n");
    }
//	printf("%s:readData[0]=%x readData[1]=%x\n", __FUNCTION__, readData[0], readData[1]);
    return ((readData[0] * 256 ) + readData[1]);
}

HALFLOAT_T getShuntVoltage_mV() {
	uint16_t data;
	int16_t vshunt;
	data = INA219_read(REG_SHUNTVOLTAGE);
	if (data > 32767) {
		vshunt = data - 65535;
	} else {
		vshunt = data;
	}
	return (vshunt * 0.01);
}

HALFLOAT_T getBusVoltage_V(){
	uint16_t data;
	data = INA219_read(REG_BUSVOLTAGE);
	return ((data >> 3) * 0.004);
}

HALFLOAT_T getPower_W(){
	uint16_t data;
	data = INA219_read(REG_POWER);
	return (data * ina219_power_lsb);
}

HALFLOAT_T getCurrent_mA(){
	uint16_t data;
	int16_t current;
	data = INA219_read(REG_CURRENT);
	if (data > 32767){
		current = data - 65535;
	} else {
		current = data;
	}
	return (current * ina219_current_lsb);
}

void checkVoltage(int32_t idx) {
	SIM_SEN_T *simSen = &simSenAr[idx];
	HALOBSERVER_T *obsWk;

 	/* Charging. */
 	if ( simSen->valueList[2] > 1 ) {
		obsWk = simSen->obs;
		while ( 0 != obsWk ) {
			obsWk->notify_event(simSen->hC,1);
			obsWk = HalLinkedList_getNext(obsWk);
		}
	}

 	/* Charging completed. */
 	if ( simSen->valueList[1] > 12.3 ) {
		obsWk = simSen->obs;
		while ( 0 != obsWk ) {
			obsWk->notify_event(simSen->hC,2);
			obsWk = HalLinkedList_getNext(obsWk);
		}
	}
 	/* The voltage is less than 10V. */
 	if ( simSen->valueList[1] < 10 ) {
		obsWk = simSen->obs;
		while ( 0 != obsWk ) {
			obsWk->notify_event(simSen->hC,3);
			obsWk = HalLinkedList_getNext(obsWk);
		}
	}

	/* Error(The voltage is less than 9V.) */
	if ( simSen->valueList[1] < 9 ) {
		simSen->errCode = 200+idx;
		obsWk = simSen->obs;
		while ( 0 != obsWk ) {
			obsWk->notify_error(simSen->hC, simSen->errCode);
			obsWk = HalLinkedList_getNext(obsWk);
		}
	}
}
