/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2016        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"			/* Obtains integer types */
#include "diskio.h"		/* Declarations of disk functions */

#include "utility/sd_spi.h"
#include "utility/sd_sdhc.h"
#include "utility/sd_msc.h"
void logVar(char *s,unsigned int v);
/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{

	switch (pdrv) {
	case DEV_SPI :
		return SPI_disk_status();

	case DEV_SDHC :
		return SDHC_disk_status();

	case DEV_MSC :
		return MSC_disk_status();
	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat=STA_NOINIT;
	int result;

	switch (pdrv) {
	case DEV_SPI :

		result = SPI_disk_initialize();
		// translate the reslut code here
		if(result==RES_OK) stat=0; else stat=STA_NODISK;

		return stat;

	case DEV_SDHC :
		result = SDHC_disk_initialize();

		// translate the reslut code here
		if(result==RES_OK) stat=0; else stat=STA_NODISK;

		return stat;

	case DEV_MSC :
		result = MSC_disk_initialize();

		// translate the reslut code here
		if(result==RES_OK) stat=0; else stat=STA_NODISK;

		return stat;
	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	DRESULT res=RES_OK;
	int result=0;
	switch (pdrv) {
	case DEV_SPI :
  
		result = SPI_disk_read(buff, sector, count);
		// translate the reslut code here
 	    if(result==0) res=RES_OK; else res=RES_READERROR;

		return res;

	case DEV_SDHC :
		// translate the arguments here

		result = SDHC_disk_read(buff, sector, count);
		// translate the reslut code here
		if(result==0) res=RES_OK; else res=RES_READERROR;

		return res;

	case DEV_MSC :
		// translate the arguments here

		result = MSC_disk_read(buff, sector, count);
		// translate the reslut code here
		if(result==0) res=RES_OK; else res=RES_READERROR;

		return res;

	}

	return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	DRESULT res=RES_OK;
	int result;

	switch (pdrv) {
	case DEV_SPI :
		// translate the arguments here

		result = SPI_disk_write(buff, sector, count);
		// translate the reslut code here
		if(result==0) res=RES_OK; else res=RES_WRITEERROR;

		return res;

	case DEV_SDHC :
		// translate the arguments here

		result = SDHC_disk_write(buff, sector, count);
		// translate the reslut code here
		if(result==0) res=RES_OK; else res=RES_WRITEERROR;

		return res;

	case DEV_MSC :
		// translate the arguments here

		result = MSC_disk_write(buff, sector, count);
		// translate the reslut code here
		if(result==0) res=RES_OK; else res=RES_WRITEERROR;

		return res;
	}

	return RES_PARERR;
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
//	DRESULT res=RES_OK;

	switch (pdrv) {
	case DEV_SPI :
		return SPI_disk_ioctl(cmd,buff);

		// Process of the command for the SPI drive

//		return res;

	case DEV_SDHC :
		return SDHC_disk_ioctl(cmd,buff);

		// Process of the command for the SDHC device

//		return res;

	case DEV_MSC :
		return SDHC_disk_ioctl(cmd,buff);

		// Process of the command for the SDHC device

//		return res;

		}

	return RES_PARERR;
}
