#ifndef __userfun_h
#define	__userfun_h
#include "type.h"


#define		AccIOIsValid			(GPIO1->IDR_b.P09 == 0)
#define		CrgIOIsValid			(GPIO0->IDR_b.P01)


#define		DisableComPullUp()	    GPIO0->P08 = 1
#define		EnableComPullUp()		    GPIO0->P08 = 0

#define		Rs485Tx_En()			GPIO0->P07 = 1
#define		Rs485Rx_En()			GPIO0->P07 = 0
#define		Rs485Txing				GPIO0->ODR_b.P07    //485


#define  ALineRxisLow  (GPIO2->IDR_b.P11 == 0)

#define		ChargerIOHigh							(GPIO0->IDR_b.P01==1)   


#define		TX_NUM						100
#define		RX_LEN						250
#define		BOOT_CMD_LEN			20
#define		FRAME_HEAD_E			'E'
#define		FRAME_HEAD_R			'R'
#define		FRAME_HEAD_T			'T'
#define		FRAME_HEAD_H			'H'
#define		OP_SUCCESS				0
#define		NO_BOOT_CTRL			1
#define		NO_BOOT_AUTH			2
#define   FRAME_TYPE_ERR		3
#define		EARASE_ERROR			4
#define		WRITE_ERROR				5
#define		READ_ERROR				6
#define		CHECK_ERROR				7
#define		CODE_OUT_SIZE			8
#define		INDEX_ERROR				9
#define		HW_VERSION_ERR		11
#define		CHIP_ERR					10
#define		BOOTLOAD_CMD			0xAA

#define		OP_SUCCESS				0
#define		NO_WRITE_AUTH			1
#define		WRITE_OUT_SIZE		2
#define		NO_READ_AUTH			3
#define		READ_OUT_SIZE			4

typedef struct{
	u16 SUM;
	u8 STATE;
	u8 INDEX;
	u8 LEN;
	u8 CMD;
	u8 DATA[RX_LEN];
	u8 RXD;
	u8 TXING;
	u8 RX_TO_CNT;
	u8 EH_FRAME;
	u8 CST_FRAME;
	s16 UART_EN_CLR;
}uart_data_type;



#define		ONE_LINE_TX_DATA_NUM  19
#define		ONE_LINE_RX_DATA_NUM	19

typedef struct {
	u32  lvjuodm;
	u16  lvjuspeed;
	u16  chglft;
	u16 RX_CNT;
	u16 RX_CNT_L;
	u8 TX_STATE;
	u8 RX_STATE;
	u8 TX_INDEX;
	u8 RX_INDEX;
	u8 TX_MASK;
	u8 RX_MASK;
	u8 RX_XOR;
	u8 RX_BAT; 
	u8 LST_BAT;
	u8 CHK_CNT;
	u8 CHK_CNT_cap;
	u8 TX_TYPE;
	u8 lst_rx_cap;
	u8 rx_cap;
	u8 cap;
	u16 DELAY;
	u8 TX_DATA[ONE_LINE_TX_DATA_NUM];
	u8 RX_DATA[ONE_LINE_RX_DATA_NUM];
}a_line_type;



#define		AIMA_K_MAX								5000
#define		AIMA_K_MIN								1000
#define		AIMA_K_DEFAULT						3335


typedef struct{ 
	
	u32 ODM;
	u32 ODM_LST;
	s32 DIS_AH_DCM;
	u16 ODM_K;
	u16 DIS_AH_LST;
	u8 SPEED;
	u8 LAST_TRIP; 
	
}aimaparam_type;



extern a_line_type OneLine;
extern aimaparam_type AimaParam;

extern u8 Uart0TxArray[],Uart2TxArray[];

extern void getchglft(void);
extern void last_trip_show_process(void);
extern void last_trip_process(void);

#endif
