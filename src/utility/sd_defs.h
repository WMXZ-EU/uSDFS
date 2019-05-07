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
//sd_defs.h
#ifndef _SD_DEFS_H
#define _SD_DEFS_H
/******************************************************************************
* Includes
******************************************************************************/
#include "../diskio.h"

/******************************************************************************
* Constants
******************************************************************************/

/** init timeout ms */
#define  SD_INIT_TIMEOUT  (2000)
/** erase timeout ms */
#define  SD_ERASE_TIMEOUT (10000)
/** read timeout ms */
#define  SD_READ_TIMEOUT (300)
/** write time out ms */
#define  SD_WRITE_TIMEOUT (600)

//------------------------------------------------------------------------------
/** status for card in the ready state */
#define R1_READY_STATE (0X00)
/** status for card in the idle state */
#define R1_IDLE_STATE (0X01)
/** status bit for illegal command */
#define R1_ILLEGAL_COMMAND (0X04)

/** start data token for read or write single block*/
#define DATA_START_BLOCK (0XFE)
/** stop token for write multiple blocks*/
#define STOP_TRAN_TOKEN (0XFD)
/** start data token for write multiple blocks*/
#define WRITE_MULTIPLE_TOKEN (0XFC)

/** mask for data response tokens after a write block operation */
#define DATA_RES_MASK (0X1F)
/** write data accepted token */
#define DATA_RES_ACCEPTED (0X05)

//------------------------------------------------------------------------------
// SD card errors
/** timeout error for command CMD0 (initialize card in SPI mode) */
#define  SD_CARD_ERROR_CMD0 (0X1)
/** CMD8 was not accepted - not a valid SD card*/
#define  SD_CARD_ERROR_CMD8 (0X2)
/** card returned an error response for CMD12 (stop multiblock read) */
#define  SD_CARD_ERROR_CMD12 (0X3)
/** card returned an error response for CMD17 (read block) */
#define  SD_CARD_ERROR_CMD17 (0X4)
/** card returned an error response for CMD18 (read multiple block) */
#define  SD_CARD_ERROR_CMD18 (0X5)
/** card returned an error response for CMD24 (write block) */
#define  SD_CARD_ERROR_CMD24 (0X6)
/**  WRITE_MULTIPLE_BLOCKS command failed */
#define  SD_CARD_ERROR_CMD25 (0X7)
/** card returned an error response for CMD58 (read OCR) */
#define  SD_CARD_ERROR_CMD58 (0X8)
/** SET_WR_BLK_ERASE_COUNT failed */
#define  SD_CARD_ERROR_ACMD23 (0X9)
/** ACMD41 initialization process timeout */
#define  SD_CARD_ERROR_ACMD41 (0XA)
/** card returned a bad CSR version field */
#define  SD_CARD_ERROR_BAD_CSD (0XB)
/** erase block group command failed */
#define  SD_CARD_ERROR_ERASE (0XC)
/** card not capable of single block erase */
#define  SD_CARD_ERROR_ERASE_SINGLE_BLOCK (0XD)
/** Erase sequence timed out */
#define  SD_CARD_ERROR_ERASE_TIMEOUT (0XE)
/** card returned an error token instead of read data */
#define  SD_CARD_ERROR_READ (0XF)
/** read CID or CSD failed */
#define  SD_CARD_ERROR_READ_REG (0X10)
/** timeout while waiting for start of read data */
#define  SD_CARD_ERROR_READ_TIMEOUT (0X11)
/** card did not accept STOP_TRAN_TOKEN */
#define  SD_CARD_ERROR_STOP_TRAN (0X12)
/** card returned an error token as a response to a write operation */
#define  SD_CARD_ERROR_WRITE (0X13)
/** attempt to write protected block zero */
#define  SD_CARD_ERROR_WRITE_BLOCK_ZERO (0X14)  // REMOVE - not used
/** card did not go ready for a multiple block write */
#define  SD_CARD_ERROR_WRITE_MULTIPLE (0X15)
/** card returned an error to a CMD13 status check after a write */
#define  SD_CARD_ERROR_WRITE_PROGRAMMING (0X16)
/** timeout occurred during write programming */
#define  SD_CARD_ERROR_WRITE_TIMEOUT (0X17)
/** incorrect rate selected */
#define  SD_CARD_ERROR_SCK_RATE (0X18)
/** init() not called */
#define  SD_CARD_ERROR_INIT_NOT_CALLED (0X19)
/** card returned an error for CMD59 (CRC_ON_OFF) */
#define  SD_CARD_ERROR_CMD59 (0X1A)
/** invalid read CRC */
#define  SD_CARD_ERROR_READ_CRC (0X1B)
/** SPI DMA error */
#define  SD_CARD_ERROR_SPI_DMA (0X1C)

