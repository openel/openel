/*
 ============================================================================
 Name        : projJASA-DEMO.c
 Author      : OpenEL-WG
 Version     : 3.2
 Copyright   : Japan Embedded Systems Technology Association(JASA)
 Description : JASA DEMO program
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include "openEL.h"

HALCOMPONENT_T *halSensor01;
HALCOMPONENT_T *halSensor02;
HALCOMPONENT_T *halMotor01;
HALCOMPONENT_T *halMotor02;

int32_t event_count;
HALFLOAT_T value_list[4];

static void notify_event201a(HALCOMPONENT_T *halComponent, int32_t eventID) {
	printf("notify_event201a : %d\n",eventID);
	if(eventID == 1) { /* CO2 > 1000[ppm] */
		printf("Warning:Ventilation is required.\n");
	}
	if(eventID == 2) { /* CO2 > 1500[ppm] */
		printf("Danger:Ventilate now!\n");
	}
}

static void notify_error201a(HALCOMPONENT_T *halComponent, int32_t errorID) {
	printf("notify_error201a : %d\n",errorID);
	if(errorID == 201)
		printf("Measurment error has occurred.\n");	
}

HALOBSERVER_T halObs201a = { {0},notify_event201a,notify_error201a };

void outProperty(HALCOMPONENT_T *hC);

int main(void) {
	int32_t timeWk, size;
	uint8_t flgObs;

	printf("openEL Start\n");

	halSensor01 = HalCreate(0x0000000A,0x00000007,0x00000002,1); // Sensirion SCD30
	halSensor02 = HalCreate(0x0000000E,0x0000000C,0x00000001,1); // BOSCH BME280
	halMotor01 = HalCreate(0x00000001,0x0000000D,0x00000001,1); // STM L298P
	halMotor02 = HalCreate(0x00000001,0x0000000D,0x00000001,2); // STM L298P

	HalInit(halSensor01);
	HalInit(halSensor02);
	HalInit(halMotor01);
	HalInit(halMotor02);

	outProperty(halSensor01);
	outProperty(halSensor02);
	outProperty(halMotor01);
	outProperty(halMotor02);

	printf("sensor01 getTime ret=%d\n", HalGetTime(halSensor01,&timeWk) );
	printf("Sensor time = %d\n",timeWk);
	printf("sensor02 getTime ret=%d\n", HalGetTime(halSensor02,&timeWk) );
	printf("Sensor time = %d\n",timeWk);

	HalAddObserver(halSensor01,&halObs201a);
	flgObs = 1;

	while(1) {
		sleep(2); /* 2[s] */
		if(HalSensorGetValueList(halSensor01,&size,value_list) == HAL_ERROR){
			printf("Error HalSensorGetValueList()\n");
			continue;
		}
		printf("timer:%3d ", event_count);
		printf("CO2:%7.2lf[ppm] Temp:%5.2lf[deg C] Humidity:%5.2lf[%%RH]\n", value_list[0], value_list[1], value_list[2]);
		if(value_list[0] < 800) {
			HalActuatorSetValue(halMotor01,HAL_REQUEST_VELOCITY_CONTROL,0);
			HalActuatorSetValue(halMotor02,HAL_REQUEST_VELOCITY_CONTROL,0);
		} else if (value_list[0] >= 800 && value_list[0] < 1000) {
			HalActuatorSetValue(halMotor01,HAL_REQUEST_VELOCITY_CONTROL,100);
			HalActuatorSetValue(halMotor02,HAL_REQUEST_VELOCITY_CONTROL,0);
		} else if (value_list[0] >= 1000) {
			HalActuatorSetValue(halMotor01,HAL_REQUEST_VELOCITY_CONTROL,100);
			HalActuatorSetValue(halMotor02,HAL_REQUEST_VELOCITY_CONTROL,100);
		}
		if(HalSensorGetValueList(halSensor02,&size,value_list) == HAL_ERROR){
			printf("Error HalSensorGetValueList()\n");
			continue;
		}
		printf("timer:%3d ", event_count);
		printf("Temp:%0.2lf[deg C] Pressure:%0.2lf[hPa] Humidity:%0.2lf[%%RH]\n", value_list[0], value_list[1], value_list[2]);
		fflush(stdout);

		if( 100 <= event_count )
			break;
		event_count++;
	}

	HalActuatorSetValue(halMotor01,HAL_REQUEST_VELOCITY_CONTROL,0);
	HalActuatorSetValue(halMotor02,HAL_REQUEST_VELOCITY_CONTROL,0);

	HalGetTime(halSensor01,&timeWk);
	printf("Sensor time = %d\n",timeWk);
	HalGetTime(halSensor02,&timeWk);
	printf("Sensor time = %d\n",timeWk);

	HalRemoveObserver(halSensor01,&halObs201a);
	flgObs = 0;

	HalFinalize(halSensor01);
	HalFinalize(halSensor02);
	HalFinalize(halMotor01);
	HalFinalize(halMotor02);

	HalDestroy(halSensor01);
	HalDestroy(halSensor02);
	HalDestroy(halMotor01);
	HalDestroy(halMotor02);

	printf("openEL End\n");
	return EXIT_SUCCESS;
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
