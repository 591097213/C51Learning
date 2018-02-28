
//AT24C02EEPROM芯片程序
#include <reg52.h>
#include <intrins.h>
#include "datatype.h"
#include "delay.h"
#include "IIC.h"
#include "qxmcs51_config.h"

/*从address中读一个数据*/
BOOL I2C_AT24C02_ReadData(uchar ADDR, uchar *AT24C02_Value)
{
    I2C_Start();                     //启动总线
    I2C_send_byte(AT24C02_ADDR + 0); //发送设备地址
    if (!Test_ACK())                 //测试从设备，即应答失败
    {
        return (0); //返回0
    }
    I2C_send_byte(ADDR); //应答成功，则发送控制字
    if (!Test_ACK())     //测试从设备，即应答失败
    {
        return (0); //返回0
    }

    I2C_Start();                     //启动总线
    I2C_send_byte(AT24C02_ADDR + 1); //发送at24c02地址+1表示读取
    if (!Test_ACK())                 //应答失败
    {
        return (0); //退出
    }
    *AT24C02_Value = I2C_read_byte(); //读取并赋值
    Master_ACK(0);                    //主机发送非应答，使芯片停止发送数据
    I2C_Stop();
    return (1);
}

//////向24c02的address地址中写入一字节数据Data/////
BOOL I2C_AT24C02_WriteData(uchar addr, uchar Data)
{
    I2C_Start();                     //启动总线
    I2C_send_byte(AT24C02_ADDR + 0); //发送设备地址。+0表示写入
    if (!Test_ACK())                 //测试从设备，即应答失败
    {
        return (0); //返回0
    }
    I2C_send_byte(addr); //应答成功，则发送控制字,即写入地址
    if (!Test_ACK())     //测试从设备，即应答失败
    {
        return (0); //返回0
    }
    I2C_send_byte(Data); //应答成功，则发送数据
    if (!Test_ACK())     //测试从设备，即应答失败
    {
        return (0); //返回0
    }
    I2C_Stop();
    return (1);
}

/*********************************************************************************	
* 【程序功能】： 将各个地址所对应的存储单元逐一清零。  
           每格式化10个地址单元，P1口的LED变交替闪烁。
		     格式化完成，发光LED全亮，作为提示。			   			            			    
**********************************************************************************/
void clear()
{
    unsigned int i;
    P1 = 0x55;
    I2C_Init(); //初始化24C02
    for (i = 0; i < 256; i++)
    {
        I2C_AT24C02_WriteData(i, 0); //将各个地址所对应的存储单元逐一清零。
        if (i % 10 == 0)
            P1 = ~P1; //每格式化10个地址单元，P1口的LED变交替闪烁。
    }
    P1 = 0X00; //格式化完成，发光LED全亮，作为提示。
    while (1)
        ;
}

/*********************************************************************************	
* 【程序功能】： 向IIC总线器件24c02EEPROM中写入多个数据，写数据过程中LED闪烁。然后读出各个数据并显示在LED上		   			            			    
**********************************************************************************/

//此为待写入24c02的数据。为了便于验证结果，数组的内容为周期重复的。
char code music[] = {
    0x55, 0xaa, 0x0f, 0xf0, 0x55, 0xaa, 0x0f, 0xf0, 0x55, 0xaa, 0x0f, 0xf0,
    0x55, 0xaa, 0x0f, 0xf0, 0x55, 0xaa, 0x0f, 0xf0, 0x55, 0xaa, 0x0f, 0xf0,
    0x55, 0xaa, 0x0f, 0xf0, 0x55, 0xaa, 0x0f, 0xf0, 0x55, 0xaa, 0x0f, 0xf0,
    0x55, 0xaa, 0x0f, 0xf0, 0x55, 0xaa, 0x0f, 0xf0, 0x55, 0xaa, 0x0f, 0xf0,
    0x55, 0xaa, 0x0f, 0xf0, 0x55, 0xaa, 0x0f, 0xf0, 0x55, 0xaa, 0x0f, 0xf0};
//由于最后还要讲这些数据读出送到P1口，故可发现P1口相连的led有规律地闪烁15个周期

uchar data buffer[60]; //用于缓存从24c02中读取的数据。

void AT24C02Example()
{
    uchar add, i, j, m;
    I2C_Init(); //初始化IIC总线
    P1 = 0X55;  //‭01010101‬
    while (add != sizeof(music))
    {
        I2C_AT24C02_WriteData(0x00 + add, music[add]);
        Delay_Ms(5); //延时，使得可以看清写入过程
        add++;
        if (add % 4 == 0)
            P1 = ~P1; //写入过程中LED闪烁
    }
    P1 = 0X00; //点亮所有LED
    //到此为止，向24C02中写入数据的过程均已结束。下面的程序为附加的。
    //将已写入的数据再读出，送到P1口，通过观察led的亮灭来检验写入的数据是否正确。
    while (m != add)
    {
        I2C_AT24C02_ReadData(m, &buffer[i]);
        i++;
        m++;
    }

    while (j != add)
    {
        P1 = buffer[j];
        j++;
        Delay_Ms(1000);
    }
    while (1)
        ;
}
