/**
 * @~japanese
 * @file openEL.c
 * @brief		OpenEL 共有部
 * @Version 3.0.0
 *
 * @~english
 * @file openEL.c
 * @brief		OpenEL Common File
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
#ifndef OPENEL_H_
#define OPENEL_H_

/* Includes */
/*---------------------------------------------------------------------------*/

#include <stdint.h>
#include "openEL.h"
#include "openEL_registry.h"
#include "openEL_platform.h"

//#include <stdio.h> POSIX互換I/Oなのでここでは使っていけない (システム依存性)
//#include <time.h> POSIX互換ライブラリなのでここでは使っていけない (システム依存性)

/* Macro (module scope) */
/*---------------------------------------------------------------------------*/
#define HAL_SZ_HANDLER_TBL	(256)
#define HAL_MSK_HANDLER_TBL	(HAL_SZ_HANDLER_TBL-1)

/* Variable (global scope) */
/*---------------------------------------------------------------------------*/



/* Variable (module scope) */
/*---------------------------------------------------------------------------*/
typedef struct HalHandler_st {
	uint32_t swUsed;
	HALCOMPONENT_T *pHalComponent;
	HAL_FNCTBL_T *pFncTbl;		/**< デバイスベンダー提供の関数テーブル */
} HAL_HANDLER_T;
HAL_HANDLER_T HalHandlerTbl[HAL_SZ_HANDLER_TBL];
static int32_t HalIdxHandlerTbl;

/* Function prototype (module scope) */
/*---------------------------------------------------------------------------*/

//static int32_t elInit_old(uint32_t handle,EL_CMN_FNC_TBL_T fncTbl,uint32_t compoIndex);
//static int32_t elGetNewHandle(void); /**< ポートIDの取得 */
//static EL_CMN_FNC_TBL_T elGetComponent(int32_t vendorID, int32_t compoID);

/* Functions */
/*---------------------------------------------------------------------------*/
/**
 * @~japanese
 * openELコンポーネントの初期化
 * @param[in]	HALCOMPONENT_T *halComponent
 * @return 0:正常 / 0以外-1以下:エラーコード
 *
 * @~english
 * Initialize the openEL component
 * @param[in]	HALCOMPONENT_T *halComponent
 * @return 0:OK / not 0 :error code
 */
HALCOMPONENT_T * HalCreate(int32_t vendorID,int32_t productID,int32_t instanceID) {
	int32_t i;
	int32_t idx = -1;
	HAL_HANDLER_T *pHandler;

	HALCOMPONENT_T *pHalComponent;
	const HAL_REG_T *pReg;
//	int32_t retVal;
//	EL_CMN_FNC_TBL_T pFncTbl;

	/* handler */
	idx = -1;
	for ( i=0; i<HAL_SZ_HANDLER_TBL; i++ ) {
		HalIdxHandlerTbl = (HalIdxHandlerTbl+1)&HAL_MSK_HANDLER_TBL;
		if ( 0 == HalHandlerTbl[HalIdxHandlerTbl].swUsed ) {
			idx = HalIdxHandlerTbl;
			break;
		}
	}
	if ( -1 == idx ) return 0;
	pHandler = &HalHandlerTbl[HalIdxHandlerTbl];

	/* scan function table / vendor , product */
	idx = -1;
	for ( i=0; i<hal_szRegTbl; i++ ) {
		if ( (vendorID == HalRegTbl[i].vendorID) && (productID == HalRegTbl[i].productID) ) {
			idx = i;
			break;
		}
	}
	if ( -1 == idx ) return 0;
	pReg = &HalRegTbl[idx];

	/* Initialize HalComponent */
	pHalComponent = (HALCOMPONENT_T *)HalMalloc( pReg->szHalComponent );
	pHalComponent->handle = HalIdxHandlerTbl;
	pHalComponent->halId.deviceKindId = pReg->deviceKindID;
	pHalComponent->halId.vendorId = vendorID;
	pHalComponent->halId.productId = productID;
	pHalComponent->halId.instanceId = instanceID;

	/* Initialize Handler member */
	pHandler->swUsed = 1;
	pHandler->pHalComponent = pHalComponent;
	pHandler->pFncTbl = pReg->pFncTbl;

	return pHalComponent;
}
/*---------------------------------------------------------------------------*/
void HalDestroy(HALCOMPONENT_T *halComponent) {
	HAL_HANDLER_T *pHandler;

	pHandler = &HalHandlerTbl[halComponent->handle];
	pHandler->swUsed = 0; /* not use */
	HalFree(halComponent);
}
/*---------------------------------------------------------------------------*/
/**
 * @~japanese
 * openELコンポーネントの初期化
 * @param[in]	HALCOMPONENT_T *halComponent
 * @return 0:正常 / 0以外-1以下:エラーコード
 *
 * @~english
 * Initialize the openEL component
 * @param[in]	HALCOMPONENT_T *halComponent
 * @return 0:OK / not 0 :error code
 */
