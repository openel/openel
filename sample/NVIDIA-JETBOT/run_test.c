/*
 ============================================================================
 Name        : run_test.c
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

HALCOMPONENT_T *halMotor01;
HALCOMPONENT_T *halMotor02;

void outProperty(HALCOMPONENT_T *hC);

void move_forward(uint32_t distance);
void move_backward(uint32_t distance);
void turn_left(void);
void turn_right(void);

//#define TURN_TEST
//#define MOVE_TEST
#define RUN_COURSE

int32_t W=118; // Body Width [mm]
int32_t R=59;  // Body Radius [mm]
int32_t D=64;  // Wheel Diameter [mm]
int32_t r=32;  // Wheel Radius [mm]

int main(void) {
	printf("openEL Start\n");

	halMotor01  = HalCreate(0x00000001,0x0000000B,0x00000001,1); // Left Motor
	halMotor02  = HalCreate(0x00000001,0x0000000B,0x00000001,2); // Right Motor

	HalInit(halMotor01);
	HalInit(halMotor02);

	outProperty(halMotor01);
	outProperty(halMotor02);

#ifdef TURN_TEST
	turn_left(); // 90 degree left turn
	turn_left();
	turn_left();
	turn_left();
	turn_right(); // 90 degree right turn
	turn_right();
	turn_right();
	turn_right();
#endif

#ifdef MOVE_TEST
	move_forward(10); // 10[cm]
	move_forward(20);
	move_forward(30);
	move_backward(10);
	move_backward(20);
	move_backward(30);
#endif

#ifdef RUN_COURSE
	// Run the sample test cource
	move_forward(30);
	turn_right();
	move_forward(40);
	turn_right();
	move_forward(30);
	turn_left();
	move_forward(30);
#endif

	HalActuatorSetValue(halMotor01,HAL_REQUEST_NO_EXCITE,0);
	HalActuatorSetValue(halMotor02,HAL_REQUEST_NO_EXCITE,0);

	HalFinalize(halMotor01);
	HalFinalize(halMotor02);

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

void move_forward(uint32_t distance){
	uint32_t i,j;
	printf("Move forward:%u[cm]\n", distance);
	j = distance / 10;
	for(i=0;i<j;i++){
		HalActuatorSetValue(halMotor01,HAL_REQUEST_VELOCITY_CONTROL,2*M_PI);
		HalActuatorSetValue(halMotor02,HAL_REQUEST_VELOCITY_CONTROL,2*M_PI);
		usleep(535000); // 10[cm]
		HalActuatorSetValue(halMotor01,HAL_REQUEST_VELOCITY_CONTROL,0);
		HalActuatorSetValue(halMotor02,HAL_REQUEST_VELOCITY_CONTROL,0);
		sleep(1);
	}
}

void move_backward(uint32_t distance){
	uint32_t i,j;
	printf("Move backward:%u[cm]\n", distance);
	j = distance / 10;
	for(i=0;i<j;i++){
		HalActuatorSetValue(halMotor01,HAL_REQUEST_VELOCITY_CONTROL,-2*M_PI);
		HalActuatorSetValue(halMotor02,HAL_REQUEST_VELOCITY_CONTROL,-2*M_PI);
		usleep(570000); // 10[cm]
		HalActuatorSetValue(halMotor01,HAL_REQUEST_VELOCITY_CONTROL,0);
		HalActuatorSetValue(halMotor02,HAL_REQUEST_VELOCITY_CONTROL,0);
		sleep(1);
	}
}

void turn_left(void){
	printf("Turn left\n");
	// 90 degree left turn
	HalActuatorSetValue(halMotor01,HAL_REQUEST_VELOCITY_CONTROL,-2*M_PI);
	HalActuatorSetValue(halMotor02,HAL_REQUEST_VELOCITY_CONTROL,2*M_PI);
	// Depends on the coefficient of friction of the floor
	usleep(565000);
	HalActuatorSetValue(halMotor01,HAL_REQUEST_VELOCITY_CONTROL,0);
	HalActuatorSetValue(halMotor02,HAL_REQUEST_VELOCITY_CONTROL,0);
	sleep(1);
}

void turn_right(void){
	printf("Turn right\n");
	// 90 degree right turn
	HalActuatorSetValue(halMotor01,HAL_REQUEST_VELOCITY_CONTROL,2*M_PI);
	HalActuatorSetValue(halMotor02,HAL_REQUEST_VELOCITY_CONTROL,-2*M_PI);
	// Depends on the coefficient of friction of the floor
	usleep(570000);
	HalActuatorSetValue(halMotor01,HAL_REQUEST_VELOCITY_CONTROL,0);
	HalActuatorSetValue(halMotor02,HAL_REQUEST_VELOCITY_CONTROL,0);
	sleep(1);
}

