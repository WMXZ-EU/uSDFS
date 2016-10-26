/*
 *  sdcard.c      SD/SDHC card support library
 */
// Karl Lundt
// http://www.seanet.com/~karllunt/bareteensy31libs.html
//
#include  <stdint.h>
#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>

#include "kinetis.h"
#include "core_pins.h"

#include  "sdcard.h"

#include "sdspi.h"

#ifndef  FALSE
#define  FALSE		0
#define  TRUE		!FALSE
#endif

void logg(char c);

/*
 *  Externally accessible variables
 */
uint32_t			SDType = SDTYPE_UNKNOWN;


/*
 *  Local variables
 */
uint8_t				registered = FALSE;
void				(*chipselect)(void);
char				(*xchg)(char  val);
void				(*xferBulk)(char *inp, char *out, uint16_t nbytes);
void				(*chipdeselect)(void);
uint8_t				crctable[256];


/*
 *  Local functions
 */
static  int8_t  	sd_send_command(uint8_t  command, uint32_t  arg);
static  int8_t		sd_wait_for_data(void);
static void			sd_clock_and_release(void);

static void 		GenerateCRCTable(void);
static uint8_t 		AddByteToCRC(uint8_t  crc, uint8_t  b);


static void GenerateCRCTable(void)
{
    int ii, jj;
 
    // generate a table value for all 256 possible byte values
    for (ii = 0; ii < 256; ii++)
    {
        crctable[ii] = (ii & 0x80) ? ii ^ CRC7_POLY : ii;
        for (jj = 1; jj < 8; jj++)
        {
            crctable[ii] <<= 1;
            if (crctable[ii] & 0x80)
                crctable[ii] ^= CRC7_POLY;
        }
    }
}


static uint8_t  AddByteToCRC(uint8_t  crc, uint8_t  b)
{
	return crctable[(crc << 1) ^ b];
}


int32_t  SDInit(void)
{
	int					ii;
	int8_t				response;
//	volatile uint32_t	dly;


	if (registered == FALSE)  return  SDCARD_NOT_REG;

	SDType = SDTYPE_UNKNOWN;			// assume this fails
/*
 *  Begin initialization by sending CMD0 and waiting until SD card
 *  responds with In Idle Mode (0x01).  If the response is not 0x01
 *  within a reasonable amount of time, there is no SD card on the bus.
 */
	chipdeselect();						// always make sure
	for (ii=0; ii<10; ii++)				// send several clocks while card power stabilizes
		xchg(0xff);

	for (ii=0; ii<0x10; ii++)
	{
		response = sd_send_command(SD_GO_IDLE, 0);	// send CMD0 - go to idle state
		if (response == 1)  break;
	}
	if (response != 1)
	{
		return  SDCARD_NO_DETECT;
	}

	sd_send_command(SD_SET_BLK_LEN, 512);		// always set block length (CMD6) to 512 bytes

	response = sd_send_command(SD_SEND_IF_COND, 0x1aa);	// probe to see if card is SDv2 (SDHC)
	if (response == 0x01)						// if card is SDHC...
	{
		for (ii=0; ii<4; ii++)						// burn the 4-byte response (OCR)
		{
			xchg(0xff);
		}
		for (ii=20000; ii>0; ii--)
		{
			response = sd_send_command(SD_ADV_INIT, 1UL<<30);
			if (response == 0)  break;
		}
		SDType = SDTYPE_SDHC;
	}
	else
	{
		response = sd_send_command(SD_READ_OCR, 0);
		if (response == 0x01)
		{
			for (ii=0; ii<4; ii++)					// OCR is 4 bytes
			{
				xchg(0xff);					// burn the 4-byte response (OCR)
			}
			for (ii=20000; ii>0; ii--)
			{
				response = sd_send_command(SD_INIT, 0);
				if (response == 0)  break;
//				for (dly=0; dly<1000; dly++)  ;		// spin-loop delay
			}
			sd_send_command(SD_SET_BLK_LEN, 512);
			SDType = SDTYPE_SD;
		}
	}
	sd_clock_and_release();					// always deselect and send final 8 clocks
/*
 *  At this point, the SD card has completed initialization.  The calling routine
 *  can now increase the SPI clock rate for the SD card to the maximum allowed by
 *  the SD card (typically, 20 MHz).
 */
	return  SDCARD_OK;					// if no power routine or turning off the card, call it good
}




