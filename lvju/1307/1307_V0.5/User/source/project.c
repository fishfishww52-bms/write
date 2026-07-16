#include "main.h"
#include "hwparam.h"
#include "init.h"
#include "flash.h"
#include "soc.h"
#include "can.h"
#include "toolfun.h"
#include "userfun.h"

typedef struct {
	u32 ADDR_WRITING;
	u32 INDEX;
	u32 INDEX_LAST;
	u32	FILE_SIZE;
	s32	ERT_SIZE;
	u32 FLASH_SIZE;
	u32 ERT_INDEX;
	u32 CODE_SIZE;
	u32 ERT_SUM;
	u32 MASK;
	u32 YAEA_CRC;
	u16 DATA_NUM;
	u8 STATE;
	u8 ERROR;
	u32 crc;
	u32 BOOT_EN;
}boot_param_type;


boot_param_type BootParam;
_zhuimi_type zhuimi;



#define		CODE_SIZE_MAX			0xE400
#define Zm_BOOT_CMD_LEN 4


const u8 Zm_BootCmd[Zm_BOOT_CMD_LEN] = "BOOT";
extern const u8 BootCmd[];

volatile u8 SystemFlgGO_SYSTEM=0;

volatile  unsigned int erase_index;
 
volatile systemflttype	SystemFlt;
u8 CHARGER_CONNECTED = 0;
u8 BAT_SW = 0;  
volatile u8  charge_current = 0xCC; 
volatile u16 chg_lft;
volatile u16 chg_state;
s16 ave_power;
s32 sum_power;

s32 sum_current5s;
s16 ave_current5s;
s32 sum_current;
s16 ave_current;

u32  ave_vol;
u8  volchg;

u16  realsoc;
 unsigned int v_cnt;
 unsigned int soc_showmax_cnt;


extern test_data_type RecordData;
extern uart_data_type Uart0Msg,Uart2Msg;


u16 bat_code[]={0x4812,0x4820,0x4823,0x6020,0x6023,0x6032,0x6038,0x7220,0x7223,0x7232,0x7238};
msg_type_id0  msg_id0=
{
	.hw_version = 0x10,
	.sw_version = 0x10,
	.boot_version = 0x10,
	.com_version = 0x10,
	.sid=0x01,
	.pid=0x01,
	.year=25,
	.month=4,
	.day=6,
	.sn=888,
	.r1[0]=0xff,
	.r1[1]=0xff,
	.r2[0]=0xff,
	.r2[1]=0xff,
	.r2[2]=0xff,
};
msg_type_id1  msg_id1;
msg_type_id8  msg_id8;
msg_type_id9  msg_id9;

static u8 err_cnt1;
static u8 err_cnt2;  

