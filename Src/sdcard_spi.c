/**
 * |----------------------------------------------------------------------
 * | Copyright (c) 2016 Tilen Majerle
 * |
 * | Permission is hereby granted, free of charge, to any person
 * | obtaining a copy of this software and associated documentation
 * | files (the "Software"), to deal in the Software without restriction,
 * | including without limitation the rights to use, copy, modify, merge,
 * | publish, distribute, sublicense, and/or sell copies of the Software,
 * | and to permit persons to whom the Software is furnished to do so,
 * | subject to the following conditions:
 * |
 * | The above copyright notice and this permission notice shall be
 * | included in all copies or substantial portions of the Software.
 * |
 * | THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * | EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * | OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
 * | AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * | HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * | WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * | FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * | OTHER DEALINGS IN THE SOFTWARE.
 * |----------------------------------------------------------------------
 */
#include <string.h>
#include "ff_gen_drv.h"
#include "sdcard_spi.h"
#include "uart_console.h"
#include "SDcard/sdcard.h"

/* DMA for STM32F4xx and STM32F7xx */
#if defined(STM32F4xx) || defined(STM32F7xx)
#include "tm_stm32_spi_dma.h"
#define FATFS_DMA           1
#else
#define FATFS_DMA           0
#endif


/* CS pin settings */
#ifndef FATFS_CS_PIN
#define FATFS_CS_PORT                       GPIOB
#define FATFS_CS_PIN                        GPIO_PIN_12
#endif

/* CS pin */
#define FATFS_CS_LOW                        HAL_GPIO_WritePin(SD_CSn_PIN_GPIO_Port, SD_CSn_PIN_Pin, GPIO_PIN_RESET)
#define FATFS_CS_HIGH                       HAL_GPIO_WritePin(SD_CSn_PIN_GPIO_Port, SD_CSn_PIN_Pin, GPIO_PIN_SET)


/* MMC/SD command */
#define CMD0    (0)         /* GO_IDLE_STATE */
#define CMD1    (1)         /* SEND_OP_COND (MMC) */
#define ACMD41  (0x80+41)   /* SEND_OP_COND (SDC) */
#define CMD8    (8)         /* SEND_IF_COND */
#define CMD9    (9)         /* SEND_CSD */
#define CMD10   (10)        /* SEND_CID */
#define CMD12   (12)        /* STOP_TRANSMISSION */
#define ACMD13  (0x80+13)   /* SD_STATUS (SDC) */
#define CMD16   (16)        /* SET_BLOCKLEN */
#define CMD17   (17)        /* READ_SINGLE_BLOCK */
#define CMD18   (18)        /* READ_MULTIPLE_BLOCK */
#define CMD23   (23)        /* SET_BLOCK_COUNT (MMC) */
#define ACMD23  (0x80+23)   /* SET_WR_BLK_ERASE_COUNT (SDC) */
#define CMD24   (24)        /* WRITE_BLOCK */
#define CMD25   (25)        /* WRITE_MULTIPLE_BLOCK */
#define CMD32   (32)        /* ERASE_ER_BLK_START */
#define CMD33   (33)        /* ERASE_ER_BLK_END */
#define CMD38   (38)        /* ERASE */
#define CMD55   (55)        /* APP_CMD */
#define CMD58   (58)        /* READ_OCR */

#define HSPI_SDCARD        hspi1
/////////////////////
static volatile DSTATUS TM_FATFS_SD_Stat = STA_NOINIT;  /* Physical drive status */
static BYTE TM_FATFS_SD_CardType;           /* Card type flags */
/////////////EXTERN VARIABLE///////////////////
extern SPI_HandleTypeDef HSPI_SDCARD;

///////////// REMAP FUNC ///////////////////

