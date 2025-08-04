/*
 * openEL_registryConfig.c
 *
 *  Created on: 2021/12/24
 *      Author: OpenEL-WG
 */

#include "openEL.h"
#include "openEL_registry.h"

#include "openEL_CO2SensorSCD41.h"
#include "openEL_SensorBME280.h"
#include "openEL_MotorL298P.h"

const HAL_REG_T HalRegTbl[] = {
/*		DevKind		Vendor		Product			function table		component size								*/
		{0x000A,	0x00000007,	0x00000002,		&HalSensorSCD41Tbl,		sizeof(HALSENSOR_T)},
		{0x000E,	0x0000000C,	0x00000001,		&HalSensorBME280Tbl,	sizeof(HALSENSOR_T)},
		{0x0001,	0x0000000D,	0x00000001,		&HalMotorL298PTbl,	sizeof(HALSENSOR_T)},
};

const int32_t hal_szRegTbl = sizeof(HalRegTbl)/sizeof(HAL_REG_T);
