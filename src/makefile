Target  = libasync.so
Include = ../3rd/xbin/src
Source  = $(shell find . -name "*.cpp")
LibDir  = ../lib ../3rd/xbin/lib
Lib     = xbin
ObjDir  = ../obj
OutDir  = ../lib
Define  = _XOPEN_SOURCE
CFLAGS  =
CXXFLAGS =
LDFLAGS =
AfterBuild = cp $(shell find . -name "*.h") ../include

$(shell mkdir -p ../include)

include ../utils/makefile