HALRETURNCODE_T HalInit(HALCOMPONENT_T *halComponent) {
	HAL_HANDLER_T *pHandler;
	HAL_ARGUMENT_T cmd;

	pHandler = &HalHandlerTbl[halComponent->handle];
	return pHandler->pFncTbl->pFncInit(halComponent,&cmd);
}
/*---------------------------------------------------------------------------*/
/** OpenELデバイスのリセット */
HALRETURNCODE_T HalReInit(HALCOMPONENT_T *halComponent) {
	HAL_HANDLER_T *pHandler;
	HAL_ARGUMENT_T cmd;

	pHandler = &HalHandlerTbl[halComponent->handle];
	return pHandler->pFncTbl->pFncReInit(halComponent,&cmd);
}
/*---------------------------------------------------------------------------*/
/** OpenELデバイスの終了 */
HALRETURNCODE_T HalFinalize(HALCOMPONENT_T *halComponent) {
	HAL_HANDLER_T *pHandler;
	HAL_ARGUMENT_T cmd;

	pHandler = &HalHandlerTbl[halComponent->handle];
	return pHandler->pFncTbl->pFncFinalize(halComponent,&cmd);
}

HALRETURNCODE_T HalAddObserver(HALCOMPONENT_T *halComponent, HALOBSERVER_T *halObserver) {
	HAL_HANDLER_T *pHandler;
	HAL_ARGUMENT_T cmd;

	halComponent->observerList = HalLinkedList_add(halComponent->observerList,halObserver);
	pHandler = &HalHandlerTbl[halComponent->handle];
	return pHandler->pFncTbl->pFncAddObserver(halComponent,&cmd);

}

HALRETURNCODE_T HalRemoveObserver(HALCOMPONENT_T *halComponent, HALOBSERVER_T *halObserver) {
	HAL_HANDLER_T *pHandler;
	HAL_ARGUMENT_T cmd;

	halComponent->observerList = HalLinkedList_remove(halComponent->observerList,halObserver);
	pHandler = &HalHandlerTbl[halComponent->handle];
	return pHandler->pFncTbl->pFncRemoveObserver(halComponent,&cmd);
}

HALRETURNCODE_T HalGetProperty(HALCOMPONENT_T *halComponent, HALPROPERTY_T *pOutProperty) {
	HAL_HANDLER_T *pHandler;
	HAL_ARGUMENT_T cmd;
	HALRETURNCODE_T retCode;

	pHandler = &HalHandlerTbl[halComponent->handle];
	retCode = pHandler->pFncTbl->pFncGetProperty(halComponent,&cmd);
	pOutProperty->deviceName = halComponent->property->deviceName;
	pOutProperty->functionList = halComponent->property->functionList;
	pOutProperty->sizeFunctionList = halComponent->property->sizeFunctionList;
	return retCode;
}

HALRETURNCODE_T HalGetTime(HALCOMPONENT_T *halComponent, int32_t *pTime_value) {
#ifdef HAL_SW_NOT_COMPATIBLE_REL20180524_B
	HAL_HANDLER_T *pHandler;
	HAL_ARGUMENT_T cmd;
	HALRETURNCODE_T retCode;

	pHandler = &HalHandlerTbl[halComponent->handle];
	retCode = pHandler->pFncTbl->pFncGetTime(halComponent,&cmd); //pTime_value);
	*pTime_value = halComponent->time;
	return retCode;
#else /* HAL_SW_NOT_COMPATIBLE_REL20180524_B */
	HAL_HANDLER_T *pHandler;
	HAL_ARGUMENT_T cmd;
	HALRETURNCODE_T retCode;

	pHandler = &HalHandlerTbl[halComponent->handle];
	retCode = pHandler->pFncTbl->pFncGetTime(halComponent,&cmd); //pTime_value);
	*pTime_value = cmd.num;
	return retCode;
#endif  /* HAL_SW_NOT_COMPATIBLE_REL20180524_B */
}
/*---------------------------------------------------------------------------*/

/* EventTimer */
/*---------------------------------------------------------------------------*/
HALRETURNCODE_T HalEventTimerStartTimer(HALEVENTTIMER_T *eventTimer) {
	if ( 0 == eventTimer->eventPeriod ) {
		return HAL_ERROR;
	}
	return HalEventTimerStartTimer_platform(eventTimer);
	/* システム依存部分は openEL_platform□□□□ に集約する */
}

