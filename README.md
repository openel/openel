# openel-cpp
OpenEL implemented in C++

# OpenEL
OpenEL(Open Embedded Library) is a unified API(Application Programming Interface) for actuators and sensors. The specifications and implementation have been developed by JASA(Japan Embedded Systems Technology Association) since 2011.

## Specification
https://openel.github.io/

## About this document
This document explains about Package contents and How to use OpenEL version 3.2.x.

## Package contents

<pre>
openel -+- include
        +- lib
        +- sample
        +- LICENSE
        +- README.md (This file)
</pre>

<pre>
include             --- Header files for OpenEL
lib                 --- Library files for OpenEL
sample              --- A sample program which use OpenEL API
LICENSE             --- License file
README.md           --- This file
</pre>

## How to use OpenEL API

### Header file
When you compile a program which use OpenEL API, set PATH to openel/include/surface and openel/include/device/your_component
directory to use the header file(openEL.h etc.) for OpenEL.

For examples, when you use gcc  
<pre>
  -Iopenel/include/surface  
  -Iopenel/include/device/your_component
</pre>

Include the header file named "openEL.h" to use OpenEL API

For examples,  
<pre>
  　#include <openEL.h>  
</pre>

### Library files
When you compile a program which use OpenEL API, set LIBRARY PATH to openel/lib/surface and openel/lib/device/your_component to link the library file for OpenEL.

For examples, when you use gcc  
<pre>
-Lopenel/lib   
-Lopenel//lib/device/your_component  
</pre>
Link the library file named "libopenel.a" to use OpenEL API.

For examples,  
<pre>
　gcc -o sample sample.c -lopenel
</pre>

See sample/simLinux/Makefile for details.

## Sample program for Linux
<pre>
$ cd sample/simLinux
$ make
$ ./sample
openEL Start
HalInit Motor1 HAL-ID 1 0 1 1
HalInit Motor2 HAL-ID 1 0 2 2
HalInit Sensor1 HAL-ID 2 0 16 1
PROPERTY - Name : MOTOR_100a
PROPERTY - fnc00 : HalInit
PROPERTY - fnc01 : HalReInit
PROPERTY - fnc02 : HalFinalize
PROPERTY - fnc03 : HalAddObserver
PROPERTY - fnc04 : HalRemoveObserver
PROPERTY - fnc05 : HalGetProperty
PROPERTY - fnc06 : HalGetTime
PROPERTY - fnc07 : HalActuatorSetValue
PROPERTY - fnc08 : HalActuatorGetValue
PROPERTY - Name : MOTOR_200
PROPERTY - fnc00 : HalInit
PROPERTY - fnc01 : HalReInit
PROPERTY - fnc02 : HalFinalize
PROPERTY - fnc03 : HalAddObserver
PROPERTY - fnc04 : HalRemoveObserver
PROPERTY - fnc05 : HalGetProperty
PROPERTY - fnc06 : HalGetTime
PROPERTY - fnc07 : HalActuatorSetValue
PROPERTY - fnc08 : HalActuatorGetValue
PROPERTY - Name : SENSOR_100
PROPERTY - fnc00 : HalInit
PROPERTY - fnc01 : HalReInit
PROPERTY - fnc02 : HalFinalize
PROPERTY - fnc03 : HalAddObserver
PROPERTY - fnc04 : HalRemoveObserver
PROPERTY - fnc05 : HalGetProperty
PROPERTY - fnc06 : HalGetTime
PROPERTY - fnc07 : HalSensorGetTimedValue
PROPERTY - fnc08 : HalSensorGetTimedValueList
motor01  getTime ret=1
motor02  getTime ret=1
sensor01 getTime ret=0
Sensor time = 0
timer      1 ,    10 ,      1 :   0.000   0.628   3.079   3.079     0(tmSen)
timer      2 ,    20 ,      1 :   0.020   1.253   3.079   3.079     0(tmSen)
timer      3 ,    30 ,      2 :   0.079   1.874   2.955   2.955     0(tmSen)
timer      4 ,    40 ,      2 :   0.177   2.487   2.955   2.955     0(tmSen)
timer      5 ,    50 ,      3 :   0.314   3.090   2.833   2.833     0(tmSen)
timer      6 ,    60 ,      3 :   0.489   3.681   2.833   2.833     0(tmSen)
timer      7 ,    70 ,      4 :   0.702   4.258   2.712   2.712     1(tmSen)
timer      8 ,    80 ,      4 :   0.952   4.818   2.712   2.712     1(tmSen)
timer      9 ,    90 ,      5 :   1.237   5.358   2.593   2.593     1(tmSen)
timer     10 ,   100 ,      5 :   1.557   5.878   2.593   2.593     1(tmSen)
timer     11 ,   110 ,      6 :   1.910   6.374   2.474   2.474     1(tmSen)
timer     12 ,   120 ,      6 :   2.295   6.845   2.474   2.474     1(tmSen)
timer     13 ,   130 ,      7 :   2.710   7.290   2.356   2.356     1(tmSen)
timer     14 ,   140 ,      7 :   3.155   7.705   2.356   2.356     1(tmSen)
timer     15 ,   150 ,      8 :   3.626   8.090   2.238   2.238     1(tmSen)
timer     16 ,   160 ,      8 :   4.122   8.443   2.238   2.238     1(tmSen)
timer     17 ,   170 ,      9 :   4.642   8.763   2.120   2.120     2(tmSen)
timer     18 ,   180 ,      9 :   5.182   9.048   2.120   2.120     2(tmSen)
timer     19 ,   190 ,     10 :   5.742   9.298   2.000   2.000     2(tmSen)
timer     20 ,   200 ,     10 :   6.319   9.511   2.000   2.000     2(tmSen)

