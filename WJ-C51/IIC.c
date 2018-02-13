/***************************************
总线相关
***************************************/

#include "qxmcs51_config.h"
#include "delay.h"
#include "IIC.h"

void I2C_Init() //I2C总线初始化
{
	SDA = 1;
	_nop_();
	SCL = 1;
	_nop_();
}

/*I2C起始信号*/
void I2C_Start()
{
	SCL = 1;
	_nop_();
	SDA = 1;
	Delay5us();
	SDA = 0;
	Delay5us();
}

/*I2C终止信号*/
void I2C_Stop()
{
	SDA = 0;
	_nop_();
	SCL = 1;
	Delay5us();
	SDA = 1;
	Delay5us();
}

/*1/0：主机发送应答/非应答*/
void Master_ACK(bit i)
{
	SCL = 0; // 拉低时钟总线允许SDA数据总线上的数据变化
	_nop_(); // 让总线稳定
	if (i)   //如果i = 1 那么拉低数据总线 表示主机应答
	{
		SDA = 0;
	}
	else
	{
		SDA = 1; //发送非应答
	}
	_nop_(); //让总线稳定
	SCL = 1; //拉高时钟总线 让从机从SDA线上读走 主机的应答信号
	Delay5us();
	SCL = 0; //拉低时钟总线， 占用总线继续通信
	_nop_();
	SDA = 1; //释放SDA数据总线。
	_nop_();
}

/*检测从机应答*/
bit Test_ACK() //返回0表示应答失败，1表示应答
{
	SCL = 1;
	Delay5us();
	if (SDA)
	{
		SCL = 0;
		_nop_();
		I2C_Stop(); //检测应答失败应该关闭总线
		return (0);
	}
	else
	{
		SCL = 0;
		_nop_();
		return (1);
	}
}

/*发送一个字节*/
void I2C_send_byte(uchar byte)
{
	uchar i;
	for (i = 0; i < 8; i++)
	{
		SCL = 0;
		_nop_();
		if (byte & 0x80)
		{
			SDA = 1;
			_nop_();
		}
		else
		{
			SDA = 0;
			_nop_();
		}
		SCL = 1;
		_nop_();
		byte <<= 1; // 0101 0100B
	}
	SCL = 0; //允许数据改变
	_nop_();
	SDA = 1; //恢复空闲状态
	_nop_();
}

/*I2C 读一字节*/
uchar I2C_read_byte()
{
	uchar dat, i;
	SCL = 0;
	_nop_();
	SDA = 1;
	_nop_();
	for (i = 0; i < 8; i++)
	{
		SCL = 1;
		_nop_();
		if (SDA)
		{
			dat |= 0x01; //0000 0001
		}
		else
		{
			dat &= 0xfe; //1111 1110
		}
		_nop_();
		SCL = 0;
		_nop_();
		if (i < 7)
		{
			dat = dat << 1;
		}
	}
	return (dat);
}