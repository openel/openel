/*
 ============================================================================
 Name        : altitude.c
 Author      : OpenEL-WG
 Version     : 3.2
 Copyright   : Japan Embedded Systems Technology Association(JASA)
 Description : Sample program for BOSCH BME280
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
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
	time_t now;

	printf("openEL Start\n");
	halSensor01 = HalCreate(0x0000000E,0x0000000C,0x00000001,1); // CO2(Sensirion SCD30)

	HalInit(halSensor01);

	outProperty(halSensor01);

	printf("sensor01 getTime ret=%d\n", HalGetTime(halSensor01,&timeWk) );
	printf("Sensor time = %d\n",timeWk);

	HalAddObserver(halSensor01,&halObs201a);
	flgObs = 1;
	now = time(&now);
	printf("%s", ctime(&now));
	HALFLOAT_T temp, atm, hum, qnh;
	HALFLOAT_T altitude,p0,h;
//	p0 = 1010.68 /* https://www.jma.go.jp/bosai/amedas/#area_type=japan&area_code=010000 */;
	h = 21.8 + 3; /* [m] */
	printf("event_count, Temp[deg C], Pressure[hPa], Humidity[%%RH], QNH[inHg], Time[s], Altitude[m], [ft]\n");
	while(1) {
		sleep(2); /* 2[s] */
		HalSensorGetValueList(halSensor01,&size,value_list);
		HalSensorGetTimedValueList(halSensor01,&size,value_list, &tmSensor);
		temp = value_list[0];
		atm = value_list[1];
		hum = value_list[2];
		qnh = value_list[1]/1013.25*29.92;
//		printf("%3d, %0.2lf, %0.2lf, %0.2lf, %0.2lf, %d\n", event_count, temp, atm, hum, qnh, tmSensor, altitude);
		/* Altitude */
		p0 = atm * powf((1-(0.0065 * h)/(temp + (0.0065 * h) + 273.15)) , -5.257);
//		printf("P0 = %0.2f[hPa]\n", p0);
		altitude = ((powf(p0/atm, 1/5.256) - 1 ) * (temp + 273.15)) / 0.0065 * powf(1013.25/p0, 1/5.256);
		printf("%3d, %0.2lf, %0.2lf, %0.2lf, %0.2lf, %d, %0.2f, %0.2f\n", event_count, temp, atm, hum, qnh, tmSensor, altitude, altitude / 3.28);
//		printf("Altitude = %0.2f[m], %0.2f[ft]\n", altitude, altitude / 3.28);	

		fflush(stdout);
		if( 300 <= event_count )
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

	/* METAR */
	now = time(&now);
	struct tm *zulu = gmtime(&now);
	printf("FUJIMINO %02d%02d%02dZ ?????KT ???? %2d/?? Q%4d A%4d\n", zulu->tm_mday, zulu->tm_hour, zulu->tm_min, (int32_t)temp, (int32_t)atm, (int32_t)(qnh*100));

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
