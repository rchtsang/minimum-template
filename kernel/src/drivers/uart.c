#include "minemu/uart.h"

#include "minemu/kirq.h"
#include "minemu/platform.h"

/*
 * Receive ring buffer. Single producer (uart_rx_isr, in IRQ mode) and single
 * consumer (uart_try_getc): only the ISR writes rx_head and only the consumer
 * writes rx_tail, so neither needs a lock. The indices run freely and are
 * reduced modulo the power-of-two size when indexing; head - tail is the fill.
 */
#define RX_BUFFER_SIZE 256u
_Static_assert((RX_BUFFER_SIZE & (RX_BUFFER_SIZE - 1)) == 0, "RX buffer size must be a power of two");

static char rx_buffer[RX_BUFFER_SIZE];
static volatile uint32_t rx_head; /* next slot the ISR writes */
static volatile uint32_t rx_tail; /* next slot the reader takes */

void uart_init(void) {
    rx_head = 0;
    rx_tail = 0;
    /* Ask the UART to raise its IRQ line while RX_READY is set... */
    MINEMU_UART0->control = MINEMU_UART_CONTROL_RX_IRQ_ENABLE;
    /* ...and let that line through the interrupt controller. */
    irq_enable_source(MINEMU_IRQ_UART0);
}

void uart_putc(char c) {
    /* The TX FIFO can fill up; writing before TX_READY would drop the byte. */
    while ((MINEMU_UART0->status & MINEMU_UART_STATUS_TX_READY) == 0) {
    }
    /* Zero-extend so only bits [7:0] are ever set in the 32-bit store. */
    MINEMU_UART0->tx_data = (uint32_t)(uint8_t)c;
}

void uart_puts(const char *s) {
    while (*s != '\0') {
        uart_putc(*s++);
    }
}

void uart_rx_isr(void) {
    /* Drain everything the device holds, so the IRQ line drops before EOI. */
    while ((MINEMU_UART0->status & MINEMU_UART_STATUS_RX_READY) != 0) {
        /* Reading RX_DATA consumes the byte; keep only the data bits. */
        char c = (char)(MINEMU_UART0->rx_data & UINT32_C(0xff));
        if (rx_head - rx_tail < RX_BUFFER_SIZE) {
            rx_buffer[rx_head % RX_BUFFER_SIZE] = c;
            /* Compiler barrier: the byte must be stored before head publishes it. */
            __asm__ volatile("" : : : "memory");
            rx_head = rx_head + 1;
        }
        /* Buffer full: the byte is dropped (it was still read to clear RX_READY). */
    }
}

int uart_try_getc(void) {
    if (rx_tail == rx_head) {
        return -1;
    }
    /* Compiler barrier: read the byte only after seeing head move past it. */
    __asm__ volatile("" : : : "memory");
    char c = rx_buffer[rx_tail % RX_BUFFER_SIZE];
    rx_tail = rx_tail + 1;
    return (int)(uint8_t)c;
}

char uart_getc(void) {
    /*
     * Spin with IRQs enabled; the RX interrupt fills the buffer meanwhile.
     * (WFI is not used: minemu does not wake from it on a pending IRQ.)
     */
    for (;;) {
        int c = uart_try_getc();
        if (c >= 0) {
            return (char)c;
        }
    }
}
