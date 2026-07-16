#include "main.h"
#include "init.h"
#include "soc.h"
#include "flash.h"
#include "toolfun.h"
#include "hwparam.h"
#include "userfun.h"
 


#define MeterIOisHigh   (GPIO1->IDR_b.P09 == 0)
#define MeterIOisLow    (GPIO1->IDR_b.P09 == 1)


#define OneLine_is_Active  (GPIO2->IDR_b.P11 == 1)
#define OneLine_Low()       GPIO1->P14 = 0
#define OneLine_High()      GPIO1->P14 = 1

#define		ChgOutHigh				GPIO1->ODR_b.P14  



a_line_type OneLine;




void getchglft(void)
{
	u32 TempU32; 
	TempU32 = 0;
	if (SocShowAllowIncState)
	{
		if (Soc.SHOW >= SOC_SHOW_MAX)
		{
			TempU32 = 0;
		}
		else if (BlockParam.CHG_CURRENT)
		{
			u16 U16Tmp;
			U16Tmp = Cap.QNOW*CURRENT_UNIT/Q_UNIT;
			if (Soc.INDEX < SOC_INDEX_VALUE(0.8))
			{
				TempU32 = math_diveder((SOC_INDEX_VALUE(0.8) - Soc.INDEX)*U16Tmp,SOC_INDEX_MAX);
				TempU32 = math_diveder(TempU32*60,BlockParam.CHG_CURRENT);
				TempU32 += math_diveder(U16Tmp*24,BlockParam.CHG_CURRENT);
			}
			else if (Soc.SHOW > SOC_SHOW_VALUE(95/100))
			{
				TempU32 = SOC_INDEX_MAX - Soc.INDEX;
			}
			else if (BlockParam.AVG_CURRENT)
			{
				TempU32 = math_diveder((SOC_INDEX_MAX - Soc.INDEX)*U16Tmp*120,BlockParam.AVG_CURRENT*SOC_INDEX_MAX);
			}
			else
			{
				TempU32 = math_diveder((SOC_INDEX_MAX - Soc.INDEX)*U16Tmp*120,BlockParam.CHG_CURRENT*SOC_INDEX_MAX);
			}
		}
	}
	OneLine.chglft = TempU32; 
}




