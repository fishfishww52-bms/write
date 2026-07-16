#include "main.h"
#include "flash.h"
#include "hwparam.h"
#include "toolfun.h"
#include "soc.h"
#include "userfun.h"


pn_type PN;

void flash_data_to_ram(u32 addr, u8 size, u32* ptr)
{
	u32 *buffer = (u32*)addr;
	
	while(size--)
	{
		*ptr++ = *buffer++;
	}
}

void flash_read(void)
{
	u32 FlashData[SECTOR_NUM_MAX];
	u8 i,*ptr; 
	
	if (FLASH_DEFAULT != *(u32*)0x01010000)
	{ 
			erase_flash_sector(0x01010000);
		  
	}			
	
	flash_data_to_ram(ERT_DATA_ADDR,ERT_DATA_NUM,FlashData);
	
	if (FlashData[0] != FLASH_DEFAULT)
	{
		SystemFlg.ERT_SAVE = 1;
	}

	BValue.OPA_REF = FlashData[5];
	b_value_verify(&BValue.OPA_REF,OPA_OFFSET_DEFAULT+ADC_VALUE(0.65),OPA_OFFSET_DEFAULT-ADC_VALUE(0.65),OPA_OFFSET_DEFAULT);
	
	KValue.CURRENT = FlashData[6];
	KValue.VOLTAGE = FlashData[7];
	
	k_value_verify(&KValue.CURRENT);
	k_value_verify(&KValue.VOLTAGE);
	
	if (FlashData[8] == WATCHDOG_RST)
	{
		SystemFlg.WDG_REST = 1;
		SystemFlg.ERT_SAVE = 1;
	}
	
	SystemFlg.BP_SWITCH = (FlashData[9] == 0)?1:0;
	
	for (i = 0; i < 6; i++)
	{
		KValue.CURRENT_RAW[i] = FlashData[10 + i];
		KValue.CURRENT_ARY[i] = FlashData[16 + i];
	}
	
	for (i = 0; i < 6; i++)
	{
		if (k_value_verify(&KValue.CURRENT_ARY[i]))
		{
			break;
		}
	}
	
	SystemFlg.CUR_VALID = 0;
	if (i >= 6)
	{
		if (KValue.CURRENT_RAW[0] > -CURRENT_A(60) && KValue.CURRENT_RAW[0] < -CURRENT_A(40) && KValue.CURRENT_RAW[1] > -CURRENT_A(40) && KValue.CURRENT_RAW[1] < -CURRENT_A(20) &&
				KValue.CURRENT_RAW[2] > -CURRENT_A(20) && KValue.CURRENT_RAW[2] < CURRENT_A(0)	 && KValue.CURRENT_RAW[3] > CURRENT_A(0)   && KValue.CURRENT_RAW[5] < CURRENT_A(60) &&
				KValue.CURRENT_RAW[3] < CURRENT_A(20)  && KValue.CURRENT_RAW[4] > CURRENT_A(20)  && KValue.CURRENT_RAW[4] < CURRENT_A(40)  && KValue.CURRENT_RAW[5] > CURRENT_A(40))
		{
			SystemFlg.CUR_VALID = 1;
		}
	}
	
	flash_data_to_ram(SOC_DATA_ADDR,SOC_DATA_NUM,FlashData);
	
	Soc.SHOW 	= (FlashData[0] > SOC_SHOW_MAX)?SOC_SHOW_VALUE(0.8):FlashData[0];
	Soc.INDEX	=	(FlashData[1] > SOC_INDEX_MAX)?SOC_INDEX_VALUE(0.8):FlashData[1];
	Soc.FULL 	= (FlashData[2] > SOC_FULL_MIN && FlashData[2] < SOC_INDEX_MAX)?FlashData[2]:SOC_INDEX_MAX;
	
	#ifdef SHOW_DEC_SLOW
	Marks.DEC_SLOW 	= 1;
	Soc.DEC_SHOW		= SOC_INDEX_MAX + SOC_ZERO_MIN;
	Soc.ZERO	= FlashData[3];
	if (Soc.INDEX > Soc.DEC_SHOW)
	{
		Soc.ZERO 	= Soc.DEC_SHOW - Soc.INDEX;
	}
	else if (Soc.ZERO > SOC_ZERO_MAX || Soc.ZERO < 0)
	{
		Soc.ZERO 	= 0;
	}
	#else
	Marks.DEC_SLOW 	= 0;
	Soc.ZERO				= (FlashData[3] < SOC_ZERO_MAX)?FlashData[3]:0;
	#endif
	
	Soc.SOH 	= (FlashData[4] > SOH_MIN && FlashData[4] < SOH_MAX)?FlashData[2]:SOH_MAX;
	for (i = 0; i < 5; i++)
	{
		Cap.ARRAY[i] = (FlashData[i+5] >= Q_VALUE_MIN && FlashData[i+5] <= Q_VALUE_MAX)?FlashData[i+5]:Q_VALUE_DEFAULT;
	}
	Bat.CAP_RATING 	= (FlashData[10] <= CAP_RATING_MAX && FlashData[10] >= CAP_RATING_MIN)?FlashData[10]:CAP_RATING_DEFAULT;
	Bat.CELL_NUM 		= (FlashData[11] <= CELL_NUM_MAX && FlashData[11] >= CELL_NUM_MIN)?FlashData[11]:CELL_NUM_DEFAULT;
	His.CAPSUM			= (FlashData[12] < 8000000)?FlashData[12]:0;
	His.CIRCLE			= (FlashData[13] < 10000)?FlashData[13]:0;
	His.WORK_SEC		= FlashData[14]+1;
	His.FLT_CNT			= (FlashData[15]!=FLASH_DEFAULT)?FlashData[15]:0;
	His.LAST_VOLT		= FlashData[16];
	Bat.CAP_NOW			= (FlashData[17] <= CAP_RATING_MAX && FlashData[17] >= CAP_RATING_MIN)?FlashData[17]:CAP_RATING_DEFAULT;
	Cap.CAP_SET			= (FlashData[18] > Q_VALUE_MAX || FlashData[18] < Q_VALUE_MIN)?0:FlashData[18];
	His.DIS_TIME		= (FlashData[19] > 100000)?0:FlashData[19];
	His.CHG_TIME		= (FlashData[20] > 100000)?0:FlashData[20];
	His.OPEN				= FlashData[21] + 1;
	
	if (Cap.CAP_SET)
	{
		Cap.SET_IMPACT = (FlashData[22] < 128)?FlashData[22]:0;
	}
	else
	{
		Cap.SET_IMPACT = 0;
	}
	 
	OneLine.cap = (FlashData[23] <= CAP_RATING_MAX && FlashData[23] >= CAP_RATING_MIN)?FlashData[23]:CAP_RATING_DEFAULT;
	
	
	AimaParam.ODM_K = FlashData[24];
	if (AIMA_K_MAX < AimaParam.ODM_K || AimaParam.ODM_K < AIMA_K_MIN)
	{
		AimaParam.ODM_K = AIMA_K_DEFAULT;
	}
	
	
	
	FlashData[0] = PNN_DATA_ADDR;
	ptr = (u8*)&PN;
	
	for (i = 0; i < ERT_PN_NUMBER + CST_PN_NUMBER + UID_NUMBER; i++)
	{
		*ptr++ = *(u8*)FlashData[0];
		FlashData[0]++;
	}
	
	Version.CP_ID = CHIP_TYPE;
	Version.HW 		= HW_VERSION;
	Version.SW_M	=	SW_VERSION_M;
	Version.SW_S	= SW_VERSION_S;
	
	SystemFlg.SOC_SAVE	= 1;
}





