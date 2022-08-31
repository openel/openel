/*
BSD 3-Clause License

Copyright (c) 2018-2022, Japan Embedded Systems Technology Association(JASA)
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of the copyright holder nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*
    Copyright (c) 2012 seeed technology inc.
    Website    : www.seeed.cc
    Author     : Jerry Yip
    Create Time: 2017-02
    Change Log :
    The MIT License (MIT)
    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:
    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.
    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.
*/

/*
#define OPENEL_SW_SURFACE_FRIEND 0
*/
#include <stdio.h>
#include "openEL.h"
#include "openEL_registry.h"
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>

#ifndef M_PI
#define M_PI           3.14159265358979323846  /* pi */
#endif

/******I2C command definitions*************/
#define MotorSpeedSet             0x82
#define PWMFrequenceSet           0x84
#define DirectionSet              0xaa
#define MotorSetA                 0xa1
#define MotorSetB                 0xa5
#define Nothing                   0x01
/**************Motor ID**********************/
#define MOTOR1 			          1
#define MOTOR2 			          2
/**************Motor Direction***************/
#define BothClockWise             0x0a
#define BothAntiClockWise         0x05
#define M1CWM2ACW                 0x06
#define M1ACWM2CW                 0x09
/**************Motor Direction***************/
#define ClockWise                 0x0a
#define AntiClockWise             0x05
/**************Prescaler Frequence***********/
#define F_31372Hz                 0x01
#define F_3921Hz                  0x02
#define F_490Hz                   0x03
#define F_122Hz                   0x04
#define F_30Hz                    0x05

// _speed1: 0~100  _speed2: 0~100
static unsigned char _speed1 = 0;
static unsigned char _speed2 = 0;
// the direction of M1 and M2 DC motor 1:clockwise  -1:anticlockwise
static int _M1_direction = 1;
static int _M2_direction = 1;
// _i2c_add: 0x00~0x0f
static unsigned char _i2c_add = 0x0f;
// Set the direction of both motors
// _direction: BothClockWise, BothAntiClockWise, M1CWM2ACW, M1ACWM2CW
static void direction(unsigned char _direction);
static unsigned char _step_cnt = 0;
static void speed(unsigned char motor_id, int _speed);
static void frequence(unsigned char _frequence);
static void stop(unsigned char motor_id);
static void StepperRun(int _step, int _type, int _mode);

static void L298P_init();
static uint8_t L298P_read(uint8_t adr);
static void L298P_write(uint8_t adr, uint8_t dat);
static void L298P_write_16(uint8_t adr, uint16_t dat);

static int32_t i2c = -1;

static const char strName[] = "MOTOR_L298P";
static const char *strFncLst[] = {
	"HalInit",
	"HalReInit",
	"HalFinalize",
	"HalGetProperty",
	"HalActuatorSetValue",
	"HalActuatorGetValue"
};
static const HALPROPERTY_T mot1_property = {
	(char *)strName,
	(char **)strFncLst,
	sizeof(strFncLst)/sizeof(char *)
};

#define MAX_AXIS	2
#define NUMBER_OF_VALUE 1

static double velCmdAr[MAX_AXIS+1],velSenAr[MAX_AXIS+1];
static HALFLOAT_T valueList[MAX_AXIS+1][NUMBER_OF_VALUE];

static HALRETURNCODE_T fncInit(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) {
    #ifdef DEBUG
	printf("%s:%s:HAL-ID:0x%x 0x%x 0x%x 0x%x\n", __FILE__, __FUNCTION__,
			pHalComponent->halId.deviceKindId,
			pHalComponent->halId.vendorId,
			pHalComponent->halId.productId,
			pHalComponent->halId.instanceId );
    #endif
	if (pHalComponent->halId.instanceId < 1 || pHalComponent->halId.instanceId > MAX_AXIS) {
		printf("instanceId should be 1 or 2.");
		return HAL_ERROR;
	}

	((HALACTUATOR_T *)pHalComponent)->valueList = valueList[pHalComponent->halId.instanceId];

	if (i2c == -1) {
		char i2cFileName[] = "/dev/i2c-1";
		int driverAddress = _i2c_add;

		if((i2c = open(i2cFileName, O_RDWR)) < 0){
			printf("I2C open err\n");
			return HAL_ERROR;
		}

		if(ioctl(i2c, I2C_SLAVE, driverAddress) < 0){
			printf("ioctl err\n");
			close(i2c);
			i2c = -1;
			return HAL_ERROR;
		}

        L298P_init();
	}
	return HAL_OK;
}

