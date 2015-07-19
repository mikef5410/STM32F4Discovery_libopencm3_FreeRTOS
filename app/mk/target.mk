

## LPC4330 Specific Definitions
ARCH = armv7e-m
CPU = cortex-m4
FLASHK ?= 512
RAMK ?= 192

# Which libc to use?
#
LIBC?=NEWLIB
#LIBC:=NANO
#DEBUG_LIBC:=/projects/gcc-arm-none-eabi-4_7-2013q1-20130313/install-native/arm-none-eabi/lib/armv7e-m/libc.a
# Switch FPU on/off based on which FreeRTOS port we use
#


ifeq ($(FREERTOS_PORT),GCC/ARM_CM4F)
	USE_FPU := yes
	FP := HARD
else
	USE_FPU := no
endif

ifeq ($(USE_FPU),yes)
ifeq ($(FP),HARD)
  CFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
  CPPFLAGS += -DCORTEX_USE_FPU=TRUE -D__TARGET_FPU_VFP=TRUE
else
  #Soft float ...
  CFLAGS += -mfloat-abi=softfp -mfpu=fpv4-sp-d16        
  CPPFLAGS += -DCORTEX_USE_FPU=FALSE
endif
endif

# WARNING WARNING ... if you use malloc, then you NEED startfiles!
LINKER_FLAGS=$(DEBUG_LIBC) -T$(LDSCRIPT) -Xlinker -o$@ -Xlinker -M -Xlinker -Map=$(patsubst %.elf,%.map,$@) -Xlinker --gc-sections
ifeq ($(LIBC),NANO)
LINKER_FLAGS += --specs=nano.specs
endif
