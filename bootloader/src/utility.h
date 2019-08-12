#include <Arduino.h>

#ifndef  _UTILITY_H
#define  _UTILITY_H
enum language_type : uint8_t
{
  LAN_NULL = 0,
  LAN_CHINESE,
  LAN_ENGLISH,
};

void eeprom_write_byte(uint8_t *pos, unsigned char value);
uint8_t eeprom_read_byte(uint8_t *pos);
void eeprom_read_block (void *__dst, const void *__src, size_t __n);
void eeprom_update_block (const void *__src, void *__dst, size_t __n);

void crc32_init_table(void);
void crc16(uint16_t *crc, const void * const data, uint16_t cnt);
uint32_t crc32(uint32_t crc, uint8_t *buffer, uint32_t size, uint8_t mode);
#endif /* _UTILITY_H */