void get_bat_code()
{
	u16 TempU16;
	//int temp;
	TempU16 = Bat.CELL_NUM*12*100 + zhuimi.cap_set;  
//	TempU16 = (temp/10)<<24 ;
//	TempU16 += (temp%10)<<16;
//	TempU16 += (zhuimi.cap_set/10)<<8;
//	TempU16 += (zhuimi.cap_set%10); 
	
	if(zhuimi.bat_brand==0)
	{
		switch(TempU16)
		{
			case 4812: zhuimi.batcode = 0; break;
			case 4820: zhuimi.batcode = 1; break;
			case 4823: zhuimi.batcode = 2; break;
			case 6020: zhuimi.batcode = 3; break;
			case 6023: zhuimi.batcode = 4; break;
			case 6032: zhuimi.batcode = 5; break;
			case 6038: zhuimi.batcode = 6; break;
			case 7220: zhuimi.batcode = 7; break;
			case 7223: zhuimi.batcode = 8; break;
			case 7232: zhuimi.batcode = 9; break;
			case 7238: zhuimi.batcode = 10; break;
			default:zhuimi.batcode = 9; break;
		}
	}
	else if (zhuimi.bat_brand>0)
	{
		switch(TempU16)
		{
			case 4812: zhuimi.batcode = 15; break;
			case 4820: zhuimi.batcode = 16; break;
			case 4823: zhuimi.batcode = 17; break;
			case 6020: zhuimi.batcode = 18; break;
			case 6023: zhuimi.batcode = 19; break;
			case 6032: zhuimi.batcode = 20; break;
			case 6038: zhuimi.batcode = 21; break;
			case 7220: zhuimi.batcode = 22; break;
			case 7223: zhuimi.batcode = 23; break;
			case 7232: zhuimi.batcode = 24; break;
			case 7238: zhuimi.batcode = 25; break;
			default:zhuimi.batcode = 24; break;
		} 
	}
}
void get_req_chg_current(void)
{
	static int cnt;
	if(charge_current==0xCC)
	{
		msg_id1.req_chg_current =  math_diveder(Bat.CAP_RATING*20,10);     //0.1A 
	}
	else
	{
		msg_id1.req_chg_current =  charge_current;   
	}	
	if(SystemParam.TEMP_NOW<10)
	{
		msg_id1.req_chg_current =  Bat.CAP_RATING*3;  
	}
	else if(SystemParam.TEMP_NOW<40)
	{
		msg_id1.req_chg_current =  Bat.CAP_RATING*5;  
	}
	else
	{
		msg_id1.req_chg_current =  Bat.CAP_RATING*3; 
	} 
	if ((Soc.SHOW== SOC_SHOW_MAX ) && (SocFullChargeState))
	{
		if(cnt++>10)
		{
			msg_id1.req_chg_current =  Bat.CAP_RATING; 
			cnt=20;
		} 
	} 
	else
	{
		cnt=0;
	}
	if(charge_current < msg_id1.req_chg_current)
	{
		msg_id1.req_chg_current = charge_current;
	}
}
void msg_pre_send()
{
	extern test_data_type RecordData;
	u8 TempU8;
	msg_id1.rated_voltage = Bat.CELL_NUM * 12;
	msg_id1.rated_cap = Bat.CAP_RATING;
	msg_id1.max_chg_voltage = Bat.CELL_NUM * 1500;	
	msg_id1.req_chg_voltage = Bat.CELL_NUM * 1470;		
  ///get_req_chg_current();
	msg_id1.soc = math_diveder(Soc.SHOW*100,SOC_SHOW_MAX);
	msg_id1.soh = Soc.SOH;
	msg_id1.bat_current = math_diveder(SystemParam.BUS_CURRENT*100,CURRENT_UNIT);
	msg_id1.bat_voltage = math_diveder(SystemParam.BUS_VOLTAGE*100,VOLTAGE_UNIT);
	msg_id1.temp_now = ((SystemParam.TEMP_NOW>>1)<<1)+40; 	
	
	 
	
	if(Uart0Msg.CMD==0x08)
	{
//		if ((Bat.CELL_NUM==4 && msg_id8.pid!=13) ||    //48V3A=13,60V4A=24,72V4A=34，72V4A=36  72V6A   48
//			(Bat.CELL_NUM==5 && msg_id8.pid!=24) || 
//		(Bat.CELL_NUM==6 && msg_id8.pid!=34 && msg_id8.pid!=36 && msg_id8.pid!=48) )
		if ( (Bat.CELL_NUM==4 && msg_id8.pid>15 ) ||
			    (Bat.CELL_NUM==5 && msg_id8.pid<16 ) || (Bat.CELL_NUM==5 && msg_id8.pid>30 ) ||
		    (Bat.CELL_NUM==6 && msg_id8.pid<31 ) || (Bat.CELL_NUM==6 && msg_id8.pid>49 ) )
		{
			msg_id1.handshake_state = 0x80;
			msg_id1.bat_state = NO_CHARGE_ALLOWED;
		}
		else
		{
			msg_id1.handshake_state = 0x0;
			msg_id1.bat_state = 0x0;
		}
		return;
	}
	else if(Uart0Msg.CMD==0x09)
	{
		if(msg_id9.charger_state&0x40)
		{
			msg_id1.handshake_state |= 0x80;
			msg_id1.bat_state |= NO_CHARGE_ALLOWED;
		}
		TempU8 = SystemFlt.WORD;
		TempU8 &=0x0f;
		if(TempU8>0)
		{
			msg_id1.bat_state |= TempU8;
			msg_id1.bat_state |= NO_CHARGE_ALLOWED;
		}
		if ((Soc.SHOW== SOC_SHOW_MAX ) && (SocFullChargeState))
		{
			msg_id1.bat_state |= 0x80;
		}
	} 
}


void UART0_msg_handle(void)    
{  
	u8 i,*p;
	if (Uart0Msg.RXD)
	{
		if(Uart0Msg.CMD == 0x08)
		{
			CHARGER_CONNECTED=1;
			err_cnt1=0;
			err_cnt2=0;
			p=(u8*)&msg_id8;
			for(i=0;i<16;i++)
			{
				*p++=Uart0Msg.DATA[i];
			} 
			Uart0Msg.RXD 			= 0;	
			msg_pre_send();
		}
		else if(Uart0Msg.CMD == 0x09)
		{
			err_cnt2=0;
			p=(u8*)&msg_id9;
			for(i=0;i<16;i++)
			{
				*p++=Uart0Msg.DATA[i];
			}
			Uart0Msg.RXD 			= 0;
			msg_pre_send();
		} 
		else
		{
			Uart0Msg.RXD 			= 0;	
		} 
	} 
//	else if (Uart0Msg.TXING)
//	{ 
//		if(DMA->INTF_b.TC1)
//		{
//			 Uart0Msg.TXING = 0; 
//			 READ_BIT(UART0->STS, UART_STS_RX_DATA_NEMP);
//			 TempU8 = (u8)(UART0->RX_DATA&0xff);
//		}
//	} 
	else if (Uart0Msg.TXING)
	{
		if (++Uart0Msg.TXING >= 250)
		{
			Uart0Msg.TXING = 0; 
		}
	}  
} 

void polling(void)   //1ms
{
	static u16 senddelay;
	static u8 bus_pulldown_cnt,bus_cnt;	
	if(ALineRxisLow)
	{
		bus_pulldown_cnt++; 
	}
	if(++bus_cnt > 200)
	{
		if(bus_pulldown_cnt>190)
		{
			SystemFlg.CRG_IN = 0;
		}
		else
		{ 
			SystemFlg.CRG_IN = 1;     
		}
		bus_cnt = 0;
		bus_pulldown_cnt = 0;
	}	
	if(SystemFlg.CRG_IN)
	{
		 if(CHARGER_CONNECTED==0)
		 { 
			 if(err_cnt1>=15) return;
			 senddelay++;
			 if(senddelay<(err_cnt1<5?200:1000))
				{
					return;
				}
				else
				{
					senddelay=0; 
				}
				err_cnt1++;
				get_bat_code();
				msg_id0.pid = zhuimi.batcode;
			  set_txarray0((u8*)&msg_id0,16,0x00);  ////////////QQ////////////////////// 0->
		 }
		 else
		 {
			  if(err_cnt2>=3)
			  {
					 CHARGER_CONNECTED=0;
					 err_cnt1 =0;
					 err_cnt2 =0; 
					 return;
			  }
				senddelay++;
				if(senddelay<200)
				{
					return;
				}
				else
				{
					senddelay=0; 
				}
			  err_cnt2++;
			  set_txarray0((u8*)&msg_id1,16,1);
		 }
	}
	else
	{
		err_cnt1 =0;
		err_cnt2 =0; 
		CHARGER_CONNECTED = 0; 
	}
}






