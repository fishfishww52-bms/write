#include "main.h"
#include "can.h"
#include "flash.h"
#include "soc.h"
#include "toolfun.h"
#include "hwparam.h"
#include "userfun.h"

u8 Can_Rx_Index, Can_Msg_Index;

void can_config(void)
{
	CANFD80->CTRL 		= CANFD8_CTRL_Init_EN | CANFD8_CTRL_CCE_EN;
	CANFD80->BITTIME 	= CAN_BIT_TIME;
	CANFD80->BRPEXTN	= 0;
	CANFD80->CTRL 	 &= ~(CANFD8_CTRL_Init_EN | CANFD8_CTRL_CCE_EN);
	while (CANFD80->CTRL & CANFD8_CTRL_Init_EN);
	//do something mask for receive, do not need to receive useless message
	
	#ifdef CAN_LOOP_BACK_TEST
	CANFD80->CTRL |= CANFD8_CTRL_TEST_EN;
	CANFD80->TEST |= CANFD8_TEST_LBack_EN;
	#endif
	
	CANFD80->IF2_CMD_MSK = 	CANFD8_IF2_CMD_MSK_WR_RD_Write
												| CANFD8_IF2_CMD_MSK_Mask_Transmit
                        | CANFD8_IF2_CMD_MSK_Arb_Transmit
                        | CANFD8_IF2_CMD_MSK_Control_Transmit
                        | CANFD8_IF2_CMD_MSK_DataA_Transmit
                        | CANFD8_IF2_CMD_MSK_DataB_Transmit;
	
	CANFD80->IF2_ARBIT2 	= (0x000<<2) | CANFD8_IF2_ARBIT2_MsgVal_Cfg | CANFD8_IF2_ARBIT2_Xtd_11BIT;
	
	CANFD80->IF2_MSG_CTRL = CANFD8_IF2_MSG_CTRL_RxIE_SET
												| CANFD8_IF2_MSG_CTRL_DLC_Leng8
												| CANFD8_IF2_MSG_CTRL_EoB_End;
		
	CANFD80->IF2_CMD_REQ = CAN_RX_MSG_OBJ_0;
	
	Can_Rx_Index 	= 0;
	Can_Msg_Index = 0;
}

can_msg_type CAN_RxMsg[CAN_RX_MSG_MAX];

