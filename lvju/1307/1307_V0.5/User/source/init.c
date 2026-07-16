#include "main.h"
#include "init.h"
#include "flash.h"
#include "hwparam.h"
#include "soc.h"
#include "can.h"
#include "userfun.h"

//P0.1 CRG input (valid is high)
//P0.7 CAN S output (contrl CAN chip, need to be low when work)
//P0.8 NC
//P0.9 BAT current ADC  //UART2_TX for 485
//P0.10 BAT voltage ADC  //UART2_RX for 485
//P0.13 NTC on-board ADC
//P0.14 NTC outside ADC

//P1.9 ACC input (valid is low)
//P1.10 NC
//P1.14 SWDAT/A_LINE_TX/UART0

//P2.1 PWR CTRL output
//P2.07 CAN_RX   //current ADC
//P2.08 CAN_TX   //voltage ADC
//P2.11 SWCLK/A_LINE_RX/UART0


void gpio_config_work(void)
{
	GPIO0->PULL = 0;
	GPIO0->CFG 	= GPIO_P07_PUSHPULL;     //485
	GPIO0->ALT1 = GPIO_ALT_P0_13_ADC | GPIO_ALT_P0_09_UART |GPIO_ALT_P0_10_UART  |   GPIO_ALT_P0_14_ADC;
	
	GPIO1->PULL = 0;
	GPIO1->CFG	= GPIO_P14_PUSHPULL;
	
	
	GPIO2->ALT1 = GPIO_ALT_P2_11_UART;   //uart0
	//GPIO1->ALT1 = GPIO_ALT_P1_14_UART;    //uart0 
	
	if (SystemFlg.SLEEPING)
	{
		
	} 
	GPIO2->PULL = 0;
	GPIO2->CFG	= GPIO_P01_PUSHPULL;
	GPIO2->ALT0 = GPIO_ALT_P2_07_ADC;
	GPIO2->ALT1 = GPIO_ALT_P2_08_ADC;    //ADC1
	

	Pwr5VIOEnable();
	DisableComPullUp(); 
 

}

void gpio_config_sleep(void)
{
	GPIO0->PULL = GPIO_P01_PULL_DOWN | GPIO_P09_PULL_DOWN | GPIO_P13_PULL_DOWN | GPIO_P14_PULL_DOWN;
//	GPIO0->P07	= 0;
	GPIO0->CFG	= 0;
	GPIO0->ALT1 = 0;
	
	GPIO1->PULL = GPIO_P09_PULL_UP | GPIO_P14_PULL_DOWN;
	GPIO1->CFG	= 0;
	GPIO1->ALT1 = 0;
	
	GPIO2->CFG	= GPIO_P01_PUSHPULL;
	GPIO2->PULL = GPIO_P11_PULL_DOWN | GPIO_P07_PULL_DOWN | GPIO_P08_PULL_DOWN;
	GPIO2->ALT0 = 0;
}
void dma_config(void)
{
	DMA_Ch0->CTRL		= DMA_Ch_REQ_2 | DMA_Ch_SRC_SIZE_8BIT|DMA_Ch_DST_SIZE_8BIT|DMA_Ch_SRC_INC_EN|DMA_Ch_CIRC_EN;
	DMA_Ch0->SRCADR = (uint32_t)Uart0TxArray;
	DMA_Ch0->DSTADR = (uint32_t)&(UART0->TX_DATA);
		
	DMA_Ch6->CTRL		= DMA_Ch_REQ_1 | DMA_Ch_SRC_SIZE_8BIT|DMA_Ch_DST_SIZE_8BIT|DMA_Ch_SRC_INC_EN|DMA_Ch_CIRC_EN;
	DMA_Ch6->SRCADR = (uint32_t)Uart2TxArray;
	DMA_Ch6->DSTADR = (uint32_t)&(UART2->TX_DATA);
		
}
// adc clock 80/3 = 26.6MHz
// a channel cost = (22 + 13)/26.6 = 1.3us
void adc_config(void)
{
	u8 i;
	ADC1->SMP0 	= ADC_CH7_SMP_22T | ADC_CH4_SMP_22T | ADC_CH5_SMP_22T; 
	ADC1->SMP1	= ADC_CH8_SMP_22T | ADC_CH15_SMP_22T;
	ADC1->CTRL0 = ADC_CLK_DIV3 | ADC_DATA_ALIGN_R | ADC_EN;
	for (i = 0; i < 25; i++)
	{
		__NOP();
	}
	ADC1->CTRL1 	= ADC_INJ0_EN | ADC_INJ1_EN | ADC_INJ2_EN | ADC_INJ3_EN|
									ADC1_INJ0_eCCU60_TRG0 | ADC1_INJ1_eCCU60_TRG0 |
									ADC1_INJ2_eCCU60_TRG0 | ADC1_INJ3_eCCU60_TRG0 | ADC_REG_EN;
	ADC1->INJ_SEQ = ADC_INJ_SEQ0_CH7 | ADC_INJ_SEQ1_CH8 |ADC_INJ_SEQ2_CH4 | ADC_INJ_SEQ3_CH5;
	ADC1->REG_SEQ0= ADC_REG_SEQ0_CH15;
}

