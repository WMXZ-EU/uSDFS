/*
 *  spi.c      library of SPI support code for Teensy 3.x
 */
// taken from
// Karl Lundt
// http://www.seanet.com/~karllunt/bareteensy31libs.html
// modified and adapted to my-style spi WMXZ 16-oct-2016
//
#include  <stdio.h>
#include  <stdint.h>
#include "kinetis.h"

#include "mk.h"
#include "sdspi.h"

//SPI_MemMapPtr spiBase[] =  SPI_BASE_PTRS ;

//void logg(char c);  // for debug only

// alternative
/*
 * // is already in kinetis.k, so commented
typedef struct {
	volatile uint32_t	MCR;	// 0
	volatile uint32_t	unused1;// 4
	volatile uint32_t	TCR;	// 8
	volatile uint32_t	CTAR0;	// c
	volatile uint32_t	CTAR1;	// 10
	volatile uint32_t	CTAR2;	// 14
	volatile uint32_t	CTAR3;	// 18
	volatile uint32_t	CTAR4;	// 1c
	volatile uint32_t	CTAR5;	// 20
	volatile uint32_t	CTAR6;	// 24
	volatile uint32_t	CTAR7;	// 28
	volatile uint32_t	SR;	// 2c
	volatile uint32_t	RSER;	// 30
	volatile uint32_t	PUSHR;	// 34
	volatile uint32_t	POPR;	// 38
	volatile uint32_t	TXFR[16]; // 3c
	volatile uint32_t	RXFR[16]; // 7c
}KINETISK_SPI_t;
*/

static KINETISK_SPI_t *SPI[] = {(KINETISK_SPI_t *)0x4002C000, (KINETISK_SPI_t *)0x4002D000, (KINETISK_SPI_t *)0x400AC000};

// define some symbols that are not in mk20dx128.h
#ifndef SPI_SR_RXCTR
	#define SPI_SR_RXCTR 0XF0
#endif  // SPI_SR_RXCTR


#define SWAP(x) (((x>>8) & 0x00FF) | ((x & 0x00FF)<<8))


// following ctarTAB adapted from SPI
typedef struct {
	uint16_t div;
	uint32_t ctar;
} CTAR_TAB_t;

const CTAR_TAB_t ctarTab[23] =
{
	{2, 	SPI_CTAR_PBR(0) | SPI_CTAR_BR(0) | SPI_CTAR_CSSCK(0) | SPI_CTAR_DBR},
	{3, 	SPI_CTAR_PBR(1) | SPI_CTAR_BR(0) | SPI_CTAR_CSSCK(0) | SPI_CTAR_DBR},
	{4, 	SPI_CTAR_PBR(0) | SPI_CTAR_BR(0) | SPI_CTAR_CSSCK(0)},
	{5, 	SPI_CTAR_PBR(2) | SPI_CTAR_BR(0) | SPI_CTAR_CSSCK(0) | SPI_CTAR_DBR},
	{6, 	SPI_CTAR_PBR(1) | SPI_CTAR_BR(0) | SPI_CTAR_CSSCK(0)},
	{8, 	SPI_CTAR_PBR(0) | SPI_CTAR_BR(1) | SPI_CTAR_CSSCK(1)},
	{10,	SPI_CTAR_PBR(2) | SPI_CTAR_BR(0) | SPI_CTAR_CSSCK(0)},
	{12,	SPI_CTAR_PBR(1) | SPI_CTAR_BR(1) | SPI_CTAR_CSSCK(1)},
	{16,	SPI_CTAR_PBR(0) | SPI_CTAR_BR(3) | SPI_CTAR_CSSCK(2)},
	{20,	SPI_CTAR_PBR(2) | SPI_CTAR_BR(1) | SPI_CTAR_CSSCK(0)},
	{24,	SPI_CTAR_PBR(1) | SPI_CTAR_BR(3) | SPI_CTAR_CSSCK(2)},
	{32,	SPI_CTAR_PBR(0) | SPI_CTAR_BR(4) | SPI_CTAR_CSSCK(3)},
	{40,	SPI_CTAR_PBR(2) | SPI_CTAR_BR(3) | SPI_CTAR_CSSCK(2)},
	{56,	SPI_CTAR_PBR(3) | SPI_CTAR_BR(3) | SPI_CTAR_CSSCK(2)},
	{64,	SPI_CTAR_PBR(0) | SPI_CTAR_BR(5) | SPI_CTAR_CSSCK(4)},
	{96,	SPI_CTAR_PBR(1) | SPI_CTAR_BR(5) | SPI_CTAR_CSSCK(4)},
	{128,	SPI_CTAR_PBR(0) | SPI_CTAR_BR(6) | SPI_CTAR_CSSCK(5)},
	{192,	SPI_CTAR_PBR(1) | SPI_CTAR_BR(6) | SPI_CTAR_CSSCK(5)},
	{256,	SPI_CTAR_PBR(0) | SPI_CTAR_BR(7) | SPI_CTAR_CSSCK(6)},
	{384,	SPI_CTAR_PBR(1) | SPI_CTAR_BR(7) | SPI_CTAR_CSSCK(6)},
	{512,	SPI_CTAR_PBR(0) | SPI_CTAR_BR(8) | SPI_CTAR_CSSCK(7)},
	{640,	SPI_CTAR_PBR(2) | SPI_CTAR_BR(7) | SPI_CTAR_CSSCK(6)},
	{768,	SPI_CTAR_PBR(1) | SPI_CTAR_BR(8) | SPI_CTAR_CSSCK(7)}
} ;

