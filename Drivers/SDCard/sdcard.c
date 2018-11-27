#include "sdcard.h"

//#include <stm32f4xx_hal_gpio.h>
//#include <stm32f4xx_hal_rcc.h>
//#include <stm32f4xx_hal_spi.h>

//#include "main.h"
#include "ff_gen_drv.h"
#include "uart_console.h"
/////
#define HSPI_SDCARD        hspi1
#define MISO_PORT          GPIOA
#define MISO_PIN           GPIO_PIN_6


/* CS pin */
#define SD_CS_LOW                        HAL_GPIO_WritePin(SD_CSn_PIN_GPIO_Port, SD_CSn_PIN_Pin, GPIO_PIN_RESET)
#define SD_CS_HIGH                       HAL_GPIO_WritePin(SD_CSn_PIN_GPIO_Port, SD_CSn_PIN_Pin, GPIO_PIN_SET)

/////
extern SPI_HandleTypeDef HSPI_SDCARD;


uint8_t  SD_CardType = SD_UNKNOWN_SD_CARD;         // SD Card type
uint32_t SD_CardCapacity;                          // SD Card capacity
uint8_t  SD_MaxBusClkFreq = 0;                     // Max. card bus frequency
uint8_t  SD_MID = 0;                               // SD Card Maunufacturer ID
uint16_t SD_OID = 0;                               // SD Card OEM/Application ID


uint8_t  SD_CSD[16];                               // CSD buffer
uint8_t  SD_CID[16];                               // CID buffer
//uint8_t  SD_sector[512];                           // sector buffer
uint16_t SD_CRC16_rcv;                             // last received CRC16
uint16_t SD_CRC16_cmp;                             // last computed CRC16

int SD_WaitReadyByHWPin(int timeout)
{
    int cnt = 0;
    uint32_t startTick = HAL_GetTick();

    while(HAL_GPIO_ReadPin(MISO_PORT, MISO_PIN) == 0)
    {
        cnt ++;
        if(HAL_GetTick() - startTick > timeout)
        {
            cnt = -1;
            break;
        }
    }
    return cnt;
}

int SD_WaitReadyByCommand(int timeout)
{
    int cnt = 0;
    uint32_t startTick = HAL_GetTick();
    uint8_t readVal = 0;

    while(readVal != 0xff)
    {
        readVal = SD_SendRecv(0xff);
        if(HAL_GetTick() - startTick > timeout)
        {
            cnt = -1;
            break;
        }
        cnt++;
    }
    return cnt;
}

static uint8_t CRC7_one(uint8_t t, uint8_t data) {
    const uint8_t g = 0x89;
    uint8_t i;

    t ^= data;
    for (i = 0; i < 8; i++) {
        if (t & 0x80) t ^= g;
        t <<= 1;
    }

    return t;
}

uint8_t CRC7_buf(const uint8_t * p, uint8_t len) {
    uint8_t j,crc = 0;

    for (j = 0; j < len; j++) crc = CRC7_one(crc,p[j]);

    return crc >> 1;
}

static uint16_t CRC16_one(uint16_t crc, uint8_t ser_data) {
    crc  = (uint8_t)(crc >> 8)|(crc << 8);
    crc ^= ser_data;
    crc ^= (uint8_t)(crc & 0xff) >> 4;
    crc ^= (crc << 8) << 4;
    crc ^= ((crc & 0xff) << 4) << 1;

    return crc;
}

uint16_t CRC16_buf(const uint8_t * p, uint32_t len) {
    uint32_t i;
    uint16_t crc = 0;

    for (i = 0; i < len; i++) crc = CRC16_one(crc,p[i]);

    return crc;
}
void SD_SPI_Init(uint16_t prescaler) {
//    SPI_InitTypeDef SPI;
//    SPI.SPI_Mode = SPI_Mode_Master;
//    SPI.SPI_BaudRatePrescaler = prescaler;
//    SPI.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
//    SPI.SPI_CPOL = SPI_CPOL_High;
//    SPI.SPI_CPHA = SPI_CPHA_2Edge;
//    SPI.SPI_CRCPolynomial = 7;
//    SPI.SPI_DataSize = SPI_DataSize_8b;
//    SPI.SPI_FirstBit = SPI_FirstBit_MSB;
//    SPI.SPI_NSS = SPI_NSS_Soft;
//    SPI_Init(SD_SPI,&SPI);
//
//    // NSS must be set to '1' due to NSS_Soft settings (otherwise it will be Multimaster mode).
//    SPI_NSSInternalSoftwareConfig(SD_SPI,SPI_NSSInternalSoft_Set);
}

