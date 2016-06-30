//Copyright 2016 by Walter Zimmer

#include "sdhc.h"

DWORD   sector  = 10000000;
uint8_t buffer[10240] __attribute__((aligned(4)));

void matrixPrint(void *buff,int nbuff)
{ int ii,jj;
  char *data=(char*)buff;
  for(jj=0;jj<nbuff;jj++)
  {
  for(ii=0;ii<512;ii++) 
  { if(!(ii%16))Serial.println(); 
    Serial.printf("%02x ",data[ii+jj*512]);
  }
  Serial.println();
  }
}

void setup() {
  // put your setup code here, to run once:

  while(!Serial);
  delay(100);
  Serial.println("SDHC_TEST");
  // Initialize card with
  // clock rate in kHz (relation to baudrate?)
  // here in relation to F_CPU
  DSTATUS stat = SDHC_InitCard((F_CPU/1000)/3); 
  Serial.printf("%d %d\n\r",stat, SDHC_Baudrate());

  Serial.printf("use DMA? %d\n\r",SDHC_TRANSFERTYPE == SDHC_TRANSFERTYPE_DMA);
  for(int ii=0;ii<10240;ii++) buffer[ii]=0xff;

// print out the first 5 sectors of a disk
// first sector is MBR having 0x55 0xaa at the end 

// sector count (last parameter) seems to be always rounded up to an odd number
// values of 4 or 5 will always transfer 5 sectors
// if this is a K66 bug or simply insufficient programming is unknown
#define COUNT 4
//
  DRESULT res = SDHC_ReadBlocks((UCHAR*) buffer, (DWORD) 0, COUNT);
  while(!SDHC_GetDMAStatus());
  
  Serial.println(" Partitions");
  Serial.println(" Start \tSize");
  for(int ii=0;ii<4;ii++)
      Serial.printf(" %d \t%d\r\n", *(uint32_t*)&buffer[454+8*ii],
                                    *(uint32_t*)&buffer[454+8*ii+4]); 

  Serial.printf("BLKATTR: %x\n\r",SDHC_BLKATTR);
  matrixPrint((void*) buffer,10);
}

void loop() {
  // put your main code here, to run repeatedly:

  // comment next line when performance should be measured 
    return;
    
    static uint32_t t0=0,t1;
    t1=micros();
    if((sector%1000)<COUNT) 
    { float Mbaud=8*512*1000.0/(t1-t0);
      Serial.printf("%d: %f s; %f Mbaud\n\r",
                  sector,(t1-t0)/1000000.0, Mbaud);
      t0=t1;
    }
    
    DRESULT res = SDHC_ReadBlocks((UCHAR*) buffer, (DWORD) sector, COUNT);
    while(!SDHC_GetDMAStatus());
      
//    matrixPrint((void*) buffer,3);

    sector+=COUNT;
}