void can_msg_receive(void)
{	
	if(CANFD80->STS & CANFD8_STS_TxOk_Msk)
	{
		CANFD80->STS &= ~CANFD8_STS_TxOk_Msk;
	}
	
	if (0 == (CANFD80->IF2_CMD_REQ & CANFD8_IF2_CMD_REQ_Busy_SetReg))
	{
		if (CAN_RxMsg[Can_Rx_Index].OBJ_ID)
		{
			if((CANFD80->IF2_ARBIT2 & CANFD8_IF2_ARBIT2_Xtd_29BIT) == 0)
			{				
				CAN_RxMsg[Can_Rx_Index].ID 	= (CANFD80->IF2_ARBIT2 & CANFD8_IF1_ARBIT2_ID28_16_Msk) >> 2;
				CAN_RxMsg[Can_Rx_Index].ID_TYPE = CAN_STD_ID;
			}
			else
			{
				CAN_RxMsg[Can_Rx_Index].ID 	= (((CANFD80->IF2_ARBIT2 ) & 0x1FFF) << 16) | CANFD80->IF2_ARBIT1;
				CAN_RxMsg[Can_Rx_Index].ID_TYPE = CAN_EXT_ID;
			}
			CAN_RxMsg[Can_Rx_Index].DLC =	 CANFD80->IF2_MSG_CTRL & CANFD8_IF2_MSG_CTRL_DLC_Msk;
			
			CAN_RxMsg[Can_Rx_Index].DATA[0] = (CANFD80->IF2_DATA_A1 & CANFD8_IF2_DATA_A1_Data0_Msk);
			CAN_RxMsg[Can_Rx_Index].DATA[1] = (CANFD80->IF2_DATA_A1 & CANFD8_IF2_DATA_A1_Data1_Msk) >> CANFD8_IF2_DATA_A1_Data1_Pos;
			CAN_RxMsg[Can_Rx_Index].DATA[2] = (CANFD80->IF2_DATA_A2 & CANFD8_IF2_DATA_A2_Data2_Msk);
			CAN_RxMsg[Can_Rx_Index].DATA[3] = (CANFD80->IF2_DATA_A2 & CANFD8_IF2_DATA_A2_Data3_Msk) >> CANFD8_IF2_DATA_A2_Data3_Pos;
			CAN_RxMsg[Can_Rx_Index].DATA[4] = (CANFD80->IF2_DATA_B1 & CANFD8_IF2_DATA_B1_Data4_Msk);
			CAN_RxMsg[Can_Rx_Index].DATA[5] = (CANFD80->IF2_DATA_B1 & CANFD8_IF2_DATA_B1_Data5_Msk) >> CANFD8_IF2_DATA_B1_Data5_Pos;
			CAN_RxMsg[Can_Rx_Index].DATA[6] = (CANFD80->IF2_DATA_B2 & CANFD8_IF2_DATA_B2_Data6_Msk);
			CAN_RxMsg[Can_Rx_Index].DATA[7] = (CANFD80->IF2_DATA_B2 & CANFD8_IF2_DATA_B2_Data7_Msk) >> CANFD8_IF2_DATA_B1_Data5_Pos;
			
			CANFD80->IF2_CMD_MSK = CANFD8_IF2_CMD_MSK_ClrIntPnd_Clear | CANFD8_IF2_CMD_MSK_TxRqst_NewDat_Set;
			CANFD80->IF2_CMD_REQ = CAN_RxMsg[Can_Rx_Index].OBJ_ID;
			CAN_RxMsg[Can_Rx_Index].OBJ_ID  	 = 0;
			
			if (++Can_Rx_Index >= CAN_RX_MSG_MAX)
			{
				Can_Rx_Index = 0;
			}
		}
	}
	else if (CAN_RxMsg[Can_Rx_Index].OBJ_ID)
	{
		if (++CAN_RxMsg[Can_Rx_Index].OT_CNT >= 100)
		{
			CAN_RxMsg[Can_Rx_Index].OBJ_ID = 0;
			CAN_RxMsg[Can_Rx_Index].RX_ERR_CNT++;
		}
	}
	
	if (CANFD80->INT == 0x8000)
	{
		if(CANFD80->STS & CANFD8_STS_RxOk_Msk)
		{
			CANFD80->STS &= ~CANFD8_STS_RxOk_Msk;
		}
	}
	else
	{		
		if (CANFD80->NEW_DATA_1 || CANFD80->NEW_DATA_2)
		{
			CANFD80->STS &= ~CANFD8_STS_RxOk_Msk;
			
			if (Can_Rx_Index >= CAN_RX_MSG_MAX)
			{
				Can_Rx_Index = 0;
			}
			
			if (CANFD80->NEW_DATA_1&(1<<(CAN_RX_MSG_OBJ_0 - 1)))
			{
				
				CANFD80->IF2_CMD_MSK = CANFD8_IF2_CMD_MSK_WR_RD_Read
													| CANFD8_IF2_CMD_MSK_Mask_Transmit
													| CANFD8_IF2_CMD_MSK_Arb_Transmit
													| CANFD8_IF2_CMD_MSK_Control_Transmit
													| CANFD8_IF2_CMD_MSK_ClrIntPnd_Clear
													|	CANFD8_IF2_CMD_MSK_TxRqst_NewDat_Set
													| CANFD8_IF2_CMD_MSK_DataA_Transmit
													| CANFD8_IF2_CMD_MSK_DataB_Transmit;
				CANFD80->IF2_CMD_REQ 								= CAN_RX_MSG_OBJ_0;
				CAN_RxMsg[Can_Rx_Index].OBJ_ID		 	= CAN_RX_MSG_OBJ_0;
				CAN_RxMsg[Can_Rx_Index].OT_CNT			= 0;
			}
		}
	}
}



