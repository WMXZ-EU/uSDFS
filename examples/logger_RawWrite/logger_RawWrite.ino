//Copyright 2019 by Walter Zimmer
// Version 08-jun-19
//
// use following lines for early definitions of multiple partition configuration in uSDFS.h
#define MY_VOL_TO_PART
#include "uSDFS.h"
//
// for use of USB-HUBs
#include <USBHost_t36.h>
extern USBHost myusb;
USBHub hub1(myusb);
USBHub hub2(myusb);

#if FF_MULTI_PARTITION		/* Multiple partition configuration */ 
	PARTITION VolToPart[] = {{DEV_SPI, 0}, //{ physical drive number, Partition: 0:Auto detect, 1-4:Forced partition)} 
							 {DEV_SDHC,0}, 
							 {DEV_USB, 0}, 
							 {DEV_USB, 1}, 
							 {DEV_USB, 2}
							 }; /* Volume - Partition resolution table */
#endif
// end of early definition


#define TEST_DRV 1
//
#define MXFN 10 // maximal number of files //was 100
#define MXRC 1000 // number of records in file // was 1000
const char *fnamePrefix = "A";

//
#if TEST_DRV == 0
  const char *Dev = "0:/";  // SPI
#elif TEST_DRV == 1
  const char *Dev = "1:/";  // SDHC
#elif TEST_DRV == 2
  const char *Dev = "2:/";  // USB
#endif

FRESULT rc;        /* Result code */
FATFS fatfs;      /* File system object */
FIL fil;        /* File object */

#if defined(__MK20DX256__)
  #define BUFFSIZE (2*1024) // size of buffer to be written
#elif defined(__MK66FX1M0__)
  #define BUFFSIZE (8*1024) // size of buffer to be written
#elif defined(__IMXRT1062__)
  #define BUFFSIZE (8*1024) // size of buffer to be written
#endif

uint32_t buffer[BUFFSIZE];
UINT wr;

/* Stop with error message */
void die(const char *text, FRESULT rc)
{ Serial.printf("%s: Failed with rc=%s.\r\n", text,FR_ERROR_STRING[rc]);  while(1) asm("wfi"); }

//=========================================================================
void blink(uint16_t msec)
{
  digitalWriteFast(13,!digitalReadFast(13)); delay(msec);
}

#ifndef HAVETM
  #include <time.h>
  #define HAVETM
#endif
extern "C" struct tm seconds2tm(uint32_t tt);
extern "C" struct tm decode_fattime (uint16_t td, uint16_t tt);


void listDir(const char *dirName)
{
	FRESULT res;
	DIR dir;
	FILINFO fno;

	res = f_opendir(&dir, dirName);                       /* Open the directory */
	Serial.printf("openDir %s\n",FR_ERROR_STRING[res]);
	if (res == FR_OK) 
	{
		for (;;) 
		{
			res = f_readdir(&dir, &fno);                   /* Read a directory item */
			if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
			if(!strncmp(fno.fname,fnamePrefix,strlen(fnamePrefix)))
			{
			  Serial.printf("%s ", fno.fname);                /* Display the object name */
			  Serial.printf("%d ", fno.fsize);                /* Display the object size */
			  struct tm tx = decode_fattime (fno.fdate, fno.ftime);
			  Serial.printf("%4d-%02d-%02d %02d:%02d:%02d\n",
							tx.tm_year,tx.tm_mon,tx.tm_mday, 
							tx.tm_hour,tx.tm_min,tx.tm_sec);
			}
		}
    }
    f_closedir(&dir);
}

void setup()
{
  // wait for serial line to come up
  pinMode(13,OUTPUT);
  pinMode(13,HIGH);

  while(!Serial);
  Serial.println("Test logger_RawWrite");
  Serial.print("uSDFS_VER:"); Serial.println(uSDFS_VER);
  Serial.print("BUFFSIZE :");  Serial.println(BUFFSIZE);
  Serial.print("Dev Type :");  Serial.println(Dev);
  if((rc = f_mount (&fatfs, Dev, 1))) die("Mount",rc);      /* Mount/Unmount a logical drive */

  Serial.printf("File System %s\n", fileSystem[fatfs.fs_type]);
  Serial.printf("Free Disk Size %d clusters\n",fatfs.free_clst);
  Serial.printf("Cluster Size %d sectors\n",fatfs.csize);
#if FF_MAX_SS != FF_MIN_SS
  Serial.printf("Sector Size %d bytes\n",fatfs.ssize);
#else
  Serial.printf("Sector Size %d bytes\n",FF_MIN_SS);
#endif
  //-----------------------------------------------------------
  Serial.printf("\nChange drive\n");
  if((rc = f_chdrive(Dev))) die("chdrive",rc);
}

