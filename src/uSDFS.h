#ifndef uSDFS_H
#define uSDFS_H 
#define uSDFS_VER "23_Oct_23_20_53"

#include "ff.h"
#include "utility/sd_config.h"

#if FF_USE_STRFUNC != 1
#error "FF_USE_STRFUNC != 1 in ffconf.h"
#endif

#if FF_CODE_PAGE != 850
#error "F_CODE_PAGE != 850 in ffconf.h"
#endif

#if FF_USE_LFN	!= 1
#error "FF_USE_LFN	!= 1 in ffconf.h"
#endif

#if FF_FS_RPATH	!= 1
#error "FF_FS_RPATH	!= 1"
#endif

#if FF_VOLUMES != 10
#error "FF_VOLUMES	!= 10 in ffconf.h"
#endif

#if FF_MULTI_PARTITION != 1
#error "FF_MULTI_PARTITION != 1 in ffconf.h"
#endif

#if FF_FS_EXFAT	!= 1
#error "FF_FS_EXFAT	!= 1 in ffconf.h"
#endif

#ifndef MY_VOL_TO_PART
	#define MY_VOL_TO_PART
	#if FF_MULTI_PARTITION		/* Multiple partition configuration */ 
		PARTITION VolToPart[] = {{DEV_SPI, 0}, //{ physical drive number, Partition: 0:Auto detect, 1-4:Forced partition)} 
								 {DEV_SDHC,0}, 
								 {DEV_USB, 0}, 
								 {DEV_USB, 1}, 
								 {DEV_USB, 2}
								 }; /* Volume - Partition resolution table */
	#endif
#endif

extern const char *STAT_ERROR_STRING[] ;
extern const char *FR_ERROR_STRING[] ;
extern const char *fileSystem[] ;


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

#endif
