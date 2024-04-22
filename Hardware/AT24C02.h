#ifndef __AT24C02_H
#define __AT24C02_H
void AT24C02_Init(void);

void AT24C02_WriteByte(u8 WordAddress,u8 Data);
u8 AT24C02_ReadByte(u8 WordAddress);

void AT24C02_Write_Float(u16 WriteAddr,float *pBuffer);
void AT24C02_Read_Float(u16 ReadAddr,float *pBuffer,u16 NumToRead);
void AT24C02_Erase(uint16_t addr, uint16_t size);
#endif
