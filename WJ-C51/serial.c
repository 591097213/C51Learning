
#include <reg52.h>
#include "datatype.h"
/**
 * 串口初始化，方式1，波特率9600.
 */
void Uart_init(void)
{
    TMOD = 0x20; //T1 方式2
    PCON = 0x00; //SMOD = 0
    SCON = 0x50; //方式1 8个数据位
    TH1 = 0xfd;
    TL1 = 0xfd;
    TR1 = 1; //启动定时器1
    ES = 1;  //开串口中断
    EA = 1;  //开总中断
}

/**
 * 用PC发送一字节数据，在8个LED中显示这个数据。LED点亮代表1
 * 例如用串口调试助手发送ff(设为16进制发送)点亮所有LED
 */
void serialExample(void)
{
    uchar temp = 0;
    Uart_init(); //初始化串口，波特率初始化为9600
    while (1)
    {
        if (RI) //串口接收标志被置位说明单片机已经接收到数据
        {
            RI = 0;      //清除接收标志
            temp = SBUF; //将数据从缓冲区读出来
        }
        P1 = ~temp; //将接收到的数据通过LED灯显示出来
    }
}