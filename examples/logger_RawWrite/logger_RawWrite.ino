//Copyright 2019 by Walter Zimmer
// Version 07-may-19
//
#include "uSDFS.h"

#define TEST_DRV 1
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

#define MXFN 100 // maximal number of files 
#if defined(__MK20DX256__)
  #define BUFFSIZE (2*1024) // size of buffer to be written
#elif defined(__MK66FX1M0__)
  #define BUFFSIZE (8*1024) // size of buffer to be written
#elif defined(__IMXRT1062__)
  #define BUFFSIZE (32*1024) // size of buffer to be written
#endif

uint32_t buffer[BUFFSIZE];
UINT wr;

/* Stop with dying message */
void die(char *str, FRESULT rc);
void setup();
void loop();

void die(char *str, FRESULT rc) 
{ Serial.printf("%s: Failed with rc=%u.\n", str, rc); for (;;) delay(100); }

//=========================================================================
void blink(uint16_t msec)
{
  digitalWriteFast(13,!digitalReadFast(13)); delay(msec);
}

void setup()
{
  // wait for serial line to come up
  pinMode(13,OUTPUT);
  pinMode(13,HIGH);

  while(!Serial);
  Serial.println("Test logger_RawWrite");
  Serial.println(Dev);
  if((rc = f_mount (&fatfs, Dev, 1))) die("Mount",rc);      /* Mount/Unmount a logical drive */

  //-----------------------------------------------------------
  Serial.printf("\nChange drive\n");
  if((rc = f_chdrive(Dev))) die("chdrive",rc);
}

void loop()
{
	static uint32_t count=0;
	static uint32_t ifn=0;
	static uint32_t isFileOpen=0;
	static char filename[80];
	static uint32_t t0=0;
	static uint32_t t1=0;

  if(ifn>MXFN) { blink(500); return; }
  
  if(!count)
  {
    // close file
    if(isFileOpen)
    {
      //close file
      if (rc = f_close(&fil)) die("close", rc);
      //
      isFileOpen=0;
      t1=micros();
      float MBs = (1000.0f*BUFFSIZE*4.0f)/(1.0f*(t1-t0));
      Serial.printf(" (%d - %f MB/s)\n\r",t1-t0,MBs);
    }
  }
    
  //
  if(!isFileOpen)
  {
    // open new file
    ifn++;
    if(ifn>MXFN) { pinMode(13,OUTPUT); return; } // at end of test: prepare for blinking

    sprintf(filename,"X_%05d.dat",ifn);
    Serial.println(filename);
    //
    // check status of file
    rc = f_stat(filename,0);
    Serial.printf("stat %d %x\n",rc,fil.obj.sclust);
    
    rc = f_open(&fil, filename, FA_WRITE | FA_CREATE_ALWAYS);
    Serial.printf(" opened %d %x\n\r",rc,fil.obj.sclust);
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
    }
    // retry open file
    if(rc = f_open(&fil, filename, FA_WRITE | FA_CREATE_ALWAYS)) die("open", rc);
    
    isFileOpen=1;
    t0=micros();
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
     if ((rc = f_write(&fil, buffer, BUFFSIZE*4, &wr)) == FR_DISK_ERR) // IO error
     {  Serial.println(" write FR_DISK_ERR");
        // only option is to close file
        // force closing file
        count=1000;
     }
     else if(rc) die("write",rc);
    //    
     count %= 1000;
  }    
}
