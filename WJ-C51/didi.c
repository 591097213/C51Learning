#include <reg52.h>
#include "datatype.h"

uint aa;
uchar pp;
bit close;

sbit didi = P2 ^ 3;
//蜂鸣器发出“滴滴声”
void didiFunction()
{
    TMOD = 0x01; //模式设置，00000001，可见采用的是定时器0，工作与模式1.16位定时/计数器模式（M1=0，M0=1）。
    TR0 = 1;     //打开定时器
    TH0 = 0xff;  //计时器每隔100微秒(0.1ms)发起一次中断。
    TL0 = 0x9c;  //产生频率为1000HZ的声音信号。
    ET0 = 1;     //开定时器0中断
    EA = 1;      //开总中断
    while (1)
    {
        if (pp == 10)
        {
            pp = 0;
            if (!close)
                didi = ~didi; //频率为1KH的音频信号
            aa++;
        }
        if (aa == 200)
        {
            didi = 1;
            close = 1; //方波音频信号持续时间为200*10*100us=0.2s
        }
        if (aa == 300) //滴滴声的周期为300*10*100us=0.3s
        {
            close = 0;
            aa = 0;
        }
    }
}

void time0() interrupt 1
{
    TH0 = 0xff;
    TL0 = 0x9c;
    pp++;
}
