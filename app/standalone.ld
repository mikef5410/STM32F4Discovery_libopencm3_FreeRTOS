/*
 * This file is part of the libopencm3 project.
 *
 * Copyright (C) 2009 Uwe Hermann <uwe@hermann-uwe.de>
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

/* Generic linker script for STM32 targets using libopencm3. */

/* Memory regions must be defined in the ld script which includes this one. */

MEMORY
{
  /* Define each memory region */
  RamLoc192 (rwx) : ORIGIN = 0x20000000, LENGTH = 128K
  FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 1M
}

REGION_ALIAS("RAM", RamLoc192);  /*Put everything here for now */
REGION_ALIAS("ram", RamLoc192);  /*Put everything here for now */
REGION_ALIAS("rom", FLASH);  /*Put everything here for now */

  /* Define a symbol for the top of each memory region */
  __top_RamLoc192 = 0x20000000 + 0x20000;
  __top_FLASH = 0x0 + 1M;

  __stack_size = 10240;

/* Enforce emmition of the vector table. */
EXTERN (vector_table)

/* Define the entry point of the output file. */
ENTRY(reset_handler)

/* Define sections. */
SECTIONS
{
	.text : {
		*(.vectors)	/* Vector table */
                *(.after_vectors*)
             	*(.text*)	/* Program code */
		. = ALIGN(4);
		*(.rodata*)	/* Read-only data */
		. = ALIGN(4);
	} >rom

	/* C++ Static constructors/destructors, also used for __attribute__
	 * ((constructor)) and the likes */
	.preinit_array : {
		. = ALIGN(4);
		__preinit_array_start = .;
		KEEP (*(.preinit_array))
		__preinit_array_end = .;
	} >rom
	.init_array : {
		. = ALIGN(4);
		__init_array_start = .;
		KEEP (*(SORT(.init_array.*)))
		KEEP (*(.init_array))
		__init_array_end = .;
	} >rom
	.fini_array : {
		. = ALIGN(4);
		__fini_array_start = .;
		KEEP (*(.fini_array))
		KEEP (*(SORT(.fini_array.*)))
		__fini_array_end = .;
	} >rom

	/*
	 * Another section used by C++ stuff, appears when using newlib with
	 * 64bit (long long) printf support
	 */
	.ARM.extab : {
		*(.ARM.extab*)
	} >rom
	.ARM.exidx : {
		__exidx_start = .;
		*(.ARM.exidx*)
		__exidx_end = .;
	} >rom

	. = ALIGN(4);
	_etext = .;

	.data : {
		_data = .;
		*(.data*)	/* Read-write initialized data */
		. = ALIGN(4);
		_edata = .;
	} >ram AT >rom
	_data_loadaddr = LOADADDR(.data);

	.bss : {
		*(.bss*)	/* Read-write zero initialized data */
		*(COMMON)
		. = ALIGN(4);
		_ebss = .;
	} >ram



        /* Set stack top to end of ram, and stack limit move down by
         * __stack_size 
         * Heap is everything between BSS and stack
         */
        __StackTop = ORIGIN(ram) + LENGTH(ram);
        __StackLimit = __StackTop - __stack_size;
        PROVIDE(__stack = __StackTop);
        PROVIDE(_stack = __StackTop);

        __heap_end = __StackLimit - 1;

        .heap (COPY):
        {
                __heap_start = .;
                __end__ = .;
                *(.heap*)
                . =  __heap_end;
        } > ram
     

        /* .stack_dummy section doesn't contains any symbols. It is only
         * used for linker to calculate size of stack sections, and assign
         * values to stack symbols later */
        .stack_dummy (COPY):
        {
           . = __StackLimit;
           . += __stack_size;
           *(.stack*)
        } > ram
   
        /* Check if data + heap + stack exceeds ram limit */
        ASSERT(__StackLimit >= __heap_end, "region ram overflowed with stack")

	 /* The .eh_frame section appears to be used for C++ exception handling.
	 * You may need to fix this if you're using C++.
	 */
	/*/DISCARD/ : { *(.eh_frame) }*/

	. = ALIGN(4);
	end = .;
}

/*PROVIDE(_stack = ORIGIN(ram) + LENGTH(ram));*/

