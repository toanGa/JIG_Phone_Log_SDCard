/*
 * @file date_time.c
 * @brief Date and time management
 *
 * @section License
 *
 * Copyright (C) 2010-2017 Oryx Embedded SARL. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * @author Oryx Embedded SARL (www.oryx-embedded.com)
 * @version 1.7.8
 **/

//Dependencies
#include <stdio.h>
#include <string.h>
#include "date_time.h"
#include <stdint.h>
#include "uart_console.h"
#if defined(_WIN32)
   #include <time.h>
#endif


/**
 * @brief Convert Unix timestamp to date
 * @param[in] t Unix timestamp
 * @param[out] date Pointer to a structure representing the date and time
 **/

void
convertUnixTimeToDate(time_t t, RTC_DateTypeDef *date, RTC_TimeTypeDef *time )
{
	uint32_t a;
	uint32_t b;
	uint32_t c;
	uint32_t d;
	uint32_t e;
	uint32_t f;

	//Negative Unix time values are not supported
	if(t < 1)
		t = 0;

	//Clear milliseconds
	time->SubSeconds = 0;

	//Retrieve hours, minutes and seconds
	time->Seconds = t % 60;
	t /= 60;
	time->Minutes = t % 60;
	t /= 60;
	time->Hours = t % 24;
	t /= 24;

	//Convert Unix time to date
	a = (uint32_t) ((4 * t + 102032) / 146097 + 15);
	b = (uint32_t) (t + 2442113 + a - (a / 4));
	c = (20 * b - 2442) / 7305;
	d = b - 365 * c - (c / 4);
	e = d * 1000 / 30601;
	f = d - e * 30 - e * 601 / 1000;

	//January and February are counted as months 13 and 14 of the previous year
	if(e <= 13)
	{
		c -= 4716;
		e -= 1;
	}
	else
	{
		c -= 4715;
		e -= 13;
	}

	//Retrieve year, month and day
//	date->Year = c;
	// TODO: sub 2000 for stm32 rtc lib
	date->Year = c - 2000;
	date->Month = e;
	date->Date = f;

	//Calculate day of week
	date->WeekDay = computeDayOfWeek (c, e, f);

}


/**
 * @brief Convert date to Unix timestamp
 * @param[in] date Pointer to a structure representing the date and time
 * @return Unix timestamp
 **/


time_t
convertDateToUnixTime(RTC_DateTypeDef *date, RTC_TimeTypeDef *time )
{
   uint_t y;
   uint_t m;
   uint_t d;
   uint32_t t;

   //Year
//    y = date->Year;
   // TODO: Add 2000 for stm32 rtc lib
	y = date->Year + 2000;
   //Month of year
	m = date->Month;
   //Day of month
	d = date->Date;

   //January and February are counted as months 13 and 14 of the previous year
   if(m <= 2)
   {
      m += 12;
      y -= 1;
   }

   //Convert years to days
   t = (365 * y) + (y / 4) - (y / 100) + (y / 400);
   //Convert months to days
   t += (30 * m) + (3 * (m + 1) / 5) + d;
   //Unix time starts on January 1st, 1970
   t -= 719561;
   //Convert days to seconds
   t *= 86400;
   //Add hours, minutes and seconds
	t += (3600 * time->Hours) + (60 * time->Minutes) + time->Seconds;

   //Return Unix time
   return t;
}


/**
 * @brief Calculate day of week
 * @param[in] y Year
 * @param[in] m Month of year (in range 1 to 12)
 * @param[in] d Day of month (in range 1 to 31)
 * @return Day of week (in range 1 to 7)
 **/

uint8_t computeDayOfWeek(uint16_t y, uint8_t m, uint8_t d)
{
   uint_t h;
   uint_t j;
   uint_t k;

   //January and February are counted as months 13 and 14 of the previous year
   if(m <= 2)
   {
      m += 12;
      y -= 1;
   }

   //J is the century
   j = y / 100;
   //K the year of the century
   k = y % 100;

   //Compute H using Zeller's congruence
   h = d + (26 * (m + 1) / 10) + k + (k / 4) + (5 * j) + (j / 4);

   //Return the day of the week
   return ((h + 5) % 7) + 1;
}

