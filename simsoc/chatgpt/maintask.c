#include "sim.h"  
#include "main.h"
#include "soc.h"
#include "toolfun.h"

/// 函数调用周期 100ms
/// voltage 单位是 1/64 V
/// current 单位是 1/64 A
/// cap 是 soc_reset() 使用的容量参数
/// cellnum 是电池节数，>0 使用指定值，<=0 自动判断
/// temp 是给定传感器温度，会由内部更新到 SystemParam.TEMPERATURE
/// init_voltage 是初始化/reset 使用的电压，>0 使用指定值，<=0 使用当前 voltage

void maintask(int voltage, int current, int cap, int cellnum, int temp, int init_voltage)
{
    static int tick;
    int reset_voltage;
    int judge_voltage;

    SystemParam.AVG_CURRENT = current;
    SystemParam.AVG_VOLTAGE = voltage;
    SystemParam.TEMP_NOW = temp;

    if (tick == 0)
    {
        soc_key_paramter_init();

        if (init_voltage > 0)
            reset_voltage = init_voltage;
        else
            reset_voltage = voltage;

        judge_voltage = reset_voltage;

        if (cellnum > 0)
        {
            Bat.CELL_NUM = cellnum;
        }
        else
        {
            if (judge_voltage > VOLTAGE_V(70))
                Bat.CELL_NUM = 6;
            else if (judge_voltage > VOLTAGE_V(60))
                Bat.CELL_NUM = 5;
            else
                Bat.CELL_NUM = 4;
        }

        SystemParam.AVG_VOLTAGE = reset_voltage;
        soc_reset(cap, Bat.CELL_NUM, 1);
        soc_init();
    }

    tick++;

    SystemParam.AVG_CURRENT = current;
    SystemParam.AVG_VOLTAGE = voltage;
    SystemParam.TEMP_NOW = temp;

    soc_process();
    soc_show_process();
}
