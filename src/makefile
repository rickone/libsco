Target  = libasyn.so
Include = $(shell find . -type d)
Source  = $(shell find . -name "*.cpp")
Lib     = box
ExportHeader = $(shell find . -name "*.h")
Define  = _XOPEN_SOURCE ASYN_DEBUG
CFLAGS  =
CXXFLAGS = -std=c++14
LDFLAGS =

UnixName = $(shell uname)
ifeq ($(UnixName),Darwin)
LDFLAGS += -flat_namespace
endif

include ../utils/makefile