u32 erase_flash_sector(u32 addr)
{
	u32 value;
	__disable_irq();
	
	if(READ_BIT(NVM->CTRL, NVM_LOCK))
	{
		NVM->FSHKEY =NVM_FSHKEY_1;
		NVM->FSHKEY =NVM_FSHKEY_2;
	}
	
	value = 0;
	
	SET_BIT(NVM->CTRL, NVM_TYPE_PAGE_ERASE);
	
	NVM->ADDR = addr;
	
	SET_BIT(NVM->CTRL, NVM_START);
	
	while(READ_BIT(NVM->STS, NVM_STS_BUSY))
	{
		if (value < 20000)
			value++;
		else
			break;
	}
	
	MODIFY_REG(NVM->CTRL, NVM_TYPE_PAGE_ERASE, NVM_TYPE_IDLE);
	
	NVM->FSHKEY = NVM_FSHKEY_LOCK;
	
	__enable_irq();
	
	if (value < 20000)
		value = 0;
	
	return value;
}


u32 flash_boot_write(u32 * FlashAddress, u32 *Data, u32 Datalength)
{
	u32 value, i, addr;
	__disable_irq();
	
	if(READ_BIT(NVM->CTRL, NVM_LOCK))
	{
		NVM->FSHKEY =NVM_FSHKEY_1;
		NVM->FSHKEY =NVM_FSHKEY_2;
	}
	SET_BIT(NVM->CTRL, NVM_TYPE_PROGRAM);
	
	for (i = 0; i < Datalength; i++)
	{
		addr = *FlashAddress;
		*(uint32_t *)addr = *(uint32_t*)(Data+i);
		value = 0;
		while(READ_BIT(NVM->STS, NVM_STS_BUSY))
		{
			if (value < 10000)
				value++;
			else
				break;
		}
		
		if (value >= 10000)
		{
			value = 2;
			break;
		}
		else if (*(uint32_t *)addr != *(uint32_t*)(Data+i))
		{
			value = 3;
			break;
		}
		else
			value = 0;
		*FlashAddress += 4; 
	}
	MODIFY_REG(NVM->CTRL, NVM_TYPE_PROGRAM, NVM_TYPE_IDLE);
	__enable_irq();
	return value;
}





