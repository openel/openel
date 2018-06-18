/**
 * @~japanese
 * @file openEL.h
 * @brief		OpenEL 共通ヘッダーファイル
 * @Version 3.0.0
 *
 * @~english
 * @file openEL.h
 * @brief		OpenEL Common Header File
 * @Version 3.0.0
 */
/*

Copyright (c) 2017,2018 Japan Embedded Systems Technology Association(JASA)
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    Neither the name of the Association nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef OPENEL_H
#define OPENEL_H

#ifdef __cplusplus
namespace el {
extern "C" {
#endif /* __cplusplus */

//#define HAL_SW_FLOAT_SIZE 1

#include <stdint.h>
#include <HAL4RT.h>
#include "openEL_linkedList.h"

/* request code */
#define HAL_REQUEST_NO_EXCITE			(0)
#define HAL_REQUEST_POSITIONE_COMMAND	(HAL_REQUEST_POSITIONE_CONTROL)
#define HAL_REQUEST_VELOVITY_COMMAND	(HAL_REQUEST_VELOVITY_CONTROL)
#define HAL_REQUEST_TORQUE_COMMAND		(HAL_REQUEST_TORQUE_CONTROL)
//										(4) /* Reserved */
#define HAL_REQUEST_POSITIONE_ACUTUAL	(5)
#define HAL_REQUEST_VELOVITY_ACUTUAL	(6)
#define HAL_REQUEST_TORQUE_ACUTUAL		(7)

/** Command Interface between surface layer and device layer*/
typedef union halArgument_ut {
	int64_t numI64; /**<  64bit=8Byte , 32bit=4Byte */
	int32_t num;
#if HAL_SW_FLOAT_SIZE
	struct { HALFLOAT_T value; int32_t _dummy; int32_t num; } FI; /**< get/setValue Command Interface */
#else
	struct { HALFLOAT_T value; int32_t num; } FI; /**< get/setValue Command Interface */
#endif

} HAL_ARGUMENT_T;

typedef union halArgumentDevice_ut HAL_ARGUMENT_DEVICE_T;

/* OpenEL追加  HALACTUATOR  */
/*---------------------------------------------------------------------------*/
/**
 * @~japanese
 * HALコンポーネントの生成
 * @param[in]	vendorID
 * @param[in]	productID
 * @param[in]	instanceID
 * @return HALCOMPONENT * 生成されたHALComponentのポインタ
 *
 * @~english
 * HAL Component Creating
 * @param[in]	vendorID
 * @param[in]	productID
 * @param[in]	instanceID
 * @return HALCOMPONENT *address of created HAL Component
 */
HALCOMPONENT_T * HalCreate(int32_t vendorID,int32_t productID,int32_t instanceID);
/** HALComponent destroy */
void HalDestroy(HALCOMPONENT_T *halComponent);

/*---------------------------------------------------------------------------*/
#ifdef __cplusplus
} /* extern "C" */
} /* namespace el */
#endif /* __cplusplus */

#endif /* OPENEL_H */
