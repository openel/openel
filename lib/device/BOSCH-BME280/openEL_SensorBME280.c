/*
BSD 3-Clause License

Copyright (c) 2018-2022, Japan Embedded Systems Technology Association(JASA)
All rights reserved.
Copyright (c) 2020 Bosch Sensortec GmbH. All rights reserved.

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

#include <HAL4RT.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include "openEL.h"
#include "openEL_registry.h"
#include "openEL_SensorBME280.h"

#include "BME280_driver/bme280.h"

/******************************************************************************/
/*!                               Structures                                  */

/* Structure that contains identifier details used in example */
struct identifier
{
	/* Variable to hold device address */
	uint8_t dev_addr;

	/* Variable that contains file descriptor */
	int8_t fd;
};

static struct bme280_dev dev;
static struct identifier id;

/****************************************************************************/
/*!                         Functions                                       */

/*!
 *  @brief Function that creates a mandatory delay required in some of the APIs.
 *
 * @param[in] period              : Delay in microseconds.
 * @param[in, out] intf_ptr       : Void pointer that can enable the linking of descriptors
 *                                  for interface related call backs
 *  @return void.
 *
 */
static void user_delay_us(uint32_t period, void *intf_ptr);

/*!
 *  @brief Function for reading the sensor's registers through I2C bus.
 *
 *  @param[in] reg_addr       : Register address.
 *  @param[out] data          : Pointer to the data buffer to store the read data.
 *  @param[in] len            : No of bytes to read.
 *  @param[in, out] intf_ptr  : Void pointer that can enable the linking of descriptors
 *                                  for interface related call backs.
 *
 *  @return Status of execution
 *
 *  @retval 0 -> Success
 *  @retval > 0 -> Failure Info
 *
 */
static int8_t user_i2c_read(uint8_t reg_addr, uint8_t *data, uint32_t len, void *intf_ptr);

/*!
 *  @brief Function for writing the sensor's registers through I2C bus.
 *
 *  @param[in] reg_addr       : Register address.
 *  @param[in] data           : Pointer to the data buffer whose value is to be written.
 *  @param[in] len            : No of bytes to write.
 *  @param[in, out] intf_ptr  : Void pointer that can enable the linking of descriptors
 *                                  for interface related call backs
 *
 *  @return Status of execution
 *
 *  @retval BME280_OK -> Success
 *  @retval BME280_E_COMM_FAIL -> Communication failure.
 *
 */
static int8_t user_i2c_write(uint8_t reg_addr, const uint8_t *data, uint32_t len, void *intf_ptr);

static int32_t i2c;
static int32_t once = 1;

