#include "main.h"
#include "soc.h"
#include "toolfun.h"
#include "type.h"
#include "lib.h"
#include "sim.h"
// 全局变量



void cap_dec_undervolt(u16 current_index, u16 cap_index)
{
	short soc_110, soc_105, soc_cap1, soc_cap2; 

	current_index = math_diveder(current_index, CURRENT_A(5)); 
	cap_index = cap_index_calc(cap_index); 
	if (current_index < 0xb) { 
		if (BlockParam.AVG_VOLTAGE < TOTAL_CELL_VOLTAGE(10.5)) {
			BlockParam.AVG_VOLTAGE = TOTAL_CELL_VOLTAGE(10.5);
		} 
		Marks.ZERO = 0;
		if (current_index == 0xa) {
			if (cap_index < 0xa) {
				if (cap_index == 0x0) {
					soc_110 = SOC_I_110V[10][0];
					soc_105 = SOC_I_105V[10][0];
				}
				else {
					soc_110 = two_point_diff_calc(
						SOC_I_110V[10][cap_index - 1],
						SOC_I_110V[10][cap_index] - SOC_I_110V[10][cap_index - 1],
						Cap.Q25 - CAP_RATING_ARRAY[cap_index - 1],
						CAP_RATING_ARRAY[cap_index] - CAP_RATING_ARRAY[cap_index - 1]);  

					soc_105 = two_point_diff_calc(SOC_I_105V[10][cap_index - 1],
						SOC_I_105V[10][cap_index] - SOC_I_105V[10][cap_index - 1],
						Cap.Q25 - CAP_RATING_ARRAY[cap_index - 1],
						CAP_RATING_ARRAY[cap_index] - CAP_RATING_ARRAY[cap_index - 1]);
				}
			}
			else { 
				soc_110 = SOC_I_110V[10][9];
				soc_105 = SOC_I_105V[10][9];
			}
			Soc.ZERO_SOC = two_point_diff_calc(
				soc_105,
				soc_110 - soc_105,
				BlockParam.AVG_VOLTAGE - TOTAL_CELL_VOLTAGE(10.5),
				TOTAL_CELL_VOLTAGE(0.5));
		}
		else { 
			if (cap_index < 0xa) {
				if (cap_index == 0x0) {
					soc_110 = two_point_diff_calc(
						SOC_I_110V[current_index][0],
						SOC_I_110V[current_index + 1][0] - SOC_I_110V[current_index][0],
						BlockParam.AVG_CURRENT - current_index * CURRENT_A(5),
						CURRENT_A(5));
					soc_105 = two_point_diff_calc(
						SOC_I_105V[current_index][0],
						SOC_I_105V[current_index + 1][0] - SOC_I_105V[current_index][0],
						BlockParam.AVG_CURRENT - current_index * CURRENT_A(5),
						CURRENT_A(5));
					Soc.ZERO_SOC = two_point_diff_calc(
						soc_105,
						soc_110 - soc_105,
						BlockParam.AVG_VOLTAGE - TOTAL_CELL_VOLTAGE(10.5),
						TOTAL_CELL_VOLTAGE(0.5));
				}
				else { 
					soc_110 = two_point_diff_calc(
						SOC_I_110V[current_index][cap_index - 1],
						SOC_I_110V[current_index + 1][cap_index - 1] - SOC_I_110V[current_index][cap_index - 1],
						BlockParam.AVG_CURRENT - current_index * CURRENT_A(5),
						CURRENT_A(5));
					soc_105 = two_point_diff_calc(
						SOC_I_105V[current_index][cap_index - 1],
						SOC_I_105V[current_index + 1][cap_index - 1] - SOC_I_105V[current_index][cap_index - 1],
						BlockParam.AVG_CURRENT - current_index * CURRENT_A(5),
						CURRENT_A(5));
					soc_cap1 = two_point_diff_calc(
						soc_105,
						soc_110 - soc_105,
						BlockParam.AVG_VOLTAGE - TOTAL_CELL_VOLTAGE(10.5),
						TOTAL_CELL_VOLTAGE(0.5));

					soc_110 = two_point_diff_calc(
						SOC_I_110V[current_index][cap_index],
						SOC_I_110V[current_index + 1][cap_index] - SOC_I_110V[current_index][cap_index],
						BlockParam.AVG_CURRENT - current_index * CURRENT_A(5),
						CURRENT_A(5));
					soc_105 = two_point_diff_calc(
						SOC_I_105V[current_index][cap_index],
						SOC_I_105V[current_index + 1][cap_index] - SOC_I_105V[current_index][cap_index],
						BlockParam.AVG_CURRENT - current_index * CURRENT_A(5),
						CURRENT_A(5));
					soc_cap2 = two_point_diff_calc(
						soc_105,
						soc_110 - soc_105,
						BlockParam.AVG_VOLTAGE - TOTAL_CELL_VOLTAGE(10.5),
						TOTAL_CELL_VOLTAGE(0.5));
					Soc.ZERO_SOC = two_point_diff_calc(
						soc_cap1,
						soc_cap2 - soc_cap1,
						Cap.Q25 - CAP_RATING_ARRAY[cap_index - 1],
						CAP_RATING_ARRAY[cap_index] - CAP_RATING_ARRAY[cap_index - 1]); 
				}
			}
			else {
				soc_110 = two_point_diff_calc(
					SOC_I_110V[current_index][0x9],
					SOC_I_110V[current_index + 0x1][0x9] - SOC_I_110V[current_index][0x9], 
					BlockParam.AVG_CURRENT - current_index * CURRENT_A(5), 
					CURRENT_A(5));
				soc_105 = two_point_diff_calc(
					SOC_I_105V[current_index][0x9],
					SOC_I_105V[current_index + 0x1][0x9] - SOC_I_105V[current_index][0x9], 
					BlockParam.AVG_CURRENT - current_index * CURRENT_A(5), 
					CURRENT_A(5));
				Soc.ZERO_SOC = two_point_diff_calc(
					soc_105,
					soc_110 - soc_105,
					BlockParam.AVG_VOLTAGE - TOTAL_CELL_VOLTAGE(10.5),
					TOTAL_CELL_VOLTAGE(0.5));
			}
		}
		if (Soc.ZERO_SOC < 0x0) {
			Soc.ZERO_SOC = 0x0;
		}
		if (abs(Soc.INDEX - Soc.ZERO_SOC) <= 20 || Soc.INDEX <= Soc.ZERO){ 
			Integral.AH_0001_CMP_DIS = Integral.AH_0001_CMP_DIS >> 0x1;
		}
		else {
			Integral.AH_0001_CMP_DIS =
				math_diveder(Soc.ZERO_SOC * Integral.AH_0001_DIS, (int)Soc.INDEX - (int)Soc.ZERO);
			if ((u32)Integral.AH_0001_CMP_DIS < 0x6ddd1) {
				if (Integral.AH_0001_CMP_DIS < 0x1194) {
					Integral.AH_0001_CMP_DIS = 0x1194;
				}
			}
			else {
				Integral.AH_0001_CMP_DIS = 0x6ddd0;
			}
			Integral.AH_0001_CMP_DIS = Integral.AH_0001_CMP_DIS - Integral.AH_0001_DIS;
		}
	}
	return;
}
u16 cap_last_calc(u16 battery_volt, s16 bus_current, u16 cap_array[14][21], u8 current_index)
{
	u32 voltage_index;
	s32 sVar3;
	s32 sVar4;  
	if (current_index * CURRENT_A(5) <= bus_current) {
		if (0xe < current_index) {
			current_index = 0xe;
		}
		for (voltage_index = 0; voltage_index < 0x15; voltage_index++)
		{
			if (Bat.CELL_NUM * VOLT_POINT_ARRAY[voltage_index] > battery_volt)   break;
		}
		if (voltage_index == 0x0) {
			if (current_index == 0x0) {
				return math_diveder(cap_array[0][0] * bus_current , CURRENT_A(5)); 
			}
			else if (current_index < 0xe) {
				return two_point_diff_calc(
					cap_array[current_index - 1][0],
					cap_array[current_index][0] - cap_array[current_index - 1][0],
					bus_current - current_index * CURRENT_A(5),
					CURRENT_A(5)); 
			}
			else {
				return cap_array[current_index - 1][0];
			}
		}
		else if (voltage_index < 0x15) {
			if (current_index == 0x0) {
				sVar3 = math_diveder(bus_current * cap_array[0][voltage_index - 0x1], CURRENT_A(5));
				sVar4 = math_diveder(bus_current * cap_array[0][voltage_index], CURRENT_A(5));
				return  two_point_diff_calc(sVar3, sVar4 - sVar3,
					battery_volt - Bat.CELL_NUM * VOLT_POINT_ARRAY[voltage_index - 1],
					(VOLT_POINT_ARRAY[voltage_index] - VOLT_POINT_ARRAY[voltage_index - 1]) * Bat.CELL_NUM
				); 
			}
			else if (current_index < 0xe) {
				sVar3 = two_point_diff_calc(
					cap_array[current_index - 1][voltage_index - 1],
					cap_array[current_index - 1][voltage_index] - cap_array[current_index - 1][voltage_index - 1],
					battery_volt - Bat.CELL_NUM * VOLT_POINT_ARRAY[voltage_index - 0x1],
					(VOLT_POINT_ARRAY[voltage_index] - VOLT_POINT_ARRAY[voltage_index - 1]) * Bat.CELL_NUM
				);
				sVar4 = two_point_diff_calc(
					cap_array[current_index][voltage_index - 1],
					cap_array[current_index][voltage_index] - cap_array[current_index][voltage_index - 1],
					battery_volt - Bat.CELL_NUM * VOLT_POINT_ARRAY[voltage_index - 0x1],
					(VOLT_POINT_ARRAY[voltage_index] - VOLT_POINT_ARRAY[voltage_index - 1]) * Bat.CELL_NUM
				);
				return  two_point_diff_calc(sVar3, sVar4 - sVar3, bus_current - current_index * CURRENT_A(5), CURRENT_A(5)); 
			}
			else {
				return two_point_diff_calc(
					cap_array[13][voltage_index - 1],
					cap_array[13][voltage_index] - cap_array[13][voltage_index - 1],
					battery_volt - Bat.CELL_NUM * VOLT_POINT_ARRAY[voltage_index - 1],
					(VOLT_POINT_ARRAY[voltage_index] - VOLT_POINT_ARRAY[voltage_index - 1]) * Bat.CELL_NUM
				); 
			}
		}
		else if (current_index == 0x0) {
			return math_diveder(bus_current * cap_array[0][0x14], CURRENT_A(5)); 
		}
		else if (current_index < 0xe) {
			return two_point_diff_calc(
				cap_array[current_index - 1][20],
				cap_array[current_index][20] - cap_array[current_index - 1][20],
				bus_current - current_index * CURRENT_A(5),
				CURRENT_A(5)
			);
		}
		else {
			return    cap_array[current_index - 1][20];
		}
	}
	else {
		return 0;
	} 
}
u16 last_cap_calc(u16 battery_volt, s16 battery_current, u16 battery_cap, u16 cap_dis_once)
{
	u8  cap_index, current_index;
	u16 last_cap_value, Temp_Cap;
	cap_index = cap_index_calc(battery_cap); 
	current_index = math_diveder(battery_current, CURRENT_A(5)); 
	if (cap_index == 0x0) {
		Temp_Cap = cap_last_calc(battery_volt, battery_current, CAP_13_LEFT, current_index);
	}
	else if (cap_index < 0xa) {
		last_cap_value = cap_last_calc(battery_volt, battery_current, CAP_TYPE_ARRAY[cap_index - 1], current_index);
		Temp_Cap = cap_last_calc(battery_volt, battery_current, CAP_TYPE_ARRAY[cap_index], current_index);
		Temp_Cap = two_point_diff_calc(
			last_cap_value, 
			Temp_Cap - last_cap_value,
			battery_cap - CAP_RATING_ARRAY[cap_index - 1],
			CAP_RATING_ARRAY[cap_index] - CAP_RATING_ARRAY[cap_index - 1]
		);  
	}
	else {
		Temp_Cap = cap_last_calc(battery_volt, battery_current, CAP_52_LEFT, current_index);
	}
	Temp_Cap = last_cap_convert_on_temp(Temp_Cap);
	if ((Cap.CAP_LEARN_MAX < Temp_Cap + cap_dis_once) &&
		(cap_dis_once + 0x8 < Cap.CAP_LEARN_MAX)) {
		Temp_Cap = Cap.CAP_LEARN_MAX - cap_dis_once;
	}
	return Temp_Cap;
}
void discharge_zero_detect(void)
{
	u16 TempU16;
	TempU16 = q_curve_inv_calc(Cap.CAP_DIS_ONCE * (u32)0x7e >> 0x7);
	if (Marks.FULL) {
		Cap.CAP_LEARN_MAX = 0x28;
		Cap.CAP_LEARN_MAX += SystemParam.TEMPERATURE < 0x0f ? q_curve_inv_calc(Cap.CAP_DIS_ONCE) : Cap.CAP_DIS_ONCE;
		if (Q_VALUE_MAX < Cap.CAP_LEARN_MAX) {
			Cap.CAP_LEARN_MAX = Q_VALUE_MAX;
		}
		if (TempU16 < Q_VALUE_MAX && TempU16 > Q_VALUE_MIN) {
			Cap.SET_IMPACT = 0x80;
			Cap.CAP_SET = TempU16;
		}
		soc_q_array_rearrange(TempU16, 3);
		if (Marks.RATING_LEARN) {
			Bat.CAP_NOW = (u8)(TempU16 >> 0x3);
		}
		else {
			Marks.RATING_LEARN = 1;
			Bat.CAP_RATING = (u8)(TempU16 >> 0x3);
			Bat.CAP_NOW = Bat.CAP_RATING;
		}
	}
	else if (Marks.FULL_DIS) {
		TempU16 = TempU16 + 4;
		if (TempU16 < Q_VALUE_MAX && TempU16 > Q_VALUE_MIN) {
			Cap.CAP_SET = TempU16;
			Cap.SET_IMPACT = 0x64;
		}
	}
	update_cap_array(TempU16, 1);
	Soc.ZERO_DELTA = 0x7;
	Marks.ZERO = 1;
	Marks.ZERO_START = 1;
}
void soc_key_paramter_init(void)
{
	Marks.VERIFYED = 1;
	Verifyied = 1;
	BlockParam.CHG_CURRENT = 0xc0;
	Cap.CAP_LEARN_MAX = Q_VALUE_MAX;
	Cap.CAP_LEARN_MIN = Q_VALUE_MIN;
}
void uid_save(void)
{
	return;
}
void cap_calc(void)
{
	u16 max_value, min_value, avg_value, i;
	min_value = Cap.ARRAY[1];
	max_value = Cap.ARRAY[1];
	avg_value = Cap.ARRAY[1];
	for (i = 2; i < 5; i++)
	{
		if (Cap.ARRAY[i] >= min_value)
		{
			if (Cap.ARRAY[i] > max_value)
			{
				max_value = Cap.ARRAY[i];
			}
		}
		else
		{
			min_value = Cap.ARRAY[i];
		}
		avg_value += Cap.ARRAY[i];
	}
	avg_value = (avg_value - max_value - min_value) >> 1;
	Cap.Q25 = (avg_value + Cap.ARRAY[0]) >> 1;
	Cap.QNOW = q_curve_calc(Cap.Q25, 1);
	if (Cap.CAP_SET < Q_VALUE_MAX && Cap.CAP_SET >  Q_VALUE_MIN)          // capset  has been limit for zero?  should check asm. ....the truth is -0x31 cmp  0x187 has two constrant.  >0x30 && <1b8
	{
		Cap.QNOW = (Cap.QNOW + q_curve_calc(Cap.CAP_SET, 1)) >> 1;
	}
	Integral.AH_0001_CHG = (Cap.QNOW * SOC_1_Q_CONST_CHG) >> SOC_1_Q_SHIFT;
	Integral.AH_0001_DIS = (Cap.QNOW * SOC_1_Q_CONST_DIS) >> SOC_1_Q_SHIFT;
	Integral.AH_0001_CMP_DIS = 0;
	Integral.AH_0001_CMP_CHG = 0;
}
u8 cap_index_calc(u16 cap)
{
	unsigned char i;
	for (i = 0; i < 10; i++)
	{
		if (cap <= CAP_RATING_ARRAY[i])  return i;
	}
	return i;
}
void cap_learn_insert(void)
{
	q_now_constrant();
	Cap.Q25 = q_curve_inv_calc(Cap.QNOW);
	soc_q_array_rearrange(Cap.Q25, 1);
}
void update_cap_array(u16 temp_cap, u8 incflg)
{
	u8 i;
	q_25_constrant(&temp_cap);
	for (i = 0; i < 5; i++)
	{
		if (incflg)
		{
			if (temp_cap > Cap.ARRAY[i])
			{
				Cap.ARRAY[i] = temp_cap;
			}
		}
		else
		{
			if (temp_cap < Cap.ARRAY[i])
			{
				Cap.ARRAY[i] = temp_cap;
			}
		}
	}
}
void q_now_constrant(void)
{
	if (Cap.QNOW > Cap.CAP_LEARN_MAX)
	{
		Cap.QNOW = Cap.CAP_LEARN_MAX;
	}
	else if (Cap.QNOW < Q_VALUE_MIN)
	{
		Cap.QNOW = Q_VALUE_MIN;
	}
}
void q_25_constrant(u16* newQ)
{
	if (*newQ > Q_VALUE_MAX)
	{
		*newQ = Q_VALUE_MAX;
	}
	else if (*newQ < Cap.CAP_LEARN_MIN)
	{
		*newQ = Cap.CAP_LEARN_MIN;
	}
}
void soc_zero_decrease_process(void)   //in charge_process
{
	SocCounter.SOC_0_CMP_CNT++;
	if (SocCounter.SOC_0_CMP_CNT >= SocCounter.SOC_0_CMP_TIME)
	{
		SocCounter.SOC_0_CMP_CNT = 0;
		Soc.ZERO--;
	}
	if (SystemParam.AVG_CURRENT > BlockParam.C01_CURRENT)
	{
		u8 TempU8 = math_diveder(SystemParam.AVG_CURRENT, BlockParam.C01_CURRENT);
		if (TempU8 > 5)
		{
			TempU8 = 5;
		}
		SocCounter.SOC_0_CMP_CNT += TempU8;
	}
}
void soc_q_array_rearrange(u16 newQ, u8 num)
{
	u8 i;
	q_25_constrant(&newQ);
	for (i = 4; i >= num; i--)
	{
		Cap.ARRAY[i] = Cap.ARRAY[i - num];
	}
	for (i = 0; i < num; i++)
	{
		Cap.ARRAY[i] = newQ;
	}
}
u16 last_cap_convert_on_temp(u16 cap)
{
	if (SystemParam.TEMPERATURE < 0x5) {
		if (-0x14 < SystemParam.TEMPERATURE) {
			return (u16)((u32)cap * (SystemParam.TEMPERATURE + 0x7b) * 0x200 >> 0x10);
		}
		cap = (u16)((u32)cap * 0x6b >> 0x7);
	}
	return cap;
}  


