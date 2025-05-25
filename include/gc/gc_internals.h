#ifndef __GC_INTERNALS_H
#define __GC_INTERNALS_H
#include "../data_struct.h"
#include <stdint.h>

void *gc_mmap(uint8_t number_of_chunks, int16_t object_size);
void *gc_malloc(uint32_t size);
void gc_free(ChunkMetadata *metadata);
bool gc_is_valid_pointer(void *ptr);
void gc_mark();
void gc_sweep();

// stack utilities
uint64_t *get_stack_top();
uint64_t *get_stack_bottom();

#endif // !__GCC_H_
