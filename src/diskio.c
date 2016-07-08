/*-----------------------------------------------------------------------*/
/* Low level disk I/O module glue functions         (C)ChaN, 2016        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/
//Copyright 2016 by Walter Zimmer
// Version 29-jun-16

#include "diskio.h"
#include "sdhc.h"

#define HAVE_HW_SERIAL
#ifdef HAVE_HW_SERIAL

#include "kinetis.h"
#include "localKinetis.h"
#include "core_pins.h" // testing only

/* some aux functions for pure c code */
#include "usb_serial.h"

#define _PUTCHAR(x) serial_putchar(x)
#define _FLUSH() serial_flush()
//
void logg(char c);
void printb(uint32_t x,uint32_t im);
//
void logg(char c) {_PUTCHAR(c); _FLUSH();}
void printb(uint32_t x,uint32_t im)
{ char c;
  int ii;
  for(ii=im;ii>=0;ii--)
  { if(!((ii+1)%4)) _PUTCHAR(' ');
    c=(x&(1<<ii))?'1':'0'; _PUTCHAR(c);
  }
  _PUTCHAR('\r');
  _PUTCHAR('\n');
  _FLUSH();
}
/* end aux functions */
#endif


/******************************************************************************
*
*   Public functions
*
******************************************************************************/
DSTATUS disk_isReady(BYTE drv) 
{   if(drv)
		return RES_PARERR;
    return SDHC_isReady();
}

//-----------------------------------------------------------------------------
// FUNCTION:    disk_initialize
// SCOPE:       SD Card public related function
// DESCRIPTION: Function initialize the disk
//              
// PARAMETERS:  drv - Physical drive nmuber (0)
//              
// RETURNS:     status of initialization(OK, nonInit, noCard, CardProtected)
//-----------------------------------------------------------------------------  
DSTATUS disk_initialize (BYTE drv)
{
  if(drv)
    return RES_PARERR;

  return SDHC_InitCard(25000);
}

//-----------------------------------------------------------------------------
// FUNCTION:    disk_status
// SCOPE:       SD Card public related function
// DESCRIPTION: Function return status of disk
//              
// PARAMETERS:  drv - Physical drive nmuber (0)
//              
// RETURNS:     status of disk(OK, nonInit, noCard, CardProtected)
//-----------------------------------------------------------------------------
DSTATUS disk_status (BYTE drv)
{
  return SDHC_GetStatus();
}

//-----------------------------------------------------------------------------
// FUNCTION:    disk_read
// SCOPE:       SD Card public related function
// DESCRIPTION: Function read block/blocks from disk
//              
// PARAMETERS:  drv - Physical drive nmuber (0)
//              buff - pointer on buffer where read data should be stored
//              sector - index of start sector
//              count - count of sector to read
//              
// RETURNS:     result of operation
//-----------------------------------------------------------------------------
DRESULT disk_read (BYTE drv, BYTE* buff, DWORD sector, UINT count)
{
  DRESULT rc;		
  if(drv || (count == 0))
    return RES_PARERR;

	while(!SDHC_isReady());
 #if MULTI_SECTOR == 1
  SDHC_ClearDMAStatus();
  rc= SDHC_ReadBlocks(buff, sector, count);
  while(!SDHC_GetDMAStatus());
#else
	BYTE*ptr=(BYTE *)buff;
	for(;count;count--)
	{
		  SDHC_ClearDMAStatus();
		  rc= SDHC_ReadBlocks(ptr, sector, 1);
		  if(rc != RES_OK) break;
		  ptr+=512; sector++;
		  while(!SDHC_GetDMAStatus());
	}
#endif
  return rc;
}

#if	_READONLY == 0
//-----------------------------------------------------------------------------
// FUNCTION:    disk_write
// SCOPE:       SD Card public related function
// DESCRIPTION: Function write block/blocks to disk
//              
// PARAMETERS:  drv - Physical drive nmuber (0)
//              buff - pointer on buffer where write data are prepared
//              sector - index of start sector
//              count - count of sector to read
//              
// RETURNS:     result of operation
//-----------------------------------------------------------------------------
DRESULT disk_write (BYTE drv, const BYTE* buff, DWORD sector, UINT count)
{
	DRESULT rc;	
  if(drv || (count == 0))
    return RES_PARERR;
  
	while(!SDHC_isReady()); // make sure uSD card is not busy
	
 #if MULTI_SECTOR == 1
  SDHC_ClearDMAStatus();
  rc= SDHC_WriteBlocks((BYTE*)buff, sector, count);
  while(!SDHC_GetDMAStatus());

#else
	BYTE *ptr=(BYTE *)buff;
	for(;count;count--)
	{
		  SDHC_ClearDMAStatus();
		  rc= SDHC_WriteBlocks(ptr, sector, 1);
		  if(rc != RES_OK) break;
		  ptr+=512; sector++;
		  while(!SDHC_GetDMAStatus());
	}
#endif
  return rc;
}
#endif

//-----------------------------------------------------------------------------
// FUNCTION:    disk_ioctl
// SCOPE:       SD Card public related function
// DESCRIPTION: Function io control for SDcard
//              
// PARAMETERS:  drv - Physical drive nmuber (0)
//              ctrl - control command
//              buff - pointer on buffer for io operations
//              
// RETURNS:     result of operation
//-----------------------------------------------------------------------------
DRESULT disk_ioctl (BYTE drv, BYTE ctrl, void* buff)
{
  DRESULT result = RES_OK;
  
  if(drv)
    return RES_PARERR;
  
  switch(ctrl)
  {
    case CTRL_SYNC:
      /*
      Make sure that the disk drive has finished pending write process. 
      When the disk I/O module has a write back cache, flush the dirty sector 
      immediately. This command is not used in read-only configuration.
      */
      // in polling mode I know that all writes operations are finished!
	  while(!SDHC_isReady());
      break;
    case GET_SECTOR_SIZE:
      /*
      Returns sector size of the drive into the WORD variable pointed by Buffer.
      This command is not used in fixed sector size configuration, 
      _MAX_SS is 512.
      */
      if(buff == NULL)
        result = RES_PARERR;
      else
        *(LWord*)buff = SDHC_BLOCK_SIZE;
      
      break;
    case GET_SECTOR_COUNT:
      /*
      Returns number of available sectors on the drive into the DWORD variable
      pointed by Buffer. This command is used by only f_mkfs function to 
      determine the volume size to be created.
      */
      if(buff == NULL)
        result = RES_PARERR;
      else
        *(LWord*)buff = SDHC_GetBlockCnt();
      break;
    case GET_BLOCK_SIZE:
      /*
      Returns erase block size of the flash memory in unit of sector into the 
      DWORD variable pointed by Buffer. The allowable value is 1 to 32768 in 
      power of 2. Return 1 if the erase block size is unknown or disk devices. 
      This command is used by only f_mkfs function and it attempts to align data
      area to the erase block boundary.
      */
      result = RES_PARERR;
      break;
#ifdef CTRL_ERASE_SECTOR
    case CTRL_ERASE_SECTOR:
      /*
      Erases a part of the flash memory specified by a DWORD array 
      {<start sector>, <end sector>} pointed by Buffer. When this feature is not
      supported or not a flash memory media, this command has no effect. The 
      FatFs does not check the result code and the file function is not affected
      even if the sectors are not erased well. This command is called on 
      removing a cluster chain when _USE_ERASE is 1.
      */
      result = RES_PARERR;
      break;
#endif
    default:
      return RES_PARERR;
    
  }
  return result;
}

//Copyright 2016 by Walter Zimmer
//
#include <time.h>
#include "kinetis.h"
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
