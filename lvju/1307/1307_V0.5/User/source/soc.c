#include "main.h"
#include "soc.h"
#include "hwparam.h"
#include "toolfun.h"
#include "userfun.h"

battype 	Bat;
soctype 	Soc;
captype 	Cap;
histype 	His;
soetype		Soe;
markstype Marks;
soc_integral_type Integral;

 


s16 soc_show_calc1(s16 index)
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

s16 soc_show_calc(s16 index)
{
	s16 value;
	
	if (index > Soc.ZERO)
	{
		value = index - Soc.ZERO;
		if (SocShowAllowIncState)
		{
			value = math_diveder(value*SOC_SHOW_MAX,Soc.FULL - Soc.ZERO);
		}
		else
		{
			value = math_diveder(value*SOC_SHOW_MAX + ((Soc.FULL - Soc.ZERO)*value /(1500 - value) ),Soc.FULL - Soc.ZERO);
		}
		
		if (value > SOC_SHOW_MAX)
		{
			value = SOC_SHOW_MAX;
		}
	}
	else
	{
		value = 0;
		Soc.ZERO = Soc.INDEX;
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
	static s16 show_cnt,realsoc;
	static u16 setfulldelay;

	
	if (show_cnt < (SocShowAllowIncState?20:4) && Marks.SHOW_DEC == 0)
	{
		show_cnt++; 
	}
	else
	{
		realsoc = soc_show_calc(Soc.INDEX); 
		if (Soc.SHOW > realsoc)
		{
			if (!SocShowAllowIncState)
			{
				Soc.SHOW--;
			} 
			Marks.SHOW_FULL = 0;
		}
		else if (Soc.SHOW < realsoc)
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






void soc_show_process1(void)
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

void soe_full_calc(u16 q_value)
{
	Soe.FULL = q_value*Bat.CELL_NUM*12>>3;
}

s16 soe_calc_by_soc(s16 soc_show)
{
	return  math_diveder(Soe.FULL*soc_show,SOC_SHOW_MAX);
}

void soe_init(void)
{
	soe_full_calc(Cap.QNOW);
	Soe.SOE = soe_calc_by_soc(Soc.SHOW);
	Soe.INTEGRAL 			= 0;
	Soe.HOURS_12_CNT 	= 0;
}

void soe_state_machine(void)
{
	switch(SystemParam.STATE)
	{
		case SOC_DISCHARGE_ST:
			
			Soe.INTEGRAL += Soe.POWER;
			Soe.CHG_WH_ONCE = 0;
		
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
			break;
		case SOC_CHARGE_ST:
			Soe.INTEGRAL += Soe.POWER;
			Soe.DIS_WH_ONCE = 0;
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
			break;
		case SOC_FULL_CHARGE_ST:
			if (Soe.SOE < Soe.FULL)
			{
				Soe.SOE++;
			}
			Soe.INTEGRAL = 0;
			break;
		default:
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
				s16 temp_max, temp_min;
				temp_max = soe_calc_by_soc(Soc.SHOW + SOC_SHOW_VALUE(15/100));
				temp_min = soe_calc_by_soc(Soc.SHOW - SOC_SHOW_VALUE(15/100));
				
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
			break;
	}	
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
	Soc.INDEX 									= soc_ocv_calc(SystemParam.BUS_VOLTAGE);
	
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
	Marks.FULL									= 0;
	Marks.FULL_DIS 							= 0;
	SystemFlg.SOC_SAVE					= 1;
	cap_learn_param_init();
	cap_calc();//when reset
	soe_init();
	AimaParam.LAST_TRIP 		= last_trip_calc();
}

void soc_init(void)
{
	s16 TempS16;
	
	if (SystemFlg.WDG_REST == 0)
	{
		if (Soc.SHOW == SOC_SHOW_MAX)
		{
			TempS16 = (SystemParam.BUS_VOLTAGE > TOTAL_CELL_VOLTAGE(13.10))?0:(His.LAST_VOLT - AdcAvg.VOLTAGE);
		}
		else
		{
			TempS16 = His.LAST_VOLT - AdcAvg.VOLTAGE;
		}
		
		if ((SystemFlg.ERT_SAVE == 0 && (TempS16 > BATTERY_VOLTAGE(1.5) || TempS16 < -BATTERY_VOLTAGE(0.5))) ||
				 SystemFlg.CNCHANGE || His.WORK_SEC < 3600)
		{
			soc_reset(CAP_RATING_DEFAULT,Bat.CELL_NUM, 0);
		}
		else
		{
			TempS16 = soc_ocv_calc(SystemParam.BUS_VOLTAGE);
			#ifdef SHOW_DEC_SLOW
			if (TempS16 > Soc.DEC_SHOW)
			{
				TempS16 = math_diveder((TempS16*2 - Soc.DEC_SHOW)*SOC_SHOW_MAX,Soc.FULL + TempS16 - Soc.DEC_SHOW);
			}
			else
			{
				TempS16 = math_diveder(TempS16*SOC_SHOW_MAX,Soc.FULL);
			}
			TempS16 -= Soc.SHOW;
			#else
			TempS16 = soc_show_calc(TempS16) - Soc.SHOW;
			#endif
			TempS16	= abs(TempS16);
			if ((SystemParam.BUS_VOLTAGE > TOTAL_CELL_VOLTAGE(13.2) && TempS16 > SOC_SHOW_VALUE(0.1)) ||
				 (SystemParam.BUS_VOLTAGE < TOTAL_CELL_VOLTAGE(11.3) && TempS16 > SOC_SHOW_VALUE(0.1)) ||
				 TempS16 > SOC_SHOW_VALUE(0.35))
			{
				Soc.INDEX = soc_ocv_calc(SystemParam.BUS_VOLTAGE);
				#ifdef SHOW_DEC_SLOW
				if (Soc.INDEX > Soc.DEC_SHOW)
				{
					Soc.ZERO 	= Soc.DEC_SHOW - Soc.INDEX;
				}
				else
				{
					Soc.ZERO 	= 0;
				}
				#else
				Soc.ZERO		= 0;
				#endif
				Soc.FULL	= SOC_INDEX_MAX;
				Soc.SHOW	= soc_show_calc(Soc.INDEX);
				soe_init();
			}
		}
	}
	
	Marks.DIS_HPN = 1;
	
	if (Soc.SHOW == SOC_SHOW_MAX)
	{
		Marks.FULL 			= 1;
		Marks.FULL_DIS 	= 1;
	}
	else if (Soc.INDEX < Soc.ZERO)
	{
		if (Soc.ZERO >= 0)
		{
			Soc.INDEX = Soc.ZERO;
		}
		else
		{
			Soc.ZERO = Soc.INDEX;
		}
		Soc.SHOW	= 0;
	}
	
	if (Soc.INDEX > Soc.FULL)
	{
		Soc.FULL = Soc.INDEX;
	}
	
	Soc.OCV_SOC = Soc.INDEX;
	Soc.DIS_SOC = Soc.INDEX;
	
	His.CAPSUM_LST = His.CAPSUM;
	cap_calc();//when initial
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
	
	use_record_end();
}

void charge_process(void)
{
 
	
		u16 TempU16;
	static  unsigned short cnt95;
//static  unsigned short cnt98;
	static  unsigned int STATE_SW_CNT_3;	
 
	
	Integral.AH += SystemParam.BUS_CURRENT;
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
	
	Integral.AH_CHG_ONCE += SystemParam.BUS_CURRENT;
	
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
		if (0 == (++SocCounter.CHG_TIME_CNT&0x1FFFFF))
		{
			SystemParam.TEMPERATURE = SystemParam.TEMP_NOW;
		}
	}
	
	if (Soc.ZERO > 0)
	{
		soc_zero_decrease_process();
	}
	else if (Marks.FBD_SLW)
	{
		Marks.FBD_SLW = 0;
	}
	
	block_voltage_current_calc_in_charge();
	
	if (Marks.CHG_VOLT)
	{
		if (SocCounter.CHARGE_VOLT_CNT < HOURS(2))
		{
			SocCounter.CHARGE_VOLT_CNT++;
		}
		
		if (SocCounter.BLOCK_CNT >= ((charge_current_smaller(1.2) && SystemParam.BUS_VOLTAGE >= TOTAL_CELL_VOLTAGE(14.30))?SECONDS(5):SECONDS(30)) && Integral.SUM_CURRENT > 0)
		{
			charge_soc_compensation_in_voltage();
		}
		
		TempU16 = BlockParam.CHG_CURRENT*14>>6;
		if (TempU16 < CURRENT_A(0.35))
		{
			TempU16 = CURRENT_A(0.35);
		}
		else if (TempU16 > CURRENT_A(1.5))
		{
			TempU16 = CURRENT_A(1.5);
		}
		
		if (SystemParam.BUS_CURRENT < TempU16 && SystemParam.BUS_VOLTAGE >= TOTAL_CELL_VOLTAGE(14.30))
		{
			if (SocCounter.CHARGE_LD_CNT < MINITES(50))
			{
				SocCounter.CHARGE_LD_CNT++;
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
					SystemParam.BUS_VOLTAGE > TOTAL_CELL_VOLTAGE(13.65) && Soc.SHOW < SOC_SHOW_VALUE(0.98))
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
		
		if (Integral.SUM_CURRENT >= AH(0.3))
		{
			charge_soc_compensation_in_current();
		}
	}
	
	if (SystemParam.BUS_VOLTAGE > TOTAL_CELL_VOLTAGE(13.6) && charge_current_smaller(1.0))
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
		if (charge_current_larger(1.0) || SystemParam.BUS_VOLTAGE > TOTAL_CELL_VOLTAGE(13.90))
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
	
	if (Marks.FORCE_FULL == 0 && ((charge_current_smaller(0.35) && SystemParam.BUS_VOLTAGE < TOTAL_CELL_VOLTAGE(13.35) && Soc.SHOW < SOC_SHOW_VALUE(96/100)) || 
			(SocCounter.CHARGE_UD_1A > MINITES(60) && SystemParam.BUS_VOLTAGE < TOTAL_CELL_VOLTAGE(13.3)) ||
			(SocCounter.CHG_TIME_CNT >= HOURS(18) && SystemParam.BUS_VOLTAGE < TOTAL_CELL_VOLTAGE(13.6) && charge_current_smaller(1.0)) ||
			discharge_current_larger(0.75) || (SystemFlg.CRG_VALID && SystemFlg.CRG_IN == 0 && charge_current_smaller(0.5))))
	{
		if (SystemParam.BUS_VOLTAGE < TOTAL_CELL_VOLTAGE(13.0))
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
	
	if (SystemParam.BUS_VOLTAGE > TOTAL_CELL_VOLTAGE(13.35) && charge_current_smaller(1.0) && discharge_current_smaller(0.75) && Soc.INDEX >= SOC_FULL_MIN &&
		 ((((Soc.SHOW > SOC_SHOW_VALUE(96/100) && (SocCounter.CHARGE_VOLT_CNT > MINITES(45) || SocCounter.CHARGE_LD_CNT > MINITES(2)))||
			(SocCounter.CHARGE_LD_CNT > MINITES(2.0) && SystemParam.BUS_VOLTAGE > TOTAL_CELL_VOLTAGE(13.45)) || 
			(SystemFlg.CRG_VALID && SystemFlg.CRG_IN && discharge_current_smaller(0.75) && charge_current_smaller(0.8) && SystemParam.BUS_VOLTAGE > TOTAL_CELL_VOLTAGE(13.5))||
			(Soc.SHOW > SOC_SHOW_VALUE(97/100) && charge_current_smaller(0.5) && SystemParam.BUS_VOLTAGE > TOTAL_CELL_VOLTAGE(13.7)))&& SystemParam.BUS_VOLTAGE < TOTAL_CELL_VOLTAGE(14.0))||
			SocCounter.CHARGE_VOLT_CNT > MINITES(140) || SocCounter.CHARGE_UD_1A > MINITES(60) || Marks.FORCE_FULL))
	{
		if (SocCounter.STATE_SW_CNT_2 < MINITES(1.5) && Marks.FORCE_FULL == 0)
		{
			SocCounter.STATE_SW_CNT_2++;
		}
		else
		{
			SystemFlg.SOC_SAVE	= 1;
			system_prepare_to_full();
		}
	}
	else
	{
		SocCounter.STATE_SW_CNT_2 = 0;
	} 
	
	
}

void full_charge_process(void)
{
	Integral.AH += SystemParam.BUS_CURRENT;
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
	
	if (SystemParam.BUS_VOLTAGE > TOTAL_CELL_VOLTAGE(14.3) && charge_current_larger(1.0))
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
	else if ((SystemParam.BUS_VOLTAGE < TOTAL_CELL_VOLTAGE(13.70) && (discharge_current_larger(0.1) || SocCounter.CHG_TIME_CNT >= HOURS(18))))
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
}

void discharge_process(void)
{
	Integral.AH += SystemParam.BUS_CURRENT;
	
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
	
	Integral.AH_DIS_ONCE -= SystemParam.BUS_CURRENT;
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
	
	if (Integral.ZERO_AH >= AH(0.06))
	{
		u32 TempU32;
		Integral.ZERO_VOLTAGE = math_diveder(Integral.ZERO_VOLTAGE,SocCounter.ZERO_CNT);
		Integral.ZERO_AH			= math_diveder(Integral.ZERO_AH,SocCounter.ZERO_CNT);
		
		if (Bat.CELL_NUM >= 6)
		{
			TempU32 = 400*CURRENT_UNIT*VOLTAGE_UNIT;
		}
		else if (Bat.CELL_NUM == 5)
		{
			TempU32 = 350*VOLTAGE_UNIT*CURRENT_UNIT;
		}
		else
		{
			TempU32 = 280*VOLTAGE_UNIT*CURRENT_UNIT;
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
	else if (SocCounter.ZERO_CNT >= MINITES(3) || SystemParam.BUS_VOLTAGE > TOTAL_CELL_VOLTAGE(12.5) || Marks.ZERO)
	{
		SocCounter.ZERO_CNT 	= 0;
		Integral.ZERO_AH			= 0;
		Integral.ZERO_VOLTAGE = 0;
	}
	else
	{
		SocCounter.ZERO_CNT++;
		Integral.ZERO_VOLTAGE += SystemParam.BUS_VOLTAGE;
		Integral.ZERO_AH 			-= SystemParam.BUS_CURRENT;
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
	
	if (charge_current_larger(1.5) || SystemParam.BUS_VOLTAGE > TOTAL_CELL_VOLTAGE(14.0))
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
		
		TempS16 = charge_current_last - SystemParam.BUS_CURRENT;
		if (abs(TempS16) > CURRENT_A(0.8))
		{
			charge_current_last = SystemParam.BUS_CURRENT;
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
}

void current_zero_process(void)
{
	Integral.AH += SystemParam.BUS_CURRENT;
	
	if (Integral.AH > Integral.AH_0001_CHG)
	{
		Integral.AH = Integral.AH_0001_CHG;
	}
	else if (Integral.AH < -Integral.AH_0001_DIS)
	{
		Integral.AH = -Integral.AH_0001_DIS;
	}
	
	block_voltage_current_calc();
	
	if (charge_current_larger(0.8) || SystemParam.BUS_VOLTAGE > TOTAL_CELL_VOLTAGE(14.0))
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
			use_record_init(1);
			system_prepare_to_charge();
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
			use_record_init(0);
			system_prepare_to_discharge();
		}
	}
	else
	{
		SocCounter.STATE_SW_CNT_1 = 0;
		
		if (Soc.SHOW && SystemParam.BUS_VOLTAGE < TOTAL_CELL_VOLTAGE(10.45))
		{
			if (SocCounter.DIS_TIME_CNT < MINITES(1.5))
			{
				SocCounter.DIS_TIME_CNT++;
				SystemTick.SLEEP_CNT = 0;
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
				if (soc_ocv_process())
				{
					SystemFlg.SOC_SAVE = 1;
				}
				Integral.AH = Integral.AH_LAST;
				Soe.HOURS_12_CNT++;
			}
			else if (SystemFlg.SLEEPING)
			{
				if (SystemTick.TICK_WAKEUP == 10)
				{
					SocCounter.STATE_SW_CNT_2 += SECONDS(SLEEP_SEC);
				}
			}
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

