/**
 * @~japanese
 * @file openEL.h
 * @brief		OpenEL 共通ヘッダーファイル
 * @Version 3.1.0
 *
 * @~english
 * @file openEL.h
 * @brief		OpenEL Common Header File
 * @Version 3.1.0
 */

#ifndef OPENEL_H
#define OPENEL_H

#ifdef __cplusplus
namespace hal {
extern "C" {
#endif /* __cplusplus */

//#define HAL_SW_FLOAT_SIZE 1

#include <stdint.h>
#include "HAL4RT.h"
#include "openEL_linkedList.h"

#define OPENEL_MAJOR      3
#define OPENEL_MINOR      1
#define OPENEL_PATCHLEVEL 0
#define OPENEL_VERSION "OpenEL 3.1.0"

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
} /* namespace hal */
#endif /* __cplusplus */

#endif /* OPENEL_H */
