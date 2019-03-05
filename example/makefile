Target  = example
Source  = $(shell find . -name "*.cpp")
Lib     = asyn box
ExportHeader =
Define  = _XOPEN_SOURCE
CFLAGS  =
CXXFLAGS = -std=c++14
LDFLAGS = -lpthread -ldl

.PHONY: example bins

example: bins

include ../utils/makefile

Targets = $(basename $(Source))
TargetBins = $(Targets:%=$(OutputPath)/%)

bins: $(TargetBins)

$(OutputPath)/%: $(ObjectPath)/%.o
	@echo Link $@ ...
	@$(Link)