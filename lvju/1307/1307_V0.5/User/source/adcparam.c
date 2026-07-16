#include "main.h"
#include "hwparam.h"
#include "soc.h"
#include "toolfun.h"

adc_type AdcResult, AdcSum, AdcAvg;
key_param_raw_type KeyRawParam;

void adc_integral_in_isr(void)
{
	if (SystemTick.TICK_ADC < 16)
	{
		SystemTick.TICK_ADC++;
		AdcSum.VOLTAGE 			+= AdcResult.VOLTAGE;
		AdcSum.CURRENT 			+= AdcResult.CURRENT;
		AdcSum.TEMP_OUTSIDE += AdcResult.TEMP_OUTSIDE;
		AdcSum.TEMPERATURE 	+= AdcResult.TEMPERATURE;
		AdcSum.AVSS					+= AdcResult.AVSS;
	}
}

u8 adc_filter_process(void)
{
	if (SystemTick.TICK_ADC >= 16)
	{
		AdcAvg.VOLTAGE			= (AdcSum.VOLTAGE>>4);
		AdcAvg.CURRENT			= (AdcSum.CURRENT>>4);
		AdcAvg.TEMPERATURE	= (AdcSum.TEMPERATURE>>4);
		AdcAvg.TEMP_OUTSIDE = (AdcSum.TEMP_OUTSIDE>>4);
		AdcAvg.AVSS					= AdcSum.AVSS>>4;
		
		AdcSum.VOLTAGE			= 0;
		AdcSum.CURRENT			= 0;
		AdcSum.AVSS					= 0;
		AdcSum.TEMPERATURE	= 0;
		AdcSum.TEMP_OUTSIDE	= 0;
		SystemTick.TICK_ADC = 0;
		BValue.ADC_VSS			= AdcAvg.AVSS;
		
		return 1;
	}
	return 0;
}

void current_offset_calc(u16 *offset)
{
	u32 sum = 0;
	u8 i;
	for (i = 0; i < 128; i++)
	{
		while(SystemTick.TICK_ADC < 16);
		adc_filter_process();
		sum += AdcAvg.CURRENT;
	}
	*offset = sum>>7;
}

void adc_offset_calc(void)
{
	u32 sum;
	u8 i,j = 0;
	
	do
	{
		for (i = 0, sum = 0; i < 32; i++)
		{
			while(SystemTick.TICK_ADC < 16);
			adc_filter_process();
			sum += AdcAvg.AVSS;
		}
		sum >>= 5;
		if (sum == 0)
		{
			if (ADC1->CTRL0_b.ADCx_OFS_N)
			{
				ADC1->CTRL0_b.ADCx_OFS_N--;
			}
			else if (ADC1->CTRL0_b.ADCx_OFS_P == 3)
			{
				break;
			}
			else
			{
				ADC1->CTRL0_b.ADCx_OFS_P++;
			}
		}
		else if (sum > 3)
		{
			if (ADC1->CTRL0_b.ADCx_OFS_P)
			{
				ADC1->CTRL0_b.ADCx_OFS_P--;
			}
			else if (ADC1->CTRL0_b.ADCx_OFS_N == 3)
			{
				break;
			}
			else
			{
				ADC1->CTRL0_b.ADCx_OFS_N++;
			}
		}
		j++;
	}while((sum == 0 || sum > 3) && j < 6);
}

void current_kvalue_insert(s16 raw_value, u16 k_value)
{
	if (raw_value < -CURRENT_A(40))
	{
		KValue.CURRENT_RAW[0] = raw_value;
		KValue.CURRENT_ARY[0] = k_value;
	}
	else if (raw_value < -CURRENT_A(20))
	{
		KValue.CURRENT_RAW[1] = raw_value;
		KValue.CURRENT_ARY[1] = k_value;
	}
	else if (raw_value < 0)
	{
		KValue.CURRENT_RAW[2] = raw_value;
		KValue.CURRENT_ARY[2] = k_value;
	}
	else if (raw_value < CURRENT_A(20))
	{
		KValue.CURRENT_RAW[3] = raw_value;
		KValue.CURRENT_ARY[3] = k_value;
	}
	else if (raw_value < CURRENT_A(40))
	{
		KValue.CURRENT_RAW[4] = raw_value;
		KValue.CURRENT_ARY[4] = k_value;
	}
	else
	{
		KValue.CURRENT_RAW[5] = raw_value;
		KValue.CURRENT_ARY[5] = k_value;
	}
}

