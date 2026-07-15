#include "main.h"
#include "soc.h"
#include "toolfun.h"
#include "userfun.h"



s16 soc_show_calc(s16 index)
{
	s16 value;
	
	if (index >= Soc.ZERO)
	{
		value = index - Soc.ZERO;
		if (SocShowAllowIncState)
		{
			value = math_diveder(value*SOC_SHOW_MAX,Soc.FULL - Soc.ZERO);
		}
		else
		{
			value = math_diveder(value*SOC_SHOW_MAX + ((Soc.FULL - Soc.ZERO)*120>>7),Soc.FULL - Soc.ZERO);
		}
		
		if (value > SOC_SHOW_MAX)
		{
			value = SOC_SHOW_MAX;
		}
	}
	else
	{
		value = 0;
	}
	
	return value;
}

void soh_calc(void)
{	
	if (His.CIRCLE < 50)
	{
		Soc.SOH = SOH_MAX;
	}
	else if (His.CIRCLE < 100)
	{
		if (Soc.SOH > SOH_MIN && Soc.SOH > math_diveder(Bat.CAP_NOW*SOH_MAX*11,Bat.CAP_RATING*10))
		{
			Soc.SOH--;
		}
	}
	else
	{
		if (Soc.SOH > SOH_MIN && Soc.SOH > math_diveder(Bat.CAP_NOW*SOH_MAX,Bat.CAP_RATING))
		{
			Soc.SOH--;
		}
	}
}

void soc_show_process(void)
{
	static s16 show_cnt;
	
	if (show_cnt < (SocShowAllowIncState?20:4) && Marks.SHOW_DEC == 0)
	{
		show_cnt++;
	}
	else
	{
		show_cnt = soc_show_calc(Soc.INDEX);
		
		if (Soc.SHOW > show_cnt)
		{
			if (!SocShowAllowIncState)
			{
				Soc.SHOW--;
			}
			
			Marks.SHOW_FULL = 0;
		}
		else if (Soc.SHOW < show_cnt)
		{
			if (SocShowAllowIncState)
			{
				if (++Soc.SHOW == SOC_SHOW_VALUE(0.8))
				{
					soh_calc();
				}
				
				if (Soc.SHOW == SOC_SHOW_MAX)
				{
					Marks.SHOW_FULL	 		= 1;
					SystemFlg.SOC_SAVE 	= 1;
				}
			}
		}
		Marks.SHOW_DEC = 0;
		show_cnt			 = 0;
	}
}


void cap_learn_param_init(void)
{
	Marks.RATING_LEARN 					= 0;
	His.CAPSUM 									= 0;
	His.CAPSUM_LST							= 0;
	Cap.CAP_LEARN_INDEX					= 0;
}

u16 q_curve_calc(u16 q_25, u8 full)
{
	u16 value;

	if (SystemParam.TEMPERATURE >= 40)
	{
		value = math_diveder(q_25*110,100);
	}
	else if (SystemParam.TEMPERATURE >= 30)
	{
		value = math_diveder(q_25*(70+SystemParam.TEMPERATURE),100);
	}
	else if (SystemParam.TEMPERATURE >= 12)
	{
		value = q_25;
	}
	else if (SystemParam.TEMPERATURE >= 0)
	{
		value = math_diveder(q_25*(88+SystemParam.TEMPERATURE),100);
	}
	else if (SystemParam.TEMPERATURE >= -5)
	{
		value = math_diveder(q_25*(88+SystemParam.TEMPERATURE*4),100);
	}
	else if (SystemParam.TEMPERATURE >= -20)
	{
		value = math_diveder(q_25*(45+(SystemParam.TEMPERATURE+20)*6/4),100);
	}
	else
	{
		value = math_diveder(q_25*45,100);
	}
	
	if (value > Cap.CAP_LEARN_MAX)
	{
		value = Cap.CAP_LEARN_MAX;
	}
	else if (value < Q_VALUE_MIN && full)
	{
		value = Q_VALUE_MIN;
	}
	return value;
}

u16 q_curve_inv_calc(u16 cap_now)
{
	u16 value;
	
	if (SystemParam.TEMPERATURE >= 40)
	{
		math_diveder(cap_now*100, 110);
	}
	else if (SystemParam.TEMPERATURE >= 30)
	{
		value = math_diveder(cap_now*100, (70+SystemParam.TEMPERATURE));
	}
	else if (SystemParam.TEMPERATURE >= 12)
	{
		value = cap_now;
	}
	else if (SystemParam.TEMPERATURE >= 0)
	{
		value = math_diveder(cap_now*100, (88+SystemParam.TEMPERATURE));
	}
	else if (SystemParam.TEMPERATURE >= -5)
	{
		value = math_diveder(cap_now*100, (88+SystemParam.TEMPERATURE*4));
	}
	else if (SystemParam.TEMPERATURE >= -20)
	{
		value = math_diveder(cap_now*100, (45+(SystemParam.TEMPERATURE + 20)*6/4));
	}
	else
	{
		value = math_diveder(cap_now*100,45);
	}
	
	if (value > Q_VALUE_MAX)
	{
		value = Q_VALUE_MAX;
	}
	else if (value < Cap.CAP_LEARN_MIN)
	{
		value = Cap.CAP_LEARN_MIN;
	}
	return value;
}

s16 soe_full_calc(u16 q_value)
{
	return q_value*Bat.CELL_NUM*12>>3;
}

s16 soe_calc_by_soc(s16 soc_show, s16 soe_full)
{
	return  math_diveder(soe_full*soc_show,SOC_SHOW_MAX);
}

void soe_init(void)
{
	Soe.FULL = soe_full_calc(Cap.QNOW);
	Soe.SOE  = soe_calc_by_soc(Soc.SHOW, Soe.FULL);
	Soe.INTEGRAL 			= 0;
	Soe.HOURS_12_CNT 	= 0;
}

void soc_reset(u8 cap, u8 cell_num, u8 cap_set)
{
	u8 i;
		
    
	#ifdef CSTM_SET_CAP
	if (cap_set && cap > CAP_RATING_MIN)
	{
		Cap.SET_IMPACT						= 100;
		Cap.CAP_SET								= cap*Q_UNIT*140>>7;
		Cap.CAP_LEARN_MAX					= Cap.CAP_SET*150>>7;
		if (Cap.CAP_LEARN_MAX > Q_VALUE_MAX)
		{
			Cap.CAP_LEARN_MAX = Q_VALUE_MAX;
		}
	}
	else
	{
		Cap.CAP_SET			= 0;
		Cap.SET_IMPACT 	= 0;
		cap 						= CAP_RATING_DEFAULT;
	}
	#else
	Cap.CAP_SET			= 0;
	Cap.SET_IMPACT 	= 0;
	#endif
	
	for(i = 0; i < 5; i++)
	{
		Cap.ARRAY[i] = cap*Q_UNIT;
	}
	Bat.CAP_RATING 							= cap;
	
	Bat.CAP_NOW									= Bat.CAP_RATING;
	Bat.CELL_NUM								= cell_num;
	SystemParam.TEMPERATURE 		= SystemParam.TEMP_NOW;
	Soc.FULL 										= SOC_INDEX_MAX;
	Soc.INDEX 									= soc_ocv_calc(SystemParam.AVG_VOLTAGE);
	
	#ifdef SHOW_DEC_SLOW
	if (Soc.INDEX > Soc.DEC_SHOW)
	{
		Soc.ZERO = Soc.DEC_SHOW - Soc.INDEX;
	}
	else
	{
		Soc.ZERO = 0;
	}
	#else
	Soc.ZERO 										= 0;
	#endif
	
	Soc.SHOW 										= soc_show_calc(Soc.INDEX);
	Soc.OCV_SOC 								= Soc.INDEX;
	Soc.DIS_SOC									= Soc.INDEX;
	Soc.SOH 										= SOH_MAX;
	Soc.CIRCLE									= 0;
	His.CIRCLE 									= 0;
	Cap.CAP_DIS_ONCE 						= 0;
	Cap.CAP_CHG_ONCE 						= 0;
	Cap.CAP_CHG_LAST 						= 0;
	Marks.FULL									= 1;
	Marks.FULL_DIS 							= 1;
	SystemFlg.SOC_SAVE					= 1;
	cap_learn_param_init();
	cap_calc();//when reset
	soe_init();
}

void soc_init(void)
{
	
}
void goto_soc_ready_state(void)
{
	GotoReadyState();
	SystemFlg.SOC_SAVE	= 1;
	
	SocCounter.STATE_SW_CNT_1 = 0;
	SocCounter.STATE_SW_CNT_2 = 0;
	SocCounter.EIGHT_HOUR 		= 0;
	SocCounter.DIS_TIME_CNT		= 0;
	SocCounter.BLOCK_CNT			= 0;
	
	Integral.SUM_VOLTAGE 			= 0;
	Integral.SUM_CURRENT 			= 0;
	Integral.AH_LAST 					= Integral.AH;
	
	Marks.A_HOUR 							= 0;
	
	//use_record_end();
}

void block_voltage_current_calc(void)
{
	Integral.SUM_VOLTAGE += SystemParam.AVG_VOLTAGE;
	Integral.SUM_CURRENT += SystemParam.AVG_CURRENT;
	if (++SocCounter.BLOCK_CNT >= 16)
	{
		BlockParam.AVG_CURRENT 	= Integral.SUM_CURRENT>>4;
		BlockParam.AVG_VOLTAGE	= Integral.SUM_VOLTAGE>>4;
		SocCounter.BLOCK_CNT 		= 0;
		Integral.SUM_VOLTAGE 		= 0;
		Integral.SUM_CURRENT 		= 0;
	}
}

void block_voltage_current_calc_in_discharge(void)
{
	Integral.SUM_VOLTAGE += SystemParam.AVG_VOLTAGE;
	Integral.SUM_CURRENT -= SystemParam.AVG_CURRENT;
	SocCounter.BLOCK_CNT++;
	
	if (charge_current_larger(2.0) || discharge_current_larger(75.0))
	{
		if (++SocCounter.DISCHG_OV_2A > SECONDS(5))
		{
			if (SocCounter.DIS_TIME_CNT > MINITES(1.6))
			{
				SocCounter.DIS_TIME_CNT	= MINITES(1.6);
			}
		}
	}
	else
	{
		SocCounter.DISCHG_OV_2A = 0;
	}
	
	if (BlockParam.MAX_CURRENT  > CURRENT_A(20.0) + BlockParam.MIN_CURRENT || 
			SocCounter.DIS_TIME_CNT < MINITES(2.0) || 
			SocCounter.BLOCK_CNT > MINITES(3.5) || 
			discharge_current_smaller(0.85))
	{
		Marks.BLOCK 	= 0;
	}
	
	if (Marks.BLOCK == 0)
	{
		Integral.SUM_VOLTAGE 		= 0;
		Integral.SUM_CURRENT 		= 0;
		SocCounter.BLOCK_CNT 		= 0;
		Marks.BLOCK							= 1;
		BlockParam.MAX_CURRENT	= SystemParam.AVG_CURRENT;
		BlockParam.MIN_CURRENT	= SystemParam.AVG_CURRENT;
	}
}