void SD_Init(void) {
//#if _SD_SPI == 1
//    // SPI1
//    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1 | RCC_APB2Periph_GPIOA,ENABLE);
//#elif _SD_SPI == 2
//    // SPI2
//    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
//    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2,ENABLE);
//#elif _SD_SPI == 3
//    // SPI3
//    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO,ENABLE);
//    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3,ENABLE);
//    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE); // Disable JTAG for use PB3
//#endif
//    GPIO_InitTypeDef PORT;
//    // Configure SPI pins
//    PORT.GPIO_Speed = GPIO_Speed_50MHz;
//    PORT.GPIO_Pin = SD_SCK_PIN | SD_MISO_PIN | SD_MOSI_PIN;
//    PORT.GPIO_Mode = GPIO_Mode_AF_PP;
//    GPIO_Init(SD_PORT,&PORT);
//    // Configure CS pin as output with Push-Pull
//    PORT.GPIO_Pin = SD_CS_PIN;
//    PORT.GPIO_Mode = GPIO_Mode_Out_PP;
//    GPIO_Init(SD_CS_PORT,&PORT);
//
//    SD_SPI_Init(SPI_BaudRatePrescaler_256); // set SPI at lowest possible speed
//    SPI_Cmd(SD_SPI,ENABLE);

    SD_CS_HIGH; // pull SD pin -> Deselect SD
}

uint8_t SD_SendRecv(uint8_t data) {
    uint8_t miso;
//    while (SPI_I2S_GetFlagStatus(SD_SPI,SPI_I2S_FLAG_TXE) == RESET);
//    SPI_I2S_SendData(SD_SPI,data);
//    while (SPI_I2S_GetFlagStatus(SD_SPI,SPI_I2S_FLAG_RXNE) == RESET);
//    miso = SPI_I2S_ReceiveData(SD_SPI);

//    HAL_SPI_Transmit(&HSPI_SDCARD, &data, 1, 10);
//    HAL_SPI_StateTypeDef status = HAL_SPI_GetState(&HSPI_SDCARD);
//    HAL_SPI_Receive(&HSPI_SDCARD, &miso, 1, 10);
//    status = HAL_SPI_GetState(&HSPI_SDCARD);
	SD_CS_LOW;
    HAL_SPI_TransmitReceive(&HSPI_SDCARD, &data, &miso, 1, 10);
	SD_CS_HIGH;
    return miso;
}

uint8_t SD_SendCmd(uint8_t cmd, uint32_t arg) {
    uint8_t wait, response, crc = 0;

    SD_SendRecv(0xff); // This dummy send necessary for some cards

    // Send: [8b]Command -> [32b]Argument -> [8b]CRC
    SD_SendRecv(cmd | 0x40);
    crc = CRC7_one(crc,cmd | 0x40);
    SD_SendRecv(arg >> 24);
    crc = CRC7_one(crc,arg >> 24);
    SD_SendRecv(arg >> 16);
    crc = CRC7_one(crc,arg >> 16);
    SD_SendRecv(arg >> 8);
    crc = CRC7_one(crc,arg >> 8);
    SD_SendRecv(arg);
    crc = CRC7_one(crc,arg);
    SD_SendRecv(crc | 0x01); // Bit 1 always must be set to "1" in CRC

    // Wait for response from SD Card
    wait = 0;
    while ((response = SD_SendRecv(0xff)) == 0xff) if (wait++ > 200) break;

    return response;
}

