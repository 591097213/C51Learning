#include <reg52.h>
#include "datatype.h"
#include "delay.h"
#include "LCD1602.h"

bit at = 0; //1表示当前处于调整时间状态，0表示处于正常显示状态
uchar code shen[] = {"CLOCK!"};
uchar code word[] = { //对字符进行编码
    //0,1,2,3,4,5,6,7,8,9,:,[space]
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x20};
uchar dispbuf[8], h, m, s, counter, add;

/*******更新缓冲区子程序*******/
void newbuf()
{ //dispbuf分别存储两位时、两位分、两位秒以及两个冒号
    dispbuf[0] = s % 10;
    dispbuf[1] = s / 10;
    dispbuf[3] = m % 10;
    dispbuf[4] = m / 10;
    dispbuf[6] = h % 10;
    dispbuf[7] = h / 10;
}

/*******显示子程序**********/
//输入显示的初始地址
void disp(uchar dispadd)
{ //在LCD中显示hh:mm:ss
    int i;
    uchar tmp;

    LCD1602_Write_Com(dispadd); //设置指针位置

    for (i = 7; i >= 0; i--)
    {
        tmp = dispbuf[i];       //取数值
        tmp = word[tmp];        //取数值对应的编码
        LCD1602_Write_Dat(tmp); //写数据
    }
}

/*********************键盘子程序***********************/
/**
 * @brief 扫描矩阵键盘，返回扫描码
 * 扫描码约定：
 * “1”为有效
 * 高四位从低→高表示按键1~4
 * 低四位从低→高表示第1~4行
 * 所有按键未被按下则返回0
 * @return uchar
 */
uchar keypro(void)
{ //采用矩阵按键。P30~P33行线，P34~P37列线
    uchar scanh, scanl;
    P3 = 0x0f;               //先将所有行线拉高，列线拉低
    if ((P3 & 0x0f) != 0x0f) //如果行线有变化
    {
        Delay_Ms(20);            //延迟一段时间。
        if ((P3 & 0x0f) != 0x0f) //如果此时此刻行线还有变化，说明确实有按键按下
        {
            scanh = 0xfe;               //从第一行开始扫描，将第一行行线拉低
            while ((scanh & 0x10) != 0) //第五次循环时1110 1111 & 0001 0000 =0，结束循环
            {
                P3 = scanh;              //给P3口赋扫描码,每次只拉低一行
                if ((P3 & 0xf0) != 0xf0) //如果判断为真,则说明找到了按键按下的列
                {
                    scanl = (P3 & 0xf0) | 0x0f; //计算识别码
                    return (~scanl) + (~scanh);
                }
                else
                    scanh = (scanh << 1) | 0x01; //否则依次将第二，第三，第四行拉低
            }
        }
    }
    return 0; //没有按键  按下 返回0
}

/********************时间调整子程序********************/
void adjustime()
{
    uchar k;
    k = keypro(); //读取按键
    switch (k)
    {
    case 0x88:   //1000 1000 第四行第一个按键
        if (!at) //如果at=0
        {
            add = 0xc1;                     //0xc1=0x80+0x41 add表示当前光标地址。刚进入调整时间状态时初始化当前光标地址，调整hh
            EA = 0;                         //关全局中断,停止显示时间
            LCD1602_Write_Com(0x80 + 0x41); //写指针地址，0x41为第二行第二个字符
            LCD1602_Write_Com(0x0f);        //开显示，显示光标，光标闪烁
            at = 1;                         //at置1
        }
        else //如果at！=0
        {
            LCD1602_Write_Com(0x80 + 0x40); //写指针地址
            LCD1602_Write_Com(0x0c);        //开显示，不显示光标，光标不闪烁
            at = 0;                         //状态切换
            EA = 1;                         //开全局中断
        }
        break;

    case 0x48:  //0100 1000 第四行第二个按键
        if (at) //如果at=1，即处于调整时间状态
        {
            if (add == 0xc1) //如果光标处于hh处，无法继续左移
            {
                add = 0xc7;             //将光标置于ss处
                LCD1602_Write_Com(add); //写地址
            }
            else //其他情况，光标左移3格
            {
                add = add - 3;
                LCD1602_Write_Com(add);
            }
        }
        break;

    case 0x28: //0010 1000 第四行第三个按键
        if (at)
        {
            if (add == 0xc7) //如果光标处于最右ss处，无法继续右移，则返回最左边
            {
                add = 0xc1;
                LCD1602_Write_Com(add);
            }
            else //光标右移3格
            {
                add = add + 3;
                LCD1602_Write_Com(add);
            }
        }
        break;

    case 0x18: //0001 1000 第四行第四个按键
        if (at)
        {
            switch (add)
            {
            case 0xc1: //调整时
                h++;
                if (h == 24)
                    h = 0;
                break;

            case 0xc4: //调整分
                m++;
                if (m == 60)
                    m = 0;
                break;

            case 0xc7: //调整秒
                s++;
                if (s == 60)
                    s = 0;
                break;
            }

            newbuf();               //刷新显示缓存
            disp(0x80 + 0x40);      //从第二行开始显示
            LCD1602_Write_Com(add); //手动归位光标。避免光标自动移动
        }
        break;
    }
    if (k != 0) //如果有按键被按下
    {
        while ((P3 & 0xf0) != 0xf0) //等待按键释放
            P3 = 0xf0;
    }
}

/*********************初始化子程序**********************/
void init()
{
    TMOD = 0x01;
    TH0 = 0x4c;
    TL0 = 0x00;
    EA = 1;
    ET0 = 1;
    TR0 = 1;
    counter = 0;
    h = 12;
    m = 0;
    s = 0;
    dispbuf[2] = 10; //在word[]中取值为冒号
    dispbuf[5] = 10;
}

/***************************主程序************************/

void clock0(void)
{
    init();
    LCD1602_Init();

    LCD1602_Write_String(0, 0, shen);

    while (1)
    {
        adjustime();
        if (!at) //如果处于正常显示状态
        {
            //使冒号闪烁
            if (counter < 10)
            {
                dispbuf[2] = 10; //在word[]中取值为冒号
                dispbuf[5] = 10;
            }
            else
            {
                dispbuf[2] = 11; //在word[]中取值为空格
                dispbuf[5] = 11;
            }
            //更新显示缓冲区及调用显示程序
            if (counter == 0)
            {
                newbuf();
                disp(0xc0);
            }
            else if (counter == 10)
                disp(0xc0);
        }
    }
}

/*************************定时器0的中断**********************/
void Time0() interrupt 1 using 2 //再次强调中断子程序执行时间越短越好
{
    TH0 = (65536 - 46075) / 256;
    TL0 = (65536 - 46075) % 256;
    counter++;
    if (counter == 20)
    {
        s++;
        counter = 0;
        if (s == 60)
        {
            m++;
            s = 0;
            if (m == 60)
            {
                h++;
                m = 0;
                if (h == 24)
                    h = 0;
            }
        }
    }
}
