//Copyright 2016 by Walter Zimmer

#include "ff.h"                   // File System

#define SERIAL Serial1 // change to USB or HW serial

/* Stop with dying message */
void die ( FRESULT rc){  SERIAL.printf("Failed with rc=%u.\r\n", rc);  for (;;) ; }

#define BUFFSIZE (16*128) 
uint8_t buffer[BUFFSIZE] __attribute__( ( aligned ( 16 ) ) );
UINT wr;

void setup() {
  // put your setup code here, to run once:

  while(!SERIAL);
  delay(100);
  SERIAL.begin(115200,SERIAL_8N1_RXINV_TXINV);
  SERIAL.println("uSDFS");

  FRESULT rc;        /* Result code */
  FATFS fatfs;      /* File system object */
  FIL fil;        /* File object */
  DIR dir;        /* Directory object */
  FILINFO fno;      /* File information object */
  UINT bw, br;
  char buff[128];

  f_mount (&fatfs, "/", 0);      /* Mount/Unmount a logical drive */

  //-----------------------------------------------------------
  SERIAL.println("\nCreate a new file (hello10.txt).\n");
  rc = f_open(&fil, "HELLO10.TXT", FA_WRITE | FA_CREATE_ALWAYS);
  if (rc) die(rc);

  SERIAL.println("\nWrite a text data. (Hello world!)\n");
  bw = f_puts("Hello world!\n", &fil);
  bw = f_puts("Second Line\n", &fil);
  bw = f_puts("Third Line\n", &fil);
  bw = f_puts("Fourth Line\n", &fil);
  bw = f_puts("Habe keine Phantasie\n", &fil);
  if (rc) die(rc);
  
  SERIAL.println("\nClose the file.\n");
  rc = f_close(&fil);
  if (rc) die(rc);

  SERIAL.println("\nOpen same file (hello10.txt).\n");
  rc = f_open(&fil, "HELLO10.TXT", FA_READ);
  if (rc) die(rc);

  SERIAL.println("\nType the file content.\n");
  for (;;) 
  {
      if(!f_gets(buff, sizeof(buff), &fil)) break; /* Read a string from file */
      SERIAL.printf("%s\r\n",buff);
}
  if (rc) die(rc);

  SERIAL.println("\nClose the file.\n");
  rc = f_close(&fil);
  if (rc) die(rc);
  
  //-----------------------------------------------------------
  SERIAL.println("open binary file");
      // open new file
      rc = f_open(&fil, "test00.bin", FA_WRITE | FA_CREATE_ALWAYS);
      if (rc) die(rc);
  SERIAL.println("write file");
       // fill buffer
       for(int ii=0;ii<BUFFSIZE;ii++) buffer[ii]='0';
       //write data to file 
       rc = f_write(&fil, buffer, BUFFSIZE, &wr);
       if (rc) die(rc);  
       // fill buffer
       for(int ii=0;ii<BUFFSIZE;ii++) buffer[ii]='1';
       //write data to file 
       rc = f_write(&fil, buffer, BUFFSIZE, &wr);
       if (rc) die(rc);  
      //close file
  SERIAL.println("close file");
      rc = f_close(&fil);
      if (rc) die(rc);
  SERIAL.println("Logger_test done");

  //-----------------------------------------------------------
  SERIAL.println("\nOpen root directory.\n");
  rc = f_opendir(&dir, "");
  if (rc) die(rc);

   SERIAL.println("\nDirectory listing...\n");
  for (;;) 
  {
      rc = f_readdir(&dir, &fno);   /* Read a directory item */
      if (rc || !fno.fname[0]) break; /* Error or end of dir */
      if (fno.fattrib & AM_DIR)
           SERIAL.printf("   <dir>  %s\r\n", fno.fname);
      else
           SERIAL.printf("%8lu  %s\r\n", fno.fsize, fno.fname);
  }
  if (rc) die(rc);

  SERIAL.println("\nTest completed.\n");
}

void loop() {
  // put your main code here, to run repeatedly:
}
