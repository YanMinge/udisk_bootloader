#include "utility.h"

bool have_table = false ;
uint32_t crc_table[256] ;

void crc16(uint16_t *crc, const void * const data, uint16_t cnt)
{
  uint8_t *ptr = (uint8_t *)data;
  while (cnt--)
  {
    *crc = (uint16_t)(*crc ^ (uint16_t)(((uint16_t)*ptr++) << 8));
    for (uint8_t i = 0; i < 8; i++)
      *crc = (uint16_t)((*crc & 0x8000) ? ((uint16_t)(*crc << 1) ^ 0x1021) : (*crc << 1));
  }
}

void crc32_init_table(void)
{
  uint32_t c;
  uint32_t i, j;
  if(have_table)
  {
    return;
  }
  for (i = 0; i < 256; i++)
  {
    c = i;
    for (j = 0; j < 8; j++)
    {
      if (c & 1)
        c = 0xedb88320L ^ (c >> 1);
      else
        c = c >> 1;
    }
    crc_table[i] = c;
  }
  have_table = true ;
}

uint32_t crc32(uint32_t crc, uint8_t *buffer, uint32_t size, uint8_t mode)
{
	uint32_t i;
  crc32_init_table();
	for (i = 0; i < size; i++)
	{
		crc = crc_table[(crc ^ buffer[i]) & 0xff] ^ (crc >> 8);
	}
  if(mode == 0)
  {
    return crc;
  }
  else
  {
    return crc^0xFFFFFFFF;
  }
}
