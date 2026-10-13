#ifndef MINEMU_UARTDRIVER_H
#define MINEMU_UARTDRIVER_H

#include <stdint.h>
#include "minemu/platform.h"

void uart_init(void);
void uart_txwrite(char c);
void uart_txwrite_string(const char *c);

#endif