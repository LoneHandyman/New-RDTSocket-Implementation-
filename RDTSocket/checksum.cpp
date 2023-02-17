#include "checksum.hpp"

unsigned int crc::CRC32(void* in, const unsigned int& inLen, unsigned int uiCRC32) {
  unsigned char* casted_in = (unsigned char*)in;

  for (unsigned int i = 0; i < inLen; ++i)
    uiCRC32 = ((uiCRC32 >> 8) & 0x00ffffff) ^ uiCRC32_Table[(uiCRC32 ^ (unsigned int)*casted_in++) & 0xff];

  return (uiCRC32 ^ 0xffffffff);
}