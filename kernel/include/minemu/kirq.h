#ifndef MINEMU_KIRQ_H
#define MINEMU_KIRQ_H

#include <stdint.h>

/*
 * Kernel interrupt-controller helpers. The CPU-side entry points
 * (minemu_irq_trampoline, minemu_irq_dispatch) are declared in minemu/irq.h.
 *
 * Flow of one interrupt:
 *   device raises line -> vector -> minemu_irq_trampoline (saves frame, reads CLAIM)
 *   -> minemu_irq_dispatch (calls the device handler, writes EOI) -> resume.
 */

/* Mask every source at the controller. Call once, before minemu_irq_enable(). */
void irq_init(void);

/* Unmask one source (MINEMU_IRQ_UART0, ...) at the controller. */
void irq_enable_source(uint32_t source);

#endif