int32_t  SDStatus(void)
{
	if (registered == FALSE)	   return  SDCARD_NOT_REG;
	if (SDType == SDTYPE_UNKNOWN)  return  SDCARD_NO_DETECT;
	if ((SDType == SDTYPE_SD) || (SDType == SDTYPE_SDHC))  return  SDCARD_OK;

	return  SDCARD_NO_DETECT;
}



#if 0
static  void  ShowBlock(void)
{
	uint32_t				i;
	uint8_t					str[17];

	str[16] = 0;
	str[0] = 0;			// only need for first newline, overwritten as chars are processed

	printf_P(PSTR("\n\rContents of block buffer:"));
	for (i=0; i<512; i++)
	{
		if ((i % 16) == 0)
		{
			printf_P(PSTR(" %s\n\r%04X: "), str, i);
		}
		printf_P(PSTR("%02X "), (uint8_t)block[i]);
		if (isalpha(block[i]) || isdigit(block[i]))  str[i%16] = block[i];
		else									     str[i%16] = '.';
	}
	printf_P(PSTR(" %s\n\r"), str);
}


	
static int8_t  ExamineSD(void)
{
	int8_t			response;

	response = ReadOCR();		// this fails with Samsung; don't test response until know why
	response = ReadCSD();
	if (response == SDCARD_OK)
	{
//		printf_P(PSTR(" ReadCSD is OK "));
		response = ReadCID();
	}
	if (response == SDCARD_OK)
	{
//		printf_P(PSTR(" ReadCID is OK "));
		response = ReadCardStatus();
	}

	return  response;
}
#endif



int32_t  SDReadOCR(uint8_t  *buff)
{
	uint8_t				ii;
	int8_t				response;

	for (ii=0; ii<4;  ii++)  buff[ii] = 0;

	if (SDType == SDTYPE_SDHC)
	{
		response = sd_send_command(SD_SEND_IF_COND, 0x1aa);
		if (response != 0)
		{
			sd_clock_and_release();			// cleanup  
			return  SDCARD_RWFAIL;
		}
		for (ii=0; ii<4; ii++)
		{
			buff[ii] = xchg(0xff);
		}
		xchg(0xff);							// burn the CRC
	}
	else
	{
		response = sd_send_command(SD_READ_OCR, 0);
		if (response != 0x00)
		{
			sd_clock_and_release();			// cleanup  
			return  SDCARD_RWFAIL;
		}
		for (ii=0; ii<4; ii++)					// OCR is 4 bytes
		{
			buff[ii] = xchg(0xff);
		}
		xchg(0xff);
	}
    sd_clock_and_release();				// cleanup  
	return  SDCARD_OK;
}



int32_t  SDReadCSD(uint8_t  *buff)
{
	uint8_t			ii;
	int8_t			response;

	for (ii=0; ii<16; ii++)  buff[ii] = 0;

	response = sd_send_command(SD_SEND_CSD, 0);
	response = sd_wait_for_data();
	if (response != (int8_t)0xfe)
	{
		return  SDCARD_RWFAIL;
	}

	for (ii=0; ii<16; ii++)
	{
		buff[ii] = xchg(0xff);
	}
	xchg(0xff);							// burn the CRC
    sd_clock_and_release();				// cleanup  
	return  SDCARD_OK;
}



int32_t  SDReadCID(uint8_t  *buff)
{
	uint8_t			ii;
	int8_t			response;

	for (ii=0; ii<16; ii++)  buff[ii] = 0;

	response = sd_send_command(SD_SEND_CID, 0);
	response = sd_wait_for_data();
	if (response != (int8_t)0xfe)
	{
		return  SDCARD_RWFAIL;
	}

	for (ii=0; ii<16; ii++)
	{
		buff[ii] = xchg(0xff);
	}
	xchg(0xff);							// burn the CRC
    sd_clock_and_release();				// cleanup  
	return  SDCARD_OK;
}



