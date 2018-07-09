/**
 * @brief 1602LCD显示日历及温度。
 * 开发板上DS1302需要用杜邦线接到P10~P12引脚，装上备用电源（电池），同时流水灯跳线帽取下来
 *
 * @file 1602calendar.c
 * @author your name
 * @date 2018-03-17
 */

#include <reg52.h>
#include "intrins.h"
#include "datatype.h"
#include "DS1302.h"
#include "LCD1602.h"
#include "DS18b20.h"
#include "delay.h"
#include "qxmcs51_config.h"

//拼音“ri”命名为“_ri_”是为了不与RI寄存器同名（C51连接器大小写不敏感）
uchar a, miao, shi, fen, _ri_, yue, nian, week, keynum, temp,
	flagtime, ledcount, count, alarmflag, flagT /*时间分段标志*/,
	year1, month1, day1, lednum;

uint flag; //取温度

//存储农历世纪
bit c_moon;
//存储农历年份、月份、日
data uchar year_moon, month_moon, day_moon;

sbit Gled = P1 ^ 6; //定义粉色led灯的管脚
sbit Yled = P1 ^ 7; //定义黄色led灯的管脚

/**
 * @brief 写字符串函数（没有延时）
 * 在LCD1602第一行a3位置写str字符串
 * @param a3
 * @param str
 */
void print(uchar a3, uchar *str)
{
	LCD1602_Write_Com(a3 | 0x80);
	while (*str != '\0')
	{
		LCD1602_Write_Dat(*str++);
	}
	*str = 0;
}

/**
 * @brief 用于头字条的显示延时
 * 在LCD1602第一行a2位置写str字符串。写的过程中有延时
 * @param a2
 * @param str
 */
void print2(uchar a2, uchar *str)
{
	LCD1602_Write_Com(a2 | 0x80);
	while (*str != '\0')
	{
		Delay_Ms(130); //延时一下
		LCD1602_Write_Dat(*str++);
	}
	*str = 0;
}

void lcdinit() //1602初始化函数
{
	LCD1602_Init();
	print2(0x80, "^_^ @_@  U_U -_-"); //第一行显示
	print2(0x40, " 1602 electronic"); //第二行显示
	Delay_Ms(3000);					  // 延时3秒
}

/*以下是温度芯片DS18b20的相关函数*/

uint ReadTemperature(void) //读取温度
{
	uint t = 0;
	float tt = 0;
	DS18B20_SendChangeCmd();
	tt = DS18B20_GetTmpValue();
	t = tt * 10 + 0.5; //放大10倍并四舍五入
	return (t);
}

/*下面是相关数据的显示函数*/

/**
 * @brief 温度显示函数
 * 在LCD1602的第二行add位置写入dat数据
 * dat除以10后才是温度的真实值，包含一位小数
 *
 * @param add 写入
 * @param dat
 */
void writetemp(uchar add, uint dat) //写入温度值函数指定位置
{
	uchar gw, sw, xw; //个位，十位，一位小数位

	gw = dat % 100 / 10; //取出个位
	sw = dat / 100;		 //十位
	xw = dat % 10;		 //小数点

	LCD1602_Write_Com(h2 + add);  //h2为头文件规定的值0x80+0x40
	LCD1602_Write_Dat(0x30 + sw); //数字+30得到显示码
	LCD1602_Write_Dat(0x30 + gw); //个位数
	LCD1602_Write_Dat(0x2e);	  //小数点
	LCD1602_Write_Dat(0x30 + xw); //小数位
	LCD1602_Write_Dat(0xdf);	  //显示“°”（度）
	LCD1602_Write_Dat(0x43);	  //显示“C”符号
}

/**
 * @brief 显示两位数数值
 * 在lcd第二行add位置写入两位数数值
 * 用于显示时分秒时被调用
 *
 * @param add
 * @param dat
 */
void writetime(uchar add, uchar dat) reentrant //写入时分秒
{
	uchar gw, sw;

	gw = dat % 10; //取得个位数
	sw = dat / 10; //取得十位数

	LCD1602_Write_Com(h2 + add);  //第二行显示
	LCD1602_Write_Dat(0x30 + sw); //显示该数字
	LCD1602_Write_Dat(0x30 + gw);
}

/**
 * @brief 在第一行指定位置写入两位数值
 * 在lcd第一行add位置写入两位数数值
 * 用于显示年月日时被调用
 *
 * @param add
 * @param dat
 */
void writeday(uchar add, uchar dat) reentrant //写入年月日函数
{
	uchar gw, sw;

	gw = dat % 10; //取得个位数字
	sw = dat / 10; //取得十位数字

	LCD1602_Write_Com(h1 + add); //在第一行显示
	LCD1602_Write_Dat(0x30 + sw);
	LCD1602_Write_Dat(0x30 + gw); //显示
}

/**
 * @brief 在LCD特定位置处写入星期值
 *
 * @param week
 */
void writeweek(uchar week) reentrant //写星期函数
{
	LCD1602_Write_Com(h1 + 0x0e); /*巨注意：第一行是从0位开始的*/

	LCD1602_Write_Dat((week + '0')); //括号内写入星期
}

/**
 * @brief 按键扫描函数
 * 按下button0选择要修改的时间单位
 * 按下button1所选的时间单位+1
 * 按下button2所选的时间单位-1
 */
