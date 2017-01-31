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
//mk.h
// contains bare metal definitions (similar to kinetis.h) to allow programmatic changes

#ifndef SPICONFIG_H
#define SPICONFIG_H

/******************************************* SPI port configurations **********************************************/
/**
 * The following SPI lines are available on Teensy
 * [Teensy digital pin, Kinetis native pin, alt function to activate spi]
 *
 * 	sck0	[13, C5, 2]   [14, D1,  2]  [27,        A15, 2]
 * 	mosi0	[7,  D2, 2]   [11, C6,  2]  [28,        A16, 2]
 * 	miso0	[8,  D3, 2]   [12, C7,  2]  [39,        A17, 2]
 *
 * 	sck1	[32, B11, 2]  [20, D5,  7]  [sdhc_clk,  E2,  2]
 * 	mosi1	[0,  B16, 2]  [21, D6,  7]  [sdhc_dat0, E1,  2] [sdhc_cmd , E3,  7]
 * 	miso1	[1,  B17, 2]  [5,  D7,  7]  [sdhc_cmd,  E3,  2] [sdhc_dat0, E1,  7]
 *
 * 	sck2	[46, B21,2]   [53, D12, 2]
 * 	mosi2	[44, B22,2]   [52, D13, 2]
 * 	miso2	[45, B23,2]   [51, D14, 2]
 */

/**
 * to use sdhc ports for spi
 *
 *  T3.x - uSD - sdhc - spi
 *  		1	dat3	CS
 *  		2	cmd		MOSI (MISO)
 *  		5	clk		CLK
 *  		7	dat0	MISO (MOSI)
 *  		8	dat1
 *  		9	dat2
 *
 * There are two possibilities using MISO/MOSI on SDHC port
 *
 * NOTE SDHC based SPI1_CS ONLY sdhc_dat3 = port PTE4
 */

typedef enum {PA=0, PB=1, PC=2, PD=3, PE=4} kPort_t;

// some fake teensy port number to simulate sdhc pins
#define sdhcCLK 61		// for clock
#define sdhcDAT0 62		// for mosi
#define sdhcCMD 63		// for miso

typedef struct
{	uint16_t Tpin;	// teensy digital port number (for comparison)
	kPort_t port;	// Kinetis port id
	uint16_t Kpin;	// Kinetis port number
	uint16_t mux;	// pin multiplxer (alt number)
}
Tpin_t;

#define SPI0_CLK_13 	(Tpin_t){13,	PC, 5,2}
#define SPI0_CLK_14 	(Tpin_t){14,	PD, 1,2}
#define SPI0_CLK_27 	(Tpin_t){27,	PA,15,2}
#define SPI1_CLK_20 	(Tpin_t){20,	PD, 5,7}
#define SPI1_CLK_32 	(Tpin_t){32,	PB,11,2}
#define SPI1_CLK_sdhc 	(Tpin_t){sdhcCLK,	PE,2,2}
#define SPI2_CLK_46 	(Tpin_t){46,	PB,21,2}
#define SPI2_CLK_53		(Tpin_t){53,	PD,12,2}

#define SPI0_MOSI_7 	(Tpin_t){7, PD, 2,2}
#define SPI0_MOSI_11 	(Tpin_t){11,PC, 6,2}
#define SPI0_MOSI_28 	(Tpin_t){28,PA,16,2}
#define SPI1_MOSI_0 	(Tpin_t){0, PB,16,2}
#define SPI1_MOSI_21 	(Tpin_t){21,PD, 6,7}
#define SPI1_MOSI_sdhc 	(Tpin_t){sdhcDAT0, PE,1,2}
#define SPI2_MOSI_44 	(Tpin_t){44,PB,22,2}
#define SPI2_MOSI_52 	(Tpin_t){52,PD,13,2}

#define SPI0_MISO_8 	(Tpin_t){8, PD, 3,2}
#define SPI0_MISO_12 	(Tpin_t){12,PC, 7,2}
#define SPI0_MISO_39 	(Tpin_t){39,PA,17,2}
#define SPI1_MISO_1 	(Tpin_t){1, PB,17,2}
#define SPI1_MISO_5 	(Tpin_t){5, PD, 7,7}
#define SPI1_MISO_sdhc 	(Tpin_t){sdhcCMD,PE,3,2}
#define SPI2_MISO_45 	(Tpin_t){45,PB,23,2}
#define SPI2_MISO_51 	(Tpin_t){51,PD,14,2}

