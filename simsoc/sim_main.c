/*
 * sim_main.c — SOC 算法仿真主程序（完整版）
 * 输出所有结构体字段到 stdout
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "type.h"
#include "main.h"
#include "soc.h"
#include "toolfun.h"

extern void maintask(int voltage, int current, int cap, int cellnum, int temp, int init_voltage);

#define H(field)  printf(",%s", #field)
#define V(field)  printf(",%d", (int)(field))

static void print_header(void)
{
    printf("tick");
    /* Soc (14 fields) */
    H(Soc.SHOW); H(Soc.INDEX); H(Soc.FULL); H(Soc.ZERO);
    H(Soc.OCV_SOC); H(Soc.SOH); H(Soc.CIRCLE); H(Soc.ZERO_DELTA);
    H(Soc.ZERO_SOC); H(Soc.DIS_SOC); H(Soc.CHG_SOC); H(Soc.OCV_START);
    H(Soc.DEC_SHOW); H(Soc.HIGH_LIMIT); H(Soc.LOW_LIMIT);
    /* Bat (3 fields) */
    H(Bat.CELL_NUM); H(Bat.CAP_RATING); H(Bat.CAP_NOW);
    /* Cap (29 fields) */
    H(Cap.Q25); H(Cap.QNOW);
    H(Cap.ARRAY_0); H(Cap.ARRAY_1); H(Cap.ARRAY_2); H(Cap.ARRAY_3); H(Cap.ARRAY_4);
    H(Cap.CAP_GUSS_IN_CURRENT_CHARGE); H(Cap.CAP_GUSS_LAST); H(Cap.CAP_GUSS_TOTAL_DIS);
    H(Cap.CAP_CHG_ONCE); H(Cap.CAP_DIS_ONCE); H(Cap.CAP_CHG_LAST);
    H(Cap.CAP_LEARN_MAX); H(Cap.CAP_SET); H(Cap.CAP_LEARN_MIN);
    H(Cap.SET_IMPACT); H(Cap.CAP_LEARN_INDEX); H(Cap.CAP_SET_DEC_CNT);
    H(Cap.AVG_VOLTAGE_0); H(Cap.AVG_VOLTAGE_1); H(Cap.AVG_VOLTAGE_2);
    H(Cap.AVG_CURRENT_0); H(Cap.AVG_CURRENT_1); H(Cap.AVG_CURRENT_2);
    H(Cap.DIS_CAP_0); H(Cap.DIS_CAP_1); H(Cap.DIS_CAP_2);
    H(Cap.INDEX);
    /* SystemParam (8 fields) */
    H(SystemParam.BUS_CURRENT); H(SystemParam.BUS_VOLTAGE);
    H(SystemParam.TEMPERATURE); H(SystemParam.TEMP_NOW);
    H(SystemParam.PCB_TEMPERATURE); H(SystemParam.STATE);
    H(SystemParam.AVG_CURRENT); H(SystemParam.AVG_VOLTAGE);
    /* Integral (13 fields) */
    H(Integral.SUM_VOLTAGE); H(Integral.SUM_CURRENT);
    H(Integral.AH_0001_DIS); H(Integral.AH_0001_CHG);
    H(Integral.AH_0001_CMP_DIS); H(Integral.AH_0001_CMP_CHG);
    H(Integral.AH); H(Integral.AH_LAST);
    H(Integral.AH_CHG_ONCE); H(Integral.AH_DIS_ONCE);
    H(Integral.ZERO_AH); H(Integral.ZERO_VOLTAGE); H(Integral.DIS_AH_BLOCK);
    /* Soe (7 fields) */
    H(Soe.INTEGRAL); H(Soe.POWER); H(Soe.SOE); H(Soe.FULL);
    H(Soe.DIS_WH_ONCE); H(Soe.CHG_WH_ONCE); H(Soe.HOURS_12_CNT);
    /* His (9 fields) */
    H(His.CIRCLE); H(His.CAPSUM); H(His.OPEN); H(His.WORK_SEC);
    H(His.LAST_VOLT); H(His.CAPSUM_LST); H(His.FLT_CNT);
    H(His.DIS_TIME); H(His.CHG_TIME);
    /* BlockParam (8 fields) */
    H(BlockParam.C01_CURRENT); H(BlockParam.CHG_CURRENT); H(BlockParam.AVG_CURRENT);
    H(BlockParam.MAX_VOLTAGE); H(BlockParam.MAX_CURRENT);
    H(BlockParam.AVG_VOLTAGE); H(BlockParam.MIN_VOLTAGE); H(BlockParam.MIN_CURRENT);
    printf("\n");
}

