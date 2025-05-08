/*
 ============================================================================
 Name        : projSEEED-ROTARY-ANGLE.c
 Author      : OpenEL-WG
 Version     : 3.2
 Copyright   : Japan Embedded Systems Technology Association(JASA)
 Description : Sample program for Seeed Grove Rotary Angle sensor
 ============================================================================
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include "openEL.h"

#ifndef M_PI
#define M_PI           3.14159265358979323846  /* pi */
#endif

HALCOMPONENT_T *halSensor01;
HALEVENTTIMER_T HalEvtTm200;
HALEVENTTIMER_T *halTvtTm200 = &HalEvtTm200;

void cbNotifyTimer201(HALEVENTTIMER_T *eventTimer);
HALTIMEROBSERVER_T tmObs201 = { { 0 }, cbNotifyTimer201 };

int32_t event_count1, event_count2, event_count3;
HALFLOAT_T value_list[1];

static void notify_event201a(HALCOMPONENT_T *halComponent, int32_t eventID) {
	printf("notify_event201a : %d\n",eventID);
}

static void notify_error201a(HALCOMPONENT_T *halComponent, int32_t errorID) {
	printf("notify_error201a : %d\n",errorID);
}

HALOBSERVER_T halObs201a = { {0},notify_event201a,notify_error201a };

void outProperty(HALCOMPONENT_T *hC);

int main(void) {
	int32_t timeWk, size;
	uint8_t flgObs;

	printf("openEL Start\n");

	halSensor01 = HalCreate(0x0000000E,0x0000000E,0x00000001,1); // Rotary Angle Sensor

	HalInit(halSensor01);

	outProperty(halSensor01);

	printf("sensor01 getTime ret=%d\n", HalGetTime(halSensor01,&timeWk) );
	printf("Sensor time = %d\n",timeWk);

	HalAddObserver(halSensor01,&halObs201a);
	flgObs = 1;

	HalEventTimerSetEventPeriod(halTvtTm200,500);
	HalEventTimerAddObserver(halTvtTm200,&tmObs201);

	HalEventTimerStartTimer(halTvtTm200);

	while(1) {
		usleep(500000); /* 0.5s */
		printf("Timer: %5d , %5d , %5d  ", event_count1, event_count2, event_count3);
		printf("Rotary Angle %6.3f degrees\n", value_list[0]);
		fflush(stdout);
		if( 60 <= event_count3 ) break;
	}

	HalGetTime(halSensor01,&timeWk);
	printf("Sensor time = %d\n",timeWk);

	HalRemoveObserver(halSensor01,&halObs201a);
	flgObs = 0;

	HalEventTimerStopTimer(halTvtTm200);
	HalEventTimerRemoveObserver(halTvtTm200,&tmObs201);

	HalFinalize(halSensor01);

	HalDestroy(halSensor01);

	printf("openEL End\n");
	return EXIT_SUCCESS;
}

void cbNotifyTimer201(HALEVENTTIMER_T *eventTimer) {
	int32_t size;
	event_count3++;
	HalSensorGetValueList(halSensor01,&size,value_list);
}

void outProperty(HALCOMPONENT_T *hC) {
	int32_t i;
	HALPROPERTY_T propertyWk , *property;

	property = &propertyWk;
	HalGetProperty(hC,&propertyWk);
	printf("PROPERTY - Name : %s\n",property->deviceName);
	for ( i=0; i<property->sizeFunctionList; i++ ) {
		printf("PROPERTY - fnc%02X : %s\n",i,property->functionList[i]);
	}
}