void keyscan()
{
	if (BUTTON0 == 0) //设置键按下
	{
		Delay_Ms(5); //延时
		if (BUTTON0 == 0)
		{
			BEEP = 0; //蜂鸣器短响一声
			Delay_Ms(20);
			BEEP = 1;
			alarmflag = 0; /*按下标志，用于防止调节时间时鸣叫*/
			while (!BUTTON0)
				;
			keynum++;
			if (keynum == 9)
				keynum = 1; //返回
			switch (keynum)
			{
			case 1:
				TR0 = 0;							   //关闭定时器
				LCD1602_Write_Com(h2 + 0x07);		   //秒的位置
				LCD1602_Write_Com(0x0f);			   //设置为光标闪烁
				temp = (miao) / 10 * 16 + (miao) % 10; //秒化为bcd码
				DS1302_Write(0x8e, 0x00);
				DS1302_Write(0x80, 0x80 | temp); //秒数据写入
				DS1302_Write(0x8e, 0x80);
				break;

			case 2:
				LCD1602_Write_Com(h2 + 4); //分的位置
				break;					   //不用再次设置为闪烁状态了

			case 3:
				LCD1602_Write_Com(h2 + 1); //时的位置
				break;

			case 4:
				LCD1602_Write_Com(h1 + 0x0e); //星期的位置
				break;

			case 5:
				LCD1602_Write_Com(h1 + 0x09); //日的位置
				break;

			case 6:
				LCD1602_Write_Com(h1 + 0x06); //月的位置
				break;

			case 7:
				LCD1602_Write_Com(h1 + 0x3); //年的位置
				break;

			case 8:
				LCD1602_Write_Com(0x0c); //第8次，光标不闪烁
				alarmflag = 1;			 /*设置标志重新设置为1*/
				TR0 = 1;				 //重新打开定时器
				temp = (miao) / 10 * 16 + (miao) % 10;
				DS1302_Write(0x8e, 0x00);		 //允许写
				DS1302_Write(0x80, 0x00 | temp); //写入秒
				DS1302_Write(0x8e, 0x80);		 //打开保护
				break;
			}
		}
	}

	if (keynum != 0) //当设置键按下时才能操作
	{
		if (BUTTON1 == 0) //加键
		{
			Delay_Ms(5);
			if (BUTTON1 == 0)
			{
				BEEP = 0; //蜂鸣器短响
				Delay_Ms(20);
				BEEP = 1;
				while (!BUTTON1)
					; //按键松开
				switch (keynum)
				{
				case 1:		//调整秒
					miao++; //
					if (miao == 60)
						miao = 0;
					writetime(0x06, miao);				   /*在十位的位置写入，因为为两位数，个位数自动再后面显示*/
					temp = (miao) / 10 * 16 + (miao) % 10; //转换为bcd码
					DS1302_Write(0x8e, 0x00);			   //允许写
					DS1302_Write(0x80, temp);			   // 写入秒
					DS1302_Write(0x8e, 0x80);			   //打开保护
					LCD1602_Write_Com(h2 + 0x07);		   //液晶模式为写入后自动右移，在此返回原来位置
					break;
				case 2: //调整分
					fen++;
					if (fen == 60)
						fen = 0;
					writetime(0x03, fen);				 //在十位数位置开始写入
					temp = (fen) / 10 * 16 + (fen) % 10; //转换为bcd码
					DS1302_Write(0x8e, 0x00);			 //允许写
					DS1302_Write(0x82, temp);			 //写入分
					DS1302_Write(0x8e, 0x80);			 //打开保护
					LCD1602_Write_Com(h2 + 4);			 //返回个位数的位置
					break;
				case 3: //调整时
					shi++;
					if (shi == 24)
						shi = 0;
					writetime(0, shi);					 //在0位开始写入
					temp = (shi) / 10 * 16 + (shi) % 10; //转换为bcd码
					DS1302_Write(0x8e, 0x00);			 //允许写
					DS1302_Write(0x84, temp);			 //写入时
					DS1302_Write(0x8e, 0x80);			 //打开保护
					LCD1602_Write_Com(h2 + 1);			 //返回到个位位置
					break;
				case 4: //调整周
					week++;
					if (week == 8)
						week = 1;
					LCD1602_Write_Com(h1 + 0x0e); //显示位置
					writeweek(week);			  //写入星期
					temp = (week) / 10 * 16 + (week) % 10;
					DS1302_Write(0x8e, 0x00);	 //允许写入
					DS1302_Write(0x8a, temp);	 //写入周
					DS1302_Write(0x8e, 0x80);	 //打开保护
					LCD1602_Write_Com(h1 + 0x0e); /*返回原来位置*/
					break;
				case 5: //调整日
					_ri_++;
					if (_ri_ == 32)
						_ri_ = 1;
					writeday(8, _ri_);					   //注意是在十位开始写入
					temp = (_ri_) / 10 * 16 + (_ri_) % 10; //转换为bcd码
					DS1302_Write(0x8e, 0x00);			   //允许写
					DS1302_Write(0x86, temp);			   //写入日
					DS1302_Write(0x8e, 0x80);			   //打开保护
					LCD1602_Write_Com(h1 + 9);			   //返回个位数
					break;
				case 6: //调整月
					yue++;
					if (yue == 13)
						yue = 1;
					writeday(5, yue);					 //在十位开始写入
					temp = (yue) / 10 * 16 + (yue) % 10; //转换为bcd码
					DS1302_Write(0x8e, 0x00);			 //允许写
					DS1302_Write(0x88, temp);			 //写入月
					DS1302_Write(0x8e, 0x80);			 //打开保护
					LCD1602_Write_Com(h1 + 6);			 //返回个位位置
					break;
				case 7: //调整年
					nian++;
					if (nian == 100)
						nian = 0;
					writeday(2, nian);					   //在第一行第三个字符开始写入
					temp = (nian) / 10 * 16 + (nian) % 10; //转换为bcd码
					DS1302_Write(0x8e, 0x00);			   //允许写
					DS1302_Write(0x8c, temp);			   //写入年
					DS1302_Write(0x8e, 0x80);			   //打开保护
					LCD1602_Write_Com(h1 + 3);			   //返回个位位置
					break;
				}
			}
		}

		//以下是减的函数
		if (BUTTON2 == 0)
		{
			Delay_Ms(5); //消抖
			if (BUTTON2 == 0)
			{
				BEEP = 0; //蜂鸣器短响一下
				Delay_Ms(20);
				BEEP = 1;
				while (!BUTTON2)
					;
				switch (keynum)
				{
				case 1:
					miao--; /*此处有疑问：无符号数据是否要修改*/
					if (miao == -1)
						miao = 59;						   //减到-1返回59
					writetime(0x06, miao);				   //在十位数写入
					temp = (miao) / 10 * 16 + (miao) % 10; //转换为bcd码
					DS1302_Write(0x8e, 0x00);			   //允许写
					DS1302_Write(0x80, temp);			   //写入秒
					DS1302_Write(0x8e, 0x80);			   //打开保护
					LCD1602_Write_Com(h2 + 0x07);		   //返回个位位置
					break;
				case 2:
					fen--;
					if (fen == -1)
						fen = 59;
					writetime(0x03, fen);				 //在十位数位置写入
					temp = (fen) / 10 * 16 + (fen) % 10; //转换为bcd码
					DS1302_Write(0x8e, 0x00);			 //允许写入
					DS1302_Write(0x82, temp);			 //写入分
					DS1302_Write(0x8e, 0x80);			 //打开保护
					LCD1602_Write_Com(h2 + 4);			 //返回个位数位置
					break;
				case 3:
					shi--;
					if (shi == -1)
						shi = 23;
					writetime(0, shi);					 //在0位开始写入
					temp = (shi) / 10 * 16 + (shi) % 10; //转换为bcd码
					DS1302_Write(0x8e, 0x00);			 //允许写入
					DS1302_Write(0x84, temp);			 //写入时
					DS1302_Write(0x8e, 0x80);			 //打开保护
					LCD1602_Write_Com(h2 + 1);			 //返回到个位位置
					break;
				case 4:
					week--;
					if (week == 0)
						week = 7;
					LCD1602_Write_Com(h1 + 0x0e);		   //显示位置
					writeweek(week);					   //写入星期
					temp = (week) / 10 * 16 + (week) % 10; //转换为bcd码
					DS1302_Write(0x8e, 0x00);			   //允许写入
					DS1302_Write(0x8a, temp);			   //写入周
					DS1302_Write(0x8e, 0x80);			   //打开保护
					LCD1602_Write_Com(h1 + 0x0e);		   //返回原来位置
					break;
				case 5:
					_ri_--;
					if (_ri_ == 0)
						_ri_ = 31;
					writeday(8, _ri_);					   //在十位开始显示
					temp = (_ri_) / 10 * 16 + (_ri_) % 10; //转换为bcd码
					DS1302_Write(0x8e, 0x00);			   //允许写入
					DS1302_Write(0x86, temp);			   //写入日
					DS1302_Write(0x8e, 0x80);			   //打开保护
					LCD1602_Write_Com(h1 + 9);			   //返回个位数
					break;
				case 6:
					yue--;
					if (yue == 0)
						yue = 12;
					writeday(5, yue);					 //在十位数位置开始写入
					temp = (yue) / 10 * 16 + (yue) % 10; //转换为bcd码
					DS1302_Write(0x8e, 0x00);			 //允许写入
					DS1302_Write(0x88, temp);			 //写入月
					DS1302_Write(0x8e, 0x80);			 //打开保护
					LCD1602_Write_Com(h1 + 6);			 //返回到个位位置
					break;
				case 7:
					nian--;
					if (nian == -1)
						nian = 99;
					writeday(2, nian);					   //第一行第三个字符开始写入
					temp = (nian) / 10 * 16 + (nian) % 10; //转换为bcd码
					DS1302_Write(0x8e, 0x00);			   //允许写入
					DS1302_Write(0x8c, temp);			   //写入年
					DS1302_Write(0x8e, 0x80);			   //打开保护
					LCD1602_Write_Com(h1 + 3);			   //返回在年的尾数位置
					break;
				}
			}
		}
	}
}