HALRETURNCODE_T HalEventTimerStopTimer(HALEVENTTIMER_T *eventTimer) {
	HalEventTimerStopTimer_platform(eventTimer);
	/* システム依存部分は openEL_platform□□□□ に集約する */
	return HAL_OK;
}
HALRETURNCODE_T HalEventTimerSetEventPeriod(HALEVENTTIMER_T *eventTimer, int32_t eventPeriod) {
	eventTimer->eventPeriod = eventPeriod;
	return HAL_OK;
}
/* Observer , EventTimer */
/*---------------------------------------------------------------------------*/

/* Observer */
/*---------------------------------------------------------------------------*/
HALRETURNCODE_T HalEventTimerAddObserver(HALEVENTTIMER_T *eventTimer, HALTIMEROBSERVER_T *timerObserver) {
	eventTimer->observerList = HalLinkedList_add( eventTimer->observerList , timerObserver );
	return HAL_OK;
}

HALRETURNCODE_T HalEventTimerRemoveObserver(HALEVENTTIMER_T *eventTimer, HALTIMEROBSERVER_T *timerObserver) {
	eventTimer->observerList =  HalLinkedList_remove( eventTimer->observerList , timerObserver );
	return HAL_OK;
}
/*---------------------------------------------------------------------------*/

/* Actuator */
/*---------------------------------------------------------------------------*/
HALRETURNCODE_T HalActuatorSetValue(HALCOMPONENT_T *halComponent, int32_t cmdID,HALFLOAT_T value) {
	HAL_HANDLER_T *pHandler;
	HAL_ARGUMENT_T cmd;

	cmd.FI.num = cmdID;
	cmd.FI.value = value;
	pHandler = &HalHandlerTbl[halComponent->handle];
	return pHandler->pFncTbl->pFncSetValue(halComponent,&cmd); //cmdID,value);

}
HALRETURNCODE_T HalActuatorGetValue(HALCOMPONENT_T *halComponent, int32_t cmdID,HALFLOAT_T *value) {
	HAL_HANDLER_T *pHandler;
	HAL_ARGUMENT_T cmd;
	HALRETURNCODE_T retCode;

	cmd.FI.num = cmdID;
	pHandler = &HalHandlerTbl[halComponent->handle];
	retCode = pHandler->pFncTbl->pFncGetValue(halComponent,&cmd); //cmdID,value);
	*value = cmd.FI.value;
	return retCode;
}
/*---------------------------------------------------------------------------*/

/* Sensor */
/*---------------------------------------------------------------------------*/
HALRETURNCODE_T HalSensorGetValueList(HALCOMPONENT_T *halComponent,int32_t *pOutSize, HALFLOAT_T *valueList) {
	HAL_HANDLER_T *pHandler;
	HAL_ARGUMENT_T cmd;
	HALRETURNCODE_T retCode;
	int32_t i;

	cmd.num = 0; /* 念のためクリアしておく , デバイス側の書き込み忘れ防止用 */
	pHandler = &HalHandlerTbl[halComponent->handle];
	retCode = pHandler->pFncTbl->pFncGetValueList(halComponent,&cmd); //pOutSize,valueList);
	*pOutSize = cmd.num;
	for ( i=0; i<cmd.num; i++ ) {
		valueList[i] = ((HALSENSOR_T *)halComponent)->valueList[i];
	}
	return retCode;
}
HALRETURNCODE_T HalSensorGetTimedValueList(HALCOMPONENT_T *halComponent,int32_t *pOutSize, HALFLOAT_T *valueList, int32_t *time) {
	HAL_HANDLER_T *pHandler;
	HAL_ARGUMENT_T cmd;
	HALRETURNCODE_T retCode;
	int32_t i;

	cmd.num = 0; /* 念のためクリアしておく , デバイス側の書き込み忘れ防止用 */
	pHandler = &HalHandlerTbl[halComponent->handle];
	retCode = pHandler->pFncTbl->pFncGetTimedValueList(halComponent,&cmd); //pOutSize,valueList);
	*pOutSize = cmd.num;
	for ( i=0; i<cmd.num; i++ ) {
		valueList[i] = ((HALSENSOR_T *)halComponent)->valueList[i];
	}
	*time = ((HALSENSOR_T *)halComponent)->time;
	return retCode;
}
/*---------------------------------------------------------------------------*/
#endif /* OPENEL_H_ */
