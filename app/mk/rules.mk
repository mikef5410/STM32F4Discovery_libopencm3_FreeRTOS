
ifeq ($(SILENT),yes)
.SILENT: $(PROG).elf $(LOADER).elf
endif

$(OBJS_DIR)/%.o : %.c
ifeq ($(SILENT),yes)	
	@echo "Compiling $< ..."
	@$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@
else
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@
endif

($(OBJS_DIR)/%.o): %.c
ifeq ($(SILENT),yes)
	@echo "Compiling $< ..."
	@$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $*.o
	@$(AR) r $@ $*.o
	@$(RM) $*.o
else
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $*.o
	$(AR) r $@ $*.o
	$(RM) $*.o
endif
