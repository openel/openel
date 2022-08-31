/*
 ============================================================================
 Name        : projSTM-298P.c
 Author      : OpenEL-WG
 Version     : 3.2
 Copyright   : Japan Embedded Systems Technology Association(JASA)
 Description : Sample program for STMicroelectronics L298P
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

HALCOMPONENT_T *halMotor01;
HALCOMPONENT_T *halMotor02;
HALCOMPONENT_T *halSensor01;
HALEVENTTIMER_T HalEvtTm100;
HALEVENTTIMER_T *halTvtTm100 = &HalEvtTm100;

void cbNotifyTimer101(HALEVENTTIMER_T *eventTimer);
void cbNotifyTimer102(HALEVENTTIMER_T *eventTimer);
HALTIMEROBSERVER_T tmObs101 = { { 0 }, cbNotifyTimer101 };
HALTIMEROBSERVER_T tmObs102 = { { 0 }, cbNotifyTimer102 };

int32_t event_count1, event_count2;
HALFLOAT_T velVal1, velVal2;

void outProperty(HALCOMPONENT_T *hC);

int main(void) {
	int32_t timeWk, size;
	uint8_t flgObs;

	printf("openEL Start\n");

	halMotor01  = HalCreate(0x00000001,0x0000000D,0x00000001,1); // Left Motor
	halMotor02  = HalCreate(0x00000001,0x0000000D,0x00000001,2); // Right Motor

	HalInit(halMotor01);
	HalInit(halMotor02);

	outProperty(halMotor01);
	outProperty(halMotor02);

	flgObs = 1;

	HalEventTimerSetEventPeriod(halTvtTm100,100);
	HalEventTimerAddObserver(halTvtTm100,&tmObs101);
	HalEventTimerAddObserver(halTvtTm100,&tmObs102);

	HalEventTimerStartTimer(halTvtTm100);

	while(1) {
		usleep(500000); /* 0.5s */
//		HalActuatorSetValue(halMotor01,HAL_REQUEST_VELOCITY_CONTROL,50);
//		HalActuatorSetValue(halMotor02,HAL_REQUEST_VELOCITY_CONTROL,0);
//		HalActuatorGetValue(halMotor01,HAL_REQUEST_VELOCITY_ACTUAL,&velVal1);
//		HalActuatorGetValue(halMotor02,HAL_REQUEST_VELOCITY_ACTUAL,&velVal2);
//		sleep(5); /* 5s */

		printf("event_count1:%5d ,event_count2:%5d, ", event_count1, event_count2);
		printf("Left Motor:%7.3lf, Right Motor:%7.3lf \n", velVal1, velVal2);
		fflush(stdout);
		if( 360 <= event_count1 ) break;
	}

	HalActuatorSetValue(halMotor01,HAL_REQUEST_NO_EXCITE,0);
	HalActuatorSetValue(halMotor02,HAL_REQUEST_NO_EXCITE,0);

	flgObs = 0;

	HalEventTimerStopTimer(halTvtTm100);
	HalEventTimerRemoveObserver(halTvtTm100,&tmObs101);
	HalEventTimerRemoveObserver(halTvtTm100,&tmObs102);

	HalFinalize(halMotor01);
	HalFinalize(halMotor02);

	HalDestroy(halMotor01);
	HalDestroy(halMotor02);

	printf("openEL End\n");
	return EXIT_SUCCESS;
}

void cbNotifyTimer101(HALEVENTTIMER_T *eventTimer) {
	HALFLOAT_T ph, velocity;
	event_count1++;
	ph = (HALFLOAT_T)event_count1/360*2*M_PI;
	velocity = 100 * sin(ph); // Between -100 and 100 [%]
	HalActuatorSetValue(halMotor01,HAL_REQUEST_VELOCITY_CONTROL,velocity);
	HalActuatorSetValue(halMotor02,HAL_REQUEST_VELOCITY_CONTROL,velocity);
}

void cbNotifyTimer102(HALEVENTTIMER_T *eventTimer) {
	event_count2++;
	HalActuatorGetValue(halMotor01,HAL_REQUEST_VELOCITY_ACTUAL,&velVal1);
	HalActuatorGetValue(halMotor02,HAL_REQUEST_VELOCITY_ACTUAL,&velVal2);
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
