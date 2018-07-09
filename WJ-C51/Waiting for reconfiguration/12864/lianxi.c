/*********************************************************************************************

程序名：	带字库12864LCC电子钟程序

器  件：	STC89C52,LCD12864(内置汉字字库),18B20温度芯片,1302时钟芯片	MCU晶振：12MHZ

设  计：     佳文电子设计

功  能：     显示公历年月日
		     农历年月日
			 星期(星期不调用DS1302内的数据，二十根据公历年月日推算的)
			 温度
			 时分秒
			 公历重大节日
			 农历纪念日与节日（用于提醒农历生日非常实用）
 注：1、当农历纪念日与公历纪念日碰到同一天，则按一秒的间隔轮流显示
     2、20100226加入阳历大小月和平闰年判断，调日历时自动控制每个月份的天数
/*********************************************************************************************

/*********************************************************************************************/

//程序部分开始//
/****************************************************************************/
//头文件
#include <AT89X52.h>
#include <string.h>
/*****************************************************************************/

//定做PCB板的灯与扬声器定义
sbit led_back = P1 ^ 7; //背光控制（V2.1程序中加入了背光控制的硬件电路）
sbit beep = P2 ^ 3;		//扬声器

/*****************************************************************************/

//定做PCB板温度传感器引脚定义
sbit DQ = P2 ^ 2; //ds18B20*/
/*****************************************************************************/

//键盘引脚定义
sbit KEY_1 = P3 ^ 2; //退出设置键，显示版本信息键
sbit KEY_2 = P3 ^ 1; //上调键
sbit KEY_3 = P3 ^ 0; //设置键
sbit KEY_4 = P3 ^ 3; //减，下调键
sbit KEY_5 = P1 ^ 4; //LED控制每次按动改变背光开关

/****************************************************************************/

//LCD接口定义
sbit RS = P3 ^ 5;  //模式位，为0输入指令，为1输入数据
sbit RW = P3 ^ 6;  //读写位，为0读，为1写
sbit E = P3 ^ 4;   //使能位
#define Lcd_Bus P0 //数据总线
/*****************************************************************************/

//定义DS1302时钟接口
//开发板上DS1302需要用杜邦线接到下列引脚，同时流水灯跳线帽取下来
sbit clock_clk = P1 ^ 2; //ds1302_clk（时钟线）
sbit clock_dat = P1 ^ 1; //ds1302_dat（数据线）
sbit clock_Rst = P1 ^ 0; //ds1302_Rst（复位线）
/*****************************************************************************/
//定义累加器A中的各位
sbit a0 = ACC ^ 0;
sbit a1 = ACC ^ 1;
sbit a2 = ACC ^ 2;
sbit a3 = ACC ^ 3;
sbit a4 = ACC ^ 4;
sbit a5 = ACC ^ 5;
sbit a6 = ACC ^ 6;
sbit a7 = ACC ^ 7;
/****************************************************************************/
//定义全局变量
unsigned char yy, mo, dd, xq, hh, mm, ss, month_moon, day_moon, week, tiangan, dizhi, moontemp1, moontemp2; //定义时间映射全局变量（专用寄存器）
bit w = 0;																									//调时标志位，=0时为走时状态，=1时为调时状态
static unsigned char menu = 0;																				//定义静态小时更新用数据变量
static unsigned char keys = 0;																				//定义静态小时更新用数据变量
static unsigned char timecount = 0;																			//定义静态软件计数器变量

signed char address, item, max, mini;

/****************************************************************************/
void DelayM(unsigned int a)
{ //延时函数 1MS/次
	unsigned char i;
	while (--a != 0)
	{
		for (i = 0; i < 125; i++)
			; //一个 ; 表示空语句,CPU空转。
	}		  //i 从0加到125，CPU大概就耗时1毫秒
}
/****************************************************************************/
//生日歌

/******************************************************************************/
//LCD驱动（DY12864CBL液晶显示屏并口驱动程序）
/******************************************************************************/
void chk_busy()
{ //检查忙位（底层）
	RS = 0;
	RW = 1;
	E = 1;
	Lcd_Bus = 0xff;
	while ((Lcd_Bus & 0x80) == 0x80)
		;
	E = 0;
}
/******************************************************************************/
void write_com(unsigned char cmdcode)
{ //写命令到LCD（底层）
	chk_busy();
	RS = 0;
	RW = 0;
	E = 1;
	Lcd_Bus = cmdcode;
	E = 0;
}
/******************************************************************************/
void write_data(unsigned char Dispdata)
{ //写数据到LCD（底层）
	chk_busy();
	RS = 1;
	RW = 0;
	E = 1;
	Lcd_Bus = Dispdata;
	E = 0;
}
/******************************************************************************/
void lcm_init()
{					 //初始化LCD屏（被调用层）
	write_com(0x30); //选择8bit数据流
	write_com(0x0c); //开显示(无游标、不反白)
	write_com(0x01); //清除显示，并且设定地址指针为00H
}
/*****************************************************************************/
void lcm_w_word(unsigned char *s)
{ //向LCM发送一个字符串,长度64字符（一行）之内。（被调用层）
	while (*s > 0)
	{ //应用：lcm_w_word("您好！")，直接显示：您好！
		write_data(*s);
		s++;
	}
}
/******************************************************************************/
void lcm_w_test(bit i, unsigned char word)
{ //写指令或数据（被调用层）
	if (i == 0)
	{
		write_com(word); //写指令或数据（0，指令）
	}
	else
	{
		write_data(word); //写指令或数据（1，数据）
	}
}
/******************************************************************************/
void lcm_clr(void)
{ //清屏函数
	lcm_w_test(0, 0x01);
}
/******************************************************************************/

void lcm_clr2(void)
{						 //清屏上面3行（用空格填满要清显示的地方，因为液晶屏是静态显示的，所以这办法常用）
	lcm_w_test(0, 0x80); //第一行
	lcm_w_word("                ");
	//标尺("1234567812345678"应该能够显示满一行)
	lcm_w_test(0, 0x90); //第二行
	lcm_w_word("                ");
	//标尺("1234567812345678"应该能够显示满一行)
	lcm_w_test(0, 0x88); //第一行
	lcm_w_word("                ");
	//标尺("1234567812345678"应该能够显示满一行)
}

