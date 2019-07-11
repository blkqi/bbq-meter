PROJECT_DIR    = $(shell pwd)
BOARD_TAG      = mini328
F_CPU          = 8000000
ARDUINO_LIBS   = ELClient
USER_LIB_PATH  = $(realpath $(PROJECT_DIR)/lib)
CPPFLAGS       = -std=c++11

include /usr/share/arduino/Arduino.mk
