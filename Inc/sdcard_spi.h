/*******************************************************************************
 ** @file:       sdcard_spi.c
 ** @author:     SPhone
 ** @version:    V1.0.0
 ** @time:       Nov 7, 2018
 ** @brief:		 
 ** @revision:
 **             	- version 1.0.1: <date> <name>
 **             	<discribe the change>
 ******************************************************************************/
#ifndef SDCARD_SPI_C_
#define SDCARD_SPI_C_

#include "ff_gen_drv.h"

/* MMC card type flags (MMC_GET_TYPE) */
#define CT_MMC      0x01        /* MMC ver 3 */
#define CT_SD1      0x02        /* SD ver 1 */
#define CT_SD2      0x04        /* SD ver 2 */
#define CT_SDC      (CT_SD1|CT_SD2) /* SD */
#define CT_BLOCK    0x08        /* Block addressing */

/* SDCARD block size */
#define SD_BLOCK_SIZE     512



void TM_FATFS_InitPins(void);
DSTATUS TM_FATFS_SD_disk_initialize (void);
DSTATUS TM_FATFS_SD_disk_status (void);
DRESULT TM_FATFS_SD_disk_read (BYTE *buff, DWORD sector, UINT count);

#endif /* SDCARD_SPI_C_ */
