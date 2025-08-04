/*
 ============================================================================
 Name        : projUTI-UTRX-17.c
 Author      : OpenEL-WG
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include "openEL.h"

HALCOMPONENT_T *halMotor01;
HALCOMPONENT_T *halMotor02;
//HALCOMPONENT_T *halSensor01;
HALEVENTTIMER_T HalEvtTm100;
HALEVENTTIMER_T *halTvtTm100 = &HalEvtTm100;
HALEVENTTIMER_T HalEvtTm200;
HALEVENTTIMER_T *halTvtTm200 = &HalEvtTm200;


void cbNotifyTimer101(HALEVENTTIMER_T *eventTimer);
void cbNotifyTimer102(HALEVENTTIMER_T *eventTimer);
void cbNotifyTimer201(HALEVENTTIMER_T *eventTimer);
HALTIMEROBSERVER_T tmObs101 = { { 0 }, cbNotifyTimer101 };
HALTIMEROBSERVER_T tmObs102 = { { 0 }, cbNotifyTimer102 };
HALTIMEROBSERVER_T tmObs201 = { { 0 }, cbNotifyTimer201 };

int32_t event_count1,event_count2,event_count3;
HALFLOAT_T simVal1,simVal2,simVal3,simVal4;
int32_t tmSensor;

static void notify_event201a(HALCOMPONENT_T *halComponent, int32_t eventID) {
	printf("notify_event201a : %d\n",eventID);
}
static void notify_error201a(HALCOMPONENT_T *halComponent, int32_t errorID) {
	printf("notify_error201a : %d\n",errorID);
}
static void notify_event201b(HALCOMPONENT_T *halComponent, int32_t eventID) {
	printf("notify_event201b : %d\n",eventID);
}
static void notify_error201b(HALCOMPONENT_T *halComponent, int32_t errorID) {
	printf("notify_error201b : %d\n",errorID);
	HalReInit(halComponent);
}

HALOBSERVER_T halObs201a = { {0},notify_event201a,notify_error201a };
HALOBSERVER_T halObs201b = { {0},notify_event201b,notify_error201b };

void outProperty(HALCOMPONENT_T *hC);


int main(void) {
	int32_t timeWk;
	uint8_t flgObs;

	printf("openEL Start\n");

	halMotor01  = HalCreate(0x00000001,0x00000000,0x00000001,0); // simMotor
	halMotor02  = HalCreate(0x00000001,0x00000002,0x00000001,1); // UTI-UTRX-17
//	halMotor02  = HalCreate(0x00000001,0x00000000,0x00000002,5);
//	halSensor01 = HalCreate(0x00000002,0x00000000,0x00000010,0);

	HalInit(halMotor01);
	HalInit(halMotor02);
//	HalInit(halSensor01);

	outProperty(halMotor01);
	outProperty(halMotor02);
//	outProperty(halSensor01);

	printf("motor01  getTime ret=%d\n", HalGetTime(halMotor01,&timeWk) );
	printf("motor02  getTime ret=%d\n", HalGetTime(halMotor02,&timeWk) );
//	printf("sensor01 getTime ret=%d\n", HalGetTime(halSensor01,&timeWk) );
//	printf("Sensor time = %d\n",timeWk);

	HalAddObserver(halMotor01,&halObs201a);
	HalAddObserver(halMotor01,&halObs201b);
	flgObs = 1;

	HalEventTimerSetEventPeriod(halTvtTm100,100);
	HalEventTimerAddObserver(halTvtTm100,&tmObs101);
	HalEventTimerAddObserver(halTvtTm100,&tmObs102);
	HalEventTimerStartTimer(halTvtTm100);

//	HalEventTimerSetEventPeriod(halTvtTm200,200);
//	HalEventTimerAddObserver(halTvtTm200,&tmObs201);
//	HalEventTimerStartTimer(halTvtTm200);
	// ループ
	while(1) {
		usleep(500000); /* 0.5s */
		printf("timer %6d , %5d , %6d : ",event_count1,event_count2,event_count3);
		printf("%7.3lf %7.3lf %7.3lf %7.3lf %5d(tmSen)\n",simVal1,simVal2,simVal3,simVal4,tmSensor);
		fflush(stdout);
		if ( (150 <= event_count1) && (1==flgObs) ) {
			HalRemoveObserver(halMotor01,&halObs201a);
			HalRemoveObserver(halMotor01,&halObs201b);
			flgObs = 0;
		}
		if( 300 <= event_count1 ) break;
	}
//	HalGetTime(halSensor01,&timeWk);
//	printf("Sensor time = %d\n",timeWk);

	HalEventTimerStopTimer(halTvtTm100);
	HalEventTimerRemoveObserver(halTvtTm100,&tmObs101);

//	HalEventTimerStopTimer(halTvtTm200);
//	HalEventTimerRemoveObserver(halTvtTm200,&tmObs201);

	HalFinalize(halMotor01);
	HalFinalize(halMotor02);
//	HalFinalize(halSensor01);

	HalDestroy(halMotor01);
	HalDestroy(halMotor02);
//	HalDestroy(halSensor01);

	printf("openEL End\n");
	return EXIT_SUCCESS;
}

HALFLOAT_T posX, posY;

void cbNotifyTimer101(HALEVENTTIMER_T *eventTimer) {
//	HALFLOAT_T posX, posY;
	double ph;
	int32_t idxPh;

	event_count1++;
	idxPh = event_count1 % 150;
	if ( idxPh <= 100 ) {
		ph = ((double)idxPh * 2.0 *M_PI)/100.0;
	} else {
		ph = ((double)100 * 2.0 *M_PI)/100.0;
	}
	posX = 10.0*(1.0-cos(ph));
//	posY = 10.0*(sin(ph));
	if ( event_count1 < 120 ) {
		posY = posY + 1.0;
	}else if ( event_count1 >= 120 && event_count1 < 240) {
		posY = posY - 1.0;
		if (posY < 0) posY = 0;
	}else if ( event_count1 >= 240) {
		posY = posY + 1.0;
	}

	HalActuatorSetValue(halMotor01,HAL_REQUEST_POSITION_CONTROL,posX);
	HalActuatorSetValue(halMotor02,HAL_REQUEST_POSITION_CONTROL,posY);
//	HalSetValue(halMotor01,HAL_REQUEST_POSITIONE_CONTROL,posX);
//	HalSetValue(halMotor02,HAL_REQUEST_POSITIONE_CONTROL,posY);
}

void cbNotifyTimer102(HALEVENTTIMER_T *eventTimer) {
	event_count2 += 10;
	HalActuatorGetValue(halMotor01,HAL_REQUEST_POSITION_ACTUAL,&simVal1);
	HalActuatorGetValue(halMotor02,HAL_REQUEST_POSITION_ACTUAL,&simVal2);
//	HalGetValue(halMotor01,HAL_REQUEST_POSITIONE_ACUTUAL,&simVal1);
//	HalGetValue(halMotor02,HAL_REQUEST_POSITIONE_ACUTUAL,&simVal2);
}

void cbNotifyTimer201(HALEVENTTIMER_T *eventTimer) {
	int32_t size;
	event_count3 += 1;
//	HalSensorGetValueList(halSensor01,&size,&simVal3);
//	HalSensorGetTimedValueList(halSensor01,&size,&simVal4,&tmSensor);
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
