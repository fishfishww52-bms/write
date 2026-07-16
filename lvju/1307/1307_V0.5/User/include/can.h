#ifndef __can_h
#define __can_h

#include "gpm32f13xx_canfd.h"
#include "type.h"
//#define  	CAN_LOOP_BACK_TEST

#define		CAN_RX_MSG_OBJ_0			1
#define		CAN_RX_MSG_OBJ_1			2
#define		CAN_TX_MSG_OBJ_S			3

#define		CAN_TX_MSG_OBJ_START	17
#define		CAN_TX_MSG_OBJ_END		32

#define		CAN_BIT_TIME				(u32)((5<<CANFD8_BITTIME_TSeg2_Pos)+(12<<CANFD8_BITTIME_TSeg1_Pos)+(3<<CANFD8_BITTIME_SJW_Pos)+3)

#define		CAN_STD_ID						0
#define		CAN_EXT_ID						1

#define		CAN_RX_MSG_MAX				5

#define		OP_SUCCESS				0
#define		NO_WRITE_AUTH			1
#define		WRITE_OUT_SIZE		2
#define		NO_READ_AUTH			3
#define		READ_OUT_SIZE			4
#define		NO_BOOT_CTRL			1
#define		NO_BOOT_AUTH			2
#define		CODE_OUT_SIZE			8
#define		HW_VERSION_ERR		11
#define		CHIP_ERR					10

#define	  REPLY_NUM_MAX			176//22*8=176   22*6=132   132/4 = 33

typedef struct{
	u32 ID;
	u8	ID_TYPE;
	u8 	OBJ_ID;
	u8 	OT_CNT;
	u8 	RX_ERR_CNT;
	u8 	DLC;
	u8 	DATA[8];
}can_msg_type;

extern void can_config(void), can_msg_receive(void), can_msg_process(void);
#endif
