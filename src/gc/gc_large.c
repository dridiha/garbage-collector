#include "../../include/gc.h"

void *gc_malloc_large(uint32_t size) {
  uint32_t rounded_size =
      size % CHUNKSIZE == 0 ? size : CHUNKSIZE * (size / CHUNKSIZE + 1);
  void *ptr = gc_mmap(rounded_size / CHUNKSIZE, -1);
  if (ptr != NULL) {
    gc_mark_large(find_metadata_by_pointer(ptr), ALLOCATED);
  }
  return ptr;
}

void gc_mark_large(ChunkMetadata *metadata, enum State flag) {
  metadata->bitmap[0] = flag;
}

enum State gc_get_state_large(ChunkMetadata *metadata) {
  return metadata->bitmap[0];
}

void gc_free_large(void *ptr, ChunkMetadata *metadata) {
  munmap(ptr, metadata->chunk_size);
  remove_chunk_metadata(ptr);
  ptr = NULL;
}

bool gc_check_if_marked_large(ChunkMetadata *metadata) {
  return (metadata->bitmap[0] == IN_USE);
}