/******************************************************************************/
//DS18B20温度传感器驱动（显示0至60度，小数点后显示一位）
/******************************************************************************/
void Delay(int num)
{ //延时函数
	while (num--)
		;
}
/******************************************************************************/
void Init_DS18B20(void)
{ //初始化ds1820
	unsigned char x = 0;
	DQ = 1;	//DQ复位
	Delay(8);  //稍做延时
	DQ = 0;	//单片机将DQ拉低
	Delay(80); //精确延时 大于 480us
	DQ = 1;	//拉高总线
	Delay(14);
	x = DQ; //稍做延时后 如果x=0则初始化成功 x=1则初始化失败
	Delay(20);
}
/******************************************************************************/
unsigned char ReadOneChar(void)
{ //读一个字节
	unsigned char i = 0;
	unsigned char dat = 0;
	for (i = 8; i > 0; i--)
	{
		DQ = 0; // 给脉冲信号
		dat >>= 1;
		DQ = 1; // 给脉冲信号
		if (DQ)
			dat |= 0x80;
		Delay(4);
	}
	return (dat);
}
/******************************************************************************/
void WriteOneChar(unsigned char dat)
{ //写一个字节
	unsigned char i = 0;
	for (i = 8; i > 0; i--)
	{
		DQ = 0;
		DQ = dat & 0x01;
		Delay(5);
		DQ = 1;
		dat >>= 1;
	}
}
/******************************************************************************/
unsigned int ReadTemperature(void)
{ //读取温度，温度值返回到此函数中，供调用
	unsigned char a = 0;
	unsigned char b = 0;
	unsigned int t = 0;
	float tt = 0;
	Init_DS18B20();
	WriteOneChar(0xCC); // 跳过读序号列号的操作
	WriteOneChar(0x44); // 启动温度转换
	Init_DS18B20();
	WriteOneChar(0xCC); //跳过读序号列号的操作
	WriteOneChar(0xBE); //读取温度寄存器
	a = ReadOneChar();  //读低8位
	b = ReadOneChar();  //读高8位
	t = b;
	t <<= 8;
	t = t | a;
	tt = t * 0.0625;
	t = tt * 10 + 0.5; //放大10倍输出并四舍五入
	return (t);		   //t内的温度值，返回到子函数中，要将温度传输到temp变量，应用temp=ReabTemperature()
}
/*****************************************************************************/
//后面会出现的函数声明（当各函数的排列适当时可不用声明，加入函数声明就不用管各个子函数出现的位置先后了？）
void lcm_w_ss(void);						 //向液晶LCM中写秒数据
void lcm_w_mm(void);						 //向液晶LCM中写分数据
void lcm_w_hh(void);						 //向液晶LCM中写小时数据
void lcm_w_dd(void);						 //写日数据
void lcm_w_mo(void);						 //写月数据
void lcm_w_yy(void);						 //写年数据
void lcm_w_xq(void);						 //星期处理并送入LCM的指定区域
unsigned char clock_in(void);				 //DS1302逐位写入字节（底层协议）
void clock_out(unsigned char dd);			 //1302驱动程序逐位读出字节（底层协议?
void Init_1302(void);						 //-设置1302的初始时间（2007年1月1日00时00分00秒星期一）
unsigned char read_clock(unsigned char ord); //DS1302读数据（底层协议）
void read_clockS(void);
void Set_time(unsigned char sel);					   //根据选择调整的相应项目sel加1并写入DS1302
void write_clock(unsigned char ord, unsigned char dd); //DS1302写数据（底层协议）
void updata(void);

void lcm_w_nl(void);																 //显示农历子函数
void Conversion(bit c, unsigned char year, unsigned char month, unsigned char day);  //农历年月日计算子函数
void Conver_week(bit c, unsigned char year, unsigned char month, unsigned char day); //星期计算子函数
void ds_w(void);

/*****************************************************************************/
//设置1302的初始时间（自动初始化）
void Init_1302(void)
{ //-设置1302的初始时间（2007年1月1日00时00分00秒星期一）
	unsigned char f;
	if (read_clock(0xc1) != 0xaa)
	{
		write_clock(0x8e, 0x00); //允许写操作
		write_clock(0x8c, 0x11); //年
		write_clock(0x8a, 0x00); //星期
		write_clock(0x88, 0x05); //月
		write_clock(0x86, 0x29); //日
		write_clock(0x84, 0x19); //小时
		write_clock(0x82, 0x55); //分钟
		write_clock(0x80, 0x23); //秒
		write_clock(0x90, 0xa5); //充电
		write_clock(0xc0, 0xaa); //写入初始化标志RAM（第00个RAM位置）
		for (f = 0; f < 60; f = f + 2)
		{ //清除闹钟RAM位为0
			write_clock(0xc2 + f, 0x00);
		}
		write_clock(0x8e, 0x80); //禁止写操作
	}
}
/*****************************************************************************/
//DS1302写数据（底层协议）
void write_clock(unsigned char ord, unsigned char dd)
{
	clock_clk = 0;
	clock_Rst = 0;
	clock_Rst = 1;
	clock_out(ord);
	clock_out(dd);
	clock_Rst = 0;
	clock_clk = 1;
}
/*****************************************************************************/
//1302驱动程序逐位读出字节（底层协议）
void clock_out(unsigned char dd)
{
	ACC = dd;
	clock_dat = a0;
	clock_clk = 1;
	clock_clk = 0;
	clock_dat = a1;
	clock_clk = 1;
	clock_clk = 0;
	clock_dat = a2;
	clock_clk = 1;
	clock_clk = 0;
	clock_dat = a3;
	clock_clk = 1;
	clock_clk = 0;
	clock_dat = a4;
	clock_clk = 1;
	clock_clk = 0;
	clock_dat = a5;
	clock_clk = 1;
	clock_clk = 0;
	clock_dat = a6;
	clock_clk = 1;
	clock_clk = 0;
	clock_dat = a7;
	clock_clk = 1;
	clock_clk = 0;
}
/*****************************************************************************/
//DS1302逐位写入字节（底层协议）
unsigned char clock_in(void)
{
	clock_dat = 1;
	a0 = clock_dat;
	clock_clk = 1;
	clock_clk = 0;
	a1 = clock_dat;
	clock_clk = 1;
	clock_clk = 0;
	a2 = clock_dat;
	clock_clk = 1;
	clock_clk = 0;
	a3 = clock_dat;
	clock_clk = 1;
	clock_clk = 0;
	a4 = clock_dat;
	clock_clk = 1;
	clock_clk = 0;
	a5 = clock_dat;
	clock_clk = 1;
	clock_clk = 0;
	a6 = clock_dat;
	clock_clk = 1;
	clock_clk = 0;
	a7 = clock_dat;
	return (ACC);
}
/*****************************************************************************/
//DS1302读数据（底层协议）供调用时钟数据
unsigned char read_clock(unsigned char ord)
{
	unsigned char dd = 0;
	clock_clk = 0;
	clock_Rst = 0;
	clock_Rst = 1;
	clock_out(ord);
	dd = clock_in();
	clock_Rst = 0;
	clock_clk = 1;
	return (dd);
}