uint16_t SD_SendCmd_2(uint8_t cmd, uint32_t arg)
{
    uint8_t r2a;
    uint8_t r2b;
    uint16_t r2 = 0;;
    uint8_t wait, crc = 0;

    SD_SendRecv(0xff); // This dummy send necessary for some cards

    // Send: [8b]Command -> [32b]Argument -> [8b]CRC
    SD_SendRecv(cmd | 0x40);
    crc = CRC7_one(crc,cmd | 0x40);
    SD_SendRecv(arg >> 24);
    crc = CRC7_one(crc,arg >> 24);
    SD_SendRecv(arg >> 16);
    crc = CRC7_one(crc,arg >> 16);
    SD_SendRecv(arg >> 8);
    crc = CRC7_one(crc,arg >> 8);
    SD_SendRecv(arg);
    crc = CRC7_one(crc,arg);
    SD_SendRecv(crc | 0x01); // Bit 1 always must be set to "1" in CRC

    // Wait for response from SD Card
    wait = 0;
    while ((r2a = SD_SendRecv(0xff)) == 0xff) if (wait++ > 200) break;
    while ((r2b = SD_SendRecv(0xff)) == 0xff) if (wait++ > 200) break;
    r2 = r2a << 8;
    r2 |= r2b;

    return r2;
}

uint8_t SD_SendCmd_n(uint8_t cmd, uint32_t arg, uint8_t* resp, uint32_t n)
{
    uint8_t wait, crc = 0;
    uint32_t i;

    SD_SendRecv(0xff); // This dummy send necessary for some cards

    // Send: [8b]Command -> [32b]Argument -> [8b]CRC
    SD_SendRecv(cmd | 0x40);
    crc = CRC7_one(crc,cmd | 0x40);
    SD_SendRecv(arg >> 24);
    crc = CRC7_one(crc,arg >> 24);
    SD_SendRecv(arg >> 16);
    crc = CRC7_one(crc,arg >> 16);
    SD_SendRecv(arg >> 8);
    crc = CRC7_one(crc,arg >> 8);
    SD_SendRecv(arg);
    crc = CRC7_one(crc,arg);
    SD_SendRecv(crc | 0x01); // Bit 1 always must be set to "1" in CRC

    // Wait for response from SD Card
    for(i = 0; i < n; i++)
    {
        wait = 0;
        while ((resp[i] = SD_SendRecv(0xff)) == 0xff)
        {
            if (wait++ > 200)
                break;
        }
    }

    return wait < 200 ? 1: 0;
}