static HALRETURNCODE_T fncReInit(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) {
    #ifdef DEBUG
	printf("%s:%s\n", __FILE__, __FUNCTION__);
    #endif
	return HAL_OK;
}

static HALRETURNCODE_T fncFinalize(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) {
    #ifdef DEBUG
	printf("%s:%s\n", __FILE__, __FUNCTION__);
    #endif
	if (i2c != -1) {
		close(i2c);
		i2c = -1;
	}
	return HAL_OK;
}

static HALRETURNCODE_T fncAddObserver(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) {
	printf("%s:%s:HalAddObserver is not supported.\n", __FILE__, __FUNCTION__);
	return HAL_ERROR;
}

static HALRETURNCODE_T fncRemoveObserver(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) {
	printf("%s:%s:HalRemoveObserver is not supported.\n", __FILE__, __FUNCTION__);
	return HAL_ERROR;
}

static HALRETURNCODE_T fncGetProperty(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) {
    #ifdef DEBUG
	printf("%s:%s\n", __FILE__, __FUNCTION__);
    #endif
	pHalComponent->property = (HALPROPERTY_T *)&mot1_property;
	return HAL_OK;
}

static HALRETURNCODE_T fncHalGetTime(HALCOMPONENT_T *halComponent,HAL_ARGUMENT_T *pCmd) {
	printf("%s:%s:HalGetTime is not supported.\n", __FILE__, __FUNCTION__);
	return HAL_ERROR;
}

static HALRETURNCODE_T fncSetVal(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) {
    #ifdef DEBUG
	printf("%s:%s\n", __FILE__, __FUNCTION__);
    #endif
	HALRETURNCODE_T retCode = HAL_ERROR;
	int32_t idx = pHalComponent->halId.instanceId;
	if (idx < 1 || idx > 2) {
		printf("%s:instanceId must be 1 or 2!\n", __FUNCTION__);
        return HAL_ERROR;
	}

	switch ( pCmd->FI.num ) {
	default:
		break;
	case HAL_REQUEST_NO_EXCITE:
        #ifdef DEBUG
		printf("%s:HAL_REQUEST_NO_EXCITE.\n", __FUNCTION__);
        #endif
		velSenAr[idx] = 0;
		velCmdAr[idx] = 0;
        speed(idx, velCmdAr[idx]);
		retCode = HAL_OK;
		break;
	case HAL_REQUEST_POSITION_CONTROL:
		printf("%s:HAL_REQUEST_POSITION_CONTROL is not supported.\n", __FUNCTION__);
		retCode = HAL_ERROR;
		break;
	case HAL_REQUEST_VELOCITY_CONTROL:
    	if (pCmd->FI.value < -100 || pCmd->FI.value > 100) {
	    	printf("%s:Velocity must be between -100 and 100!\n", __FUNCTION__);
            return HAL_ERROR;
	    }
		velSenAr[idx] = velCmdAr[idx];
		velCmdAr[idx] = pCmd->FI.value;
        #ifdef DEBUG
		printf("%s:velSenAr[%d]=%f\n", __FUNCTION__, idx, velSenAr[idx]);
		printf("%s:velCmdAr[%d]=%f\n", __FUNCTION__, idx, velCmdAr[idx]);
        #endif
        speed(idx, velCmdAr[idx]);
		retCode = HAL_OK;
		break;
	case HAL_REQUEST_TORQUE_CONTROL:
		printf("%s:HAL_REQUEST_TORQUE_CONTROL is not supported.\n", __FUNCTION__);
		retCode = HAL_ERROR;
		break;
	}
	return retCode;
}

