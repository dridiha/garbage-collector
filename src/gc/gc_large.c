#include "../../include/gc.h"

void *gc_malloc_large(uint32_t size) {
  uint32_t rounded_size =
      size % CHUNKSIZE == 0 ? size : CHUNKSIZE * (size / CHUNKSIZE + 1);
  return gc_mmap(rounded_size / CHUNKSIZE, -1);
}

void gc_mark_large(ChunkMetadata *metadata) { metadata->bitmap[0] = true; }
void gc_free_large(void *ptr, ChunkMetadata *metadata) {
  munmap(ptr, metadata->chunk_size);
  remove_chunk_metadata(ptr);
  ptr = NULL;
}

bool gc_check_if_marked_large(ChunkMetadata *metadata) {
  return metadata->bitmap[0];
}
