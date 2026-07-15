#ifndef __tool_h
#define __tool_h
#include "type.h"


extern s32 math_diveder(s32 Dividend, s32 Divisor), two_point_diff_calc(s32 , s32 , s32 , s32 );
extern u8 k_value_verify(u16 *k_value), b_value_verify(u16 *, u16, u16, u16);
extern void variable_dec(s16 delta, s16 value, s16* cmd), variable_inc(s16 delta, s16 value, s16* cmd);
extern void use_record_process(void), use_record_end(void), use_record_init(u8 type);
extern void avg_current_voltage_calc(void);
#endif
