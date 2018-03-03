/**
 * @brief DS18B20示例
 * 
 * 1602LCD在显示屏第一行显示DS18B20芯片的序列号，第二行显示当前温度
 * 
 * @file DS18B20.c
 * @author your name
 * @date 2018-03-01
 */

#include <reg52.H>
#include <math.h>
#include "datatype.h"
#include "LCD1602.h"
#include "DS18B20.h"
#include "delay.h"

/**
 * @brief LCD1602第二行显示温度
 * 
 * @param v 
 */
void display(int v)
{
    uchar count;
    uchar datas[] = {0, 0, 0, 0, 0}; //读取到的值连同小数最多5位
    uint tmp = abs(v);               //取绝对值
    datas[0] = tmp / 10000;
    datas[1] = tmp % 10000 / 1000;
    datas[2] = tmp % 1000 / 100;
    datas[3] = tmp % 100 / 10;
    datas[4] = tmp % 10;
    LCD1602_Write_Com(0xc0 + 3); //将指针设置在第二行第三个字符

    if (v < 0) //写入符号
    {
        LCD1602_Write_Dat('-');
    }
    else
    {
        LCD1602_Write_Dat('+');
    }

    if (datas[0] != 0)
    {
        LCD1602_Write_Dat('0' + datas[0]); //处理前置0
    }
    for (count = 1; count != 5; count++)
    {
        LCD1602_Write_Dat('0' + datas[count]);
        if (count == 2)
        {
            LCD1602_Write_Dat('.'); //小数点
        }
    }
}

void DS18B20Example()
{
    char RomChar[17];

    DS18B20_SendChangeCmd(); //开始温度转化。因为温度转化需要较大时间，故提前开始而不在while循环中才开始,以避免开机显示第一行后要等一秒才会显示第二行。
    LCD1602_Init();//马上初始化液晶，以免液晶第一行显示一行黑格子
    Delay_Ms(1000);

    DS18B20_RomChar(RomChar); //将序列码转化为字符串


    LCD1602_Write_String(0, 0, RomChar);
    while (1)
    {
        LCD1602_Write_Com(0xc0); //设为第二行
        display(DS18B20_GetTmpValue());
        DS18B20_SendChangeCmd(); //开始温度转化
        Delay_Ms(1000);          //温度转换时间需要750ms以上
    }
}
