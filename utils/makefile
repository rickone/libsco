#Package = pkg
#BuildPath = ../build
#Target  = appserver
#Include = ../lua ../common
#Source	= main.cpp
#Lib 	= lua common
#ExportHeader =

#Define =
#CFLAGS	=
#CXXFLAGS =
#LDFLAGS =
#Rpath =

#CC = cc
#CXX = c++

# Config: Debug/Release
Config ?= Debug
UnixName ?= $(shell uname)
BuildPath ?= ../build
ifeq ($(Package),)
	Package = $(notdir $(shell cd .. && pwd))
endif

# TargetExt "", ".a", ".so"
TargetExt = $(suffix $(Target))

# OutputPath, TargetName & TargetFile
ifeq ($(TargetExt),)
	OutputPath = $(BuildPath)/bin
	TargetName = $(Target)
	TargetFile = $(OutputPath)/$(Package)_$(Target)
endif
ifeq ($(TargetExt),.a)
	OutputPath = $(BuildPath)/lib
	TargetName = $(patsubst lib%.a,%,$(Target))
	TargetFile += $(OutputPath)/$(Target)
endif
ifeq ($(TargetExt),.so)
	OutputPath = $(BuildPath)/lib
	TargetName = $(patsubst lib%.so,%,$(Target))
	TargetFile += $(OutputPath)/$(Target)
endif

# Include & Define
Include += $(Lib:%=$(BuildPath)/include/%)
CFLAGS += $(Include:%=-I%) $(Define:%=-D%) -m64

# Library
LibraryPath = -L$(BuildPath)/lib
Library = $(Lib:%=-l%)

# ObjectPath
ObjectPath = $(BuildPath)/obj/$(Package)/$(TargetName)

# ExportHeaderPath
ExportHeaderPath = $(BuildPath)/include/$(Package)

# Config
ifeq ($(Config),Release)
	CFLAGS += -O3 -march=native
else
	CFLAGS += -g -D_DEBUG -Wall
	ifeq ($(UnixName),Linux)
		LDFLAGS += -rdynamic
	endif
endif

# Link
ifeq ($(TargetExt),)
	ifeq ($(Rpath),)
		Rpath = ../lib
	endif

	Link = g++ -Wl,-rpath,$(Rpath) $(LibraryPath) -o $@ $^ $(Library) $(LDFLAGS)
endif

ifeq ($(TargetExt),.so)
	Shared = -shared
	ifeq ($(UnixName),Darwin)
		Shared = -dynamiclib -install_name @rpath/$(Target)
	endif
	CFLAGS += -fPIC
	Link = g++ $(Shared) $(LibraryPath) $(Library) $(LDFLAGS) -o $@ $^
endif

ifeq ($(TargetExt),.a)
	CFLAGS += -fPIC
	Link = ar cr $@ $^
endif

CXXFLAGS += $(CFLAGS)

Obj = $(addsuffix .o,$(basename $(Source)))
Dep = $(addsuffix .d,$(basename $(Source)))

SourcePath = $(dir $(Source))
$(shell mkdir -p $(OutputPath) $(SourcePath:%=$(ObjectPath)/%))

define info
	@echo ">>>> $(UnixName).make:" $(Target) [$(Config)]
	@echo CC = $(CC) $(CFLAGS)
	@echo CXX = $(CXX) $(CXXFLAGS)
	@echo Link =  $(Link)
	@echo "<<<<"
endef

.PHONY: all test clean
all: test $(TargetFile) $(ExportHeaderPath)

clean:
	rm -rf $(ObjectPath)

test:
	$(info)

$(TargetFile): $(Obj:%=$(ObjectPath)/%)
	$(AfterCompile)
	@echo Link $@ ...
	@$(Link)

-include $(Dep:%=$(ObjectPath)/%)

$(ObjectPath)/%.o: %.c
	@echo CC $< ...
	@$(CC) $(CFLAGS) -o $@ -c $<

$(ObjectPath)/%.o: %.cpp
	@echo CXX $< ...
	@$(CXX) $(CXXFLAGS) -o $@ -c $<

$(ObjectPath)/%.d: %.c
	@$(CC) -MM $(CFLAGS) $< -MT $(@:%.d=%.o) -MT $@ > $@

$(ObjectPath)/%.d: %.cpp
	@$(CXX) -MM $(CXXFLAGS) $< -MT $(@:%.d=%.o) -MT $@ > $@

$(ExportHeaderPath): $(ExportHeader)
	if [ "$^" != "" ]; then mkdir -p $@ && cp $^ $@; fi
