# Bare Metal STM32 — Architecture Notes & Project Walkthrough

Target: STM32F103C6T6 (Blue Pill)  
Goal: Pong on an SH1106 OLED, bare metal C and assembly, no HAL, no CMSIS  
Toolchain: arm-none-eabi-gcc, make, st-flash, openocd

---

## Part 1 — ARM Cortex-M3 Architecture

### What kind of processor this is

The Cortex-M3 is ARM's 32-bit microcontroller core, designed specifically for
embedded systems. It is not a full application processor (no MMU, no Linux,
no cache). It runs one instruction stream, has 13 general-purpose registers
plus SP/LR/PC, and boots directly into your code with no operating system
underneath.

Key facts:

- 32-bit registers, 32-bit address space (4GB address space, though most of
  it is not wired to anything on your chip)
- **Thumb-2 instruction set only** — there is no 32-bit ARM mode on Cortex-M.
  Every instruction is either 16-bit (Thumb) or 32-bit (Thumb-2 extension).
  This is why `.thumb` is mandatory in your assembly files.
- **Von Neumann-ish, Harvard-ish** — the M3 actually has separate instruction
  and data buses internally (Harvard-like), but they map into the same flat
  address space (Von Neumann-like). You can read data from flash addresses and
  code from RAM addresses — the distinction is logical, not physical.
- **Little-endian** by default. Byte 0 of a 32-bit word is at the lowest address.

### The register file

```
r0  - r3    scratch / function arguments and return values (caller-saved)
r4  - r11   callee-saved (a function must preserve these across a call)
r12         scratch (IP — intra-procedure call scratch, rarely needed manually)
r13 (SP)    Stack Pointer — always points to top of current stack frame
r14 (LR)    Link Register — holds return address after a bl instruction
r15 (PC)    Program Counter — address of currently executing instruction
xPSR        status flags: N (negative), Z (zero), C (carry), V (overflow)
```

`r0–r3` are the ones you use freely in startup code without worrying about
saving them first. `r4–r11` must be pushed/popped if a function uses them.
This contract is ARM's AAPCS (procedure call standard) — the compiler follows
it, and your assembly must too when calling into or being called from C.

### The fixed memory map

ARM defines how the 4GB address space is divided across all Cortex-M chips.
This is architectural — the same on every vendor's Cortex-M3 part:

```
0x00000000 - 0x1FFFFFFF   Code region        (flash typically mapped here)
0x20000000 - 0x3FFFFFFF   SRAM region        (RAM lives here)
0x40000000 - 0x5FFFFFFF   Peripheral region  (all hardware registers)
0xE0000000 - 0xFFFFFFFF   System region      (NVIC, SysTick, debug hardware)
```

ST's job as a chip vendor was to decide where *within* the code region their
flash appears. They chose `0x08000000`. The chip also aliases `0x08000000`
to `0x00000000` via the BOOT pins (Blue Pill has them wired for flash boot),
which is why the core can read the vector table at address `0x00000000` on
reset even though your linker script places everything at `0x08000000`.

Your STM32F103C6T6 specifics:

```
Flash:  32 KB   at 0x08000000
RAM:    10 KB   at 0x20000000
```

### How the chip boots — no BIOS, no bootloader

On reset, the Cortex-M3 core performs exactly two hardware reads, from fixed
addresses, before executing any instruction:

1. Read the 32-bit word at `0x00000000` (offset 0) → load into **MSP** (Main Stack Pointer)
2. Read the 32-bit word at `0x00000004` (offset 4) → load into **PC**, start executing

That is the complete boot sequence at the hardware level. There is no
firmware doing this for you. The implication is unavoidable: the very first
thing in your flash image must be a valid stack pointer value followed by a
valid reset handler address. This is what the vector table is.

### The vector table

The vector table is an array of 32-bit addresses sitting at the start of
flash. It is data, not code — the core reads it, not executes it.

```
Offset 0x00   Initial Stack Pointer value  (special — loaded into MSP, not jumped to)
Offset 0x04   Reset_Handler address        (jumped to after MSP is loaded)
Offset 0x08   NMI_Handler address
Offset 0x0C   HardFault_Handler address
...
Offset 0x40   IRQ0 handler address         (first vendor-specific peripheral interrupt)
Offset 0x44   IRQ1 handler address
...
```