void  voltage_flt_process(void)
{
	static s16 volt_flt_cnt;
	
	static u32  sum_vol,sum_cnt ;
	
	sum_vol += SystemParam.BUS_VOLTAGE;
	sum_cnt++;
	if(sum_cnt>=64)
	{
		ave_vol = sum_vol>>6;
		sum_cnt = 0;
	} 
	
	
	
	
	if (SystemParam.BUS_VOLTAGE > TOTAL_CELL_VOLTAGE(OVV_VOLT) && SocShowAllowIncState)
	{
		if (volt_flt_cnt < 500)
			volt_flt_cnt++;
		else if (SystemFlt.OVV == 0)
		{
			SystemFlt.OVV = 1;
		}
	}
	else if (SystemParam.BUS_VOLTAGE < TOTAL_CELL_VOLTAGE(UVV_VOLT))
	{
		if (volt_flt_cnt > -500)
		{
			volt_flt_cnt--;
		}
		else if (SystemFlt.UVV == 0)
		{
			SystemFlt.UVV = 1;
//			Soc.INDEX = 0;
//			Soc.ZERO	= 0;
//			Soc.SHOW 	= 0;
		}		
		if (SystemParam.BUS_VOLTAGE < VOLTAGE_V(35.0))
		{
			if (volt_flt_cnt > -11000)
			{
				volt_flt_cnt--;
			}
			else if (SystemFlt.UDV == 0)
			{
				SystemFlt.UDV = 1;
			}
		}
		else if (SystemParam.BUS_VOLTAGE < TOTAL_CELL_VOLTAGE(SYS_UVV_VOLT))
		{
			if (volt_flt_cnt > -10000)
			{
				volt_flt_cnt--;
			}
			else if (SystemFlt.SYS_UVV == 0)
			{
				SystemFlt.SYS_UVV = 1;
			}
		}
	}
	else
	{
		if (SystemFlt.UVV)
		{
			if (SystemParam.BUS_VOLTAGE > TOTAL_CELL_VOLTAGE(UVB_VOLT))
			{
				if (volt_flt_cnt < 500)
				{
					volt_flt_cnt++;
				}
				else 
				{
					SystemFlt.UVV 		= 0;
					SystemFlt.SYS_UVV = 0;
					SystemFlt.UDV			= 0;
				}
			}
						
		}
		else if (SystemFlt.OVV)
		{
			if (SystemParam.BUS_VOLTAGE < TOTAL_CELL_VOLTAGE(OVB_VOLT))
			{
				if (volt_flt_cnt > -500)
				{
					volt_flt_cnt--;
				}
				else 
				{
					SystemFlt.OVV = 0;
				}
			}
		}
		else
		{
			volt_flt_cnt = 0;
		}
	}
}




void current_flt_process()
{
	static s16 ovc_cnt;
	static u16 power_cnt;
	static u16 ave_cnt;
	static u16 ave_cnt5s; 
	
//	RecordData.DATA[15] = BATTERY_VOLTAGE(SystemParam.BUS_VOLTAGE>>6);
//	RecordData.DATA[14] = AdcAvg.VOLTAGE;
	
	sum_power += (s32)(SystemParam.BUS_VOLTAGE*SystemParam.BUS_CURRENT)>>12;  
	if(++power_cnt>=1024)
	{
		power_cnt=0;
		ave_power = -(sum_power>>10);
		sum_power = 0; 
	}

  sum_current +=SystemParam.BUS_CURRENT;	
	if(++ave_cnt>=1024*32)
	{
		ave_cnt=0; 
		ave_current = sum_current>>15;
		sum_current =0;
	}
	
	sum_current5s +=SystemParam.BUS_CURRENT;
  if(++ave_cnt5s>=1024*8)
	{
		ave_cnt5s=0; 
		ave_current5s = sum_current5s>>13;
		sum_current5s =0;
	} 	 	
	
	
	if (SystemParam.BUS_CURRENT < DIS_CURRENT(OVC_SC))
	{
		SystemFlt.DIS_SC = 1;
	}
	
	if (SystemParam.BUS_CURRENT < DIS_CURRENT(OVC_DIS))
	{
		if (ovc_cnt < 10000)
		{
			ovc_cnt++;
		}
		else if (SystemFlt.DIS_OVC == 0)
		{
			SystemFlt.DIS_OVC = 1;			
		}
	}
	else if (SystemParam.BUS_CURRENT > CHG_CURRENT(OVC_CHG) && SocChargeState)
	{
		if (ovc_cnt > -5000)
		{
			ovc_cnt--;
		}
		else if (SystemFlt.CHG_OVC == 0)
		{
			SystemFlt.CHG_OVC = 1;			
		}
	}
	else if (ovc_cnt > 0)
	{
		ovc_cnt--;
	}
	else if (ovc_cnt < 0)
	{
		ovc_cnt++;
	}
	else
	{
		if (SocReadyState)
		{
			if (SystemFlt.DIS_OVC)
			{
				SystemFlt.DIS_OVC = 0;
			}
			else if (SystemFlt.CHG_OVC)
			{
				SystemFlt.CHG_OVC = 0;
			}
			SystemFlt.DIS_SC = 0;
		}
	}		
}

