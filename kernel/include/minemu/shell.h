#ifndef MINEMU_SHELL_H
#define MINEMU_SHELL_H

/*
 * msh: the kernel's interactive console shell on UART0.
 * Prints "msh> ", reads a line, runs the named built-in command, repeats.
 * Requires uart_init() and enabled interrupts, since input is IRQ-driven.
 */
void shell_run(void) __attribute__((noreturn));

#endif
