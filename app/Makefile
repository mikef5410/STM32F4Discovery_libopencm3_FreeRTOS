# MAKEFILE NOTES
# := <rule>   expanded ONCE
#  = <rule>   expanded whenever used
PROJ_ROOT=..

#make SILENT=no for a verbose build
SILENT?=no

#default BLD_CONFIG to RELEASE
#BLD_CONFIG?=RELEASE
BLD_CONFIG?=DEBUG
ifeq ($(BLD_CONFIG),RELEASE)
	CFLAGS += -DRELEASE
endif

OBJS_DIR = build
CALC_OBJS = $(addprefix $(OBJS_DIR)/, $(addsuffix .o, $(basename $(notdir $(1)))))
CALC_OBJS2 = $(addsuffix .o, $(basename $(notdir $(1))))

# Target names
PROG:=APP

#CFLAGS+=-ggdb -O0 -std=gnu99  -ffunction-sections -fdata-sections
CFLAGS+=-O2 -D__STM32__ -DSTM32F4 -ggdb -std=gnu99 -fomit-frame-pointer -falign-functions=16 -ffunction-sections -fdata-sections

#CFLAGS+= -fshort-wchar -mabi=aapcs -mapcs-frame
WARNINGS:= -Wall -Wextra
WARNINGS+= -Werror
INCLUDE=-I . 

CFLAGS += -mcpu=$(CPU) -march=$(ARCH) -mthumb $(INCLUDE) $(WARNINGS) 

all: $(OBJS_DIR) $(PROG).bin 

include mk/cross-tools.mk
include mk/freertos.mk
include mk/libopencm3.mk
include mk/target.mk

VPATH=.:./CException:$(RTOS_SOURCE_DIR):$(RTOS_SOURCE_DIR)/portable/MemMang
VPATH+=:$(RTOS_SOURCE_DIR)/portable/$(FREERTOS_PORT)



$(OBJS_DIR):
	mkdir -p $@


##############
# Primary Target (Application) Build
##############

APP_SRCS = main.c syscalls.c printf-stdarg.c cdcacm.c debug_shell.c command.c hiresTimer.c

APP_LIBS = libFreeRTOS.a $(LIBOPENCM3_LIBNAME)
APP_INCLUDE = $(FREERTOS_INC) $(LIBOPENCM3_INC)
APP_OBJS = $(call CALC_OBJS,$(APP_SRCS))


$(PROG).elf : LDSCRIPT := standalone.ld
$(PROG).elf : INCLUDE += $(APP_INCLUDE)
$(PROG).elf : CFLAGS += $(APP_CFLAGS)
$(PROG).elf : $(OBJS_DIR) $(APP_LIBS) $(APP_OBJS) Makefile
	@echo Linking $(PROG).elf ...
	$(CC) $(CFLAGS) $(APP_OBJS) $(APP_LIBS) $(LINKER_FLAGS)
	$(SIZE) $(PROG).elf
	$(OBJDUMP) -dSx $(PROG).elf > $(PROG).asm

$(PROG).bin: $(PROG).elf
	$(OBJCOPY) -O binary $(PROG).elf $(PROG).bin

download: $(PROG).bin
	st-flash --reset write $(PROG).bin 0x08000000


include mk/tags.mk

clean:: cleantags
	rm -f $(OBJS) *.elf *.a *.map *.asm \#* *~ *.o *.elf *.bin *.hdr version.h
	rm -f ../tools/version_print
	rm -rf build/
	rm -f build_*

include mk/rules.mk