void block_voltage_current_calc_in_charge(void)
{
	Integral.SUM_VOLTAGE += SystemParam.AVG_VOLTAGE;
	Integral.SUM_CURRENT += SystemParam.AVG_CURRENT;
	SocCounter.BLOCK_CNT++;
	if (Marks.CHG_VOLT)
	{		
		if (SystemParam.AVG_VOLTAGE <= TOTAL_CELL_VOLTAGE(13.75))
		{
			if (++SocCounter.DISCHG_OV_2A >= SECONDS(20))
			{
				Marks.CHG_VOLT = 0;
			}
		}
		else 
		{
			SocCounter.DISCHG_OV_2A = 0;
			if (charge_current_smaller(1.5) && SystemParam.AVG_VOLTAGE >= TOTAL_CELL_VOLTAGE(14.25))
			{
				if (SocCounter.DIS_TIME_CNT < MINITES(15))
				{
					SocCounter.DIS_TIME_CNT++;
				}
				else
				{
					Marks.CHG_HIGH = 1;
				}
			}
			else
			{
				SocCounter.DIS_TIME_CNT = 0;
			}
		}
	}
	else
	{
		if ((SystemParam.AVG_VOLTAGE >= TOTAL_CELL_VOLTAGE(13.85) && 
				(SystemParam.AVG_CURRENT < BlockParam.C01_CURRENT || SystemParam.AVG_CURRENT < (BlockParam.CHG_CURRENT>>1))) ||
				(SystemParam.AVG_VOLTAGE >= TOTAL_CELL_VOLTAGE(13.90)))
		{
			if (++SocCounter.DISCHG_OV_2A >= MINITES(1.5))
			{
				Marks.CHG_VOLT = 1;
			}
		}
		else
		{
			SocCounter.DISCHG_OV_2A = 0;
		}
		
		if (charge_current_smaller(1.0))
		{
			if (SocCounter.CHG_CNT_30S < SECONDS(40))
			{
				SocCounter.CHG_CNT_30S++;
			}
		}
		else
		{
			SocCounter.CHG_CNT_30S = 0;
		}
		
		if (SocCounter.CHG_TIME_CNT < MINITES(20) || 
				SocCounter.BLOCK_CNT > MINITES(30) || 
				SystemParam.AVG_VOLTAGE > TOTAL_CELL_VOLTAGE(14.0) || 
				SocCounter.CHG_CNT_30S >= SECONDS(20))
		{
			Marks.BLOCK = 0;
		}
	}
	
	if (Marks.BLOCK == 0)
	{
		Integral.SUM_VOLTAGE 		= 0;
		Integral.SUM_CURRENT 		= 0;
		SocCounter.BLOCK_CNT 		= 0;
		Marks.BLOCK							= 1;
	}
}

u16 chg_volt_current_diff(u16 small_value, u16 big_value, u16 index)
{
	if (index == 0)
	{
		big_value = two_point_diff_calc(small_value,big_value - small_value, CURRENT_A(1) - BlockParam.AVG_CURRENT,CURRENT_A(1.0));
	}
	else if (index <= 6)
	{
		big_value = two_point_diff_calc(small_value,big_value - small_value, CURRENT_A(1) + (CURRENT_A(index)>>1) - BlockParam.AVG_CURRENT,CURRENT_A(0.5));
	}
	else
	{
		if (index == 7)
		{
			index = CURRENT_A(6);
		}
		else if (index == 8)
		{
			index = CURRENT_A(8);
		}
		else if (index == 9)
		{
			index = CURRENT_A(10);
		}
		big_value = two_point_diff_calc(small_value,big_value - small_value, index - BlockParam.AVG_CURRENT,CURRENT_A(2));
	}
	
	return big_value;
}

u16 chg_volt_limit_calc(u8 currentindex, u8 voltageindex, const u16* soc_array)
{	
	u16 TempU16, U16Tmp, U16Temp;
	
	if (BlockParam.AVG_CURRENT < 0)
	{
		if (voltageindex >= CHG_VOLT_X - 1)
		{
			return *(soc_array + CHG_VOLT_X - 1);
		}
		else
		{
			TempU16 = *(soc_array + voltageindex + 1);
			U16Tmp 	= *(soc_array + voltageindex);
			TempU16 = two_point_diff_calc(U16Tmp,TempU16-U16Tmp,BlockParam.AVG_VOLTAGE - CHG_VOLT_ARRAY[voltageindex]*Bat.CELL_NUM,CHG_VOLT_ARRAY[voltageindex + 1]*Bat.CELL_NUM - CHG_VOLT_ARRAY[voltageindex]*Bat.CELL_NUM);
		}
	}
	else if (currentindex >= 10)
	{
		if (voltageindex >= CHG_VOLT_X - 1)
		{
			return *(soc_array + CHG_VOLT_X*CHG_VOLT_Y - 1);
		}
		else
		{
			TempU16 = *(soc_array + CHG_VOLT_X*(CHG_VOLT_Y - 1) + voltageindex + 1);
			U16Tmp	= *(soc_array + CHG_VOLT_X*(CHG_VOLT_Y - 1) + voltageindex);
			TempU16 = two_point_diff_calc(U16Tmp,TempU16-U16Tmp,BlockParam.AVG_VOLTAGE - CHG_VOLT_ARRAY[voltageindex]*Bat.CELL_NUM,CHG_VOLT_ARRAY[voltageindex + 1]*Bat.CELL_NUM - CHG_VOLT_ARRAY[voltageindex]*Bat.CELL_NUM);
		}
	}
	else if (voltageindex >= CHG_VOLT_X - 1)
	{
		TempU16 = *(soc_array + CHG_VOLT_X*currentindex + CHG_VOLT_X - 1);
		U16Tmp	= *(soc_array + CHG_VOLT_X*(currentindex + 1) + CHG_VOLT_X - 1);
		
		TempU16 = chg_volt_current_diff(U16Tmp,TempU16,currentindex);
	}
	else
	{
		TempU16 = *(soc_array + CHG_VOLT_X*currentindex + voltageindex);
		U16Tmp	= *(soc_array + CHG_VOLT_X*(currentindex + 1) + voltageindex);
		U16Temp = chg_volt_current_diff(U16Tmp,TempU16,currentindex);
		
		TempU16 = *(soc_array + CHG_VOLT_X*currentindex + voltageindex + 1);
		U16Tmp	= *(soc_array + CHG_VOLT_X*(currentindex + 1) + voltageindex + 1);
		TempU16 = chg_volt_current_diff(U16Tmp,TempU16,currentindex);
		
		TempU16 = two_point_diff_calc(U16Temp,TempU16-U16Temp,BlockParam.AVG_VOLTAGE - CHG_VOLT_ARRAY[voltageindex]*Bat.CELL_NUM,CHG_VOLT_ARRAY[voltageindex + 1]*Bat.CELL_NUM - CHG_VOLT_ARRAY[voltageindex]*Bat.CELL_NUM);
	}
	
	return TempU16;
}

void charge_soc_compensation_in_voltage(void)
{
	u32 TempU32;
	u16 showsoc,lowlimit,highlimit;
	u8 currentindex, voltageindex;
	
	if (Marks.VERIFYED)
	{
		BlockParam.AVG_CURRENT = math_diveder(Integral.SUM_CURRENT,SocCounter.BLOCK_CNT);
		BlockParam.AVG_VOLTAGE = math_diveder(Integral.SUM_VOLTAGE,SocCounter.BLOCK_CNT);
		
		if (BlockParam.CHG_CURRENT > BlockParam.AVG_CURRENT + CURRENT_A(0.5) && BlockParam.AVG_VOLTAGE >= TOTAL_CELL_VOLTAGE(13.9))
		{
			lowlimit = CURRENT_A(1);
			for (currentindex = 0; currentindex < CHG_VOLT_Y - 1; currentindex++)
			{
				if (BlockParam.AVG_CURRENT < lowlimit)
				{
					break;
				}
				else if (lowlimit > CURRENT_A(3.8))
				{
					lowlimit += CURRENT_A(2);
				}
				else
				{
					lowlimit += CURRENT_A(0.5);
				}
			}
			//currentindex is 0, current < 1.0A
			//currentindex is 1, 1.0A <= current < 1.5A
			//currentindex is 2, 1.5A <= current < 2.0A
			//currentindex is 3, 2.0A <= current < 2.5A
			//currentindex is 4, 2.5A <= current < 3.0A
			//currentindex is 5, 3.0A <= current < 3.5A
			//currentindex is 6, 3.5A <= current < 4.0A
			//currentindex is 7, 4.0A <= current < 6.0A
			//currentindex is 8, 6.0A <= current < 8.0A
			//currentindex is 9, 8.0A <= current < 10.0A
			//currentindex is 10, 10.0A <= current
			
			for (voltageindex = 0; voltageindex < CHG_VOLT_X - 1; voltageindex++)
			{
				if (BlockParam.AVG_VOLTAGE <= CHG_VOLT_ARRAY[voltageindex + 1]*Bat.CELL_NUM)
				{
					break;
				}
			}
			// voltageindex is 0, 13.9 < voltage < 14.0
			// voltageindex is 1, 14.0 < voltage < 14.1
			// voltageindex is 2, 14.1 < voltage < 14.2
			// voltageindex is 3, 14.2 < voltage < 14.3
			// voltageindex is 4, 14.3 < voltage < 14.4
			// voltageindex is 5, 14.4 < voltage < 14.5
			// voltageindex is 6, 14.5 < voltage
			
			highlimit = chg_volt_limit_calc(currentindex,voltageindex,&CHG_VOLT_HIGH_LIMIT[0][0]);
			lowlimit	= chg_volt_limit_calc(currentindex,voltageindex,&CHG_VOLT_LOW_LIMIT[0][0]);
			
			if (Soc.ZERO >= 0)
			{
				showsoc = math_diveder((Soc.INDEX-Soc.ZERO)*SOC_INDEX_MAX,(Soc.FULL-Soc.ZERO));
			}
			else
			{
				showsoc = math_diveder((Soc.INDEX)*SOC_INDEX_MAX,(Soc.FULL));
			}
			
			if (SocCounter.CHARGE_LD_CNT > MINITES(2.0))
			{
				if (showsoc < SOC_INDEX_VALUE(98/100) && lowlimit < SOC_INDEX_VALUE(98/100))
				{
					lowlimit = SOC_INDEX_VALUE(98/100);
				}
				
				if (highlimit <= lowlimit)
				{
					highlimit = lowlimit + SOC_INDEX_VALUE(1/100);
				}
			}
			else if (lowlimit >= highlimit)
			{
				lowlimit = highlimit - SOC_INDEX_VALUE(1/100);
			}
			
			Integral.AH_0001_CMP_CHG = 0;
			
			Soc.HIGH_LIMIT = highlimit;
			Soc.LOW_LIMIT	 = lowlimit;
			
			if (showsoc < lowlimit && Soc.INDEX < Soc.FULL)
			{
				if (Soc.ZERO > 0)
				{
					Soc.ZERO--;
				}
				else if (Soc.FULL > SOC_FULL_MIN)
				{
					Soc.FULL--;
					if (Soc.INDEX + 1 < Soc.FULL)
					{
						Soc.INDEX++;
					}
				}
				lowlimit = (highlimit + lowlimit)>>1;
				TempU32  = math_diveder(Integral.AH_0001_CHG*(SOC_INDEX_MAX - lowlimit),(SOC_INDEX_MAX - showsoc));
				
				if (INDEX_DEC_MAX < TempU32)
				{
					TempU32 = INDEX_DEC_MAX;
				}
				else if (TempU32 < INDEX_DEC_MIN)
				{
					TempU32 = INDEX_DEC_MIN;
				}
				
				Integral.AH_0001_CMP_CHG = TempU32 - Integral.AH_0001_CHG;
				SocCounter.CHG_LEARN_CNT++;
			}
			else if (showsoc > highlimit && Soc.INDEX < Soc.FULL)
			{
				if (Soc.FULL < SOC_INDEX_MAX)
				{
					Soc.FULL++;
				}
				else
				{
					lowlimit = (highlimit + lowlimit)>>1;
					TempU32  = math_diveder(Integral.AH_0001_CHG*(SOC_INDEX_MAX - lowlimit),(SOC_INDEX_MAX - showsoc));
					if (INDEX_DEC_MAX < TempU32)
					{
						TempU32 = INDEX_DEC_MAX;
					}
					else if (TempU32 < INDEX_DEC_MIN)
					{
						TempU32 = INDEX_DEC_MIN;
					}
					Integral.AH_0001_CMP_CHG = TempU32 - Integral.AH_0001_CHG;
				}
				SocCounter.CHG_LEARN_CNT++;
			}
		}
		Marks.BLOCK = 0;
	}
	else
	{
		Soc.SHOW = 88;
	}
}


