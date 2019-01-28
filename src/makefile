Target  = libasyn.so
Include = ../3rd/box/src
Source  = $(shell find . -name "*.cpp")
LibDir  = ../lib ../3rd/box/lib
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
