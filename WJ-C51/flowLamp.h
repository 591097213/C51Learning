#ifndef __FLOWLAMP_H
#define __FLOWLAMP_H

void rzmzy();			 //红色走马左移声明
void rlszy();			 //红色流水左移声明
void rlsyy();			 //红色流水右移声明
void rzmyy();			 //红色走马右移声明
void rzmnwyd();			 //红色走马内外移动声明
void rlsnwyd();			 //红色流水内外移动声明
void rtbyd();			 //红色跳变移动声明
void rsdzmyy();			 //红色3灯走马右移声明
void rsdzmzy();			 //红色3灯走马左移声明
void rjl();				 //红色渐亮声明
void rja();				 //红色渐暗声明
void rdlszy();			 //倒流水左移函数

//rdlsyy();	//倒流水右移函数，还未加入

void zkb(unsigned char d); //调整占空比


#endif