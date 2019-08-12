#pragma once

#include <stddef.h>
#include <stdint.h>

#define E2END 0xFFF   // EEPROM end address

class PersistentStore {
public:
  static bool access_start();
  static bool access_finish();
  static bool write_data(int &pos, const uint8_t *value, size_t size, uint16_t *crc);
  static bool read_data(int &pos, uint8_t* value, size_t size, uint16_t *crc, const bool writing=true);
  static size_t capacity();

  static inline bool write_data(const int pos, const uint8_t* value, const size_t size=sizeof(uint8_t)) {
    int data_pos = pos;
    uint16_t crc = 0;
    return write_data(data_pos, value, size, &crc);
  }

  static inline bool write_data(const int pos, const uint8_t value) { return write_data(pos, &value); }

  static inline bool read_data(const int pos, uint8_t* value, const size_t size=1) {
    int data_pos = pos;
    uint16_t crc = 0;
    return read_data(data_pos, value, size, &crc);
  }
};

extern PersistentStore persistentStore;
