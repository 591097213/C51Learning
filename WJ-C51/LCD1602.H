#ifndef __LCD1602_H
#define __LCD1602_H

#include "datatype.h"

INT8U *INT8UtostrDEC(INT8U num);
INT8U *INT8UtostrHEX(INT8U num); //将一个字节的数据转换为字符串
BOOL LCD1602_Check_Busy();       //LCD1602忙判断
INT8U LCDE1602_DATAP_ADDR();                           //LCD1602数据指针地址
void LCD1602_Write_Com(INT8U com);                     //LCD1602写命令
void LCD1602_Write_Dat(INT8U dat);                     //LCD1602写数据
void LCD1602_Write_String(INT8U x, INT8U y, INT8U *s); //LCD1602写字符串
void LCD1602_Init();                                   //LCD1602初始化
uchar *uchartostr(uchar num);                           //将一个字节的数据转换为字符串 或10进制
void LCD1602_Clear();
void LCD1602_Pos(uchar pos);
void LCD1602_Flash();
void LCD1602_Write_Char(uchar x, uchar y, uchar Data);

#endif