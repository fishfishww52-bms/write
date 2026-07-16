#include "main.h"
#include "toolfun.h"


s32 math_diveder(s32 Dividend, s32 Divisor)
{
	u8 delay_cnt = 0;
	if (Divisor == 0)
	{
		return 0;
	}
	MATH_DIV->DIVISOR0 = Divisor;
	MATH_DIV->DIVIDEND0 = Dividend;
	SET_BIT(MATH_DIV->CTRL, MATH_DIV0_STR);
	while(!(READ_BIT(MATH->INTF, MATH_INTF_DIV))){
		if (++delay_cnt > 250)
		{
			return 0;
		}
	}
	CLEAR_FLAG(MATH->INTFC, MATH_INTFC_DIV);
	return MATH_DIV->QUOTIENT0;
}

s32 two_point_diff_calc(s32 startvalue, s32 endmax, s32 diffvalue, s32 diffvaluemax)
{
	return (startvalue + math_diveder(endmax*diffvalue,diffvaluemax));
}

u8 k_value_verify(u16 *k_value)
{
	if (*k_value > K_VALUE_MAX || *k_value < K_VALUE_MIN)
	{
		*k_value = K_VALUE_DEFAULT;
		return 1;
	}
	else
	{
		return 0;
	}
}

u8 b_value_verify(u16 *b_value, u16 MAX, u16 MIN, u16 DEFAULT)
{
	if (*b_value > MAX || *b_value < MIN)
	{
		*b_value = DEFAULT;
		return 1;
	}
	else
	{
		return 0;
	}
}


void variable_dec(s16 delta, s16 value, s16* cmd)//reentrant
{
	s16 temp_cmd = *cmd;
	if (temp_cmd >= value + delta)
	{
		*cmd = temp_cmd - delta;
	}
	else if (temp_cmd > value)
	{
		*cmd = temp_cmd - 1;
	}
}

void variable_inc(s16 delta, s16 value, s16* cmd)//reentrant
{
	s16 temp_cmd = *cmd;
	if (temp_cmd + delta <= value)
	{
		*cmd = temp_cmd + delta;
	}
	else if (temp_cmd < value)
	{
		*cmd = temp_cmd + 1;
	}
}
