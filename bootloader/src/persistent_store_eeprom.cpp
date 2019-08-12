#include "persistent_store_api.h"
#include "utility.h"

bool PersistentStore::access_start(void)
{
  return true;
}

bool PersistentStore::access_finish(void)
{
  return true;
}

bool PersistentStore::write_data(int &pos, const uint8_t *value, size_t size, uint16_t *crc)
{
  while (size--)
  {
    uint8_t * const p = (uint8_t * const)pos;
    uint8_t v = *value;
    // EEPROM has only ~100,000 write cycles,
    // so only write bytes that have changed!
    if (v != eeprom_read_byte(p))
    {
      eeprom_write_byte(p, v);
      if (eeprom_read_byte(p) != v)
      {
        Serial.println("Error writing to EEPROM!");
        return true;
      }
    }
    crc16(crc, &v, 1);
    pos++;
    value++;
  };
  return false;
}

bool PersistentStore::read_data(int &pos, uint8_t* value, size_t size, uint16_t *crc, const bool writing)
{
  do
  {
    uint8_t c = eeprom_read_byte((uint8_t*)pos);
    *value = c;
    crc16(crc, &c, 1);
    pos++;
    value++;
  } while (--size);
  return false;
}

size_t PersistentStore::capacity()
{
  return E2END + 1;
}