int32_t  SDWriteCSD(uint8_t  *buff)
{
	int8_t				response;
	uint8_t				tcrc;
	uint16_t			ii;

	response = sd_send_command(SD_PROGRAM_CSD, 0);
	if (response != 0)
	{
		return  SDCARD_RWFAIL;
	}
	xchg(0xfe);							// send data token marking start of data block

	tcrc = 0;
	for (ii=0; ii<15; ii++)				// for all 15 data bytes in CSD...
	{
    	xchg(*buff);					// send each byte via SPI
		tcrc = AddByteToCRC(tcrc, *buff);		// add byte to CRC
	}
	xchg((tcrc<<1) + 1);				// format the CRC7 value and send it

	xchg(0xff);							// ignore dummy checksum
	xchg(0xff);							// ignore dummy checksum

	ii = 0xffff;							// max timeout
	while (!xchg(0xFF) && (--ii))  ;		// wait until we are not busy

    sd_clock_and_release();				// cleanup  
	if (ii)  return  SDCARD_OK;			// return success
	else  return  SDCARD_RWFAIL;		// nope, didn't work
}



#if 0
static int8_t  ReadCardStatus(void)
{
	cardstatus[0] = sd_send_command(SD_SEND_STATUS, 0);
	cardstatus[1] = xchg(0xff);
//	printf_P(PSTR("\r\nReadCardStatus = %02x %02x"), cardstatus[0], cardstatus[1]);
	xchg(0xff);
	return  SDCARD_OK;
}
#endif



int32_t  SDReadBlock(uint32_t  blocknum, uint8_t  *buff)
{
//    uint16_t					ii;
	uint8_t						status;
	uint32_t					addr;

	if (!registered)  return  SDCARD_NOT_REG;		// if no SPI functions, leave now
	if (SDType == SDTYPE_UNKNOWN)  return  SDCARD_UNKNOWN;	// card type not yet known

/*
 *  Compute byte address of start of desired sector.
 *
 *  For SD cards, the argument to CMD17 must be a byte address.
 *  For SDHC cards, the argument to CMD17 must be a block (512 bytes) number.
 */
	if (SDType == SDTYPE_SD) 	addr = blocknum << 9;	// SD card; convert block number to byte addr
	else						addr = blocknum;	// SDHC card; use the requested block number

    sd_send_command(SD_READ_BLK, addr); // send read command and logical sector address
	status = sd_wait_for_data();		// wait for valid data token from card
	if (status != 0xfe)					// card must return 0xfe for CMD17
    {
        sd_clock_and_release();			// cleanup 
        return  SDCARD_RWFAIL;			// return error code
    }

/*
    for (ii=0; ii<512; ii++)           	// read sector data
        buff[ii] = xchg(0xff);
*/
	xferBulk(buff,0,512);

	xchg(0xff);                		 	// ignore CRC
    xchg(0xff);                		 	// ignore CRC

    sd_clock_and_release();				// cleanup  
    return  SDCARD_OK;					// return success       
}


#if 0
static int8_t  ModifyPWD(uint8_t  mask)
{
	int8_t						r;
	uint16_t					i;

	mask = mask & 0x07;					// top five bits MUST be 0, do not allow forced-erase!
	r = sd_send_command(SD_LOCK_UNLOCK, 0);
	if (r != 0)
	{
		return  SDCARD_RWFAIL;
	}
	xchg(0xfe);							// send data token marking start of data block

	xchg(mask);							// always start with required command
	xchg(pwd_len);						// then send the password length
	for (i=0; i<512; i++)				// need to send one full block for CMD42
	{
		if (i < pwd_len)
		{
    		xchg(pwd[i]);					// send each byte via SPI
		}
		else
		{
			xchg(0xff);
		}
	}

	xchg(0xff);							// ignore dummy checksum
	xchg(0xff);							// ignore dummy checksum

	i = 0xffff;							// max timeout
	while (!xchg(0xFF) && (--i))  ;		// wait until we are not busy

	if (i)  return  SDCARD_OK;			// return success
	else  return  SDCARD_RWFAIL;		// nope, didn't work
}
#endif


