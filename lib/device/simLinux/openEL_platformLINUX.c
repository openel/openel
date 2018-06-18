/*
 * openEL_platforrmLINUX.c
 *
 *  Created on: 2018/05/13
 *      Author: OpenEL-WG
 */

#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <signal.h>
#include <pthread.h>
#include <string.h>

#include "openEL.h"
#include "openEL_platform.h"

//static void callbackEventTimer(dispatch_source_t _timerSource);
static void callbackEventTimer(timer_t *cb_timerSource);

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
	timer_t _timerID; // タイマーソース
	HALEVENTTIMER_T *halEventTimer;
} HAL_EVENTTIMER_PF_T;
static HAL_EVENTTIMER_PF_T *halEventTimerPF = 0;

static void
timerHandler( int sig, siginfo_t *si, void *uc )
{
    timer_t *tidp;
//		printf("%s\n",__FUNCTION__);
    tidp = si->si_value.sival_ptr;
    callbackEventTimer(tidp);
}

static int
makeTimer( char *name, timer_t *timerID, int expireMS, int intervalMS )
{
    struct sigevent         te;
    struct itimerspec       its;
    struct sigaction        sa;
    int                     sigNo = SIGRTMIN;
//		printf("%s:start\n",__FUNCTION__);

    /* Set up signal handler. */
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = timerHandler;
    sigemptyset(&sa.sa_mask);
    if (sigaction(sigNo, &sa, NULL) == -1)
    {
				printf("%s:Failed to setup signal handling for %s.\n", __FUNCTION__, name);
        return (1);
    }

    /* Set and enable alarm */
    te.sigev_notify = SIGEV_SIGNAL;
    te.sigev_signo = sigNo;
    te.sigev_value.sival_ptr = timerID;
    timer_create(CLOCK_REALTIME, &te, timerID);
//		printf("%s:timerID = %x\n",__FUNCTION__, timerID);
//		printf("%s:timerID = %x\n",__FUNCTION__, *timerID);

    its.it_interval.tv_sec = 0;
    its.it_interval.tv_nsec = intervalMS * 1000000;
    its.it_value.tv_sec = 0;
    its.it_value.tv_nsec = expireMS * 1000000;
    timer_settime(*timerID, 0, &its, NULL);
//		printf("%s:end\n",__FUNCTION__);

    return 0;
}

HALRETURNCODE_T HalEventTimerStartTimer_platform(HALEVENTTIMER_T *argHalEventTimer) {
	HALRETURNCODE_T retVal;
	HAL_EVENTTIMER_PF_T *wkHalEventTimerPF;
//	printf("%s:start\n",__FUNCTION__);
	wkHalEventTimerPF = malloc(sizeof(HAL_EVENTTIMER_PF_T));
	wkHalEventTimerPF->halEventTimer = argHalEventTimer;

	retVal = (HALRETURNCODE_T)makeTimer("First Timer", &(wkHalEventTimerPF->_timerID), 40, (argHalEventTimer->eventPeriod));
	if ( 1==retVal ) {
		free(wkHalEventTimerPF);
		retVal = HAL_ERROR;
	} else {
		halEventTimerPF = HalLinkedList_add(halEventTimerPF,wkHalEventTimerPF);
		retVal = HAL_OK;
	}

//	printf("%s:end\n",__FUNCTION__);
	return retVal;
}

/** タイムイベントの終了 */
HALRETURNCODE_T HalEventTimerStopTimer_platform(HALEVENTTIMER_T *argHalEventTimer) {
	HAL_EVENTTIMER_PF_T *wkHalEventTimerPF = halEventTimerPF;
	while ( 0 != wkHalEventTimerPF ) {
		if ( wkHalEventTimerPF->halEventTimer == argHalEventTimer ) {
// ベースタイマ破棄
			if(wkHalEventTimerPF->_timerID){
    		timer_delete(wkHalEventTimerPF->_timerID);
			}
			halEventTimerPF = HalLinkedList_remove(halEventTimerPF,wkHalEventTimerPF);
			free(wkHalEventTimerPF);
			return HAL_OK;
		}
	}
	return HAL_ERROR;
}

//static void callbackEventTimer(dispatch_source_t cb_timerSource) {
static void callbackEventTimer(timer_t *cb_timerSource) {
//	printf("%s, cb_timerSource=%x\n",__FUNCTION__, *cb_timerSource);
	HAL_EVENTTIMER_PF_T *wkHalEventTimerPF = halEventTimerPF;
	HALTIMEROBSERVER_T *wkObserverList;
//	printf("%s: &(wkHalEventTimerPF->_timerID=%x\n",__FUNCTION__,  wkHalEventTimerPF->_timerID);
	while ( 0 != wkHalEventTimerPF ) {
		if ( wkHalEventTimerPF->_timerID == *cb_timerSource ) {
			wkObserverList = wkHalEventTimerPF->halEventTimer->observerList;
			while ( 0 != wkObserverList ) {
				wkObserverList->notify_timer(wkHalEventTimerPF->halEventTimer);
				wkObserverList = HalLinkedList_getNext(wkObserverList);
			}
		}
		wkHalEventTimerPF = HalLinkedList_getNext(wkHalEventTimerPF);
	}
}
