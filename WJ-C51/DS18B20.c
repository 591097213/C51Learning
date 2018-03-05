/**
 * @brief DS18B20芯片相关
 * 
 * [注意]DS18B20芯片单总线的延时要求较高，不可随意更改本文件中的时延
 * 
 * @file DS18B20.c
 * @author your name
 * @date 2018-03-01
 */

#include <reg52.H>
#include <math.h>
#include "datatype.h"
#include "delay.h"
#include "qxmcs51_config.h"

bit presence;                                                             //芯片存在标志
uchar data RomCode[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; //用于存储64位芯片序列码28 FF 6F 8F A0 16 04 A4
uchar crc;                                                                //存放校验和

/**
 * @brief 初始化
 * 
 * @return bit 0表示初始化成功，1表示失败
 */
bit DS18B20_Init() //初始化
{
    DS = 1;   //DS复位
    Delay(8); //稍做延时

    DS = 0;    //将DQ拉低
    Delay(90); //精确延时 大于 480us

    DS = 1; //拉高总线
    Delay(8);

    presence = DS; //读取存在信号
    Delay(100);

    DS = 1;

    return (presence); //返回信号，0=presence,1= no presence
}

/**
 * @brief 读取一位
 * 
 * @return bit 
 */
bit DS18B20_ReadBit()
{
    uchar i;
    DS = 0; //将DS 拉低开始读时间隙
    DS = 1; // then return high
    for (i = 0; i < 3; i++)
        ;
    return (DS); // 返回 DS 线上的电平值
}

/**
 * @brief 读取一字节
 * 
 * @return uchar 
 */
uchar DS18B20_ReadByte()
{
    uchar i = 0;
    uchar dat = 0;
    for (i = 0; i < 8; i++)
    { // 读取字节，每次读取一个字节
        if (DS18B20_ReadBit())
            dat |= 0x01 << i; // 然后将其左移

        Delay(4);
    }

    return (dat);
}

/**
 * @brief 读取一位
 * 
 * @param bitval 
 */
void DS18B20_WriteBit(char bitval)
{
    DS = 0; // 将DQ 拉低开始写时间隙
    if (bitval == 1)
        DS = 1; // 如果写1，DQ 返回高电平
    Delay(5);   // 在时间隙内保持电平值，
    DS = 1;     // Delay函数每次循环延时16μs，因此delay(5) = 104μs
}

void DS18B20_WriteByte(uchar dat)
{
    uchar i = 0;
    uchar temp;

    for (i = 0; i < 8; i++) // 写入字节, 每次写入一位
    {
        temp = dat >> i;
        temp &= 0x01;
        DS18B20_WriteBit(temp);
    }
    Delay(5);
}

/**
 * @brief 发送温度转换指令
 * 0xCC是跳过ROM的指令
 * 此时温度值暂存至内部RAM中，并未读出
 */
void DS18B20_SendChangeCmd()
{
    DS18B20_Init();
    Delay_Ms(1);
    DS18B20_WriteByte(0xcc);
    DS18B20_WriteByte(0x44);
}

/**
 * @brief 发送读RAM指令
 * 读出转化后的温度值
 * 
 */
void DS18B20_SendReadCmd()
{
    DS18B20_Init();
    Delay_Ms(1);
    DS18B20_WriteByte(0xcc);
    DS18B20_WriteByte(0xbe);
}

/**
 * @brief 获得温度值
 * 
 * @return float 
 */
float DS18B20_GetTmpValue()
{
    uint tmpvalue;
    int value;
    float t;
    uchar low, high;
    DS18B20_SendReadCmd();

    low = DS18B20_ReadByte();
    high = DS18B20_ReadByte(); //读取两字节的温度值

    tmpvalue = high;
    tmpvalue <<= 8;
    tmpvalue |= low; //将两字节组合起来
    value = tmpvalue;

    t = value * 0.0625; //转化为摄氏度.t是浮点数类型

    return t;
}

/**
 * @brief 读芯片64位序列码
 * 
 */
void DS18B20_ReadRomCord()
{
    uchar j;
    DS18B20_Init();
    DS18B20_WriteByte(0x33); // 读序列码的操作
    for (j = 0; j < 8; j++)
    {
        RomCode[j] = DS18B20_ReadByte();
    }
}

/**
 * @brief 将8字节序列码转化为16个16进制字符
 * 
 */
void DS18B20_RomChar(char *RomChar)
{

    uchar i, temp, j;

    DS18B20_ReadRomCord(); //读取芯片序列号

    for (i = 0; i < 8; i++)
    {
        temp = ((RomCode[i] & 0xf0) >> 4); //取每一字节的高4位
        if (temp > 9)
        {
            RomChar[j] = temp + 0x37;
        }
        else
        {
            RomChar[j] = temp + 0x30;
        }
        j++;

        temp = (RomCode[i] & 0x0f); //取每一字节的低4位
        if (temp > 9)
        {
            RomChar[j] = temp + 0x37;
        }
        else
        {
            RomChar[j] = temp + 0x30;
        }
        j++;
    }
    RomChar[j] = '\0';
}

/**
 * @brief 将DS18B20_GetTmpValue()获取的数据转化成字符串储存在str中
 * 
 * 转化过程中已考虑了正负号、小数点、前导0等
 * 
 * @param t 需要转化的数据
 * @param str 转化后的字符串。需要至少8字节空间（5位数字+1位小数点+1位符号+'\0'）
 */
void DS18B20_temperToStr(float t, char *str)
{
    uchar count, j;
    uint tmp;
    uchar datas[] = {0, 0, 0, 0, 0}; //读取到的值连同小数最多5位
    int v;

    //t共有4位小数
    //因为最终返回的是int类型的值，*100然后再通过类型转化，删去末两位小数，以保留两位小数。调用该函数获得的返回值记得/100。

    v = t * 100 + (t > 0 ? 0.5 : -0.5); //大于0加0.5, 小于0减0.5。*100用于控制精度。加减0.5是四舍五入。

    tmp = abs(v); //取绝对值
    datas[0] = tmp / 10000;
    datas[1] = tmp % 10000 / 1000;
    datas[2] = tmp % 1000 / 100;
    datas[3] = tmp % 100 / 10;
    datas[4] = tmp % 10;

    j = 0;

    if (v < 0) //写入符号
    {
        str[j++] = '-';
    }
    else
    {
        str[j++] = '+';
    }

    if (datas[0] != 0) //处理前置0
    {
        str[j++] = '0' + datas[0];
    }
    for (count = 1; count != 5; count++)
    {
        str[j++] = '0' + datas[count];
        if (count == 2)
        {
            str[j++] = '.'; //小数点
        }
    }
    str[j] = '\0';
}

/**
 * @brief 直接获取温度字符串，方便输出、调用
 * 
 * @param str 存储字符串的指针。至少需要8字节
 */
void DS18B20_GetTmpStr(char *str)
{
    DS18B20_temperToStr(DS18B20_GetTmpValue(), str);
}

/**
 * @brief DS18B20的CRC8校验程序
 * 既可作为校验和生成程序，也可作为校验和校验程序
 * 
 * @return uchar 
 */
uchar DS18B20_CRC8()
{
    uchar i, x;
    uchar crcbuff;

    crc = 0;
    for (x = 0; x < 8; x++)
    {
        crcbuff = RomCode[x];
        for (i = 0; i < 8; i++)
        {
            if (((crc ^ crcbuff) & 0x01) == 0)
                crc >>= 1;
            else
            {
                crc ^= 0x18; //CRC=X8+X5+X4+1
                crc >>= 1;
                crc |= 0x80;
            }
            crcbuff >>= 1;
        }
    }
    return crc;
}