//初始化的函数
void TimerInit() //定时器初始化函数
{
	TMOD = 0x11;				 //设置为定时器0和1的工作方式1
	TH0 = (65536 - 60000) / 256; //10毫秒
	TL0 = (65536 - 60000) % 256;
	TH1 = (65536 - 50000) / 256; //设置定时器1的初值
	TL1 = (65536 - 50000) % 256;
	EA = 1;
	ET0 = 1; //允许T0中断
	TR0 = 1; //启动中断
	ET1 = 1; /*允许T1中断（未打开）*/
	TR1 = 1; //打开
}

/*////////以下为转换农历的相关函数////////////////// */
//1901~2099共199年的数据
//一年占3字节编码如下：
//0000 |0000 0000 0000 0|00|0 0000
//* 十进制数表示该年中农历闰月的月份。0为无闰月
//* 从左→右表示1~13农历月是大月(1)还是小月(0)。农历加上闰月可能有13个月，故占13位。
//		闰月紧排在对应的正常月之后。农历月从春节数起，延伸到下一公历年的春节前
//* 春节在这一年的公历月份。春节只能在公历1月或2月，故占两位
//* 春节所在的公历日。取值为1~31。占5位
code uchar year_code[597] = {
	0x04, 0xAe, 0x53, //1901 0
	0x0A, 0x57, 0x48, //1902 3
	0x55, 0x26, 0xBd, //1903 6
	0x0d, 0x26, 0x50, //1904 9
	0x0d, 0x95, 0x44, //1905 12
	0x46, 0xAA, 0xB9, //1906 15
	0x05, 0x6A, 0x4d, //1907 18
	0x09, 0xAd, 0x42, //1908 21
	0x24, 0xAe, 0xB6, //1909
	0x04, 0xAe, 0x4A, //1910
	0x6A, 0x4d, 0xBe, //1911
	0x0A, 0x4d, 0x52, //1912
	0x0d, 0x25, 0x46, //1913
	0x5d, 0x52, 0xBA, //1914
	0x0B, 0x54, 0x4e, //1915
	0x0d, 0x6A, 0x43, //1916
	0x29, 0x6d, 0x37, //1917
	0x09, 0x5B, 0x4B, //1918
	0x74, 0x9B, 0xC1, //1919
	0x04, 0x97, 0x54, //1920
	0x0A, 0x4B, 0x48, //1921
	0x5B, 0x25, 0xBC, //1922
	0x06, 0xA5, 0x50, //1923
	0x06, 0xd4, 0x45, //1924
	0x4A, 0xdA, 0xB8, //1925
	0x02, 0xB6, 0x4d, //1926
	0x09, 0x57, 0x42, //1927
	0x24, 0x97, 0xB7, //1928
	0x04, 0x97, 0x4A, //1929
	0x66, 0x4B, 0x3e, //1930
	0x0d, 0x4A, 0x51, //1931
	0x0e, 0xA5, 0x46, //1932
	0x56, 0xd4, 0xBA, //1933
	0x05, 0xAd, 0x4e, //1934
	0x02, 0xB6, 0x44, //1935
	0x39, 0x37, 0x38, //1936
	0x09, 0x2e, 0x4B, //1937
	0x7C, 0x96, 0xBf, //1938
	0x0C, 0x95, 0x53, //1939
	0x0d, 0x4A, 0x48, //1940
	0x6d, 0xA5, 0x3B, //1941
	0x0B, 0x55, 0x4f, //1942
	0x05, 0x6A, 0x45, //1943
	0x4A, 0xAd, 0xB9, //1944
	0x02, 0x5d, 0x4d, //1945
	0x09, 0x2d, 0x42, //1946
	0x2C, 0x95, 0xB6, //1947
	0x0A, 0x95, 0x4A, //1948
	0x7B, 0x4A, 0xBd, //1949
	0x06, 0xCA, 0x51, //1950
	0x0B, 0x55, 0x46, //1951
	0x55, 0x5A, 0xBB, //1952
	0x04, 0xdA, 0x4e, //1953
	0x0A, 0x5B, 0x43, //1954
	0x35, 0x2B, 0xB8, //1955
	0x05, 0x2B, 0x4C, //1956
	0x8A, 0x95, 0x3f, //1957
	0x0e, 0x95, 0x52, //1958
	0x06, 0xAA, 0x48, //1959
	0x7A, 0xd5, 0x3C, //1960
	0x0A, 0xB5, 0x4f, //1961
	0x04, 0xB6, 0x45, //1962
	0x4A, 0x57, 0x39, //1963
	0x0A, 0x57, 0x4d, //1964
	0x05, 0x26, 0x42, //1965
	0x3e, 0x93, 0x35, //1966
	0x0d, 0x95, 0x49, //1967
	0x75, 0xAA, 0xBe, //1968
	0x05, 0x6A, 0x51, //1969
	0x09, 0x6d, 0x46, //1970
	0x54, 0xAe, 0xBB, //1971
	0x04, 0xAd, 0x4f, //1972
	0x0A, 0x4d, 0x43, //1973
	0x4d, 0x26, 0xB7, //1974
	0x0d, 0x25, 0x4B, //1975
	0x8d, 0x52, 0xBf, //1976
	0x0B, 0x54, 0x52, //1977
	0x0B, 0x6A, 0x47, //1978
	0x69, 0x6d, 0x3C, //1979
	0x09, 0x5B, 0x50, //1980
	0x04, 0x9B, 0x45, //1981
	0x4A, 0x4B, 0xB9, //1982
	0x0A, 0x4B, 0x4d, //1983
	0xAB, 0x25, 0xC2, //1984
	0x06, 0xA5, 0x54, //1985
	0x06, 0xd4, 0x49, //1986
	0x6A, 0xdA, 0x3d, //1987
	0x0A, 0xB6, 0x51, //1988
	0x09, 0x37, 0x46, //1989
	0x54, 0x97, 0xBB, //1990
	0x04, 0x97, 0x4f, //1991
	0x06, 0x4B, 0x44, //1992
	0x36, 0xA5, 0x37, //1993
	0x0e, 0xA5, 0x4A, //1994
	0x86, 0xB2, 0xBf, //1995
	0x05, 0xAC, 0x53, //1996
	0x0A, 0xB6, 0x47, //1997
	0x59, 0x36, 0xBC, //1998
	0x09, 0x2e, 0x50, //1999 294
	0x0C, 0x96, 0x45, //2000 297
	0x4d, 0x4A, 0xB8, //2001
	0x0d, 0x4A, 0x4C, //2002
	0x0d, 0xA5, 0x41, //2003
	0x25, 0xAA, 0xB6, //2004
	0x05, 0x6A, 0x49, //2005
	0x7A, 0xAd, 0xBd, //2006
	0x02, 0x5d, 0x52, //2007
	0x09, 0x2d, 0x47, //2008
	0x5C, 0x95, 0xBA, //2009
	0x0A, 0x95, 0x4e, //2010
	0x0B, 0x4A, 0x43, //2011
	0x4B, 0x55, 0x37, //2012
	0x0A, 0xd5, 0x4A, //2013
	0x95, 0x5A, 0xBf, //2014
	0x04, 0xBA, 0x53, //2015
	0x0A, 0x5B, 0x48, //2016
	0x65, 0x2B, 0xBC, //2017
	0x05, 0x2B, 0x50, //2018
	0x0A, 0x93, 0x45, //2019
	0x47, 0x4A, 0xB9, //2020
	0x06, 0xAA, 0x4C, //2021
	0x0A, 0xd5, 0x41, //2022
	0x24, 0xdA, 0xB6, //2023
	0x04, 0xB6, 0x4A, //2024
	0x69, 0x57, 0x3d, //2025
	0x0A, 0x4e, 0x51, //2026
	0x0d, 0x26, 0x46, //2027
	0x5e, 0x93, 0x3A, //2028
	0x0d, 0x53, 0x4d, //2029
	0x05, 0xAA, 0x43, //2030
	0x36, 0xB5, 0x37, //2031
	0x09, 0x6d, 0x4B, //2032
	0xB4, 0xAe, 0xBf, //2033
	0x04, 0xAd, 0x53, //2034
	0x0A, 0x4d, 0x48, //2035
	0x6d, 0x25, 0xBC, //2036
	0x0d, 0x25, 0x4f, //2037
	0x0d, 0x52, 0x44, //2038
	0x5d, 0xAA, 0x38, //2039
	0x0B, 0x5A, 0x4C, //2040
	0x05, 0x6d, 0x41, //2041
	0x24, 0xAd, 0xB6, //2042
	0x04, 0x9B, 0x4A, //2043
	0x7A, 0x4B, 0xBe, //2044
	0x0A, 0x4B, 0x51, //2045
	0x0A, 0xA5, 0x46, //2046
	0x5B, 0x52, 0xBA, //2047
	0x06, 0xd2, 0x4e, //2048
	0x0A, 0xdA, 0x42, //2049
	0x35, 0x5B, 0x37, //2050
	0x09, 0x37, 0x4B, //2051
	0x84, 0x97, 0xC1, //2052
	0x04, 0x97, 0x53, //2053
	0x06, 0x4B, 0x48, //2054
	0x66, 0xA5, 0x3C, //2055
	0x0e, 0xA5, 0x4f, //2056
	0x06, 0xB2, 0x44, //2057
	0x4A, 0xB6, 0x38, //2058
	0x0A, 0xAe, 0x4C, //2059
	0x09, 0x2e, 0x42, //2060
	0x3C, 0x97, 0x35, //2061
	0x0C, 0x96, 0x49, //2062
	0x7d, 0x4A, 0xBd, //2063
	0x0d, 0x4A, 0x51, //2064
	0x0d, 0xA5, 0x45, //2065
	0x55, 0xAA, 0xBA, //2066
	0x05, 0x6A, 0x4e, //2067
	0x0A, 0x6d, 0x43, //2068
	0x45, 0x2e, 0xB7, //2069
	0x05, 0x2d, 0x4B, //2070
	0x8A, 0x95, 0xBf, //2071
	0x0A, 0x95, 0x53, //2072
	0x0B, 0x4A, 0x47, //2073
	0x6B, 0x55, 0x3B, //2074
	0x0A, 0xd5, 0x4f, //2075
	0x05, 0x5A, 0x45, //2076
	0x4A, 0x5d, 0x38, //2077
	0x0A, 0x5B, 0x4C, //2078
	0x05, 0x2B, 0x42, //2079
	0x3A, 0x93, 0xB6, //2080
	0x06, 0x93, 0x49, //2081
	0x77, 0x29, 0xBd, //2082
	0x06, 0xAA, 0x51, //2083
	0x0A, 0xd5, 0x46, //2084
	0x54, 0xdA, 0xBA, //2085
	0x04, 0xB6, 0x4e, //2086
	0x0A, 0x57, 0x43, //2087
	0x45, 0x27, 0x38, //2088
	0x0d, 0x26, 0x4A, //2089
	0x8e, 0x93, 0x3e, //2090
	0x0d, 0x52, 0x52, //2091
	0x0d, 0xAA, 0x47, //2092
	0x66, 0xB5, 0x3B, //2093
	0x05, 0x6d, 0x4f, //2094
	0x04, 0xAe, 0x45, //2095
	0x4A, 0x4e, 0xB9, //2096
	0x0A, 0x4d, 0x4C, //2097
	0x0d, 0x15, 0x41, //2098
	0x2d, 0x92, 0xB5, //2099
};

