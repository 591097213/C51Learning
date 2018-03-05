/**
 * @brief 流水灯控制示例
 * 
 * 此程序编译后超过2k字节，keil需要破解才能编译
 * 
 */
#include "flowLamp.h"
#include "delay.h"

void flowLampExample()
{
    P0 = 0Xff;
    EA = 1;   //打开中断,使用中断方便调试查看模式
    EX0 = 1;  //允许外部中断0中断
    EX1 = 1;  //允许外部中断1中断
    PX0 = 1;  // 外部中断0位高优先级
    while (1) //无限循环
    {
        rdlszy(); //倒流水左移
        rja();    //渐暗
        P1 = 0xff;
        Delay_Ms(300); //延时0.3秒
        //rdlsyy(); //	倒流水右移
        //F=0xff;
        //Delay_Ms(300);
        rzmyy(); //走马右移
        P1 = 0xff;
        Delay_Ms(300);
        rzmzy(); //走马左移
        P1 = 0xff;
        Delay_Ms(300);
        rjl(); //渐亮
        rja(); //渐暗
        P1 = 0xff;
        Delay_Ms(300);
        rzmnwyd(); //走马内外移动
        P1 = 0xff;
        Delay_Ms(300);
        rlszy(); //流水左移
        rja();   //渐暗
        Delay_Ms(300);
        rlsyy(); //流水右移
        rja();   //渐暗
        Delay_Ms(300);
        rsdzmyy(); //3灯走马右移
        P1 = 0xff;
        Delay_Ms(300);
        rsdzmzy(); //3灯走马右移
        P1 = 0xff;
        Delay_Ms(300);
        rlsnwyd(); //流水内外移动
        P1 = 0xff;
        Delay_Ms(300);
        rtbyd(); //跳变7种模式
        P1 = 0xff;
        Delay_Ms(300);
    }
}