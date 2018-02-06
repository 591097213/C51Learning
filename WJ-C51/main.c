#include <reg52.h>
#include "flowLamp.c"

void main()
{
    int led=0xaa;
    flowLamp(led);
}