//Copyright 2016 by Walter Zimmer
//
#include "ff.h"
#include "sdhc.h"

FRESULT rc;        /* Result code */
FATFS fatfs;      /* File system object */
FIL fil;        /* File object */

void die( FRESULT rc);
void setup();
void loop();
void timer_init(void);
void timer_start(void);
struct tm seconds2tm(uint32_t tt);

/* Stop with dying message */
void die( FRESULT rc)
{   Serial.printf("Failed with rc=%u.\n", rc); 
    Serial.flush();
    for (;;) ; 
}

void setup()
{
  while(!Serial);
  Serial.printf("Logger_test\n");
  
  f_mount (&fatfs, "/", 0);      /* Mount/Unmount a logical drive */

  timer_init();
  timer_start();
}

void loop()
{
}

//-------------------------------------------------------------------	

void timer_init(void)
{
	SIM_SCGC6 |= SIM_SCGC6_PIT;
	// turn on PIT     
	PIT_MCR = 0x00;
}

void timer_start(void) 
{//    
	PIT_LDVAL0 = F_BUS/150; // setup timer 0 for (F_BUS/frequency) cycles     
	PIT_TCTRL0 = 2; // enable Timer 0 interrupts      
	NVIC_SET_PRIORITY(IRQ_PIT_CH0, 8*16); 
	NVIC_ENABLE_IRQ(IRQ_PIT_CH0);
	PIT_TCTRL0 |= 1; // start Timer 0
}

void doProcessing(void);
//
void pit0_isr(void)
{ //
	PIT_TFLG0=1;
	doProcessing();
}

uint32_t isProcessing=0;
uint32_t procCount=0;
uint32_t ifn=0;
uint32_t isFileOpen=0;
char filename[80];

#define BUFFSIZE (16*128) 
uint8_t buffer[BUFFSIZE] __attribute__( ( aligned ( 16 ) ) );
UINT wr;

void doProcessing(void)
{	//
  if(isProcessing)
  {
    Serial.printf("-");
    return;  
  }
  isProcessing=1;

  if(!(procCount%1000))
  {
    if(isFileOpen)
    {
      //close file
      rc = f_close(&fil);
      if (rc) die(rc);
      //
      isFileOpen=0;
    }
    //
    if(!isFileOpen)
    {
      // open new file
      sprintf(filename,"t_%05d.dat",ifn++);
      Serial.println(filename);
      rc = f_open(&fil, filename, FA_WRITE | FA_CREATE_ALWAYS);
      if (rc) die(rc);
      //
      isFileOpen=1;
    }
  }
  
  if(isFileOpen)
  {
       Serial.printf("."); if(!(procCount%64)) Serial.println("");
       // fill buffer
       for(int ii=0;ii<BUFFSIZE;ii++) buffer[ii]='0'+(procCount%10);
       //write data to file 
       rc = f_write(&fil, buffer, BUFFSIZE, &wr);
       if (rc) die(rc);  
    //    
  }
  
  procCount++;
  isProcessing=0;
}
