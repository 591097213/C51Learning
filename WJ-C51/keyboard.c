/**
 * @brief 矩阵键盘扫描程序
 *
 * @file keyboard.c
 * @author your name
 * @date 2018-03-13
 */

#include <reg52.h>
#include "delay.h"
#include "qxmcs51_config.h"
#include "datatype.h"

/**
 * @brief 扫描矩阵键盘，返回扫描码
 * 扫描码约定：
 * “0”为有效
 * 高四位从低→高表示按键1~4列
 * 低四位从低→高表示第1~4行
 * 所有按键未按下则返回0xff
 * @return uchar
 */
uchar KeyScan(void)          //键盘扫描函数，使用行列反转扫描法
{                            //采用矩阵按键。P30~P33行线，P34~P37列线
    uchar cord_h, cord_l;    //行列值中间变量
    KeyPort = 0x0f;          //列线输出全为0
    cord_h = KeyPort & 0x0f; //读入行线值
    if (cord_h != 0x0f)      //先检测有无按键按下
    {
        Delay_Ms(10); //去抖
        if ((KeyPort & 0x0f) != 0x0f)
        {
            cord_h = KeyPort & 0x0f; //读入行线值
            KeyPort = cord_h | 0xf0; //输出当前行线值
            cord_l = KeyPort & 0xf0; //读入列线值

            while ((KeyPort & 0xf0) != 0xf0)
                ; //等待松开并输出

            return (cord_h + cord_l); //键盘最后组合码值
        }
    }
    return (0xff); //返回该值
}