s16 current_calc_by_array(s16 current)
{
	s32 TempS32;
	u8 i;
	
	for (i = 0; i < 6; i++)
	{
		if (current < KValue.CURRENT_RAW[i])
		{
			if (i==0)
			{
				return current*KValue.CURRENT_ARY[0]>>K_VALUE_SHIFT;
			}
			else
			{
				TempS32 =current*KValue.CURRENT_ARY[i - 1]>>K_VALUE_SHIFT;
				TempS32 *= KValue.CURRENT_RAW[i] - current;
				TempS32 += (current*KValue.CURRENT_ARY[i]>>K_VALUE_SHIFT)*(current - KValue.CURRENT_RAW[i-1]);
				return math_diveder(TempS32,KValue.CURRENT_RAW[i] - KValue.CURRENT_RAW[i-1]);
			}
		}
		else if (i == 5)
		{
			return current*KValue.CURRENT_ARY[5]>>K_VALUE_SHIFT;
		}
	}
	
	return current*KValue.CURRENT>>K_VALUE_SHIFT;
}


void bus_current_calc(u16 offset)
{
	static s32 raw_sum = 0, raw_index = 0;
	s16 TempS16;
	
	if (WakeUpNotReady)
	{
		return;
	}
	
	TempS16 								= offset - AdcAvg.CURRENT;
	KeyRawParam.CURRENT 		= TempS16*CURRENT_COV_CNST>>COV_SHIFT;
	
	raw_sum += KeyRawParam.CURRENT;
	if (++raw_index >= 512)
	{
		KeyRawParam.AVG_CURRENT = raw_sum>>9;
		SystemParam.AVG_CURRENT = SystemFlg.CUR_VALID?current_calc_by_array(KeyRawParam.AVG_CURRENT):(KeyRawParam.AVG_CURRENT*KValue.CURRENT>>K_VALUE_SHIFT);
		
		raw_sum 	= 0;
		raw_index = 0;
	}
	
	if (abs(KeyRawParam.CURRENT) > CURRENT_A(5.0))
	{
		SystemParam.BUS_CURRENT = (SystemFlg.CUR_VALID)?current_calc_by_array(KeyRawParam.CURRENT):(KeyRawParam.CURRENT*KValue.CURRENT>>K_VALUE_SHIFT);
	}
	else
	{
		SystemParam.BUS_CURRENT = SystemParam.AVG_CURRENT;
	}
 
	
	#ifdef BP_AUTO_SWITCH
	if (SystemFlg.BP_SWITCH)
	{
		SystemParam.BUS_CURRENT = -SystemParam.BUS_CURRENT;
	}
	#endif
}

void bus_voltage_calc(void)
{
	if (WakeUpNotReady)
	{
		return;
	}
	
	KeyRawParam.VOLTAGE	= AdcAvg.VOLTAGE*BAT_VOLT_CONST>>COV_SHIFT;
	SystemParam.BUS_VOLTAGE = KeyRawParam.VOLTAGE*KValue.VOLTAGE>>K_VALUE_SHIFT;
	
	
	 
}

void avg_power_calc(void)
{
	static s32 pwr_sum;
	static u8 pwr_cnt;
	
	pwr_sum += SystemParam.BUS_VOLTAGE*SystemParam.BUS_CURRENT;
	if (++pwr_cnt >= 32)
	{
		Soe.POWER = pwr_sum>>5;
		pwr_sum = 0;
		pwr_cnt = 0;
		if (Soe.POWER > 0)
		{
			Soe.POWER = Soe.POWER*109>>7;
		}
	}
}

