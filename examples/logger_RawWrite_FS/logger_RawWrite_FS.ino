//Copyright 2019 by Walter Zimmer
// Version 08-jun-19
//
/*
 * Logger test program
 * to test Stock SD library, Bill Greimans SdFS, CHaN SDFAT (WMXZ: uSDFS) 
 * to test SPI, SDIO, USBHost
 * to test FAT32, and exFAT
 * AS of 15-may-2019 following implementations are known
 * Stock SD library:     Fat32;         SPI(T3,T4), SDIO(T3,T4)
 * Bill Greimans SdFS:   Fat32, exFAT;  SPI(T3,T4), SDIO(T3)
 * WMXZ uSDFS:           FAT32, exFAT;  SPI(T3,T4), SDIO(I3,T4), USBHost(T3,T4)
 * Caveat: functionality only partial verified
 */
#define TEST_DRV 2 // 0: SPI; 1: SDIO; 2: USBHost

//
#define MXFN 10 // maximal number of files //was 100
#define MXRC 1000 // number of records in file // was 1000
const char *fnamePrefix = "A";

#define SDo 1   // Stock SD library
#define SdFS 2  // Greimans SD library
#define uSDFS 3 // CHaN's SD library (WMXZ's uSDFS)

#define USE_FS uSDFS

#if TEST_DRV == 0
  #define SD_CS 10
  #define SD_SCK 14
  #define SD_MOSI 7
  #define SD_MISO 12
#elif TEST_DRV == 1
  #define SD_CS BUILTIN_SDCARD
#endif

/* MSF API 
 *  init(void);
 *  void exit(void);
 *  void open(char * filename);
 *  void close(void);
 *  uint32_t write(uint8_t *buffer, uint32_t nbuf);
 *  uint32_t read(uint8_t *buffer, uint32_t nbuf);
 */

//--------------------- For File Time settings ------------------
#if USE_FS != uSDFS
#ifndef HAVETM
  #include <time.h>
  #define HAVETM
#endif
  #define EPOCH_YEAR 1970 //T3 RTC
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
  { struct tm tx;
    tx.tm_sec   = tt % 60;    tt /= 60; // now it is minutes
    tx.tm_min   = tt % 60;    tt /= 60; // now it is hours
    tx.tm_hour  = tt % 24;    tt /= 24; // now it is days
    tx.tm_wday  = ((tt + 4) % 7) + 1;   // Sunday is day 1 (tbv)
  
    // tt is now days since EPOCH_Year (1970)
    uint32_t year = 0;  
    uint32_t days = 0;
    while((unsigned)(days += (LEAP_YEAR(year) ? 366 : 365)) <= tt) year++;
  
    tx.tm_year = 1970+year; // year is offset from 1970 
  
    // correct for last (actual) year
    days -= (LEAP_YEAR(year) ? 366 : 365);
    tt  -= days; // now tt is days in this year, starting at 0
    
    uint32_t month=0;
    uint32_t monthLength=0;
    for (month=0; month<12; month++) 
    { monthLength = monthDays[month];
      if ((month==1) & LEAP_YEAR(year)) monthLength++; 
      if (tt<monthLength) break;
      tt -= monthLength;
    }
    tx.tm_mon = month + 1;   // jan is month 1  
    tx.tm_mday = tt + 1;     // day of month
    return tx;
  }
  
  uint32_t tm2seconds (struct tm *tx) 
  {
    uint32_t tt;
    tt=tx->tm_sec+tx->tm_min*60+tx->tm_hour*3600;  
  
    // count days size epoch until previous midnight
    uint32_t days=tx->tm_mday;
  
    uint32_t mm=0;
    uint32_t monthLength=0;
    for (mm=0; mm<(tx->tm_mon-1); mm++) days+=monthDays[mm]; 
    if(tx->tm_mon>2 && LEAP_YEAR(tx->tm_year-1970)) days++;
  
    uint32_t years=0;
    while(years++ < (tx->tm_year-1970)) days += (LEAP_YEAR(years) ? 366 : 365);
    //  
    tt+=(days*24*3600);
    return tt;
  }
#else
#ifndef HAVETM
  #include <time.h>
  #define HAVETM
#endif
  extern "C" struct tm seconds2tm(uint32_t tt);
#endif

#if  USE_FS == SDo
  #include "SPI.h"
  #include "SD.h"
  
  class mFS_class
  {
    private:
    File file;
  
    void die(char *txt) {Serial.println(txt); while(1) asm("wfi");}
    
    public:
      void init(void)
      { 
        Serial.println("Using SDo");
        #if TEST_DRV==0
          // Initialize the SD card
          SPI.setMISO(SD_MISO);
          SPI.setMOSI(SD_MOSI);
          SPI.setSCK(SD_SCK);
        #endif
        int rc;
        if (!(rc=SD.begin(SD_CS))) {Serial.println(rc); die("error SD.begin");}
      }
  
      void mkDir(char * dirname)  {  SD.mkdir(dirname);  }
        
      void exit(void) { }
      
      void open(char * filename) {  file = SD.open(filename, FILE_WRITE);  }
  
      void close(void)  {  file.close(); }
  
      uint32_t write(uint8_t *buffer, uint32_t nbuf)
      {
        file.write(buffer, nbuf);
        return nbuf;
      }
  
      uint32_t read(uint8_t *buffer, uint32_t nbuf)
      {      
        if ((int)nbuf != file.read(buffer, nbuf)) die("error file read");
        return nbuf;
      }
  };