uint32_t core_clk_khz = (F_BUS/1000);
/*
 *  SPIInit      initialize hardware SPI
 */
uint32_t  SPIInit(uint32_t  spinum, uint32_t  sckfreqkhz, uint32_t  numbits)
{
//	SPI_MemMapPtr	spi;
	uint16_t 		ii;
	uint32_t		ctar;
	uint32_t		rate;
	uint32_t		brscaler;
	uint32_t		final;

	if ((numbits < 4) || (numbits > 16)) return  0;	// ignore illegal transfer sizes
	if (sckfreqkhz == 0)  return -1;				// ignore illegal clock freq

	if(spinum>2) return -2;

//	spi = spiBase[spinum];
	// enable clocks
	if  (spinum == 0)
	{
		SIM_SCGC6 |= 1u << 12;		// enable clocks to SPI0
	}
	else if (spinum == 1)
	{
		SIM_SCGC6 |= 1u << 13;		// enable clocks to SPI1
	}
	else if (spinum == 2)
	{
		SIM_SCGC3 |= 1u << 12;		// enable clocks to SPI2
	}

	SPI[spinum]->MCR = SPI_MCR_MDIS | SPI_MCR_HALT;
//	SPI_MCR_REG(spi) = SPI_MCR_MDIS | SPI_MCR_HALT;	// disable and halt SPI


	rate = core_clk_khz/sckfreqkhz;
	for(ii=0; ii<23 &&  rate > ctarTab[ii].div; ii++) ;
	ctar = ctarTab[ii].ctar;
	final = core_clk_khz/ctarTab[ii].div;

/*
	ctar = 0;									// use default CTAR0 (set for 8-bit data)
	rate = core_clk_khz / 2 / sckfreqkhz;		// assumes baud prescaler of /2
	final = core_clk_khz / 2;					// assumes final freq has /2 prescaler

	if (rate < 2)								// if requested rate is too high for regular rate...
	{
		rate = rate / 2;						// scale target freq for baud-rate doubling
		final = final / 2;						// keep the final frequency calc correct
		ctar = SPI_CTAR_DBR;					// set CTAR bit for baud-rate doubling
	}

	brscaler = 1;
	while ((1<<brscaler) < rate)
	{
		brscaler++;
	}
	if (brscaler > 15)  return  -3;				// if out of range, give up; no SPI

	ctar |= brscaler;							// merge baud-rate doubler (if used) with scaler

//	ctar |= SPI_CTAR_FMSZ(numbits-1);		// add in requested frame size
//	SPI_CTAR_REG(spi, 0) = ctar;				// update attribute register CTAR0 for master mode
//	SPI_MCR_REG(spi) = SPI_MCR_MSTR;		// enable SPI0 in master mode
*/
	SPI[spinum]->CTAR0 =ctar | SPI_CTAR_FMSZ(7);
	SPI[spinum]->CTAR1 =ctar | SPI_CTAR_FMSZ(15);
	SPI[spinum]->MCR = SPI_MCR_MSTR;

//	final = final / (1 << brscaler);			// calc the actual SPI frequency
//	if(ctar &  SPI_CTAR_DBR) final <<= 1;

	return  final;								// tell the world what we got
}


