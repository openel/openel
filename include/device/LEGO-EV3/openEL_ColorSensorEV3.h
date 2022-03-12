/*
 * openEL_ColorSensorEV3.h
 *
 *  Created on: 2021/05/01
 *      Author: OpenEL-WG
 */

#ifndef OPENEL_COLORSENSOREV3_H_
#define OPENEL_COLORSENSOREV3_H_

extern HAL_FNCTBL_T HalColorSensorEV3Tbl;

/* request code */
#define HAL_REQUEST_MODE_COL_REFLECT	(0)
#define HAL_REQUEST_MODE_COL_AMBIENT	(1)
#define HAL_REQUEST_MODE_COL_COLOR	(2)
#define HAL_REQUEST_MODE_REF_RAW	(3)
#define HAL_REQUEST_MODE_RGB_RAW	(4)
#define HAL_REQUEST_MODE_COL_CAL	(5)

#endif /* OPENEL_COLORSENSOREV3_H_ */