#elif USE_FS == SdFS

  #include "SdFs.h"

  #if defined(__IMXRT1062__)
    #error "SDFS not ported yet to IMXRT1062"
  #endif
  //
  #if TEST_DRV == 0
    #define SD_CS 10
    #define SD_CONFIG SdSpiConfig(SD_CS, DEDICATED_SPI, SPI_FULL_SPEED)
  #elif TEST_DRV ==1
    #define SD_CONFIG SdioConfig(FIFO_SDIO)
    //#define SD_CONFIG SdioConfig(DMA_SDIO)
  #elif TEST_DRV == 2
    #error "USB_HOST not implemented"
  #endif 

  // Call back for file timestamps.  Only called for file create and sync().
  void dateTime(uint16_t* date, uint16_t* time) 
  {
    struct tm tx=seconds2tm(RTC_TSR);
      
    // Return date using FS_DATE macro to format fields.
    *date = FS_DATE(tx.tm_year, tx.tm_mon, tx.tm_mday);
  
    // Return time using FS_TIME macro to format fields.
    *time = FS_TIME(tx.tm_hour, tx.tm_min, tx.tm_sec);
  }

  class mFS_class
  {
    private:
    SdFs sd;
    FsFile file;
    
    public:
      void init(void)
      { Serial.println("Using SdFS");
        #if TEST_DRV==0
          // Initialize the SD card
          SPI.setMISO(SD_MISO);
          SPI.setMOSI(SD_MOSI);
          SPI.setSCK(SD_SCK);
        #endif
        if (!sd.begin(SD_CONFIG)) sd.errorHalt("sd.begin failed");
      
        // Set Time callback
        FsDateTime::callback = dateTime;
      }
  
      void mkDir(char * dirname)  { if(!sd.exists(dirname)) sd.mkdir(dirname);  }
      
      void chDir(char * dirname)  { sd.chdir(dirname);  }
      
      void exit(void) { sd.end(); }
      
      void open(char * filename)
      {
        if (!file.open(filename, O_CREAT | O_TRUNC |O_RDWR)) {
          sd.errorHalt("file.open failed");
        }
        if (!file.preAllocate(PRE_ALLOCATE_SIZE)) {
          sd.errorHalt("file.preAllocate failed");    
        }
      }
  
      void close(void)
      {
        file.truncate();
        file.close();
      }
  
      uint32_t write(uint8_t *buffer, uint32_t nbuf)
      {
        if (nbuf != file.write(buffer, nbuf)) sd.errorHalt("write failed");
        return nbuf;
      }
  
      uint32_t read(uint8_t *buffer, uint32_t nbuf)
      {      
        if ((int)nbuf != file.read(buffer, nbuf)) sd.errorHalt("read failed");
        return nbuf;
      }
  };

#elif  USE_FS == uSDFS
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


  #if TEST_DRV == 0
    const char *Dev = "0:/";  // SPI
  #elif TEST_DRV == 1
    const char *Dev = "1:/";  // SDHC
  #elif TEST_DRV == 2
    const char *Dev = "2:/";  // USB
  #elif TEST_DRV == 3
    const char *Dev = "3:/";  // USB 2nd partition
  #endif

  
  class mFS_class
  {
    private:
      FRESULT rc;     /* Result code */
      FATFS fatfs;    /* File system object */
      FIL fil;        /* File object */
  
      UINT wr;
      
      /* Stop with error message */
      void die(const char *text, FRESULT rc)
      { Serial.printf("%s: Failed with rc=%s.\r\n", text,FR_ERROR_STRING[rc]);  while(1) asm("wfi"); }
      
    public:
      void init(void)
      {
        Serial.println("Using uSDFS");
        Serial.print("uSDFS_VER:"); Serial.println(uSDFS_VER);
        if((rc = f_mount (&fatfs, Dev, 1))) die("Mount",rc);      /* Mount/Unmount a logical drive */
      
        Serial.printf("File System %s\n", fileSystem[fatfs.fs_type]);
        Serial.printf("Free Disk Size %d clusters\n",fatfs.free_clst);
        Serial.printf("Cluster Size %d sectors\n",fatfs.csize);
        #if FF_MAX_SS != FF_MIN_SS
          Serial.printf("Sector Size %d bytes\n",fatfs.ssize);
        #else
          Serial.printf("Sector Size %d bytes\n",FF_MIN_SS);
        #endif
        
        Serial.printf("\nChange drive\n");
        if((rc = f_chdrive(Dev))) die("chdrive",rc);
      }
  
      void mkDir(char * dirname)
      {
        
      }
  
      void exit(void)
      {
        rc = f_unmount(Dev);
        Serial.print("unmount "); Serial.println(FR_ERROR_STRING[rc]);        
      }
      
      void open(char * filename)
      {
        // check status of file
        rc = f_stat(filename,0);
        Serial.printf("stat %s %x\n",FR_ERROR_STRING[rc],fil.obj.sclust);
    
        rc = f_open(&fil, filename, FA_WRITE | FA_CREATE_ALWAYS);
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
          if(rc = f_open(&fil, filename, FA_WRITE | FA_CREATE_ALWAYS)) die("open", rc);
        }
      }
      
      void close(void)
      {
        //close file
        if (rc = f_close(&fil)) die("close", rc);
      }
      
      uint32_t write( uint8_t *buffer, uint32_t nbuf)
      {
         rc = f_write(&fil, buffer, nbuf, &wr);
         uint32_t tb=micros();
         if (rc == FR_DISK_ERR) // IO error
         {  // only option is to close file
            // force closing file
            return 0;
         }
         return nbuf;
      }
      
      uint32_t read(uint8_t *buffer, uint32_t nbuf)
      { return 0;
      }
  };
