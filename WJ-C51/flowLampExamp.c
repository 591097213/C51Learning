#include <reg52.h>
#include <intrins.h>

/**
 * 延时，单位为毫秒
 * @param unsigned int
 */

void delayms(unsigned int ms)
{
	unsigned int i, j;
	for (i = 0; i < ms; i++)
	{
		for (j = 0; j < 110; j++)
			;
	}
}

/**
 * @param int 传入一个数,其二进制位代表led.记得用十六进制表示
 */
void flowLamp(int led)
{
	P1 = led;
}

void main()
{
	unsigned char led = 0xfe;
	flowLamp(led);
	for (;;)
	{
		delayms(500);
		led = _crol_(led, 1);
		flowLamp(led);
	}
}