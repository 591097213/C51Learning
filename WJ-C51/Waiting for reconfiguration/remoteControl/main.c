/**********************************************************************
---------------------自学习型万能红外遥控器----------------------------
	//-------------------------程序功能-------------------------//
	使用 JW-51学习板实现一个自学习型万能红外遥控器，
	LCD1602实时显示键值和状态（数值以16进制显示）。
功能主要分为学习红外遥控板键值波形并保存至JW-51学习板上
EEPROM（AT24C02）中，和从EEPROM读取学习的键值码波形原样发送两部分。

	//---------------------------备注---------------------------//
	因键盘上按键S11、S12、S13、S14、S15和红外接收头外部中断P32
引脚相连，所以在使用过程中以上按键不做学习和发送使用！

	//-------------------------使用说明-------------------------//
      JW-51学习板上电或复位后程序默认为学习键值码波形模式，
	通过按键S5可以选择为学习模式，S10可以选择为发送模式。

工作在学习模式时：先按需要学习的红外遥控板上的按键，这时按键用户码和按
键码将会显示在LCD1602的第二行，然后在按键开发板上的按键来学习此键值并保
存到EEPROM中，学习保存成功LCD1602的0x8E地址将会显示01（01表示成功，00表
示失败）。

工作在发射模式时：按下开发板上按键LCD1602第一行会显示发送的键值码
第二行显示按键码与是否发送成功 （01表示成功，00表示失败。地址0XC5上显示）
**********************************************************************/
#include "qxmcs51_config.h"
#include "delay.h"
#include "LCD1602.h"
#include "IR_TX_RX.H"
#include "KeyScan.h"
#include "IIC_AT24.h"

//-------------------------------------------
//工程包含文件
//main.c
//delay.c
//LCD1602.c
//IR_TX_RX.c
//IIC_AT24.c
//KeyScan.c
//-------------------------------------------

/**/
extern BOOL IR_RX_Finish;   //红外是否接收完成（完成=1，非=0）
extern BOOL KeyPro_OK;		//按键处理是否完成（完成=1，非=0）
extern BOOL TorR_Mod;		//发送和学习模式选择（1发送，0接收）
extern INT8U IR_RX_CODE[4]; //IR编码接收数组
extern INT8U IR_TX_CODE[4]; //IR编程发送数组

/*主函数*/
void main()
{
	INT8U Temp_Key; //存放按键处理值
	BOOL I2C_OK;	//I2C读写是否成功（1成功，0失败）

	LCD1602_Init();  //LCD1602初始化
	IR_TX_RX_Init(); //IR发送、接收初始化
	I2C_Init();		 //I2C初始化
	Delay_Ms(50);

	LCD1602_Write_String(0, 0, "Welcome to use!");
	LCD1602_Write_String(0, 1, "IR Remote Module"); //欢迎文字
	Delay_Ms(5000);									//5秒延时

	LCD1602_Clear(); //LCD1602清屏
	LCD1602_Write_String(0, 0, "Switch Mode!");
	LCD1602_Write_String(0, 1, "Default CopyMode"); //功能选择提示文字

	while (1)
	{
		if (IR_RX_Finish) //IR是否接收完成
		{
			IR_RX_Finish = 0;
			LCD1602_Clear();

			/*LCD1602以16进制显示IR接收到的用户名与键值码*/
			LCD1602_Write_String(0, 1, INT8UtostrHEX(IR_RX_CODE[0]));
			LCD1602_Write_String(3, 1, INT8UtostrHEX(IR_RX_CODE[1]));
			LCD1602_Write_String(7, 1, INT8UtostrHEX(IR_RX_CODE[2]));
			LCD1602_Write_String(10, 1, INT8UtostrHEX(IR_RX_CODE[3]));
		}
		Temp_Key = KeyPro(); //提取按键处理值
		if (KeyPro_OK)		 //按键处理是否成功
		{
			KeyPro_OK = 0;
			if (TorR_Mod) //选择为发送模式
			{
				if (Temp_Key != 0xfd) //非选择模式按键按下(S3,发送模式)
				{
					do
					{
						I2C_OK = I2C_ReadData(Temp_Key);
					} while (!I2C_OK); //等待I2C读取发送码成功

					Transmit_TR_CODE(); //发送红外编码
					LCD1602_Clear();

					LCD1602_Write_String(0, 1, INT8UtostrHEX(Temp_Key)); //显示学习按键值
					LCD1602_Write_String(5, 1, INT8UtostrHEX(I2C_OK));   //显示成功读取IR_TX_CODE

					/*LCD1602以16进制显示从I2C取出的IR_TX_CODE*/
					LCD1602_Write_String(0, 0, INT8UtostrHEX(IR_TX_CODE[0]));
					LCD1602_Write_String(3, 0, INT8UtostrHEX(IR_TX_CODE[1]));
					LCD1602_Write_String(7, 0, INT8UtostrHEX(IR_TX_CODE[2]));
					LCD1602_Write_String(10, 0, INT8UtostrHEX(IR_TX_CODE[3]));
				}
				else //选择模式按键按下(S3)
				{
					LCD1602_Clear();
					LCD1602_Write_String(0, 0, "IR Transmit Mode");
					LCD1602_Write_String(3, 1, "Please Key!");
				}
			}
			else //学习模式
			{
				if (Temp_Key != 0xfe) //非选择模式按键按下(S2，学习模式)
				{
					LCD1602_Clear();
					LCD1602_Write_String(14, 1, INT8UtostrHEX(Temp_Key)); //显示按键值
					do
					{
						I2C_OK = I2C_WriteData(Temp_Key);
					} while (!I2C_OK);									//等待将接收红外码存入I2C成功

					LCD1602_Write_String(14, 0, INT8UtostrHEX(I2C_OK)); //显示存入成功
					Delay_Ms(100);
					/*从I2C读取刚存入的IR学习码*/
					I2C_ReadData(Temp_Key);
					LCD1602_Write_String(0, 0, INT8UtostrHEX(IR_TX_CODE[0]));
					LCD1602_Write_String(3, 0, INT8UtostrHEX(IR_TX_CODE[1]));
					LCD1602_Write_String(7, 0, INT8UtostrHEX(IR_TX_CODE[2]));
					LCD1602_Write_String(10, 0, INT8UtostrHEX(IR_TX_CODE[3]));
				}
				else //选择模式按键按下(S2，学习模式)
				{
					LCD1602_Clear();
					LCD1602_Write_String(0, 0, "IRcode Copy Mode");
					LCD1602_Write_String(0, 1, "Please In IRcode");
				}
			}
		}
	}
}