/*
 ============================================================================
 Name        : free_run.c
 Author      : OpenEL-WG
 Version     : 3.2
 Copyright   : Japan Embedded Systems Technology Association(JASA)
 Description : Sample program for RT Raspberry Pi Mouse
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

static void notify_event201a(HALCOMPONENT_T *halComponent, int32_t eventID) {
	printf("notify_event201a : %d\n",eventID);
}

static void notify_error201a(HALCOMPONENT_T *halComponent, int32_t errorID) {
	printf("notify_error201a : %d\n",errorID);
}

HALOBSERVER_T halObs201a = { {0},notify_event201a,notify_error201a };

HALFLOAT_T velocity = 0;
HALFLOAT_T velocity_l = 0;
HALFLOAT_T velocity_r = 0;
HALFLOAT_T value;

void outProperty(HALCOMPONENT_T *hC);

int main(void) {
	int32_t timeWk, size;
	uint8_t flgObs;

	printf("openEL Start\n");

	halMotor01  = HalCreate(0x00000001,0x00000005,0x00000002,1); // Left Motor
	halMotor02  = HalCreate(0x00000001,0x00000005,0x00000002,2); // Right Motor
	halSensor01 = HalCreate(0x0000000B,0x00000005,0x00000002,1); // Light Sensor

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
//	HalEventTimerAddObserver(halTvtTm100,&tmObs102);

	HalEventTimerSetEventPeriod(halTvtTm200,100);
	HalEventTimerAddObserver(halTvtTm200,&tmObs201);

	HalEventTimerStartTimer(halTvtTm100);
	HalEventTimerStartTimer(halTvtTm200);

	while(1) {
		usleep(500000); /* 0.5s */
		printf("Timer: %5d , %5d , %5d  ", event_count1, event_count2, event_count3);
		printf("Velocity[rad/s] L:%7.3lf R:%7.3lf  ", velVal1, velVal2);
		printf("Light sensor R:%6.3f RF:%6.3f LF:%6.3f L:%6.3f\n", value_list[0], value_list[1], value_list[2], value_list[3]);
		fflush(stdout);
		if( 180 <= event_count1 ) break;
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
//	HalEventTimerRemoveObserver(halTvtTm100,&tmObs102);
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
	event_count1++;
	HalActuatorSetValue(halMotor01,HAL_REQUEST_VELOCITY_CONTROL,velocity_l);
	HalActuatorSetValue(halMotor02,HAL_REQUEST_VELOCITY_CONTROL,velocity_r);
}

void cbNotifyTimer102(HALEVENTTIMER_T *eventTimer) {
	event_count2++;
	HalActuatorGetValue(halMotor01,HAL_REQUEST_VELOCITY_COMMAND,&velVal1);
	HalActuatorGetValue(halMotor02,HAL_REQUEST_VELOCITY_COMMAND,&velVal2);
}

void cbNotifyTimer201(HALEVENTTIMER_T *eventTimer) {
	int32_t size=0;
	int32_t r_thr=0;  // Right Threshold
	int32_t rf_thr=5; // Right Front Threshold
	int32_t lf_thr=5; // Left Front Threshold
	int32_t l_thr=0;  // Left Threshold
	event_count3++;
	velocity = 2*M_PI;
	velocity_l = velocity;
	velocity_r = velocity;
	HalSensorGetValueList(halSensor01,&size,value_list);
	if(value_list[0]<10 && value_list[3]<5){
		if(value_list[1]<rf_thr && value_list[2]<lf_thr){
			velocity_l = velocity;
			velocity_r = velocity;
		}
		else if(value_list[1]>=rf_thr && value_list[2]<lf_thr){ //Right Front
			velocity_l = velocity * 0.8;
			velocity_r = velocity;
		}
		else if(value_list[2]>=lf_thr && value_list[1]<rf_thr){ //Left Front
			velocity_l = velocity;
			velocity_r = velocity * 0.8;
		}
		else if(value_list[2]>=lf_thr && value_list[1]>=rf_thr){
			if(value_list[2] - value_list[1] > 0){
				velocity_l = velocity;
				velocity_r = velocity * 0.8;
			} else {
				velocity_l = velocity * 0.8;
				velocity_r = velocity;
			}
		} 
	}

	if(value_list[0]>=10 && value_list[0]<20 || value_list[3]>=5 && value_list[3]<10 ){ // Right and Left
		velocity = velocity * 0.9;
		if(value_list[1]<rf_thr && value_list[2]<lf_thr){
			velocity_l = velocity;
			velocity_r = velocity;
		}
		else if(value_list[1]>=rf_thr && value_list[2]<lf_thr){ //Right Front
			velocity_l = velocity * 0.7;
			velocity_r = velocity;
		}
		else if(value_list[2]>=lf_thr && value_list[1]<rf_thr){ //Left Front
			velocity_l = velocity;
			velocity_r = velocity * 0.7;
		}
		else if(value_list[2]>=lf_thr && value_list[1]>=rf_thr){
			if(value_list[2] - value_list[1] > 0){
				velocity_l = velocity;
				velocity_r = velocity * 0.7;
			} else {
				velocity_l = velocity * 0.7;
				velocity_r = velocity;
			}
		} 
	}

	if(value_list[0]>=20 && value_list[0]<100 || value_list[3]>=10 && value_list[3]<15 ){ // Right and Left
		velocity = velocity * 0.8;
		if(value_list[1]<rf_thr && value_list[2]<lf_thr){
			velocity_l = velocity;
			velocity_r = velocity;
		}
		else if(value_list[1]>=rf_thr && value_list[2]<lf_thr){ //Right Front
			velocity_l = velocity * 0.6;
			velocity_r = velocity;
		}
		else if(value_list[2]>=lf_thr && value_list[1]<rf_thr){ //Left Front
			velocity_l = velocity;
			velocity_r = velocity * 0.6;
		}
		else if(value_list[2]>=lf_thr && value_list[1]>=rf_thr){
			if(value_list[2] - value_list[1] > 0){
				velocity_l = velocity;
				velocity_r = velocity * 0.6;
			} else {
				velocity_l = velocity * 0.6;
				velocity_r = velocity;
			}
		} 
	}

	if(value_list[0]>=100){
		velocity = velocity * 0.8;
		if(value_list[3]<5){
			velocity_l = 0;
			velocity_r = velocity;
		} else {
			velocity_l = 0;
			velocity_r = 0;		
		}
	}	

	if(value_list[3]>=15){
		velocity = velocity * 0.8;
		if(value_list[0]<100){
			velocity_l = velocity;
			velocity_r = 0;
		} else {
			velocity_l = 0;
			velocity_r = 0;		
		}
	}	

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