static HALRETURNCODE_T fncGetVal(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) { //uint32_t id,HALFLOAT_T *pOutVal) {
    #ifdef DEBUG
	printf("%s:%s\n", __FILE__, __FUNCTION__);
    #endif
	HALRETURNCODE_T retCode = HAL_ERROR;
	int32_t idx = pHalComponent->halId.instanceId;
	if (idx < 1 || idx > 2) {
		printf("%s:instanceId must be 1 or 2!\n", __FUNCTION__);
        return HAL_ERROR;
	}

	switch ( pCmd->FI.num ) {
	default:
		break;
	case HAL_REQUEST_POSITION_COMMAND:
		pCmd->FI.value = 0;
		printf("%s:HAL_REQUEST_POSITION_COMMAND is not supported.\n", __FUNCTION__);
		retCode = HAL_ERROR;
		break;
	case HAL_REQUEST_POSITION_ACTUAL:
		pCmd->FI.value = 0;
		printf("%s:HAL_REQUEST_POSITION_ACTUAL is not supported.\n", __FUNCTION__);
		retCode = HAL_ERROR;
		break;
	case HAL_REQUEST_VELOCITY_COMMAND:
		pCmd->FI.value = velSenAr[idx];
        #ifdef DEBUG
		printf("%s:%f\n", __FUNCTION__, velSenAr[idx]);
        #endif
		retCode = HAL_OK;
		break;
	case HAL_REQUEST_VELOCITY_ACTUAL:
		pCmd->FI.value = velSenAr[idx];
        #ifdef DEBUG
		printf("%s:%f\n", __FUNCTION__, velSenAr[idx]);
        #endif
		retCode = HAL_OK;
		break;
	case HAL_REQUEST_TORQUE_COMMAND:
		pCmd->FI.value = 0;
		printf("%s:HAL_REQUEST_TORQUE_COMMAND is not supported.\n", __FUNCTION__);
		retCode = HAL_ERROR;
		break;
	case HAL_REQUEST_TORQUE_ACTUAL:
		pCmd->FI.value = 0;
		printf("%s:HAL_REQUEST_TORQUE_ACTUAL is not supported.\n", __FUNCTION__);
		retCode = HAL_ERROR;
		break;
    }
	return retCode;
}

static HALRETURNCODE_T fncGetValLst(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) { //uint32_t *pOutSize,HALFLOAT_T *pOutValLst) {
	printf("%s:%s:HalGetValueList is not supported.\n", __FILE__, __FUNCTION__);
	return HAL_ERROR;
}

static HALRETURNCODE_T fncGetTmValLst(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) { //uint32_t *pOutSize,HALFLOAT_T *pOutValLst,int32_t *pOutTime) {
	printf("%s:%s:HalGetTimedValueList is not supported.\n", __FILE__, __FUNCTION__);
	return HAL_ERROR;
}

static HALRETURNCODE_T fncNop(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) {
	return HAL_ERROR;
}

static HALRETURNCODE_T fncDeviceVensorSpec(HALCOMPONENT_T* pHalComponent,HAL_ARGUMENT_T *pCmd,HAL_ARGUMENT_DEVICE_T *pCmdDev) {
	return HAL_ERROR;
}

