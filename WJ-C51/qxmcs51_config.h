#ifndef __QXMCS51_CONFIG_H
#define __QXMCS51_CONFIG_H

#include <reg52.h>
#include <intrins.h>
#include "datatype.h"

sbit P_IR_RX = P3 ^ 2; //定义红外接收引脚
sbit LCD_EN = P3 ^ 4;  //LCD使能IO
sbit LCD_RS = P3 ^ 5;  //LCD写数据或命令控制IO
sbit LCD_RW = P3 ^ 6;  //LCD读写控制IO
sbit P_IR_TX = P2 ^ 4; //定义红外发射引脚
sbit BEEP = P2 ^ 3;    //蜂鸣器IO口定义
sbit SDA = P2 ^ 0;     //IIC数据总线
sbit SCL = P2 ^ 1;     //IIC时钟总线
sbit LED_WE = P2 ^ 6;
sbit LED_DU = P2 ^ 7;

sbit RCK = P2 ^ 5;   //移位寄存器输出存储器锁存时钟线
sbit SRCLK = P2 ^ 6; //移位寄存器数据输入时钟线
sbit SER = P2 ^ 7;   //移位寄存器数据线

sbit BUTTON0 = P3 ^ 0; //独立按键0
sbit BUTTON1 = P3 ^ 1; //独立按键1

/*********************************************************/

//#define MAIN_Fosc		24000000L	//定义主时钟
//#define MAIN_Fosc		22118400L	//定义主时钟
//#define MAIN_Fosc		12000000L	//定义主时钟
#define MAIN_Fosc 11059200L //定义主时钟
//#define MAIN_Fosc		 5529600L	//定义主时钟

/*********************************************************/
#define AT24C02_ADDR 0xa0 //AT24C02地址
#define PCF8591_ADDR 0x90 //PCF8591地址
#define botelv 9600UL     //波特率定义为9600

#endif