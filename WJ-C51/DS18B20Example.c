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

void DS18B20Example()
{
    char RomChar[17];
    char temper[8];

    DS18B20_SendChangeCmd(); //开始温度转化。因为温度转化需要较大时间，故提前开始而不在while循环中才开始,以避免开机显示第一行后要等一秒才会显示第二行。
    LCD1602_Init();          //马上初始化液晶，以免液晶第一行显示一行黑格子

    DS18B20_RomChar(RomChar); //将序列码转化为字符串
    LCD1602_Write_String(0, 0, RomChar);
    Delay_Ms(1000);

    while (1)
    {
        DS18B20_GetTmpStr(temper);
        LCD1602_Write_String(4, 1, temper);
        DS18B20_SendChangeCmd(); //开始温度转化
        Delay_Ms(1000);          //温度转换时间需要750ms以上
    }
}
