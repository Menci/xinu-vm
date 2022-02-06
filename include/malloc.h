#pragma once
#include <xinu.h>

void *malloc(uint32 size);
void free(void *ptr);
void *realloc(void *ptr, uint32 size);