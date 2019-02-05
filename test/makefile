Target  =
Include = ../include ../3rd/box/src ../3rd/hiredis
Source  = $(shell find . -name "*.cpp")
LibDir  = ../lib ../3rd/box/lib
Lib     = asyn pthread dl box hiredis
ObjDir  = ../obj
OutDir  = ../bin
Define  = _XOPEN_SOURCE
CFLAGS  =
CXXFLAGS =
LDFLAGS =
Rpath = ../lib

Targets = $(basename $(Source))
TargetBins = $(Targets:%=$(OutDir)/%)

.PHONY: tests
tests: $(TargetBins)

include ../utils/makefile

$(OutDir)/%: $(ObjDirPath)/%.o
	$(AfterCompile)
	@echo Link $@ ...
	@$(Link)
	$(AfterBuild)