void oneline_data_send(void)//500us  once
{
	static int TXFILT_CNT;
	u16 TempU16;
//	if (ChgOutHigh && !OneLine_is_Active)
//	{
//		TXFILT_CNT++;
//		if(TXFILT_CNT >= 5)
//		{
//			OneLine.TX_STATE = 43;
//		}				
//	}
//	else 
//	{
//		TXFILT_CNT = 0;
//	}			 
		
	
	switch(OneLine.TX_STATE++)
	{
		case 0:
			if (OneLine.TX_TYPE==0)
			{  
				OneLine.TX_DATA[0] = 0x50;
				OneLine.TX_DATA[1] = 0x19; 
				OneLine.TX_DATA[2] = Soc.SOH;  
				
				if(SystemParam.BUS_CURRENT > 0)
				{
					OneLine.TX_DATA[3] = 0;
					OneLine.TX_DATA[4] = 0;
				}
				else
				{
					TempU16 = abs(SystemParam.BUS_CURRENT)>>6;
					TempU16 *=SystemParam.BUS_VOLTAGE>>6; 
					TempU16= math_diveder(TempU16,100);
					OneLine.TX_DATA[3] = TempU16>>4;
					OneLine.TX_DATA[4] = TempU16<<4;
				}				
				TempU16  = (SystemParam.BUS_VOLTAGE*10)>>6; 
				OneLine.TX_DATA[4] += TempU16>>8;
				OneLine.TX_DATA[5] = TempU16; 
				 
				OneLine.TX_DATA[6] = OneLine.cap;
				OneLine.TX_DATA[7] = Bat.CELL_NUM-2;			 
			
				
				OneLine.TX_DATA[8] = His.CIRCLE>>8;
				OneLine.TX_DATA[9] = His.CIRCLE; 
				OneLine.TX_DATA[10] = 70;  //max discharge current				
				OneLine.TX_DATA[11] = math_diveder(Cap.QNOW*Soc.SHOW,8*SOC_SHOW_MAX);  
				
				OneLine.TX_DATA[12] = 0;
				OneLine.TX_DATA[13] = 1;
				OneLine.TX_DATA[13] += SystemFlg.CRG_IN<<1;
				if(SystemFlg.CRG_IN)
				{
				  if (SocChargeState)
					{
						OneLine.TX_DATA[13] += 0x04;
					}						
					else if (SocFullChargeState)
					{
						OneLine.TX_DATA[13] += 0x08;
					}
				}
				OneLine.TX_DATA[14] = 0;
				OneLine.TX_DATA[15] = 0;
				OneLine.TX_DATA[16] = 0;
				OneLine.TX_DATA[17] = 0; 
				 
				OneLine.TX_TYPE = 0x80; 
				
				
			}
			else
			{ 
				OneLine.TX_TYPE = 0x80; 
				
				OneLine.TX_DATA[0] = 0x50;
				OneLine.TX_DATA[1] = 0x19; 
				
				OneLine.TX_DATA[2] = Soc.SHOW|0x80;
				
				if(SystemParam.BUS_CURRENT < 0)
				{
					OneLine.TX_DATA[3] = 0;
					OneLine.TX_DATA[4] = 0;
				}
				else
				{
					TempU16 = abs(SystemParam.BUS_CURRENT)>>6;
					TempU16 *=SystemParam.BUS_VOLTAGE>>6; 
					TempU16= math_diveder(TempU16,10);
					OneLine.TX_DATA[3] = TempU16>>4;
					OneLine.TX_DATA[4] = TempU16<<4;
				}
				
				TempU16 = OneLine.chglft;         //chongdian shijian
				OneLine.TX_DATA[4] |= (TempU16>>8)&0x0f;
				OneLine.TX_DATA[5] = TempU16;		
				
				
				TempU16 = AimaParam.LAST_TRIP;     //shengyulicheng
				
				
				OneLine.TX_DATA[6] = TempU16>>1;				
				OneLine.TX_DATA[7] = TempU16&1;
				
				
				TempU16= math_diveder(Cap.QNOW*Bat.CELL_NUM*12*Soc.SHOW,8*SOC_SHOW_MAX);
				OneLine.TX_DATA[8] = TempU16>>8;
				OneLine.TX_DATA[9] = TempU16; 
				
				OneLine.TX_DATA[10] = Bat.CAP_RATING>>2;  //max charge current 
				OneLine.TX_DATA[11] = Bat.CELL_NUM*14.7;   
				
				OneLine.TX_DATA[12] = 0;
				OneLine.TX_DATA[13] = 1;
				OneLine.TX_DATA[13] += SystemFlg.CRG_IN<<1;
				if(SystemFlg.CRG_IN)
				{
				  if (SocChargeState)
					{
						OneLine.TX_DATA[13] += 0x04;
					}						
					else if (SocFullChargeState)
					{
						OneLine.TX_DATA[13] += 0x08;
					}
				}
				OneLine.TX_DATA[14] = 0;
				OneLine.TX_DATA[15] = 0;
				OneLine.TX_DATA[16] = 0;
				OneLine.TX_DATA[17] = 0; 
				OneLine.TX_TYPE = 0; 
			} 
			OneLine.TX_DATA[ONE_LINE_TX_DATA_NUM - 1] = 0;
		  OneLine_Low();
			break;
		case 101:
		case 104:
		case 255:
			OneLine_Low();
			break;
		case 100:
			OneLine.TX_MASK 	= 0x80;
			OneLine.TX_INDEX 	= 0; 
			OneLine_High();
			OneLine.DELAY = 0;
			break;
		case 102:
			if (OneLine.TX_DATA[OneLine.TX_INDEX]&OneLine.TX_MASK)
			{
				OneLine_High();
			}
			break;
		case 103:
			OneLine.TX_STATE = 101;
			OneLine_High();
			if (OneLine.TX_MASK == 1)
			{
				OneLine.TX_DATA[ONE_LINE_TX_DATA_NUM - 1] ^= OneLine.TX_DATA[OneLine.TX_INDEX];
				if (++OneLine.TX_INDEX < ONE_LINE_TX_DATA_NUM)
				{
					OneLine.TX_MASK = 0x80;
				}
				else
				{
					OneLine.TX_STATE = 104;
				}
			}
			else
			{
				OneLine.TX_MASK >>= 1;
			}
			break;
		case 105:
			if (OneLine.DELAY < 2850)
			{
				OneLine.DELAY++;
				OneLine.TX_STATE = 105;
				//OneLine_High();
			}
			break;
	}
	
	if (OneLine_is_Active)
	{
		SystemTick.SLEEP_CNT = 0; 
	}
	else if (SystemFlg.SLEEPING )
	{
		OneLine.TX_STATE = 200;
	}
}


