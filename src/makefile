Package =
Target  = libasyn.so
Source  = $(shell find . -name "*.cpp")
Lib     = box
ExportHeader = $(shell find . -name "*.h")
Include = $(shell find . -type d)
Define  = _XOPEN_SOURCE ASYN_DEBUG
CFLAGS  =
CXXFLAGS = -std=c++14
LDFLAGS =
Rpath =

UnixName = $(shell uname)
ifeq ($(UnixName),Darwin)
	LDFLAGS += -flat_namespace
endif

include $(CPKG_PATH)/cmk
