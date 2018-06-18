/*
 * openEL_registryConfig.c
 *
 *  Created on: 2018/05/19
 *      Author: OpenEL-WG
 */

#include "openEL.h"
#include "openEL_registry.h"

#include "openEL_simMotor1.h" /* デバイスベンダーから提供される定義ファイル */
#include "openEL_simMotor2.h" /* デバイスベンダーから提供される定義ファイル */
#include "openEL_simSensor1.h" /* デバイスベンダーから提供される定義ファイル */

/** デバイスを登録する */
const HAL_REG_T HalRegTbl[] = {
/*		DevKind		Vendor		Product			function table		component size								*/
		{0x0001,	0x00000000,	0x00000001,		&HalMotor001Tbl,	sizeof(HALACTUATOR_T)	},
		{0x0001,	0x00000000,	0x00000002,		&HalMotor002Tbl, 	sizeof(HALACTUATOR_T)	},
		{0x0002,	0x00000000,	0x00000010,		&HalGyroSen001Tbl,	sizeof(HALSENSOR_T)		},
};

const int32_t hal_szRegTbl = sizeof(HalRegTbl)/sizeof(HAL_REG_T);