void ecuu60_config(void)
{
	eCCU60->TMR_PLOADS 	= SYS_FREQ*HCLK_FREQ - 1;
	eCCU60->TMR_CTRL	 	= eCCU6_TMR_UP_COUNT|eCCU6_TMR_CK_HCLK;
	eCCU60->PWM_CTRL0		= eCCU6_PWM_ADC_TRG_EN|eCCU6_PWM_ADC_TRG_UP_COUNT;
	eCCU60->INTE 				=	eCCU6_INTE_TMR;
	eCCU60->ADC_TRG01S	= (SYS_FREQ - 10)*HCLK_FREQ - 1;
	eCCU60->TMR_CTRL 		|= eCCU6_TMR_STR;
	eCCU60->INTFC_b.TMR	= 1;
}

void uart_config(void)
{
	UART0->BAUD_RATE 	= UART0_BDRT;
	UART0->CTRL 			= UART_TX_EN |UART_RX_EN | UART_EN;
	UART0->INTFC_b.TX = 1;		
	
	UART2->BAUD_RATE 	= UART2_BDRT;
	UART2->CTRL 			= UART_RX_EN | UART_TX_EN | UART_EN;
	UART2->INTFC_b.TX = 1;
	Rs485Rx_En();
	
	
}




void key_peripheral_config_work(void)
{
	adc_config();
	
	ecuu60_config();
	
	uart_config();
}

void isr_config(void)
{
	NVIC_SetPriority(eCCU60_IRQn, 2);
	NVIC_SetPriority(WDG_IRQn,1);
	
	NVIC_EnableIRQ(eCCU60_IRQn);
	NVIC_EnableIRQ(WDG_IRQn);
}

void watchdog_config(void)
{
	WDG->KEY 	= WDG_WDG_CTRL_EN;
	WDG->CTRL =	WDG_PRESCALE_256;
	WDG->INTE =	WDG_INTE_WDG;
	SET_BIT(WDG->CTRL, (3750 <<16));//3750*256/32000 = 30second
	WDG->KEY  = WDG_WO_RST;
	NVIC_ClearPendingIRQ(WDG_IRQn);
	NVIC_EnableIRQ(WDG_IRQn);
}

