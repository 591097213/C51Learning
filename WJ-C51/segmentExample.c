#include "segmentDisplay.h"

/**
 * @brief 数码管显示例子
 * 
 */
void segmentDisplayExample()
{
    int cha1[] = {16, 16, 16, 16, 16, 16, 16, 16};
    int cha2[] = {8, 8, 8, 8, 8, 8, 8, 8};
    int boo[] = {0, 0, 0, 0, 0, 0, 0, 0};
    for (;;)
    {
        dynamic(cha1, boo);
        dynamic(cha1, boo);
        dynamic(cha2, boo);
    }
}