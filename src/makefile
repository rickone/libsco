Target  = libasyn.so
Include = ../3rd/box/src
Source  = $(shell find . -name "*.cpp")
LibDir  = ../3rd/box/lib
Lib     = box
ObjDir  = ../obj
OutDir  = ../lib
Define  = _XOPEN_SOURCE
CFLAGS  =
CXXFLAGS =
LDFLAGS =
AfterBuild = cp $(shell find . -name "*.h") ../include

UnixName = $(shell uname)
ifeq ($(UnixName),Darwin)
LDFLAGS += -flat_namespace
endif

$(shell mkdir -p ../include)

include ../utils/makefile