/*
 *  SPIExchange      exchange one byte with selected SPI channel
 *  WMXZ ToBeDone:	speed up with fifo
 *  				use of DMA
 */
uint32_t  SPIExchange(uint32_t  spinum, uint32_t  c)
{
	if(spinum>2) return 0;

//	SPI_MemMapPtr	spi;
//	spi = spiBase[spinum];

//	SPI_SR_REG(spi) = SPI_SR_TCF;				// write 1 to the TCF flag to clear it
//	SPI_PUSHR_REG(spi) = SPI_PUSHR_TXDATA((uint16_t)c);		// write data to Tx FIFO
//	while ((SPI_SR_REG(spi) & SPI_SR_TCF) == 0)  ;	// lock until transmit complete flag goes high
//	c = SPI_POPR_REG(spi);

	SPI[spinum]->SR = SPI_SR_TCF;
	SPI[spinum]->PUSHR = SPI_PUSHR_TXDATA((uint16_t)c);
	while (!(SPI[spinum]->SR & SPI_SR_RXCTR));
	c = SPI[spinum]->POPR & 0xff;

	return  c;
}

uint32_t  SPIExchange16(uint32_t  spinum, uint32_t  c)
{
	if(spinum>2) return 0;

//	SPI_MemMapPtr	spi;
//	spi = spiBase[spinum];

//	SPI_SR_REG(spi) = SPI_SR_TCF;				// write 1 to the TCF flag to clear it
//	SPI_PUSHR_REG(spi) = SPI_PUSHR_TXDATA((uint16_t)c);		// write data to Tx FIFO
//	while ((SPI_SR_REG(spi) & SPI_SR_TCF) == 0)  ;	// lock until transmit complete flag goes high
//	c = SPI_POPR_REG(spi);

	SPI[spinum]->SR = SPI_SR_TCF;
	uint16_t w = SWAP((uint16_t)c);
	SPI[spinum]->PUSHR = SPI_PUSHR_TXDATA(w) |  SPI_PUSHR_CTAS(1) ;
	while (!(SPI[spinum]->SR & SPI_SR_RXCTR));
	w = SPI[spinum]->POPR;
	c=SWAP(w);

	return  c;
}

/****************************************TEST************************************************/
#define SPI_INITIAL_FIFO_DEPTH 3

void SPIExchangeBlock(int16_t port, void *inpbuf, void *outbuf, size_t count)
{	int ii;

		if (count == 0) return;
		uint8_t *inp=(uint8_t *)inpbuf;
		uint8_t *out=(uint8_t *)outbuf;

		SPI[port]->RSER = 0; // no interrupts
		//
	    // clear any data in RX FIFO
		SPI[port]->MCR = SPI_MCR_MSTR | SPI_MCR_CLR_RXF | SPI_MCR_PCSIS(0x1F);

	    // initial number of bytes to push into TX FIFO
	    int nf = (count < SPI_INITIAL_FIFO_DEPTH) ? count : SPI_INITIAL_FIFO_DEPTH;
	    if(port>0) nf=0; // only spi0 has 4 word fifo, others have no fifo

	    if(out && inp)
	    {
		    for (ii = 0; ii < nf; ii++) SPI[port]->PUSHR = SPI_PUSHR_CONT | *out++;

		    // limit for pushing dummy data into TX FIFO
		    uint8_t *limit = (uint8_t *)out + count - nf;
		    while (out < limit)
			{
			   while (!(SPI[port]->SR & SPI_SR_RXCTR)) {}
			   SPI[port]->PUSHR = SPI_PUSHR_CONT | *out++;
			   *inp++ = SPI[port]->POPR;
		    }
		    // limit for rest of RX data
		    limit += nf;
		    while (inp < limit)
			{
			  while (!(SPI[port]->SR & SPI_SR_RXCTR)) {}
			  *inp++ = SPI[port]->POPR;
	        }
		    return;
	    }

	    if(!out && inp)
	    {
	    	  for (ii = 0; ii < nf; ii++) SPI[port]->PUSHR = 0XFF;

	    	  // limit for pushing dummy data into TX FIFO
	    	  uint8_t* limit = (uint8_t *)inp + count - nf;
	    	  while (inp < limit) {
	    	    while (!(SPI[port]->SR & SPI_SR_RXCTR)) {}
	    	    SPI[port]->PUSHR = SPI_PUSHR_CONT | 0XFF;
	    	    *inp++ = SPI[port]->POPR;
	    	  }
	    	  // limit for rest of RX data
	    	  limit += nf;
	    	  while (inp < limit) {
	    	    while (!(SPI[port]->SR & SPI_SR_RXCTR)) {}
	    	    *inp++ = SPI[port]->POPR;
	    	  }
			    return;
	    }
	    if(out && !inp)
	    {
	    	  for (ii = 0; ii < nf; ii++) SPI[port]->PUSHR = *out++;

	    	  // limit for pushing data into TX fifo
	    	  const uint8_t* limit = (uint8_t *)out + count - nf;
	    	  // write data to TX FIFO
	    	  while (out < limit) {
	    	    while (!(SPI[port]->SR & SPI_SR_RXCTR)) {}
	    	    SPI[port]->PUSHR = *out++;
	    	    SPI[port]->POPR;
	    	  }
	    	  // wait for data to be sent
	    	  while (nf) {
	    	    while (!(SPI[port]->SR & SPI_SR_RXCTR)) {}
	    	    SPI[port]->POPR;
	    	    nf--;
	    	  }
		    return;
	    }
}


