/*

	//-------------------------程序功能-------------------------//
	
	 读取JW-51学习板上4通道AD值，LCD1602以16进制实时显示。

	//---------------------------备注---------------------------//

	//-------------------------使用说明-------------------------//

**********************************************************************/
#include "fourChannelAD.h"
#include "qxmcs51_config.h"
#include "delay.h"
#include "LCD1602.h"
#include "IIC.h"
#include "PCF8591.H"

void fourChannelADExample()
{
	uchar i = 0;
	INT8U ADC_Value;
	LCD1602_Init();						//LCD1602液晶显示器初始化
	I2C_Init();							//I2C总线初始化
	LCD1602_Write_String(0, 0, "CH0:"); //第一行第一个字符开始，写入“”里的值，注意输入的是地址值
	LCD1602_Write_String(8, 0, "CH1:");
	LCD1602_Write_String(0, 1, "CH2:");
	LCD1602_Write_String(8, 1, "CH3:");
	while (1)
	{
		I2C_ADC_ReadData(1, &ADC_Value); //读数据。取ADC_Value的地址。
		//INT8UtostrHEX()函数将一个字节的数据转换为16进制字符串，返回不带“0x”这两个字符的两个16进制字符地址
		LCD1602_Write_String(4, 0, INT8UtostrHEX(ADC_Value)); // 通道0	 电位器
		Delay_Ms(100);										  //延时

		I2C_ADC_ReadData(2, &ADC_Value);					   //读数据
		LCD1602_Write_String(12, 0, INT8UtostrHEX(ADC_Value)); // 通道1	 光敏电阻
		Delay_Ms(100);										   //延时

		I2C_ADC_ReadData(3, &ADC_Value);					  //读数据
		LCD1602_Write_String(4, 1, INT8UtostrHEX(ADC_Value)); // 通道2	 热敏电阻
		Delay_Ms(100);										  //延时

		I2C_ADC_ReadData(0, &ADC_Value);					   //读数据
		LCD1602_Write_String(12, 1, INT8UtostrHEX(ADC_Value)); // 通道3	  模拟信号输入
		Delay_Ms(100);										   //延时
	}
}
