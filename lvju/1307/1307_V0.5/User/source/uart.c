#include "main.h"
#include "init.h"
#include "soc.h"
#include "flash.h"
#include "toolfun.h"
#include "hwparam.h"
#include "userfun.h"


int testcnt=0;

test_data_type RecordData;
u8 Uart0TxArray[TX_NUM],Uart2TxArray[TX_NUM];
const u8 BootCmd[BOOT_CMD_LEN] = "ENTER_ERT_BOOTLOADER";


uart_data_type Uart0Msg,Uart2Msg;

void data_record_process(void)
{
	RecordData.DATA[0] = Soc.SHOW;
	RecordData.DATA[1] = Soc.INDEX;
	RecordData.DATA[2] = Soc.ZERO;
	RecordData.DATA[3] = Soc.FULL;
	RecordData.DATA[4] = Cap.CAP_GUSS_LAST;
	RecordData.DATA[5] = Cap.QNOW;
	RecordData.DATA[6] = Cap.CAP_DIS_ONCE;
	RecordData.DATA[7] = Cap.CAP_CHG_ONCE;
	RecordData.DATA[8] = Cap.CAP_GUSS_TOTAL_DIS;
	RecordData.DATA[9] = Soe.DIS_WH_ONCE;
	RecordData.DATA[10] = Soe.CHG_WH_ONCE;
	RecordData.DATA[11] = Soe.SOE;
	RecordData.DATA[12] = BlockParam.AVG_VOLTAGE;
	RecordData.DATA[13] = BlockParam.AVG_CURRENT;
	RecordData.DATA[14] = Cap.CAP_SET;
	RecordData.DATA[15] = Cap.SET_IMPACT;
}

void uart_msg_receive(uart_data_type* Msg, UART_Type* UARTx)
{
	u8 TempU8;
	static u8 RxXor;
	static u16 RxSum;
	if (UARTx->STS_b.RX_DATA_NEMP)
	{
		TempU8 = (u8)(UARTx->RX_DATA&0xFF);
		
		if (Msg->RXD == 0 && Msg->TXING == 0)
		{
			
			switch(Msg->STATE++)
			{
				case 1:
					if (TempU8 == FRAME_HEAD_H)
					{
						Msg->STATE 		= 3;
						Msg->EH_FRAME = 1;
					}
					else if (TempU8 != FRAME_HEAD_R)
					{
						Msg->STATE 		= 0;
					}
					break;
				case 2:
					Msg->EH_FRAME = 0;
					if (TempU8 != FRAME_HEAD_T)
					{
						Msg->STATE 		= 0;
					}
					break;
				case 3:
					if (TempU8 >= RX_LEN || TempU8 == 0 )
					{
						Msg->STATE = 0;
					}
					else
					{
						Msg->SUM = 0;
						Msg->LEN = TempU8;
					}
					break;
				case 4:
					Msg->CMD = TempU8;
					Msg->INDEX = 0;
					break;
				case 5:
					Msg->DATA[Msg->INDEX++] = TempU8;
					if (Msg->INDEX < Msg->LEN)
					{
						Msg->STATE = 5;
					}
					break;
				case 6:
					Msg->SUM = ~Msg->SUM;
					if (TempU8 == (Msg->SUM&0xFF))
					{
						Msg->SUM = ~Msg->SUM;
						Msg->SUM -= TempU8;
					}
					else
					{
						Msg->STATE = 0;
					}
					break;
				case 7:
					Msg->SUM = ~Msg->SUM;
					if (TempU8 == (Msg->SUM>>8))
					{
						Msg->RXD = 1;
						Msg->CST_FRAME = 0;
						
						if(UARTx != UART2)
						{
							Uart0Msg.UART_EN_CLR = -1000; 
							GPIO1->ALT1_b.P14 = GPIO_b_UART; 
							SystemFlg.UART0_EN =1;
						}
						
					}
					Msg->STATE = 0;
					break;
					 
					
					
				default:
					if (TempU8 == FRAME_HEAD_E)
					{
						Msg->STATE = 1;
					}
					else
					{
						Msg->STATE = 0;
					}
					 
					break;
			}
		
			Msg->SUM += TempU8;
			RxSum += TempU8; 
			Msg->RX_TO_CNT = 0;
			SystemTick.SLEEP_CNT = 0;
		}
		else
		{
			Msg->STATE = 0;
		}
	}
	else if (Msg->RX_TO_CNT < 250)
	{
		Msg->RX_TO_CNT++;
	}
	else
	{
		Msg->STATE = 0;
	}
	if (UART0 == UARTx)
	{
		TempU8 = (DMA->INTF_b.TC0)?1:0;
	}
	else
	{
		TempU8 = (DMA->INTF_b.TC6)?1:0;
	}
	if (Msg->TXING && TempU8 && UARTx->INTF_b.TX)
	{
		Msg->TXING 		= 0;
		UARTx->INTFC_b.TX = 1;
		if (UART2 == UARTx)
		{
			Rs485Rx_En();
		}
	}
}

 
 