static void print_row(int tick)
{
    printf("%d", tick);
    /* Soc */
    V(Soc.SHOW); V(Soc.INDEX); V(Soc.FULL); V(Soc.ZERO);
    V(Soc.OCV_SOC); V(Soc.SOH); V(Soc.CIRCLE); V(Soc.ZERO_DELTA);
    V(Soc.ZERO_SOC); V(Soc.DIS_SOC); V(Soc.CHG_SOC); V(Soc.OCV_START);
    V(Soc.DEC_SHOW); V(Soc.HIGH_LIMIT); V(Soc.LOW_LIMIT);
    /* Bat */
    V(Bat.CELL_NUM); V(Bat.CAP_RATING); V(Bat.CAP_NOW);
    /* Cap */
    V(Cap.Q25); V(Cap.QNOW);
    V(Cap.ARRAY[0]); V(Cap.ARRAY[1]); V(Cap.ARRAY[2]); V(Cap.ARRAY[3]); V(Cap.ARRAY[4]);
    V(Cap.CAP_GUSS_IN_CURRENT_CHARGE); V(Cap.CAP_GUSS_LAST); V(Cap.CAP_GUSS_TOTAL_DIS);
    V(Cap.CAP_CHG_ONCE); V(Cap.CAP_DIS_ONCE); V(Cap.CAP_CHG_LAST);
    V(Cap.CAP_LEARN_MAX); V(Cap.CAP_SET); V(Cap.CAP_LEARN_MIN);
    V(Cap.SET_IMPACT); V(Cap.CAP_LEARN_INDEX); V(Cap.CAP_SET_DEC_CNT);
    V(Cap.AVG_VOLTAGE[0]); V(Cap.AVG_VOLTAGE[1]); V(Cap.AVG_VOLTAGE[2]);
    V(Cap.AVG_CURRENT[0]); V(Cap.AVG_CURRENT[1]); V(Cap.AVG_CURRENT[2]);
    V(Cap.DIS_CAP[0]); V(Cap.DIS_CAP[1]); V(Cap.DIS_CAP[2]);
    V(Cap.INDEX);
    /* SystemParam */
    V(SystemParam.BUS_CURRENT); V(SystemParam.BUS_VOLTAGE);
    V(SystemParam.TEMPERATURE); V(SystemParam.TEMP_NOW);
    V(SystemParam.PCB_TEMPERATURE); V(SystemParam.STATE);
    V(SystemParam.AVG_CURRENT); V(SystemParam.AVG_VOLTAGE);
    /* Integral */
    V(Integral.SUM_VOLTAGE); V(Integral.SUM_CURRENT);
    V(Integral.AH_0001_DIS); V(Integral.AH_0001_CHG);
    V(Integral.AH_0001_CMP_DIS); V(Integral.AH_0001_CMP_CHG);
    V(Integral.AH); V(Integral.AH_LAST);
    V(Integral.AH_CHG_ONCE); V(Integral.AH_DIS_ONCE);
    V(Integral.ZERO_AH); V(Integral.ZERO_VOLTAGE); V(Integral.DIS_AH_BLOCK);
    /* Soe */
    V(Soe.INTEGRAL); V(Soe.POWER); V(Soe.SOE); V(Soe.FULL);
    V(Soe.DIS_WH_ONCE); V(Soe.CHG_WH_ONCE); V(Soe.HOURS_12_CNT);
    /* His */
    V(His.CIRCLE); V(His.CAPSUM); V(His.OPEN); V(His.WORK_SEC);
    V(His.LAST_VOLT); V(His.CAPSUM_LST); V(His.FLT_CNT);
    V(His.DIS_TIME); V(His.CHG_TIME);
    /* BlockParam */
    V(BlockParam.C01_CURRENT); V(BlockParam.CHG_CURRENT); V(BlockParam.AVG_CURRENT);
    V(BlockParam.MAX_VOLTAGE); V(BlockParam.MAX_CURRENT);
    V(BlockParam.AVG_VOLTAGE); V(BlockParam.MIN_VOLTAGE); V(BlockParam.MIN_CURRENT);
    printf("\n");
}

int main(int argc, char *argv[])
{
    char line[4096];
    int voltage, current, cap, cellnum, temp, init_voltage;
    int tick = 0, first = 1;
    cap = 38; cellnum = 5; temp = 25; init_voltage = 0;
    if (argc >= 2) cap = atoi(argv[1]);
    if (argc >= 3) cellnum = atoi(argv[2]);
    if (argc >= 4) temp = atoi(argv[3]);

    print_header();

    while (fgets(line, sizeof(line), stdin))
    {
        if (line[0] == '\n' || line[0] == '\r' || line[0] == '#') continue;
        size_t len = strlen(line);
        while (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r')) line[--len] = '\0';
        if (strstr(line, "time_s") || strstr(line, "tick")) continue;

        if (first) {
            sscanf(line, "%d,%d,%d,%d,%d,%d", &voltage, &current, &cap, &cellnum, &temp, &init_voltage);
            maintask(voltage, current, cap, cellnum, temp, init_voltage);
            first = 0;
        } else {
            sscanf(line, "%d,%d", &voltage, &current);
            maintask(voltage, current, 0, 0, temp, 0);
        }
        print_row(tick++);
    }
    return 0;
}
