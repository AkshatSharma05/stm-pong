/*
 * Linker script for STM32F103C6T6
 *   Flash: 32 KB  starting at 0x08000000  (datasheet: "main memory" / Flash)
 *   RAM:   10 KB  starting at 0x20000000  (datasheet: SRAM)
 *
 * These two base addresses are fixed by the chip's bus architecture (the
 * Cortex-M3 memory map reserves the low 0.5GB region for code, and ST wires
 * its flash controller to appear at 0x08000000 within that region; SRAM
 * sits in the next 0.5GB region, "SRAM", starting at 0x20000000). The sizes
 * (32K / 10K) come from the C6T6 datasheet specifically — change these two
 * numbers if you ever swap to a C8T6 (64K / 20K).
 */

MEMORY
{
  FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 32K
  RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 10K
}

/* Top of RAM = highest valid address + 1. This value is what gets written
 * into the very first word of the vector table (the initial Main Stack
 * Pointer) — the stack grows DOWN from here on ARM/Cortex-M, so "top of RAM"
 * is the correct reset value, not the bottom. */
 
_estack = ORIGIN(RAM) + LENGTH(RAM);

SECTIONS
{
  /* The vector table MUST be the very first thing in flash, at 0x08000000,
   * because that is the fixed address the core reads on reset (PC is loaded
   * from offset 0x04, SP from offset 0x00, as in BOOT ROM). ">FLASH" places
   * this section at the start of the FLASH region defined above. */
  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector))   /* KEEP prevents the linker's garbage collector
                             * from discarding this — nothing in C ever
                             * "calls" the vector table by name, so without
                             * KEEP, --gc-sections would see it as unused
                             * and silently delete it. */
    . = ALIGN(4);
  } >FLASH

  /* All executable code and read-only constants (string literals, const
   * arrays, etc.) — placed in flash since it's read-only at runtime and
   * flash is non-volatile, which is exactly what code needs. */
  .text :
  {
    . = ALIGN(4);
    *(.text)
    *(.text*)
    *(.rodata)
    *(.rodata*)
    . = ALIGN(4);
  } >FLASH

  /* End of the flash-resident, load-time image. _etext marks where the
   * .data initializer bytes live IN FLASH (their "load address") — the
   * startup code's copy loop reads FROM here. */
  _etext = .;

  /* .data = globals/statics that have a nonzero initial value (e.g.
   * `int x = 5;`). The AT> clause is the key trick: it tells the linker
   * "the bytes for this section's initial content live in FLASH, but the
   * symbols/addresses inside it should be resolved as if it were in RAM."
   * This is what creates the split between Load Memory Address (LMA, in
   * flash) and Virtual Memory Address (VMA, in RAM) that startup.s's
   * copy loop relies on. */
  .data : AT (_etext)
  {
    . = ALIGN(4);
    _sdata = .;       /* start of .data, as it will live in RAM (VMA) */
    *(.data)
    *(.data*)
    . = ALIGN(4);
    _edata = .;       /* end of .data in RAM (VMA) */
  } >RAM

  /* _sidata = where .data's *initializer bytes* actually sit in flash.
   * Because of "AT(_etext)" above, this equals _etext numerically, but we
   * give it its own name for clarity in the startup code: "s_i_data" =
   * start, initializer, data. */
  _sidata = _etext;

  /* .bss = globals/statics with NO initializer, or explicitly initialized
   * to zero (e.g. `int y;` or `int y = 0;`). C standard requires these
   * start at zero. No initializer bytes are stored in flash at all here —
   * NOLOAD means this section occupies RAM address space but contributes
   * nothing to the flash image, since "all zero" needs no stored data,
   * only a zeroing loop at startup. */
  .bss (NOLOAD) :
  {
    . = ALIGN(4);
    _sbss = .;
    *(.bss)
    *(.bss*)
    *(COMMON)
    . = ALIGN(4);
    _ebss = .;
  } >RAM

  /* Everything from _ebss to top of RAM is left for the stack (growing
   * down from _estack) and, if you ever add one, a heap (growing up from
   * _ebss). Pong on this chip has no need for malloc, so no heap section
   * is defined — leaving it implicit and unused is intentional, not an
   * oversight. */
}
