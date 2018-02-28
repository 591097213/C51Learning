/************************************************************************
* 描述：（用串口调试助手软件观察）                                      
*   主机发送0123456789abcdef给单片机，单片机接收到数据后发送JW-MCU789abcdef给主机。  	
*   LCD1602上显示	   0123456789abcdef 
*   LCD1602显示接收数据的ASCII码。  波特率9600    发送和接收都要选择为文本格式      		        
*   注：当单片机收到数据只有凑够16个时才会一次在液晶上显示出来		    
************************************************************************/

#include <reg52.h>
#include <intrins.h>
#include "datatype.h"
#include "LCD1602.h"
#include "delay.h"

//0x20即空格。RXDdata存储LCD待显示的字符
uchar data RXDdata[] = {0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                        0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20};

uchar temp, buf, m, count;

bit playflag = 0;

uchar code cdis1[] = {" SERILA TRANFER "};
uchar code cdis2[] = {"                "};

/*********************************************************

  发送数据函数

*********************************************************/
void senddata(uchar dat)
{
    SBUF = dat;
    while (!TI)
        ;
    TI = 0;
}

/*********************************************************

  串行中断服务函数

*********************************************************/
void serial() interrupt 4
{
    ES = 0;     //关闭串行中断
    RI = 0;     //清除串行接受标志位
    buf = SBUF; //从串口缓冲区取得数据

    switch (buf)
    {
    case 0x31:
        senddata('J');
        break; //接受到1，发送字符'J'给计算机

    case 0x32:
        senddata('W');
        break; //接受到2，发送字符'W'给计算机

    case 0x33:
        senddata('-');
        break; //接受到3，发送字符'-'给计算机

    case 0x34:
        senddata('M');
        break; //接受到4，发送字符'M'给计算机

    case 0x35:
        senddata('C');
        break; //接受到5，发送字符'C'给计算机

    case 0x36:
        senddata('U');
        break; //接受到6，发送字符'U'给计算机

    default:
        senddata(buf);
        break; //接受到其它数据，将其发送给计算机
    }

    if (buf != 0x0D) //0x0D=CR
    {
        if (buf != 0x0A) //0x0A=LF
        {
            temp = buf;
            if (count < 16)
            {
                RXDdata[count] = temp; //将接受到的字符存入数组中
                count++;
                if (count == 16)
                    playflag = 1; //接收16个字符后置可显示标志为1
            }
        }
    }
    ES = 1; //允许串口中断
}

/*********************************************************

  数据显示函数

*********************************************************/
void play()
{
    if (playflag)
    {
        LCD1602_Write_String(0, 1, cdis2); //清LCD1602第二行

        LCD1602_Write_String(0, 1, RXDdata); //显示字符

        playflag = 0;
        count = 0x00;

        for (m = 0; m < 16; m++)
        {
            RXDdata[m] = 0x20; //清显存单元
            Delay_Ms(5);
        }
    }
}

/*********************************************************

  主函数

*********************************************************/
void main(void)
{
    P0 = 0xff;
    P2 = 0xff;

    SCON = 0x50; //设定串口工作方式
    PCON = 0x00; //波特率不倍增

    TMOD = 0x20; //定时器1工作于8位自动重载模式, 用于产生波特率
    EA = 1;
    ES = 1; //允许串口中断
    TL1 = 0xfd;
    TH1 = 0xfd; //波特率9600
    TR1 = 1;

    LCD1602_Init();

    LCD1602_Write_String(0, 0, cdis1);

    LCD1602_Write_String(0, 1, cdis2);

    while (1)
    {
        play();
    }
}

/*********************************************************/
