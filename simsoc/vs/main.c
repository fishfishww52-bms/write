#include "sim.h"

int main(void)
{
    /* 临时测试：5 节电池，66V，电流 0，容量参数 360，温度 25，reset 初始电压 66V */
    maintask(66 * 64, 0, 36, 5, 25, 66 * 64);
    return 0;
}
