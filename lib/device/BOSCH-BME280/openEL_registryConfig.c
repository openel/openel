/*
 * openEL_registryConfig.c
 *
 *  Created on: 2021/12/24
 *      Author: OpenEL-WG
 */

#include "openEL.h"
#include "openEL_registry.h"

#include "openEL_SensorBME280.h"

const HAL_REG_T HalRegTbl[] = {
/*		DevKind		Vendor		Product			function table		component size								*/
		{0x000E,	0x0000000C,	0x00000001,		&HalSensorBME280Tbl,	sizeof(HALSENSOR_T)},
};

const int32_t hal_szRegTbl = sizeof(HalRegTbl)/sizeof(HAL_REG_T);