Slots 0–15 are ARM-defined (same position on every Cortex-M3 from any
vendor). Slots 16+ are vendor-defined (ST assigns specific peripherals to
specific slots in the STM32F103 reference manual — the slot number is the
only thing that maps an interrupt to a peripheral, there is no name lookup).

Every address in the table has its bit 0 forced to 1 by the
assembler/linker. This is the Thumb mode indicator — Cortex-M3 only runs
Thumb code, and the architecture requires bit 0 of any branch target to be
1 to signal this. You never write this manually; the assembler handles it
automatically when you use a label reference.

### Peripheral access — memory-mapped I/O

There are no special instructions for talking to hardware on Cortex-M3. Every
peripheral (GPIO, SPI, ADC, timers...) is accessed by reading and writing
specific 32-bit addresses in the peripheral region (`0x40000000+`). The
hardware interprets reads/writes to those addresses as register operations.

```c
// This is all it takes to write a hardware register:
*(volatile uint32_t *)0x40011000 = 0x33300000;
```

`volatile` is mandatory — without it, the C compiler can legally optimize
away "redundant" reads/writes to the same address, which is catastrophically
wrong for hardware registers that have side effects on every access.

### Clock gating

Every peripheral on STM32F1 is powered down by default. The RCC (Reset and
Clock Control) peripheral controls which peripherals receive a clock signal.
Until you enable a peripheral's clock bit in RCC, writes to its registers are
silently ignored by the bus fabric. This is the single most common source of
confusion during bringup:

```
Enable GPIOC clock in RCC → GPIOC registers become live → configure pin → it works
Skip RCC step             → GPIOC register writes silently do nothing → nothing works
```

### The stack

The Cortex-M3 uses a "full descending" stack:

- The stack pointer starts at a high address (top of RAM)
- Every push **decrements** SP first, then stores
- Every pop reads first, then **increments** SP

This means the initial SP value in the vector table should point one address
past the end of RAM (`ORIGIN(RAM) + LENGTH(RAM)`). The first push will
decrement it into valid RAM before writing.

The stack is used for: function call return addresses (pushed by `bl`),
local variables, saved registers, and interrupt state (the core pushes
r0–r3, r12, LR, PC, xPSR automatically on interrupt entry — this is called
the "exception frame").

---

## Part 2 — The C Runtime Environment

Before `main()` can run, three things must be true:

1. The stack must exist and SP must point to valid RAM (done by the vector table)
2. Initialized globals (`int x = 5;`) must have their initial values in RAM
3. Zero-initialized globals (`int x;`) must actually be zero in RAM

None of these happen automatically in hardware. Your startup code does all of
them. Understanding why requires understanding how the compiler and linker
handle global variables.

### The flash/RAM split problem

The compiler puts initialized global variables' *initial values* in flash
(they must survive power-off). But at runtime, those variables must live in
RAM (they must be writable). These are two different addresses.

The linker handles this by giving `.data` two addresses:

- **LMA** (Load Memory Address) — where the initial bytes are stored in flash
- **VMA** (Virtual Memory Address) — where the variable is accessed from at runtime

Your startup code's copy loop bridges the gap: it copies the bytes from the
LMA (in flash) to the VMA (in RAM) before `main()` runs. After that copy,
writing to `x` in C writes to RAM, while the original value safely remains
in flash for the next power cycle.

`.bss` (zero-initialized data) has no LMA at all — there's nothing to store
in flash for a block of zeros. The startup code just zeroes that RAM region
directly.

---

## Part 3 — Project File Walkthrough

### File dependency order

Each file depends on decisions made in the previous one. Write them out of
order and you'll be back-patching constantly:

```
linker/stm32f103c6.ld       defines memory regions and symbol names
    ↓
startup/startup_stm32f103.s  uses those symbols; defines Reset_Handler and vector table
    ↓
include/regs.h               defines hardware register addresses for the peripherals we touch
    ↓
src/main.c                   uses regs.h; the first application code
    ↓
Makefile                     ties everything together
```

---

### `linker/stm32f103c6.ld`

**What it does:** tells the linker where physical memory exists on this chip
and how to arrange the compiled output into it.

