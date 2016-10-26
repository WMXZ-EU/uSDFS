/*
 * WMXZ Teensy uSDFS library
 * Copyright (c) 2016 Walter Zimmer.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
//ff_time.c
//
#include <time.h>
#include "kinetis.h"
#include "ff.h"
#include "ff_utils.h"

/*---------------------------------------------------------*/
/* User Provided Timer Function for FatFs module           */
/*---------------------------------------------------------*/
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


TCHAR * char2tchar( char * charString, size_t nn, TCHAR * tcharString)
{ int ii;
  for(ii = 0; ii<nn; ii++) tcharString[ii] = (TCHAR) charString[ii];
  return tcharString;
}

char * tchar2char(  TCHAR * tcharString, size_t nn, char * charString)
{ int ii;
  for(ii = 0; ii<nn; ii++) charString[ii] = (char) tcharString[ii];
  return charString;
}

