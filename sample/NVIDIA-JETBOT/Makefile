ROOT := $(dir $(lastword $(MAKEFILE_LIST)))../..

LIB_DIR = ../../lib

COMPONENT = NVIDIA-JETBOT

C_LIB_SOURCES = \
	$(LIB_DIR)/surface/openEL_linkedList.c \
	$(LIB_DIR)/surface/openEL.c \
	$(LIB_DIR)/device/$(COMPONENT)/openEL_platformLINUX.c \
	$(LIB_DIR)/device/$(COMPONENT)/openEL_registryConfig.c \
	$(LIB_DIR)/device/$(COMPONENT)/openEL_MotorPCA9685.c \
	$(LIB_DIR)/device/$(COMPONENT)/openEL_SensorINA219.c

C_OPTIMISATION_FLAGS = -Os

INC_PATH := \
	-I../../include \
	-I../../include/device \
	-I../../include/device/$(COMPONENT) \
	-I../../include/surface

O_FILES = $(C_LIB_SOURCES:c=o)

LIB_TARGET = libopenel.a

TARGET = sample

TARGET_SOURCES = projNVIDIA-JETBOT.c

#CFLAGS += -std=c99
CFLAGS +=

.PHONY : all
all : openel sample run_test combined_test

.PHONY : sample
sample : $(TARGET)
	@echo "Linking $@"
	$(CC) $(CFLAGS) $(INC_PATH) -L. -o $@ $(TARGET_SOURCES) -lopenel -lrt -lm -lpthread

.PHONY : run_test
run_test : run_test
	@echo "Linking $@"
	$(CC) $(CFLAGS) $(INC_PATH) -L. -o $@ run_test.c -lopenel -lrt -lm -lpthread

.PHONY : combined_test
combined_test : combined_test
	@echo "Linking $@"
	$(CC) $(CFLAGS) $(INC_PATH) -L. -o $@ combined_test.c -lopenel -lrt -lm -lpthread

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
