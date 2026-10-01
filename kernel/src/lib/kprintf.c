#include "minemu/kprintf.h"

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#include "minemu/uart.h"

static const char hex_digits[] = "0123456789abcdef";

/* Print value in the given base (10 or 16) with no leading zeros. */
static void print_unsigned(uint32_t value, uint32_t base) {
    char buf[10]; /* 2^32 - 1 has 10 decimal digits, the worst case. */
    int len = 0;

    /* Digits come out least-significant first, so collect then print in reverse. */
    do {
        buf[len++] = hex_digits[value % base];
        value /= base;
    } while (value != 0);

    while (len > 0) {
        uart_putc(buf[--len]);
    }
}

static void print_signed(int32_t value) {
    uint32_t magnitude = (uint32_t)value;

    if (value < 0) {
        uart_putc('-');
        /* Negate in unsigned arithmetic so INT32_MIN does not overflow. */
        magnitude = 0u - magnitude;
    }
    print_unsigned(magnitude, 10);
}

/* Always print all 8 digits so addresses line up and are unambiguous. */
static void print_pointer(uintptr_t value) {
    uart_puts("0x");
    for (int shift = 28; shift >= 0; shift -= 4) {
        uart_putc(hex_digits[(value >> shift) & 0xf]);
    }
}

void kprintf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    for (; *fmt != '\0'; ++fmt) {
        if (*fmt != '%') {
            uart_putc(*fmt);
            continue;
        }

        ++fmt; /* Step past '%' to the conversion character. */
        switch (*fmt) {
        case 's': {
            const char *s = va_arg(args, const char *);
            uart_puts(s != NULL ? s : "(null)");
            break;
        }
        case 'c':
            /* char is promoted to int when passed through "...". */
            uart_putc((char)va_arg(args, int));
            break;
        case 'd':
            print_signed(va_arg(args, int));
            break;
        case 'u':
            print_unsigned(va_arg(args, unsigned int), 10);
            break;
        case 'x':
            print_unsigned(va_arg(args, unsigned int), 16);
            break;
        case 'p':
            print_pointer((uintptr_t)va_arg(args, void *));
            break;
        case '%':
            uart_putc('%');
            break;
        case '\0':
            /* Format ended with a lone '%': print it and stop. */
            uart_putc('%');
            va_end(args);
            return;
        default:
            uart_putc('%');
            uart_putc(*fmt);
            break;
        }
    }

    va_end(args);
}
