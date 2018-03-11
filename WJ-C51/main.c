#include "qxmcs51_config.h"
#include "segmentDisplay.h"

void main()
{
	Sent(0x81);
	RCK = 0;
	RCK = 1;
}