#include "qxmcs51_config.h"
#include "delay.h"
#include "KeyScan.h"

BOOL KeyScan_OK, KeyPro_OK, TorR_Mod;

/**
 * @brief 检测按键是否被按下
 * 返回值约定：
 *
 *
 * @return INT8U
 */
INT8U KeyScan() //带返回值的子函数
{
	INT8U cord_l, cord_h; //声明列线和行线的值的储存变量

	P3 = 0x0f;				 //0000 1111
	if ((P3 & 0x0f) != 0x0f) //判断是否有按键按下
	{
		Delay_Ms(5);			 //软件消抖
		if ((P3 & 0x0f) != 0x0f) //判断是否有按键按下
		{
			cord_l = P3 & 0x0f; // 储存列线值
			P3 = cord_l | 0xf0;
			cord_h = P3 & 0xf0; // 储存行线值
			while ((P3 & 0xf0) != 0xf0)
				;					  //松手检测
			KeyScan_OK = 1;			  //标志位置1
			return (cord_l + cord_h); //返回键值码
		}
	}
}


/**
 * @brief 将键值码进行转换，并控制模式转化、蜂鸣器发声
 *
 * @return INT8U
 */
INT8U KeyPro()
{
	INT8U Key_Value;
	Key_Value = KeyScan();
	if (KeyScan_OK)
	{
		KeyScan_OK = 0;
		switch (Key_Value)
		{

		//第一行键值码
		case 0xee:
			Key_Value = 0x00;
			break;
		case 0xde:
			Key_Value = 0x04;
			break;
		case 0xbe:
			Key_Value = 0x08;
			break;
		case 0x7e:
			Key_Value = 0x0b;
			break;

		//第二行键值码
		case 0xed:
			Key_Value = 0x10;
			break;
		case 0xdd:
			Key_Value = 0x14;
			break;
		case 0xbd:
			Key_Value = 0x18;
			break;
		case 0x7d:
			Key_Value = 0x1b;
			break;

		//第三行键值码
		case 0xeb:
			Key_Value = 0x09;
			break;
		case 0xdb:
			Key_Value = 0x0A;
			break;
		case 0xbb:
			Key_Value = 0x0b;
			break;
		case 0x7b:
			Key_Value = 0x0C;
			break;

		//第四行键值码
		case 0xe7:
			Key_Value = 0x20;
			break;
		case 0xd7:
			Key_Value = 0x24;
			break;
		case 0xb7:
			Key_Value = 0x28;
			break;
		case 0x77:
			Key_Value = 0x2b;
			break;

		//独立按键
		case 0xfe:
			TorR_Mod = 0;
			break;
		case 0xfd:
			TorR_Mod = 1;
			break;
		case 0xfb:
			Key_Value = 0x13;
			break;
		case 0xf7:
			Key_Value = 0x30;
			break;
		}
		KeyPro_OK = 1;
		// BEEP = 0;
		Delay_Ms(100);
		// BEEP = 1;
		return (Key_Value);
	}
}
