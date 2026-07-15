#ifndef __LIB_H
#define __LIB_H

#include "type.h"

// ========== 常量数组 extern ==========
extern const u16 CAP_13_LEFT[14][21];
extern const u16 CAP_16_LEFT[14][21];
extern const u16 CAP_22_LEFT[14][21];
extern const u16 CAP_25_LEFT[14][21];
extern const u16 CAP_28_LEFT[14][21];
extern const u16 CAP_32_LEFT[14][21];
extern const u16 CAP_35_LEFT[14][21];
extern const u16 CAP_38_LEFT[14][21];
extern const u16 CAP_45_LEFT[14][21];
extern const u16 CAP_52_LEFT[14][21];
extern const u16 CAP_RATING_ARRAY[10];
extern const u16 SOC_I_105V[11][10];
extern const u16 SOC_I_110V[11][10];
extern const u16 VOLT_POINT_ARRAY[21];

// ========== 全局变量 extern ==========
extern u8 Verifyied;

// ========== CAP 指针数组 extern ==========
extern const u16 *const CAP_TYPE_ARRAY[];


// ========== 函数声明 ==========
extern void cap_calc(void);
extern void cap_dec_undervolt(u16 current_index,u16 cap_index);
extern u8 cap_index_calc(u16 cap);
extern u16 cap_last_calc(u16 battery_volt,s16 bus_current,u16 cap_array[14][21],u8 current_index);
extern void cap_learn_insert(void);
extern void discharge_soc_compensation(void);
extern void discharge_zero_detect(void);
extern u16 last_cap_calc(u16 battery_volt,s16 battery_current,u16 battery_cap,u16 cap_dis_once);
extern u16 last_cap_convert_on_temp(u16 cap);
extern void q_25_constrant(u16 *newQ);
extern void q_now_constrant(void);
extern void soc_key_paramter_init(void);
extern void soc_q_array_rearrange(u16 newQ,u8 num);
extern void soc_zero_decrease_process(void);
extern void uid_save(void);
extern void update_cap_array(u16 temp_cap,u8 incflg);

#endif