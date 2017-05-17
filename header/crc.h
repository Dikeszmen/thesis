#ifndef CRC_H_INCLUDED
#define CRC_H_INCLUDED

//#include <stdint.h>
#define POLYNOMIAL 0xd8
#define WIDTH (8 * sizeof(Crc))
#define TOPBIT (1 << (WIDTH-1))
#define BYTE 8

uint16_t addCRC();

int addCrcByte(uint16_t packetCrc, uint16_t countedCrc);


#endif // CRC_H_INCLUDED