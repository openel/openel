/** @file HAL4RT.h */
/*

Copyright (c) 2017,2018 Japan Embedded Systems Technology Association(JASA)
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    Neither the name of the Association nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef HAL4RT_H_
#define HAL4RT_H_


#ifdef __cplusplus
namespace el {
extern "C" {
#endif /* __cplusplus */

#include <stdint.h>

/* Macro (global scope) */
/*---------------------------------------------------------------------------*/

#define OPENEL_MAJOR      3
#define OPENEL_MINOR      1
#define OPENEL_PATCHLEVEL 0
#define OPENEL_VERSION "OpenEL 3.1.0"

/* C PSM */
typedef enum HALReturnCode_et {
	HAL_OK = 0,
	HAL_ERROR
} HALRETURNCODE_T;

/* typedef definition */
typedef float  float32_t;
typedef double float64_t;
#if HAL_SW_FLOAT_SIZE
  typedef float32_t HALFLOAT_T;
#else
  typedef float64_t HALFLOAT_T;
#endif

/** LinkedList */
typedef struct HalLinkedList_st {
	struct HalLinkedList_st *pNext;
} HAL_LINKED_LIST_T;

/** Macro of Linked List Head Member */
#define HAL_LINKED_LIST_HEAD HAL_LINKED_LIST_T linkedList;

typedef struct HalID_st {
	int32_t deviceKindId;
	int32_t vendorId;
	int32_t productId;
	int32_t instanceId;
} HALID_T;

typedef struct HalProperty_st {
	char *deviceName;
	char **functionList;
	int32_t sizeFunctionList;
} HALPROPERTY_T;

typedef struct HalComponent_st HALCOMPONENT_T;

typedef struct HALObserver {
	HAL_LINKED_LIST_HEAD // OpenEL追加
	void (*notify_event)(HALCOMPONENT_T *halComponent, int32_t eventId);
	void (*notify_error)(HALCOMPONENT_T *halComponent, int32_t errorId);
} HALOBSERVER_T;

//typedef struct HALObserver HALOBSERVER_T;

#define HALCOMPONENT_BASE_MEMBER \
	int32_t handle; \
	HALID_T halId;\
	HALPROPERTY_T *property;\
	HALOBSERVER_T *observerList;\
	int32_t time;

typedef struct HalComponent_st {
	HALCOMPONENT_BASE_MEMBER
} HALCOMPONENT_T;

HALRETURNCODE_T HalInit(HALCOMPONENT_T *halComponent);
HALRETURNCODE_T HalReInit(HALCOMPONENT_T *halComponent);
HALRETURNCODE_T HalFinalize(HALCOMPONENT_T *halComponent);
HALRETURNCODE_T HalAddObserver(HALCOMPONENT_T *halComponent, HALOBSERVER_T *halObserver);
HALRETURNCODE_T HalRemoveObserver(HALCOMPONENT_T *halComponent, HALOBSERVER_T *halObserver);
HALRETURNCODE_T HalGetProperty(HALCOMPONENT_T *halComponent, HALPROPERTY_T *property);
HALRETURNCODE_T HalGetTime(HALCOMPONENT_T *halComponent, int32_t *timeValue);

typedef struct HalEventTimer_st HALEVENTTIMER_T;
typedef struct HalTimerObserver_st {
	HAL_LINKED_LIST_HEAD // OpenEL追加
	void (*notify_timer)(HALEVENTTIMER_T *eventTimer);
} HALTIMEROBSERVER_T;

typedef struct HalEventTimer_st {
	HALTIMEROBSERVER_T *observerList;
	int32_t eventPeriod;
} HALEVENTTIMER_T;


HALRETURNCODE_T HalEventTimerStartTimer(HALEVENTTIMER_T *eventTimer);
HALRETURNCODE_T HalEventTimerStopTimer(HALEVENTTIMER_T *eventTimer);
HALRETURNCODE_T HalEventTimerSetEventPeriod(HALEVENTTIMER_T *eventTimer, int32_t eventPeriod);
HALRETURNCODE_T HalEventTimerAddObserver(HALEVENTTIMER_T *eventTimer, HALTIMEROBSERVER_T *timerObserver);
HALRETURNCODE_T HalEventTimerRemoveObserver(HALEVENTTIMER_T *eventTimer, HALTIMEROBSERVER_T *timerObserver);

/** HalActuator */
typedef struct HalActuator_st {
	HALCOMPONENT_BASE_MEMBER
	HALFLOAT_T *valueList;
} HALACTUATOR_T;

/* request code */
#define HAL_REQUEST_POSITIONE_CONTROL	(1)
#define HAL_REQUEST_VELOVITY_CONTROL	(2)
#define HAL_REQUEST_TORQUE_CONTROL		(3)
HALRETURNCODE_T HalActuatorSetValue(HALCOMPONENT_T *halComponent, int32_t request, HALFLOAT_T value);
HALRETURNCODE_T HalActuatorGetValue(HALCOMPONENT_T *halComponent, int32_t request, HALFLOAT_T *value);

/** HalSensor */
typedef struct HalSensor_st {
	HALCOMPONENT_BASE_MEMBER
	HALFLOAT_T *valueList;
} HALSENSOR_T;

HALRETURNCODE_T HalSensorGetValueList(HALCOMPONENT_T *halComponent,int32_t *size, HALFLOAT_T *valueList);
HALRETURNCODE_T HalSensorGetTimedValueList(HALCOMPONENT_T *halComponent,int32_t *size, HALFLOAT_T *valueList, int32_t *time);

/* The end of C PSM */
#endif /* HAL4RT_H_ */
