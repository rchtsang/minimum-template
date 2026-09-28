#include "minemu/boot.h"
#include "minemu/trap.h"
#include "minemu/trace.h"
#include "minemu/platform.h"
#include "minemu/uart.h"
#include "minemu/cprintf.h"
#include "minemu/irq.h"
#include "minemu/memory.h"
#include <stdbool.h>
#include <stddef.h>

#define MAX_LINE_LENGTH 20

static char buffer[MAX_LINE_LENGTH + 1];
static size_t buffer_index = 0;
static volatile bool newline = false;

void uart_handler(void) {
    while (MINEMU_UART0->status & MINEMU_UART_STATUS_RX_READY) {
        uint8_t c = (uint8_t)MINEMU_UART0->rx_data;
        minemu_trace_event(0x0d000000 | c);
        if (c == '\n') {
            newline = true; 
        } else if (c == 0x08 || c == 0x7f) {
            if (buffer_index > 0) {
            buffer_index--;
        }
        } else {
            if (buffer_index < MAX_LINE_LENGTH) {
               buffer[buffer_index++] = (char)c;
            } else {
                // Buffer is full, ignore additional characters
            }
        }
    }

}

struct minemu_trap_frame *minemu_irq_dispatch(struct minemu_trap_frame *frame) {
    uint32_t source = (uint32_t)frame->exception_id;
    if (source == MINEMU_IRQ_SYSTICK) {
        MINEMU_SYSTICK->ack = MINEMU_SYSTICK_ACK;
    } else if (source == MINEMU_IRQ_UART0) {
        uart_handler();
    } else if (source == MINEMU_IRQ_BLOCK) {
        MINEMU_BLOCK->ack = MINEMU_BLOCK_ACK;
    }
    MINEMU_INTERRUPT->eoi = source;
    return frame;
}

void minemu_kernel_main(const struct minemu_boot_info *boot_info) {
    if ((uintptr_t)boot_info != MINEMU_BOOT_INFO_VADDR ||
        boot_info->magic != MINEMU_BOOT_INFO_MAGIC ||
        boot_info->version != MINEMU_ABI_VERSION ||
        boot_info->size != sizeof(*boot_info) ||
        boot_info->system_rom_base != UINT32_C(0x08000000) ||
        boot_info->direct_map_vaddr != UINT32_C(0xc0000000) ||
        boot_info->direct_map_paddr != UINT32_C(0x40000000) ||
        boot_info->direct_map_size != UINT32_C(0x04000000)) {
        minemu_trace_event(UINT32_C(0xb007bad0));
        minemu_fail_stop();
    }
    cprintf("hello world\n");
    uart_init();
    MINEMU_INTERRUPT->enable |= UINT32_C(1) << MINEMU_IRQ_UART0;
    minemu_irq_enable();
    while (1) {
        cprintf("msh> ");
        while (!newline) {}
        minemu_irq_disable();
        char line[MAX_LINE_LENGTH + 1];
        memcpy(line, buffer, buffer_index);
        line[buffer_index] = '\0';
        buffer_index = 0;
        newline = false;
        minemu_irq_enable();   
        char *ptr = line;
        while (*ptr == ' ') {
            ++ptr;
        }
        if (*ptr == '\0') {
            continue; 
        }
        if (memcmp(ptr, "echo", 4) == 0 && (ptr[4] == ' ' || ptr[4] == '\0')) {
            char *arg = ptr + 4;
            while (*arg == ' ') {
                ++arg;
            }
            cprintf("%s\n", arg);
        } else {
            char *start = ptr;
            char *end = ptr;
            while (*end != ' ' && *end != '\0') {
                ++end;
            }
            *end = '\0';
            cprintf("command not found: %s\n", start);
        }
    }
    
}