#if 0
static void  ShowErrorCode(int8_t  status)
{
	if ((status & 0xe0) == 0)			// if status byte has an error value...
	{
		printf_P(PSTR("\n\rDate error:"));
		if (status & ERRTKN_CARD_LOCKED)
		{
			printf_P(PSTR(" Card is locked!"));
		}
		if (status & ERRTKN_OUT_OF_RANGE)
		{
			printf_P(PSTR(" Address is out of range!"));
		}
		if (status & ERRTKN_CARD_ECC)
		{
			printf_P(PSTR(" Card ECC failed!"));
		}
		if (status & ERRTKN_CARD_CC)
		{
			printf_P(PSTR(" Card CC failed!"));
		}
	}
}
#endif



#if 0
static void  ShowCardStatus(void)
{
	ReadCardStatus();
	printf_P(PSTR("\r\nPassword status: "));
	if ((cardstatus[1] & 0x01) ==  0)  printf_P(PSTR("un"));
	printf_P(PSTR("locked"));
}
#endif



#if 0
static void  LoadGlobalPWD(void)
{
	uint8_t				i;

	for (i=0; i<GLOBAL_PWD_LEN; i++)
	{
		pwd[i] = pgm_read_byte(&(GlobalPWDStr[i]));
	}
	pwd_len = GLOBAL_PWD_LEN;
}
#endif




int32_t	 SDRegisterSPI(
					void (*pselect)(void),
					char (*pxchg)(char  val),
					void (*pxferBulk)(char  *out, char *inp, uint16_t nbytes),
					void (*pdeselect)(void))
{
	uint32_t					result;

	GenerateCRCTable();				// need to do at some point, here is as good as any

	chipselect = pselect;
	xchg = pxchg;
	xferBulk = pxferBulk;
	chipdeselect = pdeselect;

	result = SDCARD_OK;				// assume all pointers are at least believable
	registered = FALSE;
	if (chipselect && xchg && chipdeselect)  registered = TRUE;
	if (!registered)  result = SDCARD_REGFAIL;	// missing function handle
	return  result;
}


#define DEVELOP 0

/*
 *  SDWriteBlock      write buffer of data to SD card
 *
 *  I've lost track of who created the original code for this
 *  routine; I found it somewhere on the web.
 *
 *	Write a single 512 byte sector to the MMC/SD card
 *  blocknum holds the number of the 512-byte block to write,
 *  buffer points to the 512-byte buffer of data to write.
 *
 *  Note that for SD (not SDHC) cards, blocknum will be converted
 *  into a byte address.
 */
int32_t  SDWriteBlock(uint32_t  blocknum, uint8_t  *buff)
{
	uint32_t				ii;
	uint8_t					status;
	uint32_t				addr;

	if (!registered)  return  SDCARD_NOT_REG;

/*
 *  Compute byte address of start of desired sector.
 *
 *  For SD cards, the argument to CMD17 must be a byte address.
 *  For SDHC cards, the argument to CMD17 must be a block (512 bytes) number.
 */
	if (SDType == SDTYPE_SD)  addr = blocknum << 9;	// SD card; convert block number to byte addr
	else					  addr = blocknum;		// SDHC card; just use blocknum as is

	status = sd_send_command(SD_WRITE_BLK, addr);

	if (status != SDCARD_OK)			// if card does not send back 0...
	{
		sd_clock_and_release();			// cleanup
		return  SDCARD_RWFAIL;
	}

#if DEVELOP == 1
	if(!usd_writeData(DATA_START_BLOCK,buff)) return SDCARD_RWFAIL;
#else

	//	DATA_START_BLOCK is 0xFE
	xchg(0xfe);							// send data token marking start of data block

/*	for (ii=0; ii<512; ii++)				// for all bytes in a sector...
	{
    	xchg(*buff++);					// send each byte via SPI
	}
*/
	xferBulk(0,buff,512);

	xchg(0xff);							// ignore dummy checksum
	xchg(0xff);							// ignore dummy checksum

	if ((xchg(0xFF) & 0x0F) != 0x05)
	{
		sd_clock_and_release();			// cleanup
		return  SDCARD_RWFAIL;
	}
#endif
	
	ii = 0xffffff;							// max timeout  (nasty timing-critical loop!)
	while (!xchg(0xFF) && (--ii))  ;		// wait until we are not busy
	sd_clock_and_release();				// cleanup
	return  SDCARD_OK;					// return success		
}


