/*************************************
PCF8591芯片相关
*************************************/

#include "qxmcs51_config.h"
#include "delay.h"
#include "IIC.h"
#include "PCF8591.H"

#define DAC_EN 0x40
#define ADC_AutoINC 0x04

/*AD读数据*/
BOOL I2C_ADC_ReadData(uchar ADDR, uchar *ADC_Value)//ADDR为通道号，ADC_Value为所读到的值.返回0，1表示读取成功与否
{
	I2C_Start();//启动总线
	I2C_send_byte(PCF8591_ADDR + 0);//发送设备地址。PCF8591_ADDR为PCF8591地址。在qxmcs51_config.h中。+0表示写入
	if (!Test_ACK())//测试从设备，即PCF8591应答失败
	{
		return(0);//返回0
	}
	I2C_send_byte(ADDR);//应答成功，则发送通道号，即读取地址
	Master_ACK(0);//主机发送应答

	I2C_Start();//启动总线
	I2C_send_byte(PCF8591_ADDR + 1);//发送PCF8591地址+1表示读取
	if (!Test_ACK())//应答失败
	{
		return(0);//退出
	}
	*ADC_Value = I2C_read_byte();//否则读数据
	Master_ACK(0);//主机发送应答
	I2C_Stop();
	return(1);	
}

/*向AD芯片写数据。数据范围0~255*/
BOOL I2C_DAC_WriteData(uchar Data)
{
	I2C_Start();//启动总线
	I2C_send_byte(PCF8591_ADDR + 0);//发送设备地址。PCF8591_ADDR为PCF8591地址。在qxmcs51_config.h中。+0表示写入
		if (!Test_ACK())//测试从设备，即PCF8591应答失败
	{
		return(0);//返回0
	}
		I2C_send_byte(0x40);//应答成功，则发送控制字，启动DA
			if (!Test_ACK())//测试从设备，即PCF8591应答失败
	{
		return(0);//返回0
	}
			I2C_send_byte(Data);//应答成功，则发送控制字，启动DA
				if (!Test_ACK())//测试从设备，即PCF8591应答失败
	{
		return(0);//返回0
	}
		I2C_Stop();
	return (1);
}
