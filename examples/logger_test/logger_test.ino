//Copyright 2016 by Walter Zimmer
// Version 29-jun-16
//
#include <stdio.h>
#include "ff.h"

#define SERIALX Serial1

FRESULT rc;        /* Result code */
FATFS fatfs;      /* File system object */
FIL fil;        /* File object */

void die( FRESULT rc);
void setup();
void loop();
void timer_init(void);
void timer_start(void);
struct tm seconds2tm(uint32_t tt);

#define BUFFSIZE (16*128) 
uint8_t buffer[BUFFSIZE] __attribute__( ( aligned ( 16 ) ) );
UINT wr;

/* Stop with dying message */
void die( FRESULT rc) { SERIALX.printf("Failed with rc=%u.\n", rc); for (;;) ; }

void setup()
{
  while(!SERIALX);
  SERIALX.begin(115200,SERIAL_8N1_RXINV_TXINV);
  SERIALX.println("\nLogger_test");
  
  f_mount (&fatfs, (TCHAR *)L"/", 0);      /* Mount/Unmount a logical drive */

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
	NVIC_SET_PRIORITY(IRQ_PIT_CH0, 9*16); 
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
TCHAR wfilename[80];

TCHAR * char2tchar( char * charString, size_t nn, TCHAR * tcharString)
{ int ii;
  for(ii = 0; ii<nn; ii++) tcharString[ii] = (TCHAR) charString[ii];
  return tcharString;
}

char * char2tchar(  TCHAR * tcharString, size_t nn, char * charString)
{ int ii;
  for(ii = 0; ii<nn; ii++) charString[ii] = (char) tcharString[ii];
  return charString;
}

void doProcessing(void)
{	//
  if(isProcessing)
  {
    SERIALX.printf("-");
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
      SERIALX.println(filename);SERIALX.flush();
      char2tchar(filename,80,wfilename);
      rc = f_open(&fil, wfilename, FA_WRITE | FA_CREATE_ALWAYS);
      if (rc) die(rc);
      //
      isFileOpen=1;
    }
  }
  
  if(isFileOpen)
  {
       SERIALX.printf("."); if(!(procCount%64)) SERIALX.println();SERIALX.flush();
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
