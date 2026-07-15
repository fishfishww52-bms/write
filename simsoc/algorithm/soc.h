#ifndef __soc_h
#define	__soc_h

#include "type.h"

#define		CELL_NUM_MIN							4
#define		CELL_NUM_MAX							6
#define		CELL_NUM_DEFAULT					5

#define		CAP_RATING_MIN						6
#define		CAP_RATING_MAX						200
#define		CAP_RATING_DEFAULT				38

#define		SOC_SHOW_MAX							200
#define		SOC_SHOW_VALUE(x)					((u8)(SOC_SHOW_MAX*x))
#define		SOH_MAX										200
#define		SOH_MIN										100

#define		SOC_INDEX_MAX							1024
#define		SOC_INDEX_VALUE(x)				((s16)(SOC_INDEX_MAX*x))
#define		SOC_FULL_MIN							SOC_INDEX_VALUE(0.8)
#define		SOC_ZERO_MAX							SOC_INDEX_VALUE(0.5)
#define		SOC_ZERO_MIN							-SOC_INDEX_VALUE(0.8)

#define		TOTAL_CELL_VOLTAGE(x)			(u16)(VOLTAGE_V(x)*Bat.CELL_NUM)

#define		Q_UNIT										8
#define		Q_VALUE(x)								((s16)(Q_UNIT*x))
#define		Q_VALUE_MAX								Q_VALUE(CAP_RATING_MAX)
#define		Q_VALUE_MIN								Q_VALUE(CAP_RATING_MIN)
#define		Q_VALUE_DEFAULT						Q_VALUE(CAP_RATING_DEFAULT)

#define		SOC_READY_ST							0
#define		SOC_DISCHARGE_ST					1
#define		SOC_CHARGE_ST							2
#define		SOC_FULL_CHARGE_ST				3

#define		GotoChargeState()					SystemParam.STATE = SOC_CHARGE_ST
#define		GotoDisChargeState()			SystemParam.STATE = SOC_DISCHARGE_ST
#define		GotoReadyState()					SystemParam.STATE = SOC_READY_ST
#define		GotoFullChargeState()			SystemParam.STATE = SOC_FULL_CHARGE_ST

#define		SocReadyState							(SystemParam.STATE == SOC_READY_ST)
#define		SocDisChargeState					(SystemParam.STATE == SOC_DISCHARGE_ST)
#define		SocChargeState						(SystemParam.STATE == SOC_CHARGE_ST)
#define		SocFullChargeState				(SystemParam.STATE == SOC_FULL_CHARGE_ST)
#define		SocShowAllowIncState			(SystemParam.STATE >= SOC_CHARGE_ST)

#define		CURRENT_UNIT							64
#define		VOLTAGE_UNIT							64
#define		CURRENT_A(x)							((s16)((x)*CURRENT_UNIT))
#define	 	VOLTAGE_V(x)							((s16)((x)*VOLTAGE_UNIT))

#define		DIS_CURRENT(x)						CURRENT_A(-x)
#define		CHG_CURRENT(x)						CURRENT_A(x)

#define		S_UNIT										(s32)(10)
#define		M_UNIT										(s32)(60*S_UNIT)
#define		H_UNIT										(s32)(60*M_UNIT)

#define		HOURS(x)									((s32)((x)*H_UNIT))
#define		SECONDS(x)								((s32)((x)*S_UNIT))
#define		MINITES(x)								((s32)((x)*M_UNIT))

#define		AH(x)											((s32)(CURRENT_UNIT*H_UNIT*(float)x))
#define		WH(x)											((s32)(CURRENT_UNIT*VOLTAGE_UNIT*H_UNIT*(float)x))

