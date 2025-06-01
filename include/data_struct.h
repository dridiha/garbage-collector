#ifndef __DATA_STRUCT_H
#define __DATA_STRUCT_H
#define LEVEL_SIZE 1024
#define MAX_BITMAP_SIZE 128
#include <stdbool.h>
#include <stdint.h>
enum State {
  FREE,
  ALLOCATED,
  IN_USE,
};
typedef struct {
  int16_t object_size;
  uint8_t offset;
  uint32_t chunk_size;
  void *base;
  uint8_t bitmap[MAX_BITMAP_SIZE];
} ChunkMetadata;

typedef struct {
  int high_bits;
  ChunkMetadata chunks[LEVEL_SIZE];
} HashBucket;
extern HashBucket HASH_MAP[LEVEL_SIZE];

uint16_t hash(void *key);
int8_t insert_chunk_entry(void *key, HashBucket value);
HashBucket *get_chunk_bucket(void *key);
bool chunk_entry_exists(void *key);

// HashBucket struct functions
//
HashBucket create_hash_bucket(void *key);

// ChunkMetadata struct functions
//
ChunkMetadata create_chunk_metadata(void *base, uint32_t chunk_size,
                                    int16_t object_size, uint8_t offset);

int8_t register_chunk_metadata(void *base, int16_t object_size,
                               uint32_t chunk_size);
ChunkMetadata *find_metadata_by_pointer(void *ptr);
void remove_chunk_metadata(void *ptr);
void parse(void (*callback)(ChunkMetadata *metadata));
#endif // !__DATA_STRUCT_H
