/*******************************************************************
*                                                                 	*
* 描述：                                                          	*
*                                                                 	*
*   上电后液晶屏先显示信息，接着按下S5，定时开始，再次按下      	*
*                                                                 	*
*   S5暂停第3次按下显示累积计时，第4次按下暂停计时，任何时候按下S10 *
*                                                                 	*
*   计数清零。                                             		    *
*									                                *
*                                                                 	*
************************************************************************/

#include <reg52.h>
#include <intrins.h>
#include "datatype.h"
#include "1602MiaoBiao.h"
#include "LCD1602.h"
#include "beepOne.h"
#include "delay.h"

uchar KeyCount = 0;

sbit S2 = P3 ^ 0;
sbit S3 = P3 ^ 1;

uchar code cdis1[] = {"  STOPWATCH  0  "}; //LCD上要显示的内容
uchar code cdis2[] = {"    JW-MCS51    "};
uchar code cdis3[] = {"TIME            "};

uchar code cdis4[] = {" BEGIN COUNT  1 "};
uchar code cdis5[] = {" PAUSE COUNT  2 "};
uchar code cdis6[] = {" BEGIN COUNT  3 "};
uchar code cdis7[] = {" PAUSE COUNT  4 "};

uchar display[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};        //用于计时。
uchar display2[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; //将display的计时数据转化为两位毫秒，两位秒，两位分，两位时。

/********************************************************
 显示函数
*********************************************************/
void play()
{
    uchar i;
    uchar str[11];
    for (i = 0; i < 6; i++) //处理display2的低6位
    {
        display2[i] = display[i] + 0x30; //0x30=48.转为char类型
    }

    display2[7] = display[6] / 10 + 0x30; //第8位。时单位数据处理
    display2[6] = display[6] % 10 + 0x30; //第7位

    //两位时:两位分:两位秒:两位毫秒
    str[10] = display2[0];
    str[9] = display2[1];
    str[8] = ':';
    str[7] = display2[2];
    str[6] = display2[3];
    str[5] = ':';
    str[4] = display2[4];
    str[3] = display2[5];
    str[2] = ':';
    str[1] = display2[6];
    str[0] = display2[7];

    LCD1602_Write_String(5, 1, str);
}

/********************************************************
 主函数
*********************************************************/
void miaobiao()
{
    uchar m;

    TMOD = 0x01; //定时器0模式1.16位定时器
    TH0 = 0xdc;
    TL0 = 0x00; //50ms定时
    EA = 1;     //开全局中断
    ET0 = 1;    //开T0中断

    LCD1602_Init();

    LCD1602_Write_String(0, 0, cdis1); //在第一行显示字符

    LCD1602_Write_Com(0x40 | 0x80); //设置显示位置为第二行
    for (m = 0; m < 16; m++)
    {
        LCD1602_Write_Dat(cdis2[m]); //显示字符
        Delay_Ms(60);                //做出渐入的效果
    }

    Delay_Ms(100);

    LCD1602_Write_String(0, 1, cdis3); //在第二行显示字符

    while (1)
    {
        if (S2 == 0)
        {
            KeyCount++; //计数
            beepOne();
            Delay_Ms(100);

            switch (KeyCount)
            {
            case 1:
                TR0 = 1;                           //启动中断
                LCD1602_Write_String(0, 0, cdis4); //在第一行显示字符
                break;

            case 2:
                TR0 = 0;                           //停止中断
                LCD1602_Write_String(0, 0, cdis5); //在第一行显示字符
                break;

            case 3:
                TR0 = 1;                           //启动中断
                LCD1602_Write_String(0, 0, cdis6); //在第一行显示字符
                break;

            case 4:
                TR0 = 0;                           //停止中断
                LCD1602_Write_String(0, 0, cdis7); //在第一行显示字符
                break;

            default:
                TR0 = 0; //停止中断
                break;
            }
        }
        if (S3 == 0)
        {
            TR0 = 0; //停止中断
            KeyCount = 0;
            for (m = 0; m < 7; m++)
                display[m] = 0; //计时单元清零
            beepOne();
            LCD1602_Write_String(0, 0, cdis1); //在第一行显示字符串
            Delay_Ms(100);
        }

        play();
    }
}

/*********************************************************
*                                                        *
* Time0中断函数                                          *
*                                                        *
**********************************************************/
void Time0(void) interrupt 1 using 0
{
    TH0 = 0xdc; //10ms定时
    TL0 = 0x00;

    display[0]++; //0.01S

    if (display[0] == 10)
    {
        display[0] = 0;
        display[1]++; //0.1S
    }
    if (display[1] == 10)
    {
        display[1] = 0;
        display[2]++; //秒个位
    }
    if (display[2] == 10)
    {
        display[2] = 0;
        display[3]++; //秒十位
    }
    if (display[3] == 6)
    {
        display[3] = 0;
        display[4]++; //分个位
    }
    if (display[4] == 10)
    {
        display[4] = 0;
        display[5]++; //分十位
    }
    if (display[5] == 6)
    {
        display[5] = 0;
        display[6]++; //时
    }
    if (display[6] == 24)
    {
        display[6] = 0;
    }
}

/*********************************************************/
