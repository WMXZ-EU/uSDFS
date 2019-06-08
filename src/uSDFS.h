#ifndef uSDFS_H
#define uSDFS_H 
#define uSDFS_VER "03_Jun_19_21_40"

#include "ff.h"
#include "utility/sd_config.h"

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

const char *STAT_ERROR_STRING[] = {
	"STA_OK", //		0x00	/* No error */
	"STA_NOINIT", //		0x01	/* Drive not initialized */
	"STA_NODISK", //		0x02	/* No medium in the drive */
	"STA_UNKNOWN", //		0x03	/* unknown error*/
	"STA_PROTECT" //		0x04	/* Write protected */
};

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

const char *fileSystem[] = {"No FS", "FS_FAT12","FS_FAT16","FS_FAT32","FS_EXFAT"};


#endif
