#Target  = appserver
#Include = ../lua ../common
#Source	= main.cpp
#LibDir	= ../lib
#Lib 	= lua common
#ObjDir	= ../obj
#OutDir	= ../bin
#Define 	=
#CFLAGS	=
#CXXFLAGS =
#LDFLAGS =
#Rpath =

#CC = gcc
#CXX = g++
CFLAGS += $(Include:%=-I%) $(Define:%=-D%) -m64

# Config: Debug/Release
Config ?= Debug

UnixName = $(shell uname)

ifeq ($(Config),Release)
CFLAGS += -O3 -march=native
else
CFLAGS += -g -D_DEBUG -Wall
	ifeq ($(UnixName),Linux)
	CFLAGS += -rdynamic
	endif
endif

ifeq ($(Rpath),)
Rpath = ./
endif

ifeq ($(suffix $(Target)),)
Link = g++ -Wl,-rpath,$(Rpath) $(LibDir:%=-L%) -o $@ $^ $(Lib:%=-l%) $(LDFLAGS)
endif

ifeq ($(suffix $(Target)),.so)

Shared = -shared
ifeq ($(UnixName),Darwin)
Shared = -dynamiclib -install_name @rpath/$(Target)
endif
CFLAGS += -fPIC
Link = g++ $(Shared) $(LibDir:%=-L%) -o $@ $^ $(Lib:%=-l%) $(LDFLAGS)

endif

ifeq ($(suffix $(Target)),.a)
CFLAGS += -fPIC
Link = ar cr $@ $^
endif

CXXFLAGS += $(CFLAGS) -std=c++14

TargetPath = $(OutDir)/$(Target)
ObjDirPath = $(ObjDir)/$(basename $(Target))

Obj = $(addsuffix .o,$(basename $(Source)))
Dep = $(addsuffix .d,$(basename $(Source)))

SourceDir = $(dir $(Source))
$(shell mkdir -p $(OutDir) $(ObjDirPath) $(SourceDir:%=$(ObjDirPath)/%))

define info
	@echo ">>>> $(UnixName).make:" $(Target) [$(Config)]
	@echo CC = $(CC) $(CFLAGS)
	@echo CXX = $(CXX) $(CXXFLAGS)
	@echo Link =  $(Link)
	@echo "<<<<"
endef

.PHONY: all start test clean
all: start $(TargetPath)

test: start

clean:
	rm -rf $(ObjDirPath)

start:
	$(info)

$(TargetPath): $(Obj:%=$(ObjDirPath)/%)
	$(AfterCompile)
	@echo Link $@ ...
	@$(Link)
	$(AfterBuild)

-include $(Dep:%=$(ObjDirPath)/%)

$(ObjDirPath)/%.o: %.c
	@echo CC $< ...
	@$(CC) $(CFLAGS) -o $@ -c $<

$(ObjDirPath)/%.o: %.cpp
	@echo CXX $< ...
	@$(CXX) $(CXXFLAGS) -o $@ -c $<

$(ObjDirPath)/%.d: %.c
	@$(CC) -MM $(CFLAGS) $< -MT $(@:%.d=%.o) -MT $@ > $@

$(ObjDirPath)/%.d: %.cpp
	@$(CXX) -MM $(CXXFLAGS) $< -MT $(@:%.d=%.o) -MT $@ > $@