**`MEMORY` block** — declares the two physical memory regions with their
hardware-fixed base addresses and part-specific sizes:

```ld
MEMORY
{
  FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 32K
  RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 10K
}
```

**`_estack`** — computes the initial stack pointer value: top of RAM.
The stack grows down, so this is the correct starting point:

```ld
_estack = ORIGIN(RAM) + LENGTH(RAM);  /* = 0x20002800 */
```

**`.isr_vector` section** — forces the vector table to be the very first
bytes in flash. `KEEP` prevents `--gc-sections` from deleting it (nothing
in C "calls" the vector table by name, so the linker would otherwise
consider it unreferenced and drop it):

```ld
.isr_vector : { KEEP(*(.isr_vector)) } >FLASH
```

**`.text` section** — all compiled code and `const` data, placed in flash:

```ld
.text : { *(.text) *(.text*) *(.rodata) *(.rodata*) } >FLASH
_etext = .;   /* marks end of flash image / start of .data initializers */
```

**`.data` section** — the LMA/VMA split. `AT(_etext)` places the initializer
bytes in flash right after code. `>RAM` makes all symbol addresses resolve
into RAM. The startup copy loop reads from `_sidata` (flash) and writes to
`_sdata.._edata` (RAM):

```ld
.data : AT (_etext) { _sdata = .; *(.data) *(.data*) _edata = .; } >RAM
_sidata = _etext;
```

**`.bss` section** — zero-initialized data. `NOLOAD` means no bytes in the
flash binary. Startup code zeroes `_sbss.._ebss` at runtime:

```ld
.bss (NOLOAD) : { _sbss = .; *(.bss) *(.bss*) *(COMMON) _ebss = .; } >RAM
```

---

### `startup/startup_stm32f103.s`

**What it does:** lays out the vector table in flash and implements
`Reset_Handler` — the first code the chip executes after power-on.

**Assembler directives at the top:**

```asm
.syntax unified   /* modern ARM unified syntax, dest-first operand order */
.cpu cortex-m3    /* target instruction set */
.thumb            /* emit Thumb encoding — mandatory, M3 has no ARM mode */
```

**The vector table** is placed in `.isr_vector` (which the linker script pins
to `0x08000000`). It is an array of `.word` directives — data, not
instructions:

```asm
.section .isr_vector, "a", %progbits
g_vector_table:
    .word _estack           /* slot 0: initial SP — loaded into MSP by hardware */
    .word Reset_Handler     /* slot 1: reset — PC is set to this address */
    .word NMI_Handler       /* slot 2: non-maskable interrupt */
    .word HardFault_Handler /* slot 3: hard fault */
    /* ... slots 4-15: remaining ARM-architected exceptions ... */
    /* ... slots 16-58: STM32F103-specific peripheral IRQs ... */
```

Slots 16+ (peripheral IRQs) currently all point to `Default_Handler` (an
infinite loop). When a peripheral interrupt is needed later, replace its
`.word` with the real handler's label. Position in the table is fixed by
the reference manual.

**`Reset_Handler`** — three steps, executed once on every power-on/reset:

```asm
Reset_Handler:
    /* Step 1: copy .data initializers from flash to RAM */
    ldr   r0, =_sidata      /* source: flash address of initializer bytes */
    ldr   r1, =_sdata       /* destination: RAM address where .data lives */
    ldr   r2, =_edata       /* end of destination region */
copy_data_loop:
    cmp   r1, r2
    bge   copy_data_done
    ldr   r3, [r0]          /* load word from flash */
    str   r3, [r1]          /* store word to RAM */
    adds  r0, r0, #4
    adds  r1, r1, #4
    b     copy_data_loop
copy_data_done:

    /* Step 2: zero .bss */
    ldr   r0, =_sbss
    ldr   r1, =_ebss
    movs  r2, #0
zero_bss_loop:
    cmp   r0, r1
    bge   zero_bss_done
    str   r2, [r0]
    adds  r0, r0, #4
    b     zero_bss_loop
zero_bss_done:

    /* Step 3: call main() */
    bl    main

hang_after_main:
    b     hang_after_main   /* trap if main() ever returns */
```

