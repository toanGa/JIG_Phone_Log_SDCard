/*******************************************************************************
 ** @file:       SchedNonOS.c
 ** @author:     SPhone
 ** @version:    V1.0.0
 ** @time:       Nov 21, 2018
 ** @brief:		 
 ** @revision:
 **             	- version 1.0.1: <date> <name>
 **             	<discribe the change>
 ******************************************************************************/
// include
#include "main.h"
#include "stm32l0xx_hal.h"
#include "fatfs.h"

/* USER CODE BEGIN Includes */
#include "string.h"
#include "uart_console.h"
#include "SDCard/sdcard.h"
#include "ExProtocol/fifo8.h"

// define

// variable
extern UART_HandleTypeDef hlpuart1;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern RTC_HandleTypeDef  hrtc;

extern uint8_t            uart1_rx;
extern uint8_t            uart2_rx;
extern uint8_t            lpuart1_rx;
extern FF8                ff8_uart1;
extern uint8_t            ff8_uart1_buff[];
extern FF8                ff8_uart2;
extern uint8_t            ff8_uart2_buff[];

extern RTC_TimeTypeDef sTime;
extern RTC_DateTypeDef sDate;

int g_bSemLPUART1 = 0;
int g_bSemUART1   = 0;
int g_bSemUART2   = 0;

// static variable - for optimize stack size
FRESULT result;
int cntOmapLog = 0;
int cntSTmLog = 0;
uint8_t byteRecev;
uint8_t byteSTM, byteSTM_Old;
uint8_t byteOMAP, byteOMAP_Old;
// function
void NonOS_sheduler()
{
//#define STMLogFile  USERFile
    /* init code for FATFS */
    MX_FATFS_Init();

    // enable switch uart omap and stm
    HAL_GPIO_WritePin(ENB_CONV_GPIO_Port, ENB_CONV_Pin, GPIO_PIN_SET);

    /* USER CODE BEGIN 5 */
	result = f_mount (&USERFatFS, USERPath, 1);
	assert_param(result == FR_OK);
	result = f_open_write_append (&USERFile, "OMAP_LOG");
	assert_param(result == FR_OK);
	result = f_open_write_append (&STMLogFile, "STM_LOG");
	assert_param(result == FR_OK);
    // verify driver OK
//    assert(result == FR_OK);

	result = f_puts (
			(TCHAR*) "\r\n\r\n------------ Start LOG OMAP ------------\r\n\r\n",
			&USERFile);
	result = f_puts (
			(TCHAR*) "\r\n\r\n------------ Start LOG STM ------------\r\n\r\n",
			&STMLogFile);
    // TODO: task save log to file

    while(1)
    {
        // comment for optimaze loop check
//        if(g_bSemLPUART1)
//        {
//            g_bSemLPUART1 = 0;
//        }
		if(g_bSemUART1)
        {
			// clear flag first
			g_bSemUART1 = 0;

			while (FF8_Pop (&ff8_uart1, &byteRecev))
			{
				byteSTM_Old = byteSTM;
				byteSTM     = byteRecev;

				if((byteSTM_Old == '\r') || (byteSTM_Old == '\n'))
				{
					if((byteSTM != '\r') && (byteSTM != '\n'))
					{
						HAL_RTC_GetDate (&hrtc, &sDate, RTC_FORMAT_BIN);
						HAL_RTC_GetTime (&hrtc, &sTime, RTC_FORMAT_BIN);
						f_printf(&STMLogFile, "[%02d:%02d:%02d-%02d:%02d:%02d]   ", sDate.Date, sDate.Month, sDate.Year, sTime.Hours, sTime.Minutes, sTime.Seconds);
					}
				}
				f_putc ((TCHAR) byteRecev, &STMLogFile);
				cntSTmLog++;
				if (cntSTmLog == 1000)
				{
					result = f_sync (&STMLogFile);
					cntSTmLog = 0;
					assert_param(result == FR_OK);
					// todo: check if result is fail
				}
            }

        }
		if (g_bSemUART2)// OMAP LOG
		{
			// clear flag first
			g_bSemUART2 = 0;
			while (FF8_Pop (&ff8_uart2, &byteRecev))
			{
				byteOMAP_Old = byteOMAP;
				byteOMAP     = byteRecev;
				if((byteOMAP_Old == '\r') || (byteOMAP_Old == '\n'))
				{
					if((byteOMAP != '\r') && (byteOMAP != '\n'))
					{
						HAL_RTC_GetDate (&hrtc, &sDate, RTC_FORMAT_BIN);
						HAL_RTC_GetTime (&hrtc, &sTime, RTC_FORMAT_BIN);
						f_printf(&USERFile, "[%02d:%02d:%02d-%02d:%02d:%02d]   ", sDate.Date, sDate.Month, sDate.Year, sTime.Hours, sTime.Minutes, sTime.Seconds);
					}
				}
				f_putc ((TCHAR) byteRecev, &USERFile);
				cntOmapLog++;
				if (cntOmapLog == 1000)
				{
					result = f_sync (&USERFile);
					cntOmapLog = 0;
					assert_param(result == FR_OK);
					// todo: check if result is fail
				}
			}
        }
    }
}


/******************************** End of file *********************************/