// return values:
//   0xff - SD card timeout
//   0xfe - Unknown or bad SD/MMC card
//   0xfd - Set block size command failed
//   0xfc - bad response for CMD58
//   0xfb - SDv2 pattern mismatch (in response to CMD8)
uint8_t SD_CardInit(void) {
    uint32_t wait = 0, r3;
    uint8_t response;

    SD_CS_LOW; // pull CS to low

    // Must send at least 74 clock ticks to SD Card
    for (wait = 0; wait < 8; wait++) SD_SendRecv(0xff);

    // Software SD Card reset
    wait = 0; response = 0x00;
    while (wait < 0x20 && response != 0x01) {
        // Wait for SD card enters idle state (R1 response = 0x01)
        response = SD_SendCmd(SD_CMD_GO_IDLE_STATE,0x00);
        wait++;
    }
    if (wait >= 0x20 && response != 0x01) return 0xff; // SD card timeout

    // CMD8: SEND_IF_COND. Send this command to verify SD card interface operating condition
    /* Argument: - [31:12]: Reserved (shall be set to '0')
                 - [11:8]: Supply Voltage (VHS) 0x1 (Range: 2.7-3.6 V)
                 - [7:0]: Check Pattern (recommended 0xAA) */
    response = SD_SendCmd(SD_CMD_HS_SEND_EXT_CSD,SD_CHECK_PATTERN); // CMD8

    if (response == 0x01) {
        // SDv2 or later
        // Read R7 responce
        r3 =  SD_SendRecv(0xff) << 24;
        r3 |= SD_SendRecv(0xff) << 16;
        r3 |= SD_SendRecv(0xff) << 8;
        r3 |= SD_SendRecv(0xff);

        if ((r3 & 0x01ff) != (SD_CHECK_PATTERN & 0x01ff)) return 0xfb; // SDv2 pattern mismatch -> unsupported SD card

        // CMD55: Send leading command for ACMD<n> command.
        // CMD41: APP_SEND_OP_COND. For only SDC - initiate initialization process.
        wait = 0; response = 0xff;
        while (++wait < 0x2710 && response != 0x00) {
            SD_SendCmd(SD_CMD_APP_CMD,0); // CMD55
            response = SD_SendCmd(SD_CMD_SD_APP_OP_COND,0x40000000); // ACMD41: HCS flag set
        }
        if (wait >= 0x2710 || response != 0x00) return 0xff; // SD card timeout

        SD_CardType = SD_STD_CAPACITY_SD_CARD_V2_0; // SDv2;

        // Read OCR register
        response = SD_SendCmd(SD_CMD_READ_OCR,0x00000000); // CMD58
        if (response == 0x00) {
            // Get R3 response
            r3  = SD_SendRecv(0xff) << 24;
            r3 |= SD_SendRecv(0xff) << 16;
            r3 |= SD_SendRecv(0xff) << 8;
            r3 |= SD_SendRecv(0xff);
        } else {
            SD_CardType = SD_UNKNOWN_SD_CARD;
            return 0xfc; // bad CMD58 response
        }
        if (r3 & (1<<30)) SD_CardType = SD_HIGH_CAPACITY_SD_CARD; // SDHC or SDXC
    } else {
        // SDv1 or MMC
        wait = 0; response = 0xff;
        while (++wait < 0xfe) {
            SD_SendCmd(SD_CMD_APP_CMD,0); // CMD55
            response = SD_SendCmd(SD_CMD_SD_APP_OP_COND,0x00000000); // CMD41
            if (response == 0x00) {
                SD_CardType = SD_STD_CAPACITY_SD_CARD_V1_0; // SDv1
                break;
            }
        }

        if (response == 0x05 && wait >= 0xfe) {
            // MMC or bad card
            // CMD1: Initiate initialization process.
            wait = 0; response = 0xff;
            while (++wait < 0xfe) {
                response = SD_SendCmd(SD_CMD_SEND_OP_COND,0x00000000); // CMD1
                if (response == 0x00) {
                    SD_CardType = SD_MULTIMEDIA_CARD; // MMC
                    break;
                }
            }
        }
    }

    if (SD_CardType == 0) return 0xfe; // Unknown or bad SD/MMC card

    // Set SPI to higher speed
    // TODO
    //SD_SPI_Init(SPI_BaudRatePrescaler_8);

    // Turn off CRC
    response = SD_SendCmd(SD_CMD_CRC_ON_OFF,0x00000001); // CMD59

    // For SDv2,SDv1,MMC must set block size. For SDHC/SDXC it fixed to 512.
    if ((SD_CardType == SD_STD_CAPACITY_SD_CARD_V1_0) ||
        (SD_CardType == SD_STD_CAPACITY_SD_CARD_V2_0) ||
        (SD_CardType == SD_MULTIMEDIA_CARD))
    {
        response = SD_SendCmd(SD_CMD_SET_BLOCKLEN,0x00000200); // CMD16: block size = 512 bytes
        if (response != 0x00) return 0xfd; // Set block size failed
    }

    SD_CS_HIGH; // pull CS to high

    return 0;
}

