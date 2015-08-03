#
# Make recipes for various tags programs ...
#
############
# Source indexing with cscope, idutils and etags 
############
cscope id etags tags: FILETYPES:=-iname "*.cpp" -o -iname "*.hpp" -o -name "*.S" -o -iname "*.c" -o -iname "*.h" -o -name "*.s" -o -name "*.cc"
cscope id etags tags: INDEX_FILES:=$(filter-out ./board.h,$(shell (find  -L . $(RTOS_SOURCE_DIR) $(LIBOPENCM3_DIR) $(FILETYPES))))


cscope: 
	@echo cscope ...
	@cscope -f cscope.out -q -b -k $(INDEX_FILES)

id: 
	@echo idutils ...
	@rm -f ID
	@mkid -s --output=ID $(INDEX_FILES)
	export IDPATH=`pwd`/ID;

etags:
	@echo etags ...
	@rm -f TAGS
	@echo $(INDEX_FILES) | xargs etags --output=TAGS --append

tags: cscope id etags

cleantags:
	rm -f cscope.out* ID TAGS