/*****************************************************************************/
//扬声器驱动程序（闹钟音乐）
/*****************************************************************************/
void Beep(void)
{					 //BELL-扬声器--整点报时
	unsigned char a; //定义变量用于发声的长度设置
	for (a = 60; a > 0; a--)
	{				  //第一个声音的长度
		beep = ~beep; //取反扬声器驱动口，以产生音频
		Delay(100);   //音调设置延时
	}
	for (a = 100; a > 0; a--)
	{ //同上
		beep = ~beep;
		Delay(80); //
	}
	for (a = 100; a > 0; a--)
	{ //同上
		beep = ~beep;
		Delay(30); //
	}
	beep = 1; //音乐结束后扬声器输出0关闭，因为后端用9014驱动，所以控制输出1时蜂鸣响
}
/*****************************************************************************/
void Beep_set(void)
{					 //BELL -扬声器--确定设置
	unsigned char a; //定义变量用于发声的长度设置
	for (a = 50; a > 0; a--)
	{				  //第一个声音的长度
		beep = ~beep; //取反扬声器驱动口，以产生音频
		Delay(100);   //音调设置延时
	}
	for (a = 100; a > 0; a--)
	{ //同上
		beep = ~beep;
		Delay(50); //
	}
	for (a = 50; a > 0; a--)
	{ //同上
		beep = ~beep;
		Delay(100); //
	}
	beep = 1; //音乐结束后扬声器输出0关闭，因为后端用9014反相驱动，所以控制输出0时蜂鸣响
}
/*****************************************************************************/
void Beep_key(void)
{					 //-扬声器--按键音
	unsigned char a; //定义变量用于发声的长度设置
	for (a = 100; a > 0; a--)
	{ //声音的长度
		beep = ~beep;
		Delay(50); //音调设置延时
	}
	beep = 1; //音乐结束后扬声器拉高关闭
}

/******************************************************************************/
//农历节日数据库表，农历纪念日数据表，加上公历的重大节日
/******************************************************************************/

void jie_nl(void)
{ //

	dd = read_clock(0x87);	 //日
	mo = read_clock(0x89);	 //月
	yy = read_clock(0x8d);	 //年
	Conversion(0, yy, mo, dd); //调用公历转农历子函数
							   //调用函数后,原有数据不变,读c_moon农历世纪,year_moon农历年,month_moon,day_moon得出阴历BCD数据

	lcm_w_test(0, 0x98); //在屏幕第四行显示节日（下面的注释行用坐标尺，不要改动）
						 //农历节日：
						 //											                           ----|----------------|标尺，汉字不要对在空隙上
	if (month_moon == 0x12 && day_moon == 0x29)
	{
		lcm_w_word("  明天是春节！！");
	}
	if (month_moon == 0x12 && day_moon == 0x30)
	{
		lcm_w_word("  今天是春节！！");
	}
	if (month_moon == 0x01 && day_moon == 0x01)
	{
		lcm_w_word("大年初一拜年啦  ");
	}
	if (month_moon == 0x01 && day_moon == 0x14)
	{
		lcm_w_word("  明天元宵节！  ");
	}
	if (month_moon == 0x01 && day_moon == 0x15)
	{
		lcm_w_word("  元宵节到啦!   ");
	}
	if (month_moon == 0x05 && day_moon == 0x04)
	{
		lcm_w_word("    明天端午    ");
	}
	if (month_moon == 0x05 && day_moon == 0x05)
	{
		lcm_w_word("    今天端午    ");
	}
	if (month_moon == 0x07 && day_moon == 0x06)
	{
		lcm_w_word("  明天七月七啦  ");
	}
	if (month_moon == 0x07 && day_moon == 0x07)
	{
		lcm_w_word("牛郎织女鹊桥相会");
	}
	if (month_moon == 0x08 && day_moon == 0x14)
	{
		lcm_w_word("  明天仲秋节    ");
	}
	if (month_moon == 0x08 && day_moon == 0x15)
	{
		lcm_w_word("    仲秋佳节    ");
	}
	if (month_moon == 0x09 && day_moon == 0x08)
	{
		lcm_w_word("    明天重阳    ");
	}
	if (month_moon == 0x09 && day_moon == 0x09)
	{
		lcm_w_word("    今天重阳节  ");
	}
	if (month_moon == 0x12 && day_moon == 0x07)
	{
		lcm_w_word("    明天腊八    ");
	}
	if (month_moon == 0x12 && day_moon == 0x08)
	{
		lcm_w_word("    今天腊八    ");
	}
	if (month_moon == 0x12 && day_moon == 0x22)
	{
		lcm_w_word("    明日辞灶    ");
	}
	if (month_moon == 0x12 && day_moon == 0x23)
	{
		lcm_w_word("    今日辞灶    ");
	}
	if (month_moon == 0x06 && day_moon == 0x06)
	{
		lcm_w_word("  明天许靓生日  ");
	}
	if (month_moon == 0x06 && day_moon == 0x07)
	{
		lcm_w_word("  祝许靓生日快乐");
	}
	if (month_moon == 0x09 && day_moon == 0x19)
	{
		lcm_w_word("  明天爸爸生日  ");
	}
	if (month_moon == 0x09 && day_moon == 0x20)
	{
		lcm_w_word("  祝爸爸生日快乐");
	}
	if (month_moon == 0x10 && day_moon == 0x13)
	{
		lcm_w_word("  明天妈妈生日  ");
	}
	if (month_moon == 0x10 && day_moon == 0x14)
	{
		lcm_w_word("  祝妈妈生日快乐");
	}
	if (month_moon == 0x10 && day_moon == 0x03)
	{
		lcm_w_word("  明天刘佳文生日");
	}
	if (month_moon == 0x10 && day_moon == 0x04)
	{
		lcm_w_word("祝刘佳文生日快乐");
	}
}

