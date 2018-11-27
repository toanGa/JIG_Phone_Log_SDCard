/*******************************************************************************
** @file:		uart_console.h
** @author:		HUY DAI
** @version:	V1.0.0
** @time:		11:00 AM Wednesday, 31 January, 2018
** @brief:		define debug header
** @revision:
**             	- version 1.0.1: <date> <name>
**             	<discribe the change>
**             	- version 1.0.1: <date> <name>
**             	<discribe the change>
*******************************************************************************/

#ifndef _UART_CONSOLE_H_
#define _UART_CONSOLE_H_

/******************************************************************************
**                               INCLUDES
**
*******************************************************************************/
#include "stm32l0xx_hal.h"


/******************************************************************************
**                               DEFINES
*******************************************************************************/								
#define USE_DEBUG   (1) // if use debug define =1 if do not use define =0
#define FULL_DEBUG  (1) // if use full debug then define =1,

#if USE_DEBUG    
#if FULL_DEBUG    
//use full debug
#define debug_printf(a,args...) UARTprintf("-Debug:%s [%s %d]\r\n\t"a,__func__,__FILE__,__LINE__,##args)

#else
// use debug normal
#define debug_printf(a,args...) UARTprintf(a,##args)

#endif

#else
// do not user debug
#define debug_printf(a,args...)     {}
#endif

#define SOS_DEBUG
//#define SOS_DEBUG          debug_printf


/******************************************************************************
**                     EXTERNAL VARIABLE DECLARATIONS
*******************************************************************************/

/******************************************************************************
**                      EXTERNAL FUNCTION PROTOTYPES
*******************************************************************************/
extern void UARTprintf( char *pcString, ...);
extern void UARTwrite( char *pcBuf, unsigned int len);


#endif
