/**
 * @brief 1602使用示例
 *
 * @file LCD1602Example.c
 * @author Chuckie Chen
 * @date 2018-03-13
 */

#include <reg52.h>
#include "datatype.h"
#include "LCD1602.h"
#include "delay.h"

uchar table1[] = " Hello  World ! ";
uchar table2[] = "****************";

uchar code dis1[] = {"   WLCOME  TO   "};
uchar code dis2[] = {"ABCDEFGHIJKLMNOP"};
uchar code dis3[] = {"   OT  EMOCLW   "};
uchar code dis4[] = {"PONMLKJIHGFEDCBA"};

/**
 * @brief 渐入显示
 *
 */
void jianru()
{
    uchar a;

    LCD1602_Init();

    LCD1602_Write_String(16, 0, table1); //将第一个字符写在向右偏移17个字符处，为后面的由右向左划入做准备。

    LCD1602_Write_String(16, 1, table2);

    for (a = 0; a < 16; a++)
    {
        LCD1602_Write_Com(0x18); //左移
        Delay_Ms(200);
    }

    while (1)
        ;
}

/**
 * @brief 滚动显示
 *
 */
void gundong()
{
    uchar i;

    LCD1602_Init();

    while (1)
    {
        LCD1602_Write_Com(0x06); //当读或写1个字符后地址指针加1,且光标加1,整屏显示不移动
        LCD1602_Pos(0);          //设置显示位置为第一行的第1个字符
        i = 0;

        while (dis1[i] != '\0')
        { //显示字符"   Welcome  TO   "
            LCD1602_Write_Dat(dis1[i]);
            i++;
            Delay_Ms(30); //控制两字之间显示速度
        }

        LCD1602_Pos(0x40); //设置显示位置为第二行第1个字符
        i = 0;

        while (dis2[i] != '\0')
        {
            LCD1602_Write_Dat(dis2[i]); //显示字符" WWW.CNJWDZ.COM "
            i++;
            Delay_Ms(30); //控制两字之间显示速度
        }

        LCD1602_Flash();         //闪动二次
        LCD1602_Write_Com(0x01); //清除LCD的显示内容
        Delay_Ms(200);           //控制转换时间
        LCD1602_Write_Com(0x04); //当读或写1个字符后地址指针减1,且光标减1,整屏显示不移动
        LCD1602_Pos(15);         //设置显示位置为第一行的第16个字符
        i = 0;

        while (dis3[i] != '\0')
        { //显示字符"   Welcome  TO   "
            LCD1602_Write_Dat(dis3[i]);
            i++;
            Delay_Ms(30); //控制两字之间显示速度
        }

        LCD1602_Pos(0x4F); //设置显示位置为第二行的第16个字符
        i = 0;

        while (dis4[i] != '\0')
        {
            LCD1602_Write_Dat(dis4[i]); //显示字符" WWW.CNJWDZ.COM "
            i++;
            Delay_Ms(30); //控制两字之间显示速度
        }

        LCD1602_Flash();         //闪动二次
        LCD1602_Write_Com(0x01); //清除LCD的显示内容
        Delay_Ms(200);           //控制转换时间
    }
}
