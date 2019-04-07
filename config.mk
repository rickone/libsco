Target = libasyn.so
Source = $(wildcard src/*.cpp src/*.c)
Test = $(wildcard test/*.cpp test/*.c)
Lib = box
LibPath =
Include =
Define = _XOPEN_SOURCE
CFLAGS = -g -D_DEBUG -Wall
CXXFLAGS = -std=c++14
LDFLAGS = -lpthread -ldl
Rpath =

UnixName = $(shell uname)
ifeq ($(UnixName),Darwin)
	LDFLAGS += -flat_namespace
endif
