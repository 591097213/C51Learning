#ifndef __DS1302_H
#define __DS1302_H

#include "datatype.h"

void DS1302_Write_Byte(uchar dat); //写一个字节函数
uchar DS1302_Read_Byte(); //读一个字节函数
void DS1302_Write(uchar add, uchar dat); //向时钟芯片写入函数，指定地址，数据
uchar DS1302_Read(uchar add); //从芯片读出数据，指定地址
uchar turnBCD(uchar bcd); //BCD码转换为十进制函数
void DS1302_Init(); //1302时钟芯片初始化函数



#endif