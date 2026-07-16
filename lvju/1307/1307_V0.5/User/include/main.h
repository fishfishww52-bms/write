#ifndef __main_h
#define __main_h

#include "type.h"
#include "gpm32f13xx_gpio.h"
#include "gpm32f13xx_math.h"
#include "gpm32f13xx_uart.h"
#include <GPM32F13xx.h>

//#define		GP_DEBUG
//#define		SHOW_DEC_SLOW
//#define		BP_AUTO_SWITCH
#define		CSTM_SET_CAP

#define		CHIP_TYPE					4
#define		HW_VERSION				1
#define		SW_VERSION_M			0
#define		SW_VERSION_S			5

#define		HCLK_FREQ					80//MHz
#define		SYS_FREQ					62.5//us
#define		ADC_MAX_RANGE			(float)4095
#define		ADC_REF_VOLT			(float)5.03
#define		ADC_AMP						(float)(ADC_MAX_RANGE/ADC_REF_VOLT)
#define		MCU_PWR						(float)5.03
#define		ADC_VALUE(x)			(u16)(ADC_AMP*(x))

#define		read_adc_current()					ADC1->INJ0_DATA
#define		read_adc_battery_voltage()	ADC1->INJ1_DATA
#define		read_adc_boardtemperature()	ADC1->INJ2_DATA
#define		read_adc_temperature()			ADC1->INJ3_DATA
#define		read_adc_vss_volt()					ADC1->REG_DATA

#define		Pwr5VIOEnable()			GPIO2->P01 = 1
#define		Pwr5VIODisable()		GPIO2->P01 = 0



#define		UART2_BAUD_RATE			9600   //115200-485
#define   UART2_BAUD_DIV     ((u16)((float)HCLK_FREQ*62500/UART2_BAUD_RATE))
#define		UART2_CMP_CYCLE		 ((u32)(1/((float)HCLK_FREQ*62500/UART2_BAUD_RATE - UART2_BAUD_DIV))&0xF)
#define		UART2_BDRT				 ((UART2_CMP_CYCLE<<16) + UART2_BAUD_DIV)


#define		UART0_BAUD_RATE			9600    //485
#define   UART0_BAUD_DIV     ((u16)((float)HCLK_FREQ*62500/UART0_BAUD_RATE))
#define		UART0_CMP_CYCLE		 ((u32)(1/((float)HCLK_FREQ*62500/UART0_BAUD_RATE - UART0_BAUD_DIV))&0xF)
#define		UART0_BDRT				 ((UART0_CMP_CYCLE<<16) + UART0_BAUD_DIV)

#define		K_VALUE_DEFAULT		8192
#define		K_VALUE_MAX				(u16)(K_VALUE_DEFAULT*1.20)
#define		K_VALUE_MIN				(u16)(K_VALUE_DEFAULT*0.90)
#define		K_VALUE_SHIFT			13

#define		WAKE_UP_MAX				200
#define		WAKE_UP_VALID			100
#define		WakeUpNotReady		(SystemTick.TICK_WAKEUP < WAKE_UP_VALID)
#define		SLEEP_SEC					25

#define		EnableAcessToWachdogCtrl()	WDG->KEY = WDG_WDG_CTRL_EN
#define		ReloadWatchdogCnt()					WDG->KEY = WDG_RELOAD_CNT
#define		StartWatchdogReset()				WDG->KEY = WDG_W_RST

extern sys_flg_type SystemFlg;
extern b_value_type	BValue;
extern k_value_type	KValue;
extern version_type Version;
extern sys_tick_type SystemTick;
extern system_param_type SystemParam;
extern u8 adc_filter_process(void);
extern void uart_receive(void);
extern void adc_integral_in_isr(void), adc_offset_calc(void), current_offset_calc(u16 *);
extern void uart_msg_process(void), bus_current_calc(u16), bus_voltage_calc(void), pcb_temperature_calc(void);
extern void goto_sleep(void), sleep_process(void), flt_reset_handle(void), his_voltage_calc(void), out_temperature_calc(void);
extern void power_off_detect(void),current_offset_re_calc(void), one_line_data_send(void), one_line_data_receive(void);
extern void b_and_p_check_process(void), data_record_process(void), avg_power_calc(void), soe_state_machine(void);


extern void oneline_data_send(void);
extern void oneline_data_receive(void);
#endif