**`.weak` aliases** — every exception handler other than `Reset_Handler` is
declared weak, aliased to `Default_Handler`. This lets you define e.g.
`void SysTick_Handler(void)` in a C file and the linker will automatically
prefer your strong C definition over the weak assembly stub:

```asm
.weak SysTick_Handler
.thumb_set SysTick_Handler, Default_Handler
```

---

### `include/regs.h`

**What it does:** maps peripheral register addresses to C macros, one
peripheral at a time as needed. No CMSIS, no vendor headers — every address
comes directly from the STM32F103 reference manual.

The macro pattern:

```c
#define REG_NAME  (*(volatile uint32_t *)(BASE_ADDR + OFFSET))
```

Milestone 1 needs only RCC (to enable the GPIOC clock) and GPIOC (to
configure and toggle PC13):

```c
/* RCC — Reset and Clock Control, base 0x40021000 */
#define RCC_APB2ENR         (*(volatile uint32_t *)(0x40021000 + 0x18))
#define RCC_APB2ENR_IOPCEN  (1U << 4)   /* bit 4 = GPIOC clock enable */

/* GPIOC — base 0x40011000 */
#define GPIOC_CRH   (*(volatile uint32_t *)(0x40011000 + 0x04))  /* pin config, pins 8-15 */
#define GPIOC_ODR   (*(volatile uint32_t *)(0x40011000 + 0x0C))  /* output data register */
```

Future milestones add registers here as needed — SPI1, ADC1, SysTick, EXTI,
etc. Never transcribe the whole reference manual up front; add only what
you're about to use.

---

### `src/main.c`

**What it does:** the first application code. Three things in sequence —
enable GPIOC clock, configure PC13 as push-pull output, blink forever.

**Step 1 — enable GPIOC clock:**

```c
RCC_APB2ENR |= RCC_APB2ENR_IOPCEN;
```

Read-modify-write with `|=` so other bits in APB2ENR aren't disturbed.
After this line, GPIOC register writes are live.

**Step 2 — configure PC13 as output:**

STM32F1 GPIO config is unusual: each pin has a 4-bit field in `CRL` (pins
0–7) or `CRH` (pins 8–15), encoding direction and output type together.
PC13 → `CRH` bits `[23:20]`. Target value: `CNF=00` (push-pull),
`MODE=11` (output 50MHz) → field value `0b0011 = 0x3`.

Always clear then set — never write the whole register:

```c
GPIOC_CRH &= ~(0xFU << 20);   /* clear 4-bit field at [23:20] */
GPIOC_CRH |=  (0x3U << 20);   /* write CNF=00, MODE=11 */
```

**Step 3 — blink:**

PC13 is active-low on Blue Pill (LED anode to 3.3V, cathode through
resistor to PC13). Low = LED on, high = LED off:

```c
for (;;) {
    GPIOC_ODR &= ~(1U << 13);   /* PC13 low  → LED on  */
    delay(400000);
    GPIOC_ODR |=  (1U << 13);   /* PC13 high → LED off */
    delay(400000);
}
```

The `delay()` function is a calibrated-by-eye busy-wait — `volatile` on the
parameter prevents the compiler from optimizing the loop away. It gets
replaced by a SysTick-based millisecond counter in milestone 2.

---

### `Makefile`

**What it does:** defines the build pipeline from source files to a flashable
binary, and drives st-flash and openocd.

Key compiler flags and why they exist:

| Flag | Reason |
|---|---|
| `-mcpu=cortex-m3` | Generate Cortex-M3 Thumb-2 instructions specifically |
| `-mthumb` | Emit Thumb encoding — M3 has no ARM mode |
| `-ffreestanding` | No hosted environment, no libc assumptions |
| `-nostdlib` | Don't link libc/crt0 — we are the runtime startup |
| `-O0` | No optimization during bringup — delay loops behave as written |
| `-g` | DWARF debug info in .elf — zero cost in .bin, needed for gdb |
| `-Iinclude` | Find regs.h and other headers without path prefixes |
| `--gc-sections` | Drop unreferenced code/data — matters on 32KB flash |
| `-Map=build/pong.map` | Linker map — essential for debugging size/address issues |

Key targets:

