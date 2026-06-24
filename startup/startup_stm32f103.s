/*
 * startup_stm32f103.s
 */

.syntax unified        /* Use ARM's modern unified asm syntax (dest, src
                         * operand order), not the legacy divergent
                         * ARM/Thumb syntaxes. */
.cpu cortex-m3
.thumb                  /* Cortex-M3 only executes Thumb/Thumb-2 encoded
                         * instructions — there is no 32-bit ARM mode on
                         * this core, unlike e.g. Cortex-A chips. */

/* ------------------------------------------------------------------------
 * Vector table
 * ------------------------------------------------------------------------

 * Word 0 = initial stack pointer value (NOT an address to jump to — this
 *          one slot is special, it's loaded directly into MSP).
 * Word 1 = reset handler address (core jumps here after loading MSP).
 * Words 2-15 = fixed Cortex-M3 system exceptions (ARM-architected, same
 *          position on every Cortex-M3 from any vendor).
 * Words 16+ = STM32F103-specific peripheral interrupts (ST-defined, exact
 *          position comes from the STM32F103 reference manual's NVIC
 *          chapter — position is the ONLY thing that maps a slot to a
 *          peripheral, there is no other indirection).
 */
.section .isr_vector, "a", %progbits
.global g_vector_table
g_vector_table:
// 32 bit machine -> 1 word = 32 bits = 4 bytes; Hence each subsequent word is offset by 4 bytes
    .word _estack                  /* 0:  Initial Main Stack Pointer */
    .word Reset_Handler             /* 1:  Reset */
    .word NMI_Handler                /* 2:  Non-Maskable Interrupt */
    .word HardFault_Handler          /* 3:  Hard Fault */
    .word MemManage_Handler          /* 4:  Memory Management Fault (MPU) */
    .word BusFault_Handler            /* 5:  Bus Fault */
    .word UsageFault_Handler          /* 6:  Usage Fault (e.g. bad instr) */
    .word 0                             /* 7:  Reserved */
    .word 0                             /* 8:  Reserved */
    .word 0                             /* 9:  Reserved */
    .word 0                             /* 10: Reserved */
    .word SVC_Handler                  /* 11: SVCall */
    .word DebugMon_Handler              /* 12: Debug Monitor */
    .word 0                              /* 13: Reserved */
    .word PendSV_Handler                  /* 14: PendSV */
    .word SysTick_Handler                  /* 15: SysTick timer */

    /* --- STM32F103-specific IRQs begin at slot 16 --- */
    /* Interrupts not really needed right now */
    // DefaultHandler is defined as an infinite loop after this
    .word Default_Handler          /* 16: WWDG */
    .word Default_Handler          /* 17: PVD */
    .word Default_Handler          /* 18: TAMPER */
    .word Default_Handler          /* 19: RTC */
    .word Default_Handler          /* 20: FLASH */
    .word Default_Handler          /* 21: RCC */
    .word Default_Handler          /* 22: EXTI0 */
    .word Default_Handler          /* 23: EXTI1 */
    .word Default_Handler          /* 24: EXTI2 */
    .word Default_Handler          /* 25: EXTI3 */
    .word Default_Handler          /* 26: EXTI4 */
    .word Default_Handler          /* 27: DMA1_Channel1 */
    .word Default_Handler          /* 28: DMA1_Channel2 */
    .word Default_Handler          /* 29: DMA1_Channel3 */
    .word Default_Handler          /* 30: DMA1_Channel4 */
    .word Default_Handler          /* 31: DMA1_Channel5 */
    .word Default_Handler          /* 32: DMA1_Channel6 */
    .word Default_Handler          /* 33: DMA1_Channel7 */
    .word Default_Handler          /* 34: ADC1_2 */
    .word Default_Handler          /* 35: USB_HP_CAN1_TX */
    .word Default_Handler          /* 36: USB_LP_CAN1_RX0 */
    .word Default_Handler          /* 37: CAN1_RX1 */
    .word Default_Handler          /* 38: CAN1_SCE */
    .word Default_Handler          /* 39: EXTI9_5 */
    .word Default_Handler          /* 40: TIM1_BRK */
    .word Default_Handler          /* 41: TIM1_UP */
    .word Default_Handler          /* 42: TIM1_TRG_COM */
    .word Default_Handler          /* 43: TIM1_CC */
    .word Default_Handler          /* 44: TIM2 */
    .word Default_Handler          /* 45: TIM3 */
    .word Default_Handler          /* 46: TIM4 */
    .word Default_Handler          /* 47: I2C1_EV */
    .word Default_Handler          /* 48: I2C1_ER */
    .word Default_Handler          /* 49: I2C2_EV */
    .word Default_Handler          /* 50: I2C2_ER */
    .word Default_Handler          /* 51: SPI1 */
    .word Default_Handler          /* 52: SPI2 */
    .word Default_Handler          /* 53: USART1 */
    .word Default_Handler          /* 54: USART2 */
    .word Default_Handler          /* 55: USART3 */
    .word Default_Handler          /* 56: EXTI15_10 */
    .word Default_Handler          /* 57: RTCAlarm */
    .word Default_Handler          /* 58: USBWakeup */

