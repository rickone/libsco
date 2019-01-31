Target  =
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

Targets = $(basename $(Source))
TargetBins = $(Targets:%=$(OutDir)/%)

$(OutDir)/build_test.log: $(TargetBins)
	@echo "Test build at `date`" > $@

include ../utils/makefile

$(OutDir)/%: $(ObjDirPath)/%.o
	$(AfterCompile)
	@echo Link $@ ...
	@$(Link)
	$(AfterBuild)