u32 ram_data_to_flash(u32 flash_addr, u32 *ramdata, u8 length)
{
	u32 i,value;
	
	__disable_irq();
	if(READ_BIT(NVM->CTRL, NVM_LOCK))
	{
		NVM->FSHKEY = NVM_FSHKEY_1;
		NVM->FSHKEY = NVM_FSHKEY_2;
	}
	SET_BIT(NVM->CTRL, NVM_TYPE_PAGE_ERASE);
	NVM->ADDR = flash_addr;
	SET_BIT(NVM->CTRL, NVM_START);
	value = 0;
	while(READ_BIT(NVM->STS, NVM_STS_BUSY))
	{
		if (value < 20000)
			value++;
		else
			break;
	}
	MODIFY_REG(NVM->CTRL, NVM_TYPE_PAGE_ERASE, NVM_TYPE_IDLE);
	if (value < 20000)
	{
		value = 0;
		SET_BIT(NVM->CTRL, NVM_TYPE_PROGRAM);
		for (i = 0; i < length; i++)
		{
			*(uint32_t *)flash_addr = *ramdata;
			while(READ_BIT(NVM->STS, NVM_STS_BUSY))
			{
				if (value < 10000)
					value++;
				else
					break;
			}
			if (value >= 10000)
			{
				value = 2;
				break;
			}
			else if (*(uint32_t *)flash_addr != *ramdata)
			{
				value = 3;
				break;
			}
			else
				value = 0;
			ramdata++;
			flash_addr += 4;
		}
		MODIFY_REG(NVM->CTRL, NVM_TYPE_PROGRAM, NVM_TYPE_IDLE);
	}
	else
	{
		value = 1;
	}
	NVM->FSHKEY =NVM_FSHKEY_LOCK;
	__enable_irq();
	
	return value;
}

