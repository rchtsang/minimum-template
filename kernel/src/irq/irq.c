#include "minemu/irq.h"

#include "minemu/kirq.h"
#include "minemu/platform.h"
#include "minemu/uart.h"

/*
 * Software copy of the interrupt controller's ENABLE register, so enabling one
 * source never needs a read-modify-write of the device register.
 */
static uint32_t enabled_sources;

void irq_init(void) {
    enabled_sources = 0;
    MINEMU_INTERRUPT->enable = 0;
}

void irq_enable_source(uint32_t source) {
    enabled_sources |= UINT32_C(1) << source;
    MINEMU_INTERRUPT->enable = enabled_sources & MINEMU_IRQ_ENABLE_MASK;
}

struct minemu_trap_frame *minemu_irq_dispatch(struct minemu_trap_frame *frame) {
    /* The trampoline already read CLAIM and stored the source here. */
    uint32_t source = (uint32_t)frame->exception_id;

    /* Nothing was pending (spurious IRQ): there is nothing to acknowledge. */
    if (source == MINEMU_IRQ_NONE) {
        return frame;
    }

    switch (source) {
    case MINEMU_IRQ_UART0:
        uart_rx_isr();
        break;
    default:
        /* No other source is ever enabled; treat one arriving as a bug. */
        minemu_fail_stop();
    }

    /* Tell the controller this source is serviced so it can fire again. */
    MINEMU_INTERRUPT->eoi = source;
    return frame;
}
