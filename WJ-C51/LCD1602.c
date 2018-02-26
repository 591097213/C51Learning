/*********************************************
LCD显示器相关

*****************************************/

#include "qxmcs51_config.h"
#include "delay.h"
#include "LCD1602.H"
#include "datatype.h"

#define LCD1602_DataPort P0 //LCD1602数据端口

//INT8U strDEC[4]; //将一个字节的数据转换为10进制字符串
INT8U strHEX[3]; //INT8UtostrHEX函数将一个字节的数据转换为16进制字符串。strHEX为全局变量，16进制字符地址
/*
//将一字节数据转换为10进制字符串
INT8U *INT8UtostrDEC(INT8U num)
{
	INT8U i2, i1, i0, i;
	i2 = num / 100;
	i1 = num % 100 / 10;
	i0 = num % 100 % 10;
	strDEC[i] = i2 + 48;
	i++;
	strDEC[i] = i1 + 48;
	i++;
	strDEC[i] = i0 + 48;
	i++;
	strDEC[i] = '\0';
	return strDEC;
}
*/
INT8U *INT8UtostrHEX(INT8U num) //将一个字节的数据转换为16进制字符串
{
	INT8U i = 0;
	switch (num / 16)
	{
	case 0:
		strHEX[i] = '0';
		i++;
		break;
	case 1:
		strHEX[i] = '1';
		i++;
		break;
	case 2:
		strHEX[i] = '2';
		i++;
		break;
	case 3:
		strHEX[i] = '3';
		i++;
		break;
	case 4:
		strHEX[i] = '4';
		i++;
		break;
	case 5:
		strHEX[i] = '5';
		i++;
		break;
	case 6:
		strHEX[i] = '6';
		i++;
		break;
	case 7:
		strHEX[i] = '7';
		i++;
		break;
	case 8:
		strHEX[i] = '8';
		i++;
		break;
	case 9:
		strHEX[i] = '9';
		i++;
		break;
	case 10:
		strHEX[i] = 'A';
		i++;
		break;
	case 11:
		strHEX[i] = 'B';
		i++;
		break;
	case 12:
		strHEX[i] = 'C';
		i++;
		break;
	case 13:
		strHEX[i] = 'D';
		i++;
		break;
	case 14:
		strHEX[i] = 'E';
		i++;
		break;
	case 15:
		strHEX[i] = 'F';
		i++;
		break;
	}

	switch (num % 16)
	{
	case 0:
		strHEX[i] = '0';
		i++;
		break;
	case 1:
		strHEX[i] = '1';
		i++;
		break;
	case 2:
		strHEX[i] = '2';
		i++;
		break;
	case 3:
		strHEX[i] = '3';
		i++;
		break;
	case 4:
		strHEX[i] = '4';
		i++;
		break;
	case 5:
		strHEX[i] = '5';
		i++;
		break;
	case 6:
		strHEX[i] = '6';
		i++;
		break;
	case 7:
		strHEX[i] = '7';
		i++;
		break;
	case 8:
		strHEX[i] = '8';
		i++;
		break;
	case 9:
		strHEX[i] = '9';
		i++;
		break;
	case 10:
		strHEX[i] = 'A';
		i++;
		break;
	case 11:
		strHEX[i] = 'B';
		i++;
		break;
	case 12:
		strHEX[i] = 'C';
		i++;
		break;
	case 13:
		strHEX[i] = 'D';
		i++;
		break;
	case 14:
		strHEX[i] = 'E';
		i++;
		break;
	case 15:
		strHEX[i] = 'F';
		i++;
		break;
	}
	strHEX[i] = '\0';
	return (strHEX);
}

BOOL LCD1602_Check_Busy() //LCD1602忙判断
{
	LCD1602_DataPort = 0xFF;
	LCD_RS = 0; //根据规定，RS为低电平，RW为高电平时，可以读状态
	LCD_RW = 1;
	LCD_EN = 0;
	_nop_();
	LCD_EN = 1;
	return (BOOL)(LCD1602_DataPort & 0x80);
}
/*
void LCD1602_Clear()
{
   LCD1602_Write_Com(0x01);    //显示清屏 
   Delay_Ms(5);	
}
*/

/*
//读取7位的地址指针
INT8U LCDE1602_DATAP_ADDR()
{
	LCD1602_DataPort = 0xFF; 
	LCD_RS = 0; //根据规定，RS为低电平，RW为高电平时，可以读状态
	LCD_RW = 1; 
	LCD_EN = 0; 
	_nop_(); 
	LCD_EN = 1;
	return (LCD1602_DataPort & 0x7f);	
}
*/

void LCD1602_Write_Com(INT8U com) //LCD1602写命令
{
	while (LCD1602_Check_Busy())
		; //忙则等待
	LCD_RS = 0;
	LCD_RW = 0;
	LCD_EN = 1;
	LCD1602_DataPort = com;
	_nop_();
	LCD_EN = 0;
}

void LCD1602_Write_Dat(INT8U dat) //LCD1602写数据
{
	while (LCD1602_Check_Busy())
		; //忙则等待
	LCD_RS = 1;
	LCD_RW = 0;
	LCD_EN = 1;
	LCD1602_DataPort = dat;
	_nop_();
	LCD_EN = 0;
}
//y表示第几行。x表示第几个字符。s是字符串指针
void LCD1602_Write_String(INT8U x, INT8U y, INT8U *s) //LCD1602写字符串
{
	if (y == 0)
	{
		LCD1602_Write_Com(0x80 + x); //表示第一行
	}
	else
	{
		LCD1602_Write_Com(0xC0 + x); //表示第二行
	}
	while (*s != '\0')
	{
		LCD1602_Write_Dat(*s++);
	}
}

void LCD1602_Init() //LCD1602初始化
{
	LCD_RW = 0;
	LED_WE = 0;
	LED_DU = 0;
	LCD_EN = 0;
	LCD1602_Write_Com(0x38); /*显示模式设置*/
	LCD1602_Write_Com(0x06); /*显示光标移动设置*/
	LCD1602_Write_Com(0x0C); /*显示开及光标设置*/
	LCD1602_Write_Com(0x01); /*显示清屏*/
	Delay_Ms(5);
}