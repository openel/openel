/*
 * openEL_registryConfig.c
 *
 *  Created on: 2021/04/17
 *      Author: OpenEL-WG
 */

#include "openEL.h"
#include "openEL_registry.h"

#include "openEL_MotorEV3.h"
#include "openEL_GyroSensorEV3.h"
#include "openEL_DistanceSensorEV3.h"
#include "openEL_ColorSensorEV3.h"
#include "openEL_TouchSensorEV3.h"

const HAL_REG_T HalRegTbl[] = {
/*	DevKind		Vendor		Product		function table		component size	*/
	{0x0001,	0x00000009,	0x00000001,	&HalMotorEV3Tbl,	sizeof(HALACTUATOR_T)	},
	{0x0002,	0x00000009,	0x00000001,	&HalGyroSensorEV3Tbl,	sizeof(HALSENSOR_T)	},
	{0x0006,	0x00000009,	0x00000001,	&HalDistanceSensorEV3Tbl,	sizeof(HALSENSOR_T)	},
	{0x000B,	0x00000009,	0x00000001,	&HalColorSensorEV3Tbl,	sizeof(HALSENSOR_T)	},
	{0x000C,	0x00000009,	0x00000001,	&HalTouchSensorEV3Tbl,	sizeof(HALSENSOR_T)	},
};

const int32_t hal_szRegTbl = sizeof(HalRegTbl)/sizeof(HAL_REG_T);

