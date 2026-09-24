#include "minemu/uart.h"
#include "minemu/platform.h"

void uart_txwrite(char c) {
    while(!(MINEMU_UART0->status & MINEMU_UART_STATUS_TX_READY)) {}
    MINEMU_UART0->tx_data = (uint32_t)c;
}
void uart_txwrite_string(const char *c) {
    for (size_t index = 0; c[index] != '\0'; ++index) {
        uart_txwrite(c[index]);
    }
}