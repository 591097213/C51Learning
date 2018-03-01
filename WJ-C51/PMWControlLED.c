
#include <reg52.h>
#include "datatype.h"
#include "qxmcs51_config.h"
#include "delay.h"

INT8U t = 0;
INT8U PWM_T = 0; //占空比控制变

void PMWControlLEDExample(void)
{
	TMOD = 0x02; //定时器0，工作模式2，8位定时模式
	TH0 = 210;   //写入预置初值（取值1-255，数越大PWM频率越高）
	TL0 = 210;   //写入预置值 （取值1-255，数越大PWM频率越高）
	TR0 = 1;	 //启动定时器
	ET0 = 1;	 //允许定时器0中断
	EA = 1;		 //允许总中断
	P1 = 0xff;   //初始化P1，输出端口
	PWM_T = 30;
	while (1)
	{
		if (!BUTTON1)
		{
			if (PWM_T < 250)
			{
				PWM_T++;
			}
			Delay_Ms(10);
		}
		if (!BUTTON0)
		{
			if (PWM_T > 0)
			{
				PWM_T--;
			}
			Delay_Ms(10);
		}
	}
}
/****************************************************
               /定时器0中断模拟PWM
****************************************************/
timer0() interrupt 1
{
	t++; //每次定时器溢出加1

	if (t == 250) //PWM周期 100个单位
	{
		t = 0;	 //使t=0，开始新的PWM周期
		P1 = 0x00; //输出端口
	}

	if (PWM_T == t) //按照当前占空比切换输出为高电平
	{
		P1 = 0xff; //
	}
}
