//**********************************************************************************************************************
// FileName : softCRC.h
// FilePath : /utility
// Author   : Christian Marty
// Date		: 24.09.2018
// Website  : www.christian-marty.ch
//**********************************************************************************************************************
#ifndef SOFT_CRC_H_
#define SOFT_CRC_H_
#include <stdint.h>

#ifdef __cplusplus
	extern "C" {
#endif

//**********************************************************************************************************************
// Name       : CCITT
// Polynomial : 0x1021
// Note       :
//**********************************************************************************************************************
static inline uint16_t crc16_addByte(uint16_t crc, uint8_t data)
{
	uint8_t x = crc >> 8 ^ data;
	x ^= x>>4;
	return (crc << 8) ^ ((uint16_t)(x << 12)) ^ ((uint16_t)(x <<5)) ^ ((uint16_t)x);
}

static inline uint16_t crc16(const uint8_t* data, uint8_t length)
{
	uint16_t crc = 0xFFFF;

	while (length--)
	{
		crc = crc16_addByte(crc, *data++);
	}
	return crc;
}

#ifdef __cplusplus
	}
#endif
#endif /* SOFT_CRC_H_ */