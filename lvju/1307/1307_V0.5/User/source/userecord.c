#include "main.h"
#include "soc.h"
#include "toolfun.h"
#include "flash.h"


typedef struct{
	s32 SUM;
	s16 MAX;
	s16 MIN;
	u16 CNT;
}rcd_param_type;

usr_record_type UseRecord;

void rcd_param_init(rcd_param_type* param, s16 initvalue)
{
	param->SUM = 0;
	param->CNT = 0;
	param->MAX = initvalue;
	param->MIN = initvalue;
}

void rcd_param_process(rcd_param_type* param, s16 newvalue)
{
	param->SUM += newvalue;
	param->CNT++;
	if (param->MAX < newvalue)
	{
		param->MAX = newvalue;
	}
	else if (param->MIN > newvalue)
	{
		param->MIN = newvalue;
	}
}

rcd_param_type RcdVoltage, RcdCurrent;

void use_record_init(u8 type)
{
	SystemFlg.RECORDING  = 1;
	
	UseRecord.TIME_START = His.WORK_SEC;
	UseRecord.SOC_START	 = Soc.INDEX;
	UseRecord.SOC_MAX		 = Soc.INDEX;
	
	if (type)
	{
		UseRecord.ACC_TIME	 = His.DIS_TIME;
		UseRecord.CRG_TIME	 = ++His.CHG_TIME;
	}
	else
	{
		UseRecord.ACC_TIME	 = ++His.DIS_TIME;
		UseRecord.CRG_TIME	 = His.CHG_TIME;
	}
	
	UseRecord.DIS_SUM			 = His.CAPSUM;
	UseRecord.TEMPERATURE	 = SystemParam.PCB_TEMPERATURE - 40;
	UseRecord.TEMP_OUTSIDE = SystemParam.TEMP_NOW;
	UseRecord.FAULT				 = 0;
	
	rcd_param_init(&RcdVoltage, SystemParam.BUS_VOLTAGE);
	rcd_param_init(&RcdCurrent, SystemParam.BUS_CURRENT);
	
}

void use_record_process(void)
{
	if (SystemFlg.RECORDING)
	{
		rcd_param_process(&RcdVoltage,SystemParam.BUS_VOLTAGE);
		rcd_param_process(&RcdCurrent,SystemParam.BUS_CURRENT);
		
		if (Soc.INDEX > UseRecord.SOC_MAX)
		{
			UseRecord.SOC_MAX = Soc.INDEX;
		}
		
		if (SystemFlg.WDG_REST)
		{
			UseRecord.FAULT |= 1;
		}
		
		if (SystemFlg.OTP_FLT)
		{
			UseRecord.FAULT |= 2;
		}
		
		if (SystemFlg.TMP_FLT)
		{
			UseRecord.FAULT |= 4;
		}
		
		if (SystemParam.PCB_TEMPERATURE > UseRecord.TEMPERATURE + 40)
		{
			UseRecord.TEMPERATURE = SystemParam.PCB_TEMPERATURE - 40;
		}
		
		if (SystemParam.TEMP_NOW > UseRecord.TEMP_OUTSIDE)
		{
			UseRecord.TEMP_OUTSIDE = SystemParam.TEMP_NOW;
		}
	}
}

void use_record_end(void)
{
	SystemFlg.RECORDING = 0;
	
	if (Soc.INDEX != UseRecord.SOC_START || His.WORK_SEC > UseRecord.TIME_START + 3600 || UseRecord.DIS_SUM + 8 < His.CAPSUM)
	{
		UseRecord.TIME_END 	= His.WORK_SEC;
		UseRecord.SOC_END		=	Soc.INDEX;
		UseRecord.DIS_SUM		= His.CAPSUM;
		
		UseRecord.VOLT_HIGH	= RcdVoltage.MAX;
		UseRecord.VOLT_LOW	= RcdVoltage.MIN;
		UseRecord.VOLT_AVG	= math_diveder(RcdVoltage.SUM,RcdVoltage.CNT);
		
		UseRecord.CURRENT_HIGH = RcdCurrent.MAX;
		UseRecord.CURRENT_LOW	 = RcdCurrent.MIN;
		UseRecord.CURRENT_AVG	 = math_diveder(RcdCurrent.SUM,RcdCurrent.CNT);
		
		record_data_save();
	}
}