void set_can_tx_boxes(u8 len, u32 id, u8 id_type, u8* data)
{
	u8 i;
		
	for (i = CAN_TX_MSG_OBJ_START; i < CAN_TX_MSG_OBJ_END; i++)
	{
		while(CANFD80->IF1_CMD_REQ & CANFD8_IF1_CMD_REQ_Busy_SetReg)
		{
			if (SystemTick.TICK_1ms > 16)
			{
				return;
			}
		}
		if ((CANFD80->TRANS_REQ_2&(1<<(i - CAN_TX_MSG_OBJ_START))) == 0)
		{
			CANFD80->IF1_CMD_MSK 	= CANFD8_IF1_CMD_MSK_WR_RD_Write
                          | CANFD8_IF1_CMD_MSK_Mask_Transmit
                          | CANFD8_IF1_CMD_MSK_Arb_Transmit
                          | CANFD8_IF1_CMD_MSK_Control_Transmit
                          | CANFD8_IF1_CMD_MSK_TxRqst_NewDat_Set
													| CANFD8_IF1_CMD_MSK_DataA_Transmit
													| CANFD8_IF1_CMD_MSK_DataB_Transmit;
			
			if (id_type == CAN_STD_ID)
			{
				CANFD80->IF1_ARBIT1 = 0;
				CANFD80->IF1_ARBIT2 = ((id & 0x7FF) << 2)
												| CANFD8_IF1_ARBIT2_Dir_TX
                        | CANFD8_IF1_ARBIT2_MsgVal_Cfg
                        | CANFD8_IF1_ARBIT2_Xtd_11BIT;
			}
			else
			{
				CANFD80->IF1_ARBIT1 = id&0xFFFF;
				CANFD80->IF1_ARBIT2 = ((id & 0x1FFF0000) >> 16)
												| CANFD8_IF1_ARBIT2_Dir_TX
                        | CANFD8_IF1_ARBIT2_MsgVal_Cfg
                        | CANFD8_IF1_ARBIT2_Xtd_29BIT;
			}
			
			CANFD80->IF1_DATA_A1 = *data++;
			CANFD80->IF1_DATA_A1 += (u16)(*data++)<<8;
			CANFD80->IF1_DATA_A2 = *data++;
			CANFD80->IF1_DATA_A2 += (u16)(*data++)<<8;
			CANFD80->IF1_DATA_B1 = *data++;
			CANFD80->IF1_DATA_B1 += (u16)(*data++)<<8;
			CANFD80->IF1_DATA_B2 = *data++;
			CANFD80->IF1_DATA_B2 += (u16)(*data++)<<8;
			
			CANFD80->IF1_MSG_CTRL = CANFD8_IF1_MSG_CTRL_NewDat_Data
														| CANFD8_IF1_MSG_CTRL_EoB_End
														| CANFD8_IF1_MSG_CTRL_DLC_Leng8;
			
			CANFD80->IF1_CMD_REQ = i;
			SystemTick.TICK_1ms  = 0;
			
			if (len > 8)
			{
				len -= 8;
			}
			else
			{
				break;
			}
		}
	}
}

u8 insert_data_to_reply(u8 *src, u8* dst, u8 len, u8 id)
{
	u8 i = 0, idx;
	
	while(len)
	{
		*dst++ = id;
		*dst++ = (len <= 6)?0xFF:(i>>3);
		i 	+= 2;
		idx = i + 6;
		for (; i < idx && len && i < REPLY_NUM_MAX; i++)
		{
			*dst++ = *src++;
			len--;
		}
		
		for (;i < idx && i < REPLY_NUM_MAX; i++)
		{
			*dst++ = 0xFF;
		}
	}
	
	return i;
}

void flash_page_send(u32 addr, u16 *index, u8* inc, u16 id)
{
	if ((CANFD80->TRANS_REQ_1&4) == 0 && (CANFD80->IF1_CMD_REQ & CANFD8_IF1_CMD_REQ_Busy_SetReg) == 0)
	{
		CANFD80->IF1_CMD_MSK 	= CANFD8_IF1_CMD_MSK_WR_RD_Write
                          | CANFD8_IF1_CMD_MSK_Mask_Transmit
                          | CANFD8_IF1_CMD_MSK_Arb_Transmit
                          | CANFD8_IF1_CMD_MSK_Control_Transmit
                          | CANFD8_IF1_CMD_MSK_TxRqst_NewDat_Set
													| CANFD8_IF1_CMD_MSK_DataA_Transmit
													| CANFD8_IF1_CMD_MSK_DataB_Transmit;
		CANFD80->IF1_ARBIT1 	= 0;
		CANFD80->IF1_ARBIT2 	= ((0x400 & 0x7FF) << 2)|CANFD8_IF1_ARBIT2_Dir_TX | CANFD8_IF1_ARBIT2_MsgVal_Cfg	| CANFD8_IF1_ARBIT2_Xtd_11BIT;
		CANFD80->IF1_DATA_A1 	= id;
		CANFD80->IF1_DATA_A1 += ((*inc >= 85)?0xFF:(((u16)*inc)))<<8;
		
		CANFD80->IF1_DATA_A2 = (u32)(*(u8*)(addr + (*index)++));
		CANFD80->IF1_DATA_A2 += (u32)(*(u8*)(addr + (*index)++))<<8;
		
		CANFD80->IF1_DATA_B1 = *(u8*)(addr + (*index)++);
		CANFD80->IF1_DATA_B1 += (u16)(*(u8*)(addr + (*index)++))<<8;
		
		CANFD80->IF1_DATA_B2 = (u32)(*(u8*)(addr + (*index)++));
		CANFD80->IF1_DATA_B2 += (u32)(*(u8*)(addr + (*index)++))<<8;
		
		CANFD80->IF1_MSG_CTRL = CANFD8_IF1_MSG_CTRL_NewDat_Data | CANFD8_IF1_MSG_CTRL_EoB_End | CANFD8_IF1_MSG_CTRL_DLC_Leng8;
		CANFD80->IF1_CMD_REQ  = CAN_TX_MSG_OBJ_S;
		
		(*inc)++;
	}
	else
	{
		return;
	}
}


