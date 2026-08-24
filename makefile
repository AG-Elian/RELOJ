MODULES = hal freertos
BOARD ?= edu-ciaa-nxp
VERBOSE=n
MUJU ?= ./muju

include $(MUJU)/module/base/makefile
