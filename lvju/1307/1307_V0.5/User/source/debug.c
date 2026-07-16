/**
 * debug.c — 绿驹一线通 C0 新协议适配
 * 基于 oneline.c 的 oneline_data_send() 修改
 * 只改 case 0 里的数据填充，其余完全不动
 */

#include "main.h"
#include "init.h"
#include "soc.h"
#include "flash.h"
#include "toolfun.h"
#include "hwparam.h"
#include "userfun.h"


void oneline_data_send(void)  // 500us once
{
    u16 TempU16;

    switch (OneLine.TX_STATE++)
    {
        case 0:
            if (OneLine.TX_TYPE == 0)
            {
                // ===== 帧0: 模式0(电压) + km格式 =====

                OneLine.TX_DATA[0] = 0x50;
                OneLine.TX_DATA[1] = 0xE0;  // 流水号0xE + 铅酸0

                // DATA2: D7-5=0(电压) D4-1=额定电压 D0=里程第9位
                TempU16 = AimaParam.LAST_TRIP;
                {
                    u8 rv = Bat.CELL_NUM - 2;
                    if (rv < 1) rv = 1;
                    if (rv > 7) rv = 7;
                    OneLine.TX_DATA[2] = (rv << 1) | ((TempU16 >> 8) & 1);
                }

                // DATA3: 里程低8位
                OneLine.TX_DATA[3] = TempU16 & 0xFF;

                // DATA4: D7-1=容量% D0=0
                {
                    u8 pct = (Soc.SHOW * 100) / SOC_SHOW_MAX;
                    if (pct > 100) pct = 100;
                    OneLine.TX_DATA[4] = (pct & 0x7F) << 1;
                }

                // DATA5: D7=0(放电) D6-0=额定容量
                OneLine.TX_DATA[5] = Bat.CAP_RATING & 0x7F;

                // DATA6-7: 电机功率 0.1kW
                if (SystemParam.BUS_CURRENT > 0)
                {
                    OneLine.TX_DATA[6] = 0;
                    OneLine.TX_DATA[7] = 0;
                }
                else
                {
                    TempU16  = abs(SystemParam.BUS_CURRENT) >> 6;
                    TempU16 *= SystemParam.BUS_VOLTAGE >> 6;
                    TempU16  = math_diveder(TempU16, 100);
                    OneLine.TX_DATA[6] = TempU16 >> 4;
                    OneLine.TX_DATA[7] = (TempU16 & 0x0F) << 4;
                }

                // DATA8: 预留
                OneLine.TX_DATA[8] = 0;

                // DATA9-10: 总电压 0.1V
                TempU16 = (SystemParam.BUS_VOLTAGE * 10) >> 6;
                OneLine.TX_DATA[9]  = TempU16 & 0xFF;
                OneLine.TX_DATA[10] = (TempU16 >> 8) & 0x0F;

                // DATA11: 保护状态
                OneLine.TX_DATA[11] = 1;
                OneLine.TX_DATA[11] += SystemFlg.CRG_IN << 1;
                if (SystemFlg.CRG_IN)
                {
                    if (SocChargeState)
                        OneLine.TX_DATA[11] += 0x04;
                    else if (SocFullChargeState)
                        OneLine.TX_DATA[11] += 0x08;
                }

                // DATA12-14: 预留
                OneLine.TX_DATA[12] = 0;
                OneLine.TX_DATA[13] = 0;
                OneLine.TX_DATA[14] = 0;

                OneLine.TX_TYPE = 0x80;
            }
            else
            {
                OneLine.TX_TYPE = 0x80;

                // ===== 帧1: 模式1(容量) + 时间格式 =====

                OneLine.TX_DATA[0] = 0x50;
                OneLine.TX_DATA[1] = 0xE0;  // 流水号0xE + 铅酸0

                // DATA2: D7-5=1(容量) D4-1=额定电压 D0=1(时间格式)
                {
                    u8 rv = Bat.CELL_NUM - 2;
                    if (rv < 1) rv = 1;
                    if (rv > 7) rv = 7;
                    OneLine.TX_DATA[2] = (1 << 5) | (rv << 1) | 1;
                }

                // DATA3-4: 剩余充电时间(12bit)
                TempU16 = OneLine.chglft;
                OneLine.TX_DATA[3] = TempU16 & 0xFF;
                OneLine.TX_DATA[4] = ((TempU16 >> 8) & 0x0F) << 4;

                // DATA5: D7=1(充电) D6-0=额定容量
                OneLine.TX_DATA[5] = 0x80 | (Bat.CAP_RATING & 0x7F);

                // DATA6-7: 电机功率 0.1kW
                if (SystemParam.BUS_CURRENT < 0)
                {
                    OneLine.TX_DATA[6] = 0;
                    OneLine.TX_DATA[7] = 0;
                }
                else
                {
                    TempU16  = abs(SystemParam.BUS_CURRENT) >> 6;
                    TempU16 *= SystemParam.BUS_VOLTAGE >> 6;
                    TempU16  = math_diveder(TempU16, 10);
                    OneLine.TX_DATA[6] = TempU16 >> 4;
                    OneLine.TX_DATA[7] = (TempU16 & 0x0F) << 4;
                }

                // DATA8: 预留
                OneLine.TX_DATA[8] = 0;

                // DATA9-10: 剩余容量Ah
                OneLine.TX_DATA[9]  = math_diveder(Cap.QNOW * Soc.SHOW, 8 * SOC_SHOW_MAX);
                OneLine.TX_DATA[10] = 0;

                // DATA11: 保护状态
                OneLine.TX_DATA[11] = 1;
                OneLine.TX_DATA[11] += SystemFlg.CRG_IN << 1;
                if (SystemFlg.CRG_IN)
                {
                    if (SocChargeState)
                        OneLine.TX_DATA[11] += 0x04;
                    else if (SocFullChargeState)
                        OneLine.TX_DATA[11] += 0x08;
                }

                // DATA12-14: 预留
                OneLine.TX_DATA[12] = 0;
                OneLine.TX_DATA[13] = 0;
                OneLine.TX_DATA[14] = 0;

                OneLine.TX_TYPE = 0;
            }

            OneLine.TX_DATA[ONE_LINE_TX_DATA_NUM - 1] = 0;  // 校验初始值
            OneLine_Low();
            break;

        case 101:
        case 104:
        case 255:
            OneLine_Low();
            break;

        case 100:
            OneLine.TX_MASK   = 0x80;
            OneLine.TX_INDEX  = 0;
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
