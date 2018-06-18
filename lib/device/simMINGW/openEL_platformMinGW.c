/*
 * openEL_platforrmMinGW.c
 *
 *  Created on: 2018/05/13
 *      Author: OpenEL-WG
 */

#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h> /* ライブラリ winmm をリンクする */

#include "openEL.h"
#include "openEL_platform.h"

static void CALLBACK callbackEventTimer(unsigned int timerID, unsigned int msg, unsigned int usrParam, unsigned int dw1, unsigned int dw2);


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
	HALEVENTTIMER_T *halEventTimer;
} HAL_EVENTTIMER_PF_T;
static HAL_EVENTTIMER_PF_T *halEventTimerPF = 0;

HALRETURNCODE_T HalEventTimerStartTimer_platform(HALEVENTTIMER_T *argHalEventTimer) {
	HALRETURNCODE_T retVal;
	int32_t wkTimerID;
	HAL_EVENTTIMER_PF_T *wkHalEventTimerPF;

	wkHalEventTimerPF = malloc(sizeof(HAL_EVENTTIMER_PF_T));
	wkHalEventTimerPF->halEventTimer = argHalEventTimer;

	wkTimerID = timeSetEvent(
			argHalEventTimer->eventPeriod,
			1,
			(LPTIMECALLBACK)callbackEventTimer,
			(DWORD) NULL,
			TIME_PERIODIC);
	if ( 0==wkTimerID ) {
		free(wkHalEventTimerPF);
		retVal = HAL_ERROR;
	} else {
		wkHalEventTimerPF->timerID = wkTimerID;
		halEventTimerPF = HalLinkedList_add(halEventTimerPF,wkHalEventTimerPF);
		retVal = HAL_OK;
	}
	return retVal;
}

/** タイムイベントの終了 */
HALRETURNCODE_T HalEventTimerStopTimer_platform(HALEVENTTIMER_T *argHalEventTimer) {
	HAL_EVENTTIMER_PF_T *wkHalEventTimerPF = halEventTimerPF;
	while ( 0 != wkHalEventTimerPF ) {
		if ( wkHalEventTimerPF->halEventTimer == argHalEventTimer ) {
			timeKillEvent(wkHalEventTimerPF->timerID);
			halEventTimerPF = HalLinkedList_remove(halEventTimerPF,wkHalEventTimerPF);
			free(wkHalEventTimerPF);
			return HAL_OK;
		}
	}
	return HAL_ERROR;
}

static void CALLBACK callbackEventTimer(unsigned int cbTimerID, unsigned int msg, unsigned int usrParam, unsigned int dw1, unsigned int dw2) {
	HAL_EVENTTIMER_PF_T *wkHalEventTimerPF = halEventTimerPF;
	HALTIMEROBSERVER_T *wkObserverList;
	while ( 0 != wkHalEventTimerPF ) {
		if ( wkHalEventTimerPF->timerID == cbTimerID ) {
			wkObserverList = wkHalEventTimerPF->halEventTimer->observerList;
			while ( 0 != wkObserverList ) {
				wkObserverList->notify_timer(wkHalEventTimerPF->halEventTimer);
				wkObserverList = HalLinkedList_getNext(wkObserverList);
			}
		}
		wkHalEventTimerPF = HalLinkedList_getNext(wkHalEventTimerPF);
	}
}
