#include "flowLamp.h"
#include <intrins.h>
#include <reg52.h>
#include "delay.h"

/**
 * @param int 传入一个数,其二进制位代表led.记得用十六进制表示
 */
void flowLamp(int led)
{
    P1 = led;
}

void flowLampExample()
{
    unsigned char led = 0xfe;
    flowLamp(led);
    for (;;)
    {
        Delay_Ms(500);
        led = _crol_(led, 1);
        flowLamp(led);
    }
}