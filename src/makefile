Package =
TargetType = so
#Test = 1
Source = $(wildcard *.cpp core/*.cpp aux/*.cpp hook/*.cpp)
Import = box
ExportHeader = $(wildcard *.h core/*.h aux/*.h)
Include = . core aux
Define = _XOPEN_SOURCE ASYN_DEBUG
CFLAGS =
CXXFLAGS = -std=c++14
LDFLAGS =
Rpath =

UnixName = $(shell uname)
ifeq ($(UnixName),Darwin)
	LDFLAGS += -flat_namespace
endif

include $(CPKG_PATH)/cmk
