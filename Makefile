# =============================================================================
# Makefile — pong-stm32, STM32F103C6T6
# =============================================================================
#
# Targets:
#   make          → build .elf and .bin, print size
#   make flash    → build then flash via st-flash
#   make clean    → delete build/
#   make debug    → print OpenOCD connection instructions
#
# Usage assumption: arm-none-eabi toolchain and st-flash are on your PATH.
# =============================================================================

# -----------------------------------------------------------------------------
# Toolchain
# -----------------------------------------------------------------------------
CC      = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE    = arm-none-eabi-size

# -----------------------------------------------------------------------------
# Paths
# -----------------------------------------------------------------------------
BUILDDIR  = build
TARGET    = $(BUILDDIR)/pong

LINKER    = linker/stm32f103c6.ld

# All .c files under src/, all .s files under startup/.
# Using wildcard means adding a new .c file to src/ is automatically picked
# up on the next build — no Makefile edit needed.
SRCS_C   := $(wildcard src/*.c)
SRCS_S   := $(wildcard startup/*.s)

# Object files: mirror the source filenames into build/, changing extension.
# patsubst pattern: replace src/%.c with build/%.o, and startup/%.s with build/%.o
OBJS     := $(patsubst src/%.c,    $(BUILDDIR)/%.o, $(SRCS_C)) \
            $(patsubst startup/%.s, $(BUILDDIR)/%.o, $(SRCS_S))

# -----------------------------------------------------------------------------
# Compiler flags
# -----------------------------------------------------------------------------
# -mcpu=cortex-m3   generate Cortex-M3 Thumb-2 instructions specifically.
#                   Without this, GCC may emit instructions this core can't
#                   execute (e.g. from a different Cortex-M variant).
#
# -mthumb           emit Thumb instruction encoding, not 32-bit ARM encoding.
#                   Cortex-M3 has no ARM mode — this flag is non-negotiable.
#
# -ffreestanding    tell GCC there is no hosted environment (no OS, no libc
#                   main-calling machinery). Disables assumptions like
#                   "memcpy is available" or "main has a specific signature."
#
# -nostdlib         do not link any standard library or runtime startup file.
#                   We ARE the runtime startup (startup_stm32f103.s). Linking
#                   libc's crt0 on top of ours would produce two Reset_Handlers
#                   and a broken binary.
#
# -Wall             enable all standard warnings. On a 32KB part, silent
#                   type-promotion bugs or unused variables are worth knowing
#                   about early.
#
# -g                embed DWARF debug info in the .elf. Has zero cost in the
#                   final .bin (objcopy strips it), but makes gdb+OpenOCD
#                   show you C source lines instead of raw addresses.
#
# -O0               no optimization for now. Makes the delay() loop behave
#                   as written (volatile on the parameter handles the loop
#                   itself, but -O0 avoids any other surprises during bringup).
#                   Raise to -Og or -O2 once the system is stable.
#
# -Iinclude         add include/ to the header search path so #include "regs.h"
#                   works from any source file without a path prefix.
CFLAGS  = -mcpu=cortex-m3 \
           -mthumb         \
           -ffreestanding  \
           -nostdlib       \
           -Wall           \
           -g              \
           -O0             \
           -Iinclude

# -----------------------------------------------------------------------------
# Linker flags
# -----------------------------------------------------------------------------
# -T linker/...     use our hand-written linker script instead of GCC's
#                   built-in default (which assumes a hosted environment with
#                   a normal OS memory layout).
#
# -nostdlib         same reason as in CFLAGS — passed again here because GCC
#                   drives the linker and would otherwise sneak libc back in
#                   during the link step even if you excluded it during compile.
#
# --gc-sections     garbage-collect unused input sections. Any function or
#                   data object that nothing references gets dropped from the
#                   output. On a 32KB part this matters — dead code from
#                   included headers or unused driver stubs won't silently
#                   consume flash. Pairs with -ffunction-sections /
#                   -fdata-sections (add those to CFLAGS when you want
#                   per-function GC granularity; for now it works at the
#                   object-file level).
#
# -Map=...          write a linker map file to build/pong.map. This is the
#                   single most useful debugging tool for "why is my binary
#                   so large" or "where did symbol X end up" questions — it
#                   lists every input section, its size, and its final address.
LDFLAGS = -T $(LINKER)              \
           -nostdlib                 \
           -Wl,--gc-sections         \
           -Wl,-Map=$(TARGET).map

# -----------------------------------------------------------------------------
# Build rules
# -----------------------------------------------------------------------------

# Default target: build everything, then print flash/RAM usage.
# The size output shows .text (flash used by code+rodata), .data (flash used
# by initialized variable initializers), and .bss (RAM used by zero-init
# globals) separately — useful to watch against your 32K/10K ceilings.
.PHONY: all
all: $(TARGET).elf $(TARGET).bin
	@$(SIZE) $(TARGET).elf

# Link all object files into the .elf
$(TARGET).elf: $(OBJS) $(LINKER) | $(BUILDDIR)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) -o $@

# Strip debug info and produce a raw binary for flashing.
# .elf contains DWARF sections, symbol tables, etc. that st-flash doesn't
# need and that would confuse the chip if written to flash as-is. objcopy
# with -O binary extracts only the loadable segments — the actual bytes that
# go into flash at 0x08000000.
$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

# Compile .c → .o
# $< = the source file, $@ = the output .o
# | $(BUILDDIR) is an order-only prerequisite: ensures build/ exists before
# trying to write into it, but doesn't trigger a rebuild if build/'s
# timestamp changes.
$(BUILDDIR)/%.o: src/%.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Assemble .s → .o
# GCC can drive the assembler directly; passing CFLAGS ensures -mcpu and
# -mthumb are set the same way for asm files as for C files.
$(BUILDDIR)/%.o: startup/%.s | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Create build/ if it doesn't exist
$(BUILDDIR):
	mkdir -p $(BUILDDIR)

# -----------------------------------------------------------------------------
# Flash
# -----------------------------------------------------------------------------
# st-flash expects the raw .bin and the target address in flash (0x8000000).
# Note: st-flash accepts 0x8000000 (7 hex digits) and 0x08000000 (8) — they
# are the same address, both work, but the reference manual always writes the
# full 8-digit form, so we do too for clarity.
.PHONY: flash
flash: $(TARGET).bin
	st-flash write $(TARGET).bin 0x08000000

# -----------------------------------------------------------------------------
# Debug (OpenOCD)
# -----------------------------------------------------------------------------
# OpenOCD is not chained into the flash target — it's a separate, longer-lived
# process you run alongside gdb in a second terminal. This target just reminds
# you of the two commands needed so you don't have to look them up.
#
# Terminal 1: make debug     (starts OpenOCD, stays running)
# Terminal 2: arm-none-eabi-gdb build/pong.elf
#             (gdb) target extended-remote :3333
#             (gdb) load          ← flashes via gdb/OpenOCD
#             (gdb) break main
#             (gdb) continue
.PHONY: debug
debug:
	openocd -f tools/openocd.cfg

# -----------------------------------------------------------------------------
# Clean
# -----------------------------------------------------------------------------
.PHONY: clean
clean:
	rm -rf $(BUILDDIR)
