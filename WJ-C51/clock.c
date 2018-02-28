#include <reg52.h>
#include "datatype.h"
#include "delay.h"
#include "LCD1602.h"

//button
sbit s1 = P3 ^ 0;
sbit s2 = P3 ^ 1;
sbit s3 = P3 ^ 2;
uchar count, s1num;
char miao, shi, fen;

uchar code table[] = "  2016-11-02 MON";
uchar code table1[] = "    00:00:00";

void clockInit()
{

    LCD1602_Write_String(0, 0, table);

    LCD1602_Write_String(0, 1, table1);

    TMOD = 0x01; //T0.16位计时器/计数器
    TH0 = (65536 - 50000) / 256;
    TL0 = (65536 - 50000) % 256;
    EA = 1;
    ET0 = 1;
    TR0 = 1;
}

void write_sfm(uchar add, uchar date) //写入时间
{
    uchar shi, ge;
    shi = date / 10;
    ge = date % 10;
    LCD1602_Write_Com(0x80 + 0x40 + add); //第二行第add个字符
    LCD1602_Write_Dat(0x30 + shi);        //个位
    LCD1602_Write_Dat(0x30 + ge);         //十位
}

void keyscan()
{
    //s1用于选择要调整的时间位数
    if (s1 == 0)
    {
        Delay_Ms(5);
        if (s1 == 0)
        {
            s1num++;
            while (!s1)
                ;
        }
        switch (s1num)
        {
        case 1:
            TR0 = 0;
            LCD1602_Write_Com(0x80 + 0x40 + 11); //光标位置第二行第十个字符
            LCD1602_Write_Com(0x0f);             //打开光标
            break;

        case 2:
            LCD1602_Write_Com(0x80 + 0x40 + 8); //光标位置第二行第7个字符
            break;

        case 3:
            LCD1602_Write_Com(0x80 + 0x40 + 5); //光标位置第二行第4个字符
            break;

        case 4:
            s1num = 0;
            LCD1602_Write_Com(0x0c); //消除光标
            TR0 = 1;
            break;
        }
    }
    if (s1num != 0) //如果处于调整时间
    {
        if (s2 == 0)
        {
            Delay_Ms(20);
            if (s2 == 0) //消抖
            {
                while (!s2)
                    ;
                if (s1num == 1)
                {
                    miao++; //加一秒
                    if (miao == 60)
                        miao = 0;
                    write_sfm(10, miao);
                    LCD1602_Write_Com(0x80 + 0x40 + 11); //重新调整光标位置。否则光标会前进一格
                }
                if (s1num == 2)
                {
                    fen++; //加一分钟
                    if (fen == 60)
                        fen = 0;
                    write_sfm(7, fen);
                    LCD1602_Write_Com(0x80 + 0x40 + 8); //光标位置
                }
                if (s1num == 3)
                {
                    shi++; //加一小时
                    if (shi == 24)
                        shi = 0;
                    write_sfm(4, shi);
                    LCD1602_Write_Com(0x80 + 0x40 + 5); //光标位置
                }
            }
        }
        if (s3 == 0)
        {
            Delay_Ms(20); //消抖
            if (s3 == 0)
            {
                while (!s3)
                    ;
                if (s1num == 1)
                {
                    miao--; //减一秒
                    if (miao == -1)
                        miao = 59;
                    write_sfm(10, miao);
                    LCD1602_Write_Com(0x80 + 0x40 + 11); //光标位置
                }
                if (s1num == 2)
                {
                    fen--; //减一分钟
                    if (fen == -1)
                        fen = 59;
                    write_sfm(7, fen);
                    LCD1602_Write_Com(0x80 + 0x40 + 8); //光标位置
                }
                if (s1num == 3)
                {
                    shi--;
                    if (shi == -1)
                        shi = 23;
                    write_sfm(4, shi);
                    LCD1602_Write_Com(0x80 + 0x40 + 5); //光标位置
                }
            }
        }
    }
}
void clock()
{
    LCD1602_Init();
    clockInit();
    while (1)
    {
        keyscan();
    }
}

void timer0() interrupt 1
{
    TH0 = (65536 - 50000) / 256;
    TL0 = (65536 - 50000) % 256;
    count++;
    if (count == 18)
    {
        count = 0;
        miao++;
        if (miao == 60)
        {
            miao = 0;
            fen++;
            if (fen == 60)
            {
                fen = 0;
                shi++;
                if (shi == 24)
                {
                    shi = 0;
                }
                write_sfm(4, shi);
            }
            write_sfm(7, fen);
        }
        write_sfm(10, miao);
    }
}