void system_init(void)
{
	u16 delay_cnt, last_volt;
	s16 delta_volt;
	
	__disable_irq();
	
	SMU->SYSLOCK = SMU_SYSUNLOCK_KEY_1;
	SMU->SYSLOCK = SMU_SYSUNLOCK_KEY_2;
	
	CLK->AHBCLKE = CLK_AHB_CLKEN_DMA | CLK_AHB_CLKEN_eCCU60 | CLK_AHB_CLKEN_MATH;
	CLK->APBCLKE0 = CLK_APB0_CLKEN_ADC1 | CLK_APB0_CLKEN_UART0 | CLK_APB0_CLKEN_UART2;
//	CLK->APBCLKE1 = CLK_APB1_CLKEN_CANFD80;
	
	gpio_config_work();
	
	dma_config();
	
	key_peripheral_config_work();
	
//	can_config();
	
	isr_config();
	
	flash_read();
	
	SMU->SYSLOCK = SMU_LOCK_KEY;
	
	__enable_irq();
	
	adc_offset_calc();
	
	for (delay_cnt = 0; delay_cnt < 1500; delay_cnt++)
	{
		while(SystemTick.TICK_ADC < 16);
		adc_filter_process();
		
		if (++SystemTick.TICK_100ms >= 100)
		{
			SystemTick.TICK_100ms = 0;
			delta_volt = last_volt - AdcAvg.VOLTAGE;
			if (abs(delta_volt) > BATTERY_VOLTAGE(0.25))
			{
				delay_cnt -= 90;
				last_volt = AdcAvg.VOLTAGE;
			}
		}
	}
	
	while(SystemTick.TICK_ADC < 16);
	adc_filter_process();
	watchdog_config();
	ReloadWatchdogCnt();
	
	#ifndef BP_AUTO_SWITCH
	SystemFlg.BP_VALID = 1;
	#endif
	
	SystemFlg.UART0_EN = 0;
	#ifndef GP_DEBUG
	if (AdcAvg.VOLTAGE > BATTERY_VOLTAGE(0) && AdcAvg.VOLTAGE < BATTERY_VOLTAGE(150))  //10
	{
		GPIO1->ALT1 = GPIO_ALT_P1_14_UART; 
		
		GPIO2->ALT1 = GPIO_ALT_P2_11_UART;
		SystemFlg.UART0_EN = 1;
		
		UART0->BAUD_RATE 	= UART0_BDRT;
		UART0->CTRL 			= UART_TX_EN |UART_RX_EN | UART_EN;
		UART0->INTFC_b.TX = 1;		
	}
	#endif
	
	soc_key_paramter_init();
	SystemTick.TICK_WAKEUP = WAKE_UP_MAX;
	bus_voltage_calc();
	bus_current_calc(BValue.OPA_REF);
	pcb_temperature_calc();
	out_temperature_calc();
	SystemParam.TEMPERATURE = SystemParam.PCB_TEMPERATURE - 40;
	///SystemFlg.E_MOTOR = E_MotorIOFlg?1:0;
	soc_init();
	His.LAST_VOLT	= AdcAvg.VOLTAGE;
	 
}

void itu_config(void)
{
	ITU->EXT_SEL0 	= ITU_EXT1_SEL_P0_01|ITU_EXT7_SEL_P0_07;
	ITU->EXT_SEL1 	= ITU_EXT9_SEL_P0_09|ITU_EXT9_SEL_P1_09|ITU_EXT11_SEL_P2_11;
	//ITU->EXT_LEVEL	= ITU_EXT1_LEVEL_HIGH|ITU_EXT9_LEVEL_LOW|ITU_EXT11_LEVEL_HIGH;
	ITU->EXT_EDGE		= ITU_EXT1_EDGE_BOTH|ITU_EXT7_EDGE_BOTH|ITU_EXT9_EDGE_BOTH|ITU_EXT11_EDGE_BOTH;
	ITU->EXT_DEB		= ITU_EXT1_DEB_EN|ITU_EXT7_DEB_EN|ITU_EXT9_DEB_EN|ITU_EXT11_DEB_EN;
	ITU->EXT_EVTE		= ITU_EXT1_EVT_EN|ITU_EXT11_EVT_EN;   //ITU_EXT7_EVT_EN|ITU_EXT9_EVT_EN|
	ITU->EXT_INTFC 	= 0xFFFFFFFF;
}