u16 chg_limit_calc(const u16 *soc_array, u8 current_index, u8 volt_index)
{
	s32 TempS32, S32Tmp, value32;
	u16 index;
	if (current_index >= SOC_ARRAY_Y - 1)
	{
		index = (SOC_ARRAY_Y - 1)*SOC_ARRAY_X;
		index += volt_index;
		
		TempS32 = *(soc_array + index - 1);
		S32Tmp 	= *(soc_array + index);
		
		value32 = two_point_diff_calc(TempS32, S32Tmp - TempS32, BlockParam.AVG_VOLTAGE - VOLT_CHG_POINT_ARRAY[volt_index - 1]*Bat.CELL_NUM, (VOLT_CHG_POINT_ARRAY[volt_index] - VOLT_CHG_POINT_ARRAY[volt_index - 1])*Bat.CELL_NUM);
	}
	else
	{
		index = (current_index)*SOC_ARRAY_X;
		index += volt_index;
		TempS32 = *(soc_array + index - 1);
		S32Tmp 	= *(soc_array + index);
		
		value32 = two_point_diff_calc(TempS32, S32Tmp - TempS32, BlockParam.AVG_VOLTAGE - VOLT_CHG_POINT_ARRAY[volt_index - 1]*Bat.CELL_NUM, (VOLT_CHG_POINT_ARRAY[volt_index] - VOLT_CHG_POINT_ARRAY[volt_index - 1])*Bat.CELL_NUM);
		
		index = (current_index + 1)*SOC_ARRAY_X;
		index += volt_index;
		TempS32 = *(soc_array + index - 1);
		S32Tmp 	= *(soc_array + index);
		
		TempS32 = two_point_diff_calc(TempS32, S32Tmp - TempS32, BlockParam.AVG_VOLTAGE - VOLT_CHG_POINT_ARRAY[volt_index - 1]*Bat.CELL_NUM, (VOLT_CHG_POINT_ARRAY[volt_index] - VOLT_CHG_POINT_ARRAY[volt_index - 1])*Bat.CELL_NUM);
		
		if (current_index == 0 || current_index == 1)
		{
			value32 = two_point_diff_calc(value32,TempS32 - value32,BlockParam.AVG_CURRENT - CURRENT_A(current_index),CURRENT_A(1));
		}
		else
		{
			value32	= two_point_diff_calc(value32,TempS32 - value32,BlockParam.AVG_CURRENT - CURRENT_A(current_index-1)*2,CURRENT_A(2));
		}
	}
	
	return value32;
}
void charge_soc_compensation_in_current(void)
{
	s32 TempS32;
	s16 TempFullMin, showsoc;
	u16 highlimit, lowlimit, deltasoc;
	u8 voltageindex, currentindex;
	
	if (Marks.VERIFYED)
	{
		Marks.BLOCK = 0;
		
		BlockParam.AVG_VOLTAGE = math_diveder(Integral.SUM_VOLTAGE,SocCounter.BLOCK_CNT);
		BlockParam.AVG_CURRENT = math_diveder(Integral.SUM_CURRENT,SocCounter.BLOCK_CNT);
		
		if (Marks.CHG_HIGH == 0)
		{
			if (BlockParam.AVG_VOLTAGE < TOTAL_CELL_VOLTAGE(13.4))
			{
				BlockParam.CHG_CURRENT += BlockParam.AVG_CURRENT;
				BlockParam.CHG_CURRENT >>= 1;
			}
			
			if (BlockParam.AVG_VOLTAGE > TOTAL_CELL_VOLTAGE(12) && BlockParam.AVG_VOLTAGE < TOTAL_CELL_VOLTAGE(14))
			{
				if (BlockParam.AVG_CURRENT < CURRENT_A(1.0))
				{
					currentindex = 0;
				}
				else
				{
					currentindex = math_diveder(BlockParam.AVG_CURRENT,CURRENT_A(2)) + 1;
					if (currentindex > SOC_ARRAY_Y - 1)
					{
						currentindex = SOC_ARRAY_Y - 1;
					}
				}
				//currentindex is 0, current < 1.0A
				//currentindex is 1, 1A <= current < 2A
				//currentindex is 2, 2A <= current < 4A
				//currentindex is 3, 4A <= current < 6A
				//currentindex is 4, 6A <= current < 8A
				//currentindex is 5, 8A <= current < 10A
				//currentindex is 6, current >= 10.0A
				
				for (voltageindex = 1; voltageindex < SOC_ARRAY_X - 1; voltageindex++)
				{
					if (BlockParam.AVG_VOLTAGE <= (u16)VOLT_CHG_POINT_ARRAY[voltageindex]*Bat.CELL_NUM)
					{
						break;
					}
				}
				//voltageindex is 1, 12.0V < voltage <= 12.2V
				//voltageindex is 2, 12.2V < voltage <= 12.4V
				//voltageindex is 3, 12.4V < voltage <= 12.6V
				//voltageindex is 4, 12.6V < voltage <= 12.8V
				//voltageindex is 5, 12.8V < voltage <= 13.0V
				//voltageindex is 6, 13.0V < voltage <= 13.2V
				//voltageindex is 7, 13.2V < voltage <= 13.4V
				//voltageindex is 8, 13.4V < voltage <= 13.6V
				//voltageindex is 9, 13.6V < voltage <= 14.0V
				//voltageindex is 10, voltage > 14.0V
				
				highlimit = chg_limit_calc(&CHG_CUR_HIGH_LIMIT[0][0],currentindex,voltageindex);
				lowlimit	= chg_limit_calc(&CHG_CUR_LOW_LIMIT[0][0],currentindex,voltageindex);
				
				if (SystemParam.TEMPERATURE < 0)
				{
					deltasoc = math_diveder((17 - SystemParam.TEMPERATURE/2)*SOC_INDEX_MAX,100);
				}
				else if (SystemParam.TEMPERATURE < 16)
				{
					deltasoc = math_diveder((16 - SystemParam.TEMPERATURE)*SOC_INDEX_MAX,100);
				}
				else
				{
					deltasoc = 0;
				}
									
				if (lowlimit >= highlimit)
				{
					highlimit = lowlimit + SOC_INDEX_VALUE(1/100);
				}
				
				if (lowlimit > deltasoc)
				{
					lowlimit -= deltasoc;
				}
				else
				{
					lowlimit = 0;
				}
				
				if (highlimit > deltasoc)
				{
					highlimit -= deltasoc;
				}
				else
				{
					highlimit = 0;
				}
				
				Soc.HIGH_LIMIT = highlimit;
				Soc.LOW_LIMIT	 = lowlimit;

				if (Soc.ZERO >= 0)
				{
					showsoc = math_diveder((Soc.INDEX-Soc.ZERO)*SOC_INDEX_MAX,(Soc.FULL-Soc.ZERO));
				}
				else
				{
					showsoc = math_diveder((Soc.INDEX)*SOC_INDEX_MAX,(Soc.FULL));
				}
				
				if (showsoc < lowlimit && Soc.ZERO > 0)
				{
					Soc.ZERO--;
				}
				
				if (showsoc > highlimit + SOC_INDEX_VALUE(3/100) || Soc.INDEX  + SOC_INDEX_VALUE(3/100) < lowlimit)
				{
					//TempFullMin = (showsoc > highlimit)?highlimit:lowlimit;
					TempFullMin = (highlimit + lowlimit)>>1;
					
					if (TempFullMin < SOC_INDEX_VALUE(5/100) || Soc.INDEX < SOC_INDEX_VALUE(5/100))
					{
						Cap.CAP_GUSS_LAST = Cap.QNOW;
					}
					else if (Marks.ZERO_START)
					{
						if (Cap.CAP_CHG_ONCE < Cap.CAP_LEARN_MAX)
						{
							Cap.CAP_GUSS_LAST = math_diveder(Cap.CAP_CHG_ONCE*(SOC_INDEX_MAX  - TempFullMin),TempFullMin);
						}
					}
					else if (TempFullMin > Soc.CHG_SOC + SOC_INDEX_VALUE(5/100))
					{
						if (Cap.CAP_CHG_ONCE  < Cap.CAP_LEARN_MAX + Cap.CAP_CHG_LAST)
						{
							Cap.CAP_GUSS_LAST = math_diveder((Cap.CAP_CHG_ONCE - Cap.CAP_CHG_LAST)*(SOC_INDEX_MAX  - TempFullMin),TempFullMin - Soc.CHG_SOC);
						}
					}
					else if (showsoc > highlimit)
					{
						Cap.CAP_GUSS_LAST = math_diveder(Cap.QNOW*(SOC_INDEX_MAX - Soc.INDEX + SOC_INDEX_VALUE(5/100)),SOC_INDEX_MAX);
					}
					else if (Soc.INDEX < SOC_INDEX_VALUE(92/100))
					{
						Cap.CAP_GUSS_LAST = math_diveder(Cap.QNOW*(SOC_INDEX_MAX - Soc.INDEX - SOC_INDEX_VALUE(5/100)),SOC_INDEX_MAX);
					}
					else
					{
						Cap.CAP_GUSS_LAST = math_diveder(Cap.QNOW*(SOC_INDEX_MAX - Soc.INDEX),SOC_INDEX_MAX*2);
					}
					
					if (Cap.CAP_GUSS_LAST > Cap.CAP_LEARN_MAX)
					{
						Cap.CAP_GUSS_LAST = Cap.CAP_LEARN_MAX;
					}
					else if (Cap.CAP_GUSS_LAST < Q_VALUE(0.5))
					{
						Cap.CAP_GUSS_LAST = Q_VALUE(0.5);
					}
					
					TempS32	= SOC_1_Q_CONST_CHG*Cap.CAP_GUSS_LAST>>SOC_1_Q_SHIFT;
					
					if (Soc.INDEX > SOC_INDEX_VALUE(90/100))
					{
						TempS32 = math_diveder(TempS32*SOC_INDEX_MAX,SOC_INDEX_VALUE(10/100));
					}
					else
					{
						TempS32 = math_diveder(TempS32*SOC_INDEX_MAX,(SOC_INDEX_MAX - Soc.INDEX));
					}
					
					if (INDEX_DEC_MAX < TempS32)
					{
						TempS32 = INDEX_DEC_MAX;
					}
					
					if (Cap.CAP_SET)
					{
						s32 S32Tmp;
						S32Tmp = q_curve_calc(Cap.CAP_SET,1);
						S32Tmp = SOC_1_Q_CONST_CHG*S32Tmp>>SOC_1_Q_SHIFT;
						TempS32 = (TempS32*50 + S32Tmp*78)>>7;
					}
					
					if (TempS32 < INDEX_DEC_MIN)
					{
						TempS32 = INDEX_DEC_MIN;
					}
					else if (INDEX_DEC_MAX < TempS32)
					{
						TempS32 = INDEX_DEC_MAX;
					}
					 
					Integral.AH_0001_CMP_CHG = TempS32 - Integral.AH_0001_CHG;
					
					if (Marks.ZERO_START)
					{
						Cap.CAP_GUSS_IN_CURRENT_CHARGE = Cap.CAP_CHG_ONCE + Cap.CAP_GUSS_LAST;
					}
					else if (SOC_INDEX_MAX > Soc.CHG_SOC + SOC_INDEX_VALUE(10/100))
					{
						Cap.CAP_GUSS_IN_CURRENT_CHARGE = Cap.CAP_GUSS_LAST + Cap.CAP_CHG_ONCE - Cap.CAP_CHG_LAST;
						Cap.CAP_GUSS_IN_CURRENT_CHARGE = math_diveder(Cap.CAP_GUSS_IN_CURRENT_CHARGE*SOC_INDEX_MAX,SOC_INDEX_MAX - Soc.CHG_SOC);
					}
					
					if (Cap.CAP_GUSS_IN_CURRENT_CHARGE > Cap.CAP_LEARN_MAX)
					{
						Cap.CAP_GUSS_IN_CURRENT_CHARGE = Cap.CAP_LEARN_MAX;
					}
					else if (Cap.CAP_GUSS_IN_CURRENT_CHARGE < Q_VALUE_MIN)
					{
						Cap.CAP_GUSS_IN_CURRENT_CHARGE = Q_VALUE_MIN;
					}
					
					SocCounter.CHG_LEARN_CNT++;
				}
			}
		}
	}
	else
	{
		Soc.SHOW = 44;
	}
}


