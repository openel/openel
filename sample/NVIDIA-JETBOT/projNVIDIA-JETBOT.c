/*
 ============================================================================
 Name        : projNVIDIA-JETBOT.c
 Author      : OpenEL-WG
 Version     : 3.2
 Copyright   : Japan Embedded Systems Technology Association(JASA)
 Description : Sample program for NVIDIA JetBot
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
HALEVENTTIMER_T HalEvtTm200;
HALEVENTTIMER_T *halTvtTm200 = &HalEvtTm200;

void cbNotifyTimer101(HALEVENTTIMER_T *eventTimer);
void cbNotifyTimer102(HALEVENTTIMER_T *eventTimer);
void cbNotifyTimer201(HALEVENTTIMER_T *eventTimer);
HALTIMEROBSERVER_T tmObs101 = { { 0 }, cbNotifyTimer101 };
HALTIMEROBSERVER_T tmObs102 = { { 0 }, cbNotifyTimer102 };
HALTIMEROBSERVER_T tmObs201 = { { 0 }, cbNotifyTimer201 };

int32_t event_count1, event_count2, event_count3;
HALFLOAT_T velVal1, velVal2, value_list[4];
HALFLOAT_T shunt_voltage, load_voltage, power, current;

static void notify_event201a(HALCOMPONENT_T *halComponent, int32_t eventID) {
	printf("notify_event201a : %d\n",eventID);
	if(eventID == 1)
		printf("Charging.\n");
	if(eventID == 2)
		printf("Charging completed.\n");	
	if(eventID == 3)
		printf("The voltage is less than 10V.\n");	
}

static void notify_error201a(HALCOMPONENT_T *halComponent, int32_t errorID) {
	printf("notify_error201a : %d\n",errorID);
	if(errorID == 201)
		printf("The voltage is less than 9V. Connect JetBot to a power source.\n");	
}

HALOBSERVER_T halObs201a = { {0},notify_event201a,notify_error201a };

void outProperty(HALCOMPONENT_T *hC);

int main(void) {
	int32_t timeWk, size;
	uint8_t flgObs;

	printf("openEL Start\n");

	halMotor01  = HalCreate(0x00000001,0x0000000B,0x00000001,1); // Left Motor
	halMotor02  = HalCreate(0x00000001,0x0000000B,0x00000001,2); // Right Motor
	halSensor01 = HalCreate(0x0000000D,0x0000000B,0x00000001,1); // Power(INA219)

	HalInit(halMotor01);
	HalInit(halMotor02);
	HalInit(halSensor01);

	outProperty(halMotor01);
	outProperty(halMotor02);
	outProperty(halSensor01);

	printf("sensor01 getTime ret=%d\n", HalGetTime(halSensor01,&timeWk) );
	printf("Sensor time = %d\n",timeWk);

	HalAddObserver(halSensor01,&halObs201a);
	flgObs = 1;

	HalEventTimerSetEventPeriod(halTvtTm100,100);
	HalEventTimerAddObserver(halTvtTm100,&tmObs101);
	HalEventTimerAddObserver(halTvtTm100,&tmObs102);

	HalEventTimerSetEventPeriod(halTvtTm200,900);
	HalEventTimerAddObserver(halTvtTm200,&tmObs201);

	HalEventTimerStartTimer(halTvtTm100);
	HalEventTimerStartTimer(halTvtTm200);

	while(1) {
		usleep(500000); /* 0.5s */
		printf("timer %5d , %5d , %5d: ", event_count1, event_count2, event_count3);
		printf("%7.3lf %7.3lf ", velVal1, velVal2);
		printf("%6.3lf %6.3lf %6.3lf %6.3lf\n", shunt_voltage, load_voltage, power, current);
		fflush(stdout);
		if( 360 <= event_count1 ) break;
	}

	HalGetTime(halSensor01,&timeWk);
	printf("Sensor time = %d\n",timeWk);

	HalActuatorSetValue(halMotor01,HAL_REQUEST_NO_EXCITE,0);
	HalActuatorSetValue(halMotor02,HAL_REQUEST_NO_EXCITE,0);

	HalRemoveObserver(halSensor01,&halObs201a);
	flgObs = 0;

	HalEventTimerStopTimer(halTvtTm100);
	HalEventTimerStopTimer(halTvtTm200);
	HalEventTimerRemoveObserver(halTvtTm100,&tmObs101);
	HalEventTimerRemoveObserver(halTvtTm100,&tmObs102);
	HalEventTimerRemoveObserver(halTvtTm200,&tmObs201);

	HalFinalize(halMotor01);
	HalFinalize(halMotor02);
	HalFinalize(halSensor01);

	HalDestroy(halMotor01);
	HalDestroy(halMotor02);
	HalDestroy(halSensor01);

	printf("openEL End\n");
	return EXIT_SUCCESS;
}

void cbNotifyTimer101(HALEVENTTIMER_T *eventTimer) {
	HALFLOAT_T ph, velocity;
	event_count1++;
	ph = (HALFLOAT_T)event_count1/360*2*M_PI;
	velocity = 15 * M_PI * sin(ph); // Between -15*M_PI and 15*M_PI [rad/s]
	HalActuatorSetValue(halMotor01,HAL_REQUEST_VELOCITY_CONTROL,velocity);
	HalActuatorSetValue(halMotor02,HAL_REQUEST_VELOCITY_CONTROL,velocity);
}

void cbNotifyTimer102(HALEVENTTIMER_T *eventTimer) {
	event_count2++;
	HalActuatorGetValue(halMotor01,HAL_REQUEST_VELOCITY_ACTUAL,&velVal1);
	HalActuatorGetValue(halMotor02,HAL_REQUEST_VELOCITY_ACTUAL,&velVal2);
}

void cbNotifyTimer201(HALEVENTTIMER_T *eventTimer) {
	int32_t size;
	event_count3++;
	HalSensorGetValueList(halSensor01,&size,value_list);
#ifdef DEBUG
	printf("Shunt Voltage: %6.3f V\n", value_list[0]/1000); /* mV -> V */
	printf("LOAD Voltage: %6.3f V\n", value_list[1]);
	printf("PSU Voltage: %6.3f V\n", value_list[1] + value_list[0]/1000);
	printf("Power: %6.3f W\n", value_list[2]);
	printf("Current: %9.6f A\n", value_list[3]/1000); /* mA -> A */
#endif
	shunt_voltage = value_list[0]/1000; /* [mV] -> [V] */
	load_voltage = value_list[1]; /* [V] */
	power = value_list[2]; /* [W] */
	current = value_list[3]/1000; /* [mA] -> [A] */
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
