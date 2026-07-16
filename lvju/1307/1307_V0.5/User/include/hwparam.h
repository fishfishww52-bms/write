#ifndef __hw_param_h
#define	__hw_param_h
#include "type.h"


#define		BUS_RES_SIG		0.0020
#define		BUS_RES_NUM		3

#define		OPA_RES_R1		3.3
#define		OPA_RES_R2		200.0
#define		OPA_RES_R3		3.3
#define		OPA_RES_R4		100.0

#define		COV_SHIFT			12
#define		COV_AMP				4096

#define		OPA_OFFSET_DEFAULT		(u16)(MCU_PWR*OPA_RES_R1*(OPA_RES_R3+OPA_RES_R4)*ADC_AMP/OPA_RES_R3/(OPA_RES_R1+OPA_RES_R2))
#define		OPA_AMP								(float)(OPA_RES_R2*(OPA_RES_R3+OPA_RES_R4)/OPA_RES_R3/(OPA_RES_R1+OPA_RES_R2))
#define		CURRENT_COV_CNST			(s32)(CURRENT_UNIT*COV_AMP/ADC_AMP/(BUS_RES_SIG/BUS_RES_NUM)/OPA_AMP)


#define		BAT_RES_R1					3000
#define		BAT_RES_R2					127       //150
#define		BAT_VOLT_AMP				(float)((BAT_RES_R1 + BAT_RES_R2)/BAT_RES_R2)
#define		BAT_VOLT_CONST			(s32)(VOLTAGE_UNIT*COV_AMP*BAT_VOLT_AMP/ADC_AMP)
#define		BATTERY_VOLTAGE(x)	(s16)((x)*ADC_AMP/BAT_VOLT_AMP)

#define		NTC_RES_R1					10
#define		NTC_RES_R2					100
#define		NTC_TMP_RES(x)			(u16)(MCU_PWR*ADC_AMP*NTC_RES_R2/(NTC_RES_R1 + NTC_RES_R2 + x))
#define		PCB_TEMP(X)					(X+40)

#define		NTC_OSD_R1					4.7
#define		NTC_OSD_R2					47.0
#define		NTC_OSD_RES(x)			(u16)(MCU_PWR*ADC_AMP*NTC_OSD_R2/(NTC_OSD_R1 + NTC_OSD_R2 + x))


extern adc_type AdcResult,AdcAvg;
extern key_param_raw_type KeyRawParam;
#endif