static uint8_t TM_SPI_Send(uint8_t data) {
    uint8_t d = 0xff;

    /* Set down counter */
    HAL_SPI_TransmitReceive(&HSPI_SDCARD, &data, &d, 1, 2);
//    HAL_SPI_Transmit(&HSPI_SDCARD, &data, 1, 2);
//    HAL_SPI_StateTypeDef status = HAL_SPI_GetState(&HSPI_SDCARD);
//    HAL_SPI_Receive(&HSPI_SDCARD, &d, 1, 2);
//    status = HAL_SPI_GetState(&HSPI_SDCARD);
    return d;
}

////////////////////////////////
/* SDCARD detect function */
static uint8_t SDCARD_IsDetected(void) {
#if FATFS_USE_DETECT_PIN
    /* Check if detected, pin LOW if detected */
    return !TM_GPIO_GetInputPinValue(FATFS_USE_DETECT_PIN_PORT, FATFS_USE_DETECT_PIN_PIN);
#endif

    /* Card is detected */
    return 1;
}

/* SDCARD write protect function */
static uint8_t SDCARD_IsWriteEnabled(void) {
#if FATFS_USE_WRITEPROTECT_PIN
    /* Check if write enabled, pin LOW if write enabled */
    return !TM_GPIO_GetInputPinValue(FATFS_USE_WRITEPROTECT_PIN_PORT, FATFS_USE_WRITEPROTECT_PIN_PIN);
#endif

    /* Card is not write protected */
    return 1;
}

/* Initialize MMC interface */
static void init_spi (void) {
    /* Init SPI */
    //TODO: init spi here

    // init in main by cubemx
    //TM_SPI_Init(FATFS_SPI, FATFS_SPI_PINSPACK);
#if FATFS_DMA
    TM_SPI_DMA_Init(FATFS_SPI);
#endif

    /* Set CS high */
    FATFS_CS_HIGH;

    /* Wait for stable */
    HAL_Delay(10);
//    int t = 1000000;
//    while(t--);
}

/* Receive multiple byte */
static void rcvr_spi_multi (
    BYTE *buff,     /* Pointer to data buffer */
    UINT btr        /* Number of bytes to receive (even number) */
)
{
    // TODO: change to transmit receive
    HAL_StatusTypeDef status = HAL_SPI_Receive(&HSPI_SDCARD, (uint8_t*)buff, btr, 1000);
    if(status != HAL_OK)
    {
        // TODO: wait complete
    }
}


#if _USE_WRITE
/* Send multiple byte */
static void xmit_spi_multi (
    const BYTE *buff,   /* Pointer to the data */
    UINT btx            /* Number of bytes to send (even number) */
)
{
    uint8_t dumyByte = 0xff;
    while(btx)
    {
        HAL_SPI_TransmitReceive(&HSPI_SDCARD, &dumyByte, (uint8_t*)buff, 1, 10);
        btx --;
        buff ++;
    }
}
#endif

/*-----------------------------------------------------------------------*/
/* Wait for card ready                                                   */
/*-----------------------------------------------------------------------*/
static int wait_ready ( /* 1:Ready, 0:Timeout */
    UINT wt         /* Timeout [ms] */
)
{
    BYTE d;

    /* Set down counter */
    uint32_t startTick;

    startTick = HAL_GetTick();
    do {
        d = TM_SPI_Send(0xFF);
    } while (d != 0xFF && (HAL_GetTick() - startTick < wt));    /* Wait for card goes ready or timeout */

    return ((d == 0xFF) && (HAL_GetTick() - startTick <= wt)) ? 1 : 0;
}

/*-----------------------------------------------------------------------*/
/* Deselect card and release SPI                                         */
/*-----------------------------------------------------------------------*/
static void deselect (void)
{
    FATFS_CS_HIGH;          /* CS = H */
    TM_SPI_Send(0xFF);           /* Dummy clock (force DO hi-z for multiple slave SPI) */
}

/*-----------------------------------------------------------------------*/
/* Select card and wait for ready                                        */
/*-----------------------------------------------------------------------*/
static int sd_select (void)    /* 1:OK, 0:Timeout */
{
    FATFS_CS_LOW;
    TM_SPI_Send(0xFF);   /* Dummy clock (force DO enabled) */

    if (wait_ready(500)) {
        return 1;   /* OK */
    }
    deselect();
    return 0;   /* Timeout */
}

