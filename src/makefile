Target  = libasync.so
Include =
Source  = $(shell find . -name "*.cpp")
LibDir  = ../lib
Lib     =
ObjDir  = ../obj
OutDir  = ../lib
Define  = _XOPEN_SOURCE
CFLAGS  =
CXXFLAGS =
LDFLAGS =
AfterBuild = cp $(shell find . -name "*.h") ../include

$(shell mkdir -p ../include)

include ../utils/makefile