const u16 NTC_PCB_VOLT[256]={
NTC_TMP_RES(3225.5), NTC_TMP_RES(3023.3), NTC_TMP_RES(2834.9), NTC_TMP_RES(2659.5), NTC_TMP_RES(2495.9), NTC_TMP_RES(2343.3), NTC_TMP_RES(2200.9), NTC_TMP_RES(2068.0), NTC_TMP_RES(1943.9), NTC_TMP_RES(1828.0),
NTC_TMP_RES(1719.7), NTC_TMP_RES(1618.4), NTC_TMP_RES(1523.7), NTC_TMP_RES(1435.0), NTC_TMP_RES(1352.1), NTC_TMP_RES(1274.4), NTC_TMP_RES(1201.6), NTC_TMP_RES(1133.4), NTC_TMP_RES(1069.4), NTC_TMP_RES(1009.5),
NTC_TMP_RES(953.19), NTC_TMP_RES(900.37), NTC_TMP_RES(850.79), NTC_TMP_RES(804.21), NTC_TMP_RES(760.45), NTC_TMP_RES(719.32), NTC_TMP_RES(680.64), NTC_TMP_RES(644.27), NTC_TMP_RES(610.04), NTC_TMP_RES(577.82),
NTC_TMP_RES(547.48), NTC_TMP_RES(518.90), NTC_TMP_RES(491.98), NTC_TMP_RES(466.60), NTC_TMP_RES(442.67), NTC_TMP_RES(420.11), NTC_TMP_RES(398.81), NTC_TMP_RES(378.72), NTC_TMP_RES(359.74), NTC_TMP_RES(341.83),
NTC_TMP_RES(324.90), NTC_TMP_RES(308.90), NTC_TMP_RES(293.78), NTC_TMP_RES(279.48), NTC_TMP_RES(265.96), NTC_TMP_RES(253.16), NTC_TMP_RES(241.05), NTC_TMP_RES(229.58), NTC_TMP_RES(218.72), NTC_TMP_RES(208.44),
NTC_TMP_RES(198.69), NTC_TMP_RES(189.45), NTC_TMP_RES(180.69), NTC_TMP_RES(172.38), NTC_TMP_RES(164.50), NTC_TMP_RES(157.02), NTC_TMP_RES(149.92), NTC_TMP_RES(143.17), NTC_TMP_RES(136.77), NTC_TMP_RES(130.69),
NTC_TMP_RES(124.91), NTC_TMP_RES(119.41), NTC_TMP_RES(114.19), NTC_TMP_RES(109.22), NTC_TMP_RES(104.50), NTC_TMP_RES(100.00), NTC_TMP_RES(95.720), NTC_TMP_RES(91.646), NTC_TMP_RES(87.766), NTC_TMP_RES(84.071),
NTC_TMP_RES(80.550), NTC_TMP_RES(77.195), NTC_TMP_RES(73.997), NTC_TMP_RES(70.947), NTC_TMP_RES(68.039), NTC_TMP_RES(65.265), NTC_TMP_RES(62.618), NTC_TMP_RES(60.092), NTC_TMP_RES(57.681), NTC_TMP_RES(55.379),
NTC_TMP_RES(53.180), NTC_TMP_RES(51.080), NTC_TMP_RES(49.073), NTC_TMP_RES(47.155), NTC_TMP_RES(45.321), NTC_TMP_RES(43.569), NTC_TMP_RES(41.892), NTC_TMP_RES(40.289), NTC_TMP_RES(38.755), NTC_TMP_RES(37.286),
NTC_TMP_RES(35.881), NTC_TMP_RES(34.536), NTC_TMP_RES(33.248), NTC_TMP_RES(32.014), NTC_TMP_RES(30.832), NTC_TMP_RES(29.699), NTC_TMP_RES(28.614), NTC_TMP_RES(27.573), NTC_TMP_RES(26.576), NTC_TMP_RES(25.619),
NTC_TMP_RES(24.701), NTC_TMP_RES(23.821), NTC_TMP_RES(22.976), NTC_TMP_RES(22.166), NTC_TMP_RES(21.388), NTC_TMP_RES(20.641), NTC_TMP_RES(19.923), NTC_TMP_RES(19.234), NTC_TMP_RES(18.572), NTC_TMP_RES(17.936),
NTC_TMP_RES(17.325), NTC_TMP_RES(16.738), NTC_TMP_RES(16.173), NTC_TMP_RES(15.630), NTC_TMP_RES(15.108), NTC_TMP_RES(14.605), NTC_TMP_RES(14.122), NTC_TMP_RES(13.657), NTC_TMP_RES(13.209), NTC_TMP_RES(12.779),
NTC_TMP_RES(12.364), NTC_TMP_RES(11.965), NTC_TMP_RES(11.580), NTC_TMP_RES(11.210), NTC_TMP_RES(10.853), NTC_TMP_RES(10.509), NTC_TMP_RES(10.177), NTC_TMP_RES(9.8580), NTC_TMP_RES(9.5500), NTC_TMP_RES(9.2530),
NTC_TMP_RES(8.9670), NTC_TMP_RES(8.6910), NTC_TMP_RES(8.4240), NTC_TMP_RES(8.1670), NTC_TMP_RES(7.9190), NTC_TMP_RES(7.6800), NTC_TMP_RES(7.4490), NTC_TMP_RES(7.2260), NTC_TMP_RES(7.0100), NTC_TMP_RES(6.8020),
NTC_TMP_RES(6.6010), NTC_TMP_RES(6.4070), NTC_TMP_RES(6.2200), NTC_TMP_RES(6.0390), NTC_TMP_RES(5.8640), NTC_TMP_RES(5.6940), NTC_TMP_RES(5.5310), NTC_TMP_RES(5.3730), NTC_TMP_RES(5.2200), NTC_TMP_RES(5.0720),
NTC_TMP_RES(4.9290), NTC_TMP_RES(4.7900), NTC_TMP_RES(4.6560), NTC_TMP_RES(4.5270), NTC_TMP_RES(4.4010), NTC_TMP_RES(4.2800), NTC_TMP_RES(4.1620), NTC_TMP_RES(4.0480), NTC_TMP_RES(3.9380), NTC_TMP_RES(3.8310),
NTC_TMP_RES(3.7280), NTC_TMP_RES(3.6280), NTC_TMP_RES(3.5310), NTC_TMP_RES(3.4370), NTC_TMP_RES(3.3460), NTC_TMP_RES(3.2570), NTC_TMP_RES(3.1720), NTC_TMP_RES(3.0890), NTC_TMP_RES(3.0080), NTC_TMP_RES(2.9300),
NTC_TMP_RES(2.8550), NTC_TMP_RES(2.7810), NTC_TMP_RES(2.7100), NTC_TMP_RES(2.6410), NTC_TMP_RES(2.5740), NTC_TMP_RES(2.5090), NTC_TMP_RES(2.4460), NTC_TMP_RES(2.3850), NTC_TMP_RES(2.3250), NTC_TMP_RES(2.2670),
NTC_TMP_RES(2.2110), NTC_TMP_RES(2.1570), NTC_TMP_RES(2.1040), NTC_TMP_RES(2.0520), NTC_TMP_RES(2.0030), NTC_TMP_RES(1.9540), NTC_TMP_RES(1.9070), NTC_TMP_RES(1.8610), NTC_TMP_RES(1.8170), NTC_TMP_RES(1.7730),
NTC_TMP_RES(1.7310), NTC_TMP_RES(1.6850), NTC_TMP_RES(1.6100), NTC_TMP_RES(1.5850), NTC_TMP_RES(1.5100), NTC_TMP_RES(1.4560), NTC_TMP_RES(1.4160), NTC_TMP_RES(1.3580), NTC_TMP_RES(1.3600), NTC_TMP_RES(1.2550)
};

