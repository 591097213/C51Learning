#include <reg52.h>
#include "delay.h"
#include "beepOne.h"
#include "datatype.h"

sbit beep = P2 ^ 3;
/*********************************************************
 蜂鸣器响一声                                           
**********************************************************/
void beepOne()
{
    unsigned char y, i;
    for (y = 0; y < 180; y++)
    {
        beep = !beep; //BEEP取反
        for (i = 0; i < 70; i++)
        {
            Delay5us();
        }
    }
    beep = 1; //关闭蜂鸣器
}