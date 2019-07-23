PROJECT_DIR    = $(shell pwd)
BOARD_TAG      = pro
BOARD_SUB      = 8MHzatmega328
ARDUINO_LIBS   = ELClient
USER_LIB_PATH  = $(realpath $(PROJECT_DIR)/lib)
CPPFLAGS       = -std=c++11 -I.

ALTERNATE_CORE_PATH = /usr/share/arduino/hardware/archlinux-arduino/avr

include /usr/share/arduino/Arduino.mk