void tempreture_flt_process()
{
	static s8 temp_flt_cnt;
	if (SocShowAllowIncState)
	{
		if (SystemParam.TEMP_NOW > OVT_CHG)
		{
			if (temp_flt_cnt < 10)
			{
				temp_flt_cnt++;
			}
			else if (SystemFlt.OVT == 0)
			{
				SystemFlt.OVT = 1;				
			}
		}		
	}
	else if (SystemFlt.OVT)
	{
		if (SystemParam.TEMP_NOW < OVT_RCV)
		{
			if (temp_flt_cnt < 10)
			{
				temp_flt_cnt++;
			}
			else
			{
				SystemFlt.OVT = 0;
			}
		}
		else
		{
			temp_flt_cnt = 0;
		}
	}	
	else
	{
		if (SystemParam.TEMP_NOW > OVT_DIS)
		{
			if (temp_flt_cnt < 10)
			{
				temp_flt_cnt++;
			}
			else if (SystemFlt.OVT == 0)
			{
				SystemFlt.OVT = 1;				
			}
		}		
		else
		{
			temp_flt_cnt = 0;
		}
	}
	
	
}




u8 batflt;

u8 bat_flt_calc(void)
{
	u8 TempU8 = 0;	
	if (SystemFlt.OVV)	
	{
		TempU8 |= 0x01;
	}
	else if (SystemFlt.CHG_OVC)	
	{
		TempU8 |= 0x02;
	}
	else if (SystemFlt.DIS_OVC)		
	{		
		TempU8 |= 0x04;		
	}	
	else if (SystemFlt.DIS_SC)
	{
		TempU8 |= 0x08;
	}
	else if (SystemFlt.OVT)
	{
		TempU8 |= 0x10;
	}	
	batflt = TempU8;
	return TempU8;
}

void set_txarray0(u8*data, u8 len, u8 cmd)
{
	u8 checksum, i;
	for(i=0;i<30;i++)
	{
		Uart0TxArray[i]=0xff;
	}
	i = 4;
	Uart0TxArray[0] = 0x5A;
	Uart0TxArray[1] = 0x4D;
	Uart0TxArray[2] = cmd;
	Uart0TxArray[3] = len;
	checksum	= len + cmd;
	while(len--)
	{
		Uart0TxArray[i++] = *data;
		checksum += *data++;
	}
	//checksum = ~checksum;
	Uart0TxArray[i++] = checksum;
	Uart0TxArray[i++] = SW_FRAME_END1;
	Uart0TxArray[i++] = SW_FRAME_END2; 
	
	UART0->INTFC_b.TX	= 1;
	DMA_Ch0->CTRL_b.E = 0;
	DMA_Ch0->DTN			= i;
	DMA->INTFC_b.TC0 	= 1;
	DMA_Ch0->CTRL_b.E = 1;
	UART0->CTRL_b.TX_DMA_E 	= 1;
	Uart0Msg.TXING					= 1;
}


 



 
 

u32 distime,realtime;

 u8 test;

u16 getrealtime(u16 SOCIX)
{
		u32 TempU16,U16Tmp,vtemp;
	  static u16 counterdown;
		if (SocFullChargeState || Soc.SHOW >= SOC_SHOW_MAX)
		{
			TempU16 = 0;
			counterdown=0;
		}
		else     
		{ 
			U16Tmp = Cap.QNOW*CURRENT_UNIT/Q_UNIT;
			if (Soc.INDEX < SOCIX  &&  (Marks.CHG_VOLT==0))  
			{
				TempU16 = math_diveder((SOCIX - Soc.INDEX)*U16Tmp,SOC_I_MAX); 
				TempU16 = math_diveder(TempU16*60,abs(ave_current)); 
				vtemp = math_diveder((SOC_I_MAX  - SOCIX)*U16Tmp*90,SOC_I_MAX); 
				TempU16 += math_diveder(vtemp,abs(ave_current)) + 20; 
			}
			else if (Soc.SHOW == SOC_SHOW_VALUE(99/100) )
			{
				TempU16 = 0;
				if(counterdown ==0)
				{
					counterdown = 64*20;
				}
				else
				{
					counterdown--;
				}
				TempU16 = counterdown>>6;
			}
			else
			{
				//TempU16 = math_diveder((SOC_I_MAX*0.991 - Soc.INDEX)*U16Tmp*120, abs(ave_current)*SOC_I_MAX )   ;				
				 TempU16 = (SOC_I_MAX*0.991 - Soc.INDEX)*U16Tmp*90/SOC_I_MAX/abs(ave_current)  + 20;						
				//TempU16 = math_diveder(TempU16*25, Soc.INDEX - SOCIX + 25 ) + 20;
			}  
		}
		return TempU16;
}









