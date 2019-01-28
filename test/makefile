Target  = test
Include = ../include ../3rd/box/src
Source  = $(shell find . -name "*.cpp")
LibDir  = ../lib ../3rd/box/lib
Lib     = asyn pthread dl box
ObjDir  = ../obj
OutDir  = ../bin
Define  = _XOPEN_SOURCE
CFLAGS  =
CXXFLAGS =
LDFLAGS =
Rpath = ../lib

include ../utils/makefile

Targets = $(basename $(Source))
TargetBins = $(Targets:%=$(OutDir)/%)

.PHONY: test
test: start $(TargetBins)

$(OutDir)/%: $(ObjDirPath)/%.o
	$(AfterCompile)
	@echo Link $@ ...
	$(Link)
	$(AfterBuild)