void charge_process(void)
{
	u16 TempU16;
	
	Integral.AH += SystemParam.AVG_CURRENT;
	if (Integral.AH >= Integral.AH_0001_CHG + Integral.AH_0001_CMP_CHG)
	{
		Integral.AH -= Integral.AH_0001_CHG + Integral.AH_0001_CMP_CHG;
		
		if (Soc.INDEX < SOC_INDEX_MAX - 1)
		{
			Soc.INDEX++;
		}
		
		if (Soc.FULL <= Soc.INDEX)
		{
			Soc.FULL = Soc.INDEX + 1;
			if (Soc.FULL > SOC_INDEX_MAX)
			{
				Soc.FULL = SOC_INDEX_MAX;
			}
		}
		
		#ifdef SHOW_DEC_SLOW
		if (Marks.FBD_SLW == 0 && Soc.ZERO <= 0 && Soc.INDEX > Soc.DEC_SHOW)
		{
			Soc.ZERO = Soc.DEC_SHOW - Soc.INDEX;
		}
		#endif
	}
	else if (Integral.AH < -Integral.AH_0001_CHG)
	{
		Integral.AH = -Integral.AH_0001_CHG;
	}
	
	Integral.AH_CHG_ONCE += SystemParam.AVG_CURRENT;
	
	if (Integral.AH_CHG_ONCE > AH(1/Q_UNIT))
	{
		Cap.CAP_CHG_ONCE++;
		Integral.AH_CHG_ONCE -= AH(1/Q_UNIT);
		TempU16 = Cap.CAP_CHG_ONCE*CHARGE_EFFICIENCY_MIN>>EFFICIENCY_SHIFT;
		TempU16 = q_curve_inv_calc(TempU16);
		update_cap_array(TempU16, 1);
	}
	else if (Integral.AH_CHG_ONCE < -AH(1/Q_UNIT))
	{
		Integral.AH_CHG_ONCE = -AH(1/Q_UNIT);
	}
	
	if (SocCounter.CHG_TIME_CNT < HOURS(24))
	{
		if (0 == (++SocCounter.CHG_TIME_CNT&0x3FFF))
		{
			SystemParam.TEMPERATURE = SystemParam.TEMP_NOW;
		}
	}
	
	if (Soc.ZERO > 0)
	{
		soc_zero_decrease_process();
	}
	#ifdef SHOW_DEC_SLOW
	else if (Marks.FBD_SLW)
	{
		Marks.FBD_SLW = 0;
	}
	#endif
	
	block_voltage_current_calc_in_charge();
	
	if (Marks.CHG_VOLT)
	{
		if (SocCounter.CHARGE_VOLT_CNT < HOURS(2))
		{
			SocCounter.CHARGE_VOLT_CNT++;
		}
		
		if (SocCounter.BLOCK_CNT >= ((charge_current_smaller(1.2) && SystemParam.AVG_VOLTAGE >= TOTAL_CELL_VOLTAGE(14.30))?SECONDS(5):SECONDS(30)) && Integral.SUM_CURRENT > 0)
		{
			charge_soc_compensation_in_voltage();
		}
		
		TempU16 = BlockParam.CHG_CURRENT*14>>6;
		if (TempU16 < CURRENT_A(0.5))
		{
			TempU16 = CURRENT_A(0.5);
		}
		else if (TempU16 > CURRENT_A(1.8))
		{
			TempU16 = CURRENT_A(1.8);
		}
		
		if (SystemParam.AVG_CURRENT < TempU16 && SystemParam.AVG_VOLTAGE >= TOTAL_CELL_VOLTAGE(14.30))
		{
			if (SocCounter.CHARGE_LD_CNT < MINITES(50))
			{
				if (++SocCounter.CHARGE_LD_CNT == MINITES(1.5))
				{
					Marks.FORCE_FULL = 1;
				}
			}
		}
		else if (SocCounter.CHARGE_LD_CNT > 10)
		{
			SocCounter.CHARGE_LD_CNT -= 10;
		}
	}
	else
	{
		if (SocCounter.CHARGE_LD_CNT)
		{
			SocCounter.CHARGE_LD_CNT--;
		}
		
		if (SocCounter.CHARGE_VOLT_CNT)
		{
			SocCounter.CHARGE_VOLT_CNT--;
		}
		
		if (Marks.CHG_HIGH)
		{
			if (SocCounter.CHARGE_VOLT_CNT > MINITES(30) && charge_current_larger(0.2) && charge_current_smaller(1.2) &&
					SystemParam.AVG_VOLTAGE > TOTAL_CELL_VOLTAGE(13.65) && Soc.SHOW < SOC_SHOW_VALUE(0.98))
			{
				if (++SocCounter.DIS_TIME_CNT >= SECONDS(45))
				{
					SocCounter.DIS_TIME_CNT = 0;
					if (Soc.ZERO > 0)
					{
						Soc.ZERO--;
					}
					else if (Soc.FULL > SOC_FULL_MIN)
					{
						Soc.FULL--;
					}
				}
			}
		}
		else
		{
			SocCounter.DIS_TIME_CNT = 0;
		}
		
		if (Integral.SUM_CURRENT >= CURRENT_CHG_LEARN_BLOCK)
		{
			charge_soc_compensation_in_current();
		}
	}
	
	if (SystemParam.AVG_VOLTAGE > TOTAL_CELL_VOLTAGE(13.6) && charge_current_smaller(1.0))
	{
		if (SocCounter.CHARGE_UD_1A < HOURS(2))
		{
			SocCounter.CHARGE_UD_1A++;
		}
	}
	else
	{
		SocCounter.CHARGE_UD_1A = 0;
	}
	
	if (SystemFlg.CRG_VALID)
	{
		if (SystemFlg.CRG_IN == 0 && charge_current_larger(1.5))
		{
			if (++SocCounter.VALID_CNT >= MINITES(1))
			{
				SystemFlg.CRG_VALID = 0;
			}
		}
		else
		{
			SocCounter.VALID_CNT = 0;
		}
	}
	else if (SystemFlg.CRG_IN)
	{
		if (charge_current_larger(1.0) || SystemParam.AVG_VOLTAGE > TOTAL_CELL_VOLTAGE(13.90))
		{
			if (++SocCounter.VALID_CNT >= MINITES(1))
			{
				SystemFlg.CRG_VALID = 1;
			}
		}
		else
		{
			SocCounter.VALID_CNT = 0;
		}
	}
	
	if (Marks.FORCE_FULL == 0 && ((charge_current_smaller(0.35) && SystemParam.AVG_VOLTAGE < TOTAL_CELL_VOLTAGE(13.35) && Soc.SHOW < SOC_SHOW_VALUE(96/100)) || 
			(charge_current_smaller(0.5) && SystemParam.AVG_VOLTAGE < TOTAL_CELL_VOLTAGE(13.3)) ||
			(SocCounter.CHG_TIME_CNT >= HOURS(18) && SystemParam.AVG_VOLTAGE < TOTAL_CELL_VOLTAGE(13.6) && charge_current_smaller(1.0)) ||
			discharge_current_larger(0.75) || (SystemFlg.CRG_VALID && SystemFlg.CRG_IN == 0 && charge_current_smaller(0.5))))
	{
		if (SystemParam.AVG_VOLTAGE < TOTAL_CELL_VOLTAGE(13.0))
		{
			SocCounter.STATE_SW_CNT_1 += 30;
		}
		
		if (discharge_current_larger(5) && SystemFlg.BP_VALID)
		{
			SocCounter.STATE_SW_CNT_1 += 60;
		}
		
		if (SocCounter.STATE_SW_CNT_1 < MINITES(1.5))
		{
			SocCounter.STATE_SW_CNT_1++;
		}
		else
		{
			Cap.CAP_CHG_LAST = Cap.CAP_CHG_ONCE - Cap.CAP_CHG_LAST;
			
			if (Soc.SHOW >= SOC_SHOW_VALUE(98/100))
			{
				Marks.FULL		 = 1;
				Marks.FULL_DIS = 1;
			}
			goto_soc_ready_state();
		}
	}
	else
	{
		SocCounter.STATE_SW_CNT_1 = 0;
	}
	
	if ((SystemParam.AVG_VOLTAGE > TOTAL_CELL_VOLTAGE(13.25) && charge_current_smaller(1.0) && discharge_current_smaller(0.75) && Soc.INDEX >= SOC_FULL_MIN &&
		 ((((Soc.SHOW > SOC_SHOW_VALUE(96/100) && (SocCounter.CHARGE_VOLT_CNT > MINITES(45) || SocCounter.CHARGE_LD_CNT > MINITES(2)))||
			(SocCounter.CHARGE_LD_CNT > MINITES(2.0) && SystemParam.AVG_VOLTAGE > TOTAL_CELL_VOLTAGE(13.45)) || 
			(SystemFlg.CRG_VALID && SystemFlg.CRG_IN && discharge_current_smaller(0.75) && charge_current_smaller(0.8) && SystemParam.AVG_VOLTAGE > TOTAL_CELL_VOLTAGE(13.5))||
			(Soc.SHOW > SOC_SHOW_VALUE(97/100) && charge_current_smaller(0.5) && SystemParam.AVG_VOLTAGE > TOTAL_CELL_VOLTAGE(13.7)))&& SystemParam.AVG_VOLTAGE < TOTAL_CELL_VOLTAGE(14.0))||
			SocCounter.CHARGE_VOLT_CNT > MINITES(140) || SocCounter.CHARGE_UD_1A > MINITES(60)))||(Marks.FORCE_FULL && SystemParam.AVG_VOLTAGE < TOTAL_CELL_VOLTAGE(14.00)))
	{
		if (SocCounter.STATE_SW_CNT_2 < MINITES(1.5) && Marks.FORCE_FULL == 0)
		{
			SocCounter.STATE_SW_CNT_2++;
		}
		else
		{
			SystemFlg.SOC_SAVE	= 1;
			SocCounter.STATE_SW_CNT_1 = 0;
			SocCounter.STATE_SW_CNT_2 = 0;
			SocCounter.FULL_CLR_CNT		= 0;
			SocCounter.HOURS_FULL_DIS	= 0;
			SocCounter.BLOCK_CNT			= 0;
			
			Integral.AH_LAST					= 0;
			Integral.SUM_VOLTAGE			= 0;
			Integral.SUM_CURRENT			= 0;
				
			Marks.FULL 								= 1;
			Marks.FULL_DIS						= 1;
			Marks.A_HOUR							= 0;
			Marks.DIS_HPN							= 0;
				
			Cap.CAP_CHG_LAST					= 0;
			
			SystemParam.TEMPERATURE = SystemParam.TEMP_NOW;
			
			if (Soc.INDEX == SOC_INDEX_MAX - 1)
			{
				Soc.INDEX = SOC_INDEX_MAX;
			}
			
			if (Soc.INDEX >= SOC_FULL_MIN)
			{
				Soc.FULL = Soc.INDEX;
			}
			
			if (Marks.DEC_SLOW)
			{
				Soc.ZERO = Soc.DEC_SHOW - Soc.INDEX;
			}
			
			if (Marks.ZERO_START)
			{
				u16 TempU16;
				if (SystemParam.TEMPERATURE > 0)
				{
					Cap.CAP_LEARN_MAX = Cap.CAP_CHG_ONCE + Q_VALUE(5.0);
				}
				TempU16 = q_curve_inv_calc(Cap.CAP_CHG_ONCE);
				update_cap_array(TempU16 + Q_VALUE(1),0);
				Marks.ZERO_START	= 0;
				
				if (Cap.CAP_SET > TempU16 + Q_VALUE(1.5))
				{
					Cap.CAP_SET += TempU16 + Q_VALUE(1.5);
					Cap.CAP_SET >>= 1;
					
					if (Cap.SET_IMPACT >= 5)
					{
						Cap.SET_IMPACT -= 5;
					}
				}
			}
			else if (Cap.CAP_CHG_ONCE > Cap.CAP_LEARN_MAX)
			{
				Cap.CAP_LEARN_MAX = Cap.CAP_CHG_ONCE + Q_VALUE(8.0);
			}
			
			if (Cap.CAP_LEARN_MAX > Q_VALUE_MAX || Cap.CAP_LEARN_MAX < Q_VALUE_MIN)
			{
				Cap.CAP_LEARN_MAX = Q_VALUE_MAX;
			}
			
			if (Cap.CAP_LEARN_MIN + Q_VALUE(5) < Cap.CAP_CHG_ONCE)
			{
				Cap.CAP_LEARN_MIN = Cap.CAP_CHG_ONCE - Q_VALUE(5);
			}
			
			cap_calc();//when goto full
			GotoFullChargeState();
		}
	}
	else
	{
		SocCounter.STATE_SW_CNT_2 = 0;
	}
	
	Soe.INTEGRAL += Soe.POWER;
	if (Soe.INTEGRAL >= WH(1))
	{
		Soe.INTEGRAL -= WH(1);
		if (Soe.SOE + 1 < Soe.FULL)
		{
			Soe.SOE++;
		}
		Soe.CHG_WH_ONCE++;
	}
	else if (Soe.INTEGRAL <= WH(-1))
	{
		Soe.INTEGRAL += WH(1);
		if (Soe.SOE > 0)
		{
			Soe.SOE--;
		}
		
		if (Soe.CHG_WH_ONCE > 0)
		{
			Soe.CHG_WH_ONCE--;
		}
	}
}

