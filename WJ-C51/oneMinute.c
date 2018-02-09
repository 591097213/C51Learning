#include <reg52.h>

const int myTIME = 0.05 / (12.0 / 12000000); //可修改每次中断的计时

unsigned char code encode[] = {
	//0     1     2     3     4     5     6     7     8     9     A     B     C     D     E     F   消隐
	0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71, 0x00}; //段码

sbit RCK = P2 ^ 5;   //输出存储器锁存时钟线
sbit SRCLK = P2 ^ 6; //数据输入时钟线
sbit SER = P2 ^ 7;   //数据线

/**
 * 向芯片发送1字节数据
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
		SRCLK = 0; //产生一个上沿，将SER存入
		SRCLK = 1;
		a <<= 1; //左移1位
	}
}

/**
 * @param int 二进制位表示要显示的数码管位置。1表示选中，0未选中。高位表示左边晶体管，低位在右
 * @param int 表示要显示的值，支持16进制字符。16表示消影
 * @param int 表示是否有小数点
 */
void Display(int loca, int num, int boo)
{
	int lo, i, temp;
	loca = ~loca; //使得1表示选中，0表示未选中
	lo = 0;
	for (i = 0; i < 8; i++) //lo为loca左右颠倒
	{						//使得高位表示左边晶体管，低位在右
		lo <<= 1;
		if (loca & 0x01)
			lo |= 0x01;
		loca >>= 1;
	}
	temp = encode[num]; //对编码添加小数点
	if (boo)
	{
		temp |= 0x80;
	}

	Sent(lo);   //发送要显示的位置，8位
	Sent(temp); //发送要显示的值，将前8位位置数据推送至下一个串联的移位寄存器
	RCK = 0;	//产生一个上沿，输出存储器中锁存的数据
	RCK = 1;
}

/**
 * 晶体管动态显示
 * @param int[] 传入一个8位的数组，每位表示要显示的值
 * @param int[] 8位数据，表示是否有小数点
 * 该函数需要在死循环中不断执行才有效果
 */
void dynamic(int cha[], int boo[])
{
	int i = 0;
	int loca = 0x80;
	for (; i < 8; i++) //分别显示8个值
	{
		Display(loca, cha[i], boo[i]);
		loca >>= 1;
	}
}

int cha[] = {16, 16, 16, 16, 16, 16, 0, 0};
int boo[] = {0, 0, 0, 0, 0, 0, 0, 0};
int num = 0;
int count = 0;

void main()
{
	TH0 = (65536 - myTIME) / 256;
	TL0 = (65536 - myTIME) % 256;

	IE = 0x82;
	TMOD = 0X01;
	TCON = 0x10;

	for (;;)
	{
		if (num >= 20)
		{
			num = 0;
			count++;
			if (count >= 60)
				count = 0;
			cha[6] = count / 10;
			cha[7] = count % 10;
		}
		dynamic(cha, boo);
	}
}

void T0Interrupt() interrupt 1
{
	TH0 = (65536 - myTIME) / 256;
	TL0 = (65536 - myTIME) % 256;
	num++;
}