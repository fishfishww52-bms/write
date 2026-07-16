#ifndef __type_h
#define __type_h

typedef   signed          char s8;
typedef   signed short     int s16;
typedef   signed           int s32;
typedef   signed       __int64 s64;

typedef unsigned          char u8;
typedef unsigned short     int u16;
typedef unsigned           int u32;
typedef unsigned       __int64 u64;


typedef	void  (*func_Pntr)(void);

#define		abs(x)				(((x)>0)?(x):(-(x)))


typedef struct{
	u32 SLEEPING 	:1;
	u32 ERT_SAVE	:1;
	u32 SOC_SAVE	:1;
	u32 WDG_REST	:1;
	volatile u32 UART0_EN	:1;
	u32 OTP_FLT		:1;
	u32 DATA_TXING	:1;
	u32 CNCHANGE		:1;
	u32 TESTERING		:1;
	u32 AIMA_CHG		:1;
	u32 CRG_IN			:1;
	u32 ACC_IN			:1;
	u32 CRG_VALID		:1;
	u32 TMP_FLT			:1;
	u32 BP_VALID		:1;
	u32 BP_SWITCH		:1;
	u32 PWR_OFF			:1;
	u32 COMEXTUP		:1;
	u32 CUR_VALID		:1;
	u32 RECORDING		:1;
	u32 RCDSEND			:1;
	u32 E_MOTOR				:1;
}sys_flg_type;

typedef struct{
	u16 R[6];
	u16 OPA_REF;
	u16 ADC_VSS;
}b_value_type;

typedef struct{
	u16 VOLTAGE;
	u16 CURRENT;
	u16 CURRENT_ARY[6];
	s16 CURRENT_RAW[6];
}k_value_type;

typedef struct{
	u8 HW;
	u8 SW_M;
	u8 SW_S;
	u8 R[4];
	u8 SOC_M;
	u8 SOC_S;
	u8 SW_S1;
	u8 SW_S2;
	u8 SW_S3;
	u8 CP_ID;
}version_type;

typedef struct{
		u16 SLEEP_CNT;
//volatile	u8 TICK_100us;
//					u8 TICK_500us;
volatile	u8 TICK_2ms;
volatile	u8 TICK_1ms;
					u8 TICK_10ms;
					u8 TICK_100ms;
					u8 TICK_1s;
					u8 SYS30STICK;
volatile	u8 TICK_ADC;
					u8 TICK_WAKEUP;
}sys_tick_type;


typedef struct{
	u16 VOLTAGE;
	u16 CURRENT;
	u16 TEMPERATURE;
	u16 TEMP_OUTSIDE;
	u16 AVSS;
}adc_type;

typedef struct{
	s16 CURRENT;
	u16 VOLTAGE;
	s16 AVG_CURRENT;
}key_param_raw_type;

typedef struct{
	s16 BUS_CURRENT;
	u16 BUS_VOLTAGE;
	s8 TEMPERATURE;
	s8 TEMP_NOW;
	u8 STATE;
	u8 PCB_TEMPERATURE;
	s16 AVG_CURRENT;
}system_param_type;

typedef struct{
	s16 DATA[16];
}test_data_type;

#define	FLASH_USE_DATA_NUM 10
typedef struct{
	u32 TIME_START;
	u32 TIME_END;
	u16 ACC_TIME;
	u16 CRG_TIME;
	u16 SOC_START;
	u16 SOC_MAX;
	u16 SOC_END;
	u16 VOLT_HIGH;
	u16 VOLT_LOW;
	u16 VOLT_AVG;
	s16 CURRENT_HIGH;
	s16 CURRENT_LOW;
	s16 CURRENT_AVG;
	u16 SOH;
	u16 DIS_SUM;
	u16 FAULT;
	s16 TEMPERATURE;
	u16 TEMP_OUTSIDE;
}usr_record_type;

#endif
