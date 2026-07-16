#ifndef __flash_h
#define __flash_h

#include "gpm32f13xx_nvm.h"
#include "type.h"

#define		FLASH_DEFAULT			0xFFFFFFFF
#define		BOOT_MODE_EN			0x5A5A5A5A
#define		WATCHDOG_RST			0xA5A5A5A5

#define		WRITE_START_ADDR	0x01000000
#define		FLASH_CODE_MAX		0x0001B800

#define		ERT_DATA_ADDR			0x0101DE00
#define		SOC_DATA_ADDR			0x0101DC00
#define		PNN_DATA_ADDR			0x0101DA00
#define		RCD_DATA_ADDR			0x0101D800

#define		BOOT_UART_ADDR		(ERT_DATA_ADDR + 4)
#define		UART_CTRL_ADDR		(ERT_DATA_ADDR + 8)
#define		UART_BDRT_ADDR		(ERT_DATA_ADDR + 0x0C)

#define		SECTOR_NUM_MAX		25
#define		ERT_DATA_NUM			22
#define		SOC_DATA_NUM			25
#define		PNN_DATA_NUM			13

#define		ERT_PN_NUMBER			14
#define		CST_PN_NUMBER			20
#define		UID_NUMBER				12

typedef struct{
	u32 UID[3];
	u8 ERT[ERT_PN_NUMBER];
	u8 CST[CST_PN_NUMBER];
}pn_type;

#if (ERT_DATA_NUM > SECTOR_NUM_MAX) || (SOC_DATA_NUM > SECTOR_NUM_MAX) || (PNN_DATA_NUM > SECTOR_NUM_MAX)
#error "Any data number must be less than SECTOR_NUM_MAX"
#endif

#if (ERT_PN_NUMBER + CST_PN_NUMBER + UID_NUMBER > PNN_DATA_NUM*4)
#error "PNN_DATA_NUM	is too small"
#endif

extern void flash_read(void), ert_save(u8 boottype, UART_Type *UARTx, u8 watchdog_rst, u32 baud_rate), pn_save(void);
extern void flash_data_save_process(void), soc_save(void), record_data_save(void);
extern pn_type PN;

#endif
