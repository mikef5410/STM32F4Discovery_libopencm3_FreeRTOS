
## FreeRTOS Definitions
RTOS_SOURCE_DIR := $(PROJ_ROOT)/FreeRTOS/Source
FREERTOS_PORT := GCC/ARM_CM4F
FREERTOS_INC := -I $(RTOS_SOURCE_DIR)/include -I $(RTOS_SOURCE_DIR)/portable/MemMang -I $(RTOS_SOURCE_DIR)/portable/$(FREERTOS_PORT)
FREERTOS_SRCS := list.c queue.c tasks.c timers.c croutine.c port.c heap_3.c
FREERTOS_OBJS := $(call CALC_OBJS,$(FREERTOS_SRCS))

##############
#Build FreeRTOS into a library
##############
libFreeRTOS.a: INCLUDE += $(FREERTOS_INC)
libFreeRTOS.a: $(OBJS_DIR) libFreeRTOS.a($(FREERTOS_OBJS))
	$(RANLIB) $@