//月份数据表
code uchar day_code1[9] = { //存储元旦到本月初共多少天。二月按28天计算。用于计算某一天是今年第几天。后续处理要记得闰年二月的处理
	//0   31   59    90    120    151   181   212   243
	0x0, 0x1f, 0x3b, 0x5a, 0x78, 0x97, 0xb5, 0xd4, 0xf3};

code uint day_code2[3] = { //同上。这3个数值用int类型的数组存储
	//273   304   334
	0x111, 0x130, 0x14e};

/**
 * @brief 检查该**农历月**为大小还是小月,大月返回1,小月返回0
 *
 *
 * @param month_p
 * @param table_addr
 * @return bit
 */
bit get_moon_day(uchar month_p, uint table_addr)
{
	uchar temp;
	switch (month_p)
	{
	case 1:
	{
		temp = year_code[table_addr] & 0x08;
		if (temp == 0)
			return (0);
		else
			return (1);
	}
	case 2:
	{
		temp = year_code[table_addr] & 0x04;
		if (temp == 0)
			return (0);
		else
			return (1);
	}
	case 3:
	{
		temp = year_code[table_addr] & 0x02;
		if (temp == 0)
			return (0);
		else
			return (1);
	}
	case 4:
	{
		temp = year_code[table_addr] & 0x01;
		if (temp == 0)
			return (0);
		else
			return (1);
	}
	case 5:
	{
		temp = year_code[table_addr + 1] & 0x80;
		if (temp == 0)
			return (0);
		else
			return (1);
	}
	case 6:
	{
		temp = year_code[table_addr + 1] & 0x40;
		if (temp == 0)
			return (0);
		else
			return (1);
	}
	case 7:
	{
		temp = year_code[table_addr + 1] & 0x20;
		if (temp == 0)
			return (0);
		else
			return (1);
	}
	case 8:
	{
		temp = year_code[table_addr + 1] & 0x10;
		if (temp == 0)
			return (0);
		else
			return (1);
	}
	case 9:
	{
		temp = year_code[table_addr + 1] & 0x08;
		if (temp == 0)
			return (0);
		else
			return (1);
	}
	case 10:
	{
		temp = year_code[table_addr + 1] & 0x04;
		if (temp == 0)
			return (0);
		else
			return (1);
	}
	case 11:
	{
		temp = year_code[table_addr + 1] & 0x02;
		if (temp == 0)
			return (0);
		else
			return (1);
	}
	case 12:
	{
		temp = year_code[table_addr + 1] & 0x01;
		if (temp == 0)
			return (0);
		else
			return (1);
	}
	case 13:
	{
		temp = year_code[table_addr + 2] & 0x80;
		if (temp == 0)
			return (0);
		else
			return (1);
	}
	default:
	{
		return (0);
	}
	}
}
/**
 * @brief 将公历转化为农历
 * 农历日期输出到全局变量year_moon、month_moon、day_moon中。
 * 输入输出数据均为BCD数据
 * @param c c=0 为21世纪,c=1 为20世纪
 * @param year year用两位数值的bcd码表示
 * @param month
 * @param day
 */
