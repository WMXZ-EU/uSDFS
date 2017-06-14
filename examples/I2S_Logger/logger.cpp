//Copyright 2017 by Walter Zimmer
// Version 18-05-17
//
// general teensy includes
#include "kinetis.h"
#include "core_pins.h"
#include "usb_serial.h"

/***************************** LOGGING ***********************/
#define MXFN 5 // maximal number of files 
#define MAX_BLOCK_COUNT 1000  // number of BUFFSIZE writes to file

#define FMT "X_%05u.dat"

uint32_t ifn=0;

/**
 * define Disk_buffer to be multiple to write buffer and multiple of data buffer  
 */
#if defined(__MK20DX256__)
  #define BUFFSIZE (8*1024) // size in bytes of buffer to be written
  #define DISK_BUFFSIZE (2*BUFFSIZE) // size in bytes of memory buffer 
#elif defined(__MK66FX1M0__)
  #define BUFFSIZE (32*1024) // size in bytes of buffer to be written
  #define DISK_BUFFSIZE (4*BUFFSIZE) // size in bytes of memory buffer
#endif

#define INF ((uint32_t) (-1))
uint8_t Buffer[BUFFSIZE] __attribute__( ( aligned ( 16 ) ) ); // to disk
uint8_t diskBuffer[DISK_BUFFSIZE] __attribute__( ( aligned ( 16 ) ) ); // memory storage

void blink(uint32_t msec);

// include FileSystem Interface
#include "mfs.h"

uint32_t isFileOpen=0;
uint32_t t0=0;
uint32_t t1=0;

uint32_t loggerCount=0;

uint32_t n1_dat = INF;
uint32_t n_guard = DISK_BUFFSIZE;

/*-------------------------------------------------------------------------*/
void logger_init()
{
  mFS.init();
}
/*-------------------------------------------------------------------------*/
void logger_write(uint8_t *data, uint16_t nbuf)
{ // data point to input having a size of nbuf bytes
  // is called whenever new data are available
  //
    // fill disk buffer
    // works only if disk buffer is multiple of nbuf
    for(int ii=0; ii<nbuf; ii++) diskBuffer[(n1_dat+ii) % DISK_BUFFSIZE]= data[ii]; 
        
    n1_dat += nbuf;
    n1_dat %= DISK_BUFFSIZE;
    //
    n_guard = DISK_BUFFSIZE-nbuf;
}

/*-------------------------------------------------------------------------*/
uint32_t logger_save(void)
{ // keeps state of filing 
  // does open/close of file when required
  //
  static uint32_t n0_dat=0;     // write-out pointer
  static uint16_t overrunRisk=0;
  static uint16_t gotData=0; // indicates that data have been written to disk

  static uint16_t isLogging = 0; // flag to ensure single access to function 

  char filename[80];

  if(isLogging) return 0;
  isLogging=1;
  //
  if(ifn>MXFN) 
  { isLogging=0; return 1; }

  if(!loggerCount && gotData)
  { // close file
    if(isFileOpen)
    { //close file
      t1=micros();
      mFS.close();
      //
      isFileOpen=0;
      float MBs = (1000.0f*BUFFSIZE)/(1.0f*(t1-t0));
      Serial.printf(" (%d - %f MB/s)\n\r",t1-t0,MBs); Serial.flush();
      gotData = 0;
    }
  }
    
  //
  if(!isFileOpen)
  {
    // open new file
    ifn++;
    if(ifn>MXFN) 
    { isLogging=0; Serial.println("end of Acq"); Serial.flush(); return INF; } // end of all operations

    sprintf(filename,FMT,(unsigned int)ifn);
    Serial.println(filename); Serial.flush();
    mFS.open(filename);
    isFileOpen=1;
    t0=micros();
  }
  
  if(isFileOpen)
  {
    // copy data to disk buffer
    if(n1_dat==INF) // reset pointers
    { n1_dat=0;
      n0_dat=0;
    }
    else if(((n1_dat - n0_dat + DISK_BUFFSIZE) % DISK_BUFFSIZE) >= n_guard) 
    { // too close to buffer overrun
      Serial.printf("x %d %d %d\n\r",n0_dat,n1_dat,overrunRisk);
      n1_dat = INF;
      isLogging=0;
      return 0;
    }

    // send to disk
    while(((n1_dat - n0_dat + DISK_BUFFSIZE) % DISK_BUFFSIZE) >= BUFFSIZE)
    {  // copy now data
       for(int ii=0;ii<BUFFSIZE;ii++) Buffer[ii]= diskBuffer[n0_dat+ii];
       n0_dat += BUFFSIZE; 
       n0_dat %= DISK_BUFFSIZE;
       overrunRisk = (n0_dat==n1_dat);
       loggerCount++;
       //
       //write data to file 
       if(!(loggerCount%10)) Serial.printf("."); Serial.flush();
       if(!(loggerCount%640)) Serial.println(); Serial.flush();
       //
       if(!mFS.write(Buffer, BUFFSIZE)) 
                 loggerCount=MAX_BLOCK_COUNT; // set to max block count to force closing file
       loggerCount %= MAX_BLOCK_COUNT;
       gotData = 1;
    }
  }   
  isLogging=0; 
  return 1;
}