//
#define  SD_CARD_ERROR_CMD2 (0X22)
//
#define  SD_CARD_ERROR_CMD3 (0X23)
//
#define  SD_CARD_ERROR_CMD6 (0X26)
//
#define  SD_CARD_ERROR_CMD7 (0X27)
//
#define  SD_CARD_ERROR_CMD9 (0X29)
//
#define  SD_CARD_ERROR_CMD10 (0X2a)
//
#define  SD_CARD_ERROR_CMD16 (0X30)
//
#define  SD_CARD_ERROR_ACMD6 (0x36)
//------------------------------------------------------------------------------
// SD card commands
/** GO_IDLE_STATE - init card in spi mode if CS low */
#define CMD0 (0X00)
/** SEND_IF_COND - verify SD Memory Card interface operating condition.*/
#define CMD8 (0X08)
/** SEND_CSD - read the Card Specific Data (CSD register) */
#define CMD9 (0X09)
/** SEND_CID - read the card identification information (CID register) */
#define CMD10 (0X0A)
/** STOP_TRANSMISSION - end multiple block read sequence */
#define CMD12 (0X0C)
/** SEND_STATUS - read the card status register */
#define CMD13 (0X0D)
/** READ_SINGLE_BLOCK - read a single data block from the card */
#define CMD17 (0X11)
/** READ_MULTIPLE_BLOCK - read a multiple data blocks from the card */
#define CMD18 (0X12)
/** WRITE_BLOCK - write a single data block to the card */
#define CMD24 (0X18)
/** WRITE_MULTIPLE_BLOCK - write blocks of data until a STOP_TRANSMISSION */
#define CMD25 (0X19)
/** ERASE_WR_BLK_START - sets the address of the first block to be erased */
#define CMD32 (0X20)
/** ERASE_WR_BLK_END - sets the address of the last block of the continuous
    range to be erased*/
#define CMD33 (0X21)
/** ERASE - erase all previously selected blocks */
#define CMD38 (0X26)
/** APP_CMD - escape for application specific command */
#define CMD55 (0X37)
/** READ_OCR - read the OCR register of a card */
#define CMD58 (0X3A)
/** CRC_ON_OFF - enable or disable CRC checking */
#define CMD59 (0X3B)
/** SET_WR_BLK_ERASE_COUNT - Set the number of write blocks to be
     pre-erased before writing */
#define ACMD23 (0X17)
/** SD_SEND_OP_COMD - Sends host capacity support information and
    activates the card's initialization process */
#define ACMD41 (0X29)

//------------------------------------------------------------------------------
// card types
/** Standard capacity V1 SD card */
#define  SD_CARD_TYPE_SD1  (1)
/** Standard capacity V2 SD card */
#define  SD_CARD_TYPE_SD2  (2)
/** High Capacity SD card */
#define  SD_CARD_TYPE_SDHC (3)

/******************************************************************************
* Macros 
******************************************************************************/


/******************************************************************************
* Types
******************************************************************************/
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

