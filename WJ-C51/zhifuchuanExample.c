/*********************************************************************************
* 【程序功能】： 往串口发送一字节数据（可通过超级终端或者串口调试助手显示出来，
		  超级终端波特率选为9600）。
		  超级终端设置好后，每按下一次S15键，松开后均会在超级终端
		  上显示一串字母"吃不胖学不忘天天向上！"。			   			            			    
* 【使用说明】： 波特率9600  接收的时候请选择文本模式。需将本文件改为GB2312编码，编译、烧录后串口调试助手中才能正常显示中文
**********************************************************************************/
#include <reg52.h>
#include "qxmcs51_config.h"
#include "delay.h"

unsigned char zifuchuan[] = "吃不胖学不忘天天向上！\n"; //待显示字符。
volatile unsigned char sending;
sbit S4 = P3 ^ 2;

void init(void) //串口初始化
{
    EA = 0;                                     //暂时关闭中断
    TMOD &= 0x0F;                               //定时器1模式控制在高4位
    TMOD |= 0x20;                               //定时器1工作在模式2，自动重装模式
    SCON = 0x50;                                //串口工作在模式1
    TH1 = 256 - MAIN_Fosc / (botelv * 12 * 16); //计算定时器重装值
    TL1 = 256 - MAIN_Fosc / (botelv * 12 * 16);
    PCON |= 0x80; //串口波特率加倍
    ES = 1;       //串行中断允许
    TR1 = 1;      //启动定时器1
    REN = 1;      //允许接收
    EA = 1;       //允许中断
}

void send(unsigned char d) //发送一个字节的数据，形参d即为待发送数据。
{

    SBUF = d;    //将数据写入到串口缓冲
    sending = 1; //设置发送标志
    while (sending)
        ; //等待发送完毕
}

void sendc(unsigned char *pd)
{
    while ((*pd) != '\0') //发送字符串，直到遇到0才结束
    {
        send(*pd); //发送一个字符
        pd++;      //移动到下一个字符
    }
}

void zifuchuanExample()
{
    init();
    while (1)
    {
        if (S4 == 0)
        {
            Delay_Ms(20);
            if (!S4)
            {
                while (!S4)
                    ;
                sendc(zifuchuan);
            }
        }
    }
}

void uart(void) interrupt 4 //串口发送中断
{
    if (RI) //收到数据
    {
        RI = 0; //清中断请求
    }
    else //发送完一字节数据
    {
        TI = 0;
        sending = 0; //清正在发送标志
    }
}