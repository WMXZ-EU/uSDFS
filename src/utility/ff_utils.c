
//#include <time.h>

#if defined(__IMXRT1052__) || (__IMXRT1062__)
    #include "imxrt.h"
#else
    #include "kinetis.h"
#endif

#include "../ff.h"

#ifndef HAVETM
#define HAVETM
	typedef struct tm
	{ int  tm_sec;
	  int tm_min;
	  int tm_hour;
	  int tm_mday;
	  int tm_mon;
	  int tm_year;
	  int tm_wday;
	  int tm_yday;
	  int tm_isdst;
	} tm_t;
#endif


const char *fileSystem[] = {"No FS", "FS_FAT12","FS_FAT16","FS_FAT32","FS_EXFAT"};

const char *FR_ERROR_STRING[] = {
	"FR_OK",				/* (0) Succeeded */
	"FR_DISK_ERR",			/* (1) A hard error occurred in the low level disk I/O layer */
	"FR_INT_ERR",				/* (2) Assertion failed */
	"FR_NOT_READY",			/* (3) The physical drive cannot work */
	"FR_NO_FILE",				/* (4) Could not find the file */
	"FR_NO_PATH",				/* (5) Could not find the path */
	"FR_INVALID_NAME",		/* (6) The path name format is invalid */
	"FR_DENIED",				/* (7) Access denied due to prohibited access or directory full */
	"FR_EXIST",				/* (8) Access denied due to prohibited access */
	"FR_INVALID_OBJECT",		/* (9) The file/directory object is invalid */
	"FR_WRITE_PROTECTED",		/* (10) The physical drive is write protected */
	"FR_INVALID_DRIVE",		/* (11) The logical drive number is invalid */
	"FR_NOT_ENABLED",			/* (12) The volume has no work area */
	"FR_NO_FILESYSTEM",		/* (13) There is no valid FAT volume */
	"FR_MKFS_ABORTED",		/* (14) The f_mkfs() aborted due to any problem */
	"FR_TIMEOUT",				/* (15) Could not get a grant to access the volume within defined period */
	"FR_LOCKED",				/* (16) The operation is rejected according to the file sharing policy */
	"FR_NOT_ENOUGH_CORE",		/* (17) LFN working buffer could not be allocated */
	"FR_TOO_MANY_OPEN_FILES",	/* (18) Number of open files > FF_FS_LOCK */
	"FR_INVALID_PARAMETER"	/* (19) Given parameter is invalid */
};

const char *STAT_ERROR_STRING[] = {
	"STA_OK", //		0x00	/* No error */
	"STA_NOINIT", //		0x01	/* Drive not initialized */
	"STA_NODISK", //		0x02	/* No medium in the drive */
	"STA_UNKNOWN", //		0x03	/* unknown error*/
	"STA_PROTECT" //		0x04	/* Write protected */
};


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
