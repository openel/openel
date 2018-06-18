/*
 * openEL_platforrmMinGW.c
 *
 *  Created on: 2018/05/13
 *      Author: OpenEL-WG
 */

#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <dispatch/dispatch.h>

#include "openEL.h"
#include "openEL_platform.h"

static void callbackEventTimer(dispatch_source_t _timerSource);


/** create buf */
void *HalMalloc(uint32_t size) {
	void *pAdr;
	uint8_t *pWk;
	int32_t i;

	pAdr = malloc(size);
	pWk = (uint8_t *)pAdr;
	for ( i=0; i<size; i++ ) {
		*pWk++ = 0;
	}
	return pAdr;
}

void HalFree(void *pMem) {
	free(pMem);
}

typedef struct HalEventTimerPF_st {
	HAL_LINKED_LIST_HEAD
	int32_t timerID;
	dispatch_source_t _timerID; // タイマーソース
	HALEVENTTIMER_T *halEventTimer;
} HAL_EVENTTIMER_PF_T;
static HAL_EVENTTIMER_PF_T *halEventTimerPF = 0;

HALRETURNCODE_T HalEventTimerStartTimer_platform(HALEVENTTIMER_T *argHalEventTimer) {
	HALRETURNCODE_T retVal;
	int32_t wkTimerID;
	dispatch_source_t _timerSource; // タイマーソース
	HAL_EVENTTIMER_PF_T *wkHalEventTimerPF;

	wkHalEventTimerPF = malloc(sizeof(HAL_EVENTTIMER_PF_T));
	wkHalEventTimerPF->halEventTimer = argHalEventTimer;

// タイマーソース作成
_timerSource = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0));
dispatch_retain(_timerSource);
// タイマーキャンセルハンドラ設定
dispatch_source_set_cancel_handler(_timerSource, ^{
    if(_timerSource){
        dispatch_release(_timerSource); // releaseを忘れずに
//        _timerSource = NULL;
    }
});
// タイマーイベントハンドラ
dispatch_source_set_event_handler(_timerSource, ^{
    // ここに定期的に行う処理を記述
		callbackEventTimer(_timerSource);
});
// インターバル等を設定
//dispatch_source_set_timer(wkHalEventTimerPF->_timerSource, dispatch_time(DISPATCH_TIME_NOW, 0), NSEC_PER_SEC * 5, NSEC_PER_SEC / 2); // 直後に開始、5秒間隔で 0.5秒の揺らぎを許可
dispatch_source_set_timer(_timerSource, dispatch_time(DISPATCH_TIME_NOW, 0), NSEC_PER_MSEC * (argHalEventTimer->eventPeriod), 0); // 直後に開始、5秒間隔で 0.5秒の揺らぎを許可
// タイマー開始
dispatch_resume(_timerSource);

		wkHalEventTimerPF->_timerID = _timerSource;
		halEventTimerPF = HalLinkedList_add(halEventTimerPF,wkHalEventTimerPF);
		retVal = HAL_OK;
	return retVal;
}

/** タイムイベントの終了 */
HALRETURNCODE_T HalEventTimerStopTimer_platform(HALEVENTTIMER_T *argHalEventTimer) {
	HAL_EVENTTIMER_PF_T *wkHalEventTimerPF = halEventTimerPF;
	while ( 0 != wkHalEventTimerPF ) {
		if ( wkHalEventTimerPF->halEventTimer == argHalEventTimer ) {
// ベースタイマ破棄
			if(wkHalEventTimerPF->_timerID){
    		dispatch_source_cancel(wkHalEventTimerPF->_timerID);
			}
			halEventTimerPF = HalLinkedList_remove(halEventTimerPF,wkHalEventTimerPF);
			free(wkHalEventTimerPF);
			return HAL_OK;
		}
	}
	return HAL_ERROR;
}

static void callbackEventTimer(dispatch_source_t cb_timerSource) {
	HAL_EVENTTIMER_PF_T *wkHalEventTimerPF = halEventTimerPF;
	HALTIMEROBSERVER_T *wkObserverList;
	while ( 0 != wkHalEventTimerPF ) {
		if ( wkHalEventTimerPF->_timerID == cb_timerSource ) {
			wkObserverList = wkHalEventTimerPF->halEventTimer->observerList;
			while ( 0 != wkObserverList ) {
				wkObserverList->notify_timer(wkHalEventTimerPF->halEventTimer);
				wkObserverList = HalLinkedList_getNext(wkObserverList);
			}
		}
		wkHalEventTimerPF = HalLinkedList_getNext(wkHalEventTimerPF);
	}
}