/*-----------------------------------------------------------------------*/
/* Receive a data packet from the MMC                                    */
/*-----------------------------------------------------------------------*/
static int rcvr_datablock ( /* 1:OK, 0:Error */
    BYTE *buff,         /* Data buffer */
    UINT btr            /* Data block length (byte) */
)
{
    BYTE token;
    uint32_t startTick = HAL_GetTick();;
    //Timer1 = 200;
    do {                            // Wait for DataStart token in timeout of 200ms
        token = TM_SPI_Send(0xFF);
        // This loop will take a time. Insert rot_rdq() here for multitask envilonment.
    } while ((token == 0xFF) && (HAL_GetTick() - startTick < 200));
    if (token != 0xFE) {
        return 0;       // Function fails if invalid DataStart token or timeout
    }

    rcvr_spi_multi(buff, btr);      // Store trailing data to the buffer
    TM_SPI_Send(0xFF); TM_SPI_Send(0xFF);         // Discard CRC
    return 1;                       // Function succeeded
}

/*-----------------------------------------------------------------------*/
/* Send a data packet to the MMC                                         */
/*-----------------------------------------------------------------------*/
#if _USE_WRITE
static int xmit_datablock ( /* 1:OK, 0:Failed */
    const BYTE *buff,   /* Ponter to SD_BLOCK_SIZE byte data to be sent */
    BYTE token          /* Token */
)
{
    BYTE resp;
    if (!wait_ready(500)) {
        return 0;       /* Wait for card ready */
    }

    TM_SPI_Send(token);                  /* Send token */

    if (token != 0xFD) {                /* Send data if token is other than StopTran */
        xmit_spi_multi(buff, SD_BLOCK_SIZE);        /* Data */

        /* Dummy CRC */
        TM_SPI_Send(0xff);
        TM_SPI_Send(0xff);

        resp = TM_SPI_Send(0xFF);                /* Receive data resp */
        if ((resp & 0x1F) != 0x05)      /* Function fails if the data packet was not accepted */
            return 0;
    }
    return 1;
}
#endif

/*-----------------------------------------------------------------------*/
/* Send a command packet to the MMC                                      */
/*-----------------------------------------------------------------------*/
static BYTE send_cmd (      /* Return value: R1 resp (bit7==1:Failed to send) */
    BYTE cmd,       /* Command index */
    DWORD arg       /* Argument */
) {
    BYTE n, res;
    if (cmd & 0x80) {   /* Send a CMD55 prior to ACMD<n> */
        cmd &= 0x7F;
        res = send_cmd(CMD55, 0);
        if (res > 1) return res;
    }

    /* Select the card and wait for ready except to stop multiple block read */
    if (cmd != CMD12) {
        deselect();
        if (!sd_select()) return 0xFF;
    }

    TM_SPI_Send(0x40 | cmd);             /* Start + command index */
    TM_SPI_Send((BYTE)(arg >> 24));      /* Argument[31..24] */
    TM_SPI_Send((BYTE)(arg >> 16));      /* Argument[23..16] */
    TM_SPI_Send((BYTE)(arg >> 8));       /* Argument[15..8] */
    TM_SPI_Send((BYTE)arg);              /* Argument[7..0] */
    n = 0x01;                                       /* Dummy CRC + Stop */
    if (cmd == CMD0) n = 0x95;                      /* Valid CRC for CMD0(0) */
    if (cmd == CMD8) n = 0x87;                      /* Valid CRC for CMD8(0x1AA) */
    TM_SPI_Send(n);

    /* Receive command resp */
    if (cmd == CMD12) {
        TM_SPI_Send(0xFF);                   /* Diacard following one byte when CMD12 */
    }

    n = 10;    /* Wait for response (10 bytes max) */
    do {
        res = TM_SPI_Send(0xFF);
    } while ((res & 0x80) && --n);

    return res;                         /* Return received response */
}

