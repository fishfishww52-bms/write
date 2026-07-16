#include "main.h"
#include "hwparam.h"
#include "init.h"
#include "flash.h"
#include "soc.h"
#include "can.h"
#include "toolfun.h"


system_param_type SystemParam;
sys_flg_type 	SystemFlg;
b_value_type	BValue;
k_value_type	KValue;
version_type 	Version;
sys_tick_type SystemTick; 

int main(void)
{  
	system_init();
	last_trip_show_process();
	for (;;)
	{ 
		if (adc_filter_process())//1ms
		{
			bus_current_calc(BValue.OPA_REF);
			bus_voltage_calc();
			avg_power_calc(); 
			
			soc_process(); 
			
			if (++SystemTick.TICK_10ms >= 10)
			{
				SystemTick.TICK_10ms = 0;
				if (++SystemTick.TICK_100ms >= 10)
				{
					SystemTick.TICK_100ms = 0;
					
					if (++SystemTick.TICK_1s >= 10)
					{
						SystemTick.TICK_1s = 0;
						His.WORK_SEC++;
						if (SystemFlg.TESTERING && His.WORK_SEC >= 3000)
						{
							SystemFlg.TESTERING = 0;
						}
					}
					else if (SystemTick.TICK_1s == 1)
					{
						soc_show_process(); 
					}
					else if (SystemTick.TICK_1s == 2)
					{
						use_record_process(); 
					}
					else if (SystemTick.TICK_1s == 3)
					{ 
						getchglft();
					}
					else if (SystemTick.TICK_1s == 4)
					{
						last_trip_process();
					}
					else if (SystemTick.TICK_1s == 5)
					{
						if (++SystemTick.SYS30STICK >= 20)
						{
							SystemTick.SYS30STICK = 0;
							last_trip_show_process();
						}
					}
				}
				else if (SystemTick.TICK_100ms == 1)
				{
					pcb_temperature_calc();
				}
				else if (SystemTick.TICK_100ms == 2)
				{
					flash_data_save_process();
				}
				else if (SystemTick.TICK_100ms == 3)
				{
					ReloadWatchdogCnt();
				}
				else if (SystemTick.TICK_100ms == 4)
				{
					his_voltage_calc();
				}
				#ifdef BP_AUTO_SWITCH
				else if (SystemTick.TICK_100ms == 5)
				{
					b_and_p_check_process();
				}
				#endif
				else if (SystemTick.TICK_100ms == 6)
				{
					data_record_process();
				}
				else if (SystemTick.TICK_100ms == 7)
				{
					soe_state_machine();
				}
				else if (SystemTick.TICK_100ms == 8)
				{
					out_temperature_calc();
				}
			}
			else if (SystemTick.TICK_10ms == 1)
			{
				uart_msg_process();
			}
			else if (SystemTick.TICK_10ms == 2)
			{
				sleep_process();
			}
			else if (SystemTick.TICK_10ms == 3)
			{
				current_offset_re_calc();
			}
			else if (SystemTick.TICK_10ms == 4)
			{
//				can_msg_process();
			}
			
			if (SystemTick.TICK_WAKEUP < WAKE_UP_MAX)
			{
				SystemTick.TICK_WAKEUP++;
			}
		}
		else
		{
			__NOP();
			__NOP();
			__NOP();
		}
	}
}


void eCCU60_IRQHandler(void)
{ 
	static tick_cnt;
	eCCU60->INTFC_b.TMR = 1; 
  oneline_data_receive(); 
	SystemTick.TICK_1ms++;
	tick_cnt++;
	if(tick_cnt>=8)
	{
		tick_cnt = 0; 
		if (SystemFlg.UART0_EN==0)
		{ 
			oneline_data_send();
		}
	}
	
	AdcResult.VOLTAGE 			= read_adc_battery_voltage();
	AdcResult.CURRENT 			= read_adc_current();
	AdcResult.TEMPERATURE 	= read_adc_boardtemperature();
	AdcResult.TEMP_OUTSIDE	= read_adc_temperature();
	AdcResult.AVSS					= read_adc_vss_volt();
	
	adc_integral_in_isr();
	uart_receive();
 
	power_off_detect();
	ADC1->CTRL0 |= ADC_SFT_STR;
}

void flt_reset_handle(void)
{
	if (SystemFlg.PWR_OFF)
	{
		return;
	}
	soc_save();
	ert_save(0,UART0,1, 0);
	NVIC_SystemReset();
}

void NMI_Handler(void)
{
	His.FLT_CNT += 13;
	flt_reset_handle();
}

void HardFault_Handler(void)
{
	His.FLT_CNT += 7;
	flt_reset_handle();
}