void Conversion(bit c, uchar year, uchar month, uchar day)
{
	uchar temp1, temp2, temp3, month_p;
	uint temp4, table_addr;
	bit flag2, flag_y;

	temp1 = year / 16; //BCD->hex 先把数据转换为十六进制。year只有两位数
	temp2 = year % 16;
	year = temp1 * 10 + temp2; //转为十进制

	temp1 = month / 16;
	temp2 = month % 16;
	month = temp1 * 10 + temp2; //转为十进制

	temp1 = day / 16;
	temp2 = day % 16;
	day = temp1 * 10 + temp2; //转为十进制
	//定位数据表地址
	if (c == 0) //c=0 为21世纪
	{
		table_addr = (year + 0x64 - 1) * 0x3; //0x64=100
	}
	else //c=1 为20世纪
	{
		table_addr = (year - 1) * 0x3;
	}

	//定位数据表地址完成

	//取当年春节所在的公历月份
	temp1 = year_code[table_addr + 2] & 0x60; //0x60=0110 0000
	temp1 = _cror_(temp1, 5);				  //循环右移5位
	//取当年春节所在的公历月份完成

	//取当年春节所在的公历日
	temp2 = year_code[table_addr + 2] & 0x1f; //0001 1111
	//取当年春节所在的公历日完成

	// 计算当年春年离当年元旦的天数，存于temp3中,春节只会在公历1月或2月
	if (temp1 == 0x1) //如果春节在1月
	{
		temp3 = temp2 - 1;
	}
	else //如果春节在2月
	{
		temp3 = temp2 + 0x1f - 1; //0x1f=31
	}

	//计算公历日离当年元旦的天数存于temp4中
	if (month < 10) //1~9月
	{
		temp4 = day_code1[month - 1] + day - 1;
	}
	else //10~12月
	{
		temp4 = day_code2[month - 10] + day - 1;
	}

	if ((month > 0x2) && (year % 0x4 == 0)) //此处判读闰年的方法有误
	{										//如果公历月大于2月并且该年的2月为闰月,天数加1
		temp4 += 1;
	}
	//计算公历日离当年元旦的天数完成

	//判断公历日在春节前还是春节后
	//temp3是当年春节离当年元旦的天数
	//temp4是公历日距离元旦的天数
	if (temp4 >= temp3) //公历日在春节后或就是春节当日
	{
		temp4 -= temp3; //temp4存储该天是当年农历第几天

		month = 0x1;							   //month是最终确定的农历月份取值1~12。这里初值为首月
		month_p = 0x1;							   //month_p为月份指向取值1~13,公历日在春节后或就是春节当日month_p先指向首月
		flag2 = get_moon_day(month_p, table_addr); //检查该农历月为大小还是小月,大月返回1,小月返回0

		flag_y = 0; //标志位。记录是否进入了农历闰月的循环

		if (flag2 == 0)
			temp1 = 0x1d; //小月29天
		else
			temp1 = 0x1e; //大小30天

		temp2 = year_code[table_addr] & 0xf0;
		temp2 = _cror_(temp2, 4); //从数据表中取该年的闰月月份,如为0则该年无闰月

		while (temp4 >= temp1) //temp4逐次减去各个月份的天数，最后算出该天在农历第几月
		{
			temp4 -= temp1;
			month_p += 1;		//指向下一个月
			if (month == temp2) //如果是闰月
			{
				flag_y = ~flag_y; //标志位取反。month跳过一次自加。而month_p不跳过
				if (flag_y == 0)
					month += 1;
			}
			else //如果不是闰月
				month += 1;

			flag2 = get_moon_day(month_p, table_addr); //判断下个月是大月还是小月
			if (flag2 == 0)							   //根据大月小月处理天数
				temp1 = 0x1d;
			else
				temp1 = 0x1e;
		}
		day = temp4 + 1; //农历的日期
	}
	else //公历日在春节前。农历年需要在公历年的基础上减1
	{
		temp3 -= temp4;  //temp3存储还差多少天才到下一农历年
		if (year == 0x0) //2000年单独处理
		{
			year = 0x63; //0x63=99
			c = 1;		 //上个世纪
		}
		else
			year -= 1;	 //公历年份减1
		table_addr -= 0x3; //数据地址上移一行，即年份减去一年
		month = 0xc;	   //mouth存储最终的农历月份。这里初值从12月开始

		temp2 = year_code[table_addr] & 0xf0;
		temp2 = _cror_(temp2, 4); //取出闰月月份

		if (temp2 == 0)	//如果该年没有闰月
			month_p = 0xc; //月份指针从12开始
		else			   //如果该年有闰月
			month_p = 0xd; //从13开始

		flag_y = 0; //是否进入闰月循环的标志。初值为0

		flag2 = get_moon_day(month_p, table_addr); //判断大月or小月
		if (flag2 == 0)							   //处理天数
			temp1 = 0x1d;
		else
			temp1 = 0x1e;

		while (temp3 > temp1)
		{
			temp3 -= temp1;
			month_p -= 1;

			if (flag_y == 0)
				month -= 1;
			if (month == temp2)
				flag_y = ~flag_y;

			flag2 = get_moon_day(month_p, table_addr);
			if (flag2 == 0)
				temp1 = 0x1d;
			else
				temp1 = 0x1e;
		}
		day = temp1 - temp3 + 1;
	}

	c_moon = c; //HEX->BCD ,运算结束后,把数据转换为BCD数据

	temp1 = year / 10;
	temp1 = _crol_(temp1, 4);
	temp2 = year % 10;
	year_moon = temp1 | temp2;

	temp1 = month / 10;
	temp1 = _crol_(temp1, 4);
	temp2 = month % 10;
	month_moon = temp1 | temp2;

	temp1 = day / 10;
	temp1 = _crol_(temp1, 4);
	temp2 = day % 10;
	day_moon = temp1 | temp2;
}

