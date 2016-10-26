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
//sdcard_prv.h
#ifndef _SDCARD_PRV_H
#define _SDCARD_PRV_H
/******************************************************************************
* Includes
******************************************************************************/

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


/******************************************************************************
* Macros 
******************************************************************************/


/******************************************************************************
* Types
******************************************************************************/


/******************************************************************************
* Private variables
******************************************************************************/
uint32_t m_usd_error=0;
uint32_t m_usd_status=0;

/******************************************************************************
* Private functions
******************************************************************************/


#endif