void pcb_temperature_calc(void)
{
	static u8 tmp_flt_cnt;
	s16 i,j;
	if (WakeUpNotReady)
	{
		return;
	}
	
	if (AdcAvg.TEMPERATURE < NTC_PCB_VOLT[0] || AdcAvg.TEMPERATURE > NTC_PCB_VOLT[190])
	{
		if (tmp_flt_cnt < 30)
		{
			tmp_flt_cnt++;
		}
		else
		{
			SystemFlg.TMP_FLT = 1;
			SystemParam.PCB_TEMPERATURE = PCB_TEMP(20);
		}
	}
	else
	{
		i = 0; 
		j = 128;
		for (;j;)
		{
			if (AdcAvg.TEMPERATURE >= NTC_PCB_VOLT[i+j])
			{
				i += j;
			}
			j >>= 1;
		}
		
		if (SystemFlg.TMP_FLT)
		{
			if (i >= PCB_TEMP(-30) && i <= PCB_TEMP(50))
			{
				if (tmp_flt_cnt)
				{
					tmp_flt_cnt--;
				}
				else
				{
					SystemFlg.TMP_FLT = 0;
				}
			}
		}
		else
		{
			tmp_flt_cnt = 0;
			SystemParam.PCB_TEMPERATURE = i;
		}
	}
}