void jie_gl(void)
{ //

	dd = read_clock(0x87); //日
	mo = read_clock(0x89); //月
	yy = read_clock(0x8d); //年
						   //Conversion(0,yy,mo,dd);//调用公历转农历子函数
						   //调用函数后,原有数据不变,读c_moon农历世纪,year_moon农历年,month_moon,day_moon得出阴历BCD数据

	lcm_w_test(0, 0x98); //在屏幕第四行显示节日（下面的注释行用坐标尺，不要改动）

	//下面是公历的，有些没意思的，就不显示了
	if (mo == 0x01 && dd == 0x01)
	{
		lcm_w_word(" Happy new year!");
	}
	// ( mo == 0x01 && dd == 0x22 ){ lcm_w_word("    节日实验    "); }//2010-01-22是农历腊八节，用于实验轮流显示
	if (mo == 0x02 && dd == 0x13)
	{
		lcm_w_word("  明天情人节了  ");
	}
	if (mo == 0x02 && dd == 0x14)
	{
		lcm_w_word("  今天是情人节  ");
	}
	if (mo == 0x03 && dd == 0x08)
	{
		lcm_w_word("   3.8妇女节    ");
	}
	if (mo == 0x03 && dd == 0x11)
	{
		lcm_w_word("  明天是植树节  ");
	}
	if (mo == 0x03 && dd == 0x12)
	{
		lcm_w_word("    植树节      ");
	}
	if (mo == 0x03 && dd == 0x14)
	{
		lcm_w_word("明天消费者权益日");
	}
	if (mo == 0x03 && dd == 0x15)
	{
		lcm_w_word("  消费者权益日  ");
	}
	if (mo == 0x04 && dd == 0x01)
	{
		lcm_w_word("愚人节  小心上当");
	}
	//if ( mo == 0x04 && dd == 0x07 ){ lcm_w_word("  世界卫生日    "); }
	if (mo == 0x04 && dd == 0x08)
	{
		lcm_w_word("    复活节      ");
	}
	//if ( mo == 0x04 && dd == 0x13 ){ lcm_w_word("  黑色星期五    "); }

	if (mo == 0x05 && dd == 0x01)
	{
		lcm_w_word("  劳动节  放假  ");
	} //5
	if (mo == 0x05 && dd == 0x04)
	{
		lcm_w_word("    青年节      ");
	}
	if (mo == 0x05 && dd == 0x08)
	{
		lcm_w_word("  世界红十字日  ");
	}
	if (mo == 0x05 && dd == 0x12)
	{
		lcm_w_word("  国际护士节    ");
	}
	if (mo == 0x05 && dd == 0x05)
	{
		lcm_w_word("近日注意母亲节  ");
	}
	if (mo == 0x05 && dd == 0x06)
	{
		lcm_w_word("近日注意母亲节  ");
	}
	//if ( mo == 0x05 && dd == 0x15 ){ lcm_w_word("  国际家庭日    "); }
	if (mo == 0x05 && dd == 0x31)
	{
		lcm_w_word("  明天儿童节    ");
	}

	if (mo == 0x06 && dd == 0x01)
	{
		lcm_w_word("月月儿童节快乐! ");
	} //6
	if (mo == 0x06 && dd == 0x05)
	{
		lcm_w_word("  世界环境日    ");
	}
	//if ( mo == 0x06 && dd == 0x26 ){ lcm_w_word("  国际禁毒日    "); }
	//if ( mo == 0x06 && dd == 0x06 ){ lcm_w_word("  全国爱眼日    "); }
	if (mo == 0x06 && dd == 0x13)
	{
		lcm_w_word("近日注意父亲节  ");
	}
	if (mo == 0x06 && dd == 0x15)
	{
		lcm_w_word("近日注意父亲节  ");
	}

	if (mo == 0x07 && dd == 0x01)
	{
		lcm_w_word("香港回归记念日  ");
	} //7
	//if ( mo == 0x07 && dd == 0x07 ){ lcm_w_word("抗日战争记念日  "); }
	//if ( mo == 0x07 && dd == 0x11 ){ lcm_w_word("  世界人口日    "); }

	if (mo == 0x08 && dd == 0x01)
	{
		lcm_w_word("  八一建军节    ");
	} //8
	  //	if ( mo == 0x08 && dd == 0x08 ){ lcm_w_word("  中国男子节    "); }
	if (mo == 0x08 && dd == 0x15)
	{
		lcm_w_word("抗战胜利记念日  ");
	}
	if (mo == 0x09 && dd == 0x09)
	{
		lcm_w_word("  明天教师节    ");
	}
	if (mo == 0x09 && dd == 0x10)
	{
		lcm_w_word("  中国教师节    ");
	} //9
	if (mo == 0x09 && dd == 0x18)
	{
		lcm_w_word("九一八事变纪念  ");
	}
	if (mo == 0x09 && dd == 0x20)
	{
		lcm_w_word("  国际爱牙日    ");
	}
	if (mo == 0x09 && dd == 0x27)
	{
		lcm_w_word("  世界旅游日    ");
	}

	if (mo == 0x10 && dd == 0x01)
	{
		lcm_w_word("  中国国庆节    ");
	} //10
	  //if ( mo == 0x10 && dd == 0x04 ){ lcm_w_word("  世界动物日    "); }
	if (mo == 0x10 && dd == 0x24)
	{
		lcm_w_word("    联合国日    ");
	}
	//if ( mo == 0x10 && dd == 0x12 ){ lcm_w_word("明天国际教师节  "); }
	//if ( mo == 0x10 && dd == 0x13 ){ lcm_w_word("  国际教师节    "); }

	//if ( mo == 0x11 && dd == 0x10 ){ lcm_w_word("  世界青年节    "); }//11
	//	if ( mo == 0x11 && dd == 0x17 ){ lcm_w_word("  世界学生节    "); }

	//	if ( mo == 0x12 && dd == 0x01 ){ lcm_w_word("  世界艾滋病日  "); }//12
	if (mo == 0x12 && dd == 0x23)
	{
		lcm_w_word("  明晚平安夜    ");
	}
	if (mo == 0x12 && dd == 0x24)
	{
		lcm_w_word("  今晚平安夜    ");
	}
	if (mo == 0x12 && dd == 0x25)
	{
		lcm_w_word("  今天圣诞节    ");
	}
	if (mo == 0x12 && dd == 0x31)
	{
		lcm_w_word("    明日元旦    ");
	}

	else
	{ //非节日时显示时晨信息，判断小时数据范围，调取显示
		if (hh >= 0x04 && hh < 0x06)
		{
			lcm_w_word("★★__▲▲__凌晨");
		}
		if (hh >= 0x06 && hh < 0x08)
		{
			lcm_w_word("☆○__▲△__早晨");
		}
		if (hh >= 0x08 && hh < 0x12)
		{
			lcm_w_word("__●__▲▲__上午");
		}
		if (hh == 0x12)
		{
			lcm_w_word("____▲●▲__中午");
		}
		if (hh >= 0x13 && hh < 0x18)
		{
			lcm_w_word("__▲▲__●__下午");
		}
		if (hh >= 0x18 && hh < 0x22)
		{
			lcm_w_word("△▲__●☆__晚上");
		}
		if (hh >= 0x22 && hh <= 0x23)
		{
			lcm_w_word("△▲__★☆__夜里");
		}
		if (hh >= 0x00 && hh < 0x04)
		{
			lcm_w_word("__★▲▲★__深夜");
		}
	}
}
/*****************************************************************************/
//电子钟应用层程序设计
/*****************************************************************************/
//向LCM中填写 年 数据
void lcm_w_yy(void)
{

	yy = read_clock(0x8d); //调用1302时钟数据中的年数据，从地址0x8d中
	lcm_w_test(0, 0x80);
	lcm_w_word("20");				 //显示内容字符20
	lcm_w_test(1, (yy / 16) + 0x30); //函数参数1，代表本行写数据，YY/16+0X30得出年十位数字的显示码地址，送显示
	lcm_w_test(1, yy % 16 + 0x30);   //函数参数1，代表本行写数据，YY%得出年个位数字的显示码地址，送显示
									 //因为年数据两位数字如果是09，C51默认是16进制数据，所以要用16来分离十位、个位。
}

void lcm_w_yy2(void)
{						   //调年时的显示程序
	yy = read_clock(0x8d); //调用1302时钟数据中的年数据，从地址0x8d中
	lcm_w_test(0, 0x93);
	lcm_w_word("20");				 //显示内容字符20
	lcm_w_test(1, (yy / 16) + 0x30); //函数参数1，代表本行写数据，YY/16+0X30得出年十位数字的显示码地址，送显示
	lcm_w_test(1, yy % 16 + 0x30);   //函数?
	lcm_w_word("  年");
}

/*****************************************************************************/
//向LCM中填写 月 数据
void lcm_w_mo(void)
{
	mo = read_clock(0x89); //调用1302时钟数据中的月数据，从地址0x89中
	lcm_w_test(0, 0x82);
	lcm_w_word("-");
	lcm_w_test(1, (mo / 16) + 0x30);
	lcm_w_test(1, mo % 16 + 0x30); //与16取余数，得到月份的个位数，加0x30得到该数字的液晶内定显示码送显示
	lcm_w_word("-");			   //调用字符显示函数，显示文字 月
}