/** Card IDentification (CID) register */
typedef struct CID {
  // byte 0
  /** Manufacturer ID */
  unsigned char mid;
  // byte 1-2
  /** OEM/Application ID */
  char oid[2];
  // byte 3-7
  /** Product name */
  char pnm[5];
  // byte 8
  /** Product revision least significant digit */
  unsigned char prv_m : 4;
  /** Product revision most significant digit */
  unsigned char prv_n : 4;
  // byte 9-12
  /** Product serial number */
  uint32_t psn;
  // byte 13
  /** Manufacturing date year low digit */
  unsigned char mdt_year_high : 4;
  /** not used */
  unsigned char reserved : 4;
  // byte 14
  /** Manufacturing date month */
  unsigned char mdt_month : 4;
  /** Manufacturing date year low digit */
  unsigned char mdt_year_low :4;
  // byte 15
  /** not used always 1 */
  unsigned char always1 : 1;
  /** CRC7 checksum */
  unsigned char crc : 7;
}__attribute__((packed)) cid_t;
//------------------------------------------------------------------------------
/** CSD for version 1.00 cards */
typedef struct CSDV1 {
  // byte 0
  unsigned char reserved1 : 6;
  unsigned char csd_ver : 2;
  // byte 1
  unsigned char taac;
  // byte 2
  unsigned char nsac;
  // byte 3
  unsigned char tran_speed;
  // byte 4
  unsigned char ccc_high;
  // byte 5
  unsigned char read_bl_len : 4;
  unsigned char ccc_low : 4;
  // byte 6
  unsigned char c_size_high : 2;
  unsigned char reserved2 : 2;
  unsigned char dsr_imp : 1;
  unsigned char read_blk_misalign :1;
  unsigned char write_blk_misalign : 1;
  unsigned char read_bl_partial : 1;
  // byte 7
  unsigned char c_size_mid;
  // byte 8
  unsigned char vdd_r_curr_max : 3;
  unsigned char vdd_r_curr_min : 3;
  unsigned char c_size_low :2;
  // byte 9
  unsigned char c_size_mult_high : 2;
  unsigned char vdd_w_cur_max : 3;
  unsigned char vdd_w_curr_min : 3;
  // byte 10
  unsigned char sector_size_high : 6;
  unsigned char erase_blk_en : 1;
  unsigned char c_size_mult_low : 1;
  // byte 11
  unsigned char wp_grp_size : 7;
  unsigned char sector_size_low : 1;
  // byte 12
  unsigned char write_bl_len_high : 2;
  unsigned char r2w_factor : 3;
  unsigned char reserved3 : 2;
  unsigned char wp_grp_enable : 1;
  // byte 13
  unsigned char reserved4 : 5;
  unsigned char write_partial : 1;
  unsigned char write_bl_len_low : 2;
  // byte 14
  unsigned char reserved5: 2;
  unsigned char file_format : 2;
  unsigned char tmp_write_protect : 1;
  unsigned char perm_write_protect : 1;
  unsigned char copy : 1;
  /** Indicates the file format on the card */
  unsigned char file_format_grp : 1;
  // byte 15
  unsigned char always1 : 1;
  unsigned char crc : 7;
}__attribute__((packed)) csd1_t;
//------------------------------------------------------------------------------
/** CSD for version 2.00 cards */
typedef struct CSDV2 {
  // byte 0
  unsigned char reserved1 : 6;
  unsigned char csd_ver : 2;
  // byte 1
  /** fixed to 0X0E */
  unsigned char taac;
  // byte 2
  /** fixed to 0 */
  unsigned char nsac;
  // byte 3
  unsigned char tran_speed;
  // byte 4
  unsigned char ccc_high;
  // byte 5
  /** This field is fixed to 9h, which indicates READ_BL_LEN=512 Byte */
  unsigned char read_bl_len : 4;
  unsigned char ccc_low : 4;
  // byte 6
  /** not used */
  unsigned char reserved2 : 4;
  unsigned char dsr_imp : 1;
  /** fixed to 0 */
  unsigned char read_blk_misalign :1;
  /** fixed to 0 */
  unsigned char write_blk_misalign : 1;
  /** fixed to 0 - no partial read */
  unsigned char read_bl_partial : 1;
  // byte 7
  /** high part of card size */
  unsigned char c_size_high : 6;
  /** not used */
  unsigned char reserved3 : 2;
  // byte 8
  /** middle part of card size */
  unsigned char c_size_mid;
  // byte 9
  /** low part of card size */
  unsigned char c_size_low;
  // byte 10
  /** sector size is fixed at 64 KB */
  unsigned char sector_size_high : 6;
  /** fixed to 1 - erase single is supported */
  unsigned char erase_blk_en : 1;
  /** not used */
  unsigned char reserved4 : 1;
  // byte 11
  unsigned char wp_grp_size : 7;
  /** sector size is fixed at 64 KB */
  unsigned char sector_size_low : 1;
  // byte 12
  /** write_bl_len fixed for 512 byte blocks */
  unsigned char write_bl_len_high : 2;
  /** fixed value of 2 */
  unsigned char r2w_factor : 3;
  /** not used */
  unsigned char reserved5 : 2;
  /** fixed value of 0 - no write protect groups */
  unsigned char wp_grp_enable : 1;
  // byte 13
  unsigned char reserved6 : 5;
  /** always zero - no partial block read*/
  unsigned char write_partial : 1;
  /** write_bl_len fixed for 512 byte blocks */
  unsigned char write_bl_len_low : 2;
  // byte 14
  unsigned char reserved7: 2;
  /** Do not use always 0 */
  unsigned char file_format : 2;
  unsigned char tmp_write_protect : 1;
  unsigned char perm_write_protect : 1;
  unsigned char copy : 1;
  /** Do not use always 0 */
  unsigned char file_format_grp : 1;
  // byte 15
  /** not used always 1 */
  unsigned char always1 : 1;
  /** checksum */
  unsigned char crc : 7;
}__attribute__((packed)) csd2_t;
//------------------------------------------------------------------------------
/** union of old and new style CSD register */
typedef union {
  csd1_t v1;
  csd2_t v2;
} csd_t ;


/******************************************************************************
* Private variables
******************************************************************************/
/******************************************************************************
* Private functions
******************************************************************************/


#endif
