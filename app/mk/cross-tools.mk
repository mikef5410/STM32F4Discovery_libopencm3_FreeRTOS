
TOOLS:=$(PROJ_ROOT)/tools
ARCH:=$(shell uname -i)
ifeq ($(ARCH),x86_64)
  TOOLSBIN:=$(TOOLS)/bin64
else
  TOOLSBIN:=$(TOOLS)/bin
endif

OPENOCDDIR:=/opt/oocd
TOOLDIR:=/opt/cross-arm-linaro
#TOOLDIR:=/opt/cross-arm-linaro-debuggable
PATH:=$(TOOLDIR)/bin:$(OPENOCDDIR)/bin:$(PATH)
OPENOCD=openocd
TOOLPREFIX=arm-none-eabi
CC=$(TOOLPREFIX)-gcc
CCP=$(TOOLPREFIX)-g++
CPP=$(TOOLPREFIX)-cpp
LD=$(TOOLPREFIX)-ld
NM=$(TOOLPREFIX)-nm
AR=$(TOOLPREFIX)-ar
RANLIB=$(TOOLPREFIX)-ranlib
OBJCOPY=$(TOOLPREFIX)-objcopy
OBJDUMP=$(TOOLPREFIX)-objdump
SIZE=$(TOOLPREFIX)-size
STRINGS=$(TOOLPREFIX)-strings
STRIP=$(TOOLPREFIX)-strip
SHELL=/bin/bash
PERL=/usr/bin/perl
CAT=/bin/cat
CP=/bin/cp
RM=/bin/rm
LEX=/usr/bin/lex
YACC=/usr/bin/yacc

#How many CPUs does our cross-compile machine have?
NCPU:=$(shell ls /sys/devices/system/cpu | grep -e 'cpu[0-9][0-9]*' | wc -l)