/* ------------------------------------------------------------------------
 * Reset_Handler — the first instruction the core ever executes.
 * ------------------------------------------------------------------------
 * Responsibilities, in required order:
 *   1. Copy .data initializer bytes from flash (_sidata) to their RAM
 *      home (_sdata .. _edata) — see linker script comment for why this
 *      copy must happen: flash holds the initial values, RAM holds the
 *      live, writable variable.
 *   2. Zero .bss (_sbss .. _ebss) — C standard guarantees uninitialized
 *      globals start at zero; nothing does this for you in hardware.
 *   3. Call main(). If main ever returns (it shouldn't, for firmware),
 *      fall into an infinite loop rather than letting the core execute
 *      whatever garbage instruction happens to follow in memory.
 */
.section .text.Reset_Handler, "ax", %progbits
.global Reset_Handler
.type Reset_Handler, %function
//whenever Reset_Handler is called, these sections get executed
// copy_data_loop : copies data from sidata(FLASH) to the sdata(RAM) so program can actually use it
// copy_data_done: when pointer of RAM address (r1) reaches END address (r2), this is called
Reset_Handler:
    /* --- Step 1: copy .data from flash to RAM --- */
    ldr   r0, =_sidata      /* r0 = source pointer, in FLASH */
    ldr   r1, =_sdata       /* r1 = destination pointer, in RAM -> destination of data at r0 */
    ldr   r2, =_edata       /* r2 = destination end address */
copy_data_loop:
    cmp   r1, r2             /* have we reached the end of .data in RAM? */
    bge   copy_data_done      /* if r1 >= r2, we're done copying */
    ldr   r3, [r0]             /* load one word from flash */
    str   r3, [r1]              /* store it to RAM */
    adds  r0, r0, #4              /* advance source pointer by 4 bytes */
    adds  r1, r1, #4              /* advance destination pointer by 4 bytes */
    b     copy_data_loop
copy_data_done:

    /* --- Step 2: zero .bss in RAM --- */
    ldr   r0, =_sbss        /* r0 = start of .bss */
    ldr   r1, =_ebss        /* r1 = end of .bss */
    movs  r2, #0              /* the value we're filling with: zero */
zero_bss_loop:
    cmp   r0, r1
    bge   zero_bss_done
    str   r2, [r0]
    adds  r0, r0, #4
    b     zero_bss_loop
zero_bss_done:

    /* --- Step 3: jump into C --- */
    bl    main                /* call main(); bl saves return addr, though
                                * we never expect to use it */

    /* If main() ever returns, trap here forever rather than running off
     * into undefined memory. */
hang_after_main:
    b     hang_after_main
.size Reset_Handler, . - Reset_Handler // . (current position) - Reset_Handler(start position) -> gives size of the ResetHandler function

/* ------------------------------------------------------------------------
 * Default fault/exception handlers
 * ------------------------------------------------------------------------
 * Every one of these is, for now, just "spin forever." That is a deliberate
 * placeholder, not a finished design: when (not if) you eventually trigger
 * a HardFault from a bad pointer or misaligned access while writing the
 * SPI/display code, having it visibly hang (rather than silently reset or
 * continue into garbage) makes it obvious in the debugger that you've hit
 * one, and which one, since you can set a breakpoint on the relevant label.
 */
 
.section .text.Default_Handler, "ax", %progbits
.global Default_Handler
.type Default_Handler, %function
Default_Handler:
infinite_loop:
    b infinite_loop
.size Default_Handler, . - Default_Handler

/* Each named exception handler is just an alias to Default_Handler unless
 * you override it. ".weak" lets you later define a real, e.g.,
 * "void HardFault_Handler(void)" in C and have the linker prefer that
 * definition over this one — useful once you want a HardFault handler
 * that, say, reads the fault status registers before hanging. */
.weak NMI_Handler
.thumb_set NMI_Handler, Default_Handler

.weak HardFault_Handler
.thumb_set HardFault_Handler, Default_Handler

.weak MemManage_Handler
.thumb_set MemManage_Handler, Default_Handler

.weak BusFault_Handler
.thumb_set BusFault_Handler, Default_Handler

.weak UsageFault_Handler
.thumb_set UsageFault_Handler, Default_Handler

.weak SVC_Handler
.thumb_set SVC_Handler, Default_Handler

.weak DebugMon_Handler
.thumb_set DebugMon_Handler, Default_Handler

.weak PendSV_Handler
.thumb_set PendSV_Handler, Default_Handler

.weak SysTick_Handler
.thumb_set SysTick_Handler, Default_Handler

.end
