/*
 * openEL_registryConfig.c
 *
 *  Created on: 2025/05/05
 *      Author: OpenEL-WG
 */

#include "openEL.h"
#include "openEL_registry.h"

#include "openEL_SensorSeeedRotaryAngle.h"

const HAL_REG_T HalRegTbl[] = {
/*		DevKind		Vendor		Product			function table		component size								*/
		{0x000E,	0x0000000E,	0x00000001,		&HalSensorSeeedRotaryAngleTbl,sizeof(HALSENSOR_T)},
};

const int32_t hal_szRegTbl = sizeof(HalRegTbl)/sizeof(HAL_REG_T);
