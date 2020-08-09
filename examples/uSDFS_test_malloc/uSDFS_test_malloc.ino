#include <stdlib.h>

#include "uSDFS.h"
/*
* for APL see http://elm-chan.org/fsw/ff/00index_e.html
*/
#define TEST_DRV 1
//
#if TEST_DRV == 0
  const char *Dev = "0:/";  // SPI
#elif TEST_DRV == 1
  const char *Dev = "1:/";  // SDHC
#elif TEST_DRV == 2
  const char *Dev = "2:/";  // USB
#endif

const char *FileNameTxt = "/Ascii/HELLO13.TXT";
const char *FileNameBin = "/Binary/test03.bin";

FRESULT rc;       /* Result code */
static FATFS *fatfs;      /* File system object */                  //<<<< static is needed for DMA
FIL fil;          /* File object */
DIR dir;          /* Directory object */
FILINFO fno;      /* File information object */
UINT bw, br, wr;

TCHAR buff[128];
TCHAR *str;
#define BUFFSIZE (16*128) 
uint8_t buffer[BUFFSIZE];

void die(const char *text, FRESULT rc)
{ Serial.printf("%s: Failed with rc=%s.\r\n", text,FR_ERROR_STRING[rc]);  while(1) asm("wfi"); }

void setup() {

    pinMode(13,OUTPUT);
  digitalWriteFast(13,1);

  // put your setup code here, to run once:
  while(!Serial);
  Serial.println("Test uSDFS with malloc'ed FATFS");
  Serial.println(Dev);

  if(!(fatfs = malloc(sizeof(FATFS)))) die("Malloc failed", 0);     //<<<< malloc  fatfs
  
  if((rc = f_mount (fatfs, Dev, 1))) die("Mount",rc);      /* Mount/Unmount a logical drive */

  //-----------------------------------------------------------
  Serial.printf("\nChange drive\n");
  if((rc = f_chdrive(Dev))) die("chdrive",rc);

  //-----------------------------------------------------------
  Serial.printf("\nCreate a new subdirectories.\n");
  if((rc = f_mkdir("/Ascii"))) if(rc != FR_EXIST) die("mkDir Ascii",rc);
  if((rc = f_mkdir("/Binary"))) if(rc != FR_EXIST) die("mkDir Binary",rc);

  //-----------------------------------------------------------
  Serial.printf("\nCreate a new file %s.\n",FileNameTxt);
  if((rc = f_open(&fil, FileNameTxt, FA_WRITE | FA_CREATE_ALWAYS))) die("Open",rc);

  Serial.println("Write some text lines. (Hello world!)");
  bw = f_puts("Hello world!\n", &fil);
  bw = f_puts("Second Line\n", &fil);
  bw = f_puts("Third Line\n", &fil);
  bw = f_puts("Fourth Line\n", &fil);
  bw = f_puts("Habe keine Phantasie\n", &fil);
  
  Serial.println("Close the file.");
  if((rc = f_close(&fil))) die("Close",rc);

  Serial.printf("\nOpen same file %s.\n",FileNameTxt);
  if((rc = f_open(&fil, FileNameTxt, FA_READ))) die("Open",rc);

  Serial.println("Get the file content.");
  for (;;) 
  {
      if((str=f_gets(buff, sizeof(buff), &fil))) 
        Serial.printf("%s",str);
      else 
      { if(f_eof(&fil)) break; else die("f_gets",FR_DISK_ERR);} 
  }

  Serial.println("Close the file.");
  if((rc = f_close(&fil))) die("Close",rc);
  
  //-----------------------------------------------------------
  Serial.println("\nopen binary file");
      // open new file
      if((rc = f_open(&fil, FileNameBin, FA_WRITE | FA_CREATE_ALWAYS))) die("Open",rc);
  Serial.println("write file");
       
       // fill buffer
       for(int ii=0;ii<BUFFSIZE;ii++) buffer[ii]='0';
       //write data to file 
       if((rc = f_write(&fil, buffer, BUFFSIZE, &wr))) die("Write1",rc);  
       
       // fill buffer
       for(int ii=0;ii<BUFFSIZE;ii++) buffer[ii]='1';
       //write data to file 
       if((rc = f_write(&fil, buffer, BUFFSIZE, &wr))) die("Write2",rc);  
       
      //close file
  Serial.println("close file");
      if((rc = f_close(&fil))) die("Close",rc);
  Serial.println("Binary test done");

  //-----------------------------------------------------------
  Serial.println("\nOpen root directory.");
  if((rc = f_opendir(&dir, Dev))) die("Dir",rc);

  Serial.println("Directory listing...");
  for (;;) 
  {
      if((rc = f_readdir(&dir, &fno))) die("Listing",rc);   /* Read a directory item */
      if (!fno.fname[0]) break; /* Error or end of dir */
      if (fno.fattrib & AM_DIR)
           Serial.printf("   <dir>  %s\r\n", fno.fname);
      else
           Serial.printf("%8d  %s\r\n", (int)fno.fsize, fno.fname);
    delay(10);
  }

  Serial.println("\nTest completed.");

  if((rc = f_mount (fatfs, Dev, 1))) die("Mount",rc);      /* Mount/Unmount a logical drive */

}

void loop() {
  // put your main code here, to run repeatedly:
  //digitalWriteFast(13,!digitalReadFast(13));
  delay(1000);
}