HAL_FNCTBL_T HalMotorL298PTbl = {
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

static void L298P_init()
{
	// Set default frequence to F_3921Hz
	frequence(F_3921Hz);

}

static uint8_t L298P_read(uint8_t adr)
{
    uint8_t sendData;
    uint8_t readData;
	int32_t ret;

    sendData = adr;
    ret = write(i2c, &sendData, 1);
    if(ret == -1){
        perror("L298P_read() err ");
		return ret;
	} else if(ret != 1){
        perror("L298P_read() err1\n");
		return ret;
    }
    ret = read(i2c, &readData, 1);
    if( ret == -1){
        perror("L298P_read() err ");
		return ret;
	} else if (ret != 1){
        perror("L298P_read() err2\n");
		return ret;
    }

    return readData;
}

static void L298P_write(uint8_t adr, uint8_t dat)
{
    uint8_t buf[3];
	int32_t ret;

    buf[0] = adr;
    buf[1] = dat;
    buf[2] = Nothing;
    ret = write(i2c, buf, 3);
    if( ret == -1){
        perror("L298P_write() err ");
		return;
	} else if(ret != 3){
        perror("L298P_write() err\n");
    }
}

static void L298P_write_16(uint8_t adr, uint16_t dat)
{
    uint8_t buf[3];
	int32_t ret;

    buf[0] = adr;
    buf[1] = (uint8_t)((0xff00 & dat) >> 8);
    buf[2] = (uint8_t)(0x00ff & dat);
    #ifdef DEBUG
	printf("%s:buf[0]=%x buf[1]=%x buf[2]=%x \n",__FUNCTION__,buf[0],buf[1],buf[2]);
    #endif
    ret = write(i2c, buf, 3);
    if(ret == -1){
        perror("L298_write() err1 ");
		return;
    } else if(ret != 3){
        perror("L298_write() err\n");
    }
}


// *****************************Private Function*******************************
// Set the direction of 2 motors
// _direction: M1CWM2ACW(M1 ClockWise M2 AntiClockWise), M1ACWM2CW, BothClockWise, BothAntiClockWise,
static void direction(unsigned char _direction) {
	L298P_write(DirectionSet, _direction);
    usleep(4*1000); 				                // wait
}

static long map(long x, long in_min, long in_max, long out_min, long out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// *****************************DC Motor Function******************************
// Set the speed of a motor, speed is equal to duty cycle here
// motor_id: MOTOR1, MOTOR2
// _speed: -100~100, when _speed>0, dc motor runs clockwise; when _speed<0,
// dc motor runs anticlockwise
static void speed(unsigned char motor_id, int _speed) {
    uint16_t dat;
    if (motor_id < MOTOR1 || motor_id > MOTOR2) {
        printf("Motor id error! Must be MOTOR1 or MOTOR2\n");
        return;
    }

    if (motor_id == MOTOR1) {
        if (_speed >= 0) {
            _M1_direction = 1;
            _speed = _speed > 100 ? 100 : _speed;
            _speed1 = map(_speed, 0, 100, 0, 255);
        } else if (_speed < 0) {
            _M1_direction = -1;
            _speed = _speed < -100 ? 100 : -(_speed);
            _speed1 = map(_speed, 0, 100, 0, 255);
        }
    } else if (motor_id == MOTOR2) {
        if (_speed >= 0) {
            _M2_direction = 1;
            _speed = _speed > 100 ? 100 : _speed;
            _speed2 = map(_speed, 0, 100, 0, 255);
        } else if (_speed < 0) {
            _M2_direction = -1;
            _speed = _speed < -100 ? 100 : -(_speed);
            _speed2 = map(_speed, 0, 100, 0, 255);
        }
    }
    // Set the direction
    if (_M1_direction == 1 && _M2_direction == 1) {
        direction(BothClockWise);
        #ifdef DEBUG
        printf("BothClockWise\n");
        #endif
    }
    if (_M1_direction == 1 && _M2_direction == -1) {
        direction(M1CWM2ACW);
        #ifdef DEBUG
        printf("M1CWM2ACW\n");
        #endif
    }
    if (_M1_direction == -1 && _M2_direction == 1) {
        direction(M1ACWM2CW);
        #ifdef DEBUG
        printf("M1ACWM2CW\n");
        #endif
    }
    if (_M1_direction == -1 && _M2_direction == -1) {
        direction(BothAntiClockWise);
        #ifdef DEBUG
        printf("BothAntiClockWise\n");
        #endif
    }
    dat = _speed1;
    dat = dat << 8;
    dat = dat | _speed2;
    // send command
	L298P_write_16(MotorSpeedSet, dat);
    usleep(4*1000); 				                // Wait
}

// Set the frequence of PWM(cycle length = 510, system clock = 16MHz)
// F_3921Hz is default
// _frequence: F_31372Hz, F_3921Hz, F_490Hz, F_122Hz, F_30Hz
static void frequence(unsigned char _frequence) {
    if (_frequence < F_31372Hz || _frequence > F_30Hz) {
        printf("frequence error! Must be F_31372Hz, F_3921Hz, F_490Hz, F_122Hz, F_30Hz\n");
        return;
    }
    L298P_write(PWMFrequenceSet, _frequence);            // set frequence header
    usleep(4*1000); 				                // wait
}

// Stop one motor
// motor_id: MOTOR1, MOTOR2
static void stop(unsigned char motor_id) {
    if (motor_id < MOTOR1 || motor_id > MOTOR2) {
        printf("Motor id error! Must be MOTOR1 or MOTOR2\n");
        return;
    }
    speed(motor_id, 0);
}

// ***************************Stepper Motor Function***************************
// Drive a stepper motor
// _step: -1024~1024, when _step>0, stepper motor runs clockwise; when _step<0,
// stepper motor runs anticlockwise; when _step is 512, the stepper motor will
// run a complete turn; if step is 1024, the stepper motor will run 2 turns.
//  _type: 0 -> 4 phase stepper motor, default
//         1 -> 2 phase stepper motor
//  _mode: 0 -> compatible mode (_step=1 corresponds 4 steps)
//         1 -> fine mode (_step1 corresponds 1 steps)
static void StepperRun(int _step, int _type, int _mode) {
    int _direction = 1;
    if (_step > 0) {
        _direction = 1; //clockwise
        _step = _step > 1024 ? 1024 : _step;
    } else if (_step < 0) {
        _direction = -1; //anti-clockwise
        _step = _step < -1024 ? 1024 : -(_step);
    }
    _speed1 = 255;
    _speed2 = 255;
	L298P_write_16(MotorSpeedSet, (_speed1 << 8 | _speed2));
    usleep(4*1000); 				                // Wait

    if (_type == 1) {
        if (_direction == 1) {				// 2 phase motor
            for (int i = 0; i < _step; i++) {
                if (_mode == 0) {
                    direction(0b0001);
                    direction(0b0101);
                    direction(0b0100);
                    direction(0b0110);
                    direction(0b0010);
                    direction(0b1010);
                    direction(0b1000);
                    direction(0b1001);
                } else {
                    switch (_step_cnt) {
                        case 0 : direction(0b0001); direction(0b0101); break;
                        case 1 : direction(0b0100); direction(0b0110); break;
                        case 2 : direction(0b0010); direction(0b1010); break;
                        case 3 : direction(0b1000); direction(0b1001); break;
                    }
                    _step_cnt = (_step_cnt + 1) % 4;
                }
            }
        } else if (_direction == -1) {
            for (int i = 0; i < _step; i++) {
                if (_mode == 0) {
                    direction(0b1000);
                    direction(0b1010);
                    direction(0b0010);
                    direction(0b0110);
                    direction(0b0100);
                    direction(0b0101);
                    direction(0b0001);
                    direction(0b1001);
                } else {
                    switch (_step_cnt) {
                        case 0 : direction(0b1000); direction(0b1010); break;
                        case 1 : direction(0b0010); direction(0b0110); break;
                        case 2 : direction(0b0100); direction(0b0101); break;
                        case 3 : direction(0b0001); direction(0b1001); break;
                    }
                    _step_cnt = (_step_cnt + 1) % 4;
                }
            }
        }
    } else if (_type == 0) {
        if (_direction == 1) {				// 4 phase motor
            for (int i = 0; i < _step; i++) {
                if (_mode == 0) {
                    direction(0b0001);
                    direction(0b0011);
                    direction(0b0010);
                    direction(0b0110);
                    direction(0b0100);
                    direction(0b1100);
                    direction(0b1000);
                    direction(0b1001);
                } else {
                    switch (_step_cnt) {
                        case 0 : direction(0b0001); direction(0b0011); break;
                        case 2 : direction(0b0010); direction(0b0110); break;
                        case 3 : direction(0b0100); direction(0b1100); break;
                        case 4 : direction(0b1000); direction(0b1001); break;
                    }
                    _step_cnt = (_step_cnt + 1) % 4;
                }
            }
        } else if (_direction == -1) {
            for (int i = 0; i < _step; i++) {
                if (_mode == 0) {
                    direction(0b1000);
                    direction(0b1100);
                    direction(0b0100);
                    direction(0b0110);
                    direction(0b0010);
                    direction(0b0011);
                    direction(0b0001);
                    direction(0b1001);
                } else {
                    switch (_step_cnt) {
                        case 0 : direction(0b1000); direction(0b1100); break;
                        case 1 : direction(0b0100); direction(0b0110); break;
                        case 2 : direction(0b0010); direction(0b0011); break;
                        case 3 : direction(0b0001); direction(0b1001); break;
                    }
                    _step_cnt = (_step_cnt + 1) % 4;
                }
            }
        }
    }
}
