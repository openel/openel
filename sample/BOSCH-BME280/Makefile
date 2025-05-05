ROOT := $(dir $(lastword $(MAKEFILE_LIST)))../..

LIB_DIR = ../../lib

COMPONENT = BOSCH-BME280

C_LIB_SOURCES = \
	$(LIB_DIR)/surface/openEL_linkedList.c \
	$(LIB_DIR)/surface/openEL.c \
	$(LIB_DIR)/device/$(COMPONENT)/openEL_platformLINUX.c \
	$(LIB_DIR)/device/$(COMPONENT)/openEL_registryConfig.c \
	$(LIB_DIR)/device/$(COMPONENT)/openEL_SensorBME280.c \
	$(LIB_DIR)/device/$(COMPONENT)/BME280_driver/bme280.c

C_OPTIMISATION_FLAGS = -Os

INC_PATH := \
	-I../../include \
	-I../../include/device \
	-I../../include/device/$(COMPONENT) \
	-I../../include/surface

O_FILES = $(C_LIB_SOURCES:c=o)

LIB_TARGET = libopenel.a

TARGET = sample

TARGET_SOURCES = projSEN-BME280.c

TARGET_ALT = altitude

TARGET_ALT_SOURCES = altitude.c

TARGET_REC = record_data

TARGET_REC_SOURCES = record_data.c

#CFLAGS += -std=c99
CFLAGS +=

.PHONY : all
all : openel sample altitude record_data

.PHONY : altitude
altitude : $(TARGET_ALT)
	@echo "Linking $@"
	$(CC) $(CFLAGS) $(INC_PATH) -L. -L$(LIB_DIR)/device/$(COMPONENT)/ -o $@ $(TARGET_ALT_SOURCES) -lopenel -lrt -lm -lpthread

.PHONY : record_data
record_data : $(TARGET_REC)
	@echo "Linking $@"
	$(CC) $(CFLAGS) $(INC_PATH) -L. -L$(LIB_DIR)/device/$(COMPONENT)/ -o $@ $(TARGET_REC_SOURCES) -lopenel -lrt -lm -lpthread

.PHONY : sample
sample : $(TARGET)
	@echo "Linking $@"
	$(CC) $(CFLAGS) $(INC_PATH) -L. -L$(LIB_DIR)/device/$(COMPONENT)/ -o $@ $(TARGET_SOURCES) -lopenel -lrt -lm -lpthread

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
