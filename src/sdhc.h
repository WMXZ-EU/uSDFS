
// following code is modified  by Walter Zimmer from 
// version provided by
// Petr Gargulak (NXP Employee) 
//https://community.nxp.com/servlet/JiveServlet/download/339474-1-263510/SDHC_K60_Baremetal.ZIP
//see also
//https://community.nxp.com/thread/99202

#ifndef _SDHC_H
#define _SDHC_H
/******************************************************************************
* Includes
******************************************************************************/

#include "kinetis.h"
#ifndef SDHC_DSADDR_DSADDR_MASK 
#include "localKinetis.h"
#endif

// type definitions for 'original' sdhc.c 
typedef unsigned int   LWord;
typedef signed int     sLWord;
typedef unsigned short  Word;
typedef signed short    sWord;
typedef unsigned char   Byte;
typedef signed char     sByte;

#include "diskio.h"

#if 0

/* These types must be 16-bit, 32-bit or larger integer */
typedef int        INT;
typedef unsigned int  UINT;

/* These types must be 8-bit integer */
typedef char      CHAR;
typedef unsigned char UCHAR;
//typedef unsigned char BYTE; //Arduino does not like this

/* These types must be 16-bit integer */
typedef short     SHORT;
typedef unsigned short  USHORT;
typedef unsigned short  WORD;
typedef unsigned short  WCHAR;

/* These types must be 32-bit integer */
typedef int      LONG;
typedef unsigned int ULONG;
typedef unsigned int DWORD;

/* Status of Disk Functions */
typedef unsigned short  DSTATUS;

/* Results of Disk Functions */
typedef enum {
  RES_OK = 0,   /* 0: Successful */
  RES_ERROR,    /* 1: R/W Error */
  RES_WRPRT,    /* 2: Write Protected */
  RES_NOTRDY,   /* 3: Not Ready */
  RES_PARERR,   /* 4: Invalid Parameter */
  RES_NONRSPNS            /* 5: No Response */
}DRESULT;

/* Disk Status Bits (DSTATUS) */

#define STA_NOINIT    0x01  /* Drive not initialized */
#define STA_NODISK    0x02  /* No medium in the drive */
#define STA_PROTECT   0x04  /* Write protected */

#endif

/******************************************************************************
* Constants
******************************************************************************/

/* SDHC commands */
#define SDHC_CMD0                           (0)
#define SDHC_CMD1                           (1)
#define SDHC_CMD2                           (2)
#define SDHC_CMD3                           (3)
#define SDHC_CMD4                           (4)
#define SDHC_CMD5                           (5)
#define SDHC_CMD6                           (6)
#define SDHC_CMD7                           (7)
#define SDHC_CMD8                           (8)
#define SDHC_CMD9                           (9)
#define SDHC_CMD10                          (10)
#define SDHC_CMD11                          (11)
#define SDHC_CMD12                          (12)
#define SDHC_CMD13                          (13)
#define SDHC_CMD15                          (15)
#define SDHC_CMD16                          (16)
#define SDHC_CMD17                          (17)
#define SDHC_CMD18                          (18)
#define SDHC_CMD20                          (20)
#define SDHC_CMD24                          (24)
#define SDHC_CMD25                          (25)
#define SDHC_CMD26                          (26)
#define SDHC_CMD27                          (27)
#define SDHC_CMD28                          (28)
#define SDHC_CMD29                          (29)
#define SDHC_CMD30                          (30)
#define SDHC_CMD32                          (32)
#define SDHC_CMD33                          (33)
#define SDHC_CMD34                          (34)
#define SDHC_CMD35                          (35)
#define SDHC_CMD36                          (36)
#define SDHC_CMD37                          (37)
#define SDHC_CMD38                          (38)
#define SDHC_CMD39                          (39)
#define SDHC_CMD40                          (40)
#define SDHC_CMD42                          (42)
#define SDHC_CMD52                          (52)
#define SDHC_CMD53                          (53)
#define SDHC_CMD55                          (55)
#define SDHC_CMD56                          (56)
#define SDHC_CMD60                          (60)
#define SDHC_CMD61                          (61)
#define SDHC_ACMD6                          (0x40 + 6)
#define SDHC_ACMD13                         (0x40 + 13)
#define SDHC_ACMD22                         (0x40 + 22)
#define SDHC_ACMD23                         (0x40 + 23)
#define SDHC_ACMD41                         (0x40 + 41)
#define SDHC_ACMD42                         (0x40 + 42)
#define SDHC_ACMD51                         (0x40 + 51)


/* ESDHC command types */
#define SDHC_TYPE_NORMAL                    (0x00)
#define SDHC_TYPE_SUSPEND                   (0x01)
#define SDHC_TYPE_RESUME                    (0x02)
#define SDHC_TYPE_ABORT                     (0x03)
#define SDHC_TYPE_SWITCH_BUSY               (0x04)

#define SDHC_TRANSFERTYPE_DMA               1
#define SDHC_TRANSFERTYPE_SWPOLL            2

#define SDHC_TRANSFERTYPE                   SDHC_TRANSFERTYPE_DMA
#define SDHC_FIFO_BUFFER_SIZE               16
#define SDHC_BLOCK_SIZE                     512

#define SDHC_DO4BITS                        1
#ifndef SDHC_USE_ISR
	#define SDHC_USE_ISR                    0
#endif

/******************************************************************************
* Macros 
******************************************************************************/


/******************************************************************************
* Types
******************************************************************************/
typedef struct
{
  DSTATUS status;
  LWord   address;
  Byte    highCapacity;  
  Byte    version2;
  LWord   numBlocks;
  LWord   lastCardStatus;
}SD_CARD_DESCRIPTOR;

/******************************************************************************
* Global variables
******************************************************************************/

/******************************************************************************
* Global functions
******************************************************************************/
#ifdef __cplusplus
extern "C"{
#endif

DSTATUS SDHC_InitCard(uint32_t kbaudrate);
DSTATUS SDHC_GetStatus(void);
LWord SDHC_GetBlockCnt(void);

uint32_t SDHC_Baudrate(void);
uint32_t SDHC_GetDMAStatus(void);
void SDHC_ClearDMAStatus(void);
//
DRESULT SDHC_ReadBlocks(UCHAR* buff, DWORD sector, UCHAR count);
DRESULT SDHC_WriteBlocks(UCHAR* buff, DWORD sector, UCHAR count);

#ifdef __cplusplus
}
#endif

#endif
