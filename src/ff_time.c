//Copyright 2016 by Walter Zimmer
//
#include <time.h>
#include "kinetis.h"
#include "ff.h"
/*---------------------------------------------------------*/
/* User Provided Timer Function for FatFs module           */
/*---------------------------------------------------------*/
#define EPOCH_YEAR 2000 //T3 RTC
#define LEAP_YEAR(Y) (((EPOCH_YEAR+Y)>0) && !((EPOCH_YEAR+Y)%4) && ( ((EPOCH_YEAR+Y)%100) || !((EPOCH_YEAR+Y)%400) ) )
static  const uint8_t monthDays[]={31,28,31,30,31,30,31,31,30,31,30,31}; 
/*  int  tm_sec;
  int tm_min;
  int tm_hour;
  int tm_mday;
  int tm_mon;
  int tm_year;
  int tm_wday;
  int tm_yday;
  int tm_isdst;
*/

struct tm seconds2tm(uint32_t tt)
{ struct tm tm;
  tm.tm_sec = tt % 60;
  tt /= 60; // now it is minutes
  tm.tm_min = tt % 60;
  tt /= 60; // now it is hours
  tm.tm_hour = tt % 24;
  tt /= 24; // now it is days
  tm.tm_wday = ((tt + 4) % 7) + 1;  // Sunday is day 1 

  // tt is now days since EPOCH_Year (1970)
  uint32_t year = 0;  
  uint32_t days = 0;
  while((unsigned)(days += (LEAP_YEAR(year) ? 366 : 365)) <= tt) year++;

  tm.tm_year = year; // year is offset from 1970 

  // correct for last (actual) year
  days -= (LEAP_YEAR(year) ? 366 : 365);
  tt  -= days; // now tt is days in this year, starting at 0
  
  uint32_t month=0;
  uint32_t monthLength=0;
  for (month=0; month<12; month++) 
  {
    monthLength = monthDays[month];
    if ((month==1) & LEAP_YEAR(year)) monthLength++; 
    if (tt<monthLength) break;
    tt -= monthLength;
  }
  tm.tm_mon = month + 1;  // jan is month 1  
  tm.tm_mday = tt + 1;     // day of month
  return tm;
}

DWORD get_fattime (void)
{
  struct tm tm=seconds2tm(RTC_TSR);
  
	/* Pack date and time into a DWORD variable */
	return	  (((DWORD)tm.tm_year-10) << 25)
			| ((DWORD)tm.tm_mon << 21)
			| ((DWORD)tm.tm_mday << 16)
			| ((DWORD)tm.tm_hour << 11)
			| ((DWORD)tm.tm_min << 5)
			| ((DWORD)tm.tm_sec >> 1);
}