/*
 *  sd_waitforready      wait until write operation completes
 *
 *  Normally not used, this function is provided to support Chan's FAT32 library.
 */
uint8_t  sd_waitforready(void)
{
   uint8_t				ii;
   uint16_t				jj;

	if (!registered)  return  SDCARD_NOT_REG;

	chipselect();						// enable CS
    xchg(0xff);         				// dummy byte

	jj = 5000;
	do  {
		ii = xchg(0xFF);
	} while ((ii != 0xFF) && --jj);

	sd_clock_and_release();

	if (ii == 0xff)	return  SDCARD_OK;	// if SD card shows ready, report OK
	else			return  SDCARD_RWFAIL;		// else report an error
}



/*
 *  ==========================================================================
 *
 *  sd_send_command      send raw command to SD card, return response
 *
 *  This routine accepts a single SD command and a 4-byte argument.  It sends
 *  the command plus argument, adding the appropriate CRC.  It then returns
 *  the one-byte response from the SD card.
 *
 *  For advanced commands (those with a command byte having bit 7 set), this
 *  routine automatically sends the required preface command (CMD55) before
 *  sending the requested command.
 *
 *  Upon exit, this routine returns the response byte from the SD card.
 *  Possible responses are:
 *    0xff	No response from card; card might actually be missing
 *    0x01  SD card returned 0x01, which is OK for most commands
 *    0x?? 	other responses are command-specific
 */
static  int8_t  sd_send_command(uint8_t  command, uint32_t  arg)
{
	uint8_t				response;
	uint8_t				ii;
	uint8_t				crc;

	if (command & 0x80)					// special case, ACMD(n) is sent as CMD55 and CMDn
	{
		command = command & 0x7f;		// strip high bit for later
		response = sd_send_command(CMD55, 0);	// send first part (recursion)
		if (response > 1)  return response;
	}

	sd_clock_and_release();

	chipselect();							// enable CS
	xchg(0xff);

    xchg(command | 0x40);				// command always has bit 6 set!
	xchg((unsigned char)(arg>>24));		// send data, starting with top byte
	xchg((unsigned char)(arg>>16));
	xchg((unsigned char)(arg>>8));
	xchg((unsigned char)(arg&0xff));
	crc = 0x01;							// good for most cases
	if (command == SD_GO_IDLE)  crc = 0x95;			// this will be good enough for most commands
	if (command == SD_SEND_IF_COND)  crc = 0x87;	// special case, have to use different CRC
    xchg(crc);         					// send final byte                          

	for (ii=0; ii<10; ii++)				// loop until timeout or response
	{
		response = xchg(0xff);
		if ((response & 0x80) == 0)  break;	// high bit cleared means we got a response
	}

/*
 *  We have issued the command but the SD card is still selected.  We
 *  only deselect the card if the command we just sent is NOT a command
 *  that requires additional data exchange, such as reading or writing
 *  a block.
 */
	if ((command != SD_READ_BLK) &&
		(command != SD_WRITE_BLK) &&
		(command != SD_READ_OCR) &&
		(command != SD_SEND_CSD) &&
		(command != SD_SEND_STATUS) &&
		(command != SD_SEND_CID) &&
		(command != SD_SEND_IF_COND) &&
		(command != SD_LOCK_UNLOCK) &&
		(command != SD_PROGRAM_CSD))
	{
		sd_clock_and_release();
	}

	return  response;					// let the caller sort it out
}