/*///////////农历转换分界线 */

/**
 * @brief 显示节日
 * 因为编译后超过8192B，这段函数只能先注释掉一些
 *
 */
void festival() //显示节日函数
{
	if (yue == 1 && _ri_ == 1)
	{
		print(0x80, " New Year's Day ");
		print(0x40, "Happy New Year!!");
	} //1月
	else if (month_moon == 0x01 && day_moon == 0x01)
	{
		print(0x80, "Happy New Year!!");
		print(0x40, "Spring Festival!");
	}
	else if (month_moon == 0x01 && day_moon == 0x15)
	{
		print(0x80, " yuan xiao jie: ");
		print(0x40, "Lantern Festival");
	} //元宵节

	else if (yue == 2 && _ri_ == 2) //世界湿地日
	{
		print(0x80, " World Wetlands ");
		print(0x40, "      Day       ");
	}								 //2月
	else if (yue == 2 && _ri_ == 14) //情人节
	{
		print(0x80, "    Today is:   ");
		print(0x40, "Valentine's Day!");
	}

	else if (yue == 3 && _ri_ == 8) //妇女节
	{
		print(0x80, " International  ");
		print(0x40, "   Women' Day   ");
	}
	else if (yue == 3 && _ri_ == 12) //植树节
	{
		print(0x80, "    Today is:   ");
		print(0x40, "China Arbor Day ");
	}
	// else if (yue == 3 && _ri_ == 14) //白色情人节
	// {
	// 	print(0x80, "    Today is:   ");
	// 	print(0x40, "   White Day   ");
	// }
	// else if (yue == 3 && _ri_ == 15) //消费者权益日
	// {
	// 	print(0x80, " World Consumer ");
	// 	print(0x40, "   Right Day    ");
	// }
	// else if (yue == 3 && _ri_ == 17) //世界航海日
	// {
	// 	print(0x80, " International  ");
	// 	print(0x40, "  sailing day   ");
	// }
	// else if (yue == 3 && _ri_ == 21) //世界森林日
	// {
	// 	print(0x80, "    Today is:   ");
	// 	print(0x40, "World Forest Day");
	// }
	// else if (yue == 3 && _ri_ == 22) //世界水日
	// {
	// 	print(0x80, "    Today is:   ");
	// 	print(0x40, "World Water Day ");
	// }
	// if (yue == 3 && _ri_ == 23) //世界气象日
	// {
	// 	print(0x80, "World Meteorolo-");
	// 	print(0x40, "   gical Day    ");
	// }
	// if (yue == 3 && _ri_ == 24) //世界防治结核病日
	// {
	// 	print(0x80, "World Tubercul-");
	// 	print(0x40, "    osis Day    ");
	// }

	// else if (yue == 4 && _ri_ == 1) //愚人节
	// {
	// 	print(0x80, "    Today is:   ");
	// 	print(0x40, "April Fools' Day");
	// }										//4
	// else if (yue == 4 && _ri_ == 5) //清明节
	// {
	// 	print(0x80, "   Ching Ming   ");
	// 	print(0x40, "    Festival    ");
	// }
	// else if (yue == 4 && _ri_ == 7) //世界健康日
	// {
	// 	print(0x80, "    Today is:   ");
	// 	print(0x40, "World Health Day");
	// }
	// else if (yue == 4 && _ri_ == 8) //复活节
	// {
	// 	print(0x80, "    Today is:   ");
	// 	print(0x40, "   Easter Day   ");
	// }

	// else if (month_moon == 0x05 && day_moon == 0x05) //端午节
	// {
	// 	print(0x80, "the Dragon-Boat ");
	// 	print(0x40, "    Festival    ");
	// }
	// else if (yue == 5 && _ri_ == 1) //劳动节
	// {
	// 	print(0x80, " International  ");
	// 	print(0x40, "   Labour Day   ");
	// }										//5
	// else if (yue == 5 && _ri_ == 4) //青年节
	// {
	// 	print(0x80, " Chinese Youth  ");
	// 	print(0x40, "      Day       ");
	// }
	// else if (yue == 5 && _ri_ == 8) //世界红十字日
	// {
	// 	print(0x80, "World Red-Cross ");
	// 	print(0x40, "      Day       ");
	// }
	// else if (yue == 5 && _ri_ == 12) //护士节
	// {
	// 	print(0x80, " International  ");
	// 	print(0x40, "   Nurse Day    ");
	// }
	// else if (yue == 5 && _ri_ == 5) //母亲节
	// {
	// 	print(0x80, "  Mother's Day  ");
	// 	print(0x40, "   is coming!   ");
	// }
	// else if (yue == 5 && _ri_ == 15) //国际家庭日
	// {
	// 	print(0x80, " International  ");
	// 	print(0x40, "   Family Day   ");
	// }
	// else if (yue == 5 && _ri_ == 31) //世界禁烟日
	// {
	// 	print(0x80, "World No-Smoking");
	// 	print(0x40, "      Day       ");
	// }

	// else if (yue == 6 && _ri_ == 1) //儿童节
	// {
	// 	print(0x80, " International  ");
	// 	print(0x40, " Children's Day ");
	// }										//6
	// else if (yue == 6 && _ri_ == 5) //环境日
	// {
	// 	print(0x80, " International  ");
	// 	print(0x40, "Environment Day ");
	// }
	// else if (yue == 6 && _ri_ == 26) //国际禁毒日
	// {
	// 	print(0x80, " International  ");
	// 	print(0x40, "Against Drug Day");
	// }
	// else if (yue == 6 && _ri_ == 6) //世界爱眼日
	// {
	// 	print(0x80, "  National Eyes ");
	// 	print(0x40, "   Caring Day   ");
	// }
	// else if (yue == 6 && _ri_ == 13) //父亲节
	// {
	// 	print(0x80, "  Father's Day  ");
	// 	print(0x40, "   is coming!   ");
	// }

	// else if (month_moon == 0x07 && day_moon == 0x07) //七夕节
	// {
	// 	print(0x80, " Double-Seventh ");
	// 	print(0x40, "      Day!      ");
	// }

	// else if (yue == 7 && _ri_ == 1) //香港回归日
	// {
	// 	print(0x80, " The return of  ");
	// 	print(0x40, " Hong Kong Day  ");
	// }										//7
	// else if (yue == 7 && _ri_ == 7) //抗日战争纪念日
	// {
	// 	print(0x80, " Anti-Japanese  ");
	// 	print(0x40, "War memorial day");
	// }
	// else if (yue == 7 && _ri_ == 11) //世界人口日
	// {
	// 	print(0x80, "World Population");
	// 	print(0x40, "      Day       ");
	// }

	// else if (month_moon == 0x08 && day_moon == 0x15) //中秋节
	// {
	// 	print(0x80, " the Mid-Autumn ");
	// 	print(0x40, "    Festival    ");
	// }
	// else if (yue == 8 && _ri_ == 1) //建军节
	// {
	// 	print(0x80, "    Today is:   ");
	// 	print(0x40, "  the Army Day  ");
	// }										//8
	// else if (yue == 8 && _ri_ == 8) //中国“爸爸节”
	// {
	// 	print(0x80, "  Chinese man   ");
	// 	print(0x40, "    Festival    ");
	// }
	// else if (yue == 8 && _ri_ == 15) //战争胜利纪念日
	// {
	// 	print(0x80, " The victory of ");
	// 	print(0x40, " war anniversary");
	// }

	// else if (month_moon == 0x09 && day_moon == 0x09) //重阳节
	// {
	// 	print(0x80, "the Double Ninth");
	// 	print(0x40, "    Festival    ");
	// }
	// else if (yue == 9 && _ri_ == 10) //教师节
	// {
	// 	print(0x80, "    Today is:   ");
	// 	print(0x40, " Teacher's Day  ");
	// } //9

	// else if (yue == 9 && _ri_ == 20) //世界爱牙日
	// {
	// 	print(0x80, " International  ");
	// 	print(0x40, "Teeth-loving Day");
	// }
	// else if (yue == 9 && _ri_ == 27) //世界旅游日
	// {
	// 	print(0x80, " World Tourism  ");
	// 	print(0x40, "      Day       ");
	// }

	// else if (yue == 10 && _ri_ == 1) //国庆
	// {
	// 	print(0x40, "    Today is:   ");
	// 	print(0x40, "  National Day  ");
	// } //10

	// else if (yue == 10 && _ri_ == 4) //世界动物日
	// {
	// 	print(0x80, "    Today is:   ");
	// 	print(0x40, "World Animal Day");
	// }
	// else if (yue == 10 && _ri_ == 24) //联合国日
	// {
	// 	print(0x80, " United Nations ");
	// 	print(0x40, "      Day       ");
	// }

	// else if (yue == 11 && _ri_ == 10) //世界青年日
	// {
	// 	print(0x80, "    Today is:   ");
	// 	print(0x40, "World Youth Day ");
	// } //11

	// else if (yue == 11 && _ri_ == 11) //光棍节
	// {
	// 	print(0x80, "Today is 1,1,1..");
	// 	print(0x40, "    One's Day   ");
	// }
	// else if (yue == 11 && _ri_ == 17) //学生节
	// {
	// 	print(0x80, "  The student   ");
	// 	print(0x40, "    section     ");
	// }

	// else if (month_moon == 0x12 && day_moon == 0x08) //腊八节
	// {
	// 	print(0x80, "the laba Rice P-");
	// 	print(0x40, "orridge Festival");
	// }
	// else if (month_moon == 0x12 && day_moon == 0x15) //小年
	// {
	// 	print(0x80, "  guo xiao nian ");
	// 	print(0x40, "  a lunar year  ");
	// }
	// if (month_moon == 0x12 && day_moon == 0x29)
	// {
	// 	print(0x80, "  腊月二十九    ");
	// }
	// else if (month_moon == 0x12 && day_moon == 0x30) //除夕
	// {
	// 	print(0x80, "    Today is:   ");
	// 	print(0x40, " New Year's Eve ");
	// }
	// else if (yue == 12 && _ri_ == 1) //世界艾滋病日
	// {
	// 	print(0x80, "    Today is:   ");
	// 	print(0x40, " World AIDS Day ");
	// } //12

	// if (yue == 12 && _ri_ == 23)
	// {
	// 	print(0x80, "  明晚平安夜    ");
	// }
	// else if (yue == 12 && _ri_ == 24) //平安夜
	// {
	// 	print(0x80, "Tonight is      ");
	// 	print(0x40, "The Silent Night");
	// }
	// else if (yue == 12 && _ri_ == 25) //圣诞节
	// {
	// 	print(0x80, "Merry Christmas!");
	// 	print(0x40, " Christmas Day  ");
	// }
	// else if (yue == 12 && _ri_ == 31)
	// {
	// 	print(0x80, "  The last day  ");
	// 	print(0x40, "  of the year   ");
	// }

	// else
	// {
	// 	print(0x40, " ...(*^_^*)...  "); //显示笑脸。。。
	// 									 //print(0x40," ...0(n_n)0...  ");
	// 	print(0x80, "Have a Good Day!");
	// }
}