//向LCM中填写调月时 月 数据显示格式（调月时调用显示格式）
void lcm_w_mo2(void)
{
	mo = read_clock(0x89); //调用1302时钟数据中的月数据，从地址0x89中
	lcm_w_test(0, 0x93);
	lcm_w_word("  ");
	lcm_w_test(1, (mo / 16) + 0x30);
	lcm_w_test(1, mo % 16 + 0x30); //与16取余数，得到月份的个位数，加0x30得到该数字的液晶内定显示码送显示
	lcm_w_word("  月");			   //调用字符显示函数，显示文字 月
}

/*****************************************************************************/
//向LCM中填写 日 数据
void lcm_w_dd(void)
{

	dd = read_clock(0x87); //从1302芯片中读取日数据，从地址0x87中
	lcm_w_test(0, 0x84);
	//if(dd/16 != 0){lcm_w_test(1,(dd/16)+0x30);}	//如果十位数不是0，就换算显示
	//else{lcm_w_test(1,0x20);}//如果十位数是0，则显示个空格，不显示数据
	lcm_w_test(1, (dd / 16) + 0x30);
	lcm_w_test(1, dd % 16 + 0x30); //第一个1参数，表示本行写数据，日数据与16取余得个位数，加0x30得到显示码
}

//向LCM中填写调日期时的 日 数据显示格式（调日时调用的显示格式）
void lcm_w_dd2(void)
{

	dd = read_clock(0x87); //从1302芯片中读取日数据，从地址0x87中
	lcm_w_test(0, 0x93);
	lcm_w_word("  ");
	lcm_w_test(1, (dd / 16) + 0x30);
	lcm_w_test(1, dd % 16 + 0x30); //第一个1参数，表示本行写数据，日数据与16取余得个位数，加0x30得到显示码
	lcm_w_word("  日");			   //显示字符 日
}

//}/*****************************************************************************/
//星期处理并送入LCM的指定区域
void lcm_w_xq(void)
{

	unsigned char sel, dd, mo, yy;

	dd = read_clock(0x87);		//日
	mo = read_clock(0x89);		//月
	yy = read_clock(0x8d);		//年
	Conver_week(0, yy, mo, dd); //调用公历换算星期子函数

	sel = week;			 //week是公历转换星期子函数的运行结果，结果为0-6，0是星期日
	lcm_w_test(0, 0x87); //显示位置
	if (sel == 0)
	{
		lcm_w_word("日");
	} //0=星期日
	if (sel == 6)
	{
		lcm_w_word("六");
	} //
	if (sel == 5)
	{
		lcm_w_word("五");
	} //
	if (sel == 4)
	{
		lcm_w_word("四");
	} //
	//if(sel==3)  {lcm_w_word("三");}经调试发现液晶屏不认这一句，只好用下一句显示 三
	if (sel == 3)
	{
		lcm_w_test(1, 0xc8);
		lcm_w_test(1, 0xfd);
	} //此指令等于lcm_w_word("三"); 因为“三”的内码失效。
	if (sel == 2)
	{
		lcm_w_word("二");
	} //
	if (sel == 1)
	{
		lcm_w_word("一");
	} //星期一
	lcm_w_test(0, 0x85);
	lcm_w_word("星期"); //调用字符显示子函数，显示 星期 两个字
}

/*****************************************************************************/
//向LCM中填写 小时 数据
void lcm_w_hh(void)
{
	if (read_clock(0x85) != hh)
	{						   //如果程序中的小时与1302芯片中的不同，
		hh = read_clock(0x85); //刷新程序中的小时数据
		if (hh > 0x07 && hh < 0x22 && w == 0)
		{			//整点判断
			Beep(); //调用一次整点报时音
		}
	}
	lcm_w_test(0, 0x90);			 //第一个参数0，表示本行写入LCM的是指令，指定显示位置88H（第三行左端）
	lcm_w_test(1, (hh / 16) + 0x30); //显示十位
	lcm_w_test(1, hh % 16 + 0x30);   //显示个位
}

//向LCM中填写 小时 数据（用于调整时间时调用）
void lcm_w_hh2(void)
{
	if (read_clock(0x85) != hh)
	{						   //如果程序中的小时与1302芯片中的不同，
		hh = read_clock(0x85); //刷新程序中的小时数据
		if (hh > 0x07 && hh < 0x22 && w == 0)
		{			//整点判断
			Beep(); //调用一次整点报时音
		}
	}
	lcm_w_test(0, 0x93); //第一个参数0，表示本行写入LCM的是指令，指定显示位置88H（第三行左端）
	lcm_w_word("  ");
	lcm_w_test(1, (hh / 16) + 0x30); //显示十位
	lcm_w_test(1, hh % 16 + 0x30);   //显示个位
	lcm_w_word("  时");
}
/*****************************************************************************/
//向LCM中填写 分钟 数据
void lcm_w_mm(void)
{
	if (read_clock(0x83) != mm)
	{						   //如果1302芯片中的分钟数据与程序中的分钟变量不相等
		mm = read_clock(0x83); //刷新程序中的分钟数据
	}
	//	if(w == 0){
	//	jie_nl();//调用节日显示子函数
	// DelayM(1000);
	// jie_gl();
	//	}
	lcm_w_test(0, 0x91);			 //写指令，指定显示位置为89H（第二行的第二个字符）
	lcm_w_test(1, 0x3a);			 //":"写数据，显示冒号，采用直接指定冒号的液晶内定显示码的方法
	lcm_w_test(1, (mm / 16) + 0x30); //向液晶写数据，显示分钟的十位数
	lcm_w_test(1, mm % 16 + 0x30);   //向液晶写数据，显示分钟的个位数
	lcm_w_test(1, 0x3a);			 //":"写数据，显示冒号，采用直接指定冒号的液晶内定显示码的方法
}

void lcm_w_mm2(void)
{
	if (read_clock(0x83) != mm)
	{						   //如果1302芯片中的分钟数据与程序中的分钟变量不相等
		mm = read_clock(0x83); //刷新程序中的分钟数据
	}

	lcm_w_test(0, 0x93); //写指令，指定显示位置为89H（第二行的第二个字符）
	lcm_w_word("  ");
	lcm_w_test(1, (mm / 16) + 0x30); //向液晶写数据，显示分钟的十位数
	lcm_w_test(1, mm % 16 + 0x30);   //向液晶写数据，显示分钟的个位数
	lcm_w_word("  分");
}

