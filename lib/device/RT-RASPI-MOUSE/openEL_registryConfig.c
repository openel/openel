/*
 * openEL_registryConfig.c
 *
 *  Created on: 2025/03/15
 *      Author: OpenEL-WG
 */

#include "openEL.h"
#include "openEL_registry.h"

#include "openEL_MotorRaspiMouse.h"
#include "openEL_SensorRaspiMouse.h"

const HAL_REG_T HalRegTbl[] = {
/*		DevKind		Vendor		Product			function table		component size								*/
		{0x0001,	0x00000005,	0x00000002,		&HalMotorRaspiMouseTbl,	sizeof(HALACTUATOR_T)},
		{0x000B,	0x00000005,	0x00000002,		&HalSensorRaspiMouseTbl,sizeof(HALSENSOR_T)},
};

const int32_t hal_szRegTbl = sizeof(HalRegTbl)/sizeof(HAL_REG_T);
