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

/* Generic linker script for pretty much any cortex-m targets using libopencm3. */

/* Memory regions MUST be defined in the ld script which includes this one. */


/* Define the entry point of the output file. */
ENTRY(Reset_Handler)

/* Define sections. */
SECTIONS
{
  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector)) 
    . = ALIGN(4);
  } >rom

	.text : {
		. = ALIGN(4);
		*(.text)						/* Program code */
		*(.text*)						/* Program code */
    *(.glue_7)          /* glue arm to thumb code */
    *(.glue_7t)         /* glue thumb to arm code */
    *(.eh_frame)

		KEEP (*(.init))
		KEEP (*(.fini))

		. = ALIGN(4);
		_etext = .;
	} >rom

	.rodata :
	{
		. = ALIGN(4);
	  *(.rodata)					/* Read-only data */
	  *(.rodata*)					/* Read-only data */
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
	.ARM.extab : 
	{
		*(.ARM.extab* .gnu.linkonce.armextab.*)
	} >rom

	.ARM.exidx : 
	{
		__exidx_start = .;
		*(.ARM.exidx*)
		__exidx_end = .;
	} >rom


	/* ram, but not cleared on reset. eg:  boot/app comms */
	.noinit (NOLOAD) : {
		*(.noinit)
	} >ram
	. = ALIGN(4);

	/* 2K section at the end of flash for storing motor controller settings*/
	.mc_settings (NOLOAD) :
	{
		. = ALIGN(4);
		FILL(0xFF)
		*(.mc_settings)
		*(.mc_settings*)
		. = ALIGN(4);
	} >mc_settings =0xFF
	
  _sidata = LOADADDR(.data);

	.data : {
		. = ALIGN(4);
		_sdata = .;

		*(.data)					/* Read-write initialized data */
		*(.data*)					/* Read-write initialized data */
		*(.ramfunctions) 	/* functions we need/want to run from ram */

		. = ALIGN(4);
		_edata = .;
	} >ram AT >rom

  _siccmram = LOADADDR(.ccmram);

  .ccmram :
  {
    . = ALIGN(4);
    _sccmram = .;       /* create a global symbol at ccmram start */
    *(.ccmram)
    *(.ccmram*)
    
    . = ALIGN(4);
    _eccmram = .;       /* create a global symbol at ccmram end */
  } >ccmram AT> rom 

	. = ALIGN(4);
	.bss : {
    _sbss = .;
		__bss_start__ = _sbss;
		*(.bss)				/* Read-write zero initialized data */
		*(.bss*)			/* Read-write zero initialized data */
		*(COMMON)

		. = ALIGN(4);
		_ebss = .;
    __bss_end__ = _ebss;
	} >ram

  /* User_heap_stack section, used to check that there is enough RAM left */
  ._user_heap_stack :
  {
    . = ALIGN(8);
    PROVIDE ( end = . );
    PROVIDE ( _end = . );
    . = . + _Min_Heap_Size;
    . = . + _Min_Stack_Size;
    . = ALIGN(8);
  } >ram

  .ARM.attributes 0 : { *(.ARM.attributes) }
}