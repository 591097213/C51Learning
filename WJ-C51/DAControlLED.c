/**
 * 以DA方式控制LED亮度变化
 */
#include "DAControlLED.h"
#include "IIC.h"
#include "PCF8591.H"
#include "qxmcs51_config.h"
#include "delay.h"

void DAControlLED()
{
    uchar i = 0; //i取值0~255，代表D/转换的数字量。
    I2C_Init();  //I2C总线初始化
    while (1)
    {
        I2C_DAC_WriteData(i);
        i++;
        Delay_Ms(20);
    }
}