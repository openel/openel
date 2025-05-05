/*
 * openEL_MotorPCA9685.c
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
#include <stdint.h>
#include <unistd.h>
#include <math.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#define PCA9685_SUBADR1 0x2
#define PCA9685_SUBADR2 0x3
#define PCA9685_SUBADR3 0x4

#define PCA9685_MODE1 0x0
#define PCA9685_PRESCALE 0xFE

#define LED0_ON_L 0x6
#define LED0_ON_H 0x7
#define LED0_OFF_L 0x8
#define LED0_OFF_H 0x9

#define ALLLED_ON_L 0xFA
#define ALLLED_ON_H 0xFB
#define ALLLED_OFF_L 0xFC
#define ALLLED_OFF_H 0xFD

#define PWM_FREQUENCY 60                    //60Hz 16.7ms
#define PWM_PULSE_WIDTH_MAX 12000           //12ms

void PCA9685_init(float freq);
uint8_t PCA9685_read(uint8_t adr);
void PCA9685_write(uint8_t adr, uint8_t dat);
void PCA9685_pwmWrite(uint8_t ch, double pulseWidth_usec);
void PCA9685_setPWM(uint8_t ch, uint16_t onTime, uint16_t offTime);

static int i2c;
int once = 1;

static const char strName[] = "MOTOR_PCA9685";
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

typedef struct simMot_st {
	HALCOMPONENT_T *hC;
	HALOBSERVER_T *obs;
	double posCmd,pos1,posSen;
	HALFLOAT_T valueList[16];
	int32_t numObs;
	int32_t errCode;
	uint8_t inPos;
} SIM_MOT_T;
SIM_MOT_T simMotAr[16];

#define MAX_AXIS	15

static double posCmdAr[MAX_AXIS],posSenAr[MAX_AXIS];
static HALFLOAT_T valueList[MAX_AXIS][16];


static HALRETURNCODE_T fncInit(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) {
	printf("HalInit MotorPCA9685 HAL-ID %d %d %d %d\n",
			pHalComponent->halId.deviceKindId,
			pHalComponent->halId.vendorId,
			pHalComponent->halId.productId,
			pHalComponent->halId.instanceId );
	((HALACTUATOR_T *)pHalComponent)->valueList = valueList[pHalComponent->halId.instanceId];

	if (once) {
		char i2cFileName[] = "/dev/i2c-1";
		int driverAddress = 0x40;
		int i;
		double dfVal;

		if((i2c = open(i2cFileName, O_RDWR)) < 0){
			printf("I2C open err\n");
			return -1;
		}

		if(ioctl(i2c, I2C_SLAVE, driverAddress) < 0){
			printf("ioctl err\n");
			return -1;
		}

		PCA9685_init(PWM_FREQUENCY);

		once = 0;
	}

	return HAL_OK;
}

static HALRETURNCODE_T fncReInit(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) {
	printf("HalReInit MotorPCA9685\n");
	return HAL_OK;
}

static HALRETURNCODE_T fncFinalize(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) {
	printf("HalFinalize MotorPCA9685\n");
	if (once == 0) {
		close(i2c);
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
	pHalComponent->property = (HALPROPERTY_T *)&mot1_property;
	return HAL_OK;
}

static HALRETURNCODE_T fncHalGetTime(HALCOMPONENT_T *halComponent,HAL_ARGUMENT_T *pCmd) {
	return HAL_ERROR;
}

static HALRETURNCODE_T fncSetVal(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) { //uint32_t id,HALFLOAT_T val) {
//	printf("HalActuatorSet_MotorPCA9685\n");

	HALRETURNCODE_T retCode = HAL_ERROR;
	int32_t idx = pHalComponent->halId.instanceId;

 // int16_t _minAngle = 0;
 // int16_t _maxAngle = 180;
 // int16_t _minServoPL = 143;
 // int16_t _maxServoPL = 471;
 	int16_t _minAngle = 0;
 	int16_t _maxAngle = 120;
	int16_t _minServoPL = 265;
	int16_t _maxServoPL = 565;
	uint16_t pulseLen = 0;

	switch ( pCmd->FI.num ) {
	default:
		break;
	case HAL_REQUEST_POSITION_CONTROL:
		posSenAr[idx] = posCmdAr[idx];
//		printf("%s:%f\n", __FUNCTION__, posSenAr[idx]);
		posCmdAr[idx] = pCmd->FI.value;
//		simSensor1_setX(posSenAr[idx]);
//		printf("HalActuatorSet_MotorPCA9685:pCmd->FI.value=%f\n", pCmd->FI.value);
		pulseLen = (pCmd->FI.value-_minAngle)*(_maxServoPL-_minServoPL)/(_maxAngle-_minAngle) + _minServoPL;
		PCA9685_setPWM(idx - 1, 0, pulseLen);
		retCode = HAL_OK;
		break;
	}
	return retCode;
}

static HALRETURNCODE_T fncGetVal(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) { //uint32_t id,HALFLOAT_T *pOutVal) {
//	printf("HalActuatorGet_MotorPCA9685\n");
	HALRETURNCODE_T retCode = HAL_ERROR;
	int32_t idx = pHalComponent->halId.instanceId;

	switch ( pCmd->FI.num ) {
	default:
		break;
	case HAL_REQUEST_POSITION_COMMAND:
		pCmd->FI.value = posSenAr[idx];
//		printf("%s:%f\n", __FUNCTION__, posSenAr[idx]);
		retCode = HAL_OK;
		break;
	case HAL_REQUEST_POSITION_ACTUAL:
		pCmd->FI.value = posSenAr[idx];
//		printf("%s:%f\n", __FUNCTION__, posSenAr[idx]);
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

HAL_FNCTBL_T HalMotorPCA9685Tbl = {
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

void PCA9685_init(float freq)
{
    float prescaleval = 25000000;

    PCA9685_write(PCA9685_MODE1, 0x0);
    usleep(100000);//100ms
    uint8_t prescale = 101;
    uint8_t oldmode = PCA9685_read(PCA9685_MODE1);
    uint8_t newmode = (oldmode&0x7F) | 0x10;
    PCA9685_write(PCA9685_MODE1, newmode);
    PCA9685_write(PCA9685_PRESCALE, prescale);
    PCA9685_write(PCA9685_MODE1, oldmode);
    sleep(5);
    PCA9685_write(PCA9685_MODE1, oldmode | 0xa1);
}

void PCA9685_pwmWrite(uint8_t ch, double pulseWidth_usec)
{
    double pulselength;
    double pulseWidth;

    pulselength = 1000000 / PWM_FREQUENCY;
    pulselength /= 4096;
    pulseWidth = pulseWidth_usec / pulselength;

    PCA9685_setPWM(ch, 0, pulseWidth);
}

void PCA9685_setPWM(uint8_t ch, uint16_t onTime, uint16_t offTime)
{
    uint8_t sendData[5];

    sendData[0] = LED0_ON_L + 4 * ch;
    sendData[1] = (uint8_t)(0x00ff & onTime);
    sendData[2] = (uint8_t)((0xff00 & onTime) >> 8);
    sendData[3] = (uint8_t)(0x00ff & offTime);
    sendData[4] = (uint8_t)((0xff00 & offTime) >> 8);

    if(write(i2c, sendData, 5) != 5){
        printf("PCA9685_setPWM() err\n");
    }
}

uint8_t PCA9685_read(uint8_t adr)
{
    uint8_t sendData;
    uint8_t readData;

    sendData = adr;
    if(write(i2c, &sendData, 1) != 1){
        printf("PCA9685_read() err1\n");
    }
    else{
        if(read(i2c, &readData, 1) != 1){
            printf("PCA9685_read() err2\n");
        }
     }

    return readData;
}

void PCA9685_write(uint8_t adr, uint8_t dat)
{
    uint8_t buf[2];

    buf[0] = adr;
    buf[1] = dat;
    if(write(i2c, buf, 2) != 2){
        printf("PCA9685_write() err\n");
    }
}

#ifdef __cplusplus
} /* extern "C" */
} /* namespace el */
#endif /* __cplusplus */
