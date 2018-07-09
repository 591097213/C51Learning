/**
 * @brief 红外线接收MP3遥控器键值并显示在lcd1602上
 *
 */
#include <reg52.h>
#include "infrared.h"
#include "LCD1602.h"

extern bit irpro_ok;
extern bit irok; //标志变量

void infrared1602Example(void)
{
    EX0init();  //初始化外部中断
    TIM0init(); //初始化定时器

    LCD1602_Init();  //初始化液晶
    LCD1602_Clear(); //清屏

    LCD1602_Write_String(0, 0, "****************");
    LCD1602_Write_String(0, 1, "Code:");

    while (1) //主循环
    {
        if (irok) //如果接收好了进行红外处理
        {
            Ircordpro();
            irok = 0;
        }

        if (irpro_ok) //如果处理好后进行工作处理，如按对应的按键后显示对应的数字等
        {
            Ir_work();
        }
    }
}
