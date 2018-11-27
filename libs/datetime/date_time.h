/*******************************************************************************
 ** @file:       date_time.h
 ** @author:     nhan
 ** @version:    V1.0.0
 ** @time:       Jul 21, 2017
 ** @brief:		 
 ** @revision:
 **             	- version 1.0.1: <date> <name>
 **             	<discribe the change>
 ******************************************************************************/

#ifndef MIDDLEWARE_TIME_SERVICE_FN_DATE_TIME_H_
#define MIDDLEWARE_TIME_SERVICE_FN_DATE_TIME_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 **                               INCLUDES
 ******************************************************************************/
#include <time.h>
#include "stm32l0xx_hal.h"
/*******************************************************************************
 **                               DEFINES
 ******************************************************************************/

typedef char char_t;
typedef signed int int_t;
typedef unsigned int uint_t;
typedef uint32_t systime_t;

/*******************************************************************************
 **                     EXTERNAL VARIABLE DECLARATIONS
 ******************************************************************************/

/*******************************************************************************
 **                     EXTERNAL FUNCTION DECLARATIONS
 ******************************************************************************/
void
convertUnixTimeToDate(time_t t, RTC_DateTypeDef *date, RTC_TimeTypeDef *time );
time_t
convertDateToUnixTime(RTC_DateTypeDef *date, RTC_TimeTypeDef *time );
uint8_t
computeDayOfWeek(uint16_t y, uint8_t m, uint8_t d );

#ifdef __cplusplus
}
#endif

#endif /* MIDDLEWARE_TIME_SERVICE_FN_DATE_TIME_H_ */

/******************************** End of file *********************************/