// return:
// 0x00 -- read OK
// 0x01..0xfe -- error response to CMD9
// 0xff -- timeout
uint8_t SD_Read_CSD(void) {
    uint32_t wait;
    uint8_t i, response;

    SD_CS_LOW; // pull CS to low

    response = SD_SendCmd(SD_CMD_SEND_CSD,0); // CMD9
    if (response != 0x00) {
        // Something wrong happened, fill buffer with zeroes
        for (i = 0; i < 16; i++) SD_CSD[i] = 0x00;
        return response;
    } else {
        wait = 0; response = 0;
        while (++wait <= 0x1ff && response != 0xfe) response = SD_SendRecv(0xff);
        if (wait >= 0x1ff) return 0xff;
        // Read 16 bytes of CSD register
        for (i = 0; i < 16; i++) SD_CSD[i] = SD_SendRecv(0xff);
    }

    // Receive 16-bit CRC (some cards demand this)
    SD_CRC16_rcv  = SD_SendRecv(0xff) << 8;
    SD_CRC16_rcv |= SD_SendRecv(0xff);

    // // Calculate CRC16 of received buffer
    SD_CRC16_cmp = CRC16_buf(&SD_CSD[0],16);

    SD_CS_HIGH; // pull CS to high

    // Parse some stuff from CID
    SD_MaxBusClkFreq = SD_CSD[3];
    uint32_t c_size,c_size_mult;
    if (SD_CardType != SD_MULTIMEDIA_CARD) {
        if (SD_CSD[0] >> 6 == 0x01) {
            // CSD Version 2.0
            c_size  = (SD_CSD[7] & 0x3f) << 16;
            c_size |= SD_CSD[8] << 8;
            c_size |= SD_CSD[7];
            SD_CardCapacity = c_size << 9; // = c_size * 512
        } else {
            // CSD Version 1.0
            c_size  = (SD_CSD[6] & 0x03) << 10;
            c_size |= (uint32_t)SD_CSD[7] << 2;
            c_size |= SD_CSD[8] >> 6;
            c_size_mult  = (SD_CSD[9] & 0x03) << 1;
            c_size_mult |= SD_CSD[10] >> 7;
            SD_CardCapacity = c_size << 10;
        }
    } else {
        SD_CardCapacity = 0;
    }

    return 0;
}

// return:
// 0x00 -- read OK
// 0x01..0xfe -- error response to CMD10
// 0xff -- timeout
uint8_t SD_Read_CID(void) {
    uint32_t wait;
    uint8_t i, response;

    SD_CS_LOW; // pull CS to low

    response = SD_SendCmd(SD_CMD_SEND_CID,0); // CMD10
    if (response != 0x00) {
        // Something wrong happened, fill buffer with zeroes
        for (i = 0; i < 16; i++) SD_CID[i] = 0x00;
        return response;
    } else {
        wait = 0; response = 0;
        while (++wait <= 0x1ff && response != 0xfe) response = SD_SendRecv(0xff);
        if (wait >= 0x1ff) return 0xff;
        // Read 16 bytes of CID register
        for (i = 0; i < 16; i++) SD_CID[i] = SD_SendRecv(0xff);
    }

    // Receive 16-bit CRC (some cards demand this)
    SD_CRC16_rcv  = SD_SendRecv(0xff) << 8;
    SD_CRC16_rcv |= SD_SendRecv(0xff);

    // // Calculate CRC16 of received buffer
    SD_CRC16_cmp = CRC16_buf(&SD_CID[0],16);

    SD_CS_HIGH; // pull CS to high

    return 0;
}

// return:
// 0x00 -- read OK
// 0x01..0xfe -- error response from CMD17
// 0xff -- timeout
uint8_t SD_Read_Block(uint32_t addr, uint8_t* buff) {
    uint32_t wait;
    uint16_t i;
    uint8_t response;

    int waitval = SD_WaitReadyByCommand(1000);
    if(waitval == -1)
    {
        SOS_DEBUG("Timeout wait CMD\r\n");
    }
    else
    {
        SOS_DEBUG("Wait CMD: %d\r\n", waitval);
    }

    SD_CS_LOW; // pull CS to low

    if (SD_CardType != SD_HIGH_CAPACITY_SD_CARD) addr <<= 9; // Convert block number to byte offset
    response = SD_SendCmd(SD_CMD_READ_SINGLE_BLOCK,addr); // CMD17
    if (response != 0x00) {
        // Something wrong happened, fill buffer with zeroes
        for (i = 0; i < 512; i++) buff[i] = 0;
        return response; // SD_CMD_READ_SINGLE_BLOCK command returns bad response
    } else {
        wait = 0; response = 0;
        while (++wait <= 0x1ff && response != 0xfe) response = SD_SendRecv(0xff);
        if (wait >= 0x1ff) return 0xff;
        // Read 512 bytes of sector
        for (i = 0; i < 512; i++) buff[i] = SD_SendRecv(0xff);
    }

    // Receive 16-bit CRC (some cards demand this)
    SD_CRC16_rcv  = SD_SendRecv(0xff) << 8;
    SD_CRC16_rcv |= SD_SendRecv(0xff);

    // // Calculate CRC16 of received buffer
    SD_CRC16_cmp = CRC16_buf(&buff[0],512);
    if(SD_CRC16_cmp != SD_CRC16_rcv)
    {
        SOS_DEBUG("CRC error when read block\r\n");
    }
    SD_CS_HIGH; // pull CS to high

    return 0;
}

