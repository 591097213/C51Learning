/*-----------------------------------------------
  名称：液晶显示计算器
  内容：由于单片机浮点数限制，可以计算6位数字运算，大于这个长度，用科学计数法表示
        小数位精度后6位，大于6位按四舍五入
------------------------------------------------*/
#include <reg52.h> //包含头文件，一般情况不需要改动，头文件包含特殊功能寄存器的定义
#include <stdio.h>
#include "LCD1602.h"
#include "delay.h"
#include "keyboard.h"
#include "datatype.h"

/*------------------------------------------------
          按键值处理函数，返回扫键值
           可以根据需要改变返回值

  			| 1 | 2 | 3 | + |
  			| 4 | 5 | 6 | - |
  			| 7 | 8 | 9 | * |
  			| 0 | . | = | / |
------------------------------------------------*/
unsigned char KeyPro(void)
{
    switch (KeyScan())
    {
    case 0x77:
        return '1';
        break; //3
    case 0xb7:
        return '2';
        break; //7
    case 0xd7:
        return '3';
        break; //b
    case 0x7b:
        return '4';
        break; //2
    case 0xbb:
        return '5';
        break; //6
    case 0xdb:
        return '6';
        break; //a
    case 0x7d:
        return '7';
        break; //1
    case 0xbd:
        return '8';
        break; //5
    case 0xdd:
        return '9';
        break; //9
    case 0x7e:
        return '0';
        break; //0
    case 0xbe:
        return '.';
        break; //4
    case 0xde:
        return '=';
        break; //8
    case 0xe7:
        return '+';
        break; //f
    case 0xeb:
        return '-';
        break; //e
    case 0xed:
        return 'x';
        break; //d
    case 0xee:
        return '/';
        break; //c
    default:
        return 0xff;
        break;
    }
}

/*------------------------------------------------
                    主程序
------------------------------------------------*/
void calculator()
{
    uchar num, i, sign;
    uchar temp[16]; //最大输入16个
    bit firstflag;
    float a = 0, b = 0;
    uchar s;

    LCD1602_Init();  //初始化液晶屏
    LCD1602_Clear(); //清屏

    LCD1602_Write_String(0, 0, " LCD calculator"); //写入第一行信息，主循环中不再更改此信息，所以在while之前写入
    LCD1602_Write_String(0, 1, " Fun: + - x / ");  //写入第二行信息，提示输入密码
    while (1)                                      //主循环
    {

        num = KeyPro();  //扫描键盘
        if (num != 0xff) //如果扫描是按键有效值则进行处理
        {
            if (i == 0) //输入是第一个字符的时候需要把改行清空，方便观看
                LCD1602_Clear();

            if (('+' == num) || (i == 16) || ('-' == num) || ('x' == num) || ('/' == num) || ('=' == num)) //输入数字最大值16，输入符号表示输入结束
            {
                i = 0; //计数器复位

                if (firstflag == 0) //如果是输入的第一个数据，赋值给a，并把标志位置1，到下一个数据输入时可以跳转赋值给b
                {
                    sscanf(temp, "%f", &a); //将temp中的数据按浮点数格式赋值给a
                    firstflag = 1;
                }
                else //如果输入的是第二个数据
                    sscanf(temp, "%f", &b);
                for (s = 0; s < 16; s++) //赋值完成后把缓冲区清零，防止下次输入影响结果
                    temp[s] = 0;
                LCD1602_Write_Char(0, 1, num); //在第二行第一个字符处写字符
                ///////////////////////
                if (num != '=') //判断当前符号位并做相应处理
                    sign = num; //如果不是等号记下标志位
                else
                {
                    firstflag = 0; //检测到输入=号，判断上次读入的符合
                    switch (sign)
                    {
                    case '+':
                        a = a + b;
                        break;
                    case '-':
                        a = a - b;
                        break;
                    case 'x':
                        a = a * b;
                        break;
                    case '/':
                        a = a / b;
                        break;
                    default:
                        break;
                    }
                    sprintf(temp, "%g", a);           //输出浮点型，无用的0不输出
                    LCD1602_Write_String(1, 1, temp); //显示到液晶屏
                    sign = 0;
                    a = b = 0; //用完后所有数据清零
                    for (s = 0; s < 16; s++)
                        temp[s] = 0;
                }
            }

            else if (i < 16) //输入的数据小于16位且非运算符
            {
                if ((1 == i) && (temp[0] == '0')) //如果第一个字符是0，判读第二个字符
                {
                    if (num == '.') //如果是小数点则正常输入，光标位置加1
                    {
                        temp[1] = '.';
                        LCD1602_Write_Char(1, 0, num); //在第一行第二个位置输出小数点
                        i++;
                    } //这里没有判断连续按小数点，如0.0.0
                    else
                    {
                        temp[0] = num;                 //如果是1-9数字，说明0没有用，则直接替换第一位0
                        LCD1602_Write_Char(0, 0, num); //输出数据
                    }
                }
                else
                {
                    temp[i] = num;
                    LCD1602_Write_Char(i, 0, num); //输出数据
                    i++;                           //输入数值累加
                }
            }
        }
    }
}
