#include "main.h"
#include "soc.h"
#include "hwparam.h"
#include "flash.h"
#include "toolfun.h"
#include "userfun.h"

void sleep_process(void)//10ms
{
	if (AccIOIsValid || CrgIOIsValid || SystemFlg.CRG_IN || SystemFlg.ACC_IN || SystemFlg.RECORDING ||
			Marks.SHOW_DEC || SystemFlg.SOC_SAVE || SystemFlg.ERT_SAVE || !SocReadyState ||
			abs(SystemParam.BUS_CURRENT) > CURRENT_A(0.8) || SystemParam.BUS_VOLTAGE > TOTAL_CELL_VOLTAGE(14.0))
	{
		SystemTick.SLEEP_CNT = 0;
		if (SystemFlg.SLEEPING)
		{
			SystemFlg.SLEEPING = 0;
		}
	}
	else if (SystemTick.SLEEP_CNT < 6000)//60s
	{
		SystemTick.SLEEP_CNT++;
	}
	else
	{
		goto_sleep();
		adc_offset_calc();
	}
}

void b_and_p_check_process(void)//100ms
{
	static u16 volt_check;
	static s16 check_state_cnt;
	static u8 check_state = 0, check_cnt;
	s16 TempS16;
	
	if (SystemFlg.TESTERING == 0 && SystemFlg.BP_VALID == 0)
	{
		if (SystemFlg.SLEEPING)
		{
			check_state = 0;
			check_cnt 	= 0;
			volt_check 	= SystemParam.BUS_VOLTAGE;
			return;
		}
		
		if (check_state <= 1)
		{
			TempS16 = volt_check - SystemParam.BUS_VOLTAGE;
			if (abs(TempS16) > VOLTAGE_V(0.5))
			{
				if (check_state && (abs(SystemParam.BUS_CURRENT) > CURRENT_A(0.8) || SystemParam.BUS_VOLTAGE > TOTAL_CELL_VOLTAGE(13.9)))
				{
					check_state = 2;
				}
				else
				{
					check_state = 0;
					check_cnt   = 0;
					volt_check  = SystemParam.BUS_VOLTAGE;
				}
			}
			else if (check_cnt < 30)
			{
				check_cnt++;
			}
			else if (check_state == 0)
			{
				check_state 		= 1;
				check_state_cnt = 0;
			}
		}
		else
		{
			if (abs(SystemParam.BUS_CURRENT) < CURRENT_A(0.5) && SystemParam.BUS_VOLTAGE < TOTAL_CELL_VOLTAGE(14))
			{
				if (check_cnt < 100)
				{
					check_cnt++;
				}
				else
				{
					check_state = 0;
					check_cnt		= 0;
				}
			}
			else
			{
				check_cnt = 0;
				
				if (abs(check_state_cnt) < 30000 || check_state_cnt*SystemParam.BUS_CURRENT < 0)
				{
					check_state_cnt += SystemParam.BUS_CURRENT>>6;
				}
				
				if (SocChargeState)
				{
					if (check_state_cnt >= 3000)
					{
						if (volt_check < SystemParam.BUS_VOLTAGE)
						{
							check_state = 2;
							if (++check_state_cnt >= 3200 && SystemParam.BUS_CURRENT > 0)
							{
								SystemFlg.BP_VALID = 1;
								SystemFlg.ERT_SAVE = 1;
							}
						}
						else if (check_state < 100)
						{
							check_state++;
							check_state_cnt = 3000;
						}
						else
						{
							SystemFlg.BP_SWITCH ^= 1;
							check_state_cnt = 0;
						}
					}
					else if (check_state_cnt < -1000)
					{
						SystemFlg.BP_SWITCH ^= 1;
						check_state_cnt = 0;
					}
					else if (SystemParam.BUS_CURRENT > CURRENT_A(25))
					{
						if (++check_state > 30)
						{
							SystemFlg.BP_SWITCH ^= 1;
							check_state_cnt = 0;
						}
					}
					else
					{
						check_state = 2;
					}
				}
				else if (SocDisChargeState)
				{
					if (check_state_cnt <= -3000)
					{
						if (volt_check > SystemParam.BUS_VOLTAGE)
						{
							check_state = 2;
							if (--check_state_cnt <= -3100 && SystemParam.BUS_CURRENT < 0)
							{
								SystemFlg.BP_VALID = 1;
								SystemFlg.ERT_SAVE = 1;
							}
						}
						else if (check_state < 200)
						{
							check_state++;
							check_state_cnt = -3000;
						}
						else
						{
							SystemFlg.BP_SWITCH ^= 1;
							check_state_cnt = 0;
						}
					}
				}
			}
		}
	}
}

