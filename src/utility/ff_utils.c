
#include <time.h>

#if defined(__IMXRT1052__) || (__IMXRT1062__)
    #include "imxrt.h"
#else
    #include "kinetis.h"
#endif

#include "../ff.h"

/*  
  int  tm_sec;
  int tm_min;
  int tm_hour;
  int tm_mday;
  int tm_mon;
  int tm_year;
  int tm_wday;
  int tm_yday;
  int tm_isdst;
*/
// following routines are based on http://howardhinnant.github.io/date_algorithms.html
struct tm seconds2tm(uint32_t tt)
{ struct tm tx;
  tx.tm_sec   = tt % 60;    tt /= 60; // now it is minutes
  tx.tm_min   = tt % 60;    tt /= 60; // now it is hours
  tx.tm_hour  = tt % 24;    tt /= 24; // now it is days
  tx.tm_wday  = ((tt + 4) % 7) ;      // Sunday is day 0
  //
  tt += 719468;
  uint32_t era = (tt >= 0 ? tt : tt - 146096) / 146097;
  uint32_t doe = (tt - era * 146097);                             // [0, 146096]
  uint32_t yoe = (doe - doe/1460 + doe/36524 - doe/146096) / 365; // [0, 399]
  uint32_t yy = (yoe) + era * 400;
  uint32_t doy = doe - (365*yoe + yoe/4 - yoe/100);            // [0, 365]
  uint32_t mp = (5*doy + 2)/153;                               // [0, 11]
  uint32_t dd = doy - (153*mp+2)/5 + 1;                        // [1, 31]
  uint32_t mm = mp + (mp < 10 ? 3 : -9);                       // [1, 12]
//
  tx.tm_year=yy + (mm <= 2);
  tx.tm_mday=dd;
  tx.tm_mon=mm;
  return tx;
}

uint32_t tm2seconds(struct tm tx)
{
  uint32_t yy = tx.tm_year;
  uint32_t mm = tx.tm_mon;
  uint32_t dd = tx.tm_mday;
  
  yy -= mm <= 2;
  uint32_t era = (yy >= 0 ? yy : yy-399) / 400;
  uint32_t yoe = (yy - era * 400);      // [0, 399]
  uint32_t doy = (153*(mm + (mm > 2 ? -3 : 9)) + 2)/5 + dd-1;  // [0, 365]
  uint32_t doe = yoe * 365 + yoe/4 - yoe/100 + doy;         // [0, 146096]

  uint32_t tt = era * 146097 + (doe) - 719468;
  tt *= (24*3600);
  tt += (tx.tm_hour*60+tx.tm_min)*60+tx.tm_sec;

  return tt;
}

DWORD get_fattime (void)
{
    struct tm tx=seconds2tm(rtc_get());

    /* Pack date and time into a DWORD variable */
    return   (((DWORD)tx.tm_year-1980) << 25)
            | ((DWORD)tx.tm_mon << 21)
            | ((DWORD)tx.tm_mday << 16)
            | ((DWORD)tx.tm_hour << 11)
            | ((DWORD)tx.tm_min << 5)
            | ((DWORD)tx.tm_sec >> 1);
}

struct tm decode_fattime (uint16_t td, uint16_t tt)
{
    struct tm tx;
	tx.tm_year= (td>>9) + 1980;
	tx.tm_mon= (td>>5) & (16-1);
	tx.tm_mday= td& (32-1);
	
	tx.tm_hour= tt>>11;
	tx.tm_min= (tt>>5) & (64-1);
	tx.tm_sec= 2*(tt& (32-1));
	return tx;

    /* Pack date and time into a DWORD variable */
	/*
    return   (((DWORD)tx.tm_year-60) << 25)
            | ((DWORD)tx.tm_mon << 21)
            | ((DWORD)tx.tm_mday << 16)
            | ((DWORD)tx.tm_hour << 11)
            | ((DWORD)tx.tm_min << 5)
            | ((DWORD)tx.tm_sec >> 1);
			*/
}
