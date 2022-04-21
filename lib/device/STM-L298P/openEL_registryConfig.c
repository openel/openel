/*
 * openEL_registryConfig.c
 *
 *  Created on: 2022/4/12
 *      Author: OpenEL-WG
 */

#include "openEL.h"
#include "openEL_registry.h"

#include "openEL_MotorL298P.h"

const HAL_REG_T HalRegTbl[] = {
/*		DevKind		Vendor		Product			function table		component size								*/
		{0x0001,	0x0000000D,	0x00000001,		&HalMotorL298PTbl,	sizeof(HALSENSOR_T)},
};

const int32_t hal_szRegTbl = sizeof(HalRegTbl)/sizeof(HAL_REG_T);