uint16_t SPIExchangeBlock16(int16_t port, void *inpbuf, void *outbuf, size_t count)
{ 	int ii;
	uint16_t *inp=(uint16_t *)inpbuf;
	uint16_t *out=(uint16_t *)outbuf;

	SPI[port]->RSER = 0; // no interrupts
	//
	// clear any data in RX FIFO
	SPI[port]->MCR = SPI_MCR_MSTR | SPI_MCR_CLR_RXF | SPI_MCR_PCSIS(0x1F);
	// use 16 bit frame to avoid TD delay between frames

	if((count & 1) || ((uint32_t)inp & (uint32_t)1) || ((uint32_t)out & (uint32_t)1) ) return 0;

	if(out && inp)
	{
		// initial number of words to push into TX FIFO
		int nf = (count/2) < SPI_INITIAL_FIFO_DEPTH ? (count/2) : SPI_INITIAL_FIFO_DEPTH;
	    if(port>0) nf=0; // only spi0 has 4 word fifo, others have no fifo

		for (ii = 0; ii < nf; ii++)
		{
		    uint16_t w = SWAP(*out); out++;
		    SPI[port]->PUSHR = SPI_PUSHR_CONT | SPI_PUSHR_CTAS(1) | w;
		}
		uint16_t* limit = inp + (count/2) - nf;
		while (inp < limit)
		{
			while (!(SPI[port]->SR & SPI_SR_RXCTR)) {}
		    uint16_t w = SWAP(*out); out++;
		    SPI[port]->PUSHR = SPI_PUSHR_CONT | SPI_PUSHR_CTAS(1) | w;
		    w = SPI[port]->POPR;
		    *inp++ = SWAP(w);
		}
		// limit for rest of RX data
		limit += nf;
		while (inp < limit)
		{
			while (!(SPI[port]->SR & SPI_SR_RXCTR)) {}
		    uint16_t w = SPI[port]->POPR;
		    *inp++ = SWAP(w);
		}
		return count;
	}
	if(!out && inp)
	{
		// initial number of words to push into TX FIFO
		int nf = (count/2) < SPI_INITIAL_FIFO_DEPTH ? (count/2) : SPI_INITIAL_FIFO_DEPTH;
	    if(port>0) nf=0; // only spi0 has 4 word fifo

		for (ii = 0; ii < nf; ii++) {
			  SPI[port]->PUSHR = SPI_PUSHR_CONT | SPI_PUSHR_CTAS(1) | 0XFFFF;
		}

		uint16_t* limit = inp + (count/2) - nf;
		while (inp < limit)
		{
		  while (!(SPI[port]->SR & SPI_SR_RXCTR)) {}
		  SPI[port]->PUSHR = SPI_PUSHR_CONT | SPI_PUSHR_CTAS(1) |  0XFFFF;
		    uint16_t w = SPI[port]->POPR;
		    *inp++ = SWAP(w);
		}
		// limit for rest of RX data
		limit += nf;
		while (inp < limit)
		{
		  while (!(SPI[port]->SR & SPI_SR_RXCTR)) {}
		    uint16_t w = SPI[port]->POPR;
		    *inp++ = SWAP(w);
		}

		return count;
	}
	if(out && ! inp)
	{
		// initial number of words to push into TX FIFO
		int nf = (count/2) < SPI_INITIAL_FIFO_DEPTH ? (count/2) : SPI_INITIAL_FIFO_DEPTH;
	    if(port>0) nf=0; // only spi0 has 4 word fifo

	    // limit for pushing data into TX fifo
		const uint16_t* limit = out + (count/2);
		for (ii = 0; ii < nf; ii++)
		{
		    uint16_t w = SWAP(*out); out++;
		    SPI[port]->PUSHR = SPI_PUSHR_CONT | SPI_PUSHR_CTAS(1) | w;
		}
		// write data to TX FIFO
		while (out < limit)
		{
			while (!(SPI[port]->SR & SPI_SR_RXCTR)) {}
		    uint16_t w = SWAP(*out); out++;
		    SPI[port]->PUSHR = SPI_PUSHR_CONT | SPI_PUSHR_CTAS(1) | w;
		    SPI[port]->POPR;
		}
		// wait for data to be sent
		while (nf)
		{
		  while (!(SPI[port]->SR & SPI_SR_RXCTR)) {}
		  SPI[port]->POPR;
		  nf--;
		}
		return count;
	}
	return 0;
}


