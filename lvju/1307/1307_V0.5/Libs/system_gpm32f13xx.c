//*** <<< Use Configuration Wizard in Context Menu >>> ***	

//----------------------------------------------------------//
//	CMSIS Header 																						//
//----------------------------------------------------------//


//----------------------------------------------------------//
//	Device Header 																					//
//----------------------------------------------------------//
#include <GPM32F13xx.h>


//----------------------------------------------------------//
//	Device Driver																						//
//----------------------------------------------------------//
#include "gpm32f13xx_acu.h"
#include "gpm32f13xx_clk.h"
#include "gpm32f13xx_gpio.h"
#include "gpm32f13xx_nvm.h"
#include "gpm32f13xx_smu.h"


//----------------------------------------------------------//
//	Hardware Abstract Layer																	//
//----------------------------------------------------------//


//----------------------------------------------------------//
//	Applications 																						//
//----------------------------------------------------------//
#include "system_gpm32f13xx.h"


//----------------------------------------------------------//
//	Global Parameters																				//
//----------------------------------------------------------//

/*  Define clocks	*/
/*	Internal sscillator frequency	*/
#define HSIRC_HZ						8000000U
#define LSIRC_HZ						32768U


/*	Default CPU clock	*/
#define DEFAULT_CPU_CLOCK				HSIRC_HZ


/*	 System Core Clock Variable	*/
/* ToDo: initialize SystemCoreClock with the system core clock frequency value
         achieved after system intitialization.
         This means system core clock frequency after call to SystemInit() */
				 
uint32_t SystemCoreClock = DEFAULT_CPU_CLOCK;  /* System Clock Frequency (Core Clock)*/

//----------------------------------------------------------//
//	Local Parameters																				//
//----------------------------------------------------------//


//----------------------------------------------------------//
//	Extern Application																			//
//----------------------------------------------------------//


//----------------------------------------------------------//
//	Extern Parameters																				//
//----------------------------------------------------------//


// <h> User Option 0 Control Register (USER_OPT0) Setting
//	<i> User Option0 Control Register Setup
//	<o.16> P2_10 pad mode
//		<0=> GPIO function
//		<1=> Reset function
//
//	<o.12..13> Boot Mode Selection (USER_BOOT) Setting
//		<0=> Boot from SRAM Memory   
//		<2=> Boot from System Memory   
//		<3=> Boot from Program Memory   
//
//	<o.11> ADC Hold Capacitor Discharge Prior Sample Operation (ADC_CHDIS_ENB) Setting
//		<0=> Enable   
//		<1=> Disable   
//
//	<o.8> User Data Mmemory (USER_DM) Setting
//		<0=> Enable   
//		<1=> Disable   
//
	#define USERCONFIG_0_DEFINED 0xFFFEF7FF
//
	const uint32_t config0 __attribute__((at(0x0F000000))) =USERCONFIG_0_DEFINED;
// </h>


// <h> Data Memory Start Address Register (USER_OPT2) Setting
//  <i> Data Memory Start Address Register Setup
//		<o.0..31> User Data Memory Start Address <0x01000000-0x0101DFFF:0x200>
//			<i> Data Memory range is from 0x01000000 to 0x0101DE00 and word-aligned
	#define USER_DATA_MEMORY_ADDRESS_START_DEFINED	0x0100FE00
//	
	#if ((USER_DATA_MEMORY_ADDRESS_START_DEFINED <=01000000) || (USER_DATA_MEMORY_ADDRESS_START_DEFINED >0x0101DE00))
//	
		#error "Data Memory Address setup error : wrong address"
//	
	#endif
//	
	const uint32_t config2 __attribute__((at(0x0F000008))) =USER_DATA_MEMORY_ADDRESS_START_DEFINED;
// </h>


// <h> Security Level Register (USER_OPT3) Setting
//  <i> Security Level Register Setup
//		<o.0..31> Security Level Register
//	 	<0xFFFFFFFF=> Non-Secure Mode   
//	 	<0x00000001=> Read Protect Mode   
//    <0xFFFFB2A5=> Interface Off Mode   
//    <0x4D69B2A5=> Chip Lock Mode   
	#define SECURITY_LVEL_DEFINED	0xFFFFFFFF
