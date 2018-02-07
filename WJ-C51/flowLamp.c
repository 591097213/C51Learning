#include <reg52.h>

/**
 * @param int 传入一个数,其二进制位代表led.记得用十六进制表示
 */
void flowLamp(int led)
{
    P1 = led;
}