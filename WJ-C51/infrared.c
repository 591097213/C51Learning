/**
 * @brief 红外接收相关函数
 *
 * @file infrared.c
 * @author your name
 * @date 2018-03-21
 */

#include "LCD1602.h"
#include "datatype.h"

sbit IR = P3 ^ 2; //红外接口标志

char code Tab[16] = "0123456789ABCDEF";

uchar irtime; //用于计数定时器T0中断中，记录每个脉冲的持续时间

bit irpro_ok, irok; //标志变量
uchar IRcord[4];    //用于存储接收到的字节
uchar irdata[33];   //用于存储每位所占据的时间

uchar TempData[16]; //将受到的4字节转化为十六进制字符

/**
 * @brief 用于计数。定时器/计数器0中断
 *
 */
void tim0_isr(void) interrupt 1 using 1 //约每0.2777ms溢出一次，即irtime每0.2777ms自加一
{
    irtime++; //用于计数2个下降沿之间的时间
}

/**
 * @brief 外部中断0服务函数
 * 用于红外芯片数据接收，将每个脉冲持续时间存入irdata[]中
 * 该函数只能记录32位的完整码数据，不能辨别重复码
 */
void EX0_ISR(void) interrupt 0
{
    //注意这里的变量是static类型，相当于全局变量
    static uchar i; //接收红外信号处理

    if (irtime < 63 && irtime >= 33) //引导码，9ms+4.5ms=13.5ms。这里检测出范围为17.4951~9.1641ms。不在这个范围内则为用户码或数据码
        i = 0;                       //引导码的长度也记录下来了
    irdata[i] = irtime;              //存储每个电平的持续时间，用于以后判断是0还是1
    irtime = 0;                      //归零，再次开始计时
    i++;
    if (i == 33) //接收完32位，i变为33则接收结束。
    {
        irok = 1; //标志位，表示红外数据接收完毕
        i = 0;
    }
}

/*------------------------------------------------
                定时器0初始化
------------------------------------------------*/
void TIM0init(void) //定时器0初始化
{

    TMOD = 0x02; //定时器0工作方式2，TH0是重装值，TL0是初值
    TH0 = 0x00;  //重载值
    TL0 = 0x00;  //初始化值
    ET0 = 1;     //开中断
    TR0 = 1;
}

/*------------------------------------------------
                  外部中断0初始化
------------------------------------------------*/
void EX0init(void)
{
    IT0 = 1; //指定外部中断0下降沿触发，INT0 (P3.2)
    EX0 = 1; //使能外部中断
    EA = 1;  //开总中断
}

/*------------------------------------------------
                  键值处理
------------------------------------------------*/

void Ir_work(void)
{
    TempData[0] = Tab[IRcord[0] / 16]; //处理客户码
    TempData[1] = Tab[IRcord[0] % 16];
    TempData[2] = '-';
    TempData[3] = Tab[IRcord[1] / 16]; //处理客户码
    TempData[4] = Tab[IRcord[1] % 16];
    TempData[5] = '-';
    TempData[6] = Tab[IRcord[2] / 16]; //处理数据码
    TempData[7] = Tab[IRcord[2] % 16];
    TempData[8] = '-';
    TempData[9] = Tab[IRcord[3] / 16]; //处理数据反码
    TempData[10] = Tab[IRcord[3] % 16];

    LCD1602_Write_String(5, 1, TempData);

    irpro_ok = 0; //处理完成标志
}

/*------------------------------------------------
                红外码值处理
------------------------------------------------*/
void Ircordpro(void) //红外码值处理函数
{
    uchar i, j, k;
    uchar cord, value;

    k = 1;                  //注意这里从1开始。irdata[0]记录的是引导码的长度
    for (i = 0; i < 4; i++) //处理4个字节
    {
        for (j = 1; j <= 8; j++) //处理1个字节8位
        {
            cord = irdata[k];
            if (cord > 7) //大于某值为1，这个和晶振有绝对关系，这里使用12M计算，此值可以有一定误差。
                          //0为1.125ms，1为2.25ms。这里取>1.9439ms为1
                value |= 0x80;
            if (j < 8)
            {
                value >>= 1;
            }
            k++;
        }
        IRcord[i] = value;
        value = 0;
    }
    irpro_ok = 1; //处理完毕标志位置1
}
