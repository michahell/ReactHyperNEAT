
### Standard Makefile for Webots physics plugins
###
### Supported platforms: Windows, Mac OS X, Linux
### Supported languages: C, C++
### 
### Authors: Olivier Michel - www.cyberbotics.com
### Revised: Yvan Bourquin - September 30th, 2009.
###
### Uncomment the variables to customize the Makefile

### -----C Sources-----
###
### if your plugin uses several C source files:
# C_SOURCES = my_plugin.c my_clever_algo.c my_graphics.c

### -----C++ Sources-----
###
### if your plugin uses several C++ source files:
CPP_SOURCES = collision_detector.cpp
###     or
# CC_SOURCES = my_plugin.cc my_clever_algo.cc my_graphics.cc

### -----Options-----
###
### if special CFLAGS are necessary, for example to set optimization level or
### to find include files:
# CFLAGS=-O3 -I/my_library_path/include
###
### if your plugin needs additional libraries:
# LIBRARIES=-L/path/to/my/library -lmy_library -lmy_other_library

### Do not modify: this includes Webots global Makefile.include
space :=
space +=
WEBOTS_HOME_PATH=$(subst $(space),\ ,$(strip $(subst \,/,$(WEBOTS_HOME))))

### Generic Makefile.include for Webots physics plugins
### to be used with GNU make
###
### Platforms: Windows, Mac OS X, Linux
### Languages: C, C++
###
### Authors: Olivier Michel, Yvan Bourquin, Edmund Ronald
###
###-----------------------------------------------------------------------------
###
### This file is meant to be included from Makefiles stored in Webots physics
### plugin directories. To see how these Makefiles should be configured, please look
### at the template Makefile located in:
###   located in $(WEBOTS_HOME)/resources/plugins/physics/template/Makefile
###

# default commands definitions
NULL_DEVICE=/dev/null
PWD_COMMAND=pwd
RM_COMMAND=rm -f
LS_COMMAND=ls

# find out whether we are in a Windows Command Prompt
ifeq ($(OS),Windows_NT)
ifeq ($(TERM),)
OSTYPE=windows
NULL_DEVICE=nul
PWD_COMMAND=cd
RM_COMMAND=del /Q
LS_COMMAND=dir /B
endif
endif

ifndef OSTYPE
OSTYPE=$(shell uname)
endif

ifeq ($(OSTYPE),Linux)
OSTYPE=linux
endif

ifeq ($(OSTYPE),linux-gnu)
OSTYPE=linux
endif

ifeq ($(OSTYPE),)
OSTYPE=windows
endif

ifeq ($(OSTYPE),msys)
OSTYPE=windows
endif

ifneq ($(findstring MINGW,$(OSTYPE)),)
OSTYPE=windows
endif

ifneq ($(findstring CYGWIN,$(OSTYPE)),)
OSTYPE=windows
endif

ifeq ($(OSTYPE),darwin)
OSTYPE=Darwin
endif

ifndef DOS_CONSOLE
WINDOWS_APP=-mwindows
endif

ifeq ($(OSTYPE),windows)
PLATFORM_CFLAG=-DWIN32 -I"$(WEBOTS_HOME)\mingw\include" -I"$(WEBOTS_HOME)\mingw\lib\gcc\mingw32\4.4.0\include"
endif

ifeq ($(OSTYPE),linux)
PLATFORM_CFLAG=-DLINUX -fPIC
endif

ifeq ($(OSTYPE),Darwin)
PLATFORM_CFLAG=-DMACOS
ECHO_CMD=/bin/echo
else
ECHO_CMD=echo
endif

WEBOTS_HOME:=$(WEBOTS_HOME)

# compute the current directory, the CURDIR variable may be already set
ifndef CURDIR
CURDIR := $(shell $(PWD_COMMAND))
endif

# compute the name of the plugin from its directory (i.e. sample)
BAD_NAME = $(basename $(notdir $(CURDIR)))

# this is a hack to work around a problem with spaces in dir names
NAME = $(word $(words $(BAD_NAME)),$(BAD_NAME))

# if the source files were not explicitly defined, we assume that
# there is only one C or Java source file

ifndef C_SOURCES
C_SOURCES   =$(shell $(LS_COMMAND) $(NAME).c 2> $(NULL_DEVICE))
endif
ifndef CPP_SOURCES
CPP_SOURCES =$(shell $(LS_COMMAND) $(NAME).cpp 2> $(NULL_DEVICE))
endif
ifndef CC_SOURCES
CC_SOURCES  =$(shell $(LS_COMMAND) $(NAME).cc 2> $(NULL_DEVICE))
endif

INCLUDE=-I"$(WEBOTS_HOME)/include"