static int8_t  sd_wait_for_data(void) // similar to usd_waitNotBusy
{
	int16_t				ii;
	uint8_t				rr;
	volatile uint32_t	dly;

	for (ii=0; ii<1000; ii++)
	{
		rr = xchg(0xff);
		if (rr != 0xff)  break;
		for (dly=0; dly<1000; dly++)  ;		// spin-loop delay
	}
	return  (int8_t) rr;
}

/*
 *  sd_clock_and_release      deselect the SD card and provide additional clocks
 *
 *  The SD card does not release its DO (MISO) line until it sees a clock on SCK
 *  AFTER the CS has gone inactive (Chan, mmc_e.html).  This is not an issue if the
 *  only device on the SPI bus is the SD card, but it can be an issue if the
 *  card shares the bus with other SPI devices.
 *
 *  To be safe, this routine deselects the SD card, then issues eight clock pulses.
 */
static void  sd_clock_and_release(void)
{
	if (!registered)  return;

	chipdeselect();				   			// release CS
	xchg(0xff);								// send 8 final clocks
}


/**************************************** for multi sector operations **************************/
/**
 */
#include "sdcard_priv.h"
//
//--------------------------------- used in uSD (from SDFAT)-----------------------------------
// should be merged with previous code
// also one command routine should be removes.

#undef USE_SD_CRC

inline void usd_chipSelectLow()
{	chipselect();
}

inline void usd_chipSelectHigh()
{	chipdeselect();
	xchg(0xff); // send 8 final clocks
}

inline void usd_error(uint32_t error){m_usd_error=error;}
uint32_t usd_getError(void) { return m_usd_error;}

inline void usd_status(uint32_t status){m_usd_status=status;}
uint32_t usd_getStatus(void) { return m_usd_status;}


// wait for card to go not busy
/**
 *
 * @param timeoutMillis
 * @return
 */
uint16_t usd_waitNotBusy(uint16_t timeoutMillis, uint16_t *status)
{
	uint16_t t0 = millis();
	xchg(0xff);
	while ((*status = xchg(0xff)) != 0XFF)
	{
		if (((uint16_t)millis() - t0) >= timeoutMillis)
		{	goto fail;
		}
	}
	return 1;
fail:
	return 0;
}

/**
 *
 * @param cmd
 * @param arg
 * @return
 */
uint16_t usd_cardCommand(uint8_t cmd, uint32_t arg)
{
	int ii;
	uint16_t status;

	usd_chipSelectLow();
	// wait if busy
	if(!usd_waitNotBusy(SD_WRITE_TIMEOUT, &status)) logg('-');

	uint8_t *pa = (uint8_t*)(&arg);

	#if USE_SD_CRC
	// form message
		uint8_t d[6] = {cmd | 0X40, pa[3], pa[2], pa[1], pa[0]};

		// add crc
		d[5] = CRC7(d, 5);

		// send message
		for (ii = 0; ii < 6; ii++) { xchg(d[ii]); }
	#else  // USE_SD_CRC
		// send command
		xchg(cmd | 0x40);

		// send argument
		for (ii = 3; ii >= 0; ii--) { xchg(pa[ii]); }

		// send CRC - correct for CMD0 with arg zero or CMD8 with arg 0X1AA
		xchg(cmd == SD_GO_IDLE ? 0X95 : 0X87);
	#endif  // USE_SD_CRC

	// skip stuff byte for stop read
	if (cmd == SD_READ_STOP) { xchg(0xff); }

	// wait for response
	for (ii = 0; ((status = xchg(0xff)) & 0X80) && ii != 0XFF; ii++) {}
	return status;
}

/**
 *
 * @param cmd
 * @param arg
 * @return
 */
uint8_t usd_cardACommand(uint8_t cmd, uint32_t arg)
{
	usd_cardCommand(CMD55, 0);
    return usd_cardCommand(cmd, arg);
}

/**
 *
 * @param dst
 * @param count
 * @return
 */