const u16 NTC_OSD_VOLT[128]={
NTC_OSD_RES(300.75), NTC_OSD_RES(282.61), NTC_OSD_RES(265.66), NTC_OSD_RES(249.82), NTC_OSD_RES(235.01), NTC_OSD_RES(221.15), NTC_OSD_RES(208.18), NTC_OSD_RES(196.04), NTC_OSD_RES(184.67), NTC_OSD_RES(174.02),
NTC_OSD_RES(164.04), NTC_OSD_RES(154.68), NTC_OSD_RES(145.91), NTC_OSD_RES(137.67), NTC_OSD_RES(129.95), NTC_OSD_RES(122.69), NTC_OSD_RES(115.88), NTC_OSD_RES(109.49), NTC_OSD_RES(103.47), NTC_OSD_RES(97.824),
NTC_OSD_RES(92.824), NTC_OSD_RES(87.513), NTC_OSD_RES(82.810), NTC_OSD_RES(78.384), NTC_OSD_RES(74.217), NTC_OSD_RES(70.292), NTC_OSD_RES(66.595), NTC_OSD_RES(63.111), NTC_OSD_RES(59.826), NTC_OSD_RES(56.729),
NTC_OSD_RES(53.808), NTC_OSD_RES(51.052), NTC_OSD_RES(48.450), NTC_OSD_RES(45.995), NTC_OSD_RES(43.676), NTC_OSD_RES(41.485), NTC_OSD_RES(39.415), NTC_OSD_RES(37.459), NTC_OSD_RES(35.610), NTC_OSD_RES(33.861),
NTC_OSD_RES(32.206), NTC_OSD_RES(30.641), NTC_OSD_RES(29.159), NTC_OSD_RES(27.757), NTC_OSD_RES(26.429), NTC_OSD_RES(25.171), NTC_OSD_RES(23.979), NTC_OSD_RES(22.849), NTC_OSD_RES(21.778), NTC_OSD_RES(20.763),
NTC_OSD_RES(19.800), NTC_OSD_RES(18.886), NTC_OSD_RES(18.019), NTC_OSD_RES(17.196), NTC_OSD_RES(16.415), NTC_OSD_RES(15.673), NTC_OSD_RES(14.968), NTC_OSD_RES(14.299), NTC_OSD_RES(13.662), NTC_OSD_RES(13.057),
NTC_OSD_RES(12.482), NTC_OSD_RES(11.935), NTC_OSD_RES(11.415), NTC_OSD_RES(10.920), NTC_OSD_RES(10.449), NTC_OSD_RES(10.000), NTC_OSD_RES(9.5729), NTC_OSD_RES(9.1662), NTC_OSD_RES(8.7788), NTC_OSD_RES(8.4096),
NTC_OSD_RES(8.0578), NTC_OSD_RES(7.7224), NTC_OSD_RES(7.4026), NTC_OSD_RES(7.0977), NTC_OSD_RES(6.8068), NTC_OSD_RES(6.5293), NTC_OSD_RES(6.2645), NTC_OSD_RES(6.0117), NTC_OSD_RES(5.7704), NTC_OSD_RES(5.5400),
NTC_OSD_RES(5.3199), NTC_OSD_RES(5.1096), NTC_OSD_RES(4.9087), NTC_OSD_RES(4.7167), NTC_OSD_RES(4.5331), NTC_OSD_RES(4.3577), NTC_OSD_RES(4.1898), NTC_OSD_RES(4.0293), NTC_OSD_RES(3.8758), NTC_OSD_RES(3.7288),
NTC_OSD_RES(3.5882), NTC_OSD_RES(3.4536), NTC_OSD_RES(3.3247), NTC_OSD_RES(3.2012), NTC_OSD_RES(3.0830), NTC_OSD_RES(2.9697), NTC_OSD_RES(2.8612), NTC_OSD_RES(2.7571), NTC_OSD_RES(2.6574), NTC_OSD_RES(2.5618),
NTC_OSD_RES(2.4702), NTC_OSD_RES(2.3822), NTC_OSD_RES(2.2979), NTC_OSD_RES(2.2169), NTC_OSD_RES(2.1393), NTC_OSD_RES(2.0647), NTC_OSD_RES(1.9931), NTC_OSD_RES(1.9244), NTC_OSD_RES(1.8584), NTC_OSD_RES(1.7950),
NTC_OSD_RES(1.7341), NTC_OSD_RES(1.6756), NTC_OSD_RES(1.6194), NTC_OSD_RES(1.5653), NTC_OSD_RES(1.5133), NTC_OSD_RES(1.4633), NTC_OSD_RES(1.4153), NTC_OSD_RES(1.3690), NTC_OSD_RES(1.3246), NTC_OSD_RES(1.2818),
NTC_OSD_RES(1.2406), NTC_OSD_RES(1.2009), NTC_OSD_RES(1.1628), NTC_OSD_RES(1.1260), NTC_OSD_RES(1.0906), NTC_OSD_RES(1.0565), NTC_OSD_RES(1.0237), NTC_OSD_RES(0.9920)};