void itu_clear(void)
{
	ITU->EXT_INTFC 	= 0xFFFFFFFF;
	ITU->EXT_EDGE		= 0;
	ITU->EXT_LEVEL	= 0;
	ITU->EXT_DEB		= 0;
	ITU->EXT_EVTE		= 0;
}

void goto_sleep(void)
{
	Pwr5VIODisable();
	DisableComPullUp();
	SystemTick.TICK_1ms = 0;
	while(SystemTick.TICK_1ms < 200);
	
	__disable_irq();
	ReloadWatchdogCnt();
	
	SMU->SYSLOCK = SMU_SYSUNLOCK_KEY_1;
	SMU->SYSLOCK = SMU_SYSUNLOCK_KEY_2;
	gpio_config_sleep();
	itu_config();
	
	ADC1->CTRL0 			= 0;
	eCCU60->TMR_CTRL 	= 0;
	UART2->CTRL				= 0;
	UART0->CTRL				= 0;
	
	NVIC_DisableIRQ(eCCU60_IRQn);
	NVIC_ClearPendingIRQ(eCCU60_IRQn);
	
	CLK->AHBCLKE 		= 0;
	CLK->APBCLKE0 	= 0;
	CLK->APBCLKE1		= 0;
	
	MODIFY_REG(CLK->AHBCLKSEL0, CLK_AHBCLKSEL0_HCLK_Msk, CLK_HCLK_HSIRC);
	SMU->SYSLOCK = SMU_LOCK_KEY;
	SystemFlg.SLEEPING = 1;
	
	__enable_irq();
	__SEV();
	__WFE();
	__WFE();
	__disable_irq();
	
	SMU->SYSLOCK = SMU_SYSUNLOCK_KEY_1;
	SMU->SYSLOCK = SMU_SYSUNLOCK_KEY_2;
	
	if (CLK_HCLK_PLL != (CLK->AHBCLKSEL0&CLK_AHBCLKSEL0_HCLK_Msk))
	{
		ACU->PLL_CTRL_b.PLL_EN = 0;
		ACU->PLL_CTRL_b.PLL_EN = 1;
		
		while(READ_BIT(CLK->STS, CLK_STS_PLLRDY));
		while(!(READ_BIT(CLK->STS, CLK_STS_PLLRDY)));
		MODIFY_REG(CLK->AHBCLKSEL0, CLK_AHBCLKSEL0_HCLK_Msk, CLK_HCLK_PLL);
	}
		
	CLK->AHBCLKE = CLK_AHB_CLKEN_DMA | CLK_AHB_CLKEN_eCCU60 | CLK_AHB_CLKEN_MATH;
	CLK->APBCLKE0 = CLK_APB0_CLKEN_ADC1 | CLK_APB0_CLKEN_UART0 | CLK_APB0_CLKEN_UART2;
	//CLK->APBCLKE1 = CLK_APB1_CLKEN_CANFD80;
	
	gpio_config_work();
	
	if (ITU->EXT_INTF)
	{
		His.WORK_SEC += 15;
		ReloadWatchdogCnt();
		NVIC_ClearPendingIRQ(WDG_IRQn);
		SystemTick.SLEEP_CNT 	= 0;
		SystemFlg.SLEEPING 		= 0;
	}
	else
	{
		His.WORK_SEC += SLEEP_SEC;
		SystemTick.SLEEP_CNT = 5980;//left 200ms for sleep work
	}
	key_peripheral_config_work();
	NVIC_EnableIRQ(eCCU60_IRQn);
	SMU->SYSLOCK 		= SMU_LOCK_KEY;
	SystemTick.TICK_WAKEUP = 0;
	itu_clear();
	__enable_irq();
}



