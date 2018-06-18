/*
 * openEL_registryConfig.c
 *
 *  Created on: 2018/05/19
 *      Author: OpenEL-WG
 */

#include "openEL.h"
#include "openEL_registry.h"

#include "openEL_SensorMPU9250.h"

const HAL_REG_T HalRegTbl[] = {
/*		DevKind		Vendor		Product			function table		component size								*/
		{0x0002,	0x00000005,	0x00000001,		&HalSensorRTUSB9AXISTbl,	sizeof(HALSENSOR_T)		},
};

const int32_t hal_szRegTbl = sizeof(HalRegTbl)/sizeof(HAL_REG_T);