void his_voltage_calc(void)//100ms
{
	static u16 last_volt, last_volt_cnt, pwr_off_clr_cnt;
	s16 TempS16;
	
	TempS16 = last_volt - AdcAvg.VOLTAGE;
	
	if (abs(TempS16) > BATTERY_VOLTAGE(0.75))
	{
		if (!WakeUpNotReady)
		{
			last_volt 		= AdcAvg.VOLTAGE;
			last_volt_cnt = 0;
		}
	}
	else if (last_volt_cnt < 30)//3s
	{
		last_volt_cnt++;
	}
	else
	{
		last_volt_cnt = 0;
		His.LAST_VOLT = last_volt;
	}
	
	if (SystemFlg.PWR_OFF)
	{
		if (++pwr_off_clr_cnt >= 80)
		{
			SystemFlg.PWR_OFF = 0;
		}
	}
	else if (pwr_off_clr_cnt)
	{
		pwr_off_clr_cnt = 0;
	}
}


void power_off_detect(void)
{
	static u16 pwr_delay;
	static u8 pwr_off_cnt;
	
	if (WakeUpNotReady)
	{
		return;
	}
	
	if (AdcResult.VOLTAGE + BATTERY_VOLTAGE(5.5) < His.LAST_VOLT &&
			abs(SystemParam.BUS_CURRENT) < CURRENT_A(1.5) &&
			AdcResult.VOLTAGE < BATTERY_VOLTAGE(35) && pwr_delay > 20000)
	{
		if (pwr_off_cnt < 5)
		{
			if (++pwr_off_cnt == 4)
			{
				pwr_delay = 0;
				soc_save();
				SystemFlg.PWR_OFF = 1;
			}
		}
	}
	else
	{
		pwr_off_cnt = 0;
		if (pwr_delay < 30000)
		{
			pwr_delay++;
		}
	}
}

void current_offset_re_calc(void)//10ms
{
	static u32 offset_cnt;
	static u16 last_voltage, offset_tmp;
	s16 TempS16;
	
	if (WakeUpNotReady)
	{
		return;
	}
	
	TempS16 = last_voltage - SystemParam.BUS_VOLTAGE;
	
	if (abs(TempS16) > VOLTAGE_V(0.6) || SystemParam.BUS_VOLTAGE > TOTAL_CELL_VOLTAGE(13.6))
	{
		last_voltage = SystemParam.BUS_VOLTAGE;
		offset_cnt = 0;
	}
	else
	{
		offset_cnt++;
		
		if (SystemFlg.SLEEPING && SystemTick.SLEEP_CNT == 5999)
		{
			offset_cnt += 3000;
		}
		
		if (offset_cnt > 1080000 && offset_cnt < 1086000)
		{
			offset_cnt = 1086001;
			current_offset_calc(&offset_tmp);
		}
		else if (offset_cnt >= 1440000)
		{
			if (0 == b_value_verify(&offset_tmp,OPA_OFFSET_DEFAULT+ADC_VALUE(0.65),OPA_OFFSET_DEFAULT-ADC_VALUE(0.65),OPA_OFFSET_DEFAULT))
			{
				for (offset_cnt = 0; offset_cnt < 1024; offset_cnt++)
				{
					while(SystemTick.TICK_ADC < 16);
					adc_filter_process();
					bus_current_calc(offset_tmp);
				}
				
				if (abs(SystemParam.BUS_CURRENT) < CURRENT_A(0.3))
				{
					BValue.OPA_REF = offset_tmp;
				}
			}
		}
	}
}

