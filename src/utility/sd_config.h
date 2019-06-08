/*
 * WMXZ Teensy uSDFS library
 * Copyright (c) 2016, 2019 Walter Zimmer.
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
 
#ifndef _SD_CONIG_H
#define _SD_CONIG_H

#include "ff.h"

/* Definitions of physical drive number for each drive */
#define DEV_SPI   0 /* Example: Map SPI card to physical drive 0 */
#define DEV_SDHC  1 /* Example: Map SDHC card to physical drive 1 */
#define DEV_MSC   2 /* Example: Map MSC card (USB disk) to physical drive 2 */
#define DEV_USB   2 /* Example: Map MSC card (USB disk) to physical drive 2 */

// SPI 
#define CS_PIN 10

#define DMA_TX 0
#define DMA_RX 1
#define DMA_PRIO 6

// use 16-bit frame if SPI_USE_8BIT_FRAME is zero
#define SPI_USE_8BIT_FRAME 1

#define USE_SD_CRC
#define USE_SD_CCITT 2
#define CHECK_PROGRAMMING 0

//SDIO
#define SDHC_USE_ISR	                    1	// must always use Interrupts (needed for CMD6)

#define SDHC_DO4BITS                        1	// use 4 bit bus
//#define SDHC_TRANSFERTYPE                   SDHC_TRANSFERTYPE_SWPOLL
#define SDHC_TRANSFERTYPE                   SDHC_TRANSFERTYPE_DMA 

//MSC
#define USE_MSC 1	// will be used in sd_msc.cpp


#endif