/*********************************** BitBang (not tested)************************************************/
GPIO_MemMapPtr kGPIO[] = GPIO_BASE_PTRS;


// for the moment hard wired


// for bit banging (SPIx)
static  uint32_t			spix_nbits;
static  uint32_t			spix_mask;

//#define  SPI2_SCK_LOW		GPIOD_PCOR=(1<<1)
//#define  SPI2_SCK_HIGH		GPIOD_PSOR=(1<<1)
//#define  SPI2_MOSI_LOW		GPIOC_PCOR=(1<<0)
//#define  SPI2_MOSI_HIGH		GPIOC_PSOR=(1<<0)
//#define  SPI2_MISO_INPUT	(GPIOB_PDIR&(1<<0))


#define  SPIx_SCK_LOW		GPIO_PCOR_REG(kGPIO[3]) = (1<<1)
#define  SPIx_SCK_HIGH		GPIO_PSOR_REG(kGPIO[3]) = (1<<1)

#define  SPIx_MOSI_LOW		GPIO_PCOR_REG(kGPIO[2]) = (1<<0)
#define  SPIx_MOSI_HIGH		GPIO_PSOR_REG(kGPIO[2]) = (1<<0)

#define  SPIx_MISO_INPUT	(GPIO_PDIR_REG(kGPIO[1]) & (1<<0))

uint32_t  SPIBitBangInit(uint32_t  sckfreqkhz, uint32_t  numbits)
{
	uint32_t	final;

	spix_nbits = numbits;
	spix_mask = (1<<(numbits-1));
	final = core_clk_khz / 32;					// just a guess...

	GPIOD_PDDR |= (1<<1);					// PD1 is SCK, define as output
	PORTD_PCR1 = PORT_PCR_MUX(0x01);		// GPIO is alt1
	SPIx_SCK_LOW;							// SCK idles low (CPHA=0, CPOL=0)

	GPIOC_PDDR |= (1<<0);					// PC0 is MOSI, define as output
	PORTC_PCR0 = PORT_PCR_MUX(0x01);		// GPIO is alt1
	SPIx_MOSI_LOW;							// MOSI starts out low

	GPIOB_PDDR &= ~(1<<0);					// PB0 is MISO, define as input
	PORTB_PCR0 = PORT_PCR_MUX(0x01) | PORT_PCR_PE | PORT_PCR_PS;		// GPIO is alt1

	return final;
}

uint32_t  SPIBitBangExchange(uint32_t  c)
{
	register  uint32_t			mask;
	register  uint32_t			nbits;
	register  uint32_t			value;

	nbits = spix_nbits;			// need register version
	mask = spix_mask;			// need register version
	value = 0;					// store incoming data in register
	while (nbits)				// for all bits in transfer...
	{
		if (c & mask)  SPIx_MOSI_HIGH;
		else           SPIx_MOSI_LOW;

		SPIx_SCK_HIGH;
		if (SPIx_MISO_INPUT)  value = value | mask;
		SPIx_SCK_LOW;
		mask = mask >> 1;
		nbits--;
	}
	return  value;				// done with SPIx

}