//
	const uint32_t config3 __attribute__((at(0x0F00000C))) =SECURITY_LVEL_DEFINED;
// </h>



void ParametersSetup(void)
{	
// <h> Analog Control Unit (ACU) Clock Source Setting
//
//		<e> Low Voltage Detect Control (ACU_LVRD_CTRL)
		#define LVD_FUNCTION_DEFINED 0
//
//		<o> LVD Debounce Time (LVD_DBT)
//			<0=> No-debounce time
//			<1=> 375ns
//			<2=> 750ns
//			<3=> 1us
		#define LVD_DEBOUNCE_TIME_DEFINED 3
//
//		<o> LVD Detect Level Selection (LVD_SEL)
//			<0=> 2.4 V
// 			<1=> 2.9 V
//			<2=> 3.4 V
//			<3=> 4.4 V
		#define LVD_SELECT_DEFINED	0
//
//		<o> Low Voltage Detect Interrupt
//			<0=> Disable 
//			<1=> Enable 
		#define LVD_INT_DEFINED 0
//	</e>


//	<e> Low Voltage Reset Control (ACU_LVRD_CTRL)
		#define LVR_FUNCTION_DEFINED 1
//
//		<o> Low Voltage Reset Detect Level Selection
//			<0=> 2.2 V
//			<1=> 2.7 V
//			<2=> 3.2 V
//			<3=> 4.2 V
			#define LVR_SELECTION_DEFINED 2
//	</e>


//	<e> Crystal Oscillator Control(ACU_XTL_CTRL)
		#define XTL_FUNCTION_DEFINED 0
//
//			<o> External (XTL) Oscillator Frequency
//			<0=> 4MHz
//			<1=> 8MHz
			#define XTL_FREQ_SELECTION_DEFINED 0
//	</e>


//	<e> PLL Clock Control (ACU_PLL_CTRL)
		#define PLL_FUNCTION_DEFINED	1
//
//		<o> PLL input clock Divide
//			<0=> Divide 1
//			<1=> Divide 2
			#define PLL_IN_DIV_DEFINED	1		
//
//		<o> PLL Multipy Value (From 11 to 24)
			#define PLL_M_DEFINED	20 
//
//		<o> PLL Input Clock Selection
//			<0=> HSIRC
//			<1=> HSXTL
			#define PLL_CLOCK_SELECTION_DEFINED 0			
//
// 	</e>
// </h>


// <h> IOSC Control Setting
//
//	<h> AHB Peripheral Clock Selection 0
//
//		<e> Clock Output Function
			#define CLOCK_OUT_FUNCTION_DEFINED 0
//
//			<o> Clock Output Source Selection
//				<0=> HCLK
//				<1=> HSIRC
//				<2=> System Clock
//				<3=> PLL Clock
//				<4=> HSXTL
//				<5=> LSIRC
			#define CKOSEL_SELECTION_DEFINED 0
//
//		<o> Clock Output Divider (From 1 to 8)
			#define CKODIV_DEFINED 8
//		</e>


//		<o> HCLK Source Selection
//			<0=> HSIRC
//			<1=> PLL Clock
//			<2=> HSXTL
			#define HCLK_SELECTION_DEFINED 1
//
//		<o>	HCLK Divider (From 1 to 16)
			#define HCLK_DIVIDER_DEFINED 1
//	</h>
// </h>


// <h> SysTick
//
//		<e> SysTick Function
			#define SYSTICK_FUNCTION_DEFINED 0
//
//			<o> SysTick Clock Source Selection
//			<0=> External: HCLK/2
//			<1=> External: HSIRC/2
//			<2=> External: LSIRC
//			<3=> External: HSXTL/2
//			<4=> Internal: HCLK
			#define  SYSTICK_CLK_SELECTION_DEFINED 0
//
//		<o> SysTick Clock Divider (From 1 to 256)
			#define SYSTICK_DIVIDER_DEFINED 2
//
//      <o> SysTick Interrupt
//      <0=> Disable
//      <1=> Enable
			#define  SYSTICK_INTE_DEFINED 1
//
//      <o> SysTick Time (us) <1-1000000000>
//      <i> Define the timer counter value
			#define  SYSTICK_TIME_COUNT_DEFINED 1000
//		</e>
// </h>
}

