ROOT := $(dir $(lastword $(MAKEFILE_LIST)))../..

LIB_DIR = ../../lib

COMPONENT = RT-RASPI-MOUSE

C_LIB_SOURCES = \
	$(LIB_DIR)/surface/openEL_linkedList.c \
	$(LIB_DIR)/surface/openEL.c \
	$(LIB_DIR)/device/$(COMPONENT)/openEL_platformLINUX.c \
	$(LIB_DIR)/device/$(COMPONENT)/openEL_registryConfig.c \
	$(LIB_DIR)/device/$(COMPONENT)/openEL_MotorRaspiMouse.c \
	$(LIB_DIR)/device/$(COMPONENT)/openEL_SensorRaspiMouse.c

C_OPTIMISATION_FLAGS = -Os

INC_PATH := \
	-I../../include \
	-I../../include/device \
	-I../../include/device/$(COMPONENT) \
	-I../../include/surface

O_FILES = $(C_LIB_SOURCES:c=o)

LIB_TARGET = libopenel.a

TARGET = sample

TARGET_SOURCES = projRT-RASPI-MOUSE.c

#CFLAGS += -std=c99
CFLAGS +=

.PHONY : all
all : openel sample mf1 mb1 tr90 tl90 tr180 tl180 light_sensor free_run

.PHONY : sample
sample :
	@echo "Linking $@"
	$(CC) $(CFLAGS) $(INC_PATH) -L. -o $@ $(TARGET_SOURCES) -lopenel -lrt -lm

.PHONY : mf1
mf1 :
	$(CC) $(CFLAGS) $(INC_PATH) -L. -o $@ move_forward.c -lopenel -lrt -lm

.PHONY : mb1
mb1 :
	$(CC) $(CFLAGS) $(INC_PATH) -L. -o $@ move_backward.c -lopenel -lrt -lm

.PHONY : tr90
tr90 :
	$(CC) $(CFLAGS) $(INC_PATH) -L. -o $@ turn_right_90.c -lopenel -lrt -lm

.PHONY : tl90
tl90 :
	$(CC) $(CFLAGS) $(INC_PATH) -L. -o $@ turn_left_90.c -lopenel -lrt -lm

.PHONY : tr180
tr180 :
	$(CC) $(CFLAGS) $(INC_PATH) -L. -o $@ turn_right_180.c -lopenel -lrt -lm

.PHONY : tl180
tl180 :
	$(CC) $(CFLAGS) $(INC_PATH) -L. -o $@ turn_left_180.c -lopenel -lrt -lm

.PHONY : light_sensor
light_sensor :
	$(CC) $(CFLAGS) $(INC_PATH) -L. -o $@ light_sensor.c -lopenel -lrt -lm

.PHONY : free_run
free_run :
	$(CC) $(CFLAGS) $(INC_PATH) -L. -o $@ free_run.c -lopenel -lrt -lm

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
	rm mf1 mb1 tr90 tl90 tr180 tl180 light_sensor free_run

.PHONY: install
install:
	cp mf1 mb1 tr90 tl90 tr180 tl180 light_sensor free_run sample run.sh ~/bin/