void ChgTimeCal()
{
	u32 rs,ds;
	u32 K;
	static u16 cnt;
	
	
	if(volchg ==0)
	{
		if(ave_current > CURRENT_A(4))
		{
			realtime = getrealtime(SOC_I(0.85)	) ;        /*TBD 85*/
		}
		else
		{
			realtime = getrealtime(SOC_I(0.92)	) ;          /*TBD 92*/
		}
	}
	else
	{
		realtime =  getrealtime(SOC_I(0.85));
	}
	
	if(realtime >= distime)
	{
		ds = realtime - distime;
	}
	else
	{
		ds = distime - realtime;
	}

//	if(realtime > 0)
//	rs = ds*100/realtime;     /*uint类型，*100倍的百分系数*/
//	else
//		rs=0;
	
	if(ds > 30)
	{
		distime = realtime;
		K=60;
	}
	else
	{
		K= 60*realtime/distime;
	}
	
//	if(realtime < distime)
//	{
//		if(rs<=1)
//		{
//			K = 60;
//		}
//		else if((rs>10)&&(ds>10))
//		{
//			distime = realtime;
//			//DlyChgUpdateTimeTim = 0;/*直接赋值时，清除延时*/
//			cnt=0;
//			K=0;
//		}
//		else
//		{
////			short temp;
////			temp = 60 - (rs*4*60/100); 
////			if(temp < 10)
////			K = 10;
////			else
////				K = temp;
//			K= 60*realtime/distime;
//		}
//	}
//	else
//	{
//		if(rs<=1)
//		{
//			K = 60;
//		}
//		else if((rs>10)&&(ds>20))
//		{
//			distime = realtime;
//			//DlyChgUpdateTimeTim = 0;/*直接赋值时，清除延时*/
//			cnt =0;
//			K = 0;
//		}
//		else
//		{
//			//K = 120 + (rs*9*60/100);
//			K= 60*realtime/distime;
//		}
//	}
  if(K < 10) 
	{
		K = 10;
	}
	cnt++;
	if(cnt>K)
	{
		cnt = 0;
		if(distime>0)
		distime--;
	}
 
	if(realtime==0)
		distime=0; 
}





void  charge_lft()
{
	static unsigned char delay, valid;
	static s16 initcurrent;
	static s16 vol_cnt;
	static u16 setchgcurrent,changecnt;
	
	if(volchg==0)
	{
		if(ave_vol > TOTAL_CELL_VOLTAGE(13.9))
		{
			vol_cnt++;
		}
		else
		{
			vol_cnt = 0;
		}
		if(vol_cnt > 20)
		{
			volchg = 1;
		}
	}
	else
	{
		if(ave_vol < TOTAL_CELL_VOLTAGE(13.8))
		{
			vol_cnt++;
		}
		else
		{
			vol_cnt = 0;
		}
		if(vol_cnt > 30)
		{
			volchg = 0;
		}
	} 
		if (SocShowAllowIncState)
		{
			if(charge_current!=setchgcurrent)
			{
				if(changecnt++>120)
				{
					changecnt=0;
					setchgcurrent = charge_current;					
					distime = realtime;					 
				}
			}
		}
		else
		{
			changecnt=0;
			setchgcurrent = charge_current;
		} 
		
	
	if (SocShowAllowIncState)
	{ 
			if(delay>80)
			{
				delay = 90;
				valid=2;
			}
			else if(delay>10)
			{  
				if(valid==1)
				{
					ave_current = initcurrent;
				}
				else
				{
					if( abs(ave_current5s - SystemParam.BUS_CURRENT)< CURRENT_A(0.5) && ave_current5s >  CURRENT_A(1)  )
					{
						valid=1;
						initcurrent = ave_current5s;
						ave_current = initcurrent;
					} 
				}
			} 
	}
	else
	{
		delay=0;
		valid=0;
		volchg = 0;
	}
	delay++;
	
	
	if  ((SocShowAllowIncState) && valid>=1)
	{ 
		 ChgTimeCal();
     chg_lft = distime;
		 if(valid==1)
		 {
			  distime = realtime;
		 }
	}
  else
  {
		chg_lft = 0xcccc;
	} 
}

   
  
 















#define POLYNOMIAL 0x04C11DB7
#define POLY_INIT  0xFFFFFFFF
#define POLY_END   0xFFFFFFFF

uint32_t reverse_bits(uint32_t num, int num_bits) {
    uint32_t result = 0;
    int i;
    for (i = 0; i < num_bits; i++) {
        result = (result << 1) | (num & 1);
        num >>= 1;
    }
    return result;
}


void crc_init(void)
{
	BootParam.crc=POLY_INIT; 
}
inline void crc_cal(u8 data)
{
	u8 i;
	BootParam.crc ^= (uint32_t)(data); 
	for (i=8; i>0; --i)  
	{ 
			if (BootParam.crc & 0x00000001)
					BootParam.crc = (BootParam.crc >> 1) ^ (uint32_t)0xEDB88320;  
			else
					BootParam.crc = (BootParam.crc >> 1);
	}
}
void crc_out(void)
{
	BootParam.crc=BootParam.crc^POLY_END;
}


uint32_t crc32(unsigned char *ptr, unsigned char len,uint8_t input_invert)
{
  unsigned char i; 
  uint32_t crc=POLY_INIT; 
	const uint32_t polynomial= POLYNOMIAL;
	while(len--)
	{
		if(input_invert)
		{
			crc ^= (uint32_t)(*ptr++);     

		    for (i=8; i>0; --i)  
				{ 
						if (crc & 0x00000001)
								crc = (crc >> 1) ^ (uint32_t)0xEDB88320;  //reverse_bits(polynomial,32);
						else
								crc = (crc >> 1);
				}
		}else
		{
			crc ^= ((uint32_t)(*ptr++)<<24); 
			for (i=8; i>0; --i)  
			{ 
				if (crc & 0x80000000)
					crc = (crc << 1) ^ polynomial;
				else
					crc = (crc << 1);
			}
		}
	}
	crc=crc^POLY_END;
  return (crc); 
}

