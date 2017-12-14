/* Include standard linker file for the STM32F303 */
INCLUDE "../Drivers/CMSIS/Device/ST/STM32F3xx/Source/Templates/gcc/linker/STM32F303XC_FLASH.ld"

/* Add our parse table data section to the end of the FLASH.  
 * The parser uses this data structure section to hold a table
 * of commands that the user has added in their C files.
 */
SECTIONS {
  .parsetable :
  {
    . = ALIGN(4);
    _parsetable_start = .; 
    KEEP(*(SORT(.parsetable.*)));
    LONG(0); LONG(0); LONG(0); /* Pad with 'last' record */
    _parsetable_end = .;
    . = ALIGN(4);
  } >FLASH
}
