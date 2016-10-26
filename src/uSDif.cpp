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
//uSDif.cpp

/************************ for SPI interface *********************/
/*
 * from Karl Lundt's fftest.c (bare-metal no *duino)
 * slightly modified to all dynamic access
 * did not use Teensy high level macros (e.g. digitalWrite) to see better "bare-metal"
 */
#include  <stdint.h>

#include "kinetis.h"
#include "core_pins.h"
#include "usb_serial.h"

#include "mk.h"
#include "uSDconfig.h"
#include "uSDif.h"

#include "sdcard.h"
#include "sdspi.h"

extern "C" void logg(char c) { Serial.printf("%c",c); Serial.flush();}

//
uint32_t kPorts[] = PORT_BASE_PTRS;

void configPort(Tpin_t tpin)
{
  uint32_t kpin = kPorts[tpin.port]+4*tpin.Kpin;
  uint32_t kmux = PORT_PCR_MUX(tpin.mux);
  *(uint32_t *)kpin  = kmux;
}

diskIO_t uSDdisks[] = USD_DISKS;
spi_port_t spi_hw_ports[] = SPI_HW_PORTS;

static void cs_init(uint16_t dev);
static void chip_select(void);
static void chip_deselect(void);
static char xchg(char  c);
static void xferBulk(char  *out, char *inp, uint16_t nbytes);

#if SPI_USE_16BITS == 1
	#define SPI_BITS 16
#else
	#define SPI_BITS 8
#endif

void sdspi_setup(uint16_t dev)
{
  uint32_t        sckfreqkhz;
  uint32_t        finalclk;

  uint16_t iodev = uSDdisks[dev].port;
  spi_port_t *spihw = &spi_hw_ports[iodev];

  configPort(spihw->clk);
  configPort(spihw->mosi);
  configPort(spihw->miso);

  sdspi_select(dev);
  cs_init(dev);

  sckfreqkhz = 400;       // use slow initial SPI clock
  finalclk = SPIInit(iodev, sckfreqkhz, SPI_BITS);

#if USE_IF_INFO == 1
  Serial.printf("\n\rSPI connected at %d kHz.", finalclk);
#endif

  SDRegisterSPI(chip_select, xchg, xferBulk, chip_deselect);  // register our SPI functions with the SD library
  SDInit();           // now init the SD interface and card

  sckfreqkhz = SPI_CLOCK_KHZ;       // switch to fast SPI clock
  finalclk = SPIInit(iodev, sckfreqkhz, SPI_BITS);
#if USE_IF_INFO ==1
  Serial.printf("\n\rSPI connected at %d kHz.\n\r", finalclk);
#endif

}

static uint16_t m_dev;
void sdspi_select(uint16_t dev)
{
	m_dev=dev;

}

/******************************************************************/
GPIO_MemMapPtr kGPIOs[] = GPIO_BASE_PTRS;

//#define   CS_HIGH   GPIOD_PSOR=(1<<0)
//#define   CS_LOW    GPIOD_PCOR=(1<<0)

/*
 *  cs_init      initialize the chip (board) select pin
 */
static void cs_init(uint16_t dev)
{
	//  PORTD_PCR0 = PORT_PCR_MUX(0x1); // CS is on PD0 (pin 2), config as GPIO (alt = 1)
	//  GPIOD_PDDR = (1<<0);      // make this an output pin

	Tpin_t cs = uSDdisks[dev].cs;
	configPort(cs);
	uint32_t addr = (uint32_t) &(kGPIOs[cs.port]->PDDR);
	*((uint32_t *)(addr)) = (1<<cs.Kpin);

	m_dev = dev;
	chip_deselect();    // and deselect the SD card
}

/*
 *  chip_select      select (enable) the SD card
 */
static  void  chip_select(void)
{ 	// CS_LOW;
	Tpin_t cs = uSDdisks[m_dev].cs;
	uint32_t addr = (uint32_t) &(kGPIOs[cs.port]->PCOR);
	*((uint32_t *)(addr)) = (1<<cs.Kpin);
}

/*
 *  chip_deselect      deselect (disable) the SD card.
 */
static  void  chip_deselect(void)
{ 	// CS_HIGH;
	Tpin_t cs = uSDdisks[m_dev].cs;
	uint32_t addr = (uint32_t) &(kGPIOs[cs.port]->PSOR);
	*((uint32_t *)(addr)) = (1<<cs.Kpin);
}

/*
 *  xchg      exchange a byte of data with the SD card via host's SPI bus
 */
static  char  xchg(char  c)
{
	return  SPIExchange(m_dev,c);
}


static void xferBulk(char  *dst, char *src, uint16_t nbytes)
{ 	/** default bulk transfer */
/*	uint16_t ii;
	if(dst && !scr)
		for(ii=0;ii<nbytes;ii++) dst[ii] = xchg(0xff);		// read from SPI
	if(!dst && src)
		for(ii=0;ii<nbytes;ii++) xchg(src[ii]);				// write to SPI
	if(dst && src)
		for(ii=0;ii<nbytes;ii++) dst[ii] = xchg(src[ii]);	// write and read from SPI
*/
	/** fifo based bulk transfer */
#if SPI_USE_16BITS == 1
	if(!SPIExchangeBlock16(m_dev, dst, src, nbytes)) Serial.println("Error: 16-bit bulk transfer");
#else
	SPIExchangeBlock(m_dev, dst, src, nbytes);
#endif
}
