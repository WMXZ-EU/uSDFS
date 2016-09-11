//Copyright 2016 by Walter Zimmer

#include "sdhc.h"

DWORD   sector0  = 0, sector; // start sector to be read

#define COUNT 4// multiblock count
// un comment next line when performance should be measured 
#define PERFORMANCE

#define LOOPS (10)

#define MBLK 6
uint8_t buffer[(1<<MBLK)*512] __attribute__((aligned(4)));
extern uint8_t m_sdhc_CMD6_Status[];
extern uint32_t m_sdhc_ocr;
extern SD_CARD_DESCRIPTOR sdCardDesc;

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
  int ii;
  pinMode(13,OUTPUT);
  digitalWrite(13,LOW);
  while(!Serial);
  digitalWrite(13,HIGH);
  delay(100);
  Serial.println("SDHC_TEST");

  // Initialize card 
  DSTATUS stat = SDHC_InitCard(); 
  Serial.printf("%d %d\n\r",stat, SDHC_Baudrate());

  Serial.printf("use DMA? %d\n\r",SDHC_TRANSFERTYPE == SDHC_TRANSFERTYPE_DMA);
  for(int ii=0;ii<(COUNT+2)*512;ii++) buffer[ii]=0xff;

// read first COUNT sectors of disk
// print out the first COUNT+2 sectors from buffer
// to see how many sectors were really read
// first sector is MBR having 0x55 0xaa at the end 
// other disk sectors should be 0
// additional buffers should be -1
//

  DRESULT res = SDHC_ReadBlocks((UCHAR*) buffer, (DWORD) 0, COUNT);
  SDHC_DMAWait();
  
  Serial.println(" Partitions");
  Serial.println(" Start \tSize");
  for(ii=0;ii<4;ii++)
      Serial.printf(" %d \t%d\r\n", *(uint32_t*)&buffer[454+8*ii],
                                    *(uint32_t*)&buffer[454+8*ii+4]); 

  sector0 = *(uint32_t*)&buffer[454];
  Serial.printf("Start Sector 0x%x\n\r",sector0);
  Serial.printf("BLKATTR: %x\n\r",SDHC_BLKATTR);
#ifndef PERFORMANCE
  matrixPrint((void*) buffer,COUNT+2);
#endif

  for(ii=0;ii<17;ii++) Serial.printf("%x ",m_sdhc_CMD6_Status[ii]);
  Serial.println(" ");
  Serial.printf("ocr: %x\n\r",m_sdhc_ocr);
  Serial.printf("%d %x  %d  %d  %u %d \n\r",
                             sdCardDesc.status,
                             sdCardDesc.address,
                             sdCardDesc.highCapacity,  
                             sdCardDesc.version2,
                             sdCardDesc.numBlocks,
                             sdCardDesc.lastCardStatus);
  Serial.flush();

  
}

void loop() {
  // put your main code here, to run repeatedly:

  static unsigned int nn=0;
  static unsigned int cnt=0;

  static unsigned int iblk=0, nblk;

#ifndef PERFORMANCE
    return;
#endif
     
    if(nn==LOOPS) 
    { 
      Serial.println("done"); nn=0; iblk++;
      Serial.printf("%d %d %d %d\n\r", nn,cnt,iblk,nblk);
    }
    if(iblk>MBLK) 
    { digitalWrite(13,LOW); 
      delay(100); 
      digitalWrite(13,HIGH); 
      delay(200);
      return;
    }
    if(!nn) sector = sector0;
    nblk=1<<iblk;
    
    static uint32_t t0=0,t1;
    delayMicroseconds(100); // need this workaround, don't know why

    t0=micros();
    DRESULT res = SDHC_ReadBlocks((UCHAR*) buffer, (DWORD) sector, nblk);
    if(res != RES_OK) {Serial.printf("res=%d\n\r",res); iblk=MBLK+1; Serial.flush();}
    SDHC_DMAWait();
    t1=micros();
    sector+=nblk;
    cnt++;
    //
    static unsigned long tmin=1<<31,tmax=0;
    static float tmean=0;
    long dt=(t1-t0);
    if(tmin>dt) tmin=dt;
    if(tmax<dt) tmax=dt;
    tmean = tmean + dt/(1000.0f); 
    if(!(cnt%1000)) 
    { 
      float Mbs = nblk*512.0f/tmean;
      Serial.printf("%d %4d %4d %7.2f (%5.2f MB/s)\n\r",nblk, tmin,tmax,tmean,Mbs);
      tmean=0.0;
      tmin = 1<<31;
      tmax=0;
      nn++;
      delay(10);
    }
}
