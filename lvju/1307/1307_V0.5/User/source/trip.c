#include "main.h"
#include "init.h"
#include "soc.h"
#include "flash.h"
#include "toolfun.h"
#include "hwparam.h"
#include "userfun.h"

aimaparam_type AimaParam;

void last_trip_param_init(void)
{
	AimaParam.ODM_LST 		= AimaParam.ODM;
	AimaParam.DIS_AH_LST 	= Cap.CAP_DIS_ONCE;
	AimaParam.DIS_AH_DCM	= Integral.AH_DIS_ONCE;
}

void last_trip_param_calc(s16 trip, s32 ah_cost, s32 constance)
{
	u16 TempU16;
	TempU16 = math_diveder(trip*constance,ah_cost);
	if (TempU16 < AIMA_K_MAX && TempU16 > AIMA_K_MIN)
	{
		AimaParam.ODM_K = TempU16;
	}
}

void last_trip_process(void)
{
	s32 TempS32;
	#define	 ACUCY_INC		32
	
	
	
	if(SocDisChargeState)
	{
		OneLine.lvjuodm += OneLine.lvjuspeed;
		AimaParam.ODM = math_diveder(OneLine.lvjuodm,360);
	}
	
	
	if (AimaParam.ODM_LST > AimaParam.ODM || AimaParam.DIS_AH_LST > Cap.CAP_DIS_ONCE ||
		 (AimaParam.ODM > AimaParam.ODM_LST + 5 && AimaParam.DIS_AH_LST == Cap.CAP_DIS_ONCE) || SocReadyState)
	{
		last_trip_param_init();
	}
	else if (AimaParam.ODM > AimaParam.ODM_LST + 15)
	{
		if (AimaParam.DIS_AH_DCM > Integral.AH_DIS_ONCE)
		{
			TempS32 = Integral.AH_DIS_ONCE + AH(1/Q_UNIT);
			TempS32 -= AimaParam.DIS_AH_DCM;
			TempS32 = math_diveder(TempS32 * ACUCY_INC, AH(1/Q_UNIT));
			TempS32 += ACUCY_INC*(Cap.CAP_DIS_ONCE - AimaParam.DIS_AH_LST - 1);
		}
		else
		{
			TempS32 = Integral.AH_DIS_ONCE - AimaParam.DIS_AH_DCM;
			TempS32 = math_diveder(TempS32 * ACUCY_INC, AH(1/Q_UNIT));
			TempS32 += ACUCY_INC*(Cap.CAP_DIS_ONCE - AimaParam.DIS_AH_LST);
		}
		
		last_trip_param_calc(AimaParam.ODM - AimaParam.ODM_LST, TempS32, 1024*ACUCY_INC);
		last_trip_param_init();
	}
}




u8 last_trip_calc(void)
{
	return math_diveder(AimaParam.ODM_K*Cap.QNOW*Soc.SHOW,SOC_SHOW_MAX*10)>>10;
}

void last_trip_show_process(void)
{
	u8 TempU8; 
	static u8 first;
	
	TempU8 = last_trip_calc();
	if(first==0)
	{
		AimaParam.LAST_TRIP=TempU8;
		first=1;
	}
	
	
	if (TempU8 > AimaParam.LAST_TRIP)
	{
		AimaParam.LAST_TRIP++;
	}
	else if (TempU8 < AimaParam.LAST_TRIP && AimaParam.LAST_TRIP > 1)
	{
		AimaParam.LAST_TRIP--;
	}
}