void full_charge_process(void)
{
	s16 TempS16;
	
	Integral.AH += SystemParam.AVG_CURRENT;
	if (Integral.AH >= Integral.AH_0001_DIS)
	{
		Integral.AH -= Integral.AH_0001_DIS;
		if (Soc.INDEX < SOC_INDEX_MAX)
		{
			Soc.INDEX++;
		}
		
		if (Soc.FULL < Soc.INDEX)
		{
			Soc.FULL = Soc.INDEX;
		}
		
		#ifdef SHOW_DEC_SLOW
		Soc.ZERO = Soc.DEC_SHOW - Soc.INDEX;
		#endif
	}
	else if (Integral.AH < -Integral.AH_0001_CHG)
	{
		Integral.AH = -Integral.AH_0001_CHG;
	}
	
	if (SocCounter.CHG_TIME_CNT < HOURS(24))
	{
		SocCounter.CHG_TIME_CNT++;
	}
	
	if (SocCounter.STATE_SW_CNT_2 < MINITES(30))
	{
		SocCounter.STATE_SW_CNT_2++;
		block_voltage_current_calc();
	}
	else
	{
		SocCounter.STATE_SW_CNT_2 = 0;
		Soc.OCV_SOC = soc_ocv_calc(BlockParam.AVG_VOLTAGE);
		
		if (Marks.A_HOUR)
		{
			SocCounter.CHG_TIME_CNT = HOURS(20);
		}
		else if (Soc.OCV_SOC >= SOC_FULL_MIN)
		{
			Soc.INDEX = Soc.OCV_SOC;
			Soc.FULL	= Soc.OCV_SOC;
		}
		
		Marks.A_HOUR 	= 1;
		Integral.AH 	= 0;
	}
	
	if (SystemParam.AVG_VOLTAGE > TOTAL_CELL_VOLTAGE(14.3) && charge_current_larger(1.0))
	{
		if (SocCounter.STATE_SW_CNT_1 > -SECONDS(15))
		{
			SocCounter.STATE_SW_CNT_1--;
		}
		else
		{
			SocCounter.CHG_CNT_30S		= 0;
			SocCounter.BLOCK_CNT 			= 0;
			SocCounter.STATE_SW_CNT_1 = 0;
			SocCounter.STATE_SW_CNT_2 = 0;
			Integral.SUM_CURRENT			= 0;
			Integral.SUM_VOLTAGE			= 0;
			Integral.AH_LAST					= Integral.AH;
			Marks.FORCE_FULL					= 0;
			GotoChargeState();
		}
	}
	else if ((SystemParam.AVG_VOLTAGE < TOTAL_CELL_VOLTAGE(13.70) && (discharge_current_larger(0.1) || SocCounter.CHG_TIME_CNT >= HOURS(18))))
	{
		if (Soc.FULL == Soc.INDEX && Marks.SHOW_FULL == 0 && discharge_current_smaller(0.8))
		{
			SocCounter.STATE_SW_CNT_1 = 0;
		}
		
		if (charge_current_smaller(0.2))
		{
			SocCounter.STATE_SW_CNT_1 += 3;
		}
		
		if ((++SocCounter.STATE_SW_CNT_1 >= SECONDS(30)))
		{
			goto_soc_ready_state();
		}
	}
	else
	{
		SocCounter.STATE_SW_CNT_1 = 0;
	}
	
	if (Soe.SOE < Soe.FULL)
	{
		Soe.SOE++;
	}
	else if (Soe.SOE > Soe.FULL)
	{
		Soe.SOE--;
	}
	
	TempS16 = soe_full_calc(Cap.QNOW);
	if (TempS16 > (Soe.FULL*154>>7) || TempS16 < (Soe.FULL*102>>7))
	{
		Soe.FULL = soe_full_calc(Cap.QNOW);
	}
	
	Soe.INTEGRAL = 0;
}

