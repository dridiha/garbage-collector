#include "../../include/gc.h"

static uint64_t lower_bound = 0;
static uint64_t higher_bound = 0;

void *gc_mmap(uint8_t number_of_chunks, int16_t object_size) {
  void *ptr =
      (void *)mmap(NULL, CHUNKSIZE * number_of_chunks, PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (ptr == MAP_FAILED) {
    return NULL;
  }
  if (register_chunk_metadata(ptr, object_size, CHUNKSIZE * number_of_chunks) ==
      -1)
    return NULL;
  if ((uint64_t)ptr < lower_bound || lower_bound == 0)
    lower_bound = (uint64_t)ptr;
  if ((uint64_t)((uint8_t *)ptr + CHUNKSIZE * number_of_chunks) > higher_bound)
    higher_bound = (uint64_t)((uint8_t *)ptr + CHUNKSIZE * number_of_chunks);
  return ptr;
}
void *gc_malloc(uint32_t size) {
  if (size <= 0)
    return NULL;
  if (size <= CHUNKSIZE / 2)
    return gc_malloc_small(size);
  return gc_malloc_large(size);
}

void gc_free(ChunkMetadata *metadata) {
  void *ptr = metadata->base;
  if (metadata->object_size != -1) {
    for (int k = 0; k < metadata->chunk_size / metadata->object_size;
         ptr = (void *)((uint8_t *)ptr + metadata->object_size * k++)) {
      if (!gc_check_if_marked_small(ptr, metadata)) {
        gc_free_small(ptr, metadata);
      }
    }
  } else if (metadata->offset == 0) {
    if (!gc_check_if_marked_large(metadata)) {
      gc_free_large(ptr, metadata);
    }
  }
}

bool gc_is_valid_pointer(void *ptr) {
  return chunk_entry_exists(ptr) && (uint64_t)ptr >= lower_bound &&
         (uint64_t)ptr <= higher_bound;
}

uint64_t *get_stack_top() {
  FILE *fd = fopen("/proc/self/maps", "r");
  if (!fd) {
    perror("Can not open /proc/self/maps\n");
    exit(EXIT_FAILURE);
  }
  uint64_t top = 0;
  uint64_t bottom = 0;
  char line[4096];
  while (fgets(line, sizeof(line), fd) != NULL) {
    if (strstr(line, "[stack]")) {
      sscanf(line, "%lx-%lx", &bottom, &top);
      break;
    }
  }
  fclose(fd);
  return (uint64_t *)top;
}

uint64_t *get_stack_bottom() {
  uint64_t bottom;
  asm volatile("movq %%rsp, %0" : "=r"(bottom)::);
  return (uint64_t *)bottom;
}

bool gc_check_if_marked(void *ptr, ChunkMetadata *metadata) {
  if (metadata->object_size == -1)
    return gc_check_if_marked_large(metadata);
  return gc_check_if_marked_small(ptr, metadata);
}

void gc_mark_ptr(void *ptr, ChunkMetadata *metadata) {
  if (metadata->object_size == -1)
    gc_mark_large(metadata, IN_USE);
  else
    gc_mark_small(ptr, metadata, IN_USE);
};

enum State gc_get_state(void *ptr) {
  ChunkMetadata *metadata = find_metadata_by_pointer(ptr);
  if (metadata == NULL)
    return -1;
  uint8_t state = gc_get_state_small(ptr, metadata);
  return metadata->object_size == -1 ? gc_get_state_large(metadata)
                                     : gc_get_state_small(ptr, metadata);
}

void gc_mark_all_ptr(void *ptr) {
  // check if marked: return ( small or large )
  // if not: mark it (small or large)
  // recuirsively check the pointers in heap
  if (!gc_is_valid_pointer(ptr))
    return;
  ChunkMetadata *metadata = find_metadata_by_pointer(ptr);
  if (metadata == NULL)
    return;
  if (gc_check_if_marked(ptr, metadata))
    return;
  gc_mark_ptr(ptr, metadata);
  uint64_t *end = metadata->object_size == -1
                      ? (uint64_t *)((uint8_t *)ptr + metadata->chunk_size)
                      : (uint64_t *)((uint8_t *)ptr + metadata->object_size);
  for (uint64_t *p = (uint64_t *)ptr; p != end; p++) {
    gc_mark_all_ptr((void *)(*p));
  }
}
void gc_mark() {
  uint64_t *top = get_stack_top();
  uint64_t *bottom = get_stack_bottom();
  for (uint64_t *ptr = bottom; ptr != top; ptr++) {
    gc_mark_all_ptr((void *)(*ptr));
  }
}

void gc_sweep_chunk(ChunkMetadata *metadata) {
  if (metadata->base == 0)
    return;
  gc_free(metadata);
}

void gc_sweep() { parse(&gc_sweep_chunk); }