void uart_receive(void)
{
	uart_msg_receive(&Uart0Msg,UART0);	
	uart_msg_receive(&Uart2Msg,UART2); 
}




void set_uart_txarray(u8*data,u8 len,u8 cmd, UART_Type* UARTx)
{
	u16 check_sum,i;
	if(UARTx == UART2)
	{
		if (Uart2Msg.TXING == 0 && len < TX_NUM - 7)
		{	 
		
			if (Uart2Msg.CST_FRAME == 0)
			{			
				Rs485Tx_En();
				Uart2TxArray[0] = FRAME_HEAD_E;
				if (Uart2Msg.EH_FRAME)
				{
					i = 2;
					Uart2TxArray[1] = FRAME_HEAD_H;
				}
				else
				{
					i = 3;
					Uart2TxArray[1] = FRAME_HEAD_R;
					Uart2TxArray[2] = FRAME_HEAD_T;
				}
				Uart2TxArray[i++] = len;
				Uart2TxArray[i++] = cmd;
				check_sum = len + cmd;
				while(len--)
				{
					Uart2TxArray[i++] = *data;
					check_sum += *data++;
				}
				check_sum = ~check_sum;
				Uart2TxArray[i++] = check_sum&0xff;
				Uart2TxArray[i++] = check_sum>>8;
				
				UART2->INTFC_b.TX	= 1;
				DMA_Ch6->CTRL_b.E = 0;
				DMA_Ch6->DTN			= i;
				DMA->INTFC_b.TC6 	= 1;
				DMA_Ch6->CTRL_b.E = 1;
				UART2->CTRL_b.TX_DMA_E 	= 1;
				Uart2Msg.TXING					= 1;
			}					
		}
	}
	else if (Uart0Msg.TXING == 0 && len < TX_NUM - 7)
	{
		if (Uart0Msg.CST_FRAME == 0)
		{
			Uart0TxArray[0] = FRAME_HEAD_E;
			if (Uart0Msg.EH_FRAME)
			{
				i = 2;
				Uart0TxArray[1] = FRAME_HEAD_H;
			}
			else
			{
				i = 3;
				Uart0TxArray[1] = FRAME_HEAD_R;
				Uart0TxArray[2] = FRAME_HEAD_T;
			}
			Uart0TxArray[i++] = len;
			Uart0TxArray[i++] = cmd;
			check_sum = len + cmd;
			while(len--)
			{
				Uart0TxArray[i++] = *data;
				check_sum += *data++;
			}
			check_sum = ~check_sum;
			Uart0TxArray[i++] = check_sum&0xff;
			Uart0TxArray[i++] = check_sum>>8;
			UART0->INTFC_b.TX	= 1;
			DMA_Ch0->CTRL_b.E = 0;
			DMA_Ch0->DTN			= i;
			DMA->INTFC_b.TC0 	= 1;
			DMA_Ch0->CTRL_b.E = 1;
			UART0->CTRL_b.TX_DMA_E 	= 1;
			Uart0Msg.TXING					= 1;
		}
	}		
}