void discharge_process(void)
{
	Integral.AH += SystemParam.AVG_CURRENT;
	
	if (Integral.AH < -Integral.AH_0001_DIS - Integral.AH_0001_CMP_DIS)
	{
		Integral.AH += Integral.AH_0001_DIS + Integral.AH_0001_CMP_DIS;
		if (Soc.INDEX > 0)
		{
			Soc.INDEX--;
			
			if (++Soc.CIRCLE >= Soc.FULL)
			{
				Soc.CIRCLE = 0;
				
				if ((++His.CIRCLE&3) == 0)
				{
					if (Cap.CAP_LEARN_MIN > Q_VALUE_MIN)
					{
						Cap.CAP_LEARN_MIN--;
					}
				}
				
				if (Cap.SET_IMPACT > 0)
				{
					Cap.SET_IMPACT--;
				}
			}
			
			if (++SocCounter.DIS_BLOCK_CNT >= 60)
			{
				SocCounter.DIS_BLOCK_CNT = 0;
				if (Integral.DIS_AH_BLOCK > AH(0.04))
				{
					Integral.DIS_AH_BLOCK -= AH(0.01);
				}
			}
		}
		
		if (Soc.INDEX < Soc.ZERO)//keep INDEX not less than ZERO
		{
			Soc.ZERO = Soc.INDEX;
		}
		
		if (Marks.RATING_LEARN == 0)
		{
			if (++Cap.CAP_LEARN_INDEX >= SOC_INDEX_MAX*3)
			{
				Marks.RATING_LEARN 	= 1;
				Bat.CAP_RATING			= math_diveder(His.CAPSUM - His.CAPSUM_LST,3);
				Bat.CAP_RATING 			= q_curve_inv_calc(Bat.CAP_RATING)/Q_UNIT;
				Bat.CAP_NOW					= Bat.CAP_RATING;
				Cap.CAP_LEARN_INDEX = 0;
				His.CAPSUM_LST			= His.CAPSUM;
			}
		}
		else if (++Cap.CAP_LEARN_INDEX >= SOC_INDEX_MAX*5)
		{
			Bat.CAP_NOW = math_diveder(His.CAPSUM - His.CAPSUM_LST,5);
			Bat.CAP_NOW = q_curve_inv_calc(Bat.CAP_NOW)/Q_UNIT;
			Cap.CAP_LEARN_INDEX = 0;
			His.CAPSUM_LST			= His.CAPSUM;
		}
		
		if (++Cap.CAP_SET_DEC_CNT >= SOC_INDEX_MAX*30)
		{
			Cap.CAP_SET_DEC_CNT = 0;
			if (Cap.CAP_SET > Q_VALUE_MIN)
			{
				Cap.CAP_SET = (u32)Cap.CAP_SET*2027>>11;//0.99
			}
		}
		
		#ifdef SHOW_DEC_SLOW
		if (Marks.FBD_SLW)
		{
			if (Soc.ZERO < 0)
			{
				Soc.ZERO++;
			}
		}
		else if (Soc.INDEX >= Soc.DEC_SHOW)
		{
			Soc.ZERO = Soc.DEC_SHOW - Soc.INDEX;
		}
		#endif
	}
	else if (Integral.AH > Integral.AH_0001_DIS*2)
	{
		Integral.AH = Integral.AH_0001_DIS*2;
	}
	
	if (SocCounter.DIS_TIME_CNT < HOURS(8))
	{
		SocCounter.DIS_TIME_CNT++;
	}
	
	Integral.AH_DIS_ONCE -= SystemParam.AVG_CURRENT;
	if (Integral.AH_DIS_ONCE >=  AH(1/Q_UNIT))
	{
		u16 TempU16;
		Cap.CAP_DIS_ONCE++;
		if (Cap.CAP_LEARN_MAX < Cap.CAP_DIS_ONCE + Q_VALUE(3.0))
		{
			Cap.CAP_LEARN_MAX = Cap.CAP_DIS_ONCE + Q_VALUE(3.0);
			if (Cap.CAP_LEARN_MAX > Q_VALUE_MAX)
			{
				Cap.CAP_LEARN_MAX = Q_VALUE_MAX;
			}
		}
		
		if (Cap.CAP_LEARN_MIN < Cap.CAP_DIS_ONCE)
		{
			Cap.CAP_LEARN_MIN = Cap.CAP_DIS_ONCE;
		}
		
		His.CAPSUM++;
		Integral.AH_DIS_ONCE -= AH(1/Q_UNIT);
		TempU16 = q_curve_inv_calc(Cap.CAP_DIS_ONCE);//Q25
		update_cap_array(TempU16, 1);
		if (TempU16 > Cap.Q25)
		{
			Cap.Q25 = TempU16;
		}
	}
	else if (Integral.AH_DIS_ONCE <  -AH(1/Q_UNIT))
	{
		Integral.AH_DIS_ONCE = -AH(1/Q_UNIT);
	}
	
	if (Integral.ZERO_AH >= DISCHARGE_ZERO_FIND_BLOCK)
	{
		u32 TempU32;
		Integral.ZERO_VOLTAGE = math_diveder(Integral.ZERO_VOLTAGE,SocCounter.ZERO_CNT);
		Integral.ZERO_AH			= math_diveder(Integral.ZERO_AH,SocCounter.ZERO_CNT);
		
		if (Bat.CELL_NUM == 6)
		{
			TempU32 = 500*CURRENT_UNIT*VOLTAGE_UNIT;
		}
		else if (Bat.CELL_NUM == 5)
		{
			TempU32 = 460*VOLTAGE_UNIT*CURRENT_UNIT;
		}
		else
		{
			TempU32 = 380*VOLTAGE_UNIT*CURRENT_UNIT;
		}
		
		if ((Integral.ZERO_VOLTAGE < TOTAL_CELL_VOLTAGE(10.64) && Integral.ZERO_AH*Integral.ZERO_VOLTAGE < TempU32)||
				(Integral.ZERO_VOLTAGE < TOTAL_CELL_VOLTAGE(10.49) && Integral.ZERO_AH < CURRENT_A(10.5)))
		{
			if (Integral.ZERO_VOLTAGE > TOTAL_CELL_VOLTAGE(10.00))
			{
				discharge_zero_detect();
			}
			
			Marks.SOE_ZERO = 1;
			if (Marks.FULL_DIS)
			{
				Soe.FULL = Soe.DIS_WH_ONCE + 5;
			}
		}
		#ifdef SHOW_DEC_SLOW
		else if (Integral.ZERO_VOLTAGE < TOTAL_CELL_VOLTAGE(11.1) && Marks.FBD_SLW == 0 && Integral.ZERO_AH < CURRENT_A(25))
		{
			Marks.FBD_SLW = 1;
		}
		#endif
		
		SocCounter.ZERO_CNT 	= 0;
		Integral.ZERO_AH			= 0;
		Integral.ZERO_VOLTAGE = 0;
	}
	else if (SocCounter.ZERO_CNT >= MINITES(3) || SystemParam.AVG_VOLTAGE > TOTAL_CELL_VOLTAGE(12.5))
	{
		SocCounter.ZERO_CNT 	= 0;
		Integral.ZERO_AH			= 0;
		Integral.ZERO_VOLTAGE = 0;
	}
	else
	{
		SocCounter.ZERO_CNT++;
		Integral.ZERO_VOLTAGE += SystemParam.AVG_VOLTAGE;
		Integral.ZERO_AH 			-= SystemParam.AVG_CURRENT;
	}
	
	if (Marks.ZERO)
	{
		Integral.AH_0001_CMP_DIS = 0;
		if (++SocCounter.CHG_CNT_30S >= SECONDS(20) && discharge_current_larger(1.5))
		{
			SocCounter.CHG_CNT_30S = 0;
			if (Soc.INDEX < Soc.FULL && Soc.ZERO < SOC_ZERO_MAX)
			{
				variable_inc(Soc.ZERO_DELTA,Soc.INDEX,&Soc.ZERO);
			}
			variable_dec(1,0,&Soe.SOE);
		}
		#ifdef SHOW_DEC_SLOW
		if (Marks.FBD_SLW == 0)
		{
			Marks.FBD_SLW = 1;
		}
		#endif
	}
	#ifdef SHOW_DEC_SLOW
	else if (Marks.FBD_SLW && Soc.ZERO < 0)
	{
		if (++SocCounter.CHG_CNT_30S >= SECONDS(20) && discharge_current_larger(1.5))
		{
			SocCounter.CHG_CNT_30S = 0;
			Soc.ZERO++;
		}
	}
	#endif
	else
	{
		SocCounter.CHG_CNT_30S = 0;
	}
	
	block_voltage_current_calc_in_discharge();
	
	discharge_soc_compensation();
	
	if (discharge_current_smaller(0.8))
	{
		SocCounter.STATE_SW_CNT_1++;
		#ifdef BP_AUTO_SWITCH
		if (Integral.AH == Integral.AH_0001_DIS*2 && charge_current_larger(20) && SystemFlg.BP_VALID == 0)
		{
			SocCounter.STATE_SW_CNT_1 += 10;
		}
		#endif
	}
	else
	{
		SocCounter.STATE_SW_CNT_1 = 0;
		
		if (SystemFlg.CRG_IN)
		{
			SystemFlg.CRG_VALID = 0;
		}
	}
	
	if (charge_current_larger(1.5) || SystemParam.AVG_VOLTAGE > TOTAL_CELL_VOLTAGE(14.0))
	{
		static s16 charge_current_last;
		s16 TempS16;
		SocCounter.STATE_SW_CNT_2++;
		
		if (SystemFlg.CRG_IN && SystemFlg.CRG_VALID)
		{
			SocCounter.STATE_SW_CNT_2 += 20;
		}
		else if (SocCounter.STATE_SW_CNT_1 > SECONDS(50))
		{
			SocCounter.STATE_SW_CNT_2 += 2;
		}
		
		TempS16 = charge_current_last - SystemParam.AVG_CURRENT;
		if (abs(TempS16) > CURRENT_A(1.0))
		{
			charge_current_last = SystemParam.AVG_CURRENT;
			SocCounter.STATE_SW_CNT_2 = 0;
		}
	}
	else
	{
		SocCounter.STATE_SW_CNT_2 = 0;
	}
	
	if (SocCounter.STATE_SW_CNT_1 > MINITES(2) || SocCounter.STATE_SW_CNT_2 > SECONDS(60))
	{
		Cap.CAP_CHG_ONCE	= 0;
		goto_soc_ready_state();
	}
	
	Soe.INTEGRAL += Soe.POWER;
	if (Soe.INTEGRAL >= WH(1))
	{
		Soe.INTEGRAL -= WH(1);
		if (Soe.SOE + 1 < Soe.FULL)
		{
			Soe.SOE++;
		}
		
		if (Soe.DIS_WH_ONCE > 0)
		{
			Soe.DIS_WH_ONCE--;
		}
	}
	else if (Soe.INTEGRAL <= WH(-1))
	{
		Soe.INTEGRAL += WH(1);
		if (Soe.SOE > 0)
		{
			Soe.SOE--;
		}
		Soe.DIS_WH_ONCE++;
	}
}


void soc_ocv_process(void)
{
	u16 TempU16;
	u8 TempU8;
	
	if (SystemParam.TEMPERATURE > SystemParam.TEMP_NOW + 20)
	{
		SystemParam.TEMPERATURE -= 4;
	}
	else if (SystemParam.TEMPERATURE > SystemParam.TEMP_NOW + 15)
	{
		SystemParam.TEMPERATURE -= 3;
	}
	else if (SystemParam.TEMPERATURE > SystemParam.TEMP_NOW + 10)
	{
		SystemParam.TEMPERATURE -= 2;
	}
	else if (SystemParam.TEMPERATURE > SystemParam.TEMP_NOW + 5)
	{
		SystemParam.TEMPERATURE--;
	}
	else if (SystemParam.TEMPERATURE + 20 < SystemParam.TEMP_NOW)
	{
		SystemParam.TEMPERATURE += 4;
	}
	else if (SystemParam.TEMPERATURE + 15 < SystemParam.TEMP_NOW)
	{
		SystemParam.TEMPERATURE += 3;
	}
	else if (SystemParam.TEMPERATURE + 10 < SystemParam.TEMP_NOW)
	{
		SystemParam.TEMPERATURE += 2;
	}
	else if (SystemParam.TEMPERATURE + 5 < SystemParam.TEMP_NOW)
	{
		SystemParam.TEMPERATURE++;
	}
	
//	cap_calc();//when 30min happen
	Soc.OCV_SOC = soc_ocv_calc(BlockParam.AVG_VOLTAGE);
	TempU8 			= 0;
	
	if (Marks.A_HOUR == 0)
	{
		if (SocCounter.EIGHT_HOUR > 0)
		{
			Marks.A_HOUR 	= 1;
			Soc.OCV_START = Soc.OCV_SOC;
		}
		
		if (Soc.OCV_SOC > Soc.FULL && Soc.FULL > Soc.ZERO && Soc.INDEX > Soc.ZERO)
		{
			TempU16 		= math_diveder((Soc.INDEX - Soc.ZERO)*SOC_INDEX_MAX,(Soc.FULL-Soc.ZERO));//current socshow
			Soc.FULL 		= Soc.OCV_SOC;
			if (Marks.DEC_SLOW && Soc.DEC_SHOW < Soc.INDEX)
			{
				Soc.INDEX 	= math_diveder((Soc.FULL - Soc.DEC_SHOW)*TempU16 + Soc.DEC_SHOW*SOC_INDEX_MAX,2*SOC_INDEX_MAX - TempU16);
				Soc.ZERO 		= Soc.DEC_SHOW - Soc.INDEX;
			}
			else
			{
				Soc.INDEX 	= math_diveder((Soc.FULL - Soc.ZERO)*TempU16,SOC_INDEX_MAX) + Soc.ZERO;//recalculate index change full keep show unchange
			}
			
			if (Soc.INDEX < Soc.FULL)
			{
				Soc.INDEX += 1;
			}
		}
	}
	else if (Soc.OCV_SOC > SOC_INDEX_VALUE(92/100))
	{
		Soc.OCV_START = Soc.OCV_SOC;
	}
	else if (Soc.OCV_START > Soc.OCV_SOC + SOC_INDEX_VALUE(4/100) || Soc.OCV_SOC < SOC_INDEX_VALUE(1/100))//if ocv soc decrease larger than 4% or ocv soc is smaller than 1%, decrease socshow
	{
		if (Soc.INDEX > SOC_INDEX_VALUE(3/100) + Soc.OCV_SOC || (Soc.OCV_SOC < SOC_INDEX_VALUE(1/100) && Soc.INDEX > Soc.OCV_SOC))//force to decrease
		{
			TempU8 = 1;
			SocCounter.FULL_CLR_CNT++;
		}
		SocCounter.EIGHT_HOUR = 16;
	}
	
	if (Soc.SHOW > 0 && (SocCounter.HOURS_FULL_DIS >= 24 || Marks.DIS_HPN))
	{
		if (Soc.OCV_SOC + SOC_INDEX_VALUE(25/100) < Soc.INDEX)//larger difference between ocv and index
		{
			if (Soc.INDEX > Soc.ZERO + SOC_INDEX_VALUE(1/100))
			{
				Soc.INDEX -= SOC_INDEX_VALUE(1/100);
				TempU8 = 100;
			}
			
			Integral.AH_LAST 			= 0;
			Marks.FULL_DIS 	 			= 0;
			Marks.FULL			 			= 0;
		}
		else if (SocCounter.EIGHT_HOUR >= 16)//eight hours past or ocv soc decrease 4%
		{
			if (Soc.OCV_SOC + SOC_INDEX_VALUE(10/100) < Soc.INDEX || TempU8)
			{
				if (Soc.INDEX > Soc.ZERO + SOC_INDEX_VALUE(0.5/100))
				{
					Soc.INDEX -= SOC_INDEX_VALUE(0.5/100);
					TempU8 		= 100;
				}
								
				if (TempU8 && Soc.OCV_START > SOC_INDEX_VALUE(0.5/100))
				{
					Soc.OCV_START -= SOC_INDEX_VALUE(0.5/100);
				}
				
				if (SocCounter.FULL_CLR_CNT > 15)
				{
					Marks.FULL_DIS 	 	= 0;
					Marks.FULL			 	= 0;
				}
				
				Integral.AH_LAST 		= 0;
			}
		}
		
		if (Marks.DEC_SLOW && Marks.FBD_SLW == 0 && Soc.INDEX > Soc.DEC_SHOW)
		{
			Soc.ZERO = Soc.DEC_SHOW - Soc.INDEX;
		}
		
		Marks.SHOW_DEC = 1;
	}
	
	if (SocCounter.HOURS_FULL_DIS >= 336)//seven days past after full charge
	{
		Marks.FULL_DIS = 0;
	}
	else if (++SocCounter.HOURS_FULL_DIS == 192)//for days past after full charge
	{
		Marks.FULL 	= 0;
	}
		
	if (SocCounter.EIGHT_HOUR < 16)
	{
		SocCounter.EIGHT_HOUR++;
	}
	else
	{
		SocCounter.EIGHT_HOUR = 0;
	}
	
	if (Marks.A_HOUR == 0)
	{
		TempU8 = 100;
	}
	
	if (++SocCounter.A_MONTH_CNT >= 1440)
	{
		SocCounter.A_MONTH_CNT = 0;
		if (Cap.SET_IMPACT)
		{
			Cap.SET_IMPACT--;
		}
		Cap.CAP_SET = Cap.CAP_SET*1019>>10;
	}
	
	if (TempU8==100)
	{
		SystemFlg.SOC_SAVE = 1;
	}
}

