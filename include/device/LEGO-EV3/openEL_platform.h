/*
 * openEL_platform.h
 *
 *  Created on: 2018/05/13
 *      Author: OpenEL-WG
 */

#ifndef OPENEL_PLATFORM_H_
#define OPENEL_PLATFORM_H_

#include <stdint.h>

void *HalMalloc(uint32_t size);
void HalFree(void *pMem);

HALRETURNCODE_T HalGetTime_platform(HALCOMPONENT_T *halComponent, int32_t *time);
HALRETURNCODE_T HalEventTimerStartTimer_platform(HALEVENTTIMER_T *eventTimer);
HALRETURNCODE_T HalEventTimerStopTimer_platform(HALEVENTTIMER_T *eventTimer);


#endif /* OPENEL_PLATFORM_H_ */

