#ifndef HAL4RT_H
#define HAL4RT_H

#ifdef __cplusplus
namespace hal {
extern "C" {
#endif /* __cplusplus */

#include <stdint.h>

typedef enum HalReturnCode_et {
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

typedef struct HalLinkedList_st {
	struct HalLinkedList_st *pNext;
} HAL_LINKED_LIST_T;

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

typedef struct HalObserver_st {
	HAL_LINKED_LIST_HEAD
	void (*notify_event)(HALCOMPONENT_T *halComponent, int32_t eventId);
	void (*notify_error)(HALCOMPONENT_T *halComponent, int32_t errorId);
} HALOBSERVER_T;

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

struct HalEventTimer_st;
typedef struct HalTimerObserver_st {
	HAL_LINKED_LIST_HEAD
	void (*notify_timer)(struct HalEventTimer_st *eventTimer);
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
	/* HALCOMPONENT */
	HALCOMPONENT_BASE_MEMBER
	/* ACTUATOR */
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
	/* HALCOMPONENT */
	HALCOMPONENT_BASE_MEMBER
	/* SENSOR */
	HALFLOAT_T *valueList;
} HALSENSOR_T;

HALRETURNCODE_T HalSensorGetValueList(HALCOMPONENT_T *halComponent,int32_t *size, HALFLOAT_T *valueList);
HALRETURNCODE_T HalSensorGetTimedValueList(HALCOMPONENT_T *halComponent,int32_t *size, HALFLOAT_T *valueList, int32_t *timeValue);

#ifdef __cplusplus
} /* extern "C" */
} /* namespace hal */
#endif /* __cplusplus */

#endif /* HAL4RT_H */
