#include "../../include/gc.h"

static void *small_mem[MEM_SMALL_SIZE] = {0};
static bool is_init = false;

void *gc_link_small(void *ptr, uint16_t object_size) {
  uint8_t *current = (uint8_t *)ptr;
  for (int i = 0; i < (CHUNKSIZE / object_size) - 1; i++) {
    *((void **)current) = (void *)(current + object_size);
    current = current + object_size;
  }
  *((void **)current) = NULL;

  return ptr;
}

void gc_init_small() {
  // Allocate one page from OS for each supported size
  void *ptr;
  uint16_t object_size;
  for (int i = 0; i < MEM_SMALL_SIZE; i++) {
    object_size = get_size_from_index(i);
    if ((ptr = gc_mmap(1, object_size)) != NULL) {
      small_mem[i] = gc_link_small(ptr, object_size);
    };
  }
}

void gc_realloc_small(uint8_t index) {
  uint16_t object_size = (uint16_t)(get_size_from_index(index));
  void *ptr = gc_mmap(1, object_size);
  if (ptr != NULL) {
    small_mem[index] = gc_link_small(ptr, object_size);
  }
}

void *gc_malloc_small(uint32_t size) {
  if (!is_init) {
    gc_init_small();
    is_init = true;
  }
  uint8_t index = get_index_from_size(size);
  if (small_mem[index] == 0 || small_mem[index] == NULL) {
    gc_realloc_small(index);
  }
  void *head = small_mem[index];
  if (head == NULL) {
    return NULL;
  }
  small_mem[index] = *(void **)head;
  memset(head, 0, get_size_from_index(index));
  gc_mark_small(head, find_metadata_by_pointer(head), ALLOCATED);
  return head;
}

void gc_free_small(void *ptr, ChunkMetadata *metadata) {

  gc_mark_small(ptr, metadata, FREE);
  enum State state = gc_get_state_small(ptr, metadata);
  uint8_t index = get_index_from_size(metadata->object_size);
  *((void **)ptr) = small_mem[index];
  small_mem[index] = ptr;
  memset(ptr, 0, metadata->object_size);
}

void gc_mark_small(void *ptr, ChunkMetadata *metadata, enum State flag) {
  uint8_t r = (((unsigned long)ptr) & 0xFFF) % (metadata->object_size);
  void *base = (void *)((uint8_t *)ptr - r);
  uint8_t w = ((uint64_t)base & 0xFFF) / (metadata->object_size * 4);
  uint8_t w_offset = (((uint64_t)base & 0xFFF) / (metadata->object_size) % 4)
                     << 1;
  uint8_t mask = (3 << (6 - w_offset));

  metadata->bitmap[w] &= ~mask;
  metadata->bitmap[w] |= (flag << (6 - w_offset));
}

enum State gc_get_state_small(void *ptr, ChunkMetadata *metadata) {
  uint8_t r = (((unsigned long)ptr) & 0xFFF) % (metadata->object_size);
  void *base = (void *)((uint8_t *)ptr - r);
  uint8_t w = ((uint64_t)base & 0xFFF) / (metadata->object_size * 4);
  uint8_t w_offset = (((uint64_t)base & 0xFFF) / (metadata->object_size) % 4)
                     << 1;
  uint8_t res = metadata->bitmap[w];
  uint8_t state = (metadata->bitmap[w] >> (6 - w_offset)) & 3;
  return state;
}
bool gc_check_if_marked_small(void *ptr, ChunkMetadata *metadata) {
  enum State state = gc_get_state_small(ptr, metadata);
  return (state == IN_USE || state == FREE) ? true : false;
}
