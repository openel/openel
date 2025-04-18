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
all : openel sample mf mb tr tl cdr cdl

.PHONY : sample
sample :
	@echo "Linking $@"
	$(CC) $(CFLAGS) $(INC_PATH) -L. -o $@ $(TARGET_SOURCES) -lopenel -lrt -lm

.PHONY : mf
mf :
	$(CC) $(CFLAGS) $(INC_PATH) -L. -o $@ move_forward.c -lopenel -lrt -lm

.PHONY : mb
mb :
	$(CC) $(CFLAGS) $(INC_PATH) -L. -o $@ move_backward.c -lopenel -lrt -lm

.PHONY : tr
tr :
	$(CC) $(CFLAGS) $(INC_PATH) -L. -o $@ turn_right.c -lopenel -lrt -lm

.PHONY : tl
tl :
	$(CC) $(CFLAGS) $(INC_PATH) -L. -o $@ turn_left.c -lopenel -lrt -lm

.PHONY : cdr
cdr :
	$(CC) $(CFLAGS) $(INC_PATH) -L. -o $@ change_direction_right.c -lopenel -lrt -lm

.PHONY : cdl
cdl :
	$(CC) $(CFLAGS) $(INC_PATH) -L. -o $@ change_direction_left.c -lopenel -lrt -lm

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
	rm mf mb tr tl cd
