#ifndef MINEMU_MEMORY_H
#define MINEMU_MEMORY_H

#include <stddef.h>

void *memcpy(void *destination, const void *source, size_t length);
void *memset(void *destination, int value, size_t length);
void *memmove(void *destination, const void *source, size_t length);
int memcmp(const void *left, const void *right, size_t length);

#endif