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