(omit)

timer     96 ,   960 ,     48 :   0.489  -2.487  -2.821  -2.821     9(tmSen)
timer     97 ,   970 ,     49 :   0.314  -1.874  -2.950  -2.950    10(tmSen)
timer     98 ,   980 ,     49 :   0.177  -1.253  -2.950  -2.950    10(tmSen)
timer     99 ,   990 ,     50 :   0.079  -0.628  -3.078  -3.078    10(tmSen)
timer    100 ,  1000 ,     50 :   0.020  -0.000  -3.078  -3.078    10(tmSen)
notify_event201a : 1
notify_event201b : 1
timer    101 ,  1010 ,     51 :   0.000  -0.000  -3.142  -3.142    10(tmSen)
timer    102 ,  1020 ,     51 :   0.000  -0.000  -3.142  -3.142    10(tmSen)
timer    103 ,  1030 ,     52 :   0.000  -0.000  -3.142  -3.142    10(tmSen)
timer    104 ,  1040 ,     52 :   0.000  -0.000  -3.142  -3.142    10(tmSen)
timer    105 ,  1050 ,     53 :   0.000  -0.000  -3.142  -3.142    10(tmSen)
timer    106 ,  1060 ,     53 :   0.000  -0.000  -3.142  -3.142    10(tmSen)
timer    107 ,  1070 ,     54 :   0.000  -0.000  -3.142  -3.142    11(tmSen)
timer    108 ,  1080 ,     54 :   0.000  -0.000  -3.142  -3.142    11(tmSen)
timer    109 ,  1090 ,     55 :   0.000  -0.000  -3.142  -3.142    11(tmSen)
timer    110 ,  1100 ,     55 :   0.000  -0.000  -3.142  -3.142    11(tmSen)
timer    111 ,  1110 ,     56 :   0.000  -0.000  -3.142  -3.142    11(tmSen)
timer    112 ,  1120 ,     56 :   0.000  -0.000  -3.142  -3.142    11(tmSen)
timer    113 ,  1130 ,     57 :   0.000  -0.000  -3.142  -3.142    11(tmSen)
timer    114 ,  1140 ,     57 :   0.000  -0.000  -3.142  -3.142    11(tmSen)
timer    115 ,  1150 ,     58 :   0.000  -0.000  -3.142  -3.142    11(tmSen)
timer    116 ,  1160 ,     58 :   0.000  -0.000  -3.142  -3.142    11(tmSen)
timer    117 ,  1170 ,     59 :   0.000  -0.000  -3.142  -3.142    12(tmSen)
timer    118 ,  1180 ,     59 :   0.000  -0.000  -3.142  -3.142    12(tmSen)
timer    119 ,  1190 ,     60 :   0.000  -0.000  -3.142  -3.142    12(tmSen)
notify_error201a : 202
notify_error201b : 202
timer    120 ,  1200 ,     60 :   0.000  -0.000  -3.142  -3.142    12(tmSen)
timer    121 ,  1210 ,     61 :   0.000  -0.000  -3.142  -3.142    12(tmSen)
timer    122 ,  1220 ,     61 :   0.000  -0.000  -3.142  -3.142    12(tmSen)
timer    123 ,  1230 ,     62 :   0.000  -0.000  -3.142  -3.142    12(tmSen)
timer    124 ,  1240 ,     62 :   0.000  -0.000  -3.142  -3.142    12(tmSen)

(omit)

timer    296 ,  2960 ,    148 :   0.000  -0.000  -3.142  -3.142    29(tmSen)
timer    297 ,  2970 ,    149 :   0.000  -0.000  -3.142  -3.142    30(tmSen)
timer    298 ,  2980 ,    149 :   0.000  -0.000  -3.142  -3.142    30(tmSen)
timer    299 ,  2990 ,    150 :   0.000  -0.000  -3.142  -3.142    30(tmSen)
timer    300 ,  3000 ,    150 :   0.000   0.000  -3.142  -3.142    30(tmSen)
Sensor time = 30
openEL End
</pre>
