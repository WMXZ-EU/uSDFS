

#include "diskio.h"
#include "sdhc.h"

#define USB_DEBUG
//#undef USB_DEBUG
#ifdef USB_DEBUG
void logg(char c);
#endif

/******************************************************************************
*
*   Public functions
*
******************************************************************************/

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
 
  return SDHC_InitCard();
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

#if MULTI_SECTOR == 1
	SDHC_DMAWait();	// make sure uSD card is not busy
	rc= SDHC_ReadBlocks(buff, sector, count);
	SDHC_DMAWait();
#else
	BYTE*ptr=(BYTE *)buff;
	for(;count;count--)
	{
		rc= SDHC_ReadBlocks(ptr, sector, 1);
		if(rc != RES_OK) break;
		ptr+=512; sector++;
		SDHC_DMAWait();
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

#if MULTI_SECTOR == 1
	SDHC_DMAWait();	// make sure uSD card is not busy
	rc= SDHC_WriteBlocks((BYTE*)buff, sector, count);
	SDHC_DMAWait();
#else
	BYTE *ptr=(BYTE *)buff;
	for(;count;count--)
	{ 	rc= SDHC_WriteBlocks(ptr, sector, 1);
		if(rc != RES_OK) break;
		ptr+=512; sector++;
		SDHC_DMAWait();
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
#ifdef OLD_IOCTL
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
    case CTRL_DMA_STATUS:
		*(LWord*)buff = SDHC_DMADone();
      break;
    default:
      return RES_PARERR;
    
  }
  return result;
}
