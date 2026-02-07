#ifndef __W25Q128_H
#define __W25Q128_H
#include <stdint.h>

#define PASSWORD_ADDRESS 0x00010000

void W25Q128_Init(void);
void W25Q128_ReadID(uint8_t *MID, uint16_t *DID);
void W25Q128_PageProgram(uint32_t Address, uint8_t *DataArray, uint16_t Count);
void W25Q128_SectorErase(uint32_t Address);
void W25Q128_ReadData(uint32_t Address, uint8_t *DataArray, uint32_t Count);
uint8_t W25Q128_ReadSR1(void);


uint8_t W25Q128_ReadSR2(void);
#endif



