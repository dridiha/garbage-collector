#ifndef __GC_SMALL_H
#define __GC_SMALL_H
#include "../data_struct.h"
#include <stdint.h>

void *gc_link_small(void *ptr, uint16_t object_size);
void gc_init_small();
void gc_realloc_small(uint8_t index);
void *gc_malloc_small(uint32_t size);
void gc_free_small(void *ptr, ChunkMetadata *metadata);
void gc_debug_small(void *ptr);
bool gc_check_if_marked_small(void *ptr, ChunkMetadata *metadata);
void gc_mark_small(void *ptr, ChunkMetadata *metadata, uint8_t flag);

#endif