#define		CHARGE_EFFICIENCY_MIN			(u16)(0.88*4096)
#define		EFFICIENCY_SHIFT					12
#define		SOC_1_Q_CONST_CHG					((s32)((float)H_UNIT*CURRENT_UNIT*515/Q_UNIT/SOC_INDEX_MAX))//0x235CB
#define		SOC_1_Q_CONST_DIS					((s32)((float)H_UNIT*CURRENT_UNIT*512/Q_UNIT/SOC_INDEX_MAX))//0x23280
#define		SOC_1_Q_SHIFT							9

#define		CURRENT_CHG_LEARN_BLOCK		AH(0.3)
#define		DISCHARGE_ZERO_FIND_BLOCK	AH(0.05)

#define		charge_current_larger(x)			(SystemParam.AVG_CURRENT > CURRENT_A(x))
#define		charge_current_smaller(x)			(SystemParam.AVG_CURRENT < CURRENT_A(x))
#define		discharge_current_larger(x)		(SystemParam.AVG_CURRENT < CURRENT_A(-(x)))
#define		discharge_current_smaller(x)	(SystemParam.AVG_CURRENT > CURRENT_A(-(x)))

#define		INDEX_DEC_MIN									(s32)(SOC_1_Q_CONST_DIS*2/64)//Full cap is 2AH
#define		INDEX_DEC_MAX									(s32)(SOC_1_Q_CONST_DIS*200/64)//Full cap is 200AH

#define		SOC_ARRAY_X					11
#define		SOC_ARRAY_Y					7
#define		CHG_VOLT_Y					11
#define		CHG_VOLT_X					7

typedef struct{
	s32 INTEGRAL;
	s32 POWER;
	s16 SOE;
	s16 FULL;
	s16 DIS_WH_ONCE;
	s16 CHG_WH_ONCE;
	u16 HOURS_12_CNT;
}soetype;

typedef struct{
	u8 CELL_NUM;
	u8 CAP_RATING;
	u8 CAP_NOW;
}battype;

typedef struct{
	s16 SHOW;
	s16 INDEX;
	s16 FULL;
	s16 ZERO;
	s16 OCV_SOC;
	s16 SOH;
	s16 CIRCLE;
	s16 ZERO_DELTA;
	s16 ZERO_SOC;
	s16 DIS_SOC;
	s16 CHG_SOC;
	s16 OCV_START;
	s16 DEC_SHOW;
	s16 HIGH_LIMIT;
	s16 LOW_LIMIT;
}soctype;

typedef struct{
	u16 Q25;
	u16 QNOW;
	u16 ARRAY[5];
	u16 CAP_GUSS_IN_CURRENT_CHARGE;
	s16 CAP_GUSS_LAST;
	s16 CAP_GUSS_TOTAL_DIS;
	u16 CAP_CHG_ONCE;
	u16 CAP_DIS_ONCE;
	u16 CAP_CHG_LAST;
	u16 CAP_LEARN_MAX;
	u16 CAP_SET;
	u16 CAP_LEARN_MIN;
	s16 SET_IMPACT;
	u16 CAP_LEARN_INDEX;
	u16 CAP_SET_DEC_CNT;
	s16 AVG_VOLTAGE[3];
	s16 AVG_CURRENT[3];
	s16 DIS_CAP[3];
	s8 INDEX;
}captype;

typedef struct{
	u32 CIRCLE;
	u32 CAPSUM;
	u32 OPEN;
	u32 WORK_SEC;
	u32 LAST_VOLT;
	u32 CAPSUM_LST;
	u32 FLT_CNT;
	u32 DIS_TIME;
	u32 CHG_TIME;
}histype;

typedef struct{
	u32 BLOCK					:1;
	u32 A_HOUR				:1;
	u32 FULL					:1;
	u32 ZERO					:1;
	u32 FULL_DIS			:1;
	u32 RATING_LEARN	:1;
	u32 ZERO_START		:1;
	u32 VERIFYED			:1;
	u32 SHOW_DEC			:1;
	u32 CAP_INC				:1;
	u32 CHG_VOLT			:1;
	u32 FORCE_FULL		:1;
	u32 SHOW_FULL			:1;
	u32 DIS_HPN				:1;
	u32 CHG_HIGH			:1;
	u32 FBD_SLW				:1;
	u32 DEC_SLOW			:1;
	u32 SOE_ZERO			:1;
}markstype;

