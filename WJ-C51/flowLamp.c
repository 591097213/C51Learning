/**
 * @brief 流水灯的各种调用
 * 
 */
#include <reg52.h> //头文件
#include "delay.h"

sbit p10 = P1 ^ 0;
sbit p11 = P1 ^ 1;
sbit p12 = P1 ^ 2;
sbit p13 = P1 ^ 3;
sbit p14 = P1 ^ 4;
sbit p15 = P1 ^ 5;
sbit p16 = P1 ^ 6;
sbit p17 = P1 ^ 7;

void rzmzy();   //红色走马左移声明
void rlszy();   //红色流水左移声明
void rlsyy();   //红色流水右移声明
void rzmyy();   //红色走马右移声明
void rzmnwyd(); //红色走马内外移动声明
void rlsnwyd(); //红色流水内外移动声明
void rtbyd();   //红色跳变移动声明
void rsdzmyy(); //红色3灯走马右移声明
void rsdzmzy(); //红色3灯走马左移声明
void rjl();		//红色渐亮声明
void rja();		//红色渐暗声明
void rdlszy();  //倒流水左移函数

//rdlsyy();	//倒流水右移函数，还未加入

void zkb(unsigned char d); //调整占空比

void rzmyy() //走马拖尾右移函数
{
	P1 = 0x7f;
	Delay_Ms(60);
	P1 = 0xbf;
	if (p17 == 1)
	{
		unsigned int a;
		for (a = 0; a < 350; a++) //调节点亮时间
		{
			unsigned char i;
			p17 = 0;
			for (i = 0; i < 15; i++)
				; //调节亮度高低
			p17 = 1;
			for (i = 0; i < 50; i++)
				;
		}
	}
	P1 = 0xdf;
	if (p16 == 1)
	{
		unsigned int a;
		for (a = 0; a < 350; a++) //调节点亮时间
		{
			unsigned char i;
			p16 = 0;
			for (i = 0; i < 15; i++)
				; //调节亮度高低
			p17 = 0;
			for (i = 0; i < 1; i++)
				; //调节亮度高低
			p16 = 1;
			p17 = 1;
			for (i = 0; i < 50; i++)
				;
		}
	}
	P1 = 0xef;
	if (p15 == 1)
	{
		unsigned int a;
		for (a = 0; a < 350; a++) //调节点亮时间
		{
			unsigned char i;
			p15 = 0;
			for (i = 0; i < 15; i++)
				; //调节亮度高低
			p16 = 0;
			for (i = 0; i < 1; i++)
				; //调节亮度高低
			p15 = 1;
			p16 = 1;
			for (i = 0; i < 50; i++)
				;
		}
	}
	P1 = 0xf7;
	if (p14 == 1)
	{
		unsigned int a;
		for (a = 0; a < 350; a++) //调节点亮时间
		{
			unsigned char i;
			p14 = 0;
			for (i = 0; i < 15; i++)
				; //调节亮度高低
			p15 = 0;
			for (i = 0; i < 1; i++)
				; //调节亮度高低
			p14 = 1;
			p15 = 1;
			for (i = 0; i < 50; i++)
				;
		}
	}
	P1 = 0xfb;
	if (p13 == 1)
	{
		unsigned int a;
		for (a = 0; a < 350; a++) //调节点亮时间
		{
			unsigned char i;
			p13 = 0;
			for (i = 0; i < 15; i++)
				; //调节亮度高低
			p14 = 0;
			for (i = 0; i < 1; i++)
				; //调节亮度高低
			p13 = 1;
			p14 = 1;
			for (i = 0; i < 50; i++)
				;
		}
	}
	P1 = 0xfd;
	if (p12 == 1)
	{
		unsigned int a;
		for (a = 0; a < 350; a++) //调节点亮时间
		{
			unsigned char i;
			p12 = 0;
			for (i = 0; i < 15; i++)
				; //调节亮度高低
			p13 = 0;
			for (i = 0; i < 1; i++)
				; //调节亮度高低
			p12 = 1;
			p13 = 1;
			for (i = 0; i < 50; i++)
				;
		}
	}
	P1 = 0xfe;
	if (p11 == 1)
	{
		unsigned int a;
		for (a = 0; a < 350; a++) //调节点亮时间
		{
			unsigned char i;
			p11 = 0;
			for (i = 0; i < 15; i++)
				; //调节亮度高低
			p12 = 0;
			for (i = 0; i < 1; i++)
				; //调节亮度高低
			p11 = 1;
			p12 = 1;
			for (i = 0; i < 50; i++)
				;
		}
	}
	P1 = 0xff;
	if (p10 == 1)
	{
		unsigned int a;
		for (a = 0; a < 350; a++) //调节点亮时间
		{
			unsigned char i;
			p10 = 0;
			for (i = 0; i < 15; i++)
				; //调节亮度高低
			p11 = 0;
			for (i = 0; i < 1; i++)
				; //调节亮度高低
			p10 = 1;
			p11 = 1;
			for (i = 0; i < 50; i++)
				;
		}
	}
	if (p17 == 1)
	{
		unsigned int a;
		for (a = 0; a < 350; a++) //调节点亮时间
		{
			unsigned char i;
			p10 = 0;
			for (i = 0; i < 1; i++)
				; //调节亮度高低
			p10 = 1;
			for (i = 0; i < 50; i++)
				;
		}
	}
}
/****************************************************************************/
void rzmzy() //走马拖尾左移函数
{
	P1 = 0xfe;
	Delay_Ms(60);
	P1 = 0xfd;
	if (p10 == 1)
	{
		unsigned int a;
		for (a = 0; a < 350; a++) //调节点亮时间
		{
			unsigned char i;
			p10 = 0;
			for (i = 0; i < 10; i++)
				; //调节亮度高低
			p10 = 1;
			for (i = 0; i < 50; i++)
				;
		}
	}
	P1 = 0xfb;
	if (p11 == 1)
	{
		unsigned int a;
		for (a = 0; a < 350; a++) //调节点亮时间
		{
			unsigned char i;
			p11 = 0;
			for (i = 0; i < 15; i++)
				; //调节亮度高低
			p10 = 0;
			for (i = 0; i < 1; i++)
				; //调节亮度高低
			p11 = 1;
			p10 = 1;
			for (i = 0; i < 50; i++)
				;
		}
	}
	P1 = 0xf7;
	if (p12 == 1)
	{
		unsigned int a;
		for (a = 0; a < 350; a++) //调节点亮时间
		{
			unsigned char i;
			p12 = 0;
			for (i = 0; i < 15; i++)
				; //调节亮度高低
			p11 = 0;
			for (i = 0; i < 1; i++)
				; //调节亮度高低
			p12 = 1;
			p11 = 1;
			for (i = 0; i < 50; i++)
				;
		}
	}
	P1 = 0xef;
	if (p13 == 1)
	{
		unsigned int a;
		for (a = 0; a < 350; a++) //调节点亮时间
		{
			unsigned char i;
			p13 = 0;
			for (i = 0; i < 15; i++)
				; //调节亮度高低
			p12 = 0;
			for (i = 0; i < 1; i++)
				; //调节亮度高低
			p13 = 1;
			p12 = 1;
			for (i = 0; i < 50; i++)
				;
		}
	}
	P1 = 0xdf;
	if (p14 == 1)
	{
		unsigned int a;
		for (a = 0; a < 350; a++) //调节点亮时间
		{
			unsigned char i;
			p14 = 0;
			for (i = 0; i < 15; i++)
				; //调节亮度高低
			p13 = 0;
			for (i = 0; i < 1; i++)
				; //调节亮度高低
			p14 = 1;
			p13 = 1;
			for (i = 0; i < 50; i++)
				;
		}
	}
	P1 = 0xbf;
	if (p15 == 1)
	{
		unsigned int a;
		for (a = 0; a < 350; a++) //调节点亮时间
		{
			unsigned char i;
			p15 = 0;
			for (i = 0; i < 15; i++)
				; //调节亮度高低
			p14 = 0;
			for (i = 0; i < 1; i++)
				; //调节亮度高低
			p15 = 1;
			p14 = 1;
			for (i = 0; i < 50; i++)
				;
		}
	}
	P1 = 0x7f;
	if (p16 == 1)
	{
		unsigned int a;
		for (a = 0; a < 350; a++) //调节点亮时间
		{
			unsigned char i;
			p16 = 0;
			for (i = 0; i < 15; i++)
				; //调节亮度高低
			p15 = 0;
			for (i = 0; i < 1; i++)
				; //调节亮度高低
			p16 = 1;
			p15 = 1;
			for (i = 0; i < 50; i++)
				;
		}
	}
	P1 = 0xff;
	if (p17 == 1)
	{
		unsigned int a;
		for (a = 0; a < 350; a++) //调节点亮时间
		{
			unsigned char i;
			p17 = 0;
			for (i = 0; i < 15; i++)
				; //调节亮度高低
			p16 = 0;
			for (i = 0; i < 1; i++)
				; //调节亮度高低
			p17 = 1;
			p16 = 1;
			for (i = 0; i < 50; i++)
				;
		}
	}
	if (p10 == 1)
	{
		unsigned int a;
		for (a = 0; a < 350; a++) //调节点亮时间
		{
			unsigned char i;
			p17 = 0;
			for (i = 0; i < 1; i++)
				; //调节亮度高低
			p17 = 1;
			for (i = 0; i < 50; i++)
				;
		}
	}
}
/****************************************************************************/
void rlszy() //流水左移函数
{
	unsigned char a, i, led; //声明无字符整形变量a,i,led
	led = 0xfe;				 //左移初始值定位
	P1 = led;				 //输出
	Delay_Ms(120);			 //延时
	for (i = 1; i < 8; i++)  //设置移动7次
	{
		a = led << i; //灯左移i位
		P1 = a;		  //依次点亮
		Delay_Ms(120);
	}
}
/***************************************************************************/
void rlsyy() //流水右移函数
{
	unsigned char a, i, led; //声明无字符整形变量a,i,led
	led = 0x7f;				 //左移初始值定位
	P1 = led;				 //输出
	Delay_Ms(120);			 //延时
	for (i = 1; i < 8; i++)  //设置移动7次
	{
		a = led >> i; //灯右移i位
		P1 = a;		  //依次点亮
		Delay_Ms(120);
	}
}
/***************************************************************************/
void rdlszy() //倒流水左移函数
{
	P1 = 0xfe;
	Delay_Ms(60);
	P1 = 0xfd;
	Delay_Ms(60);
	P1 = 0xfb;
	Delay_Ms(60);
	P1 = 0xf7;
	Delay_Ms(60);
	P1 = 0xef;
	Delay_Ms(60);
	P1 = 0xdf;
	Delay_Ms(60);
	P1 = 0xbf;
	Delay_Ms(60);
	P1 = 0x7f;
	Delay_Ms(60);

	P1 = 0x7e;
	Delay_Ms(60);
	P1 = 0x7d;
	Delay_Ms(60);
	P1 = 0x7b;
	Delay_Ms(60);
	P1 = 0x6f;
	Delay_Ms(60);
	P1 = 0x5f;
	Delay_Ms(60);
	P1 = 0x3f;
	Delay_Ms(60);

	P1 = 0x3d;
	Delay_Ms(60);
	P1 = 0x3b;
	Delay_Ms(60);
	P1 = 0x37;
	Delay_Ms(60);
	P1 = 0x2f;
	Delay_Ms(60);
	P1 = 0x1f;
	Delay_Ms(60);

	P1 = 0x1e;
	Delay_Ms(60);
	P1 = 0x1d;
	Delay_Ms(60);
	P1 = 0x1b;
	Delay_Ms(60);
	P1 = 0x17;
	Delay_Ms(60);
	P1 = 0x0f;
	Delay_Ms(60);

	P1 = 0x0e;
	Delay_Ms(60);
	P1 = 0x0d;
	Delay_Ms(60);
	P1 = 0x0b;
	Delay_Ms(60);
	P1 = 0x07;
	Delay_Ms(60);

	P1 = 0x06;
	Delay_Ms(60);
	P1 = 0x05;
	Delay_Ms(60);
	P1 = 0x04;
	Delay_Ms(60);
	P1 = 0x03;
	Delay_Ms(60);

	P1 = 0x02;
	Delay_Ms(60);
	P1 = 0x01;
	Delay_Ms(60);
	P1 = 0x00;
}
/***************************************************************************
rdlsyy()	//倒流水右移函数,参考上面左移
{
	
}
/***************************************************************************/
void rsdzmyy() //3灯走马右移函数 ，可以参考单灯走马加入拖尾
{
	P1 = 0x1f;
	Delay_Ms(60);
	P1 = 0x8f;
	Delay_Ms(60);
	P1 = 0xc7;
	Delay_Ms(60);
	P1 = 0xe3;
	Delay_Ms(60);
	P1 = 0xf1;
	Delay_Ms(60);
	P1 = 0xf8;
	Delay_Ms(60);
	P1 = 0xfc;
	Delay_Ms(60);
	P1 = 0xfe;
	Delay_Ms(60);
	P1 = 0xff;
}
/****************************************************************************/
void rsdzmzy() //3灯走马左移函数， 可以参考单灯走马加入拖尾
{
	P1 = 0xf8;
	Delay_Ms(60);
	P1 = 0xf1;
	Delay_Ms(60);
	P1 = 0xe3;
	Delay_Ms(60);
	P1 = 0xc7;
	Delay_Ms(60);
	P1 = 0x8f;
	Delay_Ms(60);
	P1 = 0x1f;
	Delay_Ms(60);
	P1 = 0x3f;
	Delay_Ms(60);
	P1 = 0x7f;
	Delay_Ms(60);
	P1 = 0xff;
}
/****************************************************************************/
void rzmnwyd() //走马内外移动函数
{
	P1 = 0x7e; //
	Delay_Ms(90);
	P1 = 0xbd;
	Delay_Ms(90);
	P1 = 0xdb;
	Delay_Ms(90);
	P1 = 0xe7;
	Delay_Ms(90);
	P1 = 0xff;
	Delay_Ms(300);
	P1 = 0xe7;
	Delay_Ms(90);
	P1 = 0xdb;
	Delay_Ms(90);
	P1 = 0xbd;
	Delay_Ms(90);
	P1 = 0x7e;
	Delay_Ms(90);
	P1 = 0xff;
}
/****************************************************************************/
void rlsnwyd() //红色流水内外移动函数
{
	P1 = 0xe7; //
	Delay_Ms(90);
	P1 = 0xc3;
	Delay_Ms(90);
	P1 = 0x81;
	Delay_Ms(90);
	P1 = 0x00;
	rja();
	Delay_Ms(300);
	P1 = 0x7e;
	Delay_Ms(90);
	P1 = 0x3c;
	Delay_Ms(90);
	P1 = 0x18;
	Delay_Ms(90);
	P1 = 0x00;
	rja();
}
/****************************************************************************/
void rtbyd() //红色跳变8模式移动函数
{
	P1 = 0x1f;
	Delay_Ms(180);
	P1 = 0xff;
	Delay_Ms(120);
	P1 = 0xf8;
	Delay_Ms(180);
	P1 = 0xff;
	Delay_Ms(120);
	P1 = 0xe7;
	Delay_Ms(180);
	P1 = 0xff;
	Delay_Ms(300);
	/****************************************************************************/
	P1 = 0xfc;
	Delay_Ms(180);
	P1 = 0xff;
	Delay_Ms(120);
	P1 = 0x3f;
	Delay_Ms(180);
	P1 = 0xff;
	Delay_Ms(300);
	/****************************************************************************/
	P1 = 0x3c;
	Delay_Ms(180);
	P1 = 0xff;
	Delay_Ms(120);
	P1 = 0x3c;
	Delay_Ms(180);
	P1 = 0xff;
	Delay_Ms(300);
	/****************************************************************************/
	P1 = 0xc3;
	Delay_Ms(120);
	P1 = 0xff;
	Delay_Ms(120);
	P1 = 0xc3;
	Delay_Ms(120);
	P1 = 0xff;
	Delay_Ms(120);
	/****************************************************************************/
	P1 = 0xf0;
	Delay_Ms(180);
	P1 = 0xff;
	Delay_Ms(120);
	P1 = 0xf0;
	Delay_Ms(180);
	P1 = 0xff;
	Delay_Ms(300);
	/****************************************************************************/
	P1 = 0xc3;
	Delay_Ms(180);
	P1 = 0xff;
	Delay_Ms(120);
	P1 = 0xc3;
	Delay_Ms(300);
	/****************************************************************************/
	P1 = 0x0f;
	Delay_Ms(180);
	P1 = 0xff;
	Delay_Ms(120);
	P1 = 0x0f;
	Delay_Ms(180);
	P1 = 0xff;
	Delay_Ms(300);
	/****************************************************************************/
	rjl();
	P1 = 0x00;
	Delay_Ms(100);
	P1 = 0xff;
	Delay_Ms(35);
	P1 = 0x00;
	Delay_Ms(100);
	P1 = 0xff;
	Delay_Ms(35);
	P1 = 0x00;
	Delay_Ms(100);
	P1 = 0xff;
	Delay_Ms(35);
	P1 = 0x00;
	Delay_Ms(100);
	P1 = 0xff;
	Delay_Ms(35);
	P1 = 0x00;
	Delay_Ms(100);
	P1 = 0xff;
	Delay_Ms(35);
	rja();
	Delay_Ms(180);
	P1 = 0xff;
}

void rjl() //渐亮函数
{
	unsigned char a, b; //a控制延时长度　b控制循环周期
	P1 = 0xff;
	a = 0;
	for (b = 250; b > 0; b--) //循环周期设置
	{
		a++;
		zkb(a); //延时长度随a而改变
		P1 = 0xff;
		a = ~a; //a值取反，决定灯灭时的占空比
		zkb(a);
		P1 = 0x00;
		a = ~a; //a值取反　使a回到原值继续循环
	}
}
/****************************************************************************/
void rja() //渐暗函数
{
	unsigned char a, b; //a控制延时长度　b控制循环周期
	P1 = 0x00;			//亮起LED
	a = 0;
	for (b = 250; b > 0; b--)
	{
		a++;
		zkb(a);
		P1 = 0x00;
		a = ~a;
		zkb(a);
		P1 = 0xff;
		a = ~a;
	}
}
/****************************************************************************/
void zkb(unsigned char d) //占空比函数，由d决定延时长度
{
	unsigned char i;
	while (--d != 0)
	{
		for (i = 0; i < 1; i++)
			; //占空比时间设置
	}
}
/****************************************************************************/