// return:
// 0x00 -- write OK
// 0x01..0xfe -- error response from CMD24
// 0xff -- timeout
uint8_t SD_Write_Block(uint32_t addr, uint8_t* buff)
{
    uint32_t wait;
    uint16_t i;
    uint8_t response;
    uint8_t status;

    int waitval = SD_WaitReadyByCommand(1000);
    if(waitval == -1)
    {
        SOS_DEBUG("Timeout wait CMD\r\n");
    }
    else
    {
        SOS_DEBUG("Wait CMD: %d\r\n", waitval);
    }

    SD_CS_LOW; // pull CS to low

    if (SD_CardType != SD_HIGH_CAPACITY_SD_CARD) addr <<= 9; // Convert block number to byte offset
    response = SD_SendCmd(SD_CMD_WRITE_SINGLE_BLOCK,addr); // CMD24
    if (response != 0x00) {
        SD_CS_HIGH;
        return response; // SD_CMD_WRITE_SINGLE_BLOCK command returns bad response
    } else {
        wait = 0; response = 0;
        while (++wait <= 0x1ff && response != 0xff) response = SD_SendRecv(0xff);
        if (wait >= 0x1ff)
        {
            SD_CS_HIGH;
            return 0xff;
        }
        // Write start token
        SD_SendRecv(SD_TOKEN_STARTBLK_WRITE_SINGGLE);

        // Write 512 bytes of sector
        for (i = 0; i < 512; i++)
            SD_SendRecv(buff[i]);
        // send 2 byte CRC
        SD_CRC16_cmp = CRC16_buf(&buff[0], 512);
        SD_SendRecv(SD_CRC16_cmp >> 8) ;
        SD_SendRecv(SD_CRC16_cmp & 0xff);

        // read data respond token
        response = SD_SendRecv(0xff);
        status = (response & 0x0F) >> 1;
        switch(status)
        {
            case 2:
                SOS_DEBUG("Data accepted\r\n");
                break;
            case 5:
                SOS_DEBUG("Data rejected due to a CRC error\r\n");
                break;
            case 6:
                SOS_DEBUG("Data Rejected due to a Write Error\r\n");
                break;
            default:
                SOS_DEBUG("Write block with unknow error\r\n");
                break;
        }
    }


    // send CMD13 to check programming error
    uint16_t program_respond = SD_SendCmd_2(SD_CMD_SEND_STATUS, 0x00);
    if(program_respond != 0)
    {
        SOS_DEBUG("Program data ERROR: %d\r\n", program_respond);
    }


    SD_CS_HIGH;
    return 0;
}

