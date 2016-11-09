//Copyright 2016 by Walter Zimmer
// Version 07-11-16
//
#include "ff.h"
#include "ff_utils.h"

#define USE_USB_SERIAL
#ifdef USE_USB_SERIAL
	#define SERIALX Serial
#else
	#define SERIALX Serial1
#endif

FRESULT rc;        /* Result code */
FATFS fatfs;      /* File system object */
FIL fil;        /* File object */

#define NSAMP 128 // number of words in data buffer
#define NDAT (2*NSAMP) // bytes in data buffer (input)
#define FSAMP 192000
uint8_t Data[NDAT] __attribute__( ( aligned ( 16 ) ) ); // frome source

uint32_t count=0;
UINT wr;

/* Stop with dying message */
void die(char *str, FRESULT rc);
void setup();
void loop();

extern "C" uint32_t usd_getError(void);
struct tm seconds2tm(uint32_t tt);


void die(char *str, FRESULT rc) 
{ SERIALX.printf("%s: Failed with rc=%u.\n", str, rc); for (;;) delay(100); }

//=========================================================================
void blink(uint16_t msec)
{ digitalWriteFast(13,!digitalReadFast(13)); delay(msec);
}
/***************************************************************/
void  doLogging( uint8_t *buffer, uint16_t ndat);

#define FREQ (FSAMP/NSAMP) // firing rate for data block input (simulated acquisition)

void timer_init(void)
{
  SIM_SCGC6 |= SIM_SCGC6_PIT;
  // turn on PIT     
  PIT_MCR = 0x00;
}

void timer_start(void) 
{//    
  PIT_LDVAL0 = F_BUS/FREQ; // setup timer 0 for (F_BUS/frequency) cycles     
  PIT_TCTRL0 = 2; // enable Timer 0 interrupts      
  NVIC_SET_PRIORITY(IRQ_PIT_CH0, 7*16); 
  NVIC_ENABLE_IRQ(IRQ_PIT_CH0);
  PIT_TCTRL0 |= 1; // start Timer 0
}

//
void pit0_isr(void)
{ //
  PIT_TFLG0=1;
  for(int ii=0; ii<NDAT; ii++) Data[ii]=(count + ii) % 256;
  doLogging(Data, NDAT);
}

/*-----------------------------------------------------------*/
void setup()
{
  // wait for serial line to come up
  pinMode(13,OUTPUT);
  pinMode(13,HIGH);
  while(!SERIALX) blink(100);
  
  #ifndef USB_SERIAL
  	SERIALX.begin(115200,SERIAL_8N1_RXINV_TXINV);
  #endif
  SERIALX.println("\nLogger Buffered Write Test");
  SERIALX.printf("F_BUS %d MHz\n\r",F_BUS/1000000);
  SERIALX.printf("Sample Rate %d Hz\n\r",FSAMP);
  SERIALX.printf("Buffer Update Rate %d Hz\n\r",FREQ);

  rc = f_mount (&fatfs, (TCHAR *)_T("0:/"), 0);      /* Mount/Unmount a logical drive */
  if (rc) die("mount", rc);

  timer_init();
  timer_start();
}

void loop()
{ delay(1000);
}

/***************************** LOGGING ***********************/
#define MXFN 100 // maximal number of files 
#define MAX_BLOCK_COUNT 1000  // number of 512-byte blocks in file

#if defined(__MK20DX256__)
  #define BUFFSIZE (8*1024) // size of buffer to be written
  #define DISK_BUFFSIZE (2*BUFFSIZE) // size of buffer to be written
#elif defined(__MK66FX1M0__)
  #define BUFFSIZE (32*1024) // size of buffer to be written
  #define DISK_BUFFSIZE (4*BUFFSIZE) // size of buffer to be written
#endif

#define INF ((uint32_t) (-1))
uint8_t Buffer[BUFFSIZE] __attribute__( ( aligned ( 16 ) ) ); // to disk
uint8_t diskBuffer[DISK_BUFFSIZE] __attribute__( ( aligned ( 16 ) ) ); // storage

