//Copyright 2016 by Walter Zimmer
// Version 06-05-17
//
#define USE_USB_SERIAL
#ifdef USE_USB_SERIAL
	#define SERIALX Serial
#else
	#define SERIALX Serial1
#endif

#define NCH 4 // number of channels
#define NSAMP 128 // number of words in data buffer per channel
#define FSAMP 192000  // sampling frequency

#define N_BITS (16)

#if N_BITS == 16
#define N_SHIFT (0)
#else
#define N_SHIFT (16)
#endif

#define NDAT (NSAMP*NCH) // words in data buffer (input)
#define NBYTES ((N_BITS/8)*NDAT) // bytes in data buffer (input)

// allocate DMA buffer for data aquisition
#define NBUF (2*NBYTES)	// dual buffer (DMA buffer is twice desired buffer)
#if N_BITS == 16
	DMAMEM int16_t rx_buffer[NBUF] __attribute__( ( aligned ( 16 ) ) );
#elif N_BITS == 32
	DMAMEM int32_t rx_buffer[NBUF] __attribute__( ( aligned ( 16 ) ) );
#endif

//==================================================================================================

/*
 * FS specific interface 
 * could go into own file
 * change to your preferred SD library
 * need following methods
 * 		void init(void)
 * 		void open( char * fmt, uint32_t ifn)
 *		void close()
 *		uint32_t write( uint8_t *buffer, uint32_t nbuf)
 * description of API should be obvious from uSDFS example below
 *
*/
#include "ff.h"
#include "ff_utils.h"

extern "C" uint32_t usd_getError(void);

class c_mFS
{
	private:
		FRESULT rc;     /* Result code */
		FATFS fatfs;    /* File system object */
		FIL fil;        /* File object */

		UINT wr;
		
		char filename[80];
		TCHAR wfilename[80];
	
		/* Stop with dying message */
		void die(char *str, FRESULT rc) 
		{ SERIALX.printf("%s: Failed with rc=%u.\n", str, rc); for (;;) yield(); }
		
	public:
		void init(void)
		{
			rc = f_mount (&fatfs, (TCHAR *)_T("0:/"), 0);      /* Mount/Unmount a logical drive */
			if (rc) die("mount", rc);
		}
		
		void open( char * fmt, uint32_t ifn)
		{
			sprintf(filename,fmt,ifn);
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
		}
		
		void close()
		{
			rc = f_close(&fil);
			if (rc) die("close", rc);
		}
		
		uint32_t write( uint8_t *buffer, uint32_t nbuf)
		{
			rc = f_write(&fil, buffer, nbuf, &wr);
			if (rc== FR_DISK_ERR) // IO error
			{	uint32_t usd_error = usd_getError();
				SERIALX.printf(" write FR_DISK_ERR : %x\n\r",usd_error);
				// only option is to close file
				// force closing file
				return 0;
			}
			else if(rc) die("write",rc);
			return nbuf;
		}
};

c_mFS mFS;

/*
	end of FS specific interface
*/

//==================================================================================================
/*
*	I2S quad interface
*	uses wmxz_core
*/
#include "dma.h"
#include "i2s.h"

void  doProcessing( uint8_t *buffer, uint16_t ndat);
void  doLogging( uint8_t *buffer, uint16_t ndat);
uint32_t i2sCount=0;

class c_mI2S
{
	public:
	void init(void)
	{
		i2s_init();
		i2s_speedConfig(AD7982_DEV, N_BITS, FSAMP);
		i2s_config(1, N_BITS, 1I2S_RX_2CH, 0);
		i2s_configurePorts(2);

		DMA_init();
		i2s_setupInput(rx_buffer,NBUF,2,8); //port, prio (8=normal)

	}
	void start(void)
	{
		i2s_enableInputDMA();
		DMA_startAll();
		i2s_startInput();
	}
};
c_mI2S mI2S;

// following is callback from I2S ISR
void i2sInProcessing(void * s, void * d)
{	int ii,jj,kk;
	static int i2sIsProcessing=0;
	static int idec=0;

	if(i2sIsProcessing) return;
	i2sIsProcessing=1;

	doprocessing((uint8_t *)d, NBYTES);
	doLogging((uint8_t *)d, NBYTES);
	
	i2sCount++;
	i2sIsProcessing=0;
}