void current_zero_process(void)
{
	Integral.AH += SystemParam.AVG_CURRENT;
	
	if (Integral.AH > Integral.AH_0001_CHG)
	{
		Integral.AH = Integral.AH_0001_CHG;
	}
	else if (Integral.AH < -Integral.AH_0001_DIS)
	{
		Integral.AH = -Integral.AH_0001_DIS;
	}
	
	block_voltage_current_calc();
	
	if (charge_current_larger(0.8) || SystemParam.AVG_VOLTAGE > TOTAL_CELL_VOLTAGE(14.0))
	{
		if (charge_current_larger(1.0))
		{
			SocCounter.STATE_SW_CNT_1 += 10;
		}
		
		if (SystemFlg.CRG_IN)
		{
			SocCounter.STATE_SW_CNT_1 += 20;
		}
		
		if (SocCounter.STATE_SW_CNT_1 < SECONDS(20))
		{
			SocCounter.STATE_SW_CNT_1++;
		}
		else
		{
			SystemFlg.CRG_VALID = (SystemFlg.CRG_IN)?1:0;
			Marks.SOE_ZERO			= 0;
			Soe.DIS_WH_ONCE			= 0;
			SocCounter.CHG_LEARN_CNT = 0;
			//use_record_init(1);
			if (Soc.FULL > Soc.INDEX)
			{
				Marks.FULL			= 0;
				Marks.FULL_DIS 	= 0;
			}
			
			Marks.SHOW_DEC		= 0;
			Marks.CHG_VOLT		= 0;
			Marks.CHG_HIGH		= 0;
			Marks.FORCE_FULL	= 0;
			
			SocCounter.DIS_TIME_CNT		= 0;
			SocCounter.CHG_TIME_CNT		= 0;
			SocCounter.STATE_SW_CNT_1	= 0;
			SocCounter.STATE_SW_CNT_2	= 0;
			SocCounter.SOC_0_CMP_CNT	= 0;
			SocCounter.CHARGE_UD_1A		= 0;
			SocCounter.CHARGE_LD_CNT	= 0;
			SocCounter.DISCHG_OV_2A 	= 0;
			SocCounter.CHARGE_VOLT_CNT= 0;
			SocCounter.CHG_CNT_30S 		= 0;
			
			Integral.AH_0001_CMP_DIS	= 0;
			Integral.AH_0001_CMP_CHG  = 0;
			Integral.AH_DIS_ONCE			= 0;
			
			Soc.CHG_SOC								= Soc.INDEX;
			if (Soc.SHOW == 0)
			{
				Soc.DIS_SOC 						= 0;
			}
			else if (Marks.A_HOUR)
			{
				Soc.DIS_SOC							= Soc.OCV_SOC;
			}
			else
			{
				Soc.DIS_SOC							= Soc.INDEX;
			}
			
			cap_calc();//when go to charge
			Cap.CAP_DIS_ONCE								= 0;
			Cap.CAP_CHG_LAST 								= Cap.CAP_CHG_ONCE;
			Cap.CAP_GUSS_IN_CURRENT_CHARGE 	= Cap.QNOW;
			
			if (Soc.ZERO > 0)
			{
				SocCounter.SOC_0_CMP_TIME = (Soc.INDEX > SOC_INDEX_VALUE(0.8))?(math_diveder(HOURS(0.3),Soc.ZERO)):(math_diveder(HOURS(2.0),Soc.ZERO));
				BlockParam.C01_CURRENT		= math_diveder(Cap.QNOW*64,80);
				if (Soc.INDEX < Soc.ZERO)
				{
					Soc.ZERO = Soc.INDEX;
				}
			}
			GotoChargeState();
		}
	}
	else if (discharge_current_larger(0.8))
	{
		if (discharge_current_larger(2.0))
		{
			SocCounter.STATE_SW_CNT_1 -= 10;
		}
		
		if (SystemFlg.ACC_IN)
		{
			SocCounter.STATE_SW_CNT_1 -= 10;
		}
		
		if (SocCounter.STATE_SW_CNT_1 > -SECONDS(20))
		{
			SocCounter.STATE_SW_CNT_1--;
		}
		else
		{
			//use_record_init(0);
			Soe.CHG_WH_ONCE = 0;
			SocCounter.DIS_LEARN_CNT = 0;
			Marks.ZERO_START					= 0;
			Marks.SHOW_DEC 						= 0;
			Marks.ZERO								= 0;
			Marks.CAP_INC							= 0;
			Marks.DIS_HPN							= 1;
			
			SocCounter.DIS_TIME_CNT		= 0;
			SocCounter.STATE_SW_CNT_1	= 0;
			SocCounter.STATE_SW_CNT_2	= 0;
			SocCounter.ZERO_CNT				= 0;
			SocCounter.DIS_BLOCK_CNT	= 0;
			SocCounter.DIS_NOFULL_CNT	= 0;
			
			Integral.AH_CHG_ONCE			= 0;
			Integral.ZERO_AH					= 0;
			Integral.ZERO_VOLTAGE			= 0;
			Integral.DIS_AH_BLOCK			= AH(0.11);
			
			BlockParam.AVG_VOLTAGE		= 0;
			BlockParam.AVG_CURRENT		= 0;
			
			cap_calc();//when goto discharge
			
			Cap.CAP_CHG_ONCE					= 0;
			Cap.CAP_GUSS_TOTAL_DIS		= 0;
			Cap.INDEX									= 2;
			Cap.CAP_GUSS_IN_CURRENT_CHARGE	= 0;
				
			if (Marks.A_HOUR && Marks.FULL_DIS == 0)
			{
				Cap.CAP_CHG_LAST 				= 0;
				Cap.CAP_DIS_ONCE 				= 0;
				Soc.DIS_SOC			 				= Soc.OCV_SOC;
			}
			
			if (Soc.DIS_SOC || Marks.FULL_DIS)
			{
				Soc.DIS_SOC = SOC_INDEX_VALUE(1/100);
			}
			GotoDisChargeState();
		}
	}
	else
	{
		SocCounter.STATE_SW_CNT_1 = 0;
		
		if (Soc.SHOW && SystemParam.AVG_VOLTAGE < TOTAL_CELL_VOLTAGE(10.45))
		{
			if (SocCounter.DIS_TIME_CNT < MINITES(1.5))
			{
				SocCounter.DIS_TIME_CNT++;
				//SystemTick.SLEEP_CNT = 0;
			}
			else
			{
				Marks.SHOW_DEC = 1;
				Soc.ZERO 			 = 0;
				Soc.INDEX 		 = 0;
			}
		}
		else
		{
			SocCounter.DIS_TIME_CNT = 0;
			
			if (++SocCounter.STATE_SW_CNT_2 >= MINITES(30))
			{
				SocCounter.STATE_SW_CNT_2 = 0;
				soc_ocv_process();
				Integral.AH = Integral.AH_LAST;
				Soe.HOURS_12_CNT++;
			}
			else if (SystemFlg.SLEEPING)
			{
				//if (SystemTick.SLEEP_CNT == 299)
				//{
				//	SocCounter.STATE_SW_CNT_2 += SECONDS(SLEEP_SEC);
				//}
			}
		}
	}
	
	if (Soe.HOURS_12_CNT >= 24)
	{
		Soe.HOURS_12_CNT = 0;
		Soe.SOE = Soe.SOE*1013>>10;
		Soe.INTEGRAL = 0;
	}
	else if (Soc.SHOW == 0 && Soe.SOE)
	{
		Soe.SOE--;
		Soe.INTEGRAL = 0;
	}
	else
	{
		s16 temp_min, temp_max;
		temp_min = soe_full_calc(Cap.QNOW);
		temp_max = soe_calc_by_soc(Soc.SHOW + SOC_SHOW_VALUE(15/100), temp_min);
		temp_min = soe_calc_by_soc(Soc.SHOW - SOC_SHOW_VALUE(15/100), temp_min);
		
		if (Soe.SOE < temp_min)
		{
			Soe.SOE++;
			Soe.INTEGRAL = 0;
		}
		else if (Soe.SOE > temp_max)
		{
			Soe.SOE--;
			Soe.INTEGRAL = 0;
		}
	}
}

