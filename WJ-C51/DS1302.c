/**
 * @brief DS1302时钟芯片相关函数
 *
 * @file DS1302.c
 * @author your name
 * @date 2018-03-17
 */


#include <reg52.h>
#include "qxmcs51_config.h"
#include "datatype.h"

void DS1302_Write_Byte(uchar dat) //写一个字节函数
{
    uchar a;
	ACC = dat;
	RST = 1;
	for (a = 8; a > 0; a--)
	{
		IO = ACC0;
		SCLK = 0;
		SCLK = 1;
		ACC = ACC >> 1;
	}
}


uchar DS1302_Read_Byte() //读一个字节函数
{
    uchar a;
	RST = 1;
	for (a = 8; a > 0; a--)
	{
		ACC7 = IO;
		SCLK = 1;
		SCLK = 0;
		ACC = ACC >> 1;
	}
	return (ACC);
}


void DS1302_Write(uchar add, uchar dat) //向时钟芯片写入函数，指定地址，数据
{
	RST = 0;
	SCLK = 0;
	RST = 1;
	DS1302_Write_Byte(add);
	DS1302_Write_Byte(dat);
	SCLK = 1;
	RST = 0;
}


uchar DS1302_Read(uchar add) //从芯片读出数据，指定地址
{
	uchar temp;
	RST = 0;
	SCLK = 0;
	RST = 1;
	DS1302_Write_Byte(add);
	temp = DS1302_Read_Byte();
	SCLK = 1;
	RST = 0;
	return (temp);
}

/**
 * @brief bcd码：
 * 一字节中前四位转为十进制表示十位；
 * 后四位转为十进制表示个位
 *
 * @param bcd 输入bcd码
 * @return uchar 输出十进制数值
 */
uchar turnBCD(uchar bcd) //BCD码转换为十进制函数
{
	uchar shijin;
	shijin = bcd >> 4;//取bcd的高4位置于shijin的低四位
	return (shijin = shijin * 10 + (bcd &= 0x0f)); //返回十进制数
}


void DS1302_Init() //1302时钟芯片初始化函数
{
	RST = 0;
	SCLK = 0;
	DS1302_Write(0x8e, 0x00); //允许写
	/*DS1302_Write(0x80,0x00);//秒：00
	DS1302_Write(0x82,0x00);//分：00
	DS1302_Write(0x84,0x12);//时：12
	DS1302_Write(0x8a,0x04);//周：4
	DS1302_Write(0x86,0x15);//日：15
	DS1302_Write(0x88,0x03);//月：3
	DS1302_Write(0x8c,0x11);//年：11	*/
	DS1302_Write(0x8e, 0x80); //打开保护
}