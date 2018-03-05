#ifndef __DS18B20_H
#define __DS18B20_H

bit DS18B20_Init(); //初始化
bit DS18B20_ReadBit();
uchar DS18B20_ReadByte();
void DS18B20_WriteBit(char bitval);
void DS18B20_WriteByte(uchar dat);
void DS18B20_SendChangeCmd();
void DS18B20_SendReadCmd();
float DS18B20_GetTmpValue();
DS18B20_ReadRomCord(void);
uchar DS18B20_CRC8();
void DS18B20_RomChar(char *RomChar);
void DS18B20_GetTmpStr(char *str);
void DS18B20_temperToStr(float v, char *str);


#endif