static const char strName[] = "SENSOR_BME280";
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
		int driverAddress = 0x76;
		uint8_t settings_sel;

		/* Make sure to select BME280_I2C_ADDR_PRIM or BME280_I2C_ADDR_SEC as needed */
		id.dev_addr = BME280_I2C_ADDR_PRIM;

		/* Variable to define the result */
		int8_t rslt = BME280_OK;

		if ((id.fd = open(i2cFileName, O_RDWR)) < 0)
		{
			fprintf(stderr, "Failed to open the i2c bus %s\n", i2cFileName);
			return HAL_ERROR;
		}

		if (ioctl(id.fd, I2C_SLAVE, id.dev_addr) < 0)
		{
			fprintf(stderr, "Failed to acquire bus access and/or talk to slave.\n");
			return HAL_ERROR;
		}

		dev.intf = BME280_I2C_INTF;
		dev.read = user_i2c_read;
		dev.write = user_i2c_write;
		dev.delay_us = user_delay_us;

		/* Update interface pointer with the structure that contains both device address and file descriptor */
		dev.intf_ptr = &id;

		/* Initialize the bme280 */
		rslt = bme280_init(&dev);
		if (rslt != BME280_OK)
		{
			fprintf(stderr, "Failed to initialize the device (code %+d).\n", rslt);
			return HAL_ERROR;
		}
		/* Recommended mode of operation: Indoor navigation */
		dev.settings.osr_h = BME280_OVERSAMPLING_1X;
		dev.settings.osr_p = BME280_OVERSAMPLING_16X;
		dev.settings.osr_t = BME280_OVERSAMPLING_2X;
		dev.settings.filter = BME280_FILTER_COEFF_16;
		dev.settings.standby_time = BME280_STANDBY_TIME_62_5_MS;

		settings_sel = BME280_OSR_PRESS_SEL;
		settings_sel |= BME280_OSR_TEMP_SEL;
		settings_sel |= BME280_OSR_HUM_SEL;
		settings_sel |= BME280_STANDBY_SEL;
		settings_sel |= BME280_FILTER_SEL;
		rslt = bme280_set_sensor_settings(settings_sel, &dev);
		if (rslt != BME280_OK)
		{
			fprintf(stderr, "Failed to set sensor settings (code %+d).\n", rslt);
			return HAL_ERROR;
		}
		rslt = bme280_set_sensor_mode(BME280_NORMAL_MODE, &dev);
		if (rslt != BME280_OK)
		{
			fprintf(stderr, "Failed to set sensor mode (code %+d).\n", rslt);
			return HAL_ERROR;
		}
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

	simSen->obs = 0;
	simSen->errCode = 0;

	if (once == 0) {
		close(i2c);
		once = 1;
	}

	if (once) {
		char i2cFileName[] = "/dev/i2c-1";
		int driverAddress = 0x76;
		uint8_t settings_sel;

		/* Make sure to select BME280_I2C_ADDR_PRIM or BME280_I2C_ADDR_SEC as needed */
		id.dev_addr = BME280_I2C_ADDR_PRIM;

		/* Variable to define the result */
		int8_t rslt = BME280_OK;

		if ((id.fd = open(i2cFileName, O_RDWR)) < 0)
		{
			fprintf(stderr, "Failed to open the i2c bus %s\n", i2cFileName);
			return HAL_ERROR;
		}

		if (ioctl(id.fd, I2C_SLAVE, id.dev_addr) < 0)
		{
			fprintf(stderr, "Failed to acquire bus access and/or talk to slave.\n");
			return HAL_ERROR;
		}

		dev.intf = BME280_I2C_INTF;
		dev.read = user_i2c_read;
		dev.write = user_i2c_write;
		dev.delay_us = user_delay_us;

		/* Update interface pointer with the structure that contains both device address and file descriptor */
		dev.intf_ptr = &id;

		/* Initialize the bme280 */
		rslt = bme280_init(&dev);
		if (rslt != BME280_OK)
		{
			fprintf(stderr, "Failed to initialize the device (code %+d).\n", rslt);
			return HAL_ERROR;
		}
		/* Recommended mode of operation: Indoor navigation */
		dev.settings.osr_h = BME280_OVERSAMPLING_1X;
		dev.settings.osr_p = BME280_OVERSAMPLING_16X;
		dev.settings.osr_t = BME280_OVERSAMPLING_2X;
		dev.settings.filter = BME280_FILTER_COEFF_16;
		dev.settings.standby_time = BME280_STANDBY_TIME_62_5_MS;

		settings_sel = BME280_OSR_PRESS_SEL;
		settings_sel |= BME280_OSR_TEMP_SEL;
		settings_sel |= BME280_OSR_HUM_SEL;
		settings_sel |= BME280_STANDBY_SEL;
		settings_sel |= BME280_FILTER_SEL;
		rslt = bme280_set_sensor_settings(settings_sel, &dev);
		if (rslt != BME280_OK)
		{
			fprintf(stderr, "Failed to set sensor settings (code %+d).\n", rslt);
			return HAL_ERROR;
		}
		rslt = bme280_set_sensor_mode(BME280_NORMAL_MODE, &dev);
		if (rslt != BME280_OK)
		{
			fprintf(stderr, "Failed to set sensor mode (code %+d).\n", rslt);
			return HAL_ERROR;
		}
		once = 0;
	}
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

    int8_t rslt;
	struct bme280_data comp_data;

	/* Delay while the sensor completes a measurement */
	dev.delay_us(70, dev.intf_ptr);
	rslt = bme280_get_sensor_data(BME280_ALL, &comp_data, &dev);
	if (rslt != BME280_OK)
	{
		fprintf(stderr, "Failed to get sensor data (code %+d).\n", rslt);
		return HAL_ERROR;
	}
	simSen->valueList[0] = (HALFLOAT_T)(comp_data.temperature);
	simSen->valueList[1] = (HALFLOAT_T)(0.01 * comp_data.pressure);
	simSen->valueList[2] = (HALFLOAT_T)(comp_data.humidity);

	HALSENSOR_T *halSensor = (HALSENSOR_T *)pHalComponent;

	pCmd->num = 3;
	halSensor->valueList[0] = simSen->valueList[0];
	halSensor->valueList[1] = simSen->valueList[1];
	halSensor->valueList[2] = simSen->valueList[2];

	return HAL_OK;
}