void loop()
{
	static uint32_t count=0;
	static int32_t ifn=0;
	static uint32_t isFileOpen=0;
	static char filename[80];
	static uint32_t t0=0;
	static uint32_t t1=0;
  static uint32_t dtwmin=1<<31, dtwmax=0;
  static uint32_t dto=1<<31, dtc=0;

  if(ifn>MXFN) { blink(500); return; }
  
  // stop testing (finish actual file)
  while (Serial.available() > 0) 
  { if ('q' == Serial.read() ) ifn = MXFN+1; }
  
  if(!count)
  {
    // close file
    if(isFileOpen)
    { dtc = micros();
      //close file
      if ((rc = f_close(&fil))) die("close", rc);
      //
      isFileOpen=0;
      t1=micros();
      dtc = t1-dtc;
      float MBs = (MXRC*BUFFSIZE*4.0f)/(1.0f*(t1-t0));
      Serial.printf(" (%d - %f MB/s)\n (open: %d us; close: %d us; write: min,max: %d %d us)\n\r",
                        t1-t0,MBs, dto, dtc, dtwmin,dtwmax);
      dtwmin=1<<31; dtwmax=0;
    }
  }
    
  //
  if(!isFileOpen)
  {
    // open new file
    ifn++;
    if(ifn>MXFN) 
    { listDir(Dev);
	  rc = f_unmount(Dev);
      Serial.print("unmount "); Serial.println(FR_ERROR_STRING[rc]);
      pinMode(13,OUTPUT); return; 
    } // at end of test: prepare for blinking

    dto=micros();
    sprintf(filename,"%s_%05d.dat",fnamePrefix,(int)ifn);
    Serial.println(filename);
    //
    // check status of file
    rc = f_stat(filename,0);
    Serial.printf("stat %s %x\n",FR_ERROR_STRING[rc],fil.obj.sclust);
    
    rc = f_open(&fil, filename, (FA_WRITE | FA_CREATE_ALWAYS));
    Serial.printf(" opened %s %x\n\r",FR_ERROR_STRING[rc],fil.obj.sclust);
    // check if file is Good
    if(rc == FR_INT_ERR)
    { // only option is to close file
        rc = f_close(&fil);
        if(rc == FR_INVALID_OBJECT)
        { Serial.println("unlinking file");
          rc = f_unlink(filename);
          if (rc) die("unlink", rc);
        }
        else
          die("close", rc);
      // retry open file
      if(rc = f_open(&fil, filename, (FA_WRITE | FA_CREATE_ALWAYS))) die("open", rc);
    }
    //
    isFileOpen=1;
    t0=micros();
    dto=t0-dto;
  }
  
  if(isFileOpen)
  {
     // fill buffer
     for(int ii=0;ii<BUFFSIZE;ii++) buffer[ii]='0'+(count%10);
     count++;
     //write data to file 
     if(!(count%10))Serial.printf(".");
     if(!(count%640)) Serial.println(); Serial.flush();
     //
     uint32_t ta=micros();
     rc = f_write(&fil, buffer, BUFFSIZE*4, &wr);
     uint32_t tb=micros();
     if (rc == FR_DISK_ERR) // IO error
     {  Serial.printf(" write FR_DISK_ERR at count # %d\n",count);
        // only option is to close file
        // force closing file
        count=MXRC;
     }
     else if(rc) die("write",rc);
    //    
     uint32_t dt=tb-ta;
     if(dt<dtwmin) dtwmin=dt;
     if(dt>dtwmax) dtwmax=dt;
     //
     count %= MXRC;
  }    
}
