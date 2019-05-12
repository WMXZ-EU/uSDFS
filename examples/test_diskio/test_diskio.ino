#include "uSDFS.h"
#include "diskio.h"
/*
* for APL see http://elm-chan.org/fsw/ff/00index_e.html
*/
#define TEST_DRV 2
//
#if TEST_DRV == 0
  const char *Dev = "0:/";  // SPI
#elif TEST_DRV == 1
  const char *Dev = "1:/";  // SDHC
#elif TEST_DRV == 2
  const char *Dev = "2:/";  // USB
#endif

struct partitionTable {
  uint8_t  boot;
  uint8_t  beginHead;
  unsigned beginSector : 6;
  unsigned beginCylinderHigh : 2;
  uint8_t  beginCylinderLow;
  uint8_t  type;
  uint8_t  endHead;
  unsigned endSector : 6;
  unsigned endCylinderHigh : 2;
  uint8_t  endCylinderLow;
  uint32_t firstSector;
  uint32_t totalSectors;
} __attribute__((packed));
typedef struct partitionTable part_t;

struct masterBootRecord {
  uint8_t  codeArea[440];
  uint32_t diskSignature;
  uint16_t usuallyZero;
  part_t   part[4];
  uint8_t  mbrSig0;
  uint8_t  mbrSig1;
} __attribute__((packed));
typedef struct masterBootRecord mbr_t;

uint32_t buffer[128];
void setup() {
  // put your setup code here, to run once:

  while(!Serial);
  
  BYTE pdrv = TEST_DRV;
  
  DSTATUS stat = disk_initialize(TEST_DRV);
  Serial.print("Disk initialize Status: "); Serial.println(STAT_ERROR_STRING[stat]);

  BYTE* buff = (BYTE *) buffer;
  DWORD sector = 0;
  UINT count = 1;
  DRESULT res = disk_read (pdrv, buff, sector, count);
  Serial.print("Disk read Result: "); Serial.println(FR_ERROR_STRING[res]);
  for(int ii=0;ii<512; ii++)
  if((ii+1)%16) Serial.printf("%02x ",buff[ii]); else Serial.printf("%02x\n",buff[ii]);

  mbr_t *mbr = (mbr_t *) buffer;
  Serial.println("\nMaster Boot Record");
  for(int ii=0;ii<4;ii++)
  {
    Serial.print("  Partition: "); Serial.print(ii);
    Serial.print(" first Sector: ");
    Serial.print(mbr->part[ii].firstSector);
    Serial.print(" total Sectors: ");
    Serial.println(mbr->part[ii].totalSectors);
  }
  pinMode(13,OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWriteFast(13,!digitalReadFast(13));
  delay(1000);
}
