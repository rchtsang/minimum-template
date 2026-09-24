#include "minemu/uart.h"
#include <stdarg.h>

void cprintf(const char *format, ...) {
    va_list args;
    va_start(args, format);

    for (const char *ptr = format; *ptr != '\0'; ++ptr) {
        if (*ptr == '%') {
            ++ptr;
            if (*ptr == 's') {
                const char *str = va_arg(args, const char *);
                uart_txwrite_string(str);
            } else if (*ptr == 'c') {
                char c = (char)va_arg(args, int);
                uart_txwrite(c);
            } else if (*ptr == '%') {
                uart_txwrite('%');
            }
        } else {
            uart_txwrite(*ptr);
        }
    }
}