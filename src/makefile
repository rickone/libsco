Target = libasyn.so
Source = $(wildcard *.cpp *.c)
Lib = box
LibPath =
Include =
Define = _XOPEN_SOURCE ASYN_DEBUG
CFLAGS = -g -D_DEBUG -Wall
CXXFLAGS = -std=c++14
LDFLAGS =
Rpath =

UnixName = $(shell uname)
ifeq ($(UnixName),Darwin)
	LDFLAGS += -flat_namespace
endif

include $(CUTILS_PATH)/template.cmk
