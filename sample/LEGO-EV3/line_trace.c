/*
 ============================================================================
 Name        : projLEGO-EV3.c
 Author      : OpenEL-WG
 Version     : 3.2
 Copyright   : Japan Embedded Systems Technology Association(JASA)
 Description : Sample Line Trace program for LEGO EV3
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include "openEL.h"

HALCOMPONENT_T *halMotor01;
HALCOMPONENT_T *halMotor02;
HALCOMPONENT_T *halSensor01;
HALCOMPONENT_T *halSensor02;
HALCOMPONENT_T *halSensor03;
HALCOMPONENT_T *halSensor04;
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
HALFLOAT_T simVal1,simVal2,simVal3,simVal4,simVal5,simVal6,simVal7,simVal8,simVal9,simVal10;
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
	int32_t size;

	printf("openEL Start\n");

	halMotor01  = HalCreate(0x00000001,0x00000009,0x00000001,1); // LEGO-EV3-L-Motor Right Wheel
	halMotor02  = HalCreate(0x00000001,0x00000009,0x00000001,2); // LEGO-EV3-L-Motor Left Wheel
	halSensor01 = HalCreate(0x0000000B,0x00000009,0x00000001,1); // LEGO-EV3-Color
	halSensor02 = HalCreate(0x0000000C,0x00000009,0x00000001,1); // LEGO-EV3-Touch
	halSensor03 = HalCreate(0x00000006,0x00000009,0x00000001,1); // LEGO-EV3-Distance
	halSensor04 = HalCreate(0x00000002,0x00000009,0x00000001,1); // LEGO-EV3-Gyro

	HalInit(halMotor01);
	HalInit(halMotor02);
	HalInit(halSensor01);
	HalInit(halSensor02);
	HalInit(halSensor03);
	HalInit(halSensor04);

	outProperty(halMotor01);
	outProperty(halMotor02);
	outProperty(halSensor01);
	outProperty(halSensor02);
	outProperty(halSensor03);
	outProperty(halSensor04);

	printf("motor01  getTime ret=%d\n", HalGetTime(halMotor01,&timeWk) );
	printf("motor02  getTime ret=%d\n", HalGetTime(halMotor02,&timeWk) );
	printf("sensor01 getTime ret=%d\n", HalGetTime(halSensor01,&timeWk) );
	printf("sensor02 getTime ret=%d\n", HalGetTime(halSensor02,&timeWk) );
	printf("sensor03 getTime ret=%d\n", HalGetTime(halSensor03,&timeWk) );
	printf("sensor04 getTime ret=%d\n", HalGetTime(halSensor04,&timeWk) );
	printf("Sensor time = %d\n",timeWk);

	HalAddObserver(halMotor01,&halObs201a);
	HalAddObserver(halMotor01,&halObs201b);
	flgObs = 1;

	while(simVal5==0){
		usleep(500000); /* 0.5s */
		HalSensorGetValueList(halSensor02,&size,&simVal5);
	}

	HalEventTimerSetEventPeriod(halTvtTm100,100);
	HalEventTimerAddObserver(halTvtTm100,&tmObs101);
	HalEventTimerAddObserver(halTvtTm100,&tmObs102);
	HalEventTimerStartTimer(halTvtTm100);

	HalEventTimerSetEventPeriod(halTvtTm200,200);
	HalEventTimerAddObserver(halTvtTm200,&tmObs201);
	HalEventTimerStartTimer(halTvtTm200);

	while(1) {
		usleep(500000); /* 0.5s */
		printf("timer %6d , %5d , %6d : ",event_count1,event_count2,event_count3);
		printf("%7.3lf %7.3lf %7.3lf %7.3lf %7.3lf %7.3lf %7.3lf %7.3lf %7.3lf %7.3lf %5d(tmSen)\n",simVal1,simVal2,simVal3,simVal4,simVal5,simVal6,simVal7,simVal8,simVal9,simVal10,tmSensor);
		fflush(stdout);
		if ( (150 <= event_count1) && (1==flgObs) ) {
			HalRemoveObserver(halMotor01,&halObs201a);
			HalRemoveObserver(halMotor01,&halObs201b);
			flgObs = 0;
		}
		if( 300 <= event_count1 || 300 <= event_count3 ) {
			HalActuatorSetValue(halMotor01,HAL_REQUEST_VELOCITY_CONTROL,0);
			HalActuatorSetValue(halMotor02,HAL_REQUEST_VELOCITY_CONTROL,0);
			break;
		}
		if ( simVal7 < 100 ) {
			HalActuatorSetValue(halMotor01,HAL_REQUEST_VELOCITY_CONTROL,0);
			HalActuatorSetValue(halMotor02,HAL_REQUEST_VELOCITY_CONTROL,0);
		}
	}

	HalEventTimerStopTimer(halTvtTm100);
	HalEventTimerRemoveObserver(halTvtTm100,&tmObs101);
	HalEventTimerRemoveObserver(halTvtTm100,&tmObs102);

	HalEventTimerStopTimer(halTvtTm200);
	HalEventTimerRemoveObserver(halTvtTm200,&tmObs201);

	HalFinalize(halMotor01);
	HalFinalize(halMotor02);
	HalFinalize(halSensor01);
	HalFinalize(halSensor02);
	HalFinalize(halSensor03);
	HalFinalize(halSensor04);

	HalDestroy(halMotor01);
	HalDestroy(halMotor02);
	HalDestroy(halSensor01);
	HalDestroy(halSensor02);
	HalDestroy(halSensor03);
	HalDestroy(halSensor04);

	printf("openEL End\n");
	return EXIT_SUCCESS;
}

void cbNotifyTimer101(HALEVENTTIMER_T *eventTimer) {
	HALFLOAT_T velocityLeft, velocityRight;

	event_count1++;

	if ( simVal3 < 5 ) {
		velocityLeft = 150;
		velocityRight = 50;
	} else if ( 5 <= simVal3 && simVal3 < 10 ) {
		velocityLeft = 100;
		velocityRight = 50;
	} else if ( 10 <= simVal3 && simVal3 < 13 ) {
		velocityLeft = 100;
		velocityRight = 80;
	} else if ( 13 <= simVal3 && simVal3 < 15 ) {
		velocityLeft = 100;
		velocityRight = 100;
	} else if ( 15 <= simVal3 && simVal3 < 17 ) {
		velocityLeft = 80;
		velocityRight = 100;
	} else if ( 17 <= simVal3 && simVal3 < 20 ) {
		velocityLeft = 50;
		velocityRight = 100;
	} else if ( 20 <= simVal3 ) {
		velocityLeft = 50;
		velocityRight = 150;
	}

	HalActuatorSetValue(halMotor01,HAL_REQUEST_VELOCITY_CONTROL,velocityRight);
	HalActuatorSetValue(halMotor02,HAL_REQUEST_VELOCITY_CONTROL,velocityLeft);
}

void cbNotifyTimer102(HALEVENTTIMER_T *eventTimer) {
	event_count2 += 10;
	HalActuatorGetValue(halMotor01,HAL_REQUEST_VELOCITY_ACTUAL,&simVal1);
	HalActuatorGetValue(halMotor02,HAL_REQUEST_VELOCITY_ACTUAL,&simVal2);
}

void cbNotifyTimer201(HALEVENTTIMER_T *eventTimer) {
	int32_t size;
	event_count3 += 1;
	HalSensorGetValueList(halSensor01,&size,&simVal3);
	HalSensorGetValueList(halSensor02,&size,&simVal5);
	HalSensorGetValueList(halSensor03,&size,&simVal7);
	HalSensorGetValueList(halSensor04,&size,&simVal9);
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

