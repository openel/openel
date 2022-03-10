/*
 * openEL_registryConfig.c
 *
 *  Created on: 2022/02/01
 *      Author: OpenEL-WG
 */

#include "openEL.h"
#include "openEL_registry.h"

#include "openEL_CO2SensorSCD30.h"

const HAL_REG_T HalRegTbl[] = {
/*		DevKind		Vendor		Product			function table		component size								*/
		{0x000A,	0x00000007,	0x00000001,		&HalSensorSCD30Tbl,	sizeof(HALSENSOR_T)},
};

const int32_t hal_szRegTbl = sizeof(HalRegTbl)/sizeof(HAL_REG_T);