#if 0
/*******************************************************************************
** @brief       today, yesterday, ...
**              GUI
** @param
** @return      None
** @time
** @revision
*******************************************************************************/
uint16_t* getTextDay(DateTime dateTime)
{
	static char textDay[20];
	static uint16_t textDayUni[20];
	static DateTime dateTimeNow;
	dateTimeNow = get_sys_date_time();
	uint8_t dayOfWeekNow =
			computeDayOfWeek(dateTimeNow.year, dateTimeNow.month, dateTimeNow.day);
	uint8_t dayOfCalWeek =
			computeDayOfWeek(dateTime.year, dateTime.month, dateTime.day);
	int8_t span = dayOfWeekNow - dayOfCalWeek;
	int32_t spanSecond =
			getCurrentUnixTime() - convertDateToUnixTime(&dateTime);
	// break if today, because time service can faster than time system some minute
	if(dayOfCalWeek == dayOfWeekNow)
	{
		if(spanSecond < 24*60*60)
		{
		    return text.today;
		}
	}

	// exception case
	if(dayOfWeekNow == 1 && dayOfCalWeek == 7)
	{
		if(spanSecond < 2*24*60*60)
		{
			return text.yesterday;
		}
	}
	if((spanSecond >= 0) && (spanSecond < 7*24*60*60) && (span > 0))
	{
		switch(span)
		{
		case 0:
			return text.today;
		case 1:
			return text.yesterday;
		case 2:
			return getNameDayOfWeek(dayOfWeekNow - 2);
		case 3:
			return getNameDayOfWeek(dayOfWeekNow - 3);
		case 4:
			return getNameDayOfWeek(dayOfWeekNow - 4);
		case 5:
			return getNameDayOfWeek(dayOfWeekNow - 5);
		case 6:
			return getNameDayOfWeek(dayOfWeekNow - 6);
		default:
			break;
		}
	}

	sprintf(textDay, "%d/%d/%d", dateTime.day, dateTime.month, dateTime.year%100);
	getUniFromStr(textDayUni, textDay);
	return textDayUni;
}


uint16_t* getNameDayOfWeek(uint8_t dayOfWeek)
{
	switch(dayOfWeek)
	{
	case 1:
		return text.monday;
	case 2:
		return text.tuesday;
	case 3:
		return text.wednesday;
	case 4:
		return text.thursday;
	case 5:
		return text.friday;
	case 6:
		return text.saturday;
	case 7:
		return text.sunday;
	default:
		return NULL;
	}
}

/*******************************************************************************
** @brief       MON, TUE, WED, THU, FRI, SAT, SUN
** @param
** @return      None
** @time        9:26 AM 3/20/2017
** @revision
*******************************************************************************/
char* getNameDayOfWeek_Clock(uint8_t dayOfWeek)
{
	static char nameDayOfWeek[20];
	switch(dayOfWeek)
	{
	case 1:
		strcpy(nameDayOfWeek, "MON");
		break;
	case 2:
		strcpy(nameDayOfWeek, "TUE");
		break;
	case 3:
		strcpy(nameDayOfWeek, "WED");
		break;
	case 4:
		strcpy(nameDayOfWeek, "THU");
		break;
	case 5:
		strcpy(nameDayOfWeek, "FRI");
		break;
	case 6:
		strcpy(nameDayOfWeek, "SAT");
		break;
	case 7:
		strcpy(nameDayOfWeek, "SUN");
		break;
	default:
		break;
	}
	return nameDayOfWeek;
}

/*******************************************************************************
** @brief       JAN, FEB, MAR, APR, MAY, JUN, JUL, AUG, SEP, OCT, NOV, DEC
** @param
** @return      None
** @time        9:26 AM 3/20/2017
** @revision
*******************************************************************************/
char* getNameOfMonth_Clock(uint8_t month)
{
	static char nameofMonth[20];
	switch(month)
	{
	case 1:
		strcpy(nameofMonth, "JAN");
		break;
	case 2:
		strcpy(nameofMonth, "FEB");
		break;
	case 3:
		strcpy(nameofMonth, "MAR");
		break;
	case 4:
		strcpy(nameofMonth, "APR");
		break;
	case 5:
		strcpy(nameofMonth, "MAY");
		break;
	case 6:
		strcpy(nameofMonth, "JUN");
		break;
	case 7:
		strcpy(nameofMonth, "JUL");
		break;
	case 8:
		strcpy(nameofMonth, "AUG");
		break;
	case 9:
		strcpy(nameofMonth, "SEP");
		break;
	case 10:
		strcpy(nameofMonth, "OCT");
		break;
	case 11:
		strcpy(nameofMonth, "NOV");
		break;
	case 12:
		strcpy(nameofMonth, "DEC");
		break;
	default:
		break;
	}
	return nameofMonth;
}

/*******************************************************************************
** @brief       20 MAY 2017
** @param
** @return      None
** @time        9:26 AM 3/20/2017
** @revision
*******************************************************************************/
char* getTextDate_Clock(uint8_t day, uint8_t month, uint16_t year)
{
	static char textDate[30];
	sprintf(textDate, "%d %s %d", day, getNameOfMonth_Clock(month), year);
	return textDate;
}

#endif