#define CS_sdhc (Tpin_t){99, PE, 4,1}
#define CS_0 	(Tpin_t){0,  PB,16,1}
#define CS_1 	(Tpin_t){1,  PB,17,1}
#define CS_2 	(Tpin_t){2,  PD, 0,1}
#define CS_3 	(Tpin_t){3,  PA,12,1}
#define CS_4 	(Tpin_t){4,  PA,13,1}
#define CS_5 	(Tpin_t){5,  PD, 7,1}
#define CS_6 	(Tpin_t){6,  PD, 4,1}
#define CS_7 	(Tpin_t){7,  PD, 2,1}
#define CS_8 	(Tpin_t){8,  PD, 3,1}
#define CS_9 	(Tpin_t){9,  PC, 3,1}
#define CS_10 	(Tpin_t){10, PC, 4,1}
#define CS_11 	(Tpin_t){11, PC, 6,1}
#define CS_12 	(Tpin_t){12, PC, 7,1}

#define CS_13 	(Tpin_t){13, PC, 5,1}
#define CS_14 	(Tpin_t){14, PD, 1,1}
#define CS_15 	(Tpin_t){15, PC, 0,1}
#define CS_16 	(Tpin_t){16, PB, 0,1}
#define CS_17 	(Tpin_t){17, PB, 1,1}
#define CS_18 	(Tpin_t){18, PB, 3,1}
#define CS_19 	(Tpin_t){19, PB, 2,1}
#define CS_20 	(Tpin_t){20, PD, 5,1}
#define CS_21 	(Tpin_t){21, PD, 6,1}
#define CS_22 	(Tpin_t){22, PC, 1,1}
#define CS_23 	(Tpin_t){23, PC, 2,1}

#define CS_24 	(Tpin_t){24, PE,26,1}
#define CS_25 	(Tpin_t){25, PA, 5,1}
#define CS_26 	(Tpin_t){26, PA,14,1}
#define CS_27 	(Tpin_t){27, PA,15,1}
#define CS_28 	(Tpin_t){28, PA,16,1}
#define CS_29 	(Tpin_t){29, PB,18,1}
#define CS_30 	(Tpin_t){30, PB,19,1}
#define CS_31 	(Tpin_t){31, PB,10,1}
#define CS_32 	(Tpin_t){32, PB,11,1}

#define CS_33 	(Tpin_t){33, PE,24,1}
#define CS_34 	(Tpin_t){34, PE,25,1}
#define CS_35 	(Tpin_t){35, PC, 8,1}
#define CS_36 	(Tpin_t){36, PC, 9,1}
#define CS_37 	(Tpin_t){37, PC,10,1}
#define CS_38 	(Tpin_t){38, PC,11,1}
#define CS_39 	(Tpin_t){39, PA,17,1}

#define CS_40 	(Tpin_t){40, PD,15,1}
#define CS_41 	(Tpin_t){41, PD,11,1}
#define CS_42 	(Tpin_t){42, PE,10,1}
#define CS_43 	(Tpin_t){43, PE,11,1}

#define CS_44 	(Tpin_t){44, PB,20,1}
#define CS_45 	(Tpin_t){45, PB,22,1}
#define CS_46 	(Tpin_t){46, PB,23,1}
#define CS_47 	(Tpin_t){47, PB,21,1}

#define CS_48 	(Tpin_t){48, PD, 8,1}
#define CS_49 	(Tpin_t){49, PD, 9,1}
#define CS_50 	(Tpin_t){50, PB, 4,1}
#define CS_51 	(Tpin_t){51, PB, 5,1}

#define CS_52 	(Tpin_t){52, PD,14,1}
#define CS_53 	(Tpin_t){53, PD,13,1}
#define CS_54 	(Tpin_t){54, PD,12,1}
#define CS_55 	(Tpin_t){55, PD,11,1}
#define CS_56 	(Tpin_t){56, PE,10,1}
#define CS_57 	(Tpin_t){57, PE,11,1}



#endif
