/*
 ============================================================================
 Name        : color.c
 Author      : OpenEL-WG
 Version     : 3.2
 Copyright   : Japan Embedded Systems Technology Association(JASA)
 Description : Sample program for LEGO EV3 Color sensor
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "openEL.h"

HALCOMPONENT_T *halSensor01;
HALEVENTTIMER_T HalEvtTm200;
HALEVENTTIMER_T *halTvtTm200 = &HalEvtTm200;


void cbNotifyTimer201(HALEVENTTIMER_T *eventTimer);
HALTIMEROBSERVER_T tmObs201 = { { 0 }, cbNotifyTimer201 };

int32_t event_count1;
HALFLOAT_T value_list[4];
int32_t tmSensor;

static void notify_event201a(HALCOMPONENT_T *halComponent, int32_t eventID) {
	printf("notify_event201a : %d\n",eventID);
}
static void notify_error201a(HALCOMPONENT_T *halComponent, int32_t errorID) {
	printf("notify_error201a : %d\n",errorID);
}

HALOBSERVER_T halObs201a = { {0},notify_event201a,notify_error201a };

void outProperty(HALCOMPONENT_T *hC);


int main(void) {
	int32_t timeWk;

	printf("openEL Start\n");

	halSensor01 = HalCreate(0x0000000B,0x00000009,0x00000001,1); // LEGO-EV3-Color

	HalInit(halSensor01);

	outProperty(halSensor01);

	printf("sensor01 getTime ret=%d\n", HalGetTime(halSensor01,&timeWk) );
	printf("Sensor time = %d\n",timeWk);

	HalEventTimerSetEventPeriod(halTvtTm200,200);
	HalEventTimerAddObserver(halTvtTm200,&tmObs201);
	HalEventTimerStartTimer(halTvtTm200);

	while(1) {
		usleep(500000); /* 0.5s */
		printf("timer %6d : ",event_count1);
		printf("%7.3lf %4d %4d %4d %5d(tmSen)\n",value_list[0],value_list[1],value_list[2],value_list[3],tmSensor);
		fflush(stdout);
		if( 300 <= event_count1 ) break;
	}
	HalGetTime(halSensor01,&timeWk);
	printf("Sensor time = %d\n",timeWk);

	HalEventTimerStopTimer(halTvtTm200);
	HalEventTimerRemoveObserver(halTvtTm200,&tmObs201);

	HalFinalize(halSensor01);

	HalDestroy(halSensor01);

	printf("openEL End\n");
	return EXIT_SUCCESS;
}

void cbNotifyTimer201(HALEVENTTIMER_T *eventTimer) {
	int32_t size;
	event_count1 += 1;
	HalSensorGetValueList(halSensor01,&size,value_list);
//	HalSensorGetTimedValueList(halSensor01,&size,value_list,&tmSensor);
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

