BOARD_TAG = mega2560
ARDUINO_PORT = /dev/cu.usb*

ARDUINO_SKETCHBOOK = .
ARDUINO_DIR = /Applications/Arduino.app/Contents/Resources/Java
ARDMK_DIR = ./libraries/Makefile

ARDUINO_LIBS = SD SD/utility UTFT UTouch AnalogInputs Directory Midi Wave UI Menu

include ./libraries/Makefile/arduino-mk/Arduino.mk