// return:
// 0x00 -- write OK
// 0x01..0xfe -- error response from CMD24
// 0xff -- timeout
uint8_t SD_WriteMultiBlock(uint32_t startBlkAddr, uint8_t* buff, uint32_t numsBlk)
{
    uint32_t  wait;
    uint16_t  i;
    uint8_t   response;
    uint8_t   status;
    uint32_t  count = numsBlk;

    SD_CS_LOW; // pull CS to low

    if (SD_CardType != SD_HIGH_CAPACITY_SD_CARD) startBlkAddr <<= 9; // Convert block number to byte offset

    // predefine number sector
    SD_SendCmd(SD_CMD_APP_CMD, 0x00); // CMD55
    SD_SendCmd(SD_CMD_SD_APP_SET_WR_BLK_ERASE_COUNT, numsBlk & 0x07FFFF);

    response = SD_SendCmd(SD_CMD_WRITE_MULT_BLOCK, startBlkAddr); // CMD24
    if (response != 0x00) {
        SD_CS_HIGH;
        return response; // SD_CMD_WRITE_SINGLE_BLOCK command returns bad response
    } else {


        while(count)
        {
            //wait for ready
             wait = 0; response = 0;
             while (++wait <= 0x1ff && response != 0xff) response = SD_SendRecv(0xff);
             if (wait >= 0x1ff)
             {
                 SD_CS_HIGH;
                 return 0xff;
             }

            // Write start token
            response = SD_SendRecv(SD_TOKEN_STARTBLK_WRITE_MULTI);

            // Write 512 bytes of sector
            for (i = 0; i < 512; i++)
                SD_SendRecv(buff[i]);
            // send 2 byte CRC
            SD_CRC16_cmp = CRC16_buf(buff, 512);
            SD_SendRecv(SD_CRC16_cmp >> 8) ;
            SD_SendRecv(SD_CRC16_cmp & 0xff);

            // read data respond token
            response = SD_SendRecv(0xff);
            status = (response & 0x0F) >> 1;
            switch(status)
            {
                case 2:
                    SOS_DEBUG("Data accepted\r\n");
                    break;
                case 5:
                    SOS_DEBUG("Data rejected due to a CRC error\r\n");
                    break;
                case 6:
                    SOS_DEBUG("Data Rejected due to a Write Error\r\n");
                    break;
                default:
                    SOS_DEBUG("Write block with unknow error\r\n");
                    break;
            }

            buff += 512;
            count --;
        }
    }

    // TODO: busy ?
//    wait = 0; response = 0;
//    while (++wait <= 0x1ff && response != 0xff) response = SD_SendRecv(0xff);
//    if (wait >= 0x1ff)
//    {
//        SD_CS_HIGH;
//        return 0xff;
//    }

    // send byte stop transmission
    response = SD_SendRecv(SD_TOKEN_STOP_MULTI_WRITE);

    // TODO: busy ?
//    wait = 0; response = 0;
//    while (++wait <= 0x1ff && response != 0xff) response = SD_SendRecv(0xff);
//    if (wait >= 0x1ff)
//    {
//        SD_CS_HIGH;
//        return 0xff;
//    }
#if 0 // TODO: need check error when write
    // send CMD13 to check programming error
    SD_SendCmd(SD_CMD_APP_CMD, 0x00); // CMD55
    uint8_t resp[6]; // = 4byte-nums block written + 2 byte crc
    uint32_t blkWritten;
    SD_SendCmd_n(SD_CMD_SD_APP_SEND_NUM_WRITE_BLOCKS, 0x00, resp, sizeof(resp));
    blkWritten = ((uint32_t)resp[0] << 24) |
                 ((uint32_t)resp[1] << 16) |
                 ((uint32_t)resp[2] << 8) |
                 (uint32_t)resp[3];
    if(blkWritten == numsBlk)
    {
        SOS_DEBUG("Write multi block success\r\n");
    }
    else
    {
        SOS_DEBUG("Write multi block fail. Number blocks written = blkWritten\r\n");
    }
#endif
    SD_CS_HIGH;
    return 0;
}

// return:
// 0x00 -- write OK
// 0x01..0xfe -- error response from CMD24
// 0xff -- timeout
uint8_t SD_ReadMultiBlock(uint32_t startBlkAddr, uint8_t* buff, uint32_t numsBlk)
{
    uint32_t   wait;
    uint16_t   i;
    uint8_t    response;
    uint32_t   count = numsBlk;

    SD_CS_LOW; // pull CS to low

    if (SD_CardType != SD_HIGH_CAPACITY_SD_CARD) startBlkAddr <<= 9; // Convert block number to byte offset
    response = SD_SendCmd(SD_CMD_READ_MULT_BLOCK,startBlkAddr); // CMD17
    if (response != 0x00) {
        // Something wrong happened, fill buffer with zeroes
        for (i = 0; i < 512; i++) buff[i] = 0;
        return response; // SD_CMD_READ_SINGLE_BLOCK command returns bad response
    } else {
        while(count)
        {
            wait = 0; response = 0;
            while (++wait <= 0x1ff && response != 0xfe) response = SD_SendRecv(0xff);
            if (wait >= 0x1ff) return 0xff;

            // Read 512 bytes of sector
            for (i = 0; i < 512; i++) buff[i] = SD_SendRecv(0xff);

            // Receive 16-bit CRC (some cards demand this)
            SD_CRC16_rcv  = SD_SendRecv(0xff) << 8;
            SD_CRC16_rcv |= SD_SendRecv(0xff);

            // // Calculate CRC16 of received buffer
            SD_CRC16_cmp = CRC16_buf(&buff[0],512);
            if(SD_CRC16_cmp != SD_CRC16_rcv)
            {
                // TODO: may be break here
                SOS_DEBUG("CRC error when read block\r\n");
            }

            buff += 512;
            count --;
        }
    }

    response = SD_SendCmd(SD_CMD_STOP_TRANSMISSION, 0x00);
    uint8_t resp = SD_SendRecv(0xff);
    SD_CS_HIGH; // pull CS to high
    return 0;
}

