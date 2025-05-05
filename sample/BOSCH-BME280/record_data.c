/*
 ============================================================================
 Name        : record_data.c
 Author      : OpenEL-WG
 Version     : 3.2
 Copyright   : Japan Embedded Systems Technology Association(JASA)
 Description : Sample program to record data.
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h> // EXIT_SUCCESS
#include <unistd.h> // sleep()
#include <time.h>
#include <errno.h>
#include <string.h>
#include "openEL.h"

HALCOMPONENT_T *halSensor01;
HALFLOAT_T value_list[4];
int32_t event_count;
//int32_t tmSensor;

int main(void) {
	int32_t timeWk, size;
	HALRETURNCODE_T ret;
	time_t t;
	struct tm ltm;
	char *dayofweek[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
	FILE *fp;
	char *fname = "/home/pi/data.csv";
#if 0
	char fname[30];
	t = time(&t);
	localtime_r(&t, &ltm);
	sprintf( fname, "%04d%02d%02d_%02d%02d%02d.csv", 
	ltm.tm_year + 1900, ltm.tm_mon + 1, ltm.tm_mday, ltm.tm_hour, ltm.tm_min, ltm.tm_sec);
#endif
//	printf("openEL Start\n");

	halSensor01 = HalCreate(0x0000000E,0x0000000C,0x00000001,1); // BOSCH BME280

	ret = HalInit(halSensor01);
	if ( ret == HAL_ERROR){
		fprintf(stderr, "Error: Could not initialize HAL Component.\n");
		return EXIT_FAILURE;
	}
	sleep(2); /* 2[s] */
#if 0
	while(1) {
#endif
		fp = fopen( fname, "a");
		if ( fp == NULL ){
			fprintf(stderr, "Error: Could not open file %s.\n", fname);
			return EXIT_FAILURE;
		}
		ret = HalSensorGetValueList(halSensor01,&size,value_list);
		if ( ret == HAL_ERROR){
			fprintf(stderr, "Error: Could not get value.\n");
			return EXIT_FAILURE;
		}
		t = time(&t);
		localtime_r(&t, &ltm);
//		printf("timer:%3d ", event_count);
//		printf("%04d/%02d/%02d/%s/%02d:%02d:%02d Temp:%0.2lf[deg C] Pressure:%0.2lf[hPa] Humidity:%0.2lf[%%RH]\n", 
//			ltm.tm_year + 1900, ltm.tm_mon + 1, ltm.tm_mday, dayofweek[ltm.tm_wday], ltm.tm_hour, ltm.tm_min, ltm.tm_sec, 
//			value_list[0], value_list[1], value_list[2]);
		fprintf( fp, "%04d,%02d,%02d,%s,%02d,%02d,%02d,%0.2lf,%0.2lf,%0.2lf\n",
			ltm.tm_year + 1900, ltm.tm_mon + 1, ltm.tm_mday, dayofweek[ltm.tm_wday], ltm.tm_hour, ltm.tm_min, ltm.tm_sec, 
			value_list[0], value_list[1], value_list[2]);
		if( fclose(fp) != 0 ){
			fprintf(stderr, "Error closing file: %s", strerror(errno));
		}
#if 0
		sleep(60); /* 60[s] */
		if( 10 <= event_count )
			break;
		event_count++;
	}
#endif
	ret = HalFinalize(halSensor01);
	if ( ret == HAL_ERROR){
		fprintf(stderr, "Error: Could not finalize HAL Component.\n");
		return EXIT_FAILURE;
	}

	HalDestroy(halSensor01);

//	printf("openEL End\n");
	return EXIT_SUCCESS;
}
