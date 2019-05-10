/*
 * WMXZ Teensy uSDFS library
 * Copyright (c) 2016 Walter Zimmer.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef _SD_SDHC_H_
#define _SD_SDHC_H_

#include "../diskio.h"

/* ESDHC command types */
#define SDHC_TYPE_NORMAL                    (0x00)
#define SDHC_TYPE_SUSPEND                   (0x01)
#define SDHC_TYPE_RESUME                    (0x02)
#define SDHC_TYPE_ABORT                     (0x03)
#define SDHC_TYPE_SWITCH_BUSY               (0x04)

#define SDHC_TRANSFERTYPE_DMA               1
#define SDHC_TRANSFERTYPE_SWPOLL            2

#define SDHC_FIFO_BUFFER_SIZE               16
#define SDHC_BLOCK_SIZE                     512


/******************************************************************************
* Macros 
******************************************************************************/

#define SDHC_ERROR(x,y) {m_sdhc_error = y; sdCardDesc.status = x; return x;}

/******************************************************************************
* Types
******************************************************************************/
/*
// type definitions for 'original' sdhc.c 
typedef unsigned int   LWord;
typedef signed int     sLWord;
typedef unsigned short  Word;
typedef signed short    sWord;
typedef unsigned char   Byte;
typedef signed char     sByte;

typedef unsigned char UCHAR; // for legacy

typedef struct
{
  DSTATUS status;
  LWord   address;
  Byte    highCapacity;  
  Byte    version2;
  LWord   numBlocks;
  LWord   lastCardStatus;
}SD_CARD_DESCRIPTOR;
*/
/*

typedef enum {
  SD_CARD_ERROR_NONE = 0,

  // Basic commands and switch command.
  SD_CARD_ERROR_CMD0 = 0X20,
  SD_CARD_ERROR_CMD2,
  SD_CARD_ERROR_CMD3,
  SD_CARD_ERROR_CMD6,
  SD_CARD_ERROR_CMD7,
  SD_CARD_ERROR_CMD8,
  SD_CARD_ERROR_CMD9,
  SD_CARD_ERROR_CMD10,
  SD_CARD_ERROR_CMD12,
  SD_CARD_ERROR_CMD13,
  SD_CARD_ERROR_CMD16,

  // Read, write, erase, and extension commands.
  SD_CARD_ERROR_CMD17 = 0X30,
  SD_CARD_ERROR_CMD18,
  SD_CARD_ERROR_CMD24,
  SD_CARD_ERROR_CMD25,
  SD_CARD_ERROR_CMD32,
  SD_CARD_ERROR_CMD33,
  SD_CARD_ERROR_CMD38,
  SD_CARD_ERROR_CMD58,
  SD_CARD_ERROR_CMD59,

  // Application specific commands.
  SD_CARD_ERROR_ACMD6 = 0X40,
  SD_CARD_ERROR_ACMD13,
  SD_CARD_ERROR_ACMD23,
  SD_CARD_ERROR_ACMD41,

  // Misc errors.
  SD_CARD_ERROR_DMA = 0X50,
  SD_CARD_ERROR_ERASE,
  SD_CARD_ERROR_ERASE_SINGLE_BLOCK,
  SD_CARD_ERROR_ERASE_TIMEOUT,
  SD_CARD_ERROR_INIT_NOT_CALLED,
  SD_CARD_ERROR_READ,
  SD_CARD_ERROR_READ_REG,
  SD_CARD_ERROR_READ_TIMEOUT,
  SD_CARD_ERROR_STOP_TRAN,
  SD_CARD_ERROR_WRITE_TIMEOUT,
  SD_CARD_ERROR_WRITE,
} sd_error_code_t;
*/
/******************************************************************************
* Global variables
******************************************************************************/


/******************************************************************************
* Global functions
******************************************************************************/
#ifdef __cplusplus
extern "C"{
#endif
/*
DSTATUS SDHC_InitCard(void);
uint16_t SDHC_DMADone(void);
void SDHC_DMAWait(void);

DSTATUS SDHC_GetStatus(void);
LWord SDHC_GetBlockCnt(void);

uint32_t SDHC_Baudrate(void);
//uint32_t SDHC_GetDMAStatus(void);
void SDHC_ClearDMAStatus(void);
//
DRESULT SDHC_ReadBlocks(UCHAR* buff, DWORD sector, UCHAR count);
DRESULT SDHC_WriteBlocks(UCHAR* buff, DWORD sector, UCHAR count);
//
DSTATUS SDHC_CardIsReady(void);
*/

DSTATUS SDHC_disk_status();
DSTATUS SDHC_disk_initialize();
DRESULT SDHC_disk_read(BYTE *buff, DWORD sector, UINT count);
DRESULT SDHC_disk_write(const BYTE *buff, DWORD sector, UINT count);
DRESULT SDHC_disk_ioctl(BYTE cmd, BYTE *buff);

#ifdef __cplusplus
}
#endif



#endif
