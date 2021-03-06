#ifndef __IIC_AT24_H
#define __IIC_AT24_H

#include "datatype.h"

void I2C_Init();
void I2C_Start();
void I2C_Stop();
void Master_ACK(bit i);
bit Test_ACK();
void I2C_send_byte(uchar);
uchar I2C_read_byte();

#endif