/*
 * openEL_registry.h
 *
 *  Created on: 2018/05/19
 *      Author: OpenEL-WG
 */

#ifndef OPENEL_REGISTRY_H_
#define OPENEL_REGISTRY_H_

typedef struct HalFncTbl_st HAL_FNCTBL_T;

/** HALComponent registry */
typedef struct HalReg_st {
	int32_t deviceKindID;
	int32_t vendorID;
	int32_t productID;
	HAL_FNCTBL_T *pFncTbl;
	int32_t szHalComponent;
} HAL_REG_T;
extern const HAL_REG_T HalRegTbl[]; /**< HALComponent registry table */
extern const int32_t hal_szRegTbl; /**< HALComponent registry table size */

/** general component --- function table */
typedef struct HalFncTbl_st {
	/* 0x00 */ HALRETURNCODE_T (*pFncInit)(HALCOMPONENT_T*,HAL_ARGUMENT_T*); /**< Initialize */ \
	/* 0x01 */ HALRETURNCODE_T (*pFncReInit)(HALCOMPONENT_T*,HAL_ARGUMENT_T*); /**< ReInit */ \
	/* 0x02 */ HALRETURNCODE_T (*pFncFinalize)(HALCOMPONENT_T*,HAL_ARGUMENT_T*); /**< Finalize */ \
	/* 0x03 */ HALRETURNCODE_T (*pFncAddObserver)(HALCOMPONENT_T*,HAL_ARGUMENT_T*); /**< AddObserver */ \
	/* 0x04 */ HALRETURNCODE_T (*pFncRemoveObserver)(HALCOMPONENT_T*,HAL_ARGUMENT_T*); /**< RemoveObserver */ \
	/* 0x05 */ HALRETURNCODE_T (*pFncGetProperty)(HALCOMPONENT_T*,HAL_ARGUMENT_T*); /**< GetProperty */ \
	/* 0x06 */ HALRETURNCODE_T (*pFncGetTime)(HALCOMPONENT_T*,HAL_ARGUMENT_T*); /**< GetTime */ \
	/* 0x07 */ HALRETURNCODE_T (*pFncDummy07)(HALCOMPONENT_T*,HAL_ARGUMENT_T*); /**< Reserved */

	/* 0x08 */ HALRETURNCODE_T (*pFncDummy08)(HALCOMPONENT_T*,HAL_ARGUMENT_T*); /**< Reserved */
	/* 0x09 */ HALRETURNCODE_T (*pFncDummy09)(HALCOMPONENT_T*,HAL_ARGUMENT_T*); /**< Reserved */
	/* 0x0A */ HALRETURNCODE_T (*pFncDummy0A)(HALCOMPONENT_T*,HAL_ARGUMENT_T*); /**< Reserved */
	/* 0x0B */ HALRETURNCODE_T (*pFncDummy0B)(HALCOMPONENT_T*,HAL_ARGUMENT_T*); /**< Reserved */
	/* 0x0C */ HALRETURNCODE_T (*pFncDummy0C)(HALCOMPONENT_T*,HAL_ARGUMENT_T*); /**< Reserved */
	/* 0x0D */ HALRETURNCODE_T (*pFncDummy0D)(HALCOMPONENT_T*,HAL_ARGUMENT_T*); /**< Reserved */
	/* 0x0E */ HALRETURNCODE_T (*pFncDummy0E)(HALCOMPONENT_T*,HAL_ARGUMENT_T*); /**< Reserved */
	/* 0x0F */ HALRETURNCODE_T (*pFncDummy0F)(HALCOMPONENT_T*,HAL_ARGUMENT_T*); /**< Reserved */

	/* 0x10 */ HALRETURNCODE_T (*pFncDummy10)(HALCOMPONENT_T*,HAL_ARGUMENT_T*); /**< Reserved */
	/* 0x11 */ HALRETURNCODE_T (*pFncDummy11)(HALCOMPONENT_T*,HAL_ARGUMENT_T*); /**< Reserved */
	/* 0x12 */ HALRETURNCODE_T (*pFncDummy12)(HALCOMPONENT_T*,HAL_ARGUMENT_T*); /**< Reserved */
	/* 0x13 */ HALRETURNCODE_T (*pFncDummy13)(HALCOMPONENT_T*,HAL_ARGUMENT_T*); /**< Reserved */
	/* 0x14 */ HALRETURNCODE_T (*pFncDummy14)(HALCOMPONENT_T*,HAL_ARGUMENT_T*); /**< Reserved */
	/* 0x15 */ HALRETURNCODE_T (*pFncDummy15)(HALCOMPONENT_T*,HAL_ARGUMENT_T*); /**< Reserved */
	/* 0x16 */ HALRETURNCODE_T (*pFncDummy16)(HALCOMPONENT_T*,HAL_ARGUMENT_T*); /**< Reserved */
	/* 0x17 */ HALRETURNCODE_T (*pFncDummy17)(HALCOMPONENT_T*,HAL_ARGUMENT_T*); /**< Reserved */

	/* 0x18 */ HALRETURNCODE_T (*pFncSetValue)(HALCOMPONENT_T *,HAL_ARGUMENT_T*); /**< SetValue */
	/* 0x19 */ HALRETURNCODE_T (*pFncGetValue)(HALCOMPONENT_T *,HAL_ARGUMENT_T*); /**< GetValue */
	/* 0x1A */ HALRETURNCODE_T (*pFncGetValueList)(HALCOMPONENT_T *halComponent,HAL_ARGUMENT_T*); /**< GetValueList */
	/* 0x1B */ HALRETURNCODE_T (*pFncGetTimedValueList)(HALCOMPONENT_T *halComponent,HAL_ARGUMENT_T*); /**< GetTimedValueList */
	/* 0x1C */ HALRETURNCODE_T (*pFncDeviceVendor1C)(HALCOMPONENT_T*,HAL_ARGUMENT_T*,HAL_ARGUMENT_DEVICE_T *); /**< Device Vendor Function */
	/* 0x1D */ HALRETURNCODE_T (*pFncDeviceVendor1D)(HALCOMPONENT_T*,HAL_ARGUMENT_T*,HAL_ARGUMENT_DEVICE_T *); /**< Device Vendor Function */
	/* 0x1E */ HALRETURNCODE_T (*pFncDeviceVendor1E)(HALCOMPONENT_T*,HAL_ARGUMENT_T*,HAL_ARGUMENT_DEVICE_T *); /**< Device Vendor Function */
	/* 0x1F */ HALRETURNCODE_T (*pFncDeviceVendor1F)(HALCOMPONENT_T*,HAL_ARGUMENT_T*,HAL_ARGUMENT_DEVICE_T *); /**< Device Vendor Function */
} HAL_FNCTBL_T;

#endif /* OPENEL_REGISTRY_H_ */
