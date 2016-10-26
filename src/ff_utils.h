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
//ff_utils.h
//
#ifndef _FF_UTILS_H
#define _FF_UTILS_H
#include <time.h>
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

#ifdef __cplusplus
extern "C" {
#endif
struct tm seconds2tm(uint32_t tt);

DWORD get_fattime (void);


TCHAR * char2tchar( char * charString, size_t nn, TCHAR * tcharString);
char * tchar2char(  TCHAR * tcharString, size_t nn, char * charString);

#ifdef __cplusplus
}
#endif


#endif