#endif


#if defined(__MK20DX256__)
  #define BUFFSIZE (2*1024) // size of buffer to be written
#elif defined(__MK66FX1M0__)
  #define BUFFSIZE (8*1024) // size of buffer to be written
#elif defined(__IMXRT1062__)
  #define BUFFSIZE (8*1024) // size of buffer to be written
#endif

// Temp Hack to print out stats from MSC library...
extern void MSC_PrintCallStats();

uint32_t buffer[BUFFSIZE];

class Logger_class
{
  private:
  class mFS_class mfs;

  uint32_t count=0;
  uint32_t ifn=0;
  uint32_t isFileOpen=0;
  char filename[80];
  uint32_t t0=0;
  uint32_t t1=0;
  uint32_t dtwmin=1<<31, dtwmax=0;
  uint32_t dto=1<<31, dtc=0;

  public:
    void init(void)
    {
      mfs.init();
    }

    void stop()
    {
      ifn = MXFN+1;
    }
    int16_t write(void *data, uint32_t ndat)
    {
      if(!count) // at the beginning of a file we have count==0;
      {
        // close file
        if(isFileOpen)
        { dtc = micros();
          //close file
          mfs.close();
          //
          isFileOpen=0;
          t1=micros();
          dtc = t1-dtc;
          float MBs = (MXRC*ndat)/(1.0f*(t1-t0));
          Serial.printf(" (%d - %f MB/s)\n (open: %d us; close: %d us; write: min,max: %d %d us)\n\r",
                            t1-t0,MBs, dto, dtc, dtwmin,dtwmax);
//          MSC_PrintCallStats();
          dtwmin=1<<31; dtwmax=0;
        }
      }
        
      //
      if(!isFileOpen)
      {
        // open new file
        ifn++;
        if(ifn>MXFN) // at end of test: exit and prepare for blinking
        { mfs.exit();
          return 0; 
        } 
    
        dto=micros();
        sprintf(filename,"%s_%05d.dat",fnamePrefix,ifn);
        Serial.println(filename);
        //
        mfs.open(filename);
        //
        isFileOpen=1;
        t0=micros();
        dto=t0-dto;
      }
      
      if(isFileOpen)
      {
         count++;
         //write data to file 
         if(!(count%10))Serial.printf(".");
         if(!(count%640)) Serial.println(); Serial.flush();
         //
         uint32_t ta=micros();
         if(!mfs.write((uint8_t *)buffer,BUFFSIZE*4))
         {  Serial.printf(" write error at count # %d\n",count);
            count=MXRC;
         }
         uint32_t tb=micros();
         //    
         uint32_t dt=tb-ta;
         if(dt<dtwmin) dtwmin=dt;
         if(dt>dtwmax) dtwmax=dt;
         //
         count %= MXRC;
      }    
      return 1;
    }
};

class Logger_class mLogger;

//=========================================================================
void blink(uint16_t msec) { digitalWriteFast(13,!digitalReadFast(13)); delay(msec); }

void setup()
{
  // wait for serial line to come up
  pinMode(13,OUTPUT);
  pinMode(13,HIGH);

  struct tm tx;
  tx=seconds2tm(rtc_get());
  
  while(!Serial);
  Serial.println("Test logger_RawWrite");
  Serial.printf("%4d-%02d-%02d %02d:%02d:%02d\n",
                tx.tm_year, tx.tm_mon, tx.tm_mday,tx.tm_hour, tx.tm_min, tx.tm_sec);  
  Serial.print("BUFFSIZE :");  Serial.println(BUFFSIZE);
  Serial.print("Dev Type :");  Serial.println(TEST_DRV);
  mLogger.init();
}

void loop()
{
  static int doLogging=1;
  if(!doLogging) { blink(500); return; }

  while (Serial.available() > 0) 
 { if ('q' == Serial.read() )  mLogger.stop();}


   // fill buffer
   static uint32_t lc=0;
   for(int ii=0;ii<BUFFSIZE;ii++) buffer[ii]='0'+(lc%10); lc++;
   // call logger
   doLogging=mLogger.write((void *) buffer, 4*BUFFSIZE);
   if(!doLogging) pinMode(13,OUTPUT); 
}
