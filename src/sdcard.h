/*
 *  sdcard.h      header file for the SD/SDHC card library code
 */
// Karl Lundt
// http://www.seanet.com/~karllunt/bareteensy31libs.html
//

#ifndef  SDCARD_H
#define  SDCARD_H

/*
 *  Define commands for the SD card
 */
#define  SD_GO_IDLE			(0x40 + 0)			/* CMD0 - go to idle state */
#define  SD_INIT			(0x40 + 1)			/* CMD1 - start initialization */
#define  SD_SEND_IF_COND	(0x40 + 8)			/* CMD8 - send interface (conditional), works for SDHC only */
#define  SD_SEND_CSD		(0x40 + 9)			/* CMD9 - send CSD block (16 bytes) */
#define  SD_SEND_CID		(0x40 + 10)			/* CMD10 - send CID block (16 bytes) */
#define  SD_SEND_STATUS		(0x40 + 13)			/* CMD13 - send card status */
#define  SD_SET_BLK_LEN		(0x40 + 16)			/* CMD16 - set length of block in bytes */
#define  SD_READ_BLK		(0x40 + 17)			/* CMD17 read single block */
#define  SD_WRITE_BLK		(0x40 + 24)			/* CMD24 write single block */
#define  SD_LOCK_UNLOCK		(0x40 + 42)			/* CMD42 - lock/unlock card */
#define  SD_READ_OCR		(0x40 + 58)			/* read OCR */
#define  SD_ADV_INIT		(0xc0 + 41)			/* ACMD41, for SDHC cards - advanced start initialization */
#define  SD_PROGRAM_CSD		(0x40 + 27)			/* CMD27 - get CSD block (15 bytes data + CRC) */

#define  SD_READ_MULTI		(0x40 + 18)			/* CMD18 - multi block read */
#define  SD_WRITE_MULTI		(0x40 + 25)			/* CMD25 - multi block write */
#define  SD_READ_STOP		(0x40 + 12)			/* CMD12 - stop multi block read */

#define  CMD55	(55)			/* multi-byte preface command */
/** SET_WR_BLK_ERASE_COUNT - Set the number of write blocks to be
     pre-erased before writing */
#define ACMD23	(23)


/** ERASE_WR_BLK_START - sets the address of the first block to be erased */
#define CMD32	(32)
/** ERASE_WR_BLK_END - sets the address of the last block of the continuous
    range to be erased*/
#define CMD33	(33)
/** ERASE - erase all previously selected blocks */
#define CMD38	(38)


/*
 *  Define error codes that can be returned by library functions
 */
#define  SDCARD_OK					0			/* success */
#define  SDCARD_NO_DETECT			-1			/* unable to detect SD card */
#define  SDCARD_TIMEOUT				-2			/* last operation timed out */
#define  SDCARD_RWFAIL				-3			/* read/write command failed */
#define  SDCARD_REGFAIL				-4			/* bad function pointer in call to SDRegister() */
#define  SDCARD_NOT_REG				-5			/* SPI access functions not known; see SDRegister() */
#define  SDCARD_UNKNOWN				-6			/* card type is unknown (SDInit not called?) */


/*
 *  Define options for accessing the SD card's PWD (CMD42)
 */
#define  MASK_ERASE					0x08		/* erase the entire card */
#define  MASK_LOCK_UNLOCK			0x04		/* lock or unlock the card with password */
#define  MASK_CLR_PWD				0x02		/* clear password */
#define  MASK_SET_PWD				0x01		/* set password */


/*
 *  Define card types that could be reported by the SD card during probe
 */
#define  SDTYPE_UNKNOWN			0				/* card type not determined */
#define  SDTYPE_SD				1				/* SD v1 (1 MB to 2 GB) */
#define  SDTYPE_SDHC			2				/* SDHC (4 GB to 32 GB) */


/*
 *  Define the CRC7 polynomial, used for block check of some SD commands
 */
#define  CRC7_POLY		0x89		/* polynomial used for CSD CRCs */


/*
 *  Define bit masks for fields in the lock/unlock command (CMD42) data structure
 */
#define  SET_PWD_MASK		(1<<0)
#define  CLR_PWD_MASK		(1<<1)
#define  LOCK_UNLOCK_MASK	(1<<2)
#define  ERASE_MASK			(1<<3)


/*
 *  Globally available variables used by the SD card library.
 */
extern  uint32_t				SDType;			// holds card type (SD or SDHC)

