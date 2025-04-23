/*
 ============================================================================
 Name        : turn_right.c
 Author      : OpenEL-WG
 Version     : 3.2
 Copyright   : Japan Embedded Systems Technology Association(JASA)
 Description : Sample program for RT Raspberry Pi Mouse
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include "openEL.h"

#ifndef M_PI
#define M_PI           3.14159265358979323846  /* pi */
#endif

HALCOMPONENT_T *halMotor01;
HALCOMPONENT_T *halMotor02;

int main(void) {
	printf("openEL Start\n");

	halMotor01  = HalCreate(0x00000001,0x00000005,0x00000002,1); // Left Motor
	halMotor02  = HalCreate(0x00000001,0x00000005,0x00000002,2); // Right Motor

	HalInit(halMotor01);
	HalInit(halMotor02);

	HalActuatorSetValue(halMotor01,HAL_REQUEST_VELOCITY_CONTROL,2*M_PI); // Change direction by turning right
	HalActuatorSetValue(halMotor02,HAL_REQUEST_VELOCITY_CONTROL,-2*M_PI);

	usleep(950000); /* 950 msec */

	HalActuatorSetValue(halMotor01,HAL_REQUEST_VELOCITY_CONTROL,0);
	HalActuatorSetValue(halMotor02,HAL_REQUEST_VELOCITY_CONTROL,0);

	HalActuatorSetValue(halMotor01,HAL_REQUEST_NO_EXCITE,0);
	HalActuatorSetValue(halMotor02,HAL_REQUEST_NO_EXCITE,0);

	HalFinalize(halMotor01);
	HalFinalize(halMotor02);

	HalDestroy(halMotor01);
	HalDestroy(halMotor02);

	printf("openEL End\n");
	return EXIT_SUCCESS;
}
