
// --------------------------------------------------------------------------
// Includes
// --------------------------------------------------------------------------
#include "utility.h"
#include <Wire.h>

#define I2C_ADDRESS(A) ((A) << 1)
// --------------------------------------------------------------------------
// Externals
// --------------------------------------------------------------------------

// --------------------------------------------------------------------------
// Local defines
// --------------------------------------------------------------------------

// --------------------------------------------------------------------------
// Types
// --------------------------------------------------------------------------

// --------------------------------------------------------------------------
// Variables
// --------------------------------------------------------------------------

// --------------------------------------------------------------------------
// Public Variables
// --------------------------------------------------------------------------

// --------------------------------------------------------------------------
// Private Variables
// --------------------------------------------------------------------------

// --------------------------------------------------------------------------
// Function prototypes
// --------------------------------------------------------------------------

// --------------------------------------------------------------------------
// Private functions
// --------------------------------------------------------------------------

// --------------------------------------------------------------------------
// Public functions
// --------------------------------------------------------------------------

static uint8_t eeprom_device_address = 0x50;

static void eeprom_init(void) {
  static bool eeprom_initialized = false;
  if (!eeprom_initialized) {
    Wire.begin();
    eeprom_initialized = true;
  }
}

void eeprom_write_byte(uint8_t *pos, unsigned char value) {
  unsigned eeprom_address = (unsigned) pos;

  eeprom_init();

  Wire.beginTransmission(I2C_ADDRESS(eeprom_device_address));
  Wire.write((int)(eeprom_address >> 8));   // MSB
  Wire.write((int)(eeprom_address & 0xFF)); // LSB
  Wire.write(value);
  Wire.endTransmission();

  // wait for write cycle to complete
  // this could be done more efficiently with "acknowledge polling"
  delay(5);
}

// WARNING: address is a page address, 6-bit end will wrap around
// also, data can be maximum of about 30 bytes, because the Wire library has a buffer of 32 bytes
void eeprom_update_block(const void *pos, void* eeprom_address, size_t n) {
  eeprom_init();

  Wire.beginTransmission(I2C_ADDRESS(eeprom_device_address));
  Wire.write((int)((unsigned)eeprom_address >> 8));   // MSB
  Wire.write((int)((unsigned)eeprom_address & 0xFF)); // LSB
  Wire.endTransmission();

  uint8_t *ptr = (uint8_t*)pos;
  uint8_t flag = 0;
  Wire.requestFrom(eeprom_device_address, (byte)n);
  for (byte c = 0; c < n && Wire.available(); c++)
    flag |= Wire.read() ^ ptr[c];

  if (flag) {
    Wire.beginTransmission(I2C_ADDRESS(eeprom_device_address));
    Wire.write((int)((unsigned)eeprom_address >> 8));   // MSB
    Wire.write((int)((unsigned)eeprom_address & 0xFF)); // LSB
    Wire.write((uint8_t*)pos, n);
    Wire.endTransmission();

    // wait for write cycle to complete
    // this could be done more efficiently with "acknowledge polling"
    delay(5);
  }
}


uint8_t eeprom_read_byte(uint8_t *pos) {
  unsigned eeprom_address = (unsigned)pos;

  eeprom_init();

  Wire.beginTransmission(I2C_ADDRESS(eeprom_device_address));
  Wire.write((int)(eeprom_address >> 8));   // MSB
  Wire.write((int)(eeprom_address & 0xFF)); // LSB
  Wire.endTransmission();
  Wire.requestFrom(I2C_ADDRESS(eeprom_device_address), (int)1);
  return Wire.available() ? Wire.read() : 0xFF;
}

// maybe let's not read more than 30 or 32 bytes at a time!
void eeprom_read_block(void* pos, const void* eeprom_address, size_t n) {
  eeprom_init();

  Wire.beginTransmission(I2C_ADDRESS(eeprom_device_address));
  Wire.write((int)((unsigned)eeprom_address >> 8));   // MSB
  Wire.write((int)((unsigned)eeprom_address & 0xFF)); // LSB
  Wire.endTransmission();
  Wire.requestFrom(I2C_ADDRESS(eeprom_device_address), (int)n);

  for (byte c = 0; c < n; c++ )
    if (Wire.available()) *((uint8_t*)pos + c) = Wire.read();
}