void uart_msg_handle(uart_data_type* Msg, UART_Type* UARTx)
{
	u32 TempU32;
	u16 TempU16;
	u8 reply[100],*ptr;
	
	if(Msg->RXD==1&&Msg->CST_FRAME==2)
	{ 
		Msg->RXD = 0; 
		SystemTick.SLEEP_CNT = 0; 
		Msg->UART_EN_CLR 			= 0;
	}
	else if(Msg->RXD==1 &&Msg->CST_FRAME==3)
	{ 
		Msg->RXD = 0; 
		SystemTick.SLEEP_CNT = 0; 
		Msg->UART_EN_CLR 			= 0;
	}
	else if (Msg->RXD)
	{ 
		switch(Msg->CMD)
		{
			case 0xA1:
				TempU32 = Msg->DATA[0];
				TempU32 += (u32)Msg->DATA[1]<<8;
				TempU32 += (u32)Msg->DATA[1]<<16;
				TempU32 += (u32)Msg->DATA[1]<<24;
				if ((TempU32 > 0x01000000 && TempU32 < 0x0100F000) || (TempU32 > 0x20000000 && TempU32 < 0x20003000))
				{
					reply[0] = *(u8*)TempU32;
				}
				set_uart_txarray(reply,1,0xA1,UARTx);
				break;
			case 0xA0:
				if (Msg->DATA[0] && Msg->LEN == CST_PN_NUMBER + 1)
				{
					for(TempU16 = 0; TempU16 < CST_PN_NUMBER; TempU16++)
					{
						PN.CST[TempU16] = Msg->DATA[TempU16 + 1];
					}
					pn_save();
					ptr = PN.CST;
					TempU16 = CST_PN_NUMBER;
				}
				else if (Msg->LEN == ERT_PN_NUMBER + 1)
				{
					for(TempU16 = 0; TempU16 < ERT_PN_NUMBER; TempU16++)
					{
						PN.ERT[TempU16] = Msg->DATA[TempU16 + 1];
					}
					uid_save();
					pn_save();
					SystemFlg.TESTERING = 1;
					SystemFlg.SOC_SAVE	= 1;
					ptr = PN.ERT;
					TempU16 = ERT_PN_NUMBER;
				}
				else
				{
					TempU16 = 1;
					reply[0] = NO_WRITE_AUTH;
					ptr = reply;
				}
				set_uart_txarray(ptr,TempU16,0xA0,UARTx);
				break;
			case 0xA7:
				TempU16 = Msg->DATA[1]<<8;
				TempU16 += Msg->DATA[0];
				TempU16 = math_diveder(CURRENT_UNIT*TempU16,10);
				KValue.CURRENT = math_diveder((s32)K_VALUE_DEFAULT*TempU16,(abs(KeyRawParam.AVG_CURRENT)));
				if (k_value_verify(&KValue.CURRENT))
				{
					reply[0] = WRITE_OUT_SIZE;
				}
				else
				{
					reply[0] = OP_SUCCESS;
					current_kvalue_insert(KeyRawParam.AVG_CURRENT,KValue.CURRENT);
					SystemFlg.ERT_SAVE = 1;
					
				}
				SystemFlg.TESTERING = 1;
				set_uart_txarray(reply,1,0xA7,UARTx);
				break;
			case 0xA8:
				TempU16 = Msg->DATA[1]<<8;
				TempU16 += Msg->DATA[0];
				TempU16 = math_diveder(VOLTAGE_UNIT*TempU16,10);
				KValue.VOLTAGE = math_diveder((s32)K_VALUE_DEFAULT*TempU16,(abs(KeyRawParam.VOLTAGE)));
				if (k_value_verify(&KValue.VOLTAGE))
				{
					reply[0] = WRITE_OUT_SIZE;
				}
				else
				{
					reply[0] = OP_SUCCESS;
					SystemFlg.ERT_SAVE = 1;
				}
				SystemFlg.TESTERING = 1;
				set_uart_txarray(reply,1,0xA8,UARTx);
				break;
			case 0xA9:
				reply[0] = OP_SUCCESS;
				if (Msg->DATA[0] == 0x5A)
				{
					if (Msg->DATA[1] >= CELL_NUM_MIN && Msg->DATA[1] <= CELL_NUM_MAX && 
							Msg->DATA[2] > CAP_RATING_MIN && Msg->DATA[2] < CAP_RATING_MAX)
					{
						soc_reset(Msg->DATA[2],Msg->DATA[1],1);
					}
					else
					{
						soc_reset(CAP_RATING_DEFAULT,CELL_NUM_DEFAULT,0);
					}
					
					current_offset_calc(&BValue.OPA_REF);
					if (b_value_verify(&BValue.OPA_REF,OPA_OFFSET_DEFAULT+ADC_VALUE(0.5),OPA_OFFSET_DEFAULT-ADC_VALUE(0.5), OPA_OFFSET_DEFAULT))
					{
						reply[0] = WRITE_OUT_SIZE;
					}
					else
					{
						SystemFlg.ERT_SAVE = 1;
					}
				}
				else
				{
					reply[0] = NO_WRITE_AUTH;
				}
				set_uart_txarray(reply,1,0xA9,UARTx);
				break;
			case 0x5D:
				if (Msg->DATA[0])
				{
					set_uart_txarray((u8*)&PN.CST,CST_PN_NUMBER,0xDD,UARTx);
				}
				else
				{
					set_uart_txarray((u8*)&PN.ERT,ERT_PN_NUMBER,0xDD,UARTx);
				}
				break;
			case 0x50:
				reply[0] = CHIP_TYPE;
				set_uart_txarray(reply,1,0xD0,UARTx);
				break;
			case 0x51:
				extern int meter_signal;
				reply[2] 	= (meter_signal)?0x20:0;  //AccIOIsValid
				reply[2] |= (CrgIOIsValid)?0x40:0; 
			  
			
			
				reply[1]	= 0;
				reply[0]	= 0;
				set_uart_txarray(reply,Msg->DATA[0],0xD1,UARTx);
				break;
			case 0x52:
				set_uart_txarray((u8*)&SystemFlg,Msg->DATA[0],0xD2,UARTx);
				break;
			case 0x57:
				set_uart_txarray((u8*)&BValue,Msg->DATA[0],0xD7,UARTx);
				break;
			case 0x58:
				set_uart_txarray((u8*)&KValue,Msg->DATA[0],0xD8,UARTx);
				break;
			case 0x5B:
				set_uart_txarray((u8*)&Version,Msg->DATA[0],0xDB,UARTx);
				break;
			case 0x5E:
				set_uart_txarray((u8*)&AdcResult,Msg->DATA[0],0xDE,UARTx);
				break;
			case 0x5F:
				set_uart_txarray((u8*)&AdcAvg,Msg->DATA[0],0xDF,UARTx);
				break;
			case 0x1A:
				set_uart_txarray((u8*)&RecordData,Msg->DATA[0],0x9A,UARTx);
				break;
			case 0x1C:
				goto_sleep1();
				break;
			case 0x13:
				set_uart_txarray((u8*)&Marks,Msg->DATA[0],0x93,UARTx);
				break;
			case 0x14:
				set_uart_txarray((u8*)&Soc,Msg->DATA[0],0x94,UARTx);
				break;
			case 0x15:
				set_uart_txarray((u8*)&Cap,Msg->DATA[0],0x95,UARTx);
				break;
			case 0x16:
				set_uart_txarray((u8*)&SystemParam,Msg->DATA[0],0x96,UARTx);
				break;
			case 0x17:
				set_uart_txarray((u8*)&Bat,Msg->DATA[0],0x97,UARTx);
				break;
			case 0x18:
				set_uart_txarray((u8*)&His,Msg->DATA[0],0x98,UARTx);
				break;
			case 0x19:
				set_uart_txarray((u8*)&Integral,Msg->DATA[0],0x99,UARTx);
				break;
			case BOOTLOAD_CMD:
				for (TempU16= 0; TempU16 < BOOT_CMD_LEN; TempU16++)
				{
					if (BootCmd[TempU16] != Msg->DATA[TempU16])
						break;
				}
				
				if (TempU16 == BOOT_CMD_LEN)
				{
					TempU16 = Msg->DATA[BOOT_CMD_LEN];
					TempU16 += (u16)Msg->DATA[BOOT_CMD_LEN + 1]<<8;
					if (*(u32*)0x02000000 == FLASH_DEFAULT)
					{
						reply[0] = NO_BOOT_CTRL;
					}
					else if (Msg->DATA[BOOT_CMD_LEN + 2] != HW_VERSION)
					{
						reply[0] = HW_VERSION_ERR;
					}
					else if (Msg->DATA[BOOT_CMD_LEN + 3] != CHIP_TYPE)
					{
						reply[0] = CHIP_ERR;
					}
					else
					{
						if (Msg->LEN > BOOT_CMD_LEN+7)
						{
							TempU32 = Msg->DATA[BOOT_CMD_LEN+4];
							TempU32 += (u32)Msg->DATA[BOOT_CMD_LEN+5]<<8;
							TempU32 += (u32)Msg->DATA[BOOT_CMD_LEN+6]<<16;
							TempU32 += (u32)Msg->DATA[BOOT_CMD_LEN+7]<<24;
							if (TempU32&0xFFE0E000)
							{
								TempU32 = 0;
							}
						}
						else
						{
							TempU32 = 0;
						}
						
						SystemFlg.ERT_SAVE = 1;
						ert_save(1,UARTx, 0, TempU32);
						if (SystemFlg.ERT_SAVE)
						{
							reply[0] = NO_BOOT_AUTH;
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
				}
				else
				{
					reply[0] = NO_BOOT_AUTH;
				}
				set_uart_txarray(reply,1,0xAA,UARTx);
				break;
		}
		Msg->RXD = 0;
		SystemTick.SLEEP_CNT 	= 0; 
	
	}
	else if (Msg->TXING)
	{
		if (++Msg->TXING >= 250)
		{
			Msg->TXING = 0;
			if (UARTx == UART2)
			{
				Rs485Rx_En();
			}
		}
		else if (UARTx == UART2)
		{
			if (Msg->TXING == 6)
			{
				UART2->CTRL_b.TX_DMA_E 	= 1;
			}
			else if (Msg->TXING == 5)
			{
				Rs485Tx_En();
			}
		}
	}
	else if (Rs485Txing && UARTx == UART2)
	{
		Rs485Rx_En();
	}
	#ifndef GP_DEBUG
	else if (SystemFlg.UART0_EN == 0 && SystemParam.BUS_VOLTAGE > VOLTAGE_V(30) && SystemParam.BUS_VOLTAGE < VOLTAGE_V(150))
	{
		
	}
	#endif
	
	
}

void uart_msg_process(void)//10ms
{	
	
	if (SystemFlg.UART0_EN)
	{
		uart_msg_handle(&Uart0Msg,UART0);
		 
		if (Uart0Msg.UART_EN_CLR < 500)//2.5s
		{
			Uart0Msg.UART_EN_CLR++;
		}
		else
		{
			SystemFlg.UART0_EN = 0;
			GPIO1->ALT1	= 0;
			//UART0->CTRL = 0;
		}
		 
	}	
	uart_msg_handle(&Uart2Msg,UART2);
}