//void ert_save(u8 boottype, u32 com_device, u8 watchdog_rst,  u32 baud_rate)
void ert_save(u8 boottype, UART_Type *UARTx, u8 watchdog_rst,  u32 baud_rate)
{
	u32 FlashData[ERT_DATA_NUM], i;
	
	FlashData[0] = boottype?BOOT_MODE_EN:FLASH_DEFAULT;
	FlashData[1] = (u32)UARTx;
	FlashData[2] = UARTx->CTRL;
	FlashData[3] = (baud_rate)?baud_rate:UARTx->BAUD_RATE;
	FlashData[4] = 0;
	FlashData[5] = BValue.OPA_REF;
	FlashData[6] = KValue.CURRENT;
	FlashData[7] = KValue.VOLTAGE;
	FlashData[8] = watchdog_rst?WATCHDOG_RST:FLASH_DEFAULT;
	FlashData[9] = SystemFlg.BP_SWITCH?0:FLASH_DEFAULT;
	
	for (i = 0; i < 6; i++)
	{
		FlashData[i + 10] = KValue.CURRENT_RAW[i];
		FlashData[i + 16] = KValue.CURRENT_ARY[i];
	}
	
	if (0 == ram_data_to_flash(ERT_DATA_ADDR,FlashData,ERT_DATA_NUM))
	{
		SystemFlg.ERT_SAVE = 0;
	}
}

void soc_save(void)
{
	u32 FlashData[SOC_DATA_NUM];
	
	FlashData[0] = Soc.SHOW;
	FlashData[1] = Soc.INDEX;
	FlashData[2] = Soc.FULL;
	FlashData[3] = Soc.ZERO;
	FlashData[4] = Soc.SOH;
	FlashData[5] = Cap.ARRAY[0];
	FlashData[6] = Cap.ARRAY[1];
	FlashData[7] = Cap.ARRAY[2];
	FlashData[8] = Cap.ARRAY[3];
	FlashData[9] = Cap.ARRAY[4];
	FlashData[10] = Bat.CAP_RATING;
	FlashData[11] = Bat.CELL_NUM;
	FlashData[12] = His.CAPSUM;
	FlashData[13] = His.CIRCLE;
	FlashData[14] = SystemFlg.TESTERING?0:His.WORK_SEC;
	FlashData[15] = His.FLT_CNT;
	FlashData[16] = SystemFlg.TESTERING?0:His.LAST_VOLT;
	FlashData[17] = Bat.CAP_NOW;
	FlashData[18] = Cap.CAP_SET;
	FlashData[19] = His.DIS_TIME;
	FlashData[20] = His.CHG_TIME;
	FlashData[21] = His.OPEN;
	FlashData[22] = Cap.SET_IMPACT;
	
	FlashData[23] = OneLine.cap;
	
	FlashData[24] = AimaParam.ODM_K;
	
	if (0 == ram_data_to_flash(SOC_DATA_ADDR,FlashData,SOC_DATA_NUM))
	{
		SystemFlg.SOC_SAVE = 0;
	}
}

void pn_save(void)
{
	u32 FlashData[PNN_DATA_NUM];
	u8 *ptr_s, *ptr_t, i;
	
	ptr_s = (u8*)&PN;
	ptr_t = (u8*)FlashData;
	for (i = 0; i < ERT_PN_NUMBER + CST_PN_NUMBER + UID_NUMBER; i++)
	{
		*ptr_t++ = *ptr_s++;
	}
	ram_data_to_flash(PNN_DATA_ADDR,FlashData,PNN_DATA_NUM);
}

void record_data_save(void)
{
	extern usr_record_type UseRecord;
	u32 flash_data[128], addr = RCD_DATA_ADDR, addr2;
	u8 i;
	
	addr2 = (u32)&flash_data[FLASH_USE_DATA_NUM];
	for (i = 0; i < 128 - FLASH_USE_DATA_NUM; i++)
	{
		*(u32*)addr2 = *(u32*)addr;
		addr  += 4;
		addr2 += 4;
	}
	
	addr = (u32)&UseRecord;
	addr2 = (u32)flash_data;
	
	for (i = 0; i < FLASH_USE_DATA_NUM; i++)
	{
		*(u32*)addr2 = *(u32*)addr;
		addr2 += 4;
		addr 	+= 4;
	}
	
	ram_data_to_flash(RCD_DATA_ADDR,flash_data,128);
}

void flash_data_save_process(void)
{
	if (SystemFlg.ERT_SAVE)
	{
		ert_save(0,UART0,0, 0);
	}
	else if (SystemFlg.SOC_SAVE)
	{
		soc_save();
	}
}

