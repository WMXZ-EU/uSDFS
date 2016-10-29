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
//uSDconfig.h
#ifndef USDCONFIG_H
#define USDCONFIG_H

#include "spiconfig.h"

// some specific types
typedef enum {uSDsdhc, uSDspi} diskDev_t;

typedef struct
{
	uint16_t port;
	diskDev_t dev;
	Tpin_t cs;
} diskIO_t;

typedef struct
{
	Tpin_t clk;
	Tpin_t mosi;
	Tpin_t miso;
}spi_port_t;


/******************* adjust to local reality ********************************/
/**
 * allow multiple physical disks
 * access disks as "0:/", "1:/",
 *
 * configure the logical uSD disks according to its index 0,1,2, etc
 * format of USD_DISKS { SPI_port (0,1,2, 99 for sdhc), IO type (uSDspi, uSDsdhc), chip select}
 * for example:
 * {
 *	{ 0, uSDspi, CS_2},
 *	{ 0, uSDspi, CS_3},
 *	{99, uSDsdhc, CS_sdhc},
 *	{ 2, uSDspi,CS_4}
 *}
 *
 *means
 *	disk 0 ("0:/") uses spi0, and CS pin # 2
 *	disk 1 ("1:/") uses spi0, and CS pin # 3
 *	disk 2 ("2:/") uses sdhc
 *	disk 3 ("3:/") uses spi2, and CS pin # 4
 *
 * in this example disk #2 used 4bit sdhc interface to uSD (T3.5/3.6 only)
 * to use the spi1 port in lieu of sdhc (say for testing)
 * use 	{ 1, uSDspi, CS_sdhc},
 */

#define USD_DISKS \
	{	{99,	uSDsdhc, 	CS_sdhc}, \
		{0,		uSDspi, 	CS_2}, \
		{0,		uSDspi, 	CS_10} \
	}	// is of type diskIO_t

/**
 * configure the hw spi ports
 * this reflects the actual wiring
 * to use SPI1 in lieu of sdhc (say for testing)
 * use 	{ SPI1_CLK_sdhc, 	SPI1_MOSI_sdhc,		SPI1_MISO_sdhc}
 * symbols are defined in "spiconfig.h"
 */

#define  SPI_HW_PORTS \
{	\
		{ SPI0_CLK_13, 		SPI0_MOSI_11, 		SPI0_MISO_12}, \
		{ SPI1_CLK_sdhc, 	SPI1_MOSI_sdhc,		SPI1_MISO_sdhc},	\
		{ SPI2_CLK_46, 		SPI2_MOSI_44, 		SPI2_MISO_45}	\
}	// is of type spi_port_t


/**
 * defines if to use 8 or 16 bit for spi transfers
 * set to one for using 16-bit and to 0 for using 8-bit transfers
 */
#define SPI_USE_16BITS 0

/**
 * defines desired clock rate in kHz for SPI operations
 */
#define SPI_CLOCK_KHZ 24000

/**
 * switch for info uSDif.cpp info printout
 */
#define USD_IF_INFO 1

/*********************************************************************/
/*
 *  test SPI_setup
 *
 *  Teensy      SD card     Signal
 *  (label)
 *  ------    -------   ---------------------------
 *     2       1    chip select (PD0 on Teensy)
 *    11       2    SD data in (PC6 on Teensy)
 *   GND     3 & 6    Vss
 *  3.3V       4    Vdd (be sure to use 3.3 V!
 *    13       5    SCLK (PC5 on Teensy)
 *    12       7    SD data out (PC7 on Teensy)
 *
 */
#endif
