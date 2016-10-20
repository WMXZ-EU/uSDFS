//Copyright 2016 by Walter Zimmer

#include <wchar.h>
#include "ff.h"                   // File System

#define USE_USB_SERIAL
#ifdef USE_USB_SERIAL
	#define SERIALX Serial
#else
	#define SERIALX Serial1
#endif

/* Stop with dying message */
void die(const char *text, FRESULT rc)
{ SERIALX.printf("%s: Failed with rc=%u.\r\n", text,rc);  for (;;) delay(100); }

FRESULT rc;        /* Result code */
FATFS fatfs;      /* File system object */
FIL fil;        /* File object */
DIR dir;        /* Directory object */
FILINFO fno;      /* File information object */
UINT bw, br;
TCHAR buff[128];

#define BUFFSIZE (16*128) 
uint8_t buffer[BUFFSIZE] __attribute__( ( aligned ( 16 ) ) );
UINT wr;

// the following is needed to print TCHAR strings (16 bit) on Teensy
// exFAT requires 16 bit TCHAR  
char text[80];
char * tchar2char( TCHAR * tcharString, int nn, char * charString)
{   int ii;
    for(ii = 0; ii<nn; ii++) 
    { charString[ii] = (char)tcharString[ii];
      if(!charString[ii]) break;
    }
    return charString;
}

/****************** setup ******************************/
void setup() {
  // put your setup code here, to run once:

  while(!SERIALX);
  #ifndef USB_SERIAL
  	SERIALX.begin(115200,SERIAL_8N1_RXINV_TXINV);
  #endif
  SERIALX.println("uSDFS");

  // 0: is first disk (configured in diskconfig.h)
  TCHAR *device = (TCHAR *)_T("0:/");
  
  f_mount (&fatfs, device, 0);      /* Mount/Unmount a logical drive */

  //-----------------------------------------------------------
  SERIALX.println("\nCreate a new file (hello10.txt).");
  rc = f_open(&fil, (TCHAR*)_T("HELLO10.TXT"), FA_WRITE | FA_CREATE_ALWAYS);
  if (rc) die("Open",rc);

  SERIALX.println("Write some text lines. (Hello world!)");
  bw = f_puts((TCHAR*)_T("Hello world!\n"), &fil);
  bw = f_puts((TCHAR*)_T("Second Line\n"), &fil);
  bw = f_puts((TCHAR*)_T("Third Line\n"), &fil);
  bw = f_puts((TCHAR*)_T("Fourth Line\n"), &fil);
  bw = f_puts((TCHAR*)_T("Habe keine Phantasie\n"), &fil);
  if (rc) die("Puts",rc);
  
  SERIALX.println("Close the file.");
  rc = f_close(&fil);
  if (rc) die("Close",rc);

  SERIALX.println("\nOpen same file (hello10.txt).");
  rc = f_open(&fil, (TCHAR*) _T("HELLO10.TXT"), FA_READ);
  if (rc) die("Open",rc);

  SERIALX.println("Get the file content.");
  for (;;) 
  {
      if(!f_gets(buff, sizeof(buff), &fil)) break; /* Read a string from file */
      SERIALX.printf("%s",tchar2char(buff,80,text));
}
  if (rc) die("Gets",rc);

  SERIALX.println("Close the file.");
  rc = f_close(&fil);
  if (rc) die("Close",rc);
  
  //-----------------------------------------------------------
  SERIALX.println("\nopen binary file");
      // open new file
      rc = f_open(&fil, (TCHAR*)_T("test00.bin"), FA_WRITE | FA_CREATE_ALWAYS);
      if (rc) die("Open",rc);
  SERIALX.println("write file");
       
       // fill buffer
       for(int ii=0;ii<BUFFSIZE;ii++) buffer[ii]='0';
       //write data to file 
       rc = f_write(&fil, buffer, BUFFSIZE, &wr);
       if (rc) die("Write1",rc);  
       
       // fill buffer
       for(int ii=0;ii<BUFFSIZE;ii++) buffer[ii]='1';
       //write data to file 
       rc = f_write(&fil, buffer, BUFFSIZE, &wr);
       if (rc) die("Write2",rc);  
       
      //close file
  SERIALX.println("close file");
      rc = f_close(&fil);
      if (rc) die("Close",rc);
  SERIALX.println("Binary test done");

  //-----------------------------------------------------------
  SERIALX.println("\nOpen root directory.");
  rc = f_opendir(&dir, (TCHAR*)_T(""));
  if (rc) die("Dir",rc);

   SERIALX.println("Directory listing...");
  for (;;) 
  {
      rc = f_readdir(&dir, &fno);   /* Read a directory item */
      if (rc || !fno.fname[0]) break; /* Error or end of dir */
      if (fno.fattrib & AM_DIR)
           SERIALX.printf("   <dir>  %s\r\n", tchar2char(fno.fname,80,text));
      else
           SERIALX.printf("%8d  %s\r\n", (int)fno.fsize, tchar2char(fno.fname,80,text)); // fsize is QWORD for exFAT
  }
  if (rc) die("Listing",rc);

  SERIALX.println("\nTest completed.");
}

/****************** loop ******************************/
void loop() {
  // put your main code here, to run repeatedly:
}