u32 crc32_out;
void testcrc()
{
	 u8 data[9];
	 u8 i;
	for(i=0;i<9;i++)
	{
		data[i]=0x31+i;
	}
	crc32_out = crc32(data, 9,1);
	
}

  


u8 load_iic_data(u8* data, u16 len)
{
	u32 TempU32;
	static u32 temp_data[32] ; 
	u8 TempU8, *tmp_ptr;
  u8 i = 0;	 
	
	if(erase_index < 80)
	{
		if(erase_index == 2)
		{ 
		  voltage_flt_process(); 
		}
		TempU32 = WRITE_START_ADDR + 512*erase_index + 0x10000;
		erase_flash_sector(TempU32);
		erase_index++; 
	} 
 
	tmp_ptr = (u8*)temp_data; 
	for (TempU8 = 0; TempU8 < len; TempU8++)
	{
		*tmp_ptr++ = *data++;
	}
	len = len>>2;
	for (TempU8 = 0; TempU8 < len; TempU8++)
	{
		temp_data[TempU8] -= BootParam.MASK;
	} 
	
	if(0==flash_boot_write(&BootParam.ADDR_WRITING,temp_data, len))
	{ 
		return 0;
	}
	else
	{
		return 1;
	}
	
}

 


 


void respond_update(uart_data_type* Msg)
{
	u16 TempU16;
	u16 i;
	u8 *UartMsgDATA;
	u8 *ptr, TempU8 = 0; 
	ptr = &TempU8;
  switch(Msg->CMD)
  {
		case 0x98:
			
			break;
	  case 0x0A:
		{
			//set_txarray1(ptr,1,0x0A);  break;
			erase_index = 0;	
			for (TempU16 = 0; TempU16 < Zm_BOOT_CMD_LEN; TempU16++)
			{
				if (Zm_BootCmd[TempU16] != Msg->DATA[TempU16])
					break;
			}
			if (TempU16 < Zm_BOOT_CMD_LEN)
			{
				TempU8 = NO_BOOT_AUTH;
			}
			else 
			{
				if ((Msg->DATA[6]<<8) + Msg->DATA[7] > CODE_SIZE_MAX)
				{
					TempU8 = CODE_OUT_SIZE;
				}
				else if (Msg->DATA[8] != HW_VERSION)
				{
					TempU8 = HW_VERSION_ERR;
				}
				else if (Msg->DATA[9] != CHIP_TYPE)
				{
					TempU8 = CHIP_ERR;
				}
				else
				{
					TempU16 = Msg->DATA[6]<<8;
				  TempU16 += Msg->DATA[7];
					BootParam.ERT_SIZE			= TempU16-128;
					BootParam.FILE_SIZE			= TempU16-128;
					BootParam.ERT_INDEX			= 0;
					BootParam.ADDR_WRITING 	= WRITE_START_ADDR + 0x10000;
					BootParam.ERT_SUM				= 0;
					BootParam.STATE 				= 3;
					BootParam.ERROR 				= 0;
					BootParam.MASK					= 0;
					//iic_config();
					crc_init();
					TempU8 += BootParam.ERROR<<4;
				}
			}   
			set_txarray1(ptr,1,0x0A); 
		}
			break;
		case 0x0B:
		{
			u8 replydata[3];
			u8 len = Msg->LEN;
			replydata[0] = 0;		 
			if ((BootParam.ERT_INDEX&0xff) != Msg->DATA[1])
			{
				replydata[0] = INDEX_ERROR;
				replydata[1] = BootParam.ERT_INDEX>>8;
				replydata[2] = BootParam.ERT_INDEX;
				set_txarray1(replydata,3,0x0B); 
				return;
			}
			if (BootParam.ADDR_WRITING + len - WRITE_START_ADDR - 0x10000 > CODE_SIZE_MAX)
			{
				replydata[0] = CODE_OUT_SIZE;
				set_txarray1(replydata,1,0x0B); 
				return;
			}
			for(i=0;i<len-3;i++)
			{
					crc_cal(Msg->DATA[2+i]);
			}
			TempU16 = Msg->DATA[1];
			TempU16 += Msg->DATA[0]<<8;
			if (TempU16 == 0)
			{
				UartMsgDATA=&Msg->DATA[0];
				for (i = 0; i < BOOT_CMD_LEN; i++)
				{
					if (BootCmd[i] != UartMsgDATA[UartMsgDATA[2]+i+2])
						break;
				}
				
				if (i >= BOOT_CMD_LEN && UartMsgDATA[UartMsgDATA[3]+2] == HW_VERSION &&
						UartMsgDATA[UartMsgDATA[3]+3] == CHIP_TYPE)
				{
					BootParam.MASK 		= UartMsgDATA[UartMsgDATA[4]+2];
					BootParam.MASK 		+= (u32)UartMsgDATA[UartMsgDATA[4]+3]<<8;
					BootParam.MASK 		+= (u32)UartMsgDATA[UartMsgDATA[4]+4]<<16;
					BootParam.MASK 		+= (u32)UartMsgDATA[UartMsgDATA[4]+5]<<24;
//					BootParam.CHECK_SUM_ERT = UartMsgDATA[UartMsgDATA[5]+2];
//					BootParam.CHECK_SUM_ERT += (u32)UartMsgDATA[UartMsgDATA[5]+3]<<8;
//					BootParam.CHECK_SUM_ERT += (u32)UartMsgDATA[UartMsgDATA[5]+4]<<16;
//					BootParam.CHECK_SUM_ERT += (u32)UartMsgDATA[UartMsgDATA[5]+5]<<24;
//					BootParam.CODE_INDEX		= 1;
					BootParam.ERT_INDEX = 1;
					replydata[0] = 0;
					
				}
				else
				{
					replydata[0] = NO_BOOT_AUTH;
				} 
 				
				set_txarray1(replydata,1,0x0B); 
				return;
			}
			else
			{ 
				len -= 3;
				load_iic_data(&Msg->DATA[2],len); 				
//				iicwait=1;
//				while(SystemFlgIIC_WAIT)
//				{
//					  ;           
//				}		 
				if (BootParam.ERROR == 0)
				{
					BootParam.ERT_INDEX++;
				}
				else
				{
					replydata[0] = WRITE_ERROR;
				}
			}  
			set_txarray1(replydata,1,0x0B); 
		}
			break;
		case 0x0C:
		{ 
			TempU8 =0;
			set_txarray1(ptr,1,0x0C); 
		}
		break;
		case 0x0D:
		{
			u32 crc;
			crc_out();
			crc= Msg->DATA[0]<<24;
			crc+= Msg->DATA[1]<<16;
			crc+= Msg->DATA[2]<<8;
			crc+= Msg->DATA[3];					
			if (crc ==BootParam.crc&& BootParam.STATE == 3)
			{ 
				BootParam.BOOT_EN=1;
				ert_save(1,UART2,1,0);
				BootParam.BOOT_EN=0;
				TempU8 = OP_SUCCESS;
			}
			else
			{
				TempU8 = CHECK_ERROR;
			} 
			set_txarray1(ptr,1,0x0D); 
			if (TempU8 == OP_SUCCESS)
			{
				SystemFlgGO_SYSTEM = 1;				
			}
		}
		break;
		case 0x0E:
		{
			u8 replydata[5];		
			replydata[0] = 0;
			replydata[1] = 0;		
      replydata[2] = 1;				
			replydata[3] = 0x30 + SW_VERSION_M;
			replydata[4] = 0x30 + SW_VERSION_S; 
			if(SW_VERSION_S<10)
			{
				replydata[4] = 0x30 + SW_VERSION_S; 
			}
			else
			{
				replydata[4] = 0x41 + SW_VERSION_S - 10; 
			}
			set_txarray1(replydata,5,0x0E); 			
		}
	}
}