uint32_t ifn=0;
uint32_t isFileOpen=0;
char filename[80];
TCHAR wfilename[80];
uint32_t t0=0;
uint32_t t1=0;


/*-------------------------------------------------------------------------*/
void doLogging(uint8_t *data, uint16_t nbuf)
{ // is called whenever new data are available
  // keeps state of filing 
  // does open/close of file when required
  //
  static uint32_t n0_dat=0, n1_dat = INF;
  static uint16_t overrunRisk=0;
  static uint16_t gotData=0; 

  static uint16_t isLogging = 0;

  if(isLogging) return;
  isLogging=1;
  if(ifn>MXFN) 
  { blink(500); isLogging=0; return; }

  if(!count && gotData)
  {
    // close file
    if(isFileOpen)
    {
      //close file
      rc = f_close(&fil);
      if (rc) die("close", rc);
      //
      isFileOpen=0;
      t1=micros();
      float MBs = (1000.0f*BUFFSIZE)/(1.0*(t1-t0));
      SERIALX.printf(" (%d - %f MB/s)\n\r",t1-t0,MBs);
      gotData = 0;
    }
  }
    
  //
  if(!isFileOpen)
  {
    // open new file
    ifn++;
    if(ifn>MXFN) 
    { pinMode(13,OUTPUT); isLogging=0; return; } // at end of test: prepare for blinking

    sprintf(filename,"X_%05d.dat",ifn);
    SERIALX.println(filename);
    char2tchar(filename,80,wfilename);
    //
    // check status of file
    rc =f_stat(wfilename,0);
    SERIALX.printf("stat %d %x\n",rc,fil.obj.sclust);
    
    rc = f_open(&fil, wfilename, FA_WRITE | FA_CREATE_ALWAYS);
    SERIALX.printf(" opened %d %x\n\r",rc,fil.obj.sclust);
    // check if file is Good
    if(rc == FR_INT_ERR)
    { // only option is to close file
        rc = f_close(&fil);
        if(rc == FR_INVALID_OBJECT)
        { SERIALX.println("unlinking file");
          rc = f_unlink(wfilename);
          if (rc) die("unlink", rc);
        }
        else
          die("close", rc);
        
    }
    // retry open file
    rc = f_open(&fil, wfilename, FA_WRITE | FA_CREATE_ALWAYS);
    if(rc) die("open", rc);
    
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
    else if((n1_dat - n0_dat + DISK_BUFFSIZE)%DISK_BUFFSIZE>=(DISK_BUFFSIZE-nbuf)) 
    { // too close to buffer overrun
      SERIALX.printf("x %d %d %d\n\r",n0_dat,n1_dat,overrunRisk);
      n1_dat = INF;
      isLogging=0;
      return;
    }

    //    
    // fill temp buffer
    for(int ii=0; ii<nbuf; ii++) diskBuffer[n1_dat+ii]= data[ii]; 
    n1_dat += nbuf;
    n1_dat %= DISK_BUFFSIZE;
    //
    // send to disk
    while((n1_dat - n0_dat + DISK_BUFFSIZE)%DISK_BUFFSIZE>=BUFFSIZE)
    {  // copy now data
       for(int ii=0;ii<BUFFSIZE;ii++) Buffer[ii]= diskBuffer[n0_dat+ii];
       n0_dat += BUFFSIZE; 
       n0_dat %= DISK_BUFFSIZE;
       overrunRisk = (n0_dat==n1_dat);
       count++;
       //
       //write data to file 
       if(!(count%10))SERIALX.printf(".");
       if(!(count%640)) SERIALX.println(); SERIALX.flush();
       //
       rc = f_write(&fil, Buffer, BUFFSIZE, &wr);
       if (rc== FR_DISK_ERR) // IO error
       {  uint32_t usd_error = usd_getError();
          SERIALX.printf(" write FR_DISK_ERR : %x\n\r",usd_error);
          // only option is to close file
          // force closing file
          count=MAX_BLOCK_COUNT; // set to max block count
       }
       else if(rc) die("write",rc);
      //
       count %= MAX_BLOCK_COUNT;
       gotData = 1;
    }
  }   
  isLogging=0; 
}