void discharge_soc_compensation(void)
{
	u8 current_index;

	markstype mVar1;
	s16 sVar2;
	u16 uVar3;
	s32 sVar4;
	u32 uVar5;
	int iVar6;
	s32 sVar7;
	u16 uVar8;
	int iVar9;
	int iVar10;
	u32 extraout_r3 = 0;
	int iVar11;
	int iVar12;
	u16 local_28[0x4];
	int local_20;
	int local_1c;

	/* Unresolved local var: s16 Temp_Cap@[???]
	   Unresolved local var: u8 current_index@[???] */
	if (Verifyied == 0) {
		Soc.SHOW = 0x37;
		return;
	}
	if ((int)Integral.DIS_AH_BLOCK >= Integral.SUM_CURRENT)
	{
		if (BlockParam.MAX_CURRENT < SystemParam.AVG_CURRENT)      ////?  zero init,can be set?
		{
			BlockParam.MAX_CURRENT = SystemParam.AVG_CURRENT;
		}
		else if (BlockParam.MIN_CURRENT > SystemParam.AVG_CURRENT)
		{
			BlockParam.MIN_CURRENT = SystemParam.AVG_CURRENT;      ////?  zero init,can be set?
		}
		return;
	}

	Marks.BLOCK = 0;
	SocCounter.DIS_BLOCK_CNT = 0;
	if (Integral.DIS_AH_BLOCK < AH(0.11)) {
		Integral.DIS_AH_BLOCK = Integral.DIS_AH_BLOCK + AH(0.005);
	}
	sVar4 = math_diveder(Integral.SUM_VOLTAGE, SocCounter.BLOCK_CNT);
	BlockParam.AVG_VOLTAGE = (s16)sVar4;
	sVar4 = math_diveder(Integral.SUM_CURRENT, SocCounter.BLOCK_CNT);
	BlockParam.AVG_CURRENT = (s16)sVar4;
	sVar2 = BlockParam.MAX_CURRENT;

	if (Cap.CAP_SET >= 0x1e1)
	{
		return;
	}

	SocCounter.DIS_LEARN_CNT = SocCounter.DIS_LEARN_CNT + 1;


	uVar5 = math_diveder(BlockParam.AVG_CURRENT, CURRENT_A(5));
	current_index = uVar5;
	iVar6 = (int)BlockParam.AVG_VOLTAGE;
	uVar8 = (u16)Bat.CELL_NUM;

	if (BlockParam.AVG_VOLTAGE <= TOTAL_CELL_VOLTAGE(11) || current_index >= 0x0b)  //b?f
	{
		if (iVar6 < (int)(u32)(u16)(uVar8 * 0x2a0)) {
			if (BlockParam.AVG_CURRENT < 0x281) {
				// [AUTO_REPLACED] if (-0x1 < (int)Marks << 0x1c) {
				if (Marks.ZERO) {
					// [AUTO_REPLACED] Marks = (markstype)((u32)Marks | 0x8);
					Marks.ZERO = 1;
					Soc.ZERO_DELTA = 0x3;
				}
			}
			else {
				/* Unresolved local var: s32 temp_current@[???] */
				iVar10 = (BlockParam.AVG_CURRENT + -0x140) >> 0x6;
				if ((int)(u32)(u16)(uVar8 * 0x289) < iVar6) {
					Soc.ZERO_DELTA = 0x0;
					cap_dec_undervolt(BlockParam.AVG_CURRENT, Cap.Q25);
				}
				else {
					if (0x5 < iVar10) {
						iVar10 = 0x5;
					}
					Soc.ZERO_DELTA = 0x6 - (short)iVar10;
					// [AUTO_REPLACED] Marks = (markstype)((u32)Marks | 0x8);
					Marks.ZERO = 1;
				}
			}
		}
		else if (iVar6 <= (int)(u32)(u16)(uVar8 * 0x2c0)) {
			cap_dec_undervolt(BlockParam.AVG_CURRENT, Cap.Q25);
		} 
		return;
	}
	

	Marks.ZERO = 0;
	Integral.AH_0001_CMP_DIS = 0x0;
	Cap.CAP_GUSS_LAST = last_cap_calc(BlockParam.AVG_VOLTAGE, BlockParam.AVG_CURRENT, Cap.Q25, Cap.CAP_DIS_ONCE);	  

	Integral.AH_0001_DIS = Cap.CAP_GUSS_LAST * 0x23280 >> 0x9;
	if (Soc.INDEX < 0x34) {
		Integral.AH_0001_DIS = math_diveder(Integral.AH_0001_DIS << 0xa, 0x1e);
	}
	else {
		Integral.AH_0001_DIS = math_diveder(Integral.AH_0001_DIS << 0xa, (int)Soc.INDEX);
	}
	if (0x6ddd0 < Integral.AH_0001_DIS) {
		Integral.AH_0001_DIS = 0x6ddd0;
	}
	if (Cap.CAP_SET != 0x0) {
		/* Unresolved local var: s32 TempS32@[???] */
		uVar3 = q_curve_calc(Cap.CAP_SET, '\x01');
		iVar6 = (int)(short)uVar3;
		uVar5 = (u32)Cap.CAP_DIS_ONCE;
		if (iVar6 < (int)(uVar5 + 0x8)) {
			Cap.CAP_SET = q_curve_inv_calc(Cap.CAP_DIS_ONCE + 0x10);
			if (0x0 < Cap.SET_IMPACT) {
				Cap.SET_IMPACT = Cap.SET_IMPACT + -0x1;
			}
		}
		else if (((iVar6 + 0x20 < (int)((int)Cap.CAP_GUSS_LAST + uVar5)) &&
			(iVar6 * 0x64 >> 0x7 < (int)uVar5)) &&
			(Cap.CAP_SET = q_curve_inv_calc(uVar3 + 0x10), 0x0 < Cap.SET_IMPACT)) {
			Cap.SET_IMPACT = Cap.SET_IMPACT + -0x1;
		}
		iVar6 = iVar6 * 0x23280 >> 0x9;
		if (0x6ddd0 < iVar6) {
			iVar6 = 0x6ddd0;
		}
		sVar4 = math_diveder(iVar6 << 0xa, (int)Soc.FULL);
		// [AUTO_REPLACED] if (((int)Marks << 0x1b < 0x0) || (Soc.DIS_SOC != 0x0)) {
		if (Marks.FULL_DIS || (Soc.DIS_SOC != 0x0)) {
			/* Unresolved local var: s16 percentage@[???] */
			if (Soc.INDEX < 0x333) {
				if (Soc.INDEX < 0xcd) {
					iVar6 = (int)(short)(Cap.SET_IMPACT + -0x59);
				}
				else {
					sVar7 = math_diveder((short)(0x333 - Soc.INDEX) * 0x59, 0x266);
					iVar6 = (int)(short)(Cap.SET_IMPACT - (short)sVar7);
				}
			}
			else {
				iVar6 = (int)Cap.SET_IMPACT;
			}
			// [AUTO_REPLACED] if (-0x1 < (int)Marks << 0x1b) {
			if (Marks.FULL_DIS) {
				iVar6 = (int)(short)((short)iVar6 + -0x32);
			}
			if (iVar6 < 0x81) {
				if (iVar6 < 0x0) {
					iVar6 = 0x0;
				}
			}
			else {
				iVar6 = 0x80;
			}
			Integral.AH_0001_DIS = (0x80 - iVar6) * Integral.AH_0001_DIS + iVar6 * sVar4 >> 0x7;
		}
	}
	
	
	
	
	if (Marks.FULL_DIS) {
		if ((int)(short)((Cap.CAP_GUSS_TOTAL_DIS - Cap.CAP_GUSS_LAST) - Cap.CAP_DIS_ONCE) + 0xfU <
			0x1f) {
			Cap.QNOW = (u16)(Cap.CAP_GUSS_LAST + Cap.CAP_DIS_ONCE + Cap.CAP_GUSS_TOTAL_DIS) >>
				0x1;
			cap_learn_insert();
		}
		Cap.CAP_GUSS_TOTAL_DIS = Cap.CAP_GUSS_LAST + Cap.CAP_DIS_ONCE;
	}
	else if (Soc.DIS_SOC == 0x0) {
		Cap.CAP_GUSS_LAST = Cap.CAP_CHG_LAST - Cap.CAP_DIS_ONCE;
		iVar6 = (int)Cap.CAP_GUSS_LAST;
		if ((iVar6 < 0x11) || (Soc.INDEX < 0x1)) {
			if (iVar6 < 0x0) {
				Cap.CAP_GUSS_LAST = 0x0;
			}
		}
		else {
			Integral.AH_0001_DIS = iVar6 * 0x23280 >> 0x9;
			Integral.AH_0001_DIS = math_diveder(Integral.AH_0001_DIS << 0xa, (int)Soc.INDEX);
		}
	}
	else {
		Cap.CAP_GUSS_TOTAL_DIS = Cap.CAP_GUSS_LAST + Cap.CAP_DIS_ONCE;
		if ((int)(u32)Cap.QNOW < (int)Cap.CAP_GUSS_TOTAL_DIS) {
			SocCounter.DIS_NOFULL_CNT = SocCounter.DIS_NOFULL_CNT + 0x1;
			if (0x3 < SocCounter.DIS_NOFULL_CNT) {
				SocCounter.DIS_NOFULL_CNT = '\0';
				Cap.QNOW = (u16)(((u32)Cap.QNOW + (int)Cap.CAP_GUSS_TOTAL_DIS & 0xffff) >> 0x1);
				cap_learn_insert();
			}
		}
		else {
			SocCounter.DIS_NOFULL_CNT = '\0';
		}
		mVar1 = Marks;
		// [AUTO_REPLACED] if ((int)Marks * 0x400000 < 0x0) {
		if (Marks.CAP_INC) {
			BlockParam.MAX_VOLTAGE = Cap.AVG_VOLTAGE[0x0];
			BlockParam.MAX_CURRENT = Cap.AVG_CURRENT[0x0];
			local_20 = (int)Cap.DIS_CAP[0x0];
			Cap.AVG_CURRENT[0x0] = BlockParam.AVG_CURRENT;
			Cap.AVG_VOLTAGE[0x0] = BlockParam.AVG_VOLTAGE;
			Cap.DIS_CAP[0x0] = Cap.CAP_DIS_ONCE;
			Cap.INDEX = '\x01';
		}
		else {
			local_20 = (int)(short)(((Cap.CAP_GUSS_LAST + Cap.CAP_DIS_ONCE) - Cap.CAP_CHG_ONCE) -
				Cap.CAP_GUSS_IN_CURRENT_CHARGE);
			out.remsoc = local_20;
			if (abs(local_20) > 0x08){  //(0x10 < local_20 + 0x8U) {
				Cap.AVG_CURRENT[Cap.INDEX] = BlockParam.AVG_CURRENT;
				Cap.AVG_VOLTAGE[Cap.INDEX] = BlockParam.AVG_VOLTAGE;
				Cap.DIS_CAP[Cap.INDEX] = Cap.CAP_DIS_ONCE;
				Cap.INDEX = Cap.INDEX + -0x1;
				if (Cap.INDEX < '\0') {
					// [AUTO_REPLACED] Marks = (markstype)((u32)mVar1 | 0x200);
					Marks.CAP_INC = 1;
				}
				Cap.CAP_CHG_ONCE = Cap.CAP_DIS_ONCE;
				Cap.CAP_GUSS_IN_CURRENT_CHARGE = Cap.CAP_GUSS_LAST;
			}
		}
		// [AUTO_REPLACED] if ((int)Marks << 0x16 < 0x0) {
		if (Marks.CAP_INC) {
			/* Unresolved local var: s16[3] last_cap@[???] */
			uVar5 = 0x0;
			do {
				uVar3 = last_cap_calc(Cap.AVG_VOLTAGE[uVar5], Cap.AVG_CURRENT[uVar5], Cap.Q25,
					Cap.DIS_CAP[uVar5]);
				local_28[uVar5] = uVar3;
				uVar5 = uVar5 + 0x1 & 0xff;
			} while (uVar5 < 0x3);
			iVar6 = (int)(short)local_28[0x0];
			iVar10 = (int)Cap.DIS_CAP[0x0];
			uVar5 = (u32)Cap.QNOW;
			if ((((int)uVar5 < iVar6 + iVar10) &&
				((int)uVar5 < (int)(short)local_28[0x1] + (int)Cap.DIS_CAP[0x1])) &&
				((int)uVar5 < (int)(short)local_28[0x2] + (int)Cap.DIS_CAP[0x2])) {
				Cap.QNOW = Cap.QNOW + 0x8;
				cap_learn_insert();
			}
			else {
				iVar9 = (int)(short)local_28[0x2] + (int)Cap.DIS_CAP[0x2];
				iVar11 = (int)Cap.DIS_CAP[0x1];
				iVar12 = (int)(short)local_28[0x1];
				local_1c = iVar11 + iVar12;
				extraout_r3 = 0;
				if ((local_1c + 0x4 < iVar9) && (iVar10 + iVar6 + 0x4 < iVar12 + iVar11)) {
					Cap.QNOW = Cap.QNOW - 0x8;
					cap_learn_insert(); extraout_r3 = 2;
				}
				else if ((iVar9 + 0x4 < local_1c) && (iVar12 + iVar11 + 0x4 < iVar10 + iVar6)) {
					Cap.QNOW = Cap.QNOW + 0x8;
					cap_learn_insert(); extraout_r3 = 2;
				}
				else if ((iVar9 + 0x2 < local_1c) && (iVar12 + iVar11 + 0x2 < iVar10 + iVar6)) {
					Cap.QNOW = Cap.QNOW + 0x4;
					cap_learn_insert(); extraout_r3 = 1;
				}
				else if ((local_1c + 0x2 < iVar9) && (iVar10 + iVar6 + 0x2 < iVar12 + iVar11)) {
					Cap.QNOW = Cap.QNOW - 0x4;
					cap_learn_insert(); extraout_r3 = 1;
				}
				else if (iVar10 + iVar6 + 0x8 < iVar9) {
					Cap.QNOW = Cap.QNOW - 0x2; extraout_r3 = 2;
					cap_learn_insert();
				}
				else if (iVar9 + 0x8 < iVar10 + iVar6) {
					Cap.QNOW = Cap.QNOW + 0x2; extraout_r3 = 2;
					cap_learn_insert();
				}
			}
			q_now_constrant();
			if ((extraout_r3 != 0x0) && (uVar5 = extraout_r3, '\0' < Cap.INDEX)) {
				for (; uVar5 != 0x0; uVar5 = uVar5 - 0x1 & 0xff) {
					Cap.AVG_CURRENT[uVar5] = Cap.AVG_VOLTAGE[uVar5 -1];
					Cap.AVG_VOLTAGE[uVar5] = Cap.AVG_VOLTAGE[uVar5 - 0x1];
					Cap.DIS_CAP[uVar5] = Cap.AVG_CURRENT[uVar5 -1];
				}
				Cap.AVG_CURRENT[0x0] = BlockParam.AVG_CURRENT;
				Cap.AVG_VOLTAGE[0x0] = BlockParam.AVG_VOLTAGE;
				Cap.DIS_CAP[0x0] = Cap.CAP_DIS_ONCE;
			}
		}
	}
	
	
	if (Integral.AH_0001_DIS < 0x6ddd1) {
		if (Integral.AH_0001_DIS < 0x1194) {
			Integral.AH_0001_DIS = 0x1194;
		}
	}
	else {
		Integral.AH_0001_DIS = 0x6ddd0;
	} 



}