uint16_t usd_readData(uint8_t* dst, size_t count)
{
	uint16_t status;
#if USE_SD_CRC
	uint16_t crc;
#endif  // USE_SD_CRC
	// wait for start block token
	if(!usd_waitNotBusy(SD_READ_TIMEOUT, &status))
	{
		usd_error(SD_CARD_ERROR_READ_TIMEOUT);
		goto fail;
	}

	if (status != DATA_START_BLOCK)
	{
		usd_error(SD_CARD_ERROR_READ);
		goto fail;
	}
	// transfer data
	xferBulk(dst, 0, count);

#if USE_SD_CRC
	// get crc
	crc = (xchg(0xff)<< 8) |xchg(0xff);
	if (crc != CRC_CCITT(dst, count))
	{
		usd_error(SD_CARD_ERROR_READ_CRC);
		goto fail;
	}
#else
	  // discard crc
	  xchg(0xff);
	  xchg(0xff);
#endif  // USE_SD_CRC
	usd_chipSelectHigh();
	return 1;
fail:
	usd_chipSelectHigh();
	return 0;
}

/**
 *
 * @param blockNumber
 * @return
 */
uint16_t usd_readStart(uint32_t blockNumber)
{
	if (usd_cardCommand(SD_READ_MULTI, blockNumber))
	{
		usd_error(SD_CARD_ERROR_CMD18);
		goto fail;
	}
	usd_chipSelectHigh();
	return 1;

	fail:
		usd_chipSelectHigh();
	return 0;
}

/**
 *
 * @return
 */
uint16_t usd_readStop(void)
{
	if (usd_cardCommand(SD_READ_STOP, 0))
	{
		usd_error(SD_CARD_ERROR_CMD12);
		goto fail;
	}
	usd_chipSelectHigh();
	return 1;

	fail:
		usd_chipSelectHigh();
	return 0;
}

/**
 *
 * @param dst
 * @return
 */
uint16_t usd_readSector(uint8_t* dst)
{
	usd_chipSelectLow();
	return usd_readData(dst, 512);
}

/**
 *
 * @param block
 * @param dst
 * @param count
 * @return
 */
uint32_t SDReadBlocks(uint32_t block, uint8_t* dst, uint32_t count)
{
	uint16_t ii;
	if (!usd_readStart(block))	return SDCARD_RWFAIL;
	for (ii=0; ii < count; ii++, dst += 512)
	{
		if (!usd_readSector(dst)) return SDCARD_RWFAIL;
	}
	if(!usd_readStop()) return SDCARD_RWFAIL;
	return SDCARD_OK;
}

//--------------------- Continuous write -------------------------
// send one block of data for write block or write multiple blocks
/**
 * Multi-block write sequence
 *   writeStart
 *   multiple writeSector //calls writeData
 *   writeStop
 */


/**
 *
 * @param token
 * @param src
 * @return
 */
uint16_t usd_writeData(uint8_t token, const uint8_t* src)
{
	uint16_t status;
	#if USE_SD_CRC
		uint16_t crc = CRC_CCITT(src, 512);
	#else  // USE_SD_CRC
		uint16_t crc = 0XFFFF;
	#endif  // USE_SD_CRC

	xchg(token);
	xferBulk(0, (char *) src, 512);
	xchg(crc >> 8);
	xchg(crc & 0XFF);

	status = xchg(0xff);
	if ((status & DATA_RES_MASK) != DATA_RES_ACCEPTED)
	{
		usd_error(SD_CARD_ERROR_WRITE);
		goto fail;
	}
	usd_chipSelectHigh();
	return 1;

fail:
	usd_chipSelectHigh();
	return 0;
}

