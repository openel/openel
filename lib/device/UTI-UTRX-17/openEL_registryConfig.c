/*
 * openEL_registryConfig.c
 *
 *  Created on: 2018/05/19
 *      Author: OpenEL-WG
 */

#include "openEL.h"
#include "openEL_registry.h"

#include "openEL_simMotor1.h" /* デバイスベンダーから提供される定義ファイル */
#include "openEL_MotorPCA9685.h" /* デバイスベンダーから提供される定義ファイル */

const HAL_REG_T HalRegTbl[] = {
/*		DevKind		Vendor		Product			function table		component size								*/
		{0x0001,	0x00000000,	0x00000001,		&HalMotor001Tbl,	sizeof(HALACTUATOR_T)	},
		{0x0001,	0x00000002,	0x00000001,		&HalMotorPCA9685Tbl,	sizeof(HALACTUATOR_T)	},
};

const int32_t hal_szRegTbl = sizeof(HalRegTbl)/sizeof(HAL_REG_T);