```
make          build .elf and .bin, print flash/RAM usage via arm-none-eabi-size
make flash    build then: st-flash write build/pong.bin 0x08000000
make debug    start openocd -f tools/openocd.cfg (blocks; run gdb in second terminal)
make clean    rm -rf build/
```

`$(wildcard src/*.c)` means adding new `.c` files to `src/` is automatically
picked up — no Makefile edits needed until a new *directory* is added.

---

## Part 4 — Milestone Status

### Milestone 1: Blink LED ✓ (implemented)

Files written:
- `linker/stm32f103c6.ld` — memory map and section layout
- `startup/startup_stm32f103.s` — vector table and Reset_Handler
- `include/regs.h` — RCC and GPIOC register definitions
- `src/main.c` — clock enable, GPIO config, blink loop
- `Makefile` — build, flash, debug targets

What this proves when it works on hardware:
- Linker script memory map is correct
- Vector table is at the right address and contains valid SP and PC values
- Reset_Handler correctly copies .data and zeroes .bss before calling main()
- RCC clock enable works
- GPIO CRH configuration works
- ODR write drives real voltage on the pin
- st-flash pipeline works end to end

### Milestone 2: SysTick (next)

Replace `delay()` busy-wait with a proper 1ms tick counter driven by the
SysTick peripheral (built into the Cortex-M3 core, no vendor-specific
setup). Adds `src/system.c` and `SysTick_Handler` in C (overrides the weak
asm alias automatically). All subsequent milestones use `delay_ms()` from
this module.

### Milestone 3: SPI1 + SH1106 OLED init

Bring up SPI1 in master mode via register-level config. Send the SH1106
initialization command sequence. Draw a static test pattern to confirm
page/column addressing is correct before building the framebuffer layer.

### Milestone 4: Framebuffer

128×64 1bpp buffer in RAM (1KB). Draw into it in software, blit to display
each frame over SPI. First candidate for hand-written assembly (the blit
loop is a tight, well-bounded inner loop — ideal for Thumb-2 asm practice).

### Milestone 5: Input

Polled GPIO buttons first (simplest, no interrupt complexity). ADC1
single-channel polling for potentiometer second. EXTI interrupt-driven
buttons as optional upgrade.

### Milestone 6: Game logic

Ball, paddles, collision, score — in C first, with asm ports done
incrementally once the C version is fully working and tested.

---

## Quick Reference

### Boot sequence, top to bottom

```
Power on
  → Hardware reads 0x08000000 → loads MSP (= _estack = 0x20002800)
  → Hardware reads 0x08000004 → loads PC (= Reset_Handler address)
  → Reset_Handler copies .data flash→RAM, zeroes .bss
  → Reset_Handler calls main()
  → main() enables GPIOC clock, configures PC13, blinks forever
```

### Read-modify-write pattern (used everywhere)

```c
REG &= ~(MASK << SHIFT);   /* clear the field */
REG |=  (VAL  << SHIFT);   /* set the new value */
```

Never use plain `=` on a peripheral register unless you intend to set every
bit in it simultaneously — peripheral registers pack multiple independent
fields, and clobbering them silently corrupts unrelated peripheral config.

### GPIO CRH field layout (pins 8–15)

Each pin gets 4 bits: `[CNF1:CNF0:MODE1:MODE0]`

```
MODE = 00  input
MODE = 01  output 10MHz
MODE = 10  output 2MHz
MODE = 11  output 50MHz

CNF (output): 00 = push-pull, 01 = open-drain, 10 = AF push-pull, 11 = AF open-drain
CNF (input):  00 = analog,    01 = floating,    10 = pull-up/pull-down
```

PC13 (pin 13) → `CRH` bits `[23:20]`. Push-pull output = `MODE=11, CNF=00` = `0x3`.

### Flash and RAM usage ceiling

```
Flash: 32KB  (0x08000000 – 0x08007FFF)
RAM:   10KB  (0x20000000 – 0x200027FF)
```

Run `make` (or `arm-none-eabi-size build/pong.elf`) after every significant
addition to watch these numbers. `.text + .data` consumes flash.
`.data + .bss` consumes RAM. Stack grows down from the top of RAM into
whatever `.bss` doesn't use — on 10KB, keep an eye on both.
