/*
 ============================================================================
 Name        : projSEN-SCD30.c
 Author      : OpenEL-WG
 Version     : 3.2
 Copyright   : Japan Embedded Systems Technology Association(JASA)
 Description : Sample program for SENSIRION SCD30
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include "openEL.h"

HALCOMPONENT_T *halSensor01;

int32_t event_count;
HALFLOAT_T value_list[4];
int32_t tmSensor;

static void notify_event201a(HALCOMPONENT_T *halComponent, int32_t eventID) {
	printf("notify_event201a : %d\n",eventID);
	if(eventID == 1) /* CO2 > 1000[ppm] */
		printf("Warning:Ventilation is required.\n");
	if(eventID == 2) /* CO2 > 1500[ppm] */
		printf("Danger:Ventilate now!\n");
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
	halSensor01 = HalCreate(0x0000000E,0x0000000C,0x00000001,1); // CO2(Sensirion SCD30)

	HalInit(halSensor01);

	outProperty(halSensor01);

	printf("sensor01 getTime ret=%d\n", HalGetTime(halSensor01,&timeWk) );
	printf("Sensor time = %d\n",timeWk);

	HalAddObserver(halSensor01,&halObs201a);
	flgObs = 1;

	while(1) {
		sleep(2); /* 2[s] */
		HalSensorGetValueList(halSensor01,&size,value_list);
		printf("timer:%3d ", event_count);
		printf("Temp:%0.2lf[deg C] Pressure:%0.2lf[hPa] Humidity:%0.2lf[%%RH]\n", value_list[0], value_list[1], value_list[2]);
		fflush(stdout);
		if( 10 <= event_count )
			break;
		event_count++;
	}

	HalGetTime(halSensor01,&timeWk);
	printf("Sensor time = %d\n",timeWk);

	HalRemoveObserver(halSensor01,&halObs201a);
	flgObs = 0;

	HalFinalize(halSensor01);

	HalDestroy(halSensor01);

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
