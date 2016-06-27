//Copyright 2016 by Walter Zimmer

#include "ff.h"                   // File System

/* Stop with dying message */
void die ( FRESULT rc){  Serial.printf("Failed with rc=%u.\n", rc);  for (;;) ; }

void setup() {
  // put your setup code here, to run once:

  while(!Serial);
  delay(100);
  Serial.println("uSDFS");

  FRESULT rc;        /* Result code */
  FATFS fatfs;      /* File system object */
  FIL fil;        /* File object */
  DIR dir;        /* Directory object */
  FILINFO fno;      /* File information object */
  UINT bw, br;
  char buff[128];

  f_mount (&fatfs, "/", 0);      /* Mount/Unmount a logical drive */

  //-----------------------------------------------------------
  Serial.printf("\nCreate a new file (hello.txt).\n");
  rc = f_open(&fil, "HELLO10.TXT", FA_WRITE | FA_CREATE_ALWAYS);
  if (rc) die(rc);

  Serial.printf("\nWrite a text data. (Hello world!)\n");
  bw = f_puts("Hello world!\n", &fil);
  bw = f_puts("Second Line\n", &fil);
  bw = f_puts("Third Line\n", &fil);
  bw = f_puts("Fourth Line\n", &fil);
  bw = f_puts("Habe keine Phantasie\n", &fil);
  if (rc) die(rc);
  
  Serial.printf("\nClose the file.\n");
  rc = f_close(&fil);
  if (rc) die(rc);

  Serial.printf("\nOpen a test file (message.txt).\n");
  rc = f_open(&fil, "HELLO10.TXT", FA_READ);
  if (rc) die(rc);

  Serial.printf("\nType the file content.\n");
  for (;;) 
  {
      if(!f_gets(buff, sizeof(buff), &fil)) break; /* Read a string from file */
      Serial.printf("%s",buff);
}
  if (rc) die(rc);

  Serial.printf("\nClose the file.\n");
  rc = f_close(&fil);
  if (rc) die(rc);

  Serial.printf("\nOpen root directory.\n");
  rc = f_opendir(&dir, "");
  if (rc) die(rc);

   Serial.printf("\nDirectory listing...\n");
  for (;;) 
  {
      rc = f_readdir(&dir, &fno);   /* Read a directory item */
      if (rc || !fno.fname[0]) break; /* Error or end of dir */
      if (fno.fattrib & AM_DIR)
           Serial.printf("   <dir>  %s\n", fno.fname);
      else
           Serial.printf("%8lu  %s\n", fno.fsize, fno.fname);
  }
  if (rc) die(rc);

  Serial.printf("\nTest completed.\n");
}


void loop() {
  // put your main code here, to run repeatedly:
}
