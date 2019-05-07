//Copyright 2016 by Walter Zimmer
// Version 04-05-17
//
#include "ff.h"
#include "ff_utils.h"

#define TEST_DRV 1
//
#if TEST_DRV == 0
  const char *Dev = "0:/";  // SPI
#elif TEST_DRV == 1
  const char *Dev = "1:/";  // SDHC
#elif TEST_DRV == 2
  const char *Dev = "2:/";  // USB
#endif

#define NCH 4 // number of channels
#define NSAMP 128 // number of words in data buffer per channel
#define NDAT (NSAMP*NCH) // words in data buffer (input)
#define NBYTES (2*NDAT) // bytes in data buffer (input)
#define FSAMP 192000

void setup();
void loop();

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

extern "C" uint32_t usd_getError(void);

class c_mFS
{
	private:
		FRESULT rc;     /* Result code */
		FATFS fatfs;    /* File system object */
		FIL fil;        /* File object */

		UINT wr;
		
		char device[40];
		char filename[80];
	
		/* Stop with dying message */
		void die(char *str, FRESULT rc) 
		{ Serial.printf("%s: Failed with rc=%u.\n", str, rc); for (;;) delay(100); }
		
	public:
		void init(void)
		{
			if (rc = f_mount (&fatfs, device, 0)) die("mount", rc);    /* Mount/Unmount a logical drive */
		}
		
		void open( char * fmt, uint32_t ifn)
		{
			sprintf(filename,fmt,ifn);
			Serial.println(filename);
			//
			// check status of file
			rc =f_stat(filename,0);
			Serial.printf("stat %d %x\n",rc,fil.obj.sclust);

			rc = f_open(&fil, filename, FA_WRITE | FA_CREATE_ALWAYS);
			Serial.printf(" opened %d %x\n\r",rc,fil.obj.sclust);
			// check if file is Good
			if(rc == FR_INT_ERR)
			{ // only option is to close file
				rc = f_close(&fil);
				if(rc == FR_INVALID_OBJECT)
				{ Serial.println("unlinking file");
				  rc = f_unlink(wfilename);
				  if (rc) die("unlink", rc);
				}
				else
				  die("close", rc);
				
			}
			// retry open file
			if(rc = f_open(&fil, filename, FA_WRITE | FA_CREATE_ALWAYS)) die("open", rc);
		}
		
		void close()
		{
			if (rc = f_close(&fil)) die("close", rc);
		}
		