void can_msg_process(void)
{
	extern test_data_type RecordData;
	static u32 flash_page;
	static u8 index = 0, reply_err, reply_index;
	static u16 flash_index, reply_id;
	
	u8 can_reply[REPLY_NUM_MAX], len, *ptr_8, idx;
	u32 TempU32;
	
	if (SystemFlg.RCDSEND)
	{
		flash_page_send(flash_page, &flash_index, &reply_index, reply_id);
		if (flash_index >= 512)
		{
			SystemFlg.RCDSEND = 0;
		}
	}
	
	while (Can_Rx_Index != Can_Msg_Index)
	{
		switch(CAN_RxMsg[Can_Msg_Index].ID)
		{
			case 0:
				if (CAN_STD_ID == CAN_RxMsg[Can_Msg_Index].ID_TYPE)
				{
					can_reply[0] = CAN_RxMsg[Can_Msg_Index].DATA[0];
					switch(CAN_RxMsg[Can_Msg_Index].DATA[0])
					{
						case 0:
							if (CAN_RxMsg[Can_Msg_Index].DATA[1] == 0)
							{
								SystemFlg.TESTERING = 1;
								can_reply[1] = 0;
								set_can_tx_boxes(8,0x400,CAN_STD_ID,can_reply);
							}
							else if (CAN_RxMsg[Can_Msg_Index].DATA[1] == 1)
							{
								goto_sleep();
							}
							else if (CAN_RxMsg[Can_Msg_Index].DATA[1] == 2)
							{
								SystemFlg.TESTERING = 0;
								can_reply[1] = 2;
								set_can_tx_boxes(8,0x400,CAN_STD_ID,can_reply);
							}
							break;
						case 0xA0:
							if (CAN_RxMsg[Can_Msg_Index].DATA[2] == 0)
							{
								index = 0;
								reply_err = 0;
							}
							else if (CAN_RxMsg[Can_Msg_Index].DATA[2] != 0xFF)
							{
								if (CAN_RxMsg[Can_Msg_Index].DATA[2] * 5 != index)
								{
									reply_err = 1;
								}
							}
							can_reply[1] = CAN_RxMsg[Can_Msg_Index].DATA[1];
							if (CAN_RxMsg[Can_Msg_Index].DATA[1] == 0)
							{
								for (len = 3; len < 8 && index < ERT_PN_NUMBER; len++)
								{
									PN.ERT[index++] = CAN_RxMsg[Can_Msg_Index].DATA[len];
								}
								
								if (CAN_RxMsg[Can_Msg_Index].DATA[2] == 0xFF)
								{
									uid_save();
									pn_save();
									can_reply[2] = reply_err;
									set_can_tx_boxes(8,0x400,CAN_STD_ID,can_reply);
									index = 0;
									reply_err = 0;
								}
							}
							else if (CAN_RxMsg[Can_Msg_Index].DATA[1] == 1)
							{
								for (len = 3; len < 8 && index < CST_PN_NUMBER; len++)
								{
									PN.CST[index++] = CAN_RxMsg[Can_Msg_Index].DATA[len];
								}
								
								if (CAN_RxMsg[Can_Msg_Index].DATA[2] == 0xFF)
								{
									pn_save();
									can_reply[2] = reply_err;
									set_can_tx_boxes(8,0x400,CAN_STD_ID,can_reply);
									index = 0;
									reply_err = 0;
								}
							}
							break;
						case 0xA1:
							TempU32 = CAN_RxMsg[Can_Msg_Index].DATA[1];
							TempU32 += (u32)CAN_RxMsg[Can_Msg_Index].DATA[2]<<8;
							TempU32 += (u32)CAN_RxMsg[Can_Msg_Index].DATA[3]<<16;
							TempU32 += (u32)CAN_RxMsg[Can_Msg_Index].DATA[4]<<24;
							
							can_reply[5] = *(u8*)TempU32;
							TempU32 = 0x400;
							for (len = 1; len < 5; len++)
							{
								can_reply[len] =  CAN_RxMsg[Can_Msg_Index].DATA[len];
							}
							set_can_tx_boxes(8,0x400,CAN_STD_ID,can_reply);
							break;
						case 0xA6:
							if (SystemFlg.RCDSEND == 0)
							{
								SystemFlg.RCDSEND = 1;
								flash_index 			= 0;
								flash_page				= RCD_DATA_ADDR;
								reply_index				= 0;
								reply_id					= 0xA6;
							}
							break;
						case 0xA7:
							TempU32 = CAN_RxMsg[Can_Msg_Index].DATA[1];
							TempU32 += (u32)CAN_RxMsg[Can_Msg_Index].DATA[2]<<8;
							can_reply[1] = OP_SUCCESS;
							can_reply[2] = CAN_RxMsg[Can_Msg_Index].DATA[3];
							if (CAN_RxMsg[Can_Msg_Index].DATA[3] == 0)
							{
								if (SystemParam.BUS_VOLTAGE > VOLTAGE_V(50) && KeyRawParam.VOLTAGE)
								{
									TempU32 = math_diveder(VOLTAGE_UNIT*TempU32,10);
									KValue.VOLTAGE = math_diveder((s32)K_VALUE_DEFAULT*TempU32,(abs(KeyRawParam.VOLTAGE)));
									if (k_value_verify(&KValue.VOLTAGE))
									{
										can_reply[1] = WRITE_OUT_SIZE;
									}
									else
									{
										SystemFlg.ERT_SAVE = 1;
									}
								}
								else
								{
									can_reply[1] = NO_WRITE_AUTH;
								}
							}
							else if (KeyRawParam.AVG_CURRENT)
							{
								TempU32 = math_diveder(CURRENT_UNIT*TempU32,10);
								KValue.CURRENT = math_diveder((s32)K_VALUE_DEFAULT*TempU32,(abs(KeyRawParam.AVG_CURRENT)));
								if (k_value_verify(&KValue.CURRENT))
								{
									can_reply[1] = WRITE_OUT_SIZE;
								}
								else
								{
									current_kvalue_insert(KeyRawParam.AVG_CURRENT,KValue.CURRENT);
									SystemFlg.ERT_SAVE = 1;
								}
							}
							else
							{
								can_reply[1] = NO_WRITE_AUTH;
							}
							set_can_tx_boxes(8,0x400,CAN_STD_ID,can_reply);
							break;
						case 0xA9:
							if (CAN_RxMsg[Can_Msg_Index].DATA[1] == 0x5A)
							{
								if (CAN_RxMsg[Can_Msg_Index].DATA[2] >= CELL_NUM_MIN && CAN_RxMsg[Can_Msg_Index].DATA[2] <= CELL_NUM_MAX && 
										CAN_RxMsg[Can_Msg_Index].DATA[3] >= CAP_RATING_MIN && CAN_RxMsg[Can_Msg_Index].DATA[3] < CAP_RATING_MAX)
								{
									soc_reset(CAN_RxMsg[Can_Msg_Index].DATA[3],CAN_RxMsg[Can_Msg_Index].DATA[2],1);
								}
								else
								{
									soc_reset(CAP_RATING_DEFAULT,CELL_NUM_DEFAULT,0);
								}
								
								current_offset_calc(&BValue.OPA_REF);
								if (b_value_verify(&BValue.OPA_REF,OPA_OFFSET_DEFAULT + ADC_VALUE(0.5),OPA_OFFSET_DEFAULT - ADC_VALUE(0.5), OPA_OFFSET_DEFAULT))
								{
									can_reply[1] = WRITE_OUT_SIZE;
								}
								else
								{
									SystemFlg.ERT_SAVE = 1;
									can_reply[1] = OP_SUCCESS;
								}
							}
							else
							{
								can_reply[1] = NO_WRITE_AUTH;
							}
							set_can_tx_boxes(8,0x400,CAN_STD_ID,can_reply);
							break;
						case 0x5D:
							idx = 1;
							if (CAN_RxMsg[Can_Msg_Index].DATA[1] == 0)
							{								
								ptr_8 = (u8*)&PN.ERT;
								idx = 0;
							}
							else if (CAN_RxMsg[Can_Msg_Index].DATA[1] == 1)
							{
								ptr_8 = (u8*)&PN.CST;
								idx = 0;
							}
							if (idx == 0)
							{
								len = insert_data_to_reply(ptr_8,can_reply,CAN_RxMsg[Can_Msg_Index].DATA[2],CAN_RxMsg[Can_Msg_Index].DATA[0]);
								set_can_tx_boxes(len,0x400,CAN_STD_ID,can_reply);
							}
							break;
						case 0x50:
							can_reply[1] = CHIP_TYPE;
							set_can_tx_boxes(8,0x400,CAN_STD_ID,can_reply);
							break;
						case 0x51:
							for (len = 1; len < 8; len++)
							{
								can_reply[len] = 0;
							}
							can_reply[3] = AccIOIsValid?0x20:0;
							can_reply[3] |= CrgIOIsValid?0x40:0;
							set_can_tx_boxes(8,0x400,CAN_STD_ID,can_reply);
							break;
						case 0x52:
							ptr_8 = (u8*)&SystemFlg;
							for (len = 1; len < 5; len++)
							{
								can_reply[len] = *ptr_8++;
							}
							for (len = 5; len < 8; len++)
							{
								can_reply[len] = 0xFF;
							}
							set_can_tx_boxes(8,0x400,CAN_STD_ID,can_reply);
							break;
						case 0x57:
							ptr_8 = (u8*)&BValue;
							len = insert_data_to_reply(ptr_8,can_reply,CAN_RxMsg[Can_Msg_Index].DATA[1],CAN_RxMsg[Can_Msg_Index].DATA[0]);
							set_can_tx_boxes(len,0x400,CAN_STD_ID,can_reply);
							break;
						case 0x58:
							ptr_8 = (u8*)&KValue;
							len = insert_data_to_reply(ptr_8,can_reply,CAN_RxMsg[Can_Msg_Index].DATA[1],CAN_RxMsg[Can_Msg_Index].DATA[0]);
							set_can_tx_boxes(len,0x400,CAN_STD_ID,can_reply);
							break;
						case 0x5B:
							ptr_8 = (u8*)&Version;
							len = insert_data_to_reply(ptr_8,can_reply,CAN_RxMsg[Can_Msg_Index].DATA[1],CAN_RxMsg[Can_Msg_Index].DATA[0]);
							set_can_tx_boxes(len,0x400,CAN_STD_ID,can_reply);
							break;
						case 0x5E:
							ptr_8 = (u8*)&AdcResult;
							len = insert_data_to_reply(ptr_8,can_reply,CAN_RxMsg[Can_Msg_Index].DATA[1],CAN_RxMsg[Can_Msg_Index].DATA[0]);
							set_can_tx_boxes(len,0x400,CAN_STD_ID,can_reply);
							break;
						case 0x5F:
							ptr_8 = (u8*)&AdcAvg;
							len = insert_data_to_reply(ptr_8,can_reply,CAN_RxMsg[Can_Msg_Index].DATA[1],CAN_RxMsg[Can_Msg_Index].DATA[0]);
							set_can_tx_boxes(len,0x400,CAN_STD_ID,can_reply);
							break;
						case 0x1A:
							ptr_8 = (u8*)&RecordData;
							len = insert_data_to_reply(ptr_8,can_reply,CAN_RxMsg[Can_Msg_Index].DATA[1],CAN_RxMsg[Can_Msg_Index].DATA[0]);
							set_can_tx_boxes(len,0x400,CAN_STD_ID,can_reply);
							break;
						case 0x13:
							ptr_8 = (u8*)&Marks;
							can_reply[0] = CAN_RxMsg[Can_Msg_Index].DATA[0];
							for (len = 1; len < 5; len++)
							{
								can_reply[len] = *ptr_8++;
							}
							set_can_tx_boxes(8,0x400,CAN_STD_ID,can_reply);
							break;
						case 0x14:
							ptr_8 = (u8*)&Soc;
							len = insert_data_to_reply(ptr_8,can_reply,CAN_RxMsg[Can_Msg_Index].DATA[1],CAN_RxMsg[Can_Msg_Index].DATA[0]);
							set_can_tx_boxes(len,0x400,CAN_STD_ID,can_reply);
							break;
						case 0x15:
							ptr_8 = (u8*)&Cap;
							len = insert_data_to_reply(ptr_8,can_reply,CAN_RxMsg[Can_Msg_Index].DATA[1],CAN_RxMsg[Can_Msg_Index].DATA[0]);
							set_can_tx_boxes(len,0x400,CAN_STD_ID,can_reply);
							break;
						case 0x16:
							ptr_8 = (u8*)&SystemParam;
							len = insert_data_to_reply(ptr_8,can_reply,CAN_RxMsg[Can_Msg_Index].DATA[1],CAN_RxMsg[Can_Msg_Index].DATA[0]);
							set_can_tx_boxes(len,0x400,CAN_STD_ID,can_reply);
							break;
						case 0x17:
							ptr_8 = (u8*)&Bat;
							len = insert_data_to_reply(ptr_8,can_reply,CAN_RxMsg[Can_Msg_Index].DATA[1],CAN_RxMsg[Can_Msg_Index].DATA[0]);
							set_can_tx_boxes(len,0x400,CAN_STD_ID,can_reply);
							break;
						case 0x18:
							ptr_8 = (u8*)&His;
							len = insert_data_to_reply(ptr_8,can_reply,CAN_RxMsg[Can_Msg_Index].DATA[1],CAN_RxMsg[Can_Msg_Index].DATA[0]);
							set_can_tx_boxes(len,0x400,CAN_STD_ID,can_reply);
							break;
						case 0x19:
							ptr_8 = (u8*)&Integral;
							len = insert_data_to_reply(ptr_8,can_reply,CAN_RxMsg[Can_Msg_Index].DATA[1],CAN_RxMsg[Can_Msg_Index].DATA[0]);
							set_can_tx_boxes(len,0x400,CAN_STD_ID,can_reply);
							break;
						case 0xAA:
							TempU32 = CAN_RxMsg[Can_Msg_Index].DATA[1];
							TempU32 += (u32)CAN_RxMsg[Can_Msg_Index].DATA[2]<<8;
							TempU32 += (u32)CAN_RxMsg[Can_Msg_Index].DATA[3]<<16;
							TempU32 += (u32)CAN_RxMsg[Can_Msg_Index].DATA[4]<<24;
						
							if (CAN_RxMsg[Can_Msg_Index].DATA[7] != 'E')
							{
								can_reply[1] = NO_BOOT_AUTH;
							}
							else if (TempU32 > FLASH_CODE_MAX)
							{
								can_reply[1] = CODE_OUT_SIZE;
							}
							else if (CAN_RxMsg[Can_Msg_Index].DATA[5] != HW_VERSION)
							{
								can_reply[1] = HW_VERSION_ERR;
							}
							else if (CAN_RxMsg[Can_Msg_Index].DATA[6] != CHIP_TYPE)
							{
								can_reply[1] = CHIP_ERR;
							}
							else
							{
								SystemFlg.ERT_SAVE = 1;
							//	ert_save(1,(u32)CANFD80, 0, 0);
								if (SystemFlg.ERT_SAVE)
								{
									can_reply[1] = NO_BOOT_AUTH;
								}
								else
								{
									__disable_irq();
									NVM->FSHKEY = NVM_FSHKEY_1;
									NVM->FSHKEY = NVM_FSHKEY_2;
									MODIFY_REG(NVM->CTRL, NVM_CTRL_BOOT_Msk, NVM_BOOT_SYSTEM);
									WRITE_REG(RCU->STS, RCU->STS);
									NVIC_SystemReset();
								}
							}
							set_can_tx_boxes(8,0x400,CAN_STD_ID,can_reply);
							break;
					}
				}
				break;
		}
		
		SystemTick.SLEEP_CNT 	= 0;
		
		if (++Can_Msg_Index >= CAN_RX_MSG_MAX)
		{
			Can_Msg_Index = 0;
		}
	}
}


