/**
 * gpt.c - 绿驹一线通 C0 报文数据预处理
 *
 * 说明：
 * 1. 仅重写 oneline_data_send() 的 case 0 数据填充部分；
 * 2. case 100～105 等底层发送时序保持原代码不变；
 * 3. 正式启用 C0 协议时，userfun.h 中 ONE_LINE_TX_DATA_NUM 必须改为 16，
 *    此时 DATA15 才是 DATA0～DATA14 的异或校验。
 */

#include "main.h"
#include "init.h"
#include "soc.h"
#include "flash.h"
#include "toolfun.h"
#include "hwparam.h"
#include "userfun.h"

#define OneLine_is_Active  (GPIO2->IDR_b.P11 == 1)
#define OneLine_Low()      GPIO1->P14 = 0
#define OneLine_High()     GPIO1->P14 = 1


void oneline_data_send(void)  // 500us once
{
    static u8 ChargeDataType;

    u32 TempU32;
    u16 TempU16;
    u16 PosPower;
    u16 NegPower;
    u16 RemainTrip;
    u16 ChargeTime;
    u8 FrameType;
    u8 ChargeFrame;
    u8 VoltageCode;
    u8 SocPercent;
    u8 i;

    switch (OneLine.TX_STATE++)
    {
        case 0:
            /*----------------------------------------------------------
             * C0公共字段及6种DATA9/DATA10内容准备
             * FrameType: 0~5，对应DATA2.D7~D5
             *---------------------------------------------------------*/

            FrameType = OneLine.TX_TYPE;
            if (FrameType >= 6)
            {
                FrameType = 0;
            }

            /* 非充电固定发里程/SOC；充电时里程/SOC与充电时间交替。
             * ChargeDataType在非充电状态持续清零，因此进入充电后的首帧必为0类型。
             */
            if (SystemFlg.CRG_IN)
            {
                ChargeFrame = ChargeDataType;
                ChargeDataType ^= 1;
            }
            else
            {
                ChargeDataType = 0;
                ChargeFrame = 0;
            }

            /* 整个发送数组先清零，预留位自然保持0 */
            for (i = 0; i < ONE_LINE_TX_DATA_NUM; i++)
            {
                OneLine.TX_DATA[i] = 0;
            }

            /* DATA0：设备编码 */
            OneLine.TX_DATA[0] = 0x50;

            /* DATA1：流水号0xE + 铅酸电池类型0 */
            OneLine.TX_DATA[1] = 0xE0;

            /* 标称电压编码：36V=1，48V=2，60V=3，72V=4... */
            VoltageCode = Bat.CELL_NUM - 2;
            if (VoltageCode < 1)
            {
                VoltageCode = 1;
            }
            else if (VoltageCode > 7)
            {
                VoltageCode = 7;
            }

            /* 剩余里程，协议范围0~509km；当前工程变量范围较小，仍做协议限幅 */
            RemainTrip = AimaParam.LAST_TRIP;
            if (RemainTrip > 509)
            {
                RemainTrip = 509;
            }

            /* DATA2：D7~D5帧类型，D4~D1标称电压，D0里程高1位 */
            OneLine.TX_DATA[2] = (FrameType << 5) | (VoltageCode << 1);

            if (ChargeFrame == 0)
            {
                /* DATA5.D7=0：DATA2.D0 + DATA3发送剩余里程，DATA4发送SOC */
                OneLine.TX_DATA[2] |= (RemainTrip >> 8) & 0x01;
                OneLine.TX_DATA[3] = RemainTrip & 0xFF;

                TempU32 = (u32)Soc.SHOW * 100;
                SocPercent = math_diveder(TempU32, SOC_SHOW_MAX);
                if (SocPercent > 100)
                {
                    SocPercent = 100;
                }
                OneLine.TX_DATA[4] = SocPercent << 1;
            }
            else
            {
                /* DATA5.D7=1：DATA3为12位时间高8位，DATA4.D7~D4为低4位 */
                ChargeTime = OneLine.chglft;
                if (ChargeTime > 4093)
                {
                    ChargeTime = 4093;
                }
                OneLine.TX_DATA[3] = ChargeTime >> 4;
                OneLine.TX_DATA[4] = (ChargeTime & 0x0F) << 4;
            }

            /* DATA5：D7帧内容选择，D6~D0标称容量 */
            OneLine.TX_DATA[5] = (ChargeFrame << 7) | (Bat.CAP_RATING & 0x7F);

            /* DATA6~DATA8：正功率0.1kW，负功率0.01kW。
             * 保持原工程电流方向：BUS_CURRENT<0为放电，>0为充电。
             */
            PosPower = 0;
            NegPower = 0;

            if (SystemParam.BUS_CURRENT < 0)
            {
                TempU32  = (u32)(abs(SystemParam.BUS_CURRENT) >> 6);
                TempU32 *= (u32)(SystemParam.BUS_VOLTAGE >> 6);
                PosPower = math_diveder(TempU32, 100);
                if (PosPower > 4093)
                {
                    PosPower = 4093;
                }
            }
            else if (SystemParam.BUS_CURRENT > 0)
            {
                TempU32  = (u32)(abs(SystemParam.BUS_CURRENT) >> 6);
                TempU32 *= (u32)(SystemParam.BUS_VOLTAGE >> 6);
                NegPower = math_diveder(TempU32, 10);
                if (NegPower > 4093)
                {
                    NegPower = 4093;
                }
            }

            OneLine.TX_DATA[6] = PosPower >> 4;
            OneLine.TX_DATA[7] = ((PosPower & 0x0F) << 4) |
                                 ((NegPower >> 8) & 0x0F);
            OneLine.TX_DATA[8] = NegPower & 0xFF;

            /* DATA9、DATA10：6种内容循环 */
            switch (FrameType)
            {
                case 0:     /* 电池总电压，0.1V，12bit */
                    TempU16 = (SystemParam.BUS_VOLTAGE * 10) >> 6;
                    if (TempU16 > 4093)
                    {
                        TempU16 = 4093;
                    }
                    OneLine.TX_DATA[9]  = TempU16 >> 4;
                    OneLine.TX_DATA[10] = (TempU16 & 0x0F) << 4;
                    break;

                case 1:     /* 剩余容量，Ah */
                    TempU32 = (u32)Cap.QNOW * Soc.SHOW;
                    TempU16 = math_diveder(TempU32, 8 * SOC_SHOW_MAX);
                    if (TempU16 > 253)
                    {
                        TempU16 = 253;
                    }
                    OneLine.TX_DATA[9]  = TempU16;
                    OneLine.TX_DATA[10] = 0;
                    break;

                case 2:     /* 剩余能量，Wh */
                    TempU32  = (u32)Cap.QNOW * Bat.CELL_NUM;
                    TempU32 *= 12;
                    TempU32 *= Soc.SHOW;
                    TempU32  = math_diveder(TempU32, 8 * SOC_SHOW_MAX);
                    if (TempU32 > 65533)
                    {
                        TempU32 = 65533;
                    }
                    OneLine.TX_DATA[9]  = TempU32 >> 8;
                    OneLine.TX_DATA[10] = TempU32;
                    break;

                case 3:     /* 循环次数 */
                    TempU32 = His.CIRCLE;
                    if (TempU32 > 65533)
                    {
                        TempU32 = 65533;
                    }
                    OneLine.TX_DATA[9]  = TempU32 >> 8;
                    OneLine.TX_DATA[10] = TempU32;
                    break;

                case 4:     /* 最大允许充电电压V + 最大允许放电电流A */
                    TempU16 = math_diveder((u32)Bat.CELL_NUM * 147, 10);
                    if (TempU16 > 253)
                    {
                        TempU16 = 253;
                    }
                    OneLine.TX_DATA[9]  = TempU16;
                    OneLine.TX_DATA[10] = 70;
                    break;

                case 5:     /* 最大允许充电电流0.1A + SOH */
                    TempU16 = math_diveder((u32)Bat.CAP_RATING * 10, 4);
                    if (TempU16 > 511)
                    {
                        TempU16 = 511;
                    }
                    OneLine.TX_DATA[9] = TempU16 >> 1;

                    TempU32 = Soc.SOH;
                    if (TempU32 > 100)
                    {
                        TempU32 = 100;
                    }
                    OneLine.TX_DATA[10] = ((TempU16 & 0x01) << 7) |
                                          (TempU32 & 0x7F);
                    break;
            }

            /* DATA11：充电状态 */
            OneLine.TX_DATA[11] = 0x01;                    /* 充电允许 */
            OneLine.TX_DATA[11] |= SystemFlg.CRG_IN << 1; /* 充电连接 */

            if (SystemFlg.CRG_IN)
            {
                if (SocChargeState)
                {
                    OneLine.TX_DATA[11] |= 0x04;           /* 充电中 */
                }
                else if (SocFullChargeState)
                {
                    OneLine.TX_DATA[11] |= 0x08;           /* 充电完成 */
                }
            }

            /* DATA12~DATA14已清零，DATA15作为校验初始值 */
            OneLine.TX_DATA[15] = 0;

            /* 6种帧依次循环 */
            FrameType++;
            if (FrameType >= 6)
            {
                FrameType = 0;
            }
            OneLine.TX_TYPE = FrameType;

            /* 保留原发送状态机入口 */
            OneLine.TX_DATA[ONE_LINE_TX_DATA_NUM - 1] = 0;
            OneLine_Low();
            break;

        case 101:
        case 104:
        case 255:
            OneLine_Low();
            break;

        case 100:
            OneLine.TX_MASK  = 0x80;
            OneLine.TX_INDEX = 0;
            OneLine_High();
            OneLine.DELAY = 0;
            break;

        case 102:
            if (OneLine.TX_DATA[OneLine.TX_INDEX] & OneLine.TX_MASK)
            {
                OneLine_High();
            }
            break;

        case 103:
            OneLine.TX_STATE = 101;
            OneLine_High();
            if (OneLine.TX_MASK == 1)
            {
                OneLine.TX_DATA[ONE_LINE_TX_DATA_NUM - 1] ^=
                    OneLine.TX_DATA[OneLine.TX_INDEX];

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
            }
            break;
    }

    if (OneLine_is_Active)
    {
        SystemTick.SLEEP_CNT = 0;
    }
    else if (SystemFlg.SLEEPING)
    {
        OneLine.TX_STATE = 200;
    }
}