// the following is only for comparison
int32_t  _SDWriteBlock(uint32_t  blocknum, uint8_t  *buff)
{
	uint32_t				ii;
	uint8_t					status;
	uint32_t				addr;

	if (!registered)  return  SDCARD_NOT_REG;

/*
 *  Compute byte address of start of desired sector.
 *
 *  For SD cards, the argument to CMD17 must be a byte address.
 *  For SDHC cards, the argument to CMD17 must be a block (512 bytes) number.
 */
	if (SDType == SDTYPE_SD)  addr = blocknum << 9;	// SD card; convert block number to byte addr
	else					  addr = blocknum;		// SDHC card; just use blocknum as is

	status = sd_send_command(SD_WRITE_BLK, addr);

	if (status != SDCARD_OK)			// if card does not send back 0...
	{
		sd_clock_and_release();			// cleanup
		return  SDCARD_RWFAIL;
	}

	if(!usd_writeData(DATA_START_BLOCK,buff)) return SDCARD_RWFAIL;

//	DATA_START_BLOCK is 0xFE
//	xchg(0xfe);							// send data token marking start of data block
//
// /*	for (ii=0; ii<512; ii++)				// for all bytes in a sector...
//	{
//    	xchg(*buff++);					// send each byte via SPI
//	}
// */
//	xferBulk(0,buff,512);
//
//	xchg(0xff);							// ignore dummy checksum
//	xchg(0xff);							// ignore dummy checksum
//
//	if ((xchg(0xFF) & 0x0F) != 0x05)
//	{
//		sd_clock_and_release();			// cleanup
//		return  SDCARD_RWFAIL;
//	}


	ii = 0xffffff;							// max timeout  (nasty timing-critical loop!)
	while (!xchg(0xFF) && (--ii))  ;		// wait until we are not busy
	sd_clock_and_release();				// cleanup
	return  SDCARD_OK;					// return success
}

/**
 *
 * @param blockNumber
 * @param eraseCount
 * @return
 */
uint16_t usd_writeStart(uint32_t blockNumber, uint32_t eraseCount)
{
	uint32_t				addr;

	// send pre-erase count
	if(eraseCount)
	{
		if (usd_cardACommand(ACMD23, eraseCount))
		{	usd_error(SD_CARD_ERROR_ACMD23);
			goto fail;
		}
	}

	// use address if not SDHC card
	if (SDType == SDTYPE_SD)  addr = blockNumber << 9;	// SD card; convert block number to byte addr
	else					  addr = blockNumber;		// SDHC card; just use blocknum as is

	if (usd_cardCommand(SD_WRITE_MULTI, addr))
	{	usd_error(SD_CARD_ERROR_CMD25);
		goto fail;
	}
	usd_chipSelectHigh();
	return 1;

fail:
	usd_chipSelectHigh();
	return 0;
}

/**
 *
 * @return
 */
uint16_t usd_writeStop(void)
{
	uint16_t status;
	usd_chipSelectLow();
	if (!usd_waitNotBusy(SD_WRITE_TIMEOUT, &status)) goto fail;
	xchg(STOP_TRAN_TOKEN);
	if (!usd_waitNotBusy(SD_WRITE_TIMEOUT, &status)) goto fail;
	usd_chipSelectHigh();
	return 1;
fail:
	usd_status(status);
	usd_error(SD_CARD_ERROR_STOP_TRAN);
	usd_chipSelectHigh();
	return 0;
}

/**
 *
 * @param src
 * @return
 */
uint16_t usd_writeSector(const uint8_t* src)
{
	uint16_t status;
	usd_chipSelectLow();
	if (!usd_waitNotBusy(SD_WRITE_TIMEOUT,&status)) { usd_error(SD_CARD_ERROR_WRITE_TIMEOUT); goto fail;}
	if (!usd_writeData(WRITE_MULTIPLE_TOKEN, src)) { usd_error(SD_CARD_ERROR_WRITE_MULTIPLE); goto fail; }
	return 1;
fail:
	usd_chipSelectHigh();
	return 0;
}


//------------------------------------------------------------------------------
/**
 *
 * @param block
 * @param dst
 * @param count
 * @return
 */
uint32_t SDWriteBlocks(uint32_t block, const uint8_t* src, uint32_t count)
{
	uint16_t ii;
	if (!usd_writeStart(block, 0)) 	return SDCARD_RWFAIL;
	for (ii = 0; ii < count; ii++, src += 512)
	{
		if (!usd_writeSector(src))  return  SDCARD_RWFAIL;
	}
	if(!usd_writeStop()) return  SDCARD_RWFAIL;
	return  SDCARD_OK;
}