		uint32_t write( uint8_t *buffer, uint32_t nbuf)
		{
			rc = f_write(&fil, buffer, nbuf, &wr);
			if (rc== FR_DISK_ERR) // IO error
			{	uint32_t usd_error = usd_getError();
				Serial.printf(" write FR_DISK_ERR : %x\n\r",usd_error);
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


//=========================================================================
void blink(uint32_t msec)
{ static uint32_t to=0,t1; 
  t1=millis();
  if(t1-to<msec) return;
  digitalWriteFast(13,!digitalReadFast(13)); 
  to=t1;
}
/***************************************************************/
void  doLogging( uint8_t *buffer, uint16_t ndat);

#define FREQ (FSAMP/NSAMP) // firing rate for data block input (simulated acquisition)
uint32_t Data[NDAT]; 		// from source
uint32_t count=0;

#if defined(__MK20DX256__) ||  defined(__MK66FX1M0__)
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

#elif defined(__IMXRT0162__)
void timer_init(void)
{
//  SIM_SCGC6 |= SIM_SCGC6_PIT;
//  // turn on PIT     
//  PIT_MCR = 0x00;
}

void timer_start(void) 
{//    
//  PIT_LDVAL0 = F_BUS/FREQ; // setup timer 0 for (F_BUS/frequency) cycles     
//  PIT_TCTRL0 = 2; // enable Timer 0 interrupts      
  NVIC_SET_PRIORITY(IRQ_PIT_CH0, 7*16); 
  NVIC_ENABLE_IRQ(IRQ_PIT_CH0);
//  PIT_TCTRL0 |= 1; // start Timer 0
}

//
void pit0_isr(void)
{ //
//  PIT_TFLG0=1;
  for(int ii=0; ii<NDAT; ii++) Data[ii]=(count + ii) % 256;
  doLogging(Data, NDAT);
}
#endif

/*-----------------------------------------------------------*/
void setup()
{
	// wait for serial line to come up
	pinMode(13,OUTPUT);
	pinMode(13,HIGH);
	while(!Serial) blink(100);

	mFS.init();

	timer_init();
	timer_start();
}

void loop()
{	delay(1000);
}

/***************************** LOGGING ***********************/
uint32_t ifn=0;
#define MXFN 100 // maximal number of files 
#define MAX_BLOCK_COUNT 1000  // number of DISK_BUFFSIZE writes to file

#if defined(__MK20DX256__)
  #define DISK_BUFFSIZE (4*NDAT) // size in words of buffer to be written
  #define MEM_BUFFSIZE (2*DISK_BUFFSIZE) // size in words of memory buffer 
#elif defined(__MK66FX1M0__)
  #define DISK_BUFFSIZE (16*NDAT) // size in words of buffer to be written
  #define MEM_BUFFSIZE (4*DISK_BUFFSIZE) // size in words of memory buffer
#elif defined(__IMXRT1062__)
  #define DISK_BUFFSIZE (16*NDAT) // size in words of buffer to be written
  #define MEM_BUFFSIZE (4*DISK_BUFFSIZE) // size in words of memory buffer
#endif

#define INF ((uint32_t) (-1))
uint32_t diskBuffer[DISK_BUFFSIZE]; // to disk
uint32_t memBuffer[MEM_BUFFSIZE]; // memory storage

uint32_t isFileOpen=0;
uint32_t t0=0;
uint32_t t1=0;

static uint32_t n0_dat = 0; // read_pointer
static uint32_t n1_dat = INF; // write pointer
  
void storeBufferData(uint8_t *data, uint16_t nbuf)
{
	// copy first data to memmory
	for(int ii=0; ii<nbuf; ii++) memBuffer[n1_dat+ii]= data[ii]; 
	n1_dat += nbuf;
	n1_dat %= MEM_BUFFSIZE; // is save as 
}

void *getBufferData(uint32_t ndat)
{ void *ptr;
  if(n1_dat==INF) return 0; // no data yet
  //
  if((n1_dat - n0_dat + MEM_BUFFSIZE) & MEM_BUFFSIZE) > ndat)
  { ptr = (void *)&memBuffer[n0_dat];
	n0_dat += ndat;
	n0_dat %= MEM_BUFFSIZE;
  }
  else
	ptr=0;
  return ptr;
}	

/*-------------------------------------------------------------------------*/
void doLogging(uint8_t *data, uint16_t nbuf)
{ // data point to input having a size of nbuf bytes
  // is called whenever new data are available
  //
  // keeps state of filing 
  // does open/close of file when required
  //
  static uint16_t overrunRisk=0;
  static uint16_t gotData=0; 

  static uint16_t isLogging = 0;

  if(isLogging) return;
  isLogging=1;
  if(ifn>MXFN) { blink(500); isLogging=0; return; }

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
      float MBs = (1000.0f*DISK_BUFFSIZE*4)/(1.0f*(t1-t0));
      Serial.printf(" (%d - %f MB/s)\n\r",t1-t0,MBs);
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
  //
  if(isFileOpen)
  {
	//write data to file 
	count++;
	if(!(count%10))Serial.printf(".");
	if(!(count%640)) Serial.println(); Serial.flush();
	//
	if(!mFS.write(data, nbuf)) count=MAX_BLOCK_COUNT; // on error set to max block count to force closing file

	count %= MAX_BLOCK_COUNT; 
	gotData = 1;
  }
  isLogging=0; 
}