void TM_FATFS_InitPins(void)
{
//    /* CS pin */
//    GPIO_InitTypeDef GPIO_InitStruct;
//
//    /* GPIO Ports Clock Enable */
//    __HAL_RCC_GPIOC_CLK_ENABLE();
//    __HAL_RCC_GPIOH_CLK_ENABLE();
//    __HAL_RCC_GPIOA_CLK_ENABLE();
//    __HAL_RCC_GPIOB_CLK_ENABLE();
//    __HAL_RCC_GPIOD_CLK_ENABLE();
//
//    /*Configure GPIO pin Output Level */
//    HAL_GPIO_WritePin(SD_CSn_PIN_GPIO_Port, SD_CSn_PIN_Pin, GPIO_PIN_RESET);
//
//    /*Configure GPIO pin : SD_CSn_PIN_Pin */
//    GPIO_InitStruct.Pin = SD_CSn_PIN_Pin;
//    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
//    GPIO_InitStruct.Pull = GPIO_PULLUP;
//    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
//    HAL_GPIO_Init(SD_CSn_PIN_GPIO_Port, &GPIO_InitStruct);

    HAL_GPIO_WritePin(SD_CSn_PIN_GPIO_Port, SD_CSn_PIN_Pin, GPIO_PIN_SET);
    /* Detect pin */
#if FATFS_USE_DETECT_PIN > 0
    TM_GPIO_Init(FATFS_DETECT_PORT, FATFS_DETECT_PIN, TM_GPIO_Mode_IN, TM_GPIO_OType_PP, TM_GPIO_PuPd_UP, TM_GPIO_Speed_Low);
#endif

    /* Write protect pin */
#if FATFS_USE_WRITEPROTECT_PIN > 0
    TM_GPIO_Init(FATFS_WRITEPROTECT_PORT, FATFS_WRITEPROTECT_PIN, TM_GPIO_Mode_IN, TM_GPIO_OType_PP, TM_GPIO_PuPd_UP, TM_GPIO_Speed_Low);
#endif
}

DSTATUS TM_FATFS_SD_disk_initialize (void) {
    BYTE n, cmd, ty, ocr[4];

    //Initialize CS pin
    TM_FATFS_InitPins();
    init_spi();

    if (!SDCARD_IsDetected()) {
        return STA_NODISK;
    }

    for (n = 10; n; n--) {
        TM_SPI_Send(0xFF);
    }
    ty = 0;
    if (send_cmd(CMD0, 0) == 1) {               /* Put the card SPI/Idle state */
//        TM_DELAY_SetTime2(1000);                /* Initialization timeout = 1 sec */
        if (send_cmd(CMD8, 0x1AA) == 1) {   /* SDv2? */
            for (n = 0; n < 4; n++) {
                ocr[n] = TM_SPI_Send(0xFF);  /* Get 32 bit return value of R7 resp */
            }
            if (ocr[2] == 0x01 && ocr[3] == 0xAA) {             /* Is the card supports vcc of 2.7-3.6V? */
                uint32_t startTick = HAL_GetTick();
                while ((HAL_GetTick() - startTick < 1000) && send_cmd(ACMD41, 1UL << 30)) ;   /* Wait for end of initialization with ACMD41(HCS) */
                if ((HAL_GetTick() - startTick < 1000) && send_cmd(CMD58, 0) == 0) {      /* Check CCS bit in the OCR */
                    for (n = 0; n < 4; n++) {
                        ocr[n] = TM_SPI_Send(0xFF);
                    }
                    ty = (ocr[0] & 0x40) ? CT_SD2 | CT_BLOCK : CT_SD2;  /* Card id SDv2 */
                }
            }
        } else {    /* Not SDv2 card */
            if (send_cmd(ACMD41, 0) <= 1)   {   /* SDv1 or MMC? */
                ty = CT_SD1; cmd = ACMD41;  /* SDv1 (ACMD41(0)) */
            } else {
                ty = CT_MMC; cmd = CMD1;    /* MMCv3 (CMD1(0)) */
            }
            uint32_t startTick = HAL_GetTick();
            while ((HAL_GetTick() - startTick < 1000) && send_cmd(cmd, 0));           /* Wait for end of initialization */
            if ((HAL_GetTick() - startTick < 1000) || send_cmd(CMD16, SD_BLOCK_SIZE) != 0) {  /* Set block length: SD_BLOCK_SIZE */
                ty = 0;
            }
        }
    }
    TM_FATFS_SD_CardType = ty;  /* Card type */
    deselect();

    if (ty) {           /* OK */
        TM_FATFS_SD_Stat &= ~STA_NOINIT;    /* Clear STA_NOINIT flag */
    } else {            /* Failed */
        TM_FATFS_SD_Stat = STA_NOINIT;
    }

    if (!SDCARD_IsWriteEnabled()) {
        TM_FATFS_SD_Stat |= STA_PROTECT;
    } else {
        TM_FATFS_SD_Stat &= ~STA_PROTECT;
    }

    return TM_FATFS_SD_Stat;
}

