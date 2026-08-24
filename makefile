MODULES = hal
MODULES += module/freertos
BOARD ?= edu-ciaa-nxp
VERBOSE=n
MUJU ?= ./muju

include $(MUJU)/module/base/makefile
