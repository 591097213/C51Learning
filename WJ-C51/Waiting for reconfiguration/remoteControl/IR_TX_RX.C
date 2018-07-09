/**
 * @brief 红外发送、接收相关
 * IR_TX_RX 表示 中断_发送_接收
 * @file IR_TX_RX.C
 * @author your name
 * @date 2018-03-18
 */

/*FOSC=11.0592MHz*/
#include "qxmcs51_config.h"
#include "delay.h"
#include "IR_TX_RX.H"

BOOL IR_RX_Finish;			//接收完成标志
INT8U IR_RX_CODE[4];		//用于存储接收到的4个字节
extern INT8U IR_TX_CODE[4]; //存储要发送的4字节数据
							//（包括两字节用户码、一字节数据码和一字节数据反码）
/**
 * @brief 初始化两个定时器
 *
 */
void IR_TX_RX_Init()
{
	//timer0用于控制红外线发射频率，相当于对信号进行调制
	//timer1控制所发送的数据
	TMOD = 0x12; // 0001(timer1 16位定时/计数器) 0010(timer0 初值自动重装的8位定时/计数器)

	TH0 = 0xf4;
	TL0 = 0xf4; //38KHz

	EA = 1;		 // Enable All Interrupt
	ET1 = 1;	 // Enable Timer1，使能T1，未启动
	ET0 = 1;	 // Enable Timer0，使能T0，未启动
	EX0 = 1;	 // Enable External Interrupt0 外部中断0
	P_IR_TX = 1; // stop emit 刚初始化时不发射红外线。P_IR_TX是红外发射引脚
}

/**
 * @brief 红外线发射1位信号
 * th1与tl1共同控制发射时间
 * @param th1
 * @param tl1
 * @param mod 通过控制timer0的启止表示要发送的值
 */
void Timer1_Delay(INT8U th1, INT8U tl1, BOOL mod)
{
	TH1 = th1;
	TL1 = tl1;
	TR1 = 1;   //timer1 run 启动T1
	TR0 = mod; //timer0 run or stop
	while (!TF1)
		;		 //wait timer1 flag TF1定时器1溢出标志位，T1溢出时由硬件置1,并申请中断
	TF1 = 0;	 //由于未进入中断函数中，不会执行硬件清零，故需在此用软件清零的方式清零标志位
	TR0 = 0;	 //timer0 stop run
	TR1 = 0;	 //timer1 stop run
	P_IR_TX = 1; // stop emit
}

/**
 * @brief 红外发送4字节数据
 *
 */
void Transmit_TR_CODE()
{
	INT8U num, num1, IR_CodeBuf[4];

	EX0 = 0; //Disable External Interrupt0

	//引导码
	Timer1_Delay(0xdf, 0x9a, 1); //emit 9ms
	Timer1_Delay(0xef, 0xcd, 0); //delay 4.5ms

	for (num = 0; num < 4; num++) //处理4字节
	{
		IR_CodeBuf[num] = IR_TX_CODE[num];
		for (num1 = 0; num1 < 8; num1++) //处理每字节中的8位
		{
			Timer1_Delay(0xfd, 0xfc, 1); //emit 0.56ms
			if (IR_CodeBuf[num] & 0x01)  //该位为1
			{
				Timer1_Delay(0xf9, 0xef, 0); //delay 1.685ms
			}
			else //该位为0
			{
				Timer1_Delay(0xfd, 0xfc, 0); //delay 0.56ms
			}
			IR_CodeBuf[num] >>= 1;
		}
	}
	Timer1_Delay(0xfd, 0xd7, 1); //emit 600us.结束码
	EX0 = 1;					 //open interrupt
}

/**
 * @brief 对信号进行调制
 * 通过timer0 8位初值自动重装的方式将红外信号调制为38kHz
 *
 */
void Timer0() interrupt 1 //1 mean timer0
{
	P_IR_TX = ~P_IR_TX;
}

/**
 * @brief timer1中断函数
 * timer1只起定时作用，通过查询TF1来判断是否计时结束。
 * timer1中断函数中无需执行任何语句。TF1需要软件清零，不会硬件清零
 */
void Timer1() interrupt 3 //3 means timer1
{
	//进入中断后无需做任何事，亦可不写该中断函数。这里写出是为了方便理解
}

/**
 * @brief 外部中断0中断函数
 * 用于接收红外信号并存入IR_RXCODE[]中
 */
void External() interrupt 0 //0 mean external interrupt0
{
	BOOL Start_Flag;
	INT8U num, num1;

	EX0 = 0; //close interrupt 进入外部中断后先关外部中断
	TH1 = 0xe0;
	TL1 = 0x52; //8800us 8.8ms
	TF1 = 0;	//timer1溢出标志清零
	TR1 = 1;	// timer1 run
	Start_Flag = 1;
	while (!TF1) //timer flag 等待timer1溢出
	{
		if (P_IR_RX) //P_IR_RX红外接收引脚为1
			Start_Flag = 0;
	}
	TR1 = 0; //important timer1溢出后关闭timer1
	TF1 = 0; //clear timer1 flag 溢出标志清零
	TH1 = 0xee;
	TL1 = 0x12; //4980us 4.5ms
	TR1 = 1;	//timer1 run
	while (!TF1)
		;
	{
		if (P_IR_RX)
			Start_Flag = 0;
	}
	TR1 = 0; //timer1 stop run
	TF1 = 0; //clear timer1 flag
	if ((P_IR_RX == 0) && Start_Flag)
	{
		for (num = 0; num < 4; num++)
		{
			for (num1 = 0; num1 < 8; num1++)
			{
				IR_RX_CODE[num] >>= 1;
				while (!P_IR_RX)
					; //wait for high level
				TH1 = 0xfc;
				TL1 = 0xFA; //840us
				TR1 = 1;	//timer1 run
				while (!TF1)
					;	//wait for timer1 flag
				TR1 = 0; //timer1 run
				TF1 = 0; //clear timer1 flag
				if (P_IR_RX)
				{
					IR_RX_CODE[num] |= 0x80;
					TH1 = 0xfb;
					TL1 = 0xf3; //1125us
					TR1 = 1;	//timer1 run
					while (!TF1)
						;	//wait for timer1 flag
					TR1 = 0; //timer1 run
					TF1 = 0; //clear timer1 flag
				}
			}
		}
		IR_RX_Finish = 1;
		for (num1 = 0; num1 < 5; num1++)
		{
			BEEP = !BEEP;
			Delay_Ms(20);
		}
		BEEP = 1;
	}
	EX0 = 1;
}