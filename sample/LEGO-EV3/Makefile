ROOT := $(dir $(lastword $(MAKEFILE_LIST)))../..

LIB_DIR = ../../lib

COMPONENT = LEGO-EV3

C_LIB_SOURCES = \
	$(LIB_DIR)/surface/openEL_linkedList.c \
	$(LIB_DIR)/surface/openEL.c \
	$(LIB_DIR)/device/$(COMPONENT)/openEL_platformLINUX.c \
	$(LIB_DIR)/device/$(COMPONENT)/openEL_registryConfig.c \
	$(LIB_DIR)/device/$(COMPONENT)/openEL_MotorLEV3.c \
	$(LIB_DIR)/device/$(COMPONENT)/openEL_MotorMEV3.c \
	$(LIB_DIR)/device/$(COMPONENT)/openEL_ColorSensorEV3.c \
	$(LIB_DIR)/device/$(COMPONENT)/openEL_TouchSensorEV3.c \
	$(LIB_DIR)/device/$(COMPONENT)/openEL_DistanceSensorEV3.c \
	$(LIB_DIR)/device/$(COMPONENT)/openEL_GyroSensorEV3.c

C_OPTIMISATION_FLAGS = -Os

#CROSS_COMPILE = arm-none-linux-gnueabi-
CROSS_COMPILE = 

CC = $(CROSS_COMPILE)gcc
AR = $(CROSS_COMPILE)ar

BASE = $(LIB_DIR)/ev3dev-c/source/ev3

INC_PATH := \
	-I../../include \
	-I../../include/device \
	-I../../include/device/$(COMPONENT) \
	-I../../include/surface \
	-I$(BASE)

#CFLAGS += -DDEBUG
CFLAGS +=
LDFLAGS += -L. -L$(BASE)/lib
LIBS += -lopenel -lrt -lm -lpthread -lev3dev-c 

O_FILES = $(C_LIB_SOURCES:c=o)

LIB_TARGET = libopenel.a

TARGET = sample

TARGET_SOURCES = projLEGO-EV3.c

.PHONY : all
all : openel sample

.PHONY : sample
sample : $(TARGET)
	@echo "Linking $@"
	$(CC) $(CFLAGS) $(INC_PATH) $(LDFLAGS) -o $@ $(TARGET_SOURCES) $(LIBS)

.PHONY : color
color : color
	@echo "Linking $@"
	$(CC) $(CFLAGS) $(INC_PATH) $(LDFLAGS) -o $@ color.c $(LIBS)

.PHONY : line_trace
line_trace : line_trace
	@echo "Linking $@"
	$(CC) $(CFLAGS) $(INC_PATH) $(LDFLAGS) -o $@ line_trace.c $(LIBS)

.PHONY : openel
openel : $(LIB_TARGET)
  $(LIB_TARGET): $(O_FILES)
	@echo "Creating $@"
	$(AR) rv $(LIB_TARGET) $(O_FILES)

%.o: %.c
	@echo "Compiling $< to $@"
	$(CC) $(CFLAGS) $(INC_PATH) -c -o $@ $<

.PHONY: release
release:
	rm $(O_FILES)

.PHONY: clean
clean:
	rm $(LIB_TARGET)
	rm $(TARGET)
	rm $(O_FILES)