/////////// my sdcard driver
uint8_t MySD_SPI_Trasmit(uint8_t cmd)
{
    HAL_StatusTypeDef status = HAL_SPI_Transmit(&HSPI_SDCARD, &cmd, 1, 0);
    return (status == HAL_OK);
}

uint8_t MySD_SPI_TrasmitReceive(uint8_t trans)
{
    uint8_t recv = 0xff;
    HAL_StatusTypeDef status = HAL_SPI_TransmitReceive(&HSPI_SDCARD, &trans, &recv, 1, 10);
    return recv;
}

uint8_t MySD_SendCmd(uint8_t cmd, uint32_t arg) {
    uint8_t wait, response, crc = 0;

    //MySD_SPI_Trasmit(0xff); // This dummy send necessary for some cards

    // Send: [8b]Command -> [32b]Argument -> [8b]CRC
    MySD_SPI_Trasmit(cmd | 0x40);
    crc = CRC7_one(crc,cmd | 0x40);
    MySD_SPI_Trasmit(arg >> 24);
    crc = CRC7_one(crc,arg >> 24);
    MySD_SPI_Trasmit(arg >> 16);
    crc = CRC7_one(crc,arg >> 16);
    MySD_SPI_Trasmit(arg >> 8);
    crc = CRC7_one(crc,arg >> 8);
    MySD_SPI_Trasmit(arg);
    crc = CRC7_one(crc,arg);
    MySD_SPI_Trasmit(crc | 0x01); // Bit 1 always must be set to "1" in CRC

    // Wait for response from SD Card
    wait = 0;
    while ((response = MySD_SPI_TrasmitReceive(0xff)) == 0xff) if (wait++ > 200) break;

    return response;
}

uint8_t My_SDInit()
{
    SD_CS_LOW;
    uint8_t dummy = 0xff;
    uint8_t recv;
    uint16_t cycleCheck = 1000;
    uint8_t wait;

    // switch to SPI mode
    SD_CS_HIGH;
    SD_CS_LOW;
    MySD_SendCmd(SD_CMD_GO_IDLE_STATE, 0x00);
    SD_CS_HIGH;

    // wait 74 cycle clock when first startup
    for (wait = 0; wait < 10; wait++)
    {
        HAL_SPI_Transmit(&HSPI_SDCARD, &dummy, 1, 10);
    }

    // force module to idle -> check status register
    do
    {
        SD_CS_LOW;

        MySD_SendCmd(SD_CMD_APP_CMD, 0x00);
        MySD_SendCmd(SD_CMD_SD_APP_OP_COND, 0x00);

        // get result in R1
        HAL_SPI_Receive(&HSPI_SDCARD, &recv, 1, 1000);
        cycleCheck --;
        SD_CS_HIGH;
    }
    while(recv != 0x01 && cycleCheck);

    // send 0xff for 74 clocks
    for (wait = 0; wait < 10; wait++)
    {
        SD_CS_LOW;
        HAL_SPI_Transmit(&HSPI_SDCARD, &dummy, 1, 10);


        SD_CS_HIGH;
    }

    SD_CS_HIGH;
    if(recv == 0x00)
    {
        return 1;
    }
    return 0;
}