void delay_ms(int delaytime)
{
	int i;
	while(delaytime--)
	{
		for(i=0;i<5000;i++) ;
	}
}
void goto_sleep1(void)
{
	Pwr5VIODisable();
	DisableComPullUp();
	ReloadWatchdogCnt();
//	SystemTick.TICK_1ms = 0;
//	while(SystemTick.TICK_1ms < 200);
	
	
	__disable_irq();
	ReloadWatchdogCnt();
	
	SMU->SYSLOCK = SMU_SYSUNLOCK_KEY_1;
	SMU->SYSLOCK = SMU_SYSUNLOCK_KEY_2;
	gpio_config_sleep();
	itu_config();
	delay_ms(2000);
	
	
	ADC1->CTRL0 			= 0;
	eCCU60->TMR_CTRL 	= 0;
	UART2->CTRL				= 0;
	UART0->CTRL				= 0;
	
	NVIC_DisableIRQ(eCCU60_IRQn);
	NVIC_ClearPendingIRQ(eCCU60_IRQn);
	
	CLK->AHBCLKE 		= 0;
	CLK->APBCLKE0 	= 0;
	CLK->APBCLKE1		= 0;
	
	MODIFY_REG(CLK->AHBCLKSEL0, CLK_AHBCLKSEL0_HCLK_Msk, CLK_HCLK_HSIRC);
	SMU->SYSLOCK = SMU_LOCK_KEY;
	SystemFlg.SLEEPING = 1;
	
	__enable_irq();
	__SEV();
	__WFE();
	__WFE();
	__disable_irq();
	
	SMU->SYSLOCK = SMU_SYSUNLOCK_KEY_1;
	SMU->SYSLOCK = SMU_SYSUNLOCK_KEY_2;
	
	if (CLK_HCLK_PLL != (CLK->AHBCLKSEL0&CLK_AHBCLKSEL0_HCLK_Msk))
	{
		ACU->PLL_CTRL_b.PLL_EN = 0;
		ACU->PLL_CTRL_b.PLL_EN = 1;
		
		while(READ_BIT(CLK->STS, CLK_STS_PLLRDY));
		while(!(READ_BIT(CLK->STS, CLK_STS_PLLRDY)));
		MODIFY_REG(CLK->AHBCLKSEL0, CLK_AHBCLKSEL0_HCLK_Msk, CLK_HCLK_PLL);
	}
		
	CLK->AHBCLKE = CLK_AHB_CLKEN_DMA | CLK_AHB_CLKEN_eCCU60 | CLK_AHB_CLKEN_MATH;
	CLK->APBCLKE0 = CLK_APB0_CLKEN_ADC1 | CLK_APB0_CLKEN_UART0 | CLK_APB0_CLKEN_UART2;
	//CLK->APBCLKE1 = CLK_APB1_CLKEN_CANFD80;
	
	gpio_config_work();
	
	if (ITU->EXT_INTF)
	{
		His.WORK_SEC += 15;
		ReloadWatchdogCnt();
		NVIC_ClearPendingIRQ(WDG_IRQn);
		SystemTick.SLEEP_CNT 	= 0;
		SystemFlg.SLEEPING 		= 0;
	}
	else
	{
		His.WORK_SEC += SLEEP_SEC;
		SystemTick.SLEEP_CNT = 5980;//left 200ms for sleep work
	}
	key_peripheral_config_work();
	NVIC_EnableIRQ(eCCU60_IRQn);
	SMU->SYSLOCK 		= SMU_LOCK_KEY;
	SystemTick.TICK_WAKEUP = 0;
	itu_clear();
	__enable_irq();
}





void WDG_IRQHandler(void)
{
	EnableAcessToWachdogCtrl();
	CLEAR_FLAG(WDG->INTFC, WDG_INTFC_TMOUT);
	if (SystemFlg.SLEEPING == 0)
	{
		StartWatchdogReset();
		His.FLT_CNT += 3;
		flt_reset_handle();
	}
	ReloadWatchdogCnt();
}
