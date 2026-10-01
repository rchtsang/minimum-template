#ifndef MINEMU_UART_H
#define MINEMU_UART_H

/*
 * Driver for the console UART (UART0).
 *
 * Transmit is polled: uart_putc() busy-waits on TX_READY.
 * Receive is interrupt-driven: the RX interrupt handler (uart_rx_isr) moves
 * bytes from the device into a ring buffer, and uart_getc()/uart_try_getc()
 * read from that buffer, never from the device directly.
 *
 * Register access rules (violations data-abort in minemu):
 *   rx_data  read-only   one received byte in bits [7:0]
 *   tx_data  write-only  one byte to transmit in bits [7:0]
 *   status   read-only   MINEMU_UART_STATUS_RX_READY / _TX_READY
 *   control  read/write  only MINEMU_UART_CONTROL_RX_IRQ_ENABLE is defined
 * Every access is a single aligned 32-bit load or store through MINEMU_UART0.
 */

/*
 * Empty the receive buffer and turn on the UART0 RX interrupt, both in the
 * UART and at the interrupt controller. Input only arrives once the caller
 * also unmasks IRQs on the CPU with minemu_irq_enable().
 */
void uart_init(void);

/* Wait until the transmitter is ready, then send one byte. */
void uart_putc(char c);

/* Send a NUL-terminated string, byte for byte (no newline translation). */
void uart_puts(const char *s);

/* Return the next received byte (0-255), or -1 if none is buffered. Never blocks. */
int uart_try_getc(void);

/* Wait until a byte has been received, then return it. Requires IRQs enabled. */
char uart_getc(void);

/* RX interrupt handler: drain the device into the buffer. Called by the IRQ dispatcher. */
void uart_rx_isr(void);

#endif
