#include "minemu/shell.h"

#include <stddef.h>

#include "minemu/kprintf.h"
#include "minemu/uart.h"

#define SHELL_PROMPT "msh> "
#define LINE_MAX 128 /* bytes per input line, including the terminating NUL */
#define ARGS_MAX 16  /* words per command line, including the command name */

/* A built-in command: receives the words of the line, argv[0] being its name. */
struct shell_command {
    const char *name;
    const char *help;
    void (*run)(int argc, char **argv);
};

static void cmd_echo(int argc, char **argv);
static void cmd_help(int argc, char **argv);

/* Dispatch table: adding a command means adding one line here. */
static const struct shell_command commands[] = {
    {"echo", "print the arguments separated by spaces", cmd_echo},
    {"help", "list the available commands", cmd_help},
};

#define COMMAND_COUNT (sizeof(commands) / sizeof(commands[0]))

static int str_equal(const char *a, const char *b) {
    while (*a != '\0' && *a == *b) {
        ++a;
        ++b;
    }
    return *a == *b;
}

static void cmd_echo(int argc, char **argv) {
    for (int i = 1; i < argc; ++i) {
        kprintf(i > 1 ? " %s" : "%s", argv[i]);
    }
    kprintf("\n");
}

static void cmd_help(int argc, char **argv) {
    (void)argc;
    (void)argv;
    for (size_t i = 0; i < COMMAND_COUNT; ++i) {
        kprintf("  %s - %s\n", commands[i].name, commands[i].help);
    }
}

/*
 * Read one line into buf with local echo and backspace editing.
 * Enter (CR or LF) ends the line; the newline is not stored. Characters past
 * the buffer size are ignored. A LF directly after a CR is swallowed, so a
 * terminal sending "\r\n" yields one line rather than an extra empty one.
 */
static void read_line(char *buf, size_t size) {
    static int last_was_cr;
    size_t len = 0;

    for (;;) {
        char c = uart_getc();

        if (c == '\n' && last_was_cr) {
            last_was_cr = 0;
            continue;
        }
        last_was_cr = (c == '\r');

        if (c == '\r' || c == '\n') {
            uart_putc('\n');
            buf[len] = '\0';
            return;
        }
        if (c == '\b' || c == 0x7f) {
            if (len > 0) {
                --len;
                uart_puts("\b \b"); /* move back, blank the character, move back */
            }
            continue;
        }
        /* Store and echo printable characters only, leaving room for the NUL. */
        if (c >= ' ' && c <= '~' && len < size - 1) {
            buf[len++] = c;
            uart_putc(c);
        }
    }
}

/* Split line in place at spaces/tabs. Returns the word count; extra words are dropped. */
static int split_words(char *line, char **argv, int max) {
    int argc = 0;

    while (*line != '\0' && argc < max) {
        while (*line == ' ' || *line == '\t') {
            *line++ = '\0';
        }
        if (*line == '\0') {
            break;
        }
        argv[argc++] = line;
        while (*line != '\0' && *line != ' ' && *line != '\t') {
            ++line;
        }
    }
    return argc;
}

static void run_line(char *line) {
    char *argv[ARGS_MAX];
    int argc = split_words(line, argv, ARGS_MAX);

    if (argc == 0) {
        return; /* empty line: just prompt again */
    }
    for (size_t i = 0; i < COMMAND_COUNT; ++i) {
        if (str_equal(argv[0], commands[i].name)) {
            commands[i].run(argc, argv);
            return;
        }
    }
    kprintf("msh: unknown command: %s\n", argv[0]);
}

void shell_run(void) {
    static char line[LINE_MAX];

    for (;;) {
        uart_puts(SHELL_PROMPT);
        read_line(line, sizeof(line));
        run_line(line);
    }
}