/*********************************************************/

void SystemInit(void)
{
	SystemCoreClock = DEFAULT_CPU_CLOCK;

	#if (SYSTICK_FUNCTION_DEFINED ==1)
		uint32_t systickclk =0;
	#endif
	
	//	NVM Key Unlock	//
	NVM->FSHKEY =NVM_FSHKEY_1;
	NVM->FSHKEY =NVM_FSHKEY_2;
	
	//	System Key Unlock	//
	SMU->SYSLOCK =SMU_SYSUNLOCK_KEY_1;
	SMU->SYSLOCK =SMU_SYSUNLOCK_KEY_2;
	
	//	Select boot source according to User Config 0.	//
	#if ((USERCONFIG_0_DEFINED &0x00003000) ==0x00000000)
		NVM->CTRL =NVM_BOOT_SRAM;
	
	#elif ((USERCONFIG_0_DEFINED &0x00003000) ==0x00002000)
		NVM->CTRL =NVM_BOOT_SYSTEM;
	
	#elif ((USERCONFIG_0_DEFINED &0x00003000) ==0x00003000)
		NVM->CTRL =NVM_BOOT_PROGRAM;
		
	#endif
	
	//  Step 1. Choose Internal or External Clock Source
	//	Initial clock source	//
	//	Wait until HSIRC_8M clock is ready.
	#if (XTL_FUNCTION_DEFINED ==0)
		while(!(CLK->STS_b.HSIRCRDY)){};	
	
	//	Initial external Oscillator	//
	#else
		//	Change GPIO P1_09 and P1_10 to external oscillator input and output.	
		GPIO1->ALT1_b.P09 =GPIO_b_XTL;
		GPIO1->ALT1_b.P10 =GPIO_b_XTL;
		
		#if (XTL_FREQ_SELECTION_DEFINED ==0)
			#define XTL_FREQ_DEFINED 4000000

		#else
			#define XTL_FREQ_DEFINED 8000000
	
		#endif
		
		//	Enable xtl
		ACU->XTL_CTRL_b.HSXTL_EN =b_ENABLE;
	
		//	Wait until xtl clock is ready.
		while(!(CLK->STS_b.HSXTLRDY)){};
			
	#endif			
	
	
	//	Setp 2. Determine whether PLL function is enable or disable	//
	#if (PLL_FUNCTION_DEFINED ==1)
		//	Select PLL clock source.
		//	Choose PLL clock from HSIRC_8M.
		#if (PLL_CLOCK_SELECTION_DEFINED ==0)
			
			#if (PLL_IN_DIV_DEFINED ==0)
				#error "Please Select PLL input clock divide 2"
			
			#else			
				ACU->PLL_CTRL =  ACU_PLLCKI_HSIRC | ACU_PLLIN_DIV2;
				#define PLL_CLOCK_SOURCE (HSIRC_HZ /2 /1000000)
				
			#endif			
			
		//	Choose PLL clock from xtl clock.
		#else			
			#if (XTL_FUNCTION_DEFINED ==0)
				#error "Please enable Crystal Oscillator"
	
			#elif (PLL_IN_DIV_DEFINED ==0)
				#if (XTL_FREQ_SELECTION_DEFINED ==0)
					ACU->PLL_CTRL = ACU_PLLCKI_HSXTL;
					#define PLL_CLOCK_SOURCE 	(XTL_FREQ_DEFINED /1000000)
					
				#else
					#error "Please Select PLL input clock divide 2"	
					
				#endif
				
			#else
				ACU->PLL_CTRL = ACU_PLLCKI_HSXTL | ACU_PLLIN_DIV2;
				#define PLL_CLOCK_SOURCE 	(XTL_FREQ_DEFINED /2 /1000000)
				
			#endif

		#endif			
	
		//	Check PLL clock configuration is valid or invalid.
		#if (((PLL_M_DEFINED -1) <10) || ((PLL_M_DEFINED -1) >23))
			#error "PLL Multipy Value setup error"
	
		#endif
	
		//  Check PLL Clock range
		#if ((PLL_M_DEFINED*PLL_CLOCK_SOURCE) <44)
			#error "PLL Clock is under valid frequency, please change PLL Multipyvalue"
	
		#elif ((PLL_M_DEFINED*PLL_CLOCK_SOURCE) >96)
			#error "PLL Clock excesses the maximun CPU frequency, please change PLL Multipy value"
	
		#endif
	
		//	Set PLL M and enable PLL function.
		ACU->PLL_CTRL_b.PLL_M =(PLL_M_DEFINED -1);
		
		ACU->PLL_CTRL_b.PLL_EN =b_ENABLE;
		while(!(CLK->STS_b.PLLRDY));
		
	#endif
	
	
	// Step 3.	Configurate Clock_out, SysTick, and HCLK clock divider	//
	#if ((HCLK_DIVIDER_DEFINED <1) || (HCLK_DIVIDER_DEFINED >16))
		#error "HCLK Clock Divier value is illegal, please use the value between 1 to 16"
	
	#endif
	
	//	Clear clock divider contents and setup corresponding value	//
	CLK->DIV =0;

	CLK->DIV_b.HCLK =(HCLK_DIVIDER_DEFINED -1);
	
	//	Step 4. Select HCLK clock source	//
	#if (HCLK_SELECTION_DEFINED ==0)
	
		SET_BIT(NVM->CTRL, NVM_LATENCY_0T);
	
		SET_BIT(CLK->AHBCLKSEL0, CLK_HCLK_HSIRC);
		
		SystemCoreClock = HSIRC_HZ;
	
	#elif (HCLK_SELECTION_DEFINED ==1)

	  #if (PLL_FUNCTION_DEFINED ==0)
		  #error "PLL is disable, please enable PLL function"
	
		#else
			//	Set NVM wait cycle.
			#if (((PLL_M_DEFINED*PLL_CLOCK_SOURCE) / HCLK_DIVIDER_DEFINED ) <=24)
				SET_BIT(NVM->CTRL, NVM_LATENCY_0T);
	
			#elif ((((PLL_M_DEFINED*PLL_CLOCK_SOURCE) / HCLK_DIVIDER_DEFINED ) >24) && (((PLL_M_DEFINED*PLL_CLOCK_SOURCE) /HCLK_DIVIDER_DEFINED ) <=40))
				SET_BIT(NVM->CTRL, NVM_LATENCY_1T);
	
			#elif ((((PLL_M_DEFINED*PLL_CLOCK_SOURCE) / HCLK_DIVIDER_DEFINED ) >40) && (((PLL_M_DEFINED*PLL_CLOCK_SOURCE) / HCLK_DIVIDER_DEFINED) <=70))
				SET_BIT(NVM->CTRL, NVM_LATENCY_2T);
	
			#elif ((((PLL_M_DEFINED*PLL_CLOCK_SOURCE) / HCLK_DIVIDER_DEFINED ) >70) && (((PLL_M_DEFINED*PLL_CLOCK_SOURCE) / HCLK_DIVIDER_DEFINED ) <=96))
				SET_BIT(NVM->CTRL, NVM_LATENCY_3T);
				
			#elif (((PLL_M_DEFINED*PLL_CLOCK_SOURCE) / HCLK_DIVIDER_DEFINED ) >96)
				#error "HCLK excesses the maximun CPU frequency, please change PLL Multipy and Divide Value"
	
			#endif

			SET_BIT(CLK->AHBCLKSEL0, CLK_HCLK_PLL);
			
			SystemCoreClock = ((((PLL_M_DEFINED)*(PLL_CLOCK_SOURCE) *1000000)) / HCLK_DIVIDER_DEFINED);
	
		#endif
		
	#elif (HCLK_SELECTION_DEFINED ==2)
	  #if (XTL_FUNCTION_DEFINED ==0)
		  #error "Please enable Crystal Oscillator"
		#else
			
			SET_BIT(CLK->AHBCLKSEL0, CLK_HCLK_HSXTL);
			
			SystemCoreClock = XTL_FREQ_DEFINED;
		
		#endif
	
	#endif
	
	//	Configurate LVD and LVR function	//
	ACU->LVRD_CTRL =0x00000000;
	
	//	LVD configuration	//
	#if (LVD_FUNCTION_DEFINED ==1)
		#if (LVD_DEBOUNCE_TIME_DEFINED ==0)
			SET_BIT(ACU->LVRD_CTRL, ACU_DBT_0ns);
	
		#elif (LVD_DEBOUNCE_TIME_DEFINED ==1)
			SET_BIT(ACU->LVRD_CTRL, ACU_DBT_375ns);
	
		#elif (LVD_DEBOUNCE_TIME_DEFINED ==2)
			SET_BIT(ACU->LVRD_CTRL, ACU_DBT_750ns);
	
		#elif (LVD_DEBOUNCE_TIME_DEFINED ==3)
			SET_BIT(ACU->LVRD_CTRL, ACU_DBT_1us);
			
		#endif
	
		#if (LVD_SELECT_DEFINED ==0)
			SET_BIT(ACU->LVRD_CTRL, ACU_LVD_2V4);
			
		#elif (LVD_SELECT_DEFINED ==1)
			SET_BIT(ACU->LVRD_CTRL, ACU_LVD_2V9);
	
		#elif (LVD_SELECT_DEFINED ==2)
			SET_BIT(ACU->LVRD_CTRL, ACU_LVD_3V4);
	
		#elif (LVD_SELECT_DEFINED ==3)
			SET_BIT(ACU->LVRD_CTRL, ACU_LVD_4V4);
	
		#endif
	
		#if (LVD_INT_DEFINED ==0)
			MODIFY_REG(ACU->INTE, ACU_INTE_LVD_Msk, b_DISABLE);
			
		#else
			SET_BIT(ACU->INTE, ACU_INTE_LVD);
			
		#endif
			SET_BIT(ACU->LVRD_CTRL, ACU_LVD_EN);
			
	#endif
	
	//	LVR configuration	//
	#if (LVR_FUNCTION_DEFINED ==1)
		#if (LVR_SELECTION_DEFINED ==0)
			SET_BIT(ACU->LVRD_CTRL,  (ACU_LVR_2V2 | ACU_LVR_EN));
			
		#elif (LVR_SELECTION_DEFINED ==1)
			SET_BIT(ACU->LVRD_CTRL,  (ACU_LVR_2V7 | ACU_LVR_EN));
	
		#elif (LVR_SELECTION_DEFINED ==2)
			SET_BIT(ACU->LVRD_CTRL,  (ACU_LVR_3V2 | ACU_LVR_EN));
	
		#elif (LVR_SELECTION_DEFINED ==3)
			SET_BIT(ACU->LVRD_CTRL,  (ACU_LVR_4V2 | ACU_LVR_EN));
	
		#endif
	
	#endif

	//	Configurate AHB clock	//
	#if (CLOCK_OUT_FUNCTION_DEFINED ==1)
	
		SET_BIT(CLK->APBCLKE0, CLK_APB0_CLKEN_CKO);
		
		GPIO2->ALT0_b.P01 = GPIO_b_CKO;
		
		#if (CKOSEL_SELECTION_DEFINED ==0)
		 SET_BIT(CLK->AHBCLKSEL0, CLK_CKO_HCLK);
			
		#elif (CKOSEL_SELECTION_DEFINED ==1)
			SET_BIT(CLK->AHBCLKSEL0, CLK_CKO_HSIRC);
	
		#elif (CKOSEL_SELECTION_DEFINED ==2)
			SET_BIT(CLK->AHBCLKSEL0, CLK_CKO_SYSCLK);
	
		#elif (CKOSEL_SELECTION_DEFINED ==3)
			SET_BIT(CLK->AHBCLKSEL0, CLK_CKO_PLL);
			
		#elif (CKOSEL_SELECTION_DEFINED ==4)
			SET_BIT(CLK->AHBCLKSEL0, CLK_CKO_HSXTL);

		#elif (CKOSEL_SELECTION_DEFINED ==5)
			SET_BIT(CLK->AHBCLKSEL0, CLK_CKO_LSIRC);
	
		#endif
		
		#if ((CKODIV_DEFINED <1) || (CKODIV_DEFINED >8))
			#error "Clock Out Divider value is illegal, please use the value between 1 to 8"
	
		#endif
	
		CLK->DIV_b.CKO =(CKODIV_DEFINED -1);
	
	#endif
	
	//SystemCoreClockUpdate();
	
	//	Enable SysTick function	//
	#if (SYSTICK_FUNCTION_DEFINED ==1)
	
		#if (SYSTICK_CLK_SELECTION_DEFINED ==4)
			//	Select SysTick clock source from System Clock (CPU)
			systickclk = (SystemCoreClock/1000000);
	
			SysTick->LOAD = (uint32_t)( (systickclk * SYSTICK_TIME_COUNT_DEFINED) - 1UL);                     /* set reload register */
			NVIC_SetPriority (SysTick_IRQn, (1UL << __NVIC_PRIO_BITS) - 1UL);	/* set Priority for Systick Interrupt */
			SysTick->VAL   = 0UL;                                             /* Load the SysTick Counter Value */
			SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk;
	
		#else
			CLK->AHBCLKE_b.SYSTICKEN =b_ENABLE;
			
			#if (SYSTICK_CLK_SELECTION_DEFINED ==0)
			
				CLK->AHBCLKSEL0_b.SYSTICK =0;
				
				systickclk =((((SystemCoreClock /1000000) /2 ) * SYSTICK_TIME_COUNT_DEFINED) / SYSTICK_DIVIDER_DEFINED);
	
			#elif (SYSTICK_CLK_SELECTION_DEFINED ==1)
			
				CLK->AHBCLKSEL0_b.SYSTICK =1;
	
				systickclk = ((((HSIRC_HZ /1000000) / 2) * SYSTICK_TIME_COUNT_DEFINED) / SYSTICK_DIVIDER_DEFINED);
	
			#elif  (SYSTICK_CLK_SELECTION_DEFINED ==2)

				ACU->IRC_CTRL_b.LSIRC_EN =b_ENABLE;
				
				CLK->AHBCLKSEL0_b.SYSTICK =2;
				
				#if ((SYSTICK_TIME_COUNT_DEFINED /SYSTICK_DIVIDER_DEFINED) < 62)
					#error "LSIRC time base is (62.5us/SysTick Clock Divider). Therefore, User MUST reconfigure the time value."
	
				#endif
				
				systickclk = (((LSIRC_HZ * SYSTICK_TIME_COUNT_DEFINED) /1000000) /SYSTICK_DIVIDER_DEFINED);
	
			#elif (SYSTICK_CLK_SELECTION_DEFINED ==3)
				
				CLK->AHBCLKSEL0_b.SYSTICK =3;
				
				systickclk = ((((XTL_FREQ_DEFINED /1000) / 2) * SYSTICK_TIME_COUNT_DEFINED) /SYSTICK_DIVIDER_DEFINED);	
	
			#endif
			
			SysTick->LOAD  = (uint32_t)(systickclk - 1UL);                    /* set reload register */
			NVIC_SetPriority (SysTick_IRQn, (1UL << __NVIC_PRIO_BITS) - 1UL); /* set Priority for Systick Interrupt */
			SysTick->VAL   = 0UL;                                             /* Load the SysTick Counter Value */
		
		#endif
	
		#if (SYSTICK_INTE_DEFINED ==0)
			SysTick->CTRL |=  SysTick_CTRL_ENABLE_Msk;                         /* Enable SysTick IRQ and SysTick Timer */
	
	  #else
			SysTick->CTRL |=  SysTick_CTRL_TICKINT_Msk   |
												SysTick_CTRL_ENABLE_Msk;                         /* Enable SysTick IRQ and SysTick Timer */
	
		#endif
		
		#if ((SYSTICK_DIVIDER_DEFINED <1) || (SYSTICK_DIVIDER_DEFINED >256))
			#error "SysTick Clock Divier value is illegal, please use the value between 1 to 256"
		
		#endif
		
		CLK->DIV_b.SYSTICK =(SYSTICK_DIVIDER_DEFINED -1);
	
	#endif
	
	//	NVM Key Lock	//
	NVM->FSHKEY =NVM_FSHKEY_LOCK;
	
	//	System Key Lock	//
	SMU->SYSLOCK =SMU_LOCK_KEY;
}


void SystemCoreClockUpdate (void)
{

}
//*** <<< end of configuration section >>>    ***
