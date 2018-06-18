# OpenEL
OpenEL(Open Embedded Library) was developed by Japan Embedded Systems Technology Association(JASA).

Documentation is in the wiki:
http://www.jasa.or.jp/openel/Main_Page

## About this document
This document explains about Package contents and How to use OpenEL version 3.1.x.

## Package contents

    openel -+- include  
            +- lib  
            +- sample  
            +- LICENSE  
            +- README.md (This file)  

include             --- Header files for OpenEL  
lib                 --- Library files for OpenEL  
sample              --- A sample program which use OpenEL API  
LICENSE             --- License file  
README.md           --- This file  

## How to use OpenEL API

### Header file
When you compile a program which use OpenEL API, set PATH to openel/include/surface and openel/include/device/your_component
directory to use the header file(openEL.h etc.) for OpenEL.

For examples, when you use gcc  
  -Iopenel/include/surface  
  -Iopenel/include/device/your_component

Include the header file named "openEL.h" to use OpenEL API

For examples,  
  　#include <openEL.h>  

### Library files
When you compile a program which use OpenEL API, set LIBRARY PATH to openel/lib/surface and openel/lib/device/your_component to link the library file for OpenEL.

For examples, when you use gcc  
-Lopenel/lib   
-Lopenel//lib/device/your_component  

Link the library file named "libopenel.a" to use OpenEL API.

For examples,  
　gcc -o sample sample.c -lopenel