void boot_to_system(void)
{
	__disable_irq();
	NVM->FSHKEY = NVM_FSHKEY_1;
	NVM->FSHKEY = NVM_FSHKEY_2;
	MODIFY_REG(NVM->CTRL, NVM_CTRL_BOOT_Msk, NVM_BOOT_SYSTEM);
	WRITE_REG(RCU->STS, RCU->STS);
	NVIC_SystemReset();
}






u8 data[40];

void zhuimi485(uart_data_type* Msg)
{  
	u16 TempU16; // U16Tmp;
	
	if(Msg->CMD == 0x99)
	{  
		
		  for(int i=Msg->LEN-1;i<20;i++)
		  {
				Msg->DATA[i] = 0xCC;
			}
		  if (Msg->DATA[0] == 0x01)
			{
				zhuimi.cap_set = CAP_RATING_DEFAULT;
				His.CIRCLE = 0;
				soc_reset(CAP_RATING_DEFAULT,CELL_NUM_DEFAULT, 0);
			}
			if  ((Msg->DATA[1] != Bat.CELL_NUM)  || zhuimi.cap_set != Msg->DATA[2] )
			{
				BAT_SW = 1;
				if (Msg->DATA[1] >= CELL_NUM_MIN && Msg->DATA[1] <= CELL_NUM_MAX && 
							Msg->DATA[2] > CAP_RATING_MIN && Msg->DATA[2] < CAP_RATING_MAX)
				{
					//SystemParam.BUS_VOLTAGE = TOTAL_CELL_VOLTAGE(12.4);
					zhuimi.cap_set = Msg->DATA[2];
					soc_reset(Msg->DATA[2],Msg->DATA[1], 1); 
				}
			}
			if (Msg->DATA[3] <= 100)
			{
			//	if(Msg->DATA[3]==0 && Soc.SHOW > 10)
			//		His.CIRCLE += 10;
				Soc.SHOW = Msg->DATA[3];
			}
			charge_current = Msg->DATA[4]; 
			if ((Msg->DATA[5] != zhuimi.bat_brand) && (Msg->DATA[5]<=2))
			{
				zhuimi.bat_brand = Msg->DATA[5];
				SystemFlg.SOC_SAVE	= 1;
			}		

      /////add 2 bytes 
			data[0] = (SW_VERSION_M<<4) + SW_VERSION_S;     //>6
			data[1] = HW_VERSION;                           //>7			
			TempU16 = math_diveder(SystemParam.BUS_VOLTAGE*10,VOLTAGE_UNIT);		
			data[2] = TempU16>>8;  //v 0.1              8
			data[3] = TempU16;     //v 0.1              9
			data[4] = math_diveder(Soc.SHOW*100,SOC_SHOW_MAX);  //soc  10
			
			data[5]=realsoc;     //rsoc                               //11
			if(SystemParam.TEMP_NOW < -40 || SystemParam.TEMP_NOW > 130)
			{
				data[6] = 0xCC; 
			}
			else
			{
				data[6] = 40+SystemParam.TEMP_NOW;              //12     ((SystemParam.TEMP_NOW>>1)<<1)
			}			
			TempU16 = math_diveder(SystemParam.BUS_CURRENT*10,CURRENT_UNIT) + 5000 ;     //a 0.1A 
      data[7] = TempU16>>8;  //A 0.1              13
			data[8] = TempU16;     //A 0.1              14
      TempU16 = ave_power;    //w
			data[9] = TempU16>>8;            // 15
			data[10] = TempU16;              //16
			
			data[11]  = His.CIRCLE>>8;   //17
			data[12]  = His.CIRCLE;      //18
			data[13]  = His.CIRCLE>>8;    //19
			data[14]  = His.CIRCLE;        //20
			data[15]  = bat_flt_calc();   //21
			if ((Soc.SHOW== SOC_SHOW_MAX ) && (SocFullChargeState))
			{
				data[16] = 0x02;
			}
			else if ((Soc.SHOW== SOC_SHOW_MAX ) && (SocShowAllowIncState))
			{
				data[16] = 0x05;
			}
			else if (SocShowAllowIncState)
			{
				data[16] = 0x01;
			}
			else if(SystemFlt.UVV == 1)
			{
				data[16] = 0x04;
			}
			else if(SocDisChargeState)
			{
				data[16] = 3;
			}
			else
			{
				data[16] = 0;          //22
			}
			chg_state = data[16];     
			
			if(Soc.SOH < 0 || Soc.SOH > 100)
			{
				data[17] = 0xCC;    //23
			}
			else
			{
				data[17] = Soc.SOH; 
			}
			
					
			TempU16 = math_diveder((Cap.QNOW>>3)*Bat.CELL_NUM*12*Soc.SHOW,100);    //soe
			data[18] = TempU16>>8;              //24
			data[19] = TempU16;		    //25
			TempU16 = chg_lft;		   
			data[20] = TempU16>>8;   //26
			data[21] = TempU16;		 		//27	 
      data[22] = 12;         //28
			data[23] = 0xcc;	 	     //29
			
			TempU16 = (Cap.QNOW>>3)*1000;		

          // TempU16 = realtime;			
      data[24] = TempU16>>8;           //30
			data[25] = TempU16;  			 //31
			set_uart_txarray(data,26,0x99,UART2);
			return;
	}	
	else if(Msg->CMD == 0x98)
	{
		data[0] = 'E';    //零部件号
		data[1] = 'B';
		data[2] = 0x13;
		data[3] = 0x01;
		data[4] = 0x00;  
		
		
//		const unsigned char az[26]={0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0xFF,0x18,0x19,0x20,0x21,0x22,0xFF,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x30,0x31,0x32,0x33};
//		data[5] = PN.CST[12]>0x39? (az[PN.CST[12]-0x41]>>4):0;     
//		data[6] = PN.CST[12]>0x39? (az[PN.CST[12]-0x41]<<4):(PN.CST[12]-0x30)<<4;
//		data[6] += PN.CST[13] - 0x30;
//		data[7] = PN.CST[15]-0x30 + ((PN.CST[14]-0x30)<<4);
			
		data[5]=0x00;
		data[6]=0x00;
		data[7]=0x01;
 
		data[8] = 0x30 +  SW_VERSION_M ;      //版本号  H W_VERSION  SW_VERSION_M  SW_VERSION_S
		if(SW_VERSION_S<10)
		{
			data[9] = 0x30 +  SW_VERSION_S ;
		}
		else
		{
			data[9] = 0x41 +  SW_VERSION_S - 10 ;
		}
		data[10] = 0x00 ;
		data[11] = 0x30 + HW_VERSION;
		data[12] = 0xcc;
		data[13] = 0xcc;
		data[14] = 0xcc;
		data[15] = 0xcc;
		data[16]= 0xcc;
		data[17] = 0xcc;
		data[18] = 0xcc; 
		set_uart_txarray(data,18,0x98,UART2);
	}
  respond_update(Msg);
}




