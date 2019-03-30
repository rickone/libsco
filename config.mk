Target = libasyn.so
Source = $(wildcard src/*.cpp src/*.c)
Test = $(wildcard test/*.cpp test/*.c)
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