void out_temperature_calc(void)
{
	static u8 tmpr_flt_cnt;
	u8 i,j;
	if (WakeUpNotReady)
	{
		return;
	}
	
	if (AdcAvg.TEMP_OUTSIDE < NTC_OSD_VOLT[0] || AdcAvg.TEMP_OUTSIDE > NTC_OSD_VOLT[127])
	{
		if (tmpr_flt_cnt < 30)
		{
			tmpr_flt_cnt++;
		}
		else
		{
			SystemFlg.OTP_FLT 		= 1;
			SystemParam.TEMP_NOW 	= SystemParam.PCB_TEMPERATURE - 40;
		}
	}
	else
	{
		i = 0;
		j = 64;
		
		for (;j;)
		{
			if (AdcAvg.TEMP_OUTSIDE >= NTC_OSD_VOLT[i+j])
			{
				i += j;
			}
			j >>= 1;
		}
		
		if (SystemFlg.OTP_FLT)
		{
			if (i >= PCB_TEMP(-30) && i <= PCB_TEMP(70))
			{
				if (tmpr_flt_cnt)
				{
					tmpr_flt_cnt--;
				}
				else
				{
					SystemFlg.OTP_FLT = 0;
				}
			}
			SystemParam.TEMP_NOW = SystemParam.PCB_TEMPERATURE - 40;
		}
		else
		{
			tmpr_flt_cnt 					= 0;
			SystemParam.TEMP_NOW 	= i - 40;
		}
	}
	
 


SystemParam.TEMP_NOW  = SystemParam.PCB_TEMPERATURE - 40;

}
