#include "../include/data_struct.h"
#include "../include/gc.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

HashBucket HASH_MAP[LEVEL_SIZE] = {0};

uint16_t hash(void *key) {
  // key is 64 bit address
  unsigned long hi = ((unsigned long)key) >> (22);
  return (hi & 0x3FF) % LEVEL_SIZE;
}

bool chunk_entry_exists(void *key) {
  int hashed_key = hash(key);
  return (HASH_MAP[hashed_key].high_bits != 0);
}
int8_t insert_chunk_entry(void *key, HashBucket value) {
  int hashed_key = hash(key);
  if (chunk_entry_exists(key)) {
    return -1;
  }
  HASH_MAP[hashed_key] = value;
  return 0;
}

HashBucket *get_chunk_bucket(void *key) { return HASH_MAP + hash(key); }

HashBucket create_hash_bucket(void *key) {
  HashBucket hash_bucket = {.high_bits = ((unsigned long)key >> 22),
                            .chunks = {0}};

  return hash_bucket;
}

ChunkMetadata create_chunk_metadata(void *base, uint32_t chunk_size,
                                    int16_t object_size, uint8_t offset) {
  ChunkMetadata ChunkMetadata = {.object_size = object_size,
                                 .offset = offset,
                                 .chunk_size = chunk_size,
                                 .base = base,
                                 .bitmap = {0}};
  return ChunkMetadata;
}

int8_t register_chunk_metadata(void *base, int16_t object_size,
                               uint32_t chunk_size) {
  uint16_t mid = ((unsigned long)base >> 12) & 0x3FF;
  if (!chunk_entry_exists(base)) {
    if (insert_chunk_entry(base, create_hash_bucket(base)) == -1) {
      return -1;
    }
  }
  HashBucket *hb = get_chunk_bucket(base);
  for (int i = 0; i < chunk_size / CHUNKSIZE; i++) {
    if (mid + i < LEVEL_SIZE) {
      if (hb->chunks[mid + i].base != NULL) {
        return -1;
      }
      hb->chunks[mid + i] =
          create_chunk_metadata(base, chunk_size, object_size, i);
      ;
    }
  }
  return 0;
}

ChunkMetadata *find_metadata_by_pointer(void *ptr) {
  unsigned long hi = ((unsigned long)ptr) >> (22);
  int mid = (((unsigned long)ptr) >> 12) & 0x3FF;
  if (!chunk_entry_exists(ptr))
    return NULL;
  HashBucket *fl = get_chunk_bucket(ptr);
  if (fl->high_bits != hi) {
    return NULL;
  }
  int offset = fl->chunks[mid].offset;
  return offset == 0 ? fl->chunks + mid : fl->chunks + (mid - offset);
}

void remove_chunk_metadata(void *ptr) {
  unsigned long hi = ((unsigned long)ptr) >> (22);
  int mid = (((unsigned long)ptr) >> 12) & 0x3FF;
  HashBucket *fl = get_chunk_bucket(ptr);
  int offset = fl->chunks[mid].offset;
  int start = offset == 0 ? mid : mid - offset;
  int end = fl->chunks[mid].chunk_size / CHUNKSIZE;
  for (int i = start; i < end; i++) {
    fl->chunks[i] = (ChunkMetadata){.object_size = 0,
                                    .offset = 0,
                                    .chunk_size = 0,
                                    .base = NULL,
                                    .bitmap = {0}

    };
  }
}

void parse(void (*callback)(ChunkMetadata *)) {
  for (int i = 0; i < LEVEL_SIZE; i++) {
    if (HASH_MAP[i].high_bits != 0) {
      ChunkMetadata *chunks = HASH_MAP[i].chunks;
      for (int j = 0; j < LEVEL_SIZE; j++) {
        callback(chunks + j);
      }
    }
  }
}
