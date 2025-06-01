#ifndef __GC_LARGE_H
#define __GC_LARGE_H
#include "../data_struct.h"
#include <stdint.h>

void *gc_malloc_large(uint32_t size);
enum State gc_get_state_large(ChunkMetadata *metadata);
void gc_mark_large(ChunkMetadata *metadata, enum State flag);
void gc_free_large(void *ptr, ChunkMetadata *metadata);
bool gc_check_if_marked_large(ChunkMetadata *metadata);

#endif
