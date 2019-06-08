//Copyright 2019 by Walter Zimmer
// Version 08-jun-19
//// use following lines for early definitions of multiple partition configuration in uSDFS.h
#define MY_VOL_TO_PART
#include "sd_config.h"
#if FF_MULTI_PARTITION		/* Multiple partition configuration */ 
	PARTITION VolToPart[] = {{DEV_SPI, 0}, //{ physical drive number, Partition: 0:Auto detect, 1-4:Forced partition)} 
							 {DEV_SDHC,0}, 
							 {DEV_USB, 0}, 
							 {DEV_USB, 1}, 
							 {DEV_USB, 2}
							 }; /* Volume - Partition resolution table */
#endif
// end of early definition

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

struct guid {
  uint8_t signature[8];  //8 bytes
  uint8_t revision[4];  //pos 3
  uint8_t hdr_sz[4];    //pos 1
  uint32_t crc32;     //single 32bit val
  uint8_t reserved[4];
  uint8_t prim_lba[8];  //pos 1 always = 1
  uint8_t back_lba[8];  //Address of backup LBA
  uint8_t first_lba[8];
  uint8_t last_lba[8];
  uint8_t disk_guid[16];
  uint8_t part_entry_lba[8];
  uint8_t number_parts[4];
  uint8_t sz_parts[4];
  uint32_t part_entry_crc32;
  uint8_t temp1[420];
}__attribute__((packed));
typedef struct guid guid_t;

struct fat32_boot {
  uint8_t jump[3];
  char    oemId[8];
  uint16_t bytesPerSector;
  uint8_t  sectorsPerCluster;
  uint16_t reservedSectorCount;
  uint8_t  fatCount;
  uint16_t rootDirEntryCount;
  uint16_t totalSectors16;
  uint8_t  mediaType;
  uint16_t sectorsPerFat16;
  uint16_t sectorsPerTrack;
  uint16_t headCount;
  uint32_t hidddenSectors;
  uint32_t totalSectors32;
  uint32_t sectorsPerFat32;
  uint16_t fat32Flags;
  uint16_t fat32Version;
  uint32_t fat32RootCluster;
  uint16_t fat32FSInfo;
  uint16_t fat32BackBootBlock;
  uint8_t  fat32Reserved[12];
  uint8_t  driveNumber;
  uint8_t  reserved1;
  uint8_t  bootSignature;
  uint32_t volumeSerialNumber;
  char     volumeLabel[11];
  char     fileSystemType[8];
  uint8_t  bootCode[420];
  uint8_t  bootSectorSig0;
  uint8_t  bootSectorSig1;
}__attribute__((packed));
typedef struct fat32_boot fat32_boot_t;

typedef uint16_t  le16_t;
typedef uint32_t  le32_t;
typedef uint64_t  le64_t;

struct exfat_super_block
{
  uint8_t jump[3];        /* 0x00 jmp and nop instructions */
  uint8_t oem_name[8];      /* 0x03 "EXFAT   " */
  uint8_t __unused1[53];      /* 0x0B always 0 */
  le64_t sector_start;      /* 0x40 partition first sector */
  le64_t sector_count;      /* 0x48 partition sectors count */
  le32_t fat_sector_start;    /* 0x50 FAT first sector */
  le32_t fat_sector_count;    /* 0x54 FAT sectors count */
  le32_t cluster_sector_start;  /* 0x58 first cluster sector */
  le32_t cluster_count;     /* 0x5C total clusters count */
  le32_t rootdir_cluster;     /* 0x60 first cluster of the root dir */
  le32_t volume_serial;     /* 0x64 volume serial number */
  struct              /* 0x68 FS version */
  {
    uint8_t minor;
    uint8_t major;
  }
  version;
  le16_t volume_state;      /* 0x6A volume state flags */
  uint8_t sector_bits;      /* 0x6C sector size as (1 << n) */
  uint8_t spc_bits;       /* 0x6D sectors per cluster as (1 << n) */
  uint8_t fat_count;        /* 0x6E always 1 */
  uint8_t drive_no;       /* 0x6F always 0x80 */
  uint8_t allocated_percent;    /* 0x70 percentage of allocated space */
  uint8_t __unused2[397];     /* 0x71 always 0 */
  le16_t boot_signature;      /* the value of 0xAA55 */
} __attribute__((packed));
typedef struct exfat_super_block exfat_boot_t;