typedef struct{
	u32 SUM_VOLTAGE;
	s32 SUM_CURRENT;
	s32 AH_0001_DIS;
	s32 AH_0001_CHG;
	s32 AH_0001_CMP_DIS;
	s32 AH_0001_CMP_CHG;
	s32 AH;
	s32 AH_LAST;
	s32 AH_CHG_ONCE;
	s32 AH_DIS_ONCE;
	s32 ZERO_AH;
	u32 ZERO_VOLTAGE;
	u32 DIS_AH_BLOCK;
}soc_integral_type;

typedef struct{
	s32 CHG_CNT_30S;//30s-40s delay counter for charge and discharge process
	s32 STATE_SW_CNT_1;//delay counter for soc state swtich 1
	s32 STATE_SW_CNT_2;//delay counter for soc state switch 2
	s32 BLOCK_CNT;//use to filter current and voltage in a block
	u32 CHG_TIME_CNT;//time counter for charge and full charge
	u32 DIS_TIME_CNT;//time counter for discharge
	u32 SOC_0_CMP_TIME;//period for decrease soc zero in charge process
	u32 SOC_0_CMP_CNT;//counter for decrease soc zero in charge process
	u32 CHARGE_VOLT_CNT;//time counter for constant voltage charge process
	u32 DISCHG_OV_2A;
	u32 CHARGE_UD_1A;//counter for charge current smaller than 1A in charge process
	u32 CHARGE_LD_CNT;//counter for charge current smaller than 0.22 constant charge current
	s32 VALID_CNT;
	u32 ZERO_CNT;
	u16 HOURS_FULL_DIS;//use to clear full learning flag
	u16 A_MONTH_CNT;
	u8 EIGHT_HOUR;
	u8 FULL_CLR_CNT;
	u8 DIS_BLOCK_CNT;
	u8 DIS_NOFULL_CNT;
	u8 DIS_LEARN_CNT;
	u8 CHG_LEARN_CNT;
}soc_count_type;

typedef struct{
	s16 C01_CURRENT;
	s16 CHG_CURRENT;
	s16 AVG_CURRENT;
	s16 MAX_VOLTAGE;
	s16 MAX_CURRENT;
	s16 AVG_VOLTAGE;
	s16 MIN_VOLTAGE;
	s16 MIN_CURRENT;
}block_type;


extern battype 	Bat;
extern soctype 	Soc;
extern soetype	Soe;
extern captype 	Cap;
extern histype 	His;
extern markstype Marks;
extern soc_integral_type 	Integral;
extern soc_count_type			SocCounter;
extern block_type 				BlockParam;

extern const u16 CHG_VOLT_ARRAY[],CHG_VOLT_HIGH_LIMIT[CHG_VOLT_Y][CHG_VOLT_X],CHG_VOLT_LOW_LIMIT[CHG_VOLT_Y][CHG_VOLT_X];
extern const u16 VOLT_CHG_POINT_ARRAY[],CHG_CUR_LOW_LIMIT[SOC_ARRAY_Y][SOC_ARRAY_X],CHG_CUR_HIGH_LIMIT[SOC_ARRAY_Y][SOC_ARRAY_X];

extern void soc_process(void), soc_key_paramter_init(void), soc_init(void), cap_calc(void), soc_reset(u8 cap, u8 cell_num, u8 set_cap);
extern void soc_show_process(void), uid_save(void);
extern void soc_zero_decrease_process(void), update_cap_array(u16 temp_cap, u8 incflg);
extern void discharge_zero_detect(void);
extern void discharge_soc_compensation(void);
extern u16 soc_ocv_calc(u16);
extern u16 q_curve_inv_calc(u16 cap_now);
extern u16 q_curve_calc(u16 q_25, u8 full);
#endif