/*报时函数*/
void timealarm()
{
	if (shi > 7) //大于7点才启动报时
	{
		if (alarmflag == 1) //不按下设置按键时才有效
		{
			if (fen == 0 && miao < 2)
			{
				BEEP = 0; //启动蜂鸣器
			}
			else if (fen == 30 && miao == 0)
			{
				BEEP = 0;
			}
			else
				BEEP = 1;
		}
	}
}

//按键初始化时间函数
/**
 * @brief button1和button2同时按下时初始化时间
 *
 */
void chushihua() //初始化时间函数
{
	if (BUTTON1 == 0)
	{
		if (BUTTON2 == 0)
		{
			Delay_Ms(5);
			if (BUTTON2 == 0)
			{
				RST = 0;
				SCLK = 0;
				DS1302_Write(0x8e, 0x00); //允许写
				DS1302_Write(0x80, 0x00); //秒：00
				DS1302_Write(0x82, 0x00); //分：00
				DS1302_Write(0x84, 0x12); //时：12
				DS1302_Write(0x8a, 0x06); //周：4
				DS1302_Write(0x86, 0x01); //日：15
				DS1302_Write(0x88, 0x01); //月：3
				DS1302_Write(0x8c, 0x11); //年：11
				DS1302_Write(0x8e, 0x80); //打开保护
			}
		}
	}
}

