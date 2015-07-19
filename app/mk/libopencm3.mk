
## libopencm3 Definitions

LIBOPENCM3_TARGET := stm32/f4

LIBOPENCM3_DIR := $(PROJ_ROOT)/external/libopencm3

LIBOPENCM3_INC :=  -I$(LIBOPENCM3_DIR)/include 

LIBOPENCM3_LIBNAME := libopencm3_$(subst /,,$(LIBOPENCM3_TARGET)).a

##############
#Build FreeRTOS into a library
##############
$(LIBOPENCM3_LIBNAME): $(LIBOPENCM3_DIR)/lib/$(LIBOPENCM3_LIBNAME)
	$(CP) $< .
	$(RANLIB) $@


$(LIBOPENCM3_DIR)/lib/$(LIBOPENCM3_LIBNAME):
	$(MAKE) -C $(LIBOPENCM3_DIR) lib/$(LIBOPENCM3_TARGET)


clean::
	$(MAKE) -C $(LIBOPENCM3_DIR) clean
	rm -f $(LIBOPENCM3_LIBNAME)