#ifdef __cplusplus
extern "C"{
#endif

/*
 *  Function available in the library module.  These functions are available
 *  to external routines.
 */

/*
 *  SDRegisterSPI      register SPI access functions with the SD library
 *
 *  This routine allows the calling program to provide function pointers
 *  for performing SPI operations with the SD card.
 *
 *  The SD card library is hardware-agnostic; it does not know or care
 *  how the SD card is connected to the target hardware.  All SPI access
 *  is done through the three supplied function pointers.  Code in the
 *  calling firmware must perform the functions provided by these pointers,
 *  using whatever hardware is properl.
 *
 *  Argument select points to a function that sets the SD card's select line
 *  to active state.
 *
 *  Argument xchg points to a function that sends a byte of data to the
 *  SD card and recives a byte of data in exchange.
 *
 *  Argument xferBulk points to a function that sends multiple bytes of data to the
 *  SD card and receives the same amount of bytes of data in exchange.
 *
 *  Argument deselect points to a function that sets the SD card's select
 *  line to inactive state.
 *
 *  Upon exit, this routine returns a status code.  SDCARD_OK means the
 *  function addresses have been recorded and ready for later use.
 *  SDCARD_REGFAIL means one of the function pointers was invalid or
 *  null.
 */
int32_t	SDRegisterSPI(
		void (*select)(void),
		char (*xchg)(char  val),
		void (*xferBulk)(char  *out, char *inp, uint16_t nbytes),
		void (*deselect)(void));

/*
 *  SDInit      initlialize the SD card and the library support variables
 *
 *  This routine forces the card to perform its startup ritual, then checks
 *  the card for its type.  If the card responds properly, the type is stored
 *  in the global variable SDType for external access.  If the card does
 *  not respond properly, SDType will show card type as unknown.
 *
 *  This routine relies on three external functions for control of data
 *  exchange via SPI.  Refer to SDRegister() for details.
 *
 *  Upon entry, this routine uses SPI traffic to access the SD card and
 *  begin the card's startup ritual.  If the SPI functions needed for this
 *  exchange are not availalbe (SDRegister not called yet), this routine
 *  returns an error code.
 *
 *  Upon exit, this routine returns a status code:
 *  SDCARD_OK means the SD/SDHC card initialized properly; card type is stored
 *  in SDType.
 *  SDCARD_NO_DETECT means the code could not get a response of any kind from
 *  the card; check the electrical connections to the card.
 *  SDCARD_TIMEOUT means one of the exchanges with the card did not complete
 *  peroperly.  This could mean a defective or out-of-spec SD card.
 *  SDCARD_RWFAIL means the card returned an unexcpected result during a data
 *  exhange.  This could mean a defective or out-of-spec SD card.
 */
int32_t		SDInit(void);


/*
 *  SDStatus      returns status of SD card and support library
 *
 *  This routine returns a status code based on the detected SD card (if any)
 *  and the state of the SD support routines.
 *
 *  Upon exit, this routine returns SDCARD_OK if an SD card (either SD or SDHC)
 *  has been detected.  It returns SDCARD_NOT_REG if the SDInit() routine was
 *  never called or if the most recent call failed.  It returns SDCARD_NO_DETECT
 *  if the previous attempt to initialize the card returned some kind of error,
 *  such as timeout or no detect.
 */
int32_t		SDStatus(void);


/*
 *  SDReadOCR      read SD card's OCR data to buffer
 *
 *  This routine reads the card's OCR data and saves it to the buffer
 *  passed by pointer in argument buff.
 *
 *  The buffer must be large enough to hold the entire OCR data block (4 bytes).
 *
 *  Upon exit, this routine returns a status code showing the result of
 *  the read operation.
 */
int32_t		SDReadOCR(uint8_t  *buff);


/*
 *  SDReadCSD      read SD card's CSD data to buffer
 *
 *  This routine reads the card's CSD data and saves it to the buffer
 *  passed by pointer in argument buff.
 *
 *  The buffer must be large enough to hold the entire CSD data block
 *  (15 bytes of data plus ending CRC byte, total of 16 bytes).
 *
 *  Upon exit, this routine returns a status code showing the result of
 *  the read operation.
 */
int32_t		SDReadCSD(uint8_t  *buff);


/*
 *  SDReadCID      read SD card's CID data to buffer
 *
 *  This routine reads the card's CID data and saves it to the buffer
 *  passed by pointer in argument buff.
 *
 *  The buffer must be large enough to hold the entire CID data block
 *  (15 bytes of data plus ending CRC byte, total of 16 bytes).
 *
 *  Upon exit, this routine returns a status code showing the result of
 *  the read operation.
 */
int32_t		SDReadCID(uint8_t  *buff);


/*
 *  SDWriteCSD      write data to card's CSD block
 *
 *  This routine writes the data in the buffer passed by pointer in argument
 *  buff to the SD card's CSD block.
 *
 *  The buffer must be large enough to hold the entire CSD data block
 *  (15 bytes of data); the ending CRC byte will be computed and appended
 *  by this routine.
 *
 *  Upon exit, this routine returns a status code showing the result of
 *  the write operation.
 *
 *  The CSD can be modified to enable/disable features of the card, such
 *  as copy protection and permanent or temporary write protection.  For
 *  details, refer to the SD Physical Layer Specification.
 */
int32_t		SDWriteCSD(uint8_t  *buff);


/*
 *  SDReadBlock      read a block of data from the SD card
 *
 *  This routine reads a block (512 bytes) from the SD card from
 *  the specified block number in argument blocknum.  The data is written
 *  to the buffer pointed to by argument buff.
 */
int32_t		SDReadBlock(uint32_t  blocknum, uint8_t  *buff);


/*
 *  SDWriteBlock      write a block of data to the SD card
 *
 *  This routine writes a block (512 bytes) to the SD card at the specified
 *  block number in argument blocknum.  The data is read from the buffer
 *  pointed to by argument buff.
 */
int32_t		SDWriteBlock(uint32_t  blocknum, uint8_t  *buff);

/*
 * addition for multi block operation
 */

uint32_t SDReadBlocks(uint32_t block, uint8_t* dst, uint32_t count);

uint32_t SDWriteBlocks(uint32_t block, const uint8_t* src, uint32_t count);

uint8_t  sd_waitforready(void);

uint32_t usd_getError(void) ;
uint32_t usd_getStatus(void) ;


#ifdef __cplusplus
}
#endif

#endif