ifeq ($(OSTYPE),windows)
LIB_EXTENSION=.dll
else
ifeq ($(OSTYPE),Darwin)
LIB_EXTENSION=.dylib
else
LIB_EXTENSION=.so
endif
endif

# the objects files result from the C or C++ sources
C_OBJECTS   = $(C_SOURCES:.c=.o)
CPP_OBJECTS = $(CPP_SOURCES:.cpp=.o)
CC_OBJECTS  = $(CC_SOURCES:.cc=.o)

# set CC to your favorit C or C++ compiler
ifneq ($(C_SOURCES),)
CC=gcc
else
ifneq ($(CPP_SOURCES),)
CC=g++
else
ifneq ($(CC_SOURCES),)
CC=g++
else
CC=unknow_compiler
endif
endif
endif

ifneq ($(OSTYPE),windows)
PLATFORM_LIBS=
else
PLATFORM_LIBS=-lmingw32 $(WINDOWS_APP) -B"$(WEBOTS_HOME)\mingw\lib\gcc\mingw32\4.4.0"
endif

LIBRARY=$(PLATFORM_LIBS) $(LIBRARIES)

ifeq ($(OSTYPE),windows)
CFLAGS_WIN=$(WINDOWS_APP)
WINSTUFF=-L"$(WEBOTS_HOME)/lib" -lode -lOpenGL32
else
WINSTUFF=
endif

C_SOURCES+=$(WEBOTS_HOME_PATH)/resources/plugins/physics/physics.c

# set the C compiler options
CC_OPTS=-c $(INCLUDE) -Wall $(CFLAGS) $(CFLAGS_WIN) $(PLATFORM_CFLAG)

ifeq ($(OSTYPE),Darwin)
CC_OPTS +=-mmacosx-version-min=10.9 -arch i386
LINK_OPTS=-mmacosx-version-min=10.9 -arch i386 -dynamiclib -flat_namespace -undefined suppress -install_name $(NAME).dylib -compatibility_version 1.0 -current_version 1.0.0
else
LINK_OPTS=-shared
endif

TARGET=$(NAME)$(LIB_EXTENSION)

all: $(TARGET)
compil: all

# rule to make automatic dependencies
# adapted from the GNU make info file

# this one is special for the physics.c file
$(WEBOTS_HOME_PATH)/resources/plugins/physics/physics.d:$(WEBOTS_HOME_PATH)/resources/plugins/physics/physics.c
	$(CC) $(INCLUDE) $(CFLAGS) $(PLATFORM_CFLAG) -MM -MF $(WEBOTS_HOME_PATH)/resources/plugins/physics/physics.d-tmp $< > $@
	$(ECHO_CMD) -n $(WEBOTS_HOME_PATH)/resources/plugins/physics/ > $(WEBOTS_HOME_PATH)/resources/plugins/physics/physics.d
	cat $(WEBOTS_HOME_PATH)/resources/plugins/physics/physics.d-tmp >> $(WEBOTS_HOME_PATH)/resources/plugins/physics/physics.d
	rm $(WEBOTS_HOME_PATH)/resources/plugins/physics/physics.d-tmp

%.d:%.c
	$(CC) $(INCLUDE) $(CFLAGS) $(PLATFORM_CFLAG) -MM $< > $@

%.d:%.cc
	$(CXX) $(INCLUDE) $(CFLAGS) $(PLATFORM_CFLAG) -MM $< > $@

%.d:%.cpp
	$(CXX) $(INCLUDE) $(CFLAGS) $(PLATFORM_CFLAG) -MM $< > $@

# include dependencies file
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(C_SOURCES)$(CC_SOURCES)$(CPP_SOURCES),)
-include $(C_SOURCES:.c=.d) $(CC_SOURCES:.cc=.d) $(CPP_SOURCES:.cpp=.d)
endif
endif

# rules to make shared lib from C/C++ objects
$(NAME)$(LIB_EXTENSION): $(C_OBJECTS) $(CC_OBJECTS) $(CPP_OBJECTS)
	$(CC) $(LINK_OPTS) -o $(NAME)$(LIB_EXTENSION) $(C_OBJECTS) $(CC_OBJECTS) $(CPP_OBJECTS) $(LIBRARY) $(WINSTUFF)
	$(EXTRA_FINAL_CMD)

# generic rule to make the object files
%.o:%.c
	$(CC) $(CC_OPTS) $< -o $@

%.o:%.cpp
	$(CC) $(CC_OPTS) $<

%.o:%.cc
	$(CC) $(CC_OPTS) $<

# how to clean up the directory
clean:
	@-$(RM_COMMAND) *.o *.so *.d *~ $(NAME)$(LIB_EXTENSION) *.opt *.ncb *.obj *.plg *.exp *.lib vc60.idb core

# show the ostype recognized by the Makefile
ostype:
	@echo OSTYPE=$(OSTYPE)