void soc_process(void)
{
	switch(SystemParam.STATE)
	{
		case SOC_DISCHARGE_ST:
			discharge_process();
			break;
		case SOC_CHARGE_ST:
			charge_process();
			break;
		case SOC_FULL_CHARGE_ST:
			full_charge_process();
			break;
		default:
			current_zero_process();
			break;
	}
}
//																	0								10								20								30							40								50							60							70								80							90							100
const u16 OCV_35C[11] 	= {VOLTAGE_V(11.40),VOLTAGE_V(11.63),VOLTAGE_V(11.86),VOLTAGE_V(12.05),VOLTAGE_V(12.22),VOLTAGE_V(12.40),VOLTAGE_V(12.58),VOLTAGE_V(12.74),VOLTAGE_V(12.90),VOLTAGE_V(13.05),VOLTAGE_V(13.15)};
const u16 OCV_25C[11] 	= {VOLTAGE_V(11.40),VOLTAGE_V(11.68),VOLTAGE_V(11.87),VOLTAGE_V(12.06),VOLTAGE_V(12.25),VOLTAGE_V(12.43),VOLTAGE_V(12.62),VOLTAGE_V(12.78),VOLTAGE_V(12.90),VOLTAGE_V(13.06),VOLTAGE_V(13.18)};
const u16 OCV_15C[11] 	= {VOLTAGE_V(11.45),VOLTAGE_V(11.73),VOLTAGE_V(11.92),VOLTAGE_V(12.11),VOLTAGE_V(12.30),VOLTAGE_V(12.46),VOLTAGE_V(12.65),VOLTAGE_V(12.82),VOLTAGE_V(12.95),VOLTAGE_V(13.08),VOLTAGE_V(13.16)};
const u16 OCV_0C[11] 		= {VOLTAGE_V(11.58),VOLTAGE_V(11.75),VOLTAGE_V(11.96),VOLTAGE_V(12.12),VOLTAGE_V(12.29),VOLTAGE_V(12.42),VOLTAGE_V(12.62),VOLTAGE_V(12.80),VOLTAGE_V(12.90),VOLTAGE_V(13.04),VOLTAGE_V(13.12)};
const u16 OCV__10C[11] 	= {VOLTAGE_V(11.75),VOLTAGE_V(11.85),VOLTAGE_V(11.97),VOLTAGE_V(12.11),VOLTAGE_V(12.24),VOLTAGE_V(12.39),VOLTAGE_V(12.53),VOLTAGE_V(12.66),VOLTAGE_V(12.80),VOLTAGE_V(12.95),VOLTAGE_V(13.05)};

u16 soc_ocv_calc(u16 voltage)
{
	s32 temps32, ocv_value, ocv_value2;
	
	if (SystemParam.TEMPERATURE >= 35)
	{
		for (temps32 = 0; temps32 < 11; temps32++)
		{
			if (voltage < OCV_35C[temps32]*Bat.CELL_NUM)
			{
				break;
			}
		}
		
		if (temps32 == 0)
		{
			ocv_value = 0;
		}
		else if (temps32 >= 11)
		{
			ocv_value = SOC_INDEX_MAX;
		}
		else
		{
			temps32--;
			ocv_value = temps32*SOC_INDEX_MAX;
			ocv_value = two_point_diff_calc(ocv_value, SOC_INDEX_MAX, (voltage - OCV_35C[temps32]*Bat.CELL_NUM), Bat.CELL_NUM*(OCV_35C[temps32 + 1] - OCV_35C[temps32]));
			ocv_value = math_diveder(ocv_value,10);
		}
	}
	else if (SystemParam.TEMPERATURE >= 25)
	{
		for (temps32 = 0; temps32 < 11; temps32++)
		{
			if (voltage < OCV_35C[temps32]*Bat.CELL_NUM)
			{
				break;
			}
		}
		
		if (temps32 == 0)
		{
			ocv_value = 0;
		}
		else if (temps32 >= 11)
		{
			ocv_value = SOC_INDEX_MAX;
		}
		else
		{
			temps32--;
			ocv_value = temps32*SOC_INDEX_MAX;
			ocv_value = two_point_diff_calc(ocv_value, SOC_INDEX_MAX, (voltage - OCV_35C[temps32]*Bat.CELL_NUM), Bat.CELL_NUM*(OCV_35C[temps32 + 1] - OCV_35C[temps32]));
			ocv_value = math_diveder(ocv_value,10);
		}
		
		for (temps32 = 0; temps32 < 11; temps32++)
		{
			if (voltage < OCV_25C[temps32]*Bat.CELL_NUM)
				break;
		}
		
		if (temps32 == 0)
		{
			ocv_value2 = 0;
		}
		else if (temps32 >= 11)
		{
			ocv_value2 = SOC_INDEX_MAX;
		}
		else
		{
			temps32--;
			ocv_value2 = temps32*SOC_INDEX_MAX;
			ocv_value2 = two_point_diff_calc(ocv_value2, SOC_INDEX_MAX, (voltage - OCV_25C[temps32]*Bat.CELL_NUM), Bat.CELL_NUM*(OCV_25C[temps32 + 1] - OCV_25C[temps32]));
			ocv_value2 = math_diveder(ocv_value2,10);
		}
		
		ocv_value = two_point_diff_calc(ocv_value2, ocv_value - ocv_value2, SystemParam.TEMPERATURE-25, 10);
	}
	else if (SystemParam.TEMPERATURE >= 15)
	{
		for (temps32 = 0; temps32 < 11; temps32++)
		{
			if (voltage < OCV_25C[temps32]*Bat.CELL_NUM)
			{
				break;
			}
		}
		
		if (temps32 == 0)
		{
			ocv_value = 0;
		}
		else if (temps32 >= 11)
		{
			ocv_value = SOC_INDEX_MAX;
		}
		else
		{
			temps32--;
			ocv_value = temps32*SOC_INDEX_MAX;
			ocv_value = two_point_diff_calc(ocv_value, SOC_INDEX_MAX, (voltage - OCV_25C[temps32]*Bat.CELL_NUM), Bat.CELL_NUM*(OCV_25C[temps32 + 1] - OCV_25C[temps32]));
			ocv_value = math_diveder(ocv_value,10);
		}
		
		for (temps32 = 0; temps32 < 11; temps32++)
		{
			if (voltage < OCV_15C[temps32]*Bat.CELL_NUM)
				break;
		}
		
		if (temps32 == 0)
		{
			ocv_value2 = 0;
		}
		else if (temps32 >= 11)
		{
			ocv_value2 = SOC_INDEX_MAX;
		}
		else
		{
			temps32--;
			ocv_value2 = temps32*SOC_INDEX_MAX;
			ocv_value2 = two_point_diff_calc(ocv_value2, SOC_INDEX_MAX, (voltage - OCV_15C[temps32]*Bat.CELL_NUM), Bat.CELL_NUM*(OCV_15C[temps32 + 1] - OCV_15C[temps32]));
			ocv_value2 = math_diveder(ocv_value2,10);
		}
		
		ocv_value = two_point_diff_calc(ocv_value2, ocv_value - ocv_value2, SystemParam.TEMPERATURE-15, 10);
	}
	else if (SystemParam.TEMPERATURE >= 0)
	{
		for (temps32 = 0; temps32 < 11; temps32++)
		{
			if (voltage < OCV_15C[temps32]*Bat.CELL_NUM)
			{
				break;
			}
		}
		
		if (temps32 == 0)
		{
			ocv_value = 0;
		}
		else if (temps32 >= 11)
		{
			ocv_value = SOC_INDEX_MAX;
		}
		else
		{
			temps32--;
			ocv_value = temps32*SOC_INDEX_MAX;
			ocv_value = two_point_diff_calc(ocv_value, SOC_INDEX_MAX, (voltage - OCV_15C[temps32]*Bat.CELL_NUM), Bat.CELL_NUM*(OCV_15C[temps32 + 1] - OCV_15C[temps32]));
			ocv_value = math_diveder(ocv_value,10);
		}
		
		for (temps32 = 0; temps32 < 11; temps32++)
		{
			if (voltage < OCV_0C[temps32]*Bat.CELL_NUM)
			{
				break;
			}
		}
		
		if (temps32 == 0)
		{
			ocv_value2 = 0;
		}
		else if (temps32 >= 11)
		{
			ocv_value2 = SOC_INDEX_MAX;
		}
		else
		{
			temps32--;
			ocv_value2 = temps32*SOC_INDEX_MAX;
			ocv_value2 = two_point_diff_calc(ocv_value2, SOC_INDEX_MAX, (voltage - OCV_0C[temps32]*Bat.CELL_NUM), Bat.CELL_NUM*(OCV_0C[temps32 + 1] - OCV_0C[temps32]));
			ocv_value2 = math_diveder(ocv_value2,10);
		}
		
		ocv_value = two_point_diff_calc(ocv_value2, ocv_value - ocv_value2, SystemParam.TEMPERATURE, 15);
	}
	else if (SystemParam.TEMPERATURE >= -10)
	{
		for (temps32 = 0; temps32 < 11; temps32++)
		{
			if (voltage < OCV_0C[temps32]*Bat.CELL_NUM)
			{
				break;
			}
		}
		
		if (temps32 == 0)
		{
			ocv_value = 0;
		}
		else if (temps32 >= 11)
		{
			ocv_value = SOC_INDEX_MAX;
		}
		else
		{
			temps32--;
			ocv_value = temps32*SOC_INDEX_MAX;
			ocv_value = two_point_diff_calc(ocv_value, SOC_INDEX_MAX, (voltage - OCV_0C[temps32]*Bat.CELL_NUM), Bat.CELL_NUM*(OCV_0C[temps32 + 1] - OCV_0C[temps32]));
			ocv_value = math_diveder(ocv_value,10);
		}
		
		for (temps32 = 0; temps32 < 11; temps32++)
		{
			if (voltage < OCV__10C[temps32]*Bat.CELL_NUM)
			{
				break;
			}
		}
		
		if (temps32 == 0)
		{
			ocv_value2 = 0;
		}
		else if (temps32 >= 11)
		{
			ocv_value2 = SOC_INDEX_MAX;
		}
		else
		{
			temps32--;
			ocv_value2 = temps32*SOC_INDEX_MAX;
			ocv_value2 = two_point_diff_calc(ocv_value2, SOC_INDEX_MAX, (voltage - OCV__10C[temps32]*Bat.CELL_NUM), Bat.CELL_NUM*(OCV__10C[temps32 + 1] - OCV__10C[temps32]));
			ocv_value2 = math_diveder(ocv_value2,10);
		}
		
		ocv_value = two_point_diff_calc(ocv_value2, ocv_value - ocv_value2, SystemParam.TEMPERATURE + 10, 10);
	}
	else
	{
		for (temps32 = 0; temps32 < 11; temps32++)
		{
			if (voltage < OCV__10C[temps32]*Bat.CELL_NUM)
			{
				break;
			}
		}
		
		if (temps32 == 0)
		{
			ocv_value = 0;
		}
		else if (temps32 >= 11)
		{
			ocv_value = SOC_INDEX_MAX;
		}
		else
		{
			temps32--;
			ocv_value = temps32*SOC_INDEX_MAX;
			ocv_value = two_point_diff_calc(ocv_value, SOC_INDEX_MAX, (voltage - OCV__10C[temps32]*Bat.CELL_NUM), Bat.CELL_NUM*(OCV__10C[temps32 + 1] - OCV__10C[temps32]));
			ocv_value = math_diveder(ocv_value,10);
		}
	}
	
	if (ocv_value > SOC_INDEX_MAX)
	{
		ocv_value = SOC_INDEX_MAX;
	}
	else if (ocv_value < 0)
	{
		ocv_value = 0;
	}
	
	return ocv_value;
}

