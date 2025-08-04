ROOT := $(dir $(lastword $(MAKEFILE_LIST)))../..

LIB_DIR = ../../lib

COMPONENT1 = SEN-SCD41
COMPONENT2 = BOSCH-BME280
COMPONENT3 = STM-L298P

C_LIB_SOURCES = \
	$(LIB_DIR)/surface/openEL_linkedList.c \
	$(LIB_DIR)/surface/openEL.c \
	$(LIB_DIR)/device/$(COMPONENT1)/openEL_platformLINUX.c \
	$(LIB_DIR)/device/$(COMPONENT1)/openEL_CO2SensorSCD41.c \
	$(LIB_DIR)/device/$(COMPONENT2)/openEL_SensorBME280.c \
	$(LIB_DIR)/device/$(COMPONENT2)/BME280_driver/bme280.c \
	$(LIB_DIR)/device/$(COMPONENT3)/openEL_MotorL298P.c \
	openEL_registryConfig.c

SCD_LIB_SOURCES = \
	$(LIB_DIR)/device/$(COMPONENT1)/embedded-i2c-scd4x/sample-implementations/linux_user_space/sensirion_i2c_hal.c \
	$(LIB_DIR)/device/$(COMPONENT1)/embedded-i2c-scd4x/sensirion_config.h \
	$(LIB_DIR)/device/$(COMPONENT1)/embedded-i2c-scd4x/sensirion_i2c.h \
	$(LIB_DIR)/device/$(COMPONENT1)/embedded-i2c-scd4x/sensirion_i2c.c \
	$(LIB_DIR)/device/$(COMPONENT1)/embedded-i2c-scd4x/sensirion_i2c_hal.h \
	$(LIB_DIR)/device/$(COMPONENT1)/embedded-i2c-scd4x/sensirion_i2c_hal.c \
	$(LIB_DIR)/device/$(COMPONENT1)/embedded-i2c-scd4x/sensirion_common.h \
	$(LIB_DIR)/device/$(COMPONENT1)/embedded-i2c-scd4x/sensirion_common.c \
	$(LIB_DIR)/device/$(COMPONENT1)/embedded-i2c-scd4x/scd4x_i2c.h \
	$(LIB_DIR)/device/$(COMPONENT1)/embedded-i2c-scd4x/scd4x_i2c.c

C_OPTIMISATION_FLAGS = -Os

INC_PATH := \
	-I../../include \
	-I../../include/device \
	-I../../include/device/$(COMPONENT1) \
	-I../../include/device/$(COMPONENT2) \
	-I../../include/device/$(COMPONENT3) \
	-I../../include/surface \
	-I$(LIB_DIR)/device/$(COMPONENT1)/embedded-i2c-scd4x \
	-I$(LIB_DIR)/device/$(COMPONENT2)/BME280_driver

O_FILES = $(C_LIB_SOURCES:c=o)
SCD_O_FILES = $(SCD_LIB_SOURCES:c=o)

LIB_TARGET = libopenel.a
SCD_LIB_TARGET = libscd41.a

TARGET = sample

TARGET_SOURCES = projJASA-DEMO.c

#CFLAGS += -std=c99
CFLAGS +=

.PHONY : all
all : scd41 openel sample

.PHONY : sample
sample : $(TARGET)
	@echo "Linking $@"
	$(CC) $(CFLAGS) $(INC_PATH) -L. -L$(LIB_DIR)/device/$(COMPONENT)/ -o $@ $(TARGET_SOURCES) -lopenel -lrt -lm -lpthread -lscd41

.PHONY : scd41
scd41 : $(SCD_LIB_TARGET)
  $(SCD_LIB_TARGET): $(SCD_O_FILES)
	@echo "Creating $@"
	$(AR) rv $(SCD_LIB_TARGET) $(SCD_O_FILES)

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
	rm $(SCD_O_FILES)

.PHONY: clean
clean:
	rm $(LIB_TARGET)
	rm $(SCD_LIB_TARGET)
	rm $(TARGET)
	rm $(O_FILES)
	rm $(SCD_O_FILES)
