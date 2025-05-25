#include "../include/helper.h"
#include <stdint.h>
uint16_t get_size_from_index(uint8_t index) { return 8 << index; }
uint8_t get_index_from_size(uint16_t value) {
  value--;
  value |= (value >> 1);
  value |= (value >> 2);
  value |= (value >> 4);
  value |= (value >> 8);
  value++;
  uint8_t result = 0;
  value >>= 3;
  while (!(value & 1) && value != 0) {
    value >>= 1;
    result++;
  }
  return result;
}