/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/
DSTATUS TM_FATFS_SD_disk_status (void) {
    /* Check card detect pin if enabled */
    if (!SDCARD_IsDetected()) {
        return STA_NOINIT;
    }

    /* Check if write is enabled */
    if (!SDCARD_IsWriteEnabled()) {
        TM_FATFS_SD_Stat |= STA_PROTECT;
    } else {
        TM_FATFS_SD_Stat &= ~STA_PROTECT;
    }

    return TM_FATFS_SD_Stat;    /* Return disk status */
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/
DRESULT TM_FATFS_SD_disk_read (
    BYTE *buff,     /* Data buffer to store read data */
    DWORD sector,   /* Sector address (LBA) */
    UINT count      /* Number of sectors to read (1..128) */
)
{
    uint8_t readStatus;
//    if (!SDCARD_IsDetected() || (TM_FATFS_SD_Stat & STA_NOINIT)) {
//        return RES_NOTRDY;
//    }

    if (count == 1) {   /* Single sector read */
        readStatus = SD_Read_Block(sector, buff);
    } else {                /* Multiple sector read */
        readStatus = SD_ReadMultiBlock(sector, buff, count);
    }
    if(readStatus != 0)
    {
       SOS_DEBUG("Read SD card fail\r\n");
    }
    else
    {
        SOS_DEBUG("Read SD card OK\r\n");
    }
    return (readStatus != 0) ? RES_ERROR : RES_OK;  /* Return result */
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/
#if _USE_WRITE
DRESULT TM_FATFS_SD_disk_write (
    const BYTE *buff,   /* Data to be written */
    DWORD sector,       /* Sector address (LBA) */
    UINT count          /* Number of sectors to write (1..128) */
) {
    uint8_t writeStatus;

//    if (!SDCARD_IsDetected()) {
//        return RES_ERROR;
//    }
//    if (!SDCARD_IsWriteEnabled()) {
//        return RES_WRPRT;
//    }
//    if (TM_FATFS_SD_Stat & STA_NOINIT) {
//        return RES_NOTRDY;  /* Check drive status */
//    }
//    if (TM_FATFS_SD_Stat & STA_PROTECT) {
//        return RES_WRPRT;   /* Check write protect */
//    }

    if (count == 1) {   /* Single sector write */
        writeStatus = SD_Write_Block(sector, (uint8_t*) buff);
    } else {                /* Multiple sector write */
        writeStatus = SD_WriteMultiBlock(sector, (uint8_t*) buff, count);
    }
    if(writeStatus != 0)
    {
       SOS_DEBUG("Write SD card fail\r\n");
    }
    else
    {
        SOS_DEBUG("Write SD card OK\r\n");
    }
    return (writeStatus != 0) ? RES_ERROR : RES_OK;  /* Return result */
}
#endif

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/
#if _USE_IOCTL
DRESULT TM_FATFS_SD_disk_ioctl (
    BYTE cmd,       /* Control code */
    void *buff      /* Buffer to send/receive control data */
)
{
    DRESULT res;
    BYTE n;
    BYTE* csd = SD_CSD;
    DWORD *dp, st, ed, csize;

//    if (TM_FATFS_SD_Stat & STA_NOINIT) {
//        return RES_NOTRDY;  /* Check if drive is ready */
//    }
//    if (!SDCARD_IsDetected()) {
//        return RES_NOTRDY;
//    }

    res = RES_ERROR;

    switch (cmd) {
        case CTRL_SYNC :        /* Wait for end of internal write process of the drive */
            if (sd_select()) res = RES_OK;
            break;

        /* Size in bytes for single sector */
        case GET_SECTOR_SIZE:
            *(WORD *)buff = SD_BLOCK_SIZE;
            res = RES_OK;
            break;

        case GET_SECTOR_COUNT : /* Get drive capacity in unit of sector (DWORD) */
            {
                if ((csd[0] >> 6) == 1) {   /* SDC ver 2.00 */
                    csize = csd[9] + ((WORD)csd[8] << 8) + ((DWORD)(csd[7] & 63) << 16) + 1;
                    *(DWORD*)buff = csize << 10;
                } else {                    /* SDC ver 1.XX or MMC ver 3 */
                    n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
                    csize = (csd[8] >> 6) + ((WORD)csd[7] << 2) + ((WORD)(csd[6] & 3) << 10) + 1;
                    *(DWORD*)buff = csize << (n - 9);
                }
                res = RES_OK;
            }
            break;

        case GET_BLOCK_SIZE :   /* Get erase block size in unit of sector (DWORD) */
            if (SD_CardType == SD_STD_CAPACITY_SD_CARD_V2_0) {    /* SDC ver 2.00 */
                *(DWORD*)buff = 16UL << (csd[10] >> 4);
                res = RES_OK;
            } else {                    /* SDC ver 1.XX or MMC */
                if (SD_CardType & SD_STD_CAPACITY_SD_CARD_V1_0) {    /* SDC ver 1.XX */
                    *(DWORD*)buff = (((csd[10] & 63) << 1) + ((WORD)(csd[11] & 128) >> 7) + 1) << ((csd[13] >> 6) - 1);
                } else {                    /* MMC */
                    *(DWORD*)buff = ((WORD)((csd[10] & 124) >> 2) + 1) * (((csd[11] & 3) << 3) + ((csd[11] & 224) >> 5) + 1);
                }
                res = RES_OK;
            }
            break;

//        case CTRL_ERASE_SECTOR :    /* Erase a block of sectors (used when _USE_ERASE == 1) */
        case CTRL_TRIM:
//            if (!(TM_FATFS_SD_CardType & CT_SDC)) break;                /* Check if the card is SDC */
//            if (TM_FATFS_SD_disk_ioctl(MMC_GET_CSD, csd)) break;    /* Get CSD */
//            if (!(csd[0] >> 6) && !(csd[10] & 0x40)) break; /* Check if sector erase can be applied to the card */
//            dp = buff; st = dp[0]; ed = dp[1];              /* Load sector block */
//            if (!(TM_FATFS_SD_CardType & CT_BLOCK)) {
//                st *= SD_BLOCK_SIZE; ed *= SD_BLOCK_SIZE;
//            }
//            if (send_cmd(CMD32, st) == 0 && send_cmd(CMD33, ed) == 0 && send_cmd(CMD38, 0) == 0 && wait_ready(30000))   /* Erase sector block */
//                res = RES_OK;   /* FatFs does not check result of this command */
            // TODO: write for TRIM command
            break;

        default:
            res = RES_PARERR;
    }

    deselect();

    return res;
}
#endif

/******************************** End of file *********************************/
