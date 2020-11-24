#ifndef _malloc_H
#define _malloc_H

#include <xinu.h>

void *malloc(uint32 size);
void free(void *ptr);
void *realloc(void *ptr, uint32 size);

#endif // _malloc_H