/*以下是主函数部分*/
void LCD1602calendar()
{
	alarmflag = 1; //alarmflag=1表示允许报时，alarmflag=0表示按下按键，不允许报时

	lcdinit();	 //初始化液晶并写入相关显示数据。注意lcdinit()中已经封装了LCD1602_Init()
	DS1302_Init(); //DS1302时钟芯片初始化函数
	TimerInit();   //定时器初始化函数
	BEEP = 0;
	Delay_Ms(200);
	BEEP = 1;

	while (1)
	{
		year1 = (nian) / 10 * 16 + (nian) % 10; //转换为bcd码
		month1 = (yue) / 10 * 16 + (yue) % 10;
		day1 = (_ri_) / 10 * 16 + (_ri_) % 10;
		chushihua();						//初始化时间函数
		Conversion(0, year1, month1, day1); //
		timealarm();						//报时程序
		keyscan();							//不断扫面按键函数
	}
}

void timer0() interrupt 1 //中断任务：取数据并显示
{
	TH0 = (65536 - 60000) / 256; //重新赋初值
	TL0 = (65536 - 60000) % 256;
	//读取数据
	flag = ReadTemperature();		   //读取温度（考虑是否放在此处）
	miao = turnBCD(DS1302_Read(0x81)); //读出秒
	fen = turnBCD(DS1302_Read(0x83));  //读出分
	shi = turnBCD(DS1302_Read(0x85));  //读出时
	_ri_ = turnBCD(DS1302_Read(0x87)); //读出日
	yue = turnBCD(DS1302_Read(0x89));  //读出月
	nian = turnBCD(DS1302_Read(0x8d)); //读出年
	week = turnBCD(DS1302_Read(0x8b)); //读出周
	//显示数据

	if (((0 <= miao) && (miao < 15)) || (((miao / 10 == 2) || (miao / 10 == 4)) && (miao % 10 <= 5)) || ((54 <= miao) && (miao < 60))) /*显示所有信息的时间段*/
	{
		print(0x80, "20  /  /   W < >");
		LCD1602_Write_Com(h1 + 0x0c);
		LCD1602_Write_Dat(0x7e); //→右箭头
		writeday(8, _ri_);		 //显示日
		writeday(5, yue);		 //显示月
		writeday(2, nian);		 //显示年
		writeweek(week);		 //显示星期
		print(0x40, "  :  :          ");
		writetemp(9, flag); //显示温度，第二行显示
		writetime(6, miao); //显示出秒
		writetime(3, fen);  //显示出分
		writetime(0, shi);  //显示出时，第二行第一个开始
	}

	if (((miao / 10 == 1) || (miao / 10 == 3)) && (miao % 10 >= 5))
	{
		switch (flag / 100) //取出温度的十位数
		{
		case 0:
			print(0x80, "Pretty cold now!");
			break;
		case 1:
			print(0x80, "little cold now!");
			break;
		case 2:
			print(0x80, "It is warm now. ");
			break;
		case 3:
			print(0x80, "It's hot !!!    ");
			break;
		case 4:
			print(0x80, "Very very hot!!!");
			break;
		case 5:
			print(0x80, "Very very hot!!!");
			break;
		case 6:
			print(0x80, "Very very hot!!!");
			break;
		}

		print(0x40, "  :  :          ");
		writetemp(9, flag); //显示温度，第二行显示
		writetime(6, miao); //显示出秒
		writetime(3, fen);  //显示出分
		writetime(0, shi);  //显示出时，第二行第一个开始
	}
	if (((miao / 10 == 2) || (miao / 10 == 4)) && (miao % 10 >= 5)) /*25-30 45-50*/
	{
		if ((5 <= shi) && (shi <= 11))
			flagT = 1; //5-11时为早上
		if ((11 < shi) && (shi <= 14))
			flagT = 2; //11-14为中午
		if ((14 < shi) && (shi <= 18))
			flagT = 3; //14-18为下午
		if ((18 < shi) || (shi < 5))
			flagT = 4; //18-24或者0-5为晚上
		switch (flagT) //分段时间
		{
		case 1:
			print(0x80, " Good Morning!  ");
			break;
		case 2:
			print(0x80, "   Good Noon!   ");
			break;
		case 3:
			print(0x80, "Good afternoon! ");
			break;
		case 4:
			print(0x80, "  Good night!   ");
			break;
		}
		print(0x40, "  :  :          ");
		writetemp(9, flag); //显示温度，第二行显示
		writetime(6, miao); //显示出秒
		writetime(3, fen);  //显示出分
		writetime(0, shi);  //显示出时，第二行第一个开始
	}

	if (((30 <= miao) && (miao < 35)) || ((50 <= miao) && (miao < 54))) //30-35 50-54
	{																	//else改为范围
		festival();														//选择显示节日
	}
	//}
}

void timer1() interrupt 3 //任务：计时用作报时
{
	TH1 = (65536 - 50000) / 256; //中断后重新赋初值
	TL1 = (65536 - 50000) % 256;
	//count++;//计算时间
	if (alarmflag == 1) //设置键按下才有效
	{
		if (fen == 0 && miao < 4)
		{
			count++;
			ledcount = count / 10; //分开时间段
			switch (ledcount)
			{
			case 0:
				Gled = 0;
				Yled = 1;
				break; //从零开始
			case 1:
				Gled = 1;
				Yled = 0;
				break;
			case 2:
				Gled = 0;
				Yled = 1;
				break;
			case 3:
				Gled = 1;
				Yled = 0;
				break;
			case 4:
				Gled = 0;
				Yled = 0;
				break;
			case 5:
				Gled = 1;
				Yled = 1;
				break;
			case 6:
				Gled = 0;
				Yled = 0;
				break;
			case 7:
				Gled = 0;
				Yled = 0;
				count = 0;
				break; //在这里面清零
			}
		}
		else if (fen == 30 && miao < 2)
		{
			Gled = 0;
			Yled = 0;
		}
		else
		{
			Gled = 1;
			Yled = 1;
		}
	}
}