uint32_t buffer[128];
void setup() {
  // put your setup code here, to run once:

  while(!Serial);
  Serial.println("Test diskio");
  Serial.print("uSDFS_VER:"); Serial.println(uSDFS_VER);
  
  BYTE pdrv = TEST_DRV;
  
  DSTATUS stat = disk_initialize(TEST_DRV);
  Serial.print("Disk initialize Status: "); Serial.println(STAT_ERROR_STRING[stat]);

  BYTE* buff = (BYTE *) buffer;
  DWORD sector = 0;
  DWORD sector1 = 0;
  UINT count = 1;
  UINT count1 = 1;
  
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

  // read now first partition sector
  Serial.println("\nFirst partition Sector");
  sector = mbr->part[0].firstSector;
  count = 1;
  res = disk_read (pdrv, buff, sector, count);
  Serial.print("Disk read Result: "); Serial.println(FR_ERROR_STRING[res]);
  for(int ii=0;ii<512; ii++)
  if((ii+1)%16) Serial.printf("%02x ",buff[ii]); else Serial.printf("%02x\n",buff[ii]);

  fat32_boot_t * ptr1=(fat32_boot_t *) buffer;
  exfat_boot_t * ptr2=(exfat_boot_t *) buffer;

  
  if(mbr->part[0].type == 0xee)
  {
    // read now first partition sector
    Serial.println("\nFirst partition Sector");
    sector = mbr->part[0].firstSector;
    sector1 = mbr->part[1].firstSector;
    count = 1;
    res = disk_read (pdrv, buff, sector, count);
    Serial.print("Disk read Result: "); Serial.println(FR_ERROR_STRING[res]);
    for(int ii=0;ii<512; ii++)
    if((ii+1)%16) Serial.printf("%02x ",buff[ii]); else Serial.printf("%02x\n",buff[ii]);
  
    guid_t * ptr1=(guid_t *) buffer;

    Serial.println("====  GPT GUID HEADER ====");
    Serial.print("Signature: ");
    for(uint8_t ii = 0; ii<8; ii++){ 
      Serial.print(ptr1->signature[ii], HEX);
      Serial.print(", ");
    }
    Serial.println();
  
    Serial.print("Number of Partitions: ");
    Serial.println(ptr1->number_parts[0], HEX);
  } 
  else 
  {
    // read now first partition sector
    Serial.println("\nFirst partition Sector");
    sector = mbr->part[0].firstSector;
    sector1 = mbr->part[1].firstSector;
    count = 1;
    res = disk_read (pdrv, buff, sector, count);
    Serial.print("Disk read Result: "); Serial.println(FR_ERROR_STRING[res]);
    for(int ii=0;ii<512; ii++)
    if((ii+1)%16) Serial.printf("%02x ",buff[ii]); else Serial.printf("%02x\n",buff[ii]);
  
    fat32_boot_t * ptr1=(fat32_boot_t *) buffer;
    exfat_boot_t * ptr2=(exfat_boot_t *) buffer;
  
    if(strncmp(ptr1->fileSystemType,"FAT32",5)==0)
    {
      Serial.println("FAT32");
      Serial.print("bytes per sector :");Serial.println(ptr1->bytesPerSector);
      Serial.print("sectors per cluster :");Serial.println(ptr1->sectorsPerCluster);
    }
    else if(strncmp(ptr2->oem_name,"EXFAT",5)==0)
    {
      Serial.println("EXFAT");
      Serial.print("bytes per sector :");Serial.println(1<<ptr2->sector_bits);
      Serial.print("sectors per cluster :");Serial.println(1<<ptr2->spc_bits);
    }
  
    // read now Second partition sector
    Serial.println("\nSecond partition Sector");
    count = 1;
    res = disk_read (pdrv, buff, sector1, count);
    Serial.print("Disk read Result: "); Serial.println(FR_ERROR_STRING[res]);
    for(int ii=0;ii<512; ii++)
    if((ii+1)%16) Serial.printf("%02x ",buff[ii]); else Serial.printf("%02x\n",buff[ii]);
  
    if(strncmp(ptr1->fileSystemType,"FAT32",5)==0)
    {
      Serial.println("FAT32");
      Serial.print("bytes per sector :");Serial.println(ptr1->bytesPerSector);
      Serial.print("sectors per cluster :");Serial.println(ptr1->sectorsPerCluster);
    }
    else if(strncmp(ptr2->oem_name,"EXFAT",5)==0)
    {
      Serial.println("EXFAT");
      Serial.print("bytes per sector :");Serial.println(1<<ptr2->sector_bits);
      Serial.print("sectors per cluster :");Serial.println(1<<ptr2->spc_bits);
    }
  }
  pinMode(13,OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWriteFast(13,!digitalReadFast(13));
  delay(1000);
}
