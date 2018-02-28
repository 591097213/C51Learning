/*读取DS18B20的64位序列码并显示在1602液晶上，如果读取正确结果，则在
液晶第一行显示DS18B20 OK，第二行显示序列码，蜂鸣器鸣一次，如果读取失败，则在液晶上
显示DS18B20  ERR0R PLEASE CHECK，蜂鸣器连续鸣叫 ，用户可通过更改18B20接口自己外接。 */
#include <reg52.h>
#include <intrins.h>
#include "datatype.h"
#include "beepOne.h"
#include "LCD1602.h"
#include "delay.h"

sbit DQ = P2 ^ 2; //定义DS18B20端口DQ

bit presence; //DS18B20存在信号

uchar code cdis1[] = {"   DS18B20 OK   "};
uchar code cdis2[] = {"                "};
uchar code cdis3[] = {" DS18B20  ERR0R "};
uchar code cdis4[] = {"  PLEASE CHECK  "};

unsigned char data display[2] = {0x00, 0x00};

unsigned char data RomCode[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; //用于存储序列码

unsigned char Temp; //用于校验
unsigned char crc;  //用于校验

#define delayNOP() \
    ;              \
    {              \
        _nop_();   \
        _nop_();   \
        _nop_();   \
        _nop_();   \
    };

/*******************************************************************/
void delay1(int ms)
{
    unsigned char y;
    while (ms--)
    {
        for (y = 0; y < 250; y++)
        {
            _nop_();
            _nop_();
            _nop_();
            _nop_();
        }
    }
}

/*******************************************************************/
/*                                                                 */
/*us级延时函数                                                     */
/*                                                                 */
/*******************************************************************/

void Delay(unsigned int num)
{
    while (--num)
        ;
}

/*******************************************************************/
/*                                                                 */
/*初始化ds1820                                                     */
/*                                                                 */
/*******************************************************************/
Init_DS18B20(void)
{
    DQ = 1;      //DQ复位
    Delay5us(8); //稍做延时

    DQ = 0;       //将DQ拉低
    Delay5us(90); //精确延时 大于 480us

    DQ = 1; //拉高总线
    Delay5us(8);

    presence = DQ; //读取存在信号
    Delay5us(100);
    DQ = 1;

    return (presence); //返回信号，0=presence,1= no presence
}

/*******************************************************************/
/*                                                                 */
/* 读一位（bit）                                                   */
/*                                                                 */
/*******************************************************************/
uchar read_bit(void)
{
    unsigned char i;
    DQ = 0; //将DQ 拉低开始读时间隙
    DQ = 1; // then return high
    for (i = 0; i < 3; i++)
        ;        // 延时15μs
    return (DQ); // 返回 DQ 线上的电平值
}

/*******************************************************************/
/*                                                                 */
/* 读一个字节                                                      */
/*                                                                 */
/*******************************************************************/
ReadOneChar(void)
{
    unsigned char i = 0;
    unsigned char dat = 0;
    for (i = 0; i < 8; i++)
    { // 读取字节，每次读取一位
        if (read_bit())
            dat |= 0x01 << i; // 然后将其左移
        Delay5us(4);
    }

    return (dat);
}

/*******************************************************************/
/*                                                                 */
/* 写一位                                                          */
/*                                                                 */
/*******************************************************************/
void write_bit(char bitval)
{
    DQ = 0; // 将DQ 拉低开始写时间隙
    if (bitval == 1)
        DQ = 1;  // 如果写1，DQ 返回高电平
    Delay5us(5); // 在时间隙内保持电平值，
    DQ = 1;      // Delay函数每次循环延时16μs，因此delay(5) = 104μs
}

/*******************************************************************/
/*                                                                 */
/* 写一个字节                                                      */
/*                                                                 */
/*******************************************************************/
WriteOneChar(unsigned char dat)
{
    unsigned char i = 0;
    unsigned char temp;
    for (i = 0; i < 8; i++) // 写入字节, 每次写入一位
    {
        temp = dat >> i;
        temp &= 0x01;
        write_bit(temp);
    }
    Delay5us(5);
}

/*******************************************************************/
/*                                                                 */
/* 读取64位序列码                                                  */
/*                                                                 */
/*******************************************************************/
Read_RomCord(void)
{
    unsigned char j;
    Init_DS18B20();

    WriteOneChar(0x33); // 读序列码的操作
    for (j = 0; j < 8; j++)
    {
        RomCode[j] = ReadOneChar(); //储存序列码
    }
}

/*******************************************************************/
/*                                                                 */
/*DS18B20的CRC8校验程序                                            */
/*                                                                 */
/*******************************************************************/
uchar CRC8()
{
    uchar i, x;
    uchar crcbuff;

    crc = 0;
    for (x = 0; x < 8; x++)
    {
        crcbuff = RomCode[x];   //取出8字节中的每一字节
        for (i = 0; i < 8; i++) //取出该字节的每一位
        {
            if (((crc ^ crcbuff) & 0x01) == 0) //如果最低位为0
                crc >>= 1;                     //crc右移1
            else                               //如果该字节最低位为1
            {
                crc ^= 0x18; //CRC=X8+X5+X4+1.0x18=0001 1000
                crc >>= 1;   //crc右移1
                crc |= 0x80; //crc最高位置1
            }
            crcbuff >>= 1; //crcbuff右移，处理下一位
        }
    }
    return crc;
}

/*******************************************************************/
/*                                                                 */
/* 数据转换与显示                                                  */
/*                                                                 */
/*******************************************************************/

Disp_RomCode()
{
    uchar j;
    uchar H_num = 0x40; //LCD第二行初始位置

    for (j = 0; j < 8; j++)
    {
        Temp = RomCode[j];

        display[0] = ((Temp & 0xf0) >> 4);
        if (display[0] > 9)
        {
            display[0] = display[0] + 0x37;
        }
        else
        {
            display[0] = display[0] + 0x30;
        }

        lcd_pos(H_num);
        lcd_wdat(display[0]); //高位数显示

        H_num++;
        display[1] = (Temp & 0x0f);
        if (display[1] > 9)
        {
            display[1] = display[1] + 0x37;
        }
        else
        {
            display[1] = display[1] + 0x30;
        }

        lcd_pos(H_num);
        lcd_wdat(display[1]); //低位数显示
        H_num++;
    }
}

/*******************************************************************/
/*                                                                 */
/* DS18B20 OK 显示菜单                                             */
/*                                                                 */
/*******************************************************************/
void Ok_Menu()
{
    uchar m;
    lcd_init(); //初始化LCD

    lcd_pos(0); //设置显示位置为第一行的第1个字符
    m = 0;
    while (cdis1[m] != '\0')
    { //显示字符
        lcd_wdat(cdis1[m]);
        m++;
    }

    lcd_pos(0x40); //设置显示位置为第二行第1个字符
    m = 0;
    while (cdis2[m] != '\0')
    {
        lcd_wdat(cdis2[m]); //显示字符
        m++;
    }
}

/*******************************************************************/
/*                                                                 */
/* DS18B20 ERROR 显示菜单                                          */
/*                                                                 */
/*******************************************************************/
void Error_Menu()
{
    uchar m;
    lcd_init(); //初始化LCD

    lcd_pos(0); //设置显示位置为第一行的第1个字符
    m = 0;
    while (cdis3[m] != '\0')
    { //显示字符
        lcd_wdat(cdis3[m]);
        m++;
    }

    lcd_pos(0x40); //设置显示位置为第二行第1个字符
    m = 0;
    while (cdis4[m] != '\0')
    {
        lcd_wdat(cdis4[m]); //显示字符
        m++;
    }
}

/*******************************************************************/
/*                                                                 */
/* 主函数                                                          */
/*                                                                 */
/*******************************************************************/
void main()
{
    P0 = 0xff;
    P2 = 0xff;

    while (1)
    {
        Ok_Menu();
        Read_RomCord(); //读取64位序列码
        CRC8();         //CRC效验
        if (crc == 0)   //CRC效验正确
        {
            Disp_RomCode(); //显示64位序列码
            beepOne();
        }
        while (!presence)
        {
            Init_DS18B20();
            delay1(1000);
        }

        Error_Menu();
        do
        {
            Init_DS18B20();
            beepOne();
        } while (presence);
    }
}

/*******************************************************************/