
#include <reg52.h>

/**
 * 串口初始化，方式1，波特率9600.
 */
void Uart_init(void)
{
    TMOD = 0x20; //T1 方式2
    PCON = 0x00; //SMOD = 0
    SCON = 0x50; //方式1 8个数据位
    TH1 = 0xfd;
    TL1 = 0xfd;
    TR1 = 1; //启动定时器1
    ES = 1;  //开串口中断
    EA = 1;  //开总中断
}