static HALRETURNCODE_T fncGetTmValLst(HALCOMPONENT_T *pHalComponent,HAL_ARGUMENT_T *pCmd) { //uint32_t *pOutSize,HALFLOAT_T *pOutValLst,int32_t *pOutTime) {
#ifdef DEBUG
	printf("%s:%s\n", __FILE__, __FUNCTION__);
#endif
	time_t timeWk;
	int32_t idx = pHalComponent->halId.instanceId;
	SIM_SEN_T *simSen = &simSenAr[idx];

	int8_t rslt;
	struct bme280_data comp_data;

	/* Delay while the sensor completes a measurement */
	dev.delay_us(70, dev.intf_ptr);
	rslt = bme280_get_sensor_data(BME280_ALL, &comp_data, &dev);
	if (rslt != BME280_OK)
	{
		fprintf(stderr, "Failed to get sensor data (code %+d).\n", rslt);
		return HAL_ERROR;
	}
	simSen->valueList[0] = (HALFLOAT_T)(comp_data.temperature);
	simSen->valueList[1] = (HALFLOAT_T)(0.01 * comp_data.pressure);
	simSen->valueList[2] = (HALFLOAT_T)(comp_data.humidity);

	HALSENSOR_T *halSensor = (HALSENSOR_T *)pHalComponent;

	pCmd->num = 3;
	halSensor->valueList[0] = simSen->valueList[0];
	halSensor->valueList[1] = simSen->valueList[1];
	halSensor->valueList[2] = simSen->valueList[2];

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

HAL_FNCTBL_T HalSensorBME280Tbl = {
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

/*!
 * @brief This function reading the sensor's registers through I2C bus.
 */
int8_t user_i2c_read(uint8_t reg_addr, uint8_t *data, uint32_t len, void *intf_ptr)
{
	struct identifier id;

	id = *((struct identifier *)intf_ptr);

	write(id.fd, &reg_addr, 1);
	read(id.fd, data, len);

	return 0;
}

/*!
 * @brief This function provides the delay for required time (Microseconds) as per the input provided in some of the
 * APIs
 */
void user_delay_us(uint32_t period, void *intf_ptr)
{
	usleep(period);
}

/*!
 * @brief This function for writing the sensor's registers through I2C bus.
 */
int8_t user_i2c_write(uint8_t reg_addr, const uint8_t *data, uint32_t len, void *intf_ptr)
{
	uint8_t *buf;
	struct identifier id;

	id = *((struct identifier *)intf_ptr);

	buf = malloc(len + 1);
	buf[0] = reg_addr;
	memcpy(buf + 1, data, len);
	if (write(id.fd, buf, len + 1) < (uint16_t)len)
	{
		return BME280_E_COMM_FAIL;
	}

	free(buf);

	return BME280_OK;
}
