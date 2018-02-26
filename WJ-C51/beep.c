

#include <reg52.h>
#include "delay.h"
#include "beep.h"
#include "datatype.h"

//当按键S5被按下时，蜂鸣器响。松开后不响
sbit s2 = P3 ^ 0;
sbit beep = P2 ^ 3;

void beepExample()
{
    while (1)
    {
        if (!s2)
            beep = 0;
        else
            beep = 1;
    }
}

//按下JW - 51 实验板上的S5按键时（一直按着），会发出频率为1000HZ和2000HZ交替
//的信号，通过蜂鸣器发出类似于救护车发出的报警声。 sbit kaiguan = P3 ^ 0;
void baojing()
{
    while (1)
    {
        if (!s2)
        {
            uint m;
            for (m = 800; m > 0; m--) //持续时间0.5ms*800
            {
                beep = ~beep;
                Delay_Ms(5); //2000HZ的信号。
            }
            for (m = 500; m > 0; m--) //持续时间0.5ms*2*500
            {
                beep = ~beep;
                Delay_Ms(5);
                Delay_Ms(5); //1000HZ的信号。
            }
        }
    }
}