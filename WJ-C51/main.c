/******************74595芯片说明*****************
SCLR=L:移位寄存器清零
（无需控制，板上常接H）
SRCLK=上沿，存入SER
RCK=上沿，锁存移位寄存器中的值
***********************************************/

#include <reg52.h>

sbit RCK = P2 ^ 5;   //输出存储器锁存时钟线
sbit SRCLK = P2 ^ 6; //数据输入时钟线
sbit SER = P2 ^ 7;   //数据线

unsigned char code encode[] = {
	// 0    1     2     3     4     5     6     7     8     9    消隐
	0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F, 0x00};

/**
 * @param int 二进制位表示要显示的数码管位置。0表示选中，1未选中。高位表示左边晶体管，低位在右
 * @param int 表示要显示的值
 */

void Sent(int a)
{
	int i;
	for (i = 0; i < 8; i++) //发送8位数据
	{
		if (a & 0x80) //1000 0000。如果a的最高位为真
			SER = 1;  //SER为要存入的值
		else
			SER = 0;
		SRCLK = 1; //产生一个上沿，将SER存入
		SRCLK = 0;
		a <<= 1; //左移1位
	}
}

void Display(int loca, int num)
{
	int lo, i;
	lo = 0;
	for (i = 0; i < 8; i++) //lo为loca左右颠倒
	{						//使得高位表示左边晶体管，低位在右
		lo <<= 1;
		if (loca & 0x01)
			lo |= 0x01;
		loca >>= 1;
	}

	Sent(lo);		   //发送要显示的位置，8位
	Sent(encode[num]); //发送要显示的值，将前8位位置数据推送至下一个串联的移位寄存器
	RCK = 1;		   //产生一个上沿，输出存储器中锁存的数据
	RCK = 0;
}

void main()
{
	Display(0xFE, 0);
	for(;;);
}