/*
*	end I2S quad interface
*/

//==================================================================================================
void blink(uint32_t msec)
{ static uint32_t to=0,t1; 
  t1=millis();
  if(t1-to<msec) return;
  digitalWriteFast(13,!digitalReadFast(13)); 
  to=t1;
}

/***************************** Processing ***********************/
float avg[NCH]={0,0,0,0};
float sqr[NCH]={0,0,0,0};
float var[NCH]={0,0,0,0};

/*-------------------------------------------------------------------------*/
void doProcessing(uint8_t *data, uint16_t nbuf)
{
  static uint16_t isProcessing = 0;

  if(isProcessing) return;
  isProcessing=1;

#if N_BITS == 16
	int16_t *ptr = (int16_t *) d;
#elif N_BITS == 32
	int32_t *ptr = (int32_t *) d;
#endif
	for(ii=0;ii<NCH; ii++)
	{
		avg[ii]=0.0f; sqr[ii]=0.0f;
	}

	for(ii=0; ii<NDAT; ii++)
	{
		float val = (float) (ptr[ii]>>N_SHIFT);
		avg[ii % NCH] += val;
		sqr[ii % NCH] += val*val;
	}

	for(ii=0;ii<NCH;ii++)
		var[ii]=(sqr[ii]-avg[ii]*avg[ii]/NDAT);

  isProcessing=0;
  
}


/***************************** LOGGING ***********************/
uint32_t ifn=0;
#define MXFN 100 // maximal number of files 
#define MAX_BLOCK_COUNT 1000  // number of BUFFSIZE writes to file

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

uint32_t isFileOpen=0;
uint32_t t0=0;
uint32_t t1=0;


/*-------------------------------------------------------------------------*/
void doLogging(uint8_t *data, uint16_t nbuf)
{ // data point to input having a size of nbuf bytes
  // is called whenever new data are available
  //
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
		mFS.close();
      //
      isFileOpen=0;
      t1=micros();
      float MBs = (1000.0f*BUFFSIZE)/(1.0f*(t1-t0));
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

	mFS.open("X_%05d.dat",ifn);
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
	  if(!mFS.write(Buffer, BUFFSIZE)) 
		  count=MAX_BLOCK_COUNT; // set to max block count to force closing file
       count %= MAX_BLOCK_COUNT;
       gotData = 1;
    }
  }   
  isLogging=0; 
}


//==================================== main ==============================================================
void setup(void)
{
	// wait for serial line to come up
	pinMode(13,OUTPUT);
	pinMode(13,HIGH);
	while(!SERIALX && millis()<3000) blink(100);

	#ifndef USB_SERIAL
	SERIALX.begin(115200,SERIAL_8N1_RXINV_TXINV);
	#endif
	SERIALX.println("\nLogger Buffered Write Test");
	SERIALX.printf("F_BUS %d MHz\n\r",F_BUS/1000000);
	SERIALX.printf("Sample Rate %d Hz\n\r",FSAMP);
	SERIALX.printf("Buffer Update Rate %d Hz\n\r",FREQ);

	mFS.init();

	for(int ii=0;ii<NCH;ii++) {avg[ii]=sqr[ii]=var[ii]=0.0f;
	
	//------------- initialize aquisition --------------------------------------------------
	mI2S.init();
	//---------------Lets go---------------------------------------------------------
	mI2S.start();

}

void loop(void) 
{
	// put your main code here, to run repeatedly:
	//
	int ii;
	delay(1000);
	SERIALX.print(i2sCount); SERIALX.print(" | ");
	for(ii=0; ii<NCH; ii++) { SERIALX.print(avg[ii]/NDAT); SERIALX.print(" ");} SERIALX.print(" | ");
	for(ii=0; ii<NCH; ii++) { SERIALX.print(sqrtf(var[ii]/NDAT)); SERIALX.print(" ");}
	SERIALX.println("");
	i2sCount=0;
}
