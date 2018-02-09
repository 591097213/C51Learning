#include <reg51.h>

typedef unsigned char uint8;
typedef unsigned int  uint16;

sbit down = P3^0;
sbit up = P3^1;

uint8 t = 0;
uint8 PWM_T = 0;   //占空比控制变

void delay_1ms(uint16 t)
{
	uint16 x,y;
	for(x=t;x>0;x--)
		for(y=120;y>0;y--);
}

void main(void)      
{

	TMOD = 0x02;   //定时器0，工作模式2，8位定时模式
	TH0=210;     //写入预置初值（取值1-255，数越大PWM频率越高）
	TL0=210;     //写入预置值 （取值1-255，数越大PWM频率越高）
	TR0=1;       //启动定时器
	ET0=1;       //允许定时器0中断
	EA=1;        //允许总中断
	P1=0xff; 	 //初始化P1，输出端口
	PWM_T=30;
	 while(1)      
	
	 {   
	  	 if(!up)
		 	{
				if(PWM_T<250)
					{
					  PWM_T++;
					}
				
				delay_1ms(10);
			}
		 if(!down)
		 	{
				if(PWM_T>0)
					{
					  PWM_T--;
					}
				
				delay_1ms(10);
			}

	
	 }      
}
/****************************************************
               /定时器0中断模拟PWM
****************************************************/
timer0() interrupt 1  
{ 
	 t++;    //每次定时器溢出加1
	 
	 if(t==250)   //PWM周期 100个单位
		 {
			  t=0;  //使t=0，开始新的PWM周期
			  P1=0x00;  //输出端口
		 }
	 
	 if(PWM_T==t)  //按照当前占空比切换输出为高电平
		 {  
		 	P1=0xff;        //  
		 }
}