/*****************************************************************************/
//向LCM中填写 秒 数据 和温度数据
void lcm_w_ss(void)
{
	unsigned int i = 0;
	unsigned char a = 0, b = 0, c = 0;
	if (read_clock(0x81) != ss)
	{									 //判断是否需要更新，如果1302芯片中的秒数据与程序秒变量不同
		ss = read_clock(0x81);			 //更新程序中的秒数据
		lcm_w_test(0, 0x93);			 //指定秒值在LCM上的显示位置是8BH
		lcm_w_test(1, (ss / 16) + 0x30); //显示秒十位
		lcm_w_test(1, ss % 16 + 0x30);   //显示秒个位(除以16，取余数)

		updata(); //刷新数据子函数

		lcm_w_test(0, 0x95);	  //指定温度在LCM上的显示位置为8DH
		i = ReadTemperature();	//读温度并送显，调用了18B20的读取温度函数返回值，共3位，含十分之一位数字
		a = i / 100;			  //得到十位上的数字
		lcm_w_test(1, a + 0x30);  //参数1，表示该行写数据，换算显示温度十位数字的显示码
		b = i / 10 - a * 10;	  //处理得到第二位数字（个位）
		lcm_w_test(1, b + 0x30);  //参数1，表示该行写的是数据，显示温度个位数字
		lcm_w_test(1, 0x2e);	  //"."参数1，表示该行写的数据，显示一个小数点
		c = i - a * 100 - b * 10; //取得温度的第三位（最末尾，也就是小数点后的一位数字
		lcm_w_test(1, c + 0x30);  //换算小数点后的一位数字的显示码（加0x30），并显示
		lcm_w_word("℃");		  //调用字符显示函数，显示摄氏度的符号
	}
}

void jie(void) //公历和农历节日按照一秒间隔轮换显示（节日显示）
{
	ss = read_clock(0x81);
	if (ss % 16 == 1 | ss % 16 == 3 | ss % 16 == 5 | ss % 16 == 7 | ss % 16 == 9)
	{
		jie_nl();
	} //秒是奇数调用农历节日显示子函数
	if (ss % 16 == 2 | ss % 16 == 4 | ss % 16 == 6 | ss % 16 == 8 | ss % 16 == 0)
	{
		jie_gl();
	} //秒是偶数数，调用公历节日显示子函数
}

/*****************************************************************************/
//刷新数据子函数，供调用，在这段程序之前已经调用过
void updata(void)
{
	lcm_w_mm(); //刷新 分
	lcm_w_hh(); //刷新 小时
	lcm_w_dd(); //刷新 日
	lcm_w_xq(); //更新星期值
	lcm_w_mo(); //刷新 月
	lcm_w_yy(); //刷新 年
	lcm_w_nl(); //刷新农历
	jie();		//刷新节日显示
}
/*****************************************************************************/
void welcome(void)
{ //开机欢迎显示内容，标尺不能改动，文字不要对准横线之间的空隙

	lcm_w_word("佳文电子欢迎你    刘佳文制作                                       "); //
																					   //----------|-------1-------|-------3-------|-------2-------|-------4-------|-----//标尺
}
//标尺能提示文字显示的位置在哪一行，是否在中心

/*****************************************************************************/
//调整时间子函数，设置键、数据范围、上调加一，下调减一功能。
void Set_time(unsigned char sel)
{ //根据选择调整的相应项目加1并写入DS1302，函数参数是按动设置键的次数

	lcm_w_test(0, 0x9b); //第一参数0表示本行写入指令，指定下面行的 调整 显示起始位置为9AH
	lcm_w_word("★调整"); //调用字符显示函数，显示 调整字样

	//if(sel==6)  {lcm_w_word("秒钟");address=0x80; max=0;mini=0;}     //秒7，没有必要调了

	if (sel == 4)
	{
		lcm_w_word("分钟");
		address = 0x82;
		max = 59;
		mini = 0;
		lcm_w_mm2();
		ds_w();
		lcm_w_mm2();

	} //分钟6，按动6次显示 调整分钟
	//并指定分钟数据写入1302芯片的地址是0x82，分钟数据的最大值是59，最小值是0

	if (sel == 3)
	{
		lcm_w_word("小时");
		address = 0x84;
		max = 23;
		mini = 0;

		lcm_w_hh2();
		ds_w();
		lcm_w_hh2();

	} //小时5，按动5次显示 调整小时
	//规定小时数据写入1302芯片的位置是0x84，小时数据最大值23，最小值是0

	if (sel == 2)
	{
		lcm_w_word("日期");
		address = 0x86;

		mo = read_clock(0x89); //读月
		moontemp1 = mo / 16;
		moontemp2 = mo % 16;
		mo = moontemp1 * 10 + moontemp2; //转换成10进制月份数据

		yy = read_clock(0x8d); //读年
		moontemp1 = yy / 16;
		moontemp2 = yy % 16;
		yy = moontemp1 * 10 + moontemp2; //转换成10进制年份数据

		if (mo == 2 && yy % 4 != 0)
		{
			max = 28;
			mini = 1;
		} //平年2月28天
		if (mo == 2 && yy % 4 == 0)
		{
			max = 29;
			mini = 1;
		} //闰年2月29天
		if (mo == 1 || mo == 3 || mo == 5 || mo == 7 || mo == 8 || mo == 10 || mo == 12)
		{
			max = 31;
			mini = 1;
		} //31天的月份
		if (mo == 4 || mo == 6 || mo == 9 || mo == 11)
		{
			max = 30;
			mini = 1;
		} //30天的月份
		lcm_w_dd2();
		ds_w();
		lcm_w_dd2();
	} //日3，按动3次显示 调整日期
	//规定日期数据写入2302的位置地址是0x86，日期最大值31，最小值是1

	if (sel == 1)
	{
		lcm_w_word("月份");
		address = 0x88;
		max = 12;
		mini = 1;
		lcm_w_mo2();
		ds_w();
		lcm_w_mo2();

	} //月2，按动2次显示 调整月份
	//规定月份写入1302的位置地址是0x88，月份最大值12，最小值1

	if (sel == 0)
	{
		lcm_w_word("年份");
		address = 0x8c;
		max = 99;
		mini = 0;
		lcm_w_yy2();
		ds_w();
		lcm_w_yy2();

	} //年1，按动1次显示 调整年份，
	  //规定年份写入1302的地址是0x8c,年份的最大值99，最小值0
}
/*****************************************************************************/
//被调数据加一或减一，并检查数据范围，写入1302指定地址保存
void ds_w(void)
{

	item = ((read_clock(address + 1)) / 16) * 10 + (read_clock(address + 1)) % 16;
	if (KEY_2 == 0)
	{			//如果按动上调键
		item++; //数加 1
	}
	if (KEY_4 == 0)
	{			//如果按动下调键
		item--; //数减 1
	}
	if (item > max)
		item = mini; //查看数值是否在有效范围之内
	if (item < mini)
		item = max;										//如果数值小于最小值，则自动等于最大值
	write_clock(0x8e, 0x00);							//允许写1302芯片操作
	write_clock(address, (item / 10) * 16 + item % 10); //转换成16进制写入1302
	write_clock(0x8e, 0x80);							//写保护，禁止写操作
}

