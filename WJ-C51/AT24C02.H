#ifndef __AT24C02_H
#define __AT24C02_H

#include "datatype.h"

bit I2C_AT24C02_ReadData(uchar ADDR, uchar *AT24C02_Value);
bit I2C_AT24C02_WriteData(uchar addr, uchar Data);
void AT24C02Example();
void clear();

#endif