int meter_signal = 0;
void meter_data_process(void)
{
	u8 reset_signal = 0;
	if (OneLine.RX_DATA[0] == 8 && OneLine.RX_DATA[1] == 0x61)
	{
		meter_signal = 1;
		OneLine.lvjuspeed = OneLine.RX_DATA[7];
		
		if ((OneLine.RX_DATA[3]&0x80)==0)
		{
			if (0 == (OneLine.RX_DATA[10]&0xC0))
			{
				if (OneLine.RX_DATA[10]&0x10)
				{
					OneLine.RX_BAT = 6;
				}
				else if (OneLine.RX_DATA[10]&0x4)
				{
					OneLine.RX_BAT = 5;
				}
				else if (OneLine.RX_DATA[10]&0x2)
				{
					OneLine.RX_BAT = 4;
				}
				else
				{
					OneLine.RX_BAT = 0;
				}
				
				if (OneLine.RX_BAT != Bat.CELL_NUM && OneLine.RX_BAT == OneLine.LST_BAT && OneLine.RX_BAT)
				{
					if (OneLine.CHK_CNT < 6)
					{
						OneLine.CHK_CNT++;
					}
					else
					{
						soc_reset(OneLine.cap, OneLine.RX_BAT,1);
					}
				}
				else
				{
					OneLine.CHK_CNT = 0;
				}
				OneLine.LST_BAT = OneLine.RX_BAT;
			}
		}
		if ((OneLine.RX_DATA[3]&0x80)==0)
		{
					OneLine.rx_cap = OneLine.RX_DATA[15]&0x7f;
					
			    if (OneLine.rx_cap != OneLine.cap && OneLine.rx_cap == OneLine.lst_rx_cap && OneLine.rx_cap > 6)
					{
						if (OneLine.CHK_CNT_cap < 6)
						{
							OneLine.CHK_CNT_cap++;
						}
						else
						{
							OneLine.cap = OneLine.rx_cap;
							soc_reset(OneLine.cap, Bat.CELL_NUM,1);
						}
					}
					else
					{
						OneLine.CHK_CNT_cap = 0;
					}
					OneLine.lst_rx_cap = OneLine.rx_cap;
			
		}
	} 
	
}


void oneline_data_receive(void)//100us  62.5
{
	if (SystemFlg.SLEEPING)
	{
		OneLine.RX_STATE = 0;
	}
	
	switch(OneLine.RX_STATE)
	{
		case 1:
			if (MeterIOisHigh)
			{
				if (OneLine.RX_CNT > 400*100/62)
				{
					OneLine.RX_STATE = 2;
					OneLine.RX_CNT	 = 1;
				}
				else
				{
					OneLine.RX_STATE = 0;
				}
			}
			else if (OneLine.RX_CNT < 500*100/62)
			{
				OneLine.RX_CNT++;
			}
			break;
		case 2:
			if (MeterIOisLow)
			{
				OneLine.RX_STATE 		= 3;
				OneLine.RX_CNT	 		= 1;
				OneLine.RX_MASK			= 0x80;
				OneLine.RX_INDEX 		= 0;
				OneLine.RX_DATA[0] 	= 0;
				OneLine.RX_XOR			= 0;
			}
			else if (++OneLine.RX_CNT >= 200*100/62)
			{
				OneLine.RX_STATE = 0;
			}
			break;
		case 3:
			if (MeterIOisHigh)
			{
				OneLine.RX_CNT_L = OneLine.RX_CNT++;
				OneLine.RX_STATE = 4;
			}
			else if (++OneLine.RX_CNT >= 250*100/62)
			{
				OneLine.RX_STATE = 0;
				if (OneLine.RX_INDEX == 12 && OneLine.RX_XOR == 0 && OneLine.RX_MASK == 0x80)
				{
					meter_data_process();
				}
			}
			break;
		case 4:
			if (MeterIOisLow)
			{
				OneLine.RX_STATE = 3;
				if (OneLine.RX_CNT_L * 2 < OneLine.RX_CNT)
				{
					OneLine.RX_DATA[OneLine.RX_INDEX] |= OneLine.RX_MASK;
				}
				
				if (OneLine.RX_MASK == 1)
				{
					OneLine.RX_MASK = 0x80;
					OneLine.RX_XOR ^= OneLine.RX_DATA[OneLine.RX_INDEX];
					if (++OneLine.RX_INDEX >= ONE_LINE_RX_DATA_NUM)
					{
						OneLine.RX_STATE = 0;
						if (OneLine.RX_XOR == 0)
						{
							meter_data_process();
						}
					}
					else
					{
						OneLine.RX_DATA[OneLine.RX_INDEX] = 0;
					}
				}
				else
				{
					OneLine.RX_MASK >>=1;
				}
				OneLine.RX_CNT = 1;
			}
			else if (++OneLine.RX_CNT > 500*100/62)
			{
				OneLine.RX_STATE = 0;
			}
			break;
		default:
			OneLine.RX_STATE = (MeterIOisLow)?1:0;
			OneLine.RX_CNT 	 = 1;
			break;
	}
	
	if (OneLine.RX_STATE > 2)
	{
		SystemTick.SLEEP_CNT = 0; 
	}
}



void judge_chg_in(void)
{
	static int charge_io_cnt;
	if (SystemFlg.CRG_IN)
	{
		if (ChargerIOHigh)
		{
			charge_io_cnt = 0;
		}
		else if (charge_io_cnt < 20)
		{
			charge_io_cnt++;
		}
		else
		{
			SystemFlg.CRG_IN = 0;
		}
	}
	else if (ChargerIOHigh)
	{ 
		if (charge_io_cnt < 20)
		{
			charge_io_cnt++;
		}
		else
		{
			SystemFlg.CRG_IN = 0;
		}
	}
	else
	{
		charge_io_cnt = 1;
	}
}