void set_txarray1(u8*data,u8 len,u8 cmd)
{
	    u16  i, checkxor; 
	    Rs485Tx_En();
			Uart2TxArray[0] = FRAME_HEAD_C6;
			Uart2TxArray[1] = FRAME_HEAD_6C;
			Uart2TxArray[2] = FRAME_HEAD_AA;	    
			Uart2TxArray[3] = FRAME_HEAD_5A;
			Uart2TxArray[4] = len+1; 	 
	    Uart2TxArray[5] = cmd; 	
	    checkxor = 	Uart2TxArray[2];
	    checkxor ^= Uart2TxArray[3];
	    checkxor ^= Uart2TxArray[4]; 
	    checkxor ^= Uart2TxArray[5]; 
	    i = 6;
			while(len--)
			{
				Uart2TxArray[i++] = *data;
				checkxor ^= *data++;
			}
			Uart2TxArray[i++] = checkxor;
			Uart2TxArray[i++] = FRAME_END_FD;
			
			UART2->INTFC_b.TX	= 1;
				DMA_Ch6->CTRL_b.E = 0;
				DMA_Ch6->DTN			= i;
				DMA->INTFC_b.TC6 	= 1;
				DMA_Ch6->CTRL_b.E = 1;
				UART2->CTRL_b.TX_DMA_E 	= 1;
				Uart2Msg.TXING					= 1;
}