/*********************************************************************
农历有关程序：

**********************************************************************
********************************************************************/
//C51写的公历转农历和星期
#define uchar unsigned char
#define uint unsigned int
#include <intrins.h>
/*
公历年对应的农历数据,每年三字节,
格式第一字节BIT7-4 位表示闰月月份,值为0 为无闰月,BIT3-0 对应农历第1-4 月的大小
第二字节BIT7-0 对应农历第5-12 月大小,第三字节BIT7 表示农历第13 个月大小
月份对应的位为1 表示本农历月大(30 天),为0 表示小(29 天)
第三字节BIT6-5 表示春节的公历月份,BIT4-0 表示春节的公历日期
*/
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
///月份数据表
code uchar day_code1[9] = {0x0, 0x1f, 0x3b, 0x5a, 0x78, 0x97, 0xb5, 0xd4, 0xf3};
code uint day_code2[3] = {0x111, 0x130, 0x14e};
/*
函数功能:输入BCD阳历数据,输出BCD阴历数据(只允许1901-2099年)
调用函数示例:Conversion(c_sun,year_sun,month_sun,day_sun)
如:计算2004年10月16日Conversion(0,0x4,0x10,0x16);
c_sun,year_sun,month_sun,day_sun均为BCD数据,c_sun为世纪标志位,c_sun=0为21世
纪,c_sun=1为19世纪
调用函数后,原有数据不变,读c_moon,year_moon,month_moon,day_moon得出阴历BCD数据
*/
bit c_moon;
data uchar year_moon, month_moon, day_moon, week;
/*子函数,用于读取数据表中农历月的大月或小月,如果该月为大返回1,为小返回0*/
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
	}
}
/*
函数功能:输入BCD阳历数据,输出BCD阴历数据(只允许1901-2099年)
调用函数示例:Conversion(c_sun,year_sun,month_sun,day_sun)
如:计算2004年10月16日Conversion(0,0x4,0x10,0x16);
c_sun,year_sun,month_sun,day_sun均为BCD数据,c_sun为世纪标志位,c_sun=0为21世
纪,c_sun=1为19世纪
调用函数后,原有数据不变,读c_moon,year_moon,month_moon,day_moon得出阴历BCD数据
*/
void Conversion(bit c, uchar year, uchar month, uchar day)
{ //c=0 为21世纪,c=1 为19世纪 输入输出数据均为BCD数据
	uchar temp1, temp2, temp3, month_p;
	uint temp4, table_addr;
	bit flag2, flag_y;
	temp1 = year / 16; //BCD->hex 先把数据转换为十六进制
	temp2 = year % 16;
	year = temp1 * 10 + temp2;
	temp1 = month / 16;
	temp2 = month % 16;
	month = temp1 * 10 + temp2;
	temp1 = day / 16;
	temp2 = day % 16;
	day = temp1 * 10 + temp2;
	//定位数据表地址
	if (c == 0)
	{
		table_addr = (year + 0x64 - 1) * 0x3;
	}
	else
	{
		table_addr = (year - 1) * 0x3;
	}
	//定位数据表地址完成
	//取当年春节所在的公历月份
	temp1 = year_code[table_addr + 2] & 0x60;
	temp1 = _cror_(temp1, 5);
	//取当年春节所在的公历月份完成
	//取当年春节所在的公历日
	temp2 = year_code[table_addr + 2] & 0x1f;
	//取当年春节所在的公历日完成
	// 计算当年春年离当年元旦的天数,春节只会在公历1月或2月
	if (temp1 == 0x1)
	{
		temp3 = temp2 - 1;
	}
	else
	{
		temp3 = temp2 + 0x1f - 1;
	}
	// 计算当年春年离当年元旦的天数完成
	//计算公历日离当年元旦的天数,为了减少运算,用了两个表
	//day_code1[9],day_code2[3]
	//如果公历月在九月或前,天数会少于0xff,用表day_code1[9],
	//在九月后,天数大于0xff,用表day_code2[3]
	//如输入公历日为8月10日,则公历日离元旦天数为day_code1[8-1]+10-1
	//如输入公历日为11月10日,则公历日离元旦天数为day_code2[11-10]+10-1
	if (month < 10)
	{
		temp4 = day_code1[month - 1] + day - 1;
	}
	else
	{
		temp4 = day_code2[month - 10] + day - 1;
	}
	if ((month > 0x2) && (year % 0x4 == 0))
	{ //如果公历月大于2月并且该年的2月为闰月,天数加1
		temp4 += 1;
	}
	//计算公历日离当年元旦的天数完成
	//判断公历日在春节前还是春节后
	if (temp4 >= temp3)
	{ //公历日在春节后或就是春节当日使用下面代码进行运算
		temp4 -= temp3;
		month = 0x1;
		month_p = 0x1; //month_p为月份指向,公历日在春节前或就是春节当日month_p指向首月
		flag2 = get_moon_day(month_p, table_addr);
		//检查该农历月为大小还是小月,大月返回1,小月返回0
		flag_y = 0;
		if (flag2 == 0)
			temp1 = 0x1d; //小月29天
		else
			temp1 = 0x1e; //大小30天
		temp2 = year_code[table_addr] & 0xf0;
		temp2 = _cror_(temp2, 4); //从数据表中取该年的闰月月份,如为0则该年无闰月
		while (temp4 >= temp1)
		{
			temp4 -= temp1;
			month_p += 1;
			if (month == temp2)
			{
				flag_y = ~flag_y;
				if (flag_y == 0)
					month += 1;
			}
			else
				month += 1;
			flag2 = get_moon_day(month_p, table_addr);
			if (flag2 == 0)
				temp1 = 0x1d;
			else
				temp1 = 0x1e;
		}
		day = temp4 + 1;
	}
	else
	{ //公历日在春节前使用下面代码进行运算
		temp3 -= temp4;
		if (year == 0x0)
		{
			year = 0x63;
			c = 1;
		}
		else
			year -= 1;
		table_addr -= 0x3;
		month = 0xc;
		temp2 = year_code[table_addr] & 0xf0;
		temp2 = _cror_(temp2, 4);
		if (temp2 == 0)
			month_p = 0xc;
		else
			month_p = 0xd; //
		/*month_p为月份指向,如果当年有闰月,一年有十三个月,月指向13,无闰月指向12*/
		flag_y = 0;
		flag2 = get_moon_day(month_p, table_addr);
		if (flag2 == 0)
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

/*************************************************************************
/*函数功能:输入BCD阳历数据,输出BCD星期数据(只允许1901-2099年)
调用函数示例:Conver_week(c_sun,year_sun,month_sun,day_sun)
如:计算2004年10月16日Conversion(0,0x4,0x10,0x16);
c_sun,year_sun,month_sun,day_sun均为BCD数据,c_sun为世纪标志位,c_sun=0为21世
纪,c_sun=1为19世纪
调用函数后,原有数据不变,读week得出阴历BCD数据
*/
code uchar table_week[12] = {0, 3, 3, 6, 1, 4, 6, 2, 5, 0, 3, 5}; //月修正数据表
/*
算法:日期+年份+所过闰年数+月较正数之和除7 的余数就是星期但如果是在
闰年又不到3 月份上述之和要减一天再除7
星期数为0
*/

void Conver_week(bit c, uchar year, uchar month, uchar day)
{ //c=0 为21世纪,c=1 为19世纪 输入输出数据均为BCD数据
	uchar temp1, temp2;
	temp1 = year / 16; //BCD->hex 先把数据转换为十六进制
	temp2 = year % 16;
	year = temp1 * 10 + temp2;
	temp1 = month / 16;
	temp2 = month % 16;
	month = temp1 * 10 + temp2;
	temp1 = day / 16;
	temp2 = day % 16;
	day = temp1 * 10 + temp2;
	if (c == 0)
	{
		year += 0x64;
	}					//如果为21世纪,年份数加100
	temp1 = year / 0x4; //所过闰年数只算1900年之后的
	temp2 = year + temp1;
	temp2 = temp2 % 0x7; //为节省资源,先进行一次取余,避免数大于0xff,避免使用整型数据
	temp2 = temp2 + day + table_week[month - 1];
	if (year % 0x4 == 0 && month < 3)
		temp2 -= 1;
	week = temp2 % 0x7;
}

/******************************************
显示农历年月日

*******************************************/
//向LCM中填写 农历年月日数据
void lcm_w_nl(void)
{

	dd = read_clock(0x87);	 //日
	mo = read_clock(0x89);	 //月
	yy = read_clock(0x8d);	 //年
	Conversion(0, yy, mo, dd); //调用公历转农历子函数
	//调用函数后,原有数据不变,读c_moon农历世纪,year_moon农历年,month_moon,day_moon得出阴历BCD数据
	lcm_w_test(0, 0x88);
	lcm_w_word("农历20"); //显示内容字符20

	lcm_w_test(1, (year_moon / 16) + 0x30); //函数参数1，代表本行写数据，YY/16+0X30得出年十位数字的显示码地址，送显示
	lcm_w_test(1, year_moon % 16 + 0x30);   //函数参数1，代表本行写数据，YY%得出年个位数字的显示码地址，送显示

	lcm_w_word("-");

	lcm_w_test(1, (month_moon / 16) + 0x30);
	lcm_w_test(1, month_moon % 16 + 0x30); //与16取余数，得到月份的个位数，加0x30得到该数字的液晶内定显示码送显示

	lcm_w_word("-"); //调用字符显示函数，显示文字 月

	lcm_w_test(1, (day_moon / 16) + 0x30);
	lcm_w_test(1, day_moon % 16 + 0x30); //第一个1参数，表示本行写数据，日数据与16取余得个位数，加0x30得到显示码
}

/*****************************************************************************/
//---主程序---//
/*****************************************************************************/
main()
{
	unsigned char e = 0;
	//KEY_I = 0;
	KEY_1 = 1;
	KEY_2 = 1;
	KEY_3 = 1;
	KEY_4 = 1;
	KEY_5 = 1; //初始键盘
	beep = 0;  //因为用9014三极管驱动蜂鸣器，所以beep=1时，蜂鸣器响
	led_back = 1;
	yy = 0xff;
	mo = 0xff;
	dd = 0xff;
	xq = 0xff;
	hh = 0xff;
	mm = 0xff;
	ss = 0xff;	//各数据刷新
	Beep();		  //鸣响一声
	Init_1302();  //初始化1302时钟芯片
	lcm_init();   //初始化液晶显示器
	lcm_clr();	//清屏
	welcome();	//显示欢迎信息，显示欢迎函数内设定的内容
	DelayM(5000); //欢迎信息显示停留1秒
	lcm_init();   //
	lcm_w_nl();   //
	/*****************************************************************************/
	while (1)
	{ //主循环

		if (KEY_5 == 0) // 按下背光控制
		{
			DelayM(20); //防抖

			if (KEY_5 == 0)
			{
				Beep_key();			  //确定按键音
				led_back = ~led_back; //改变背光状态
			}
			while (KEY_5 == 0)
				; //等待键松开
		}

		if (w == 0)
		{				//W为程序头段规定的正常走时标志位，=0时说明在正常走时
			lcm_w_ss(); //刷新 秒
		}
		else
		{ //否则（W!=0的情况下就是调时状态）启动调时
		}
		//------------------------------------------------------------------
		if (KEY_3 == 0) // 按下设置键
		{
			DelayM(20); //去抖
			//led_g3 = 0;led_r = 1;
			if (KEY_3 == 0 && w == 1) //如果按下设置键时，系统是调时状态 本键用于调整下一项
			{
				Beep_key(); //按键音
				e++;
				if (e >= 6)
				{
					e = 0;
				}
				Set_time(e); //调用调整时间子函数
			}

			if (KEY_3 == 0 && w == 0) //如果按下设置键时，系统正在正常走时状态时就进入调时状态
			{
				Beep_set(); //确定按键音
				//led_b = 1;//
				lcm_clr();  //清屏
				lcm_clr2(); //清前三行屏幕显示
				w = 1;		//进入调时，w是正常走时的标志位，=1进入调试状态，=0进入正常走时状态

				lcm_w_test(0, 0x80);
				lcm_w_word("Esc");
				lcm_w_test(0, 0x90); //指定显示位置
				lcm_w_word("[+]");   //在上一行指定位置显示引号内的字符
				lcm_w_test(0, 0x88);
				lcm_w_word("Sel");
				lcm_w_test(0, 0x98); //函数第一参数是0，表示本很写入液晶指令，指定显示位置98H
				lcm_w_word("[-]");   //在上一行指定的位置显示 引号内的字符
				Set_time(e);		 //调用调整时间子函数
			}
			while (KEY_3 == 0)
				; //等待键松开
		}
		//------------------------------------------------------------------
		if (KEY_1 == 0) // 按下退出键Esc，当在调时状态时就退出调时
		{
			DelayM(20);
			//led_g1 = 0;led_r = 1;
			if (KEY_1 == 0 && w == 1)
			{				//如果w==1说明在调时状态，按动这个键，退出调时
				Beep_set(); //确定按键音
				w = 0;		//退出调时，进入正常走时状体
				e = 0;		//设置键功能循环中控制显示 下一项 的参数归0
				lcm_w_nl(); //刷新农历显示
			}
			if (KEY_1 == 0 && w == 0)
			{
				lcm_clr(); //清屏
				lcm_w_test(0, 0x80);
				welcome(); //显示欢迎信息
				while (KEY_1 == 0)
					; //等待键松开
			}
			lcm_clr();  //清屏
			updata();   //刷新数据
			lcm_w_mm(); //刷新 分
			lcm_w_nl(); //
			while (KEY_1 == 0)
				; //等待键松开
		}
		//------------------------------------------------------------------
		if (KEY_2 == 0 && w == 1)
		{				// 加减调整，KEY_2为上调键，如果上调键按下，且系统在调时状态{
			DelayM(20); //延时消抖动20毫秒
			if (KEY_2 == 0 && w == 1)
			{				 //确认上调键是按下的
				Beep_key();  //按键音
				Set_time(e); //调用调时子函数
			}
			while (KEY_2 == 0)
				; //等待键松开
		}
		//------------------------------------------------------------------
		if (KEY_4 == 0 && w == 1)
		{				// 加减调整，KEY_4为下调键，如果下调键按下而且系统在调时状态
			DelayM(20); //延时消抖动20毫秒
			if (KEY_4 == 0 && w == 1)
			{				 //确定下调键是按下的
				Beep_key();  //按键音
				Set_time(e); //调用调时的子函数
			}
			while (KEY_4 == 0)
				; //等待键松开
		}
	}
}
