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
 
#include <stddef.h>
#include "core_pins.h"
//#include "usb_serial.h"
//#define DO_DEBUG

#include "../ff.h"
#include "../diskio.h"

#include "sd_config.h"
#include "sd_defs.h"
#include "sd_spi.h"

#define false 0
#define true 1

uint16_t sd_init(uint16_t cs_pin);
uint16_t sd_readStart(uint32_t blockNumber) ;
uint16_t sd_readStop() ;
uint16_t sd_writeStart(uint32_t blockNumber, uint32_t eraseCount) ;
uint16_t sd_writeStop(void) ;

uint16_t sd_readData2(uint8_t* dst);
uint16_t sd_writeData2(const uint8_t* src) ;

uint16_t sd_readBlock(uint32_t blockNumber, uint8_t* dst);
uint16_t sd_writeBlock(uint32_t blockNumber, const uint8_t* src) ;

DSTATUS SPI_disk_status()
{
    return 0;
}
DSTATUS SPI_disk_initialize()
{
    if(!sd_init(CS_PIN)) return STA_NOINIT;
    
    return 0;
}

DRESULT SPI_disk_read(BYTE *buff, DWORD sector, UINT count)
{	
	DRESULT res = RES_OK;
    if(!sd_readStart(sector)) res = RES_READERROR;
    for(int ii=0; ii<count;ii++)
    {
      if(!sd_readData2(buff)) res = RES_READERROR;
      sector++;
      buff += 512;    
    }
    if(!sd_readStop()) res = RES_READERROR;

    return res;
}
DRESULT SPI_disk_write(const BYTE *buff, DWORD sector, UINT count)
{
    DRESULT res = RES_OK;
    if(!sd_writeStart(sector,count)) res = RES_WRITEERROR;
    for(int ii=0; ii<count;ii++)
    {
      if(!sd_writeData2(buff)) res = RES_WRITEERROR;
      sector++;
      buff += 512;    
    }
    if(!sd_writeStop()) res = RES_WRITEERROR;

    return res;
}
DRESULT SPI_disk_ioctl(BYTE cmd, BYTE *buff)
{
    return RES_OK;
}

#if defined(__MK20DX128__) || defined(__MK20DX256__) || defined(__MK64FX512__) || defined(__MK66FX1M0__) \
		|| defined(__IMXRT1052__) || defined(__IMXRT1062__) 

/*************************************** SD ***********************************/
#define FALSE false
#define TRUE true

  uint16_t m_chipSelectPin=-1;
  uint16_t m_sd_errorCode;
  uint16_t m_sckDivisor;
  uint16_t m_sd_status;
  uint16_t m_sd_type;

// some short functions 
void sd_setError(uint8_t error) {m_sd_errorCode=error;}
uint16_t sd_getError(void) {return m_sd_errorCode;}

void sd_setType(uint16_t type) {m_sd_type=type;}
uint16_t sd_getType(void) {return m_sd_type;}

void sd_setChipSelect(uint16_t pin) {m_chipSelectPin=pin;}
void sd_chipSelect(uint16_t high_low); 

// some forward declarations for sd_init
int sd_connect();
uint32_t sd_cardSize(void) ;

void spi_configPorts(int iconf);
uint32_t spi_setup(uint32_t clk);
uint8_t spi_receive();
void spi_send(uint8_t b);
uint8_t spi_receiveBulk(uint8_t* buf, size_t n) ;
void spi_sendBulk(const uint8_t* buf , size_t n) ;

//
//***************************************************************************/

#ifdef USE_SD_CRC
  //------------------------------------------------------------------------------
  // CRC functions
  static uint8_t CRC7(const uint8_t* data, uint8_t n) {
    uint8_t crc = 0;
    uint8_t ii,jj;
    for (ii = 0; ii < n; ii++) 
    {
    uint8_t d = data[ii];
    for (jj = 0; jj < 8; jj++) {
      crc <<= 1;
      if ((d & 0x80) ^ (crc & 0x80)) crc ^= 0x09;
      d <<= 1;
    }
    }
    return (crc << 1) | 1;
  }
  //------------------------------------------------------------------------------
  #if USE_SD_CCITT == 1
    // slower CRC-CCITT
    // uses the x^16,x^12,x^5,x^1 polynomial.
    static uint16_t CRC_CCITT(const uint8_t *data, size_t n) {
      uint16_t crc = 0;
      size_t ii;
      for (ii = 0; ii < n; ii++) {
      crc = (uint8_t)(crc >> 8) | (crc << 8);
      crc ^= data[ii];
      crc ^= (uint8_t)(crc & 0xff) >> 4;
      crc ^= crc << 12;
      crc ^= (crc & 0xff) << 5;
      }
      return crc;
    }
  #elif USE_SD_CCITT > 1  // CRC_CCITT
    //------------------------------------------------------------------------------
    // faster CRC-CCITT
    // uses the x^16,x^12,x^5,x^1 polynomial.
    static const uint16_t crctab[] = {
      0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
      0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
      0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
      0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
      0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
      0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
      0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
      0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
      0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
      0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
      0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
      0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
      0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
      0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
      0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
      0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
      0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
      0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
      0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
      0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
      0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
      0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
      0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
      0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
      0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
      0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
      0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
      0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
      0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
      0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
      0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
      0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
    };

    static uint16_t CRC_CCITT(const uint8_t* data, size_t n) {
      uint16_t crc = 0;
      size_t ii;
      for (ii = 0; ii < n; ii++) {
      crc = crctab[(crc >> 8 ^ data[ii]) & 0XFF] ^ (crc << 8);
      }
      return crc;
    }
  #endif  // USE_SD_CCITT
#endif  // USE_SD_SRC

//***************************************************************************/
uint16_t sd_init(uint16_t cs_pin)
{  int ii;
  pinMode(cs_pin, OUTPUT);

  spi_configPorts(1); // 1 is PJRC Audio

  spi_setup(100);

  sd_setChipSelect(cs_pin);
  sd_chipSelect(HIGH);
  // must supply min of 74 clock cycles with CS high.
  for (ii = 0; ii < 10; ii++) spi_send(0XFF);

  if(!sd_connect())
  {  // uint32_t errorcode=sd_getError();
//     printDebug("connect:", errorcode);
    return false;
  }
  else
  {  
#ifdef DEBUG_THIS
     int32_t sd_size,sd_type;bb[4]; 
     bb[0]=bb[1]=bb[2]=bb[3]=0;
     sd_type=sd_getType();
     printDebug("Card Type:",sd_type);
     sd_size=sd_cardSize();
     printDebug("Card Size:",sd_size);
#endif
  }
  
  #define MEG (1000*1000)
//  int divide = F_BUS/(12.5f*MEG);
//    printDebug("F_BUS:",F_BUS);
//    printDebug("divide:",divide);
  spi_setup(24*MEG);

  return true;
}

//------------------------------------------------------------------------------
// wait for card to go not busy
uint16_t sd_waitNotBusy(uint16_t timeoutMillis) 
{
  uint16_t t0;
  uint16_t ret;
  t0  = millis();
  while ((ret=spi_receive()) != 0XFF) 
  { if (((uint16_t)millis() - t0) >= timeoutMillis) return FALSE;
  }
  return TRUE;
}

//------------------------------------------------------------------------------

void sd_chipSelect(uint16_t high_low) 
{ if(m_chipSelectPin<0) return;
//
  if(high_low==HIGH)
  {
    digitalWrite(m_chipSelectPin, HIGH);
    // insure MISO goes high impedance
    spi_send(0XFF); 
  }
  else
  {
//    spi_init(m_sckDivisor); 
    digitalWrite(m_chipSelectPin, LOW);
  }
}

// send command and return error code.  Return zero for OK
uint8_t sd_cardCommand(uint8_t cmd, uint32_t arg) 
{ uint8_t d[6], *pa, kk;

  pa = (uint8_t *)(&arg);
  // select card
  sd_chipSelect(LOW);

  // wait if busy
 // unused // uint16_t ret=sd_waitNotBusy(SD_WRITE_TIMEOUT);
// form message
  d[0]=cmd | 0x40;
  for(kk=1;kk<5;kk++) d[kk]=pa[4-kk];
  
#ifdef USE_SD_CRC  // add crc
  d[5] = CRC7(d, 5);
#else
  d[5]=((cmd == CMD0) ? 0X95 : 0X87);
#endif  // USE_SD_CRC

  // send message
  for (kk = 0; kk < 6; kk++) spi_send(d[kk]);

  // skip stuff byte for stop read
  if (cmd == CMD12) spi_receive();

  // wait for response
  for (kk = 0; ((m_sd_status = spi_receive()) & 0X80) && kk != 0XFF; kk++);
  return m_sd_status;
} 

//typedef long size_t;

uint16_t sd_readData(uint8_t* dst, size_t count) {
#ifdef USE_SD_CRC
  uint16_t crc;
#endif  // USE_SD_CRC
  // wait for start block token
  uint16_t t0 = millis();
  while ((m_sd_status = spi_receive()) == 0XFF) {
    if (((uint16_t)millis() - t0) > SD_READ_TIMEOUT) {
      sd_setError(SD_CARD_ERROR_READ_TIMEOUT);
      goto fail;
    }
  }
  if (m_sd_status != DATA_START_BLOCK) {
    sd_setError(SD_CARD_ERROR_READ);
    goto fail;
  }
  // transfer data
  if ((m_sd_status = spi_receiveBulk(dst, count))) {
    sd_setError(SD_CARD_ERROR_SPI_DMA);
    goto fail;
  }

#ifdef USE_SD_CRC
  // get crc
  crc = (spi_receive() << 8) | spi_receive();
  if (crc != CRC_CCITT(dst, count)) {
    sd_setError(SD_CARD_ERROR_READ_CRC);
    goto fail;
  }
#else
  // discard crc
  spi_receive();
  spi_receive();
#endif  // USE_SD_CRC

  sd_chipSelect(HIGH);
  return TRUE;

 fail:
  sd_chipSelect(HIGH);
  return FALSE;
}

// read CID or CSR register
uint16_t sd_readRegister(uint8_t cmd, void* buf) 
{
    uint8_t* dst = (uint8_t*)(buf);
  uint16_t ret=sd_cardCommand(cmd, 0);
    if (ret) 
  {   sd_setError(SD_CARD_ERROR_READ_REG);
    sd_chipSelect(HIGH);
    return FALSE;
    }
  ret= sd_readData(dst, 16);
  return ret;
}

  uint16_t sd_readCID(cid_t* cid) {  return sd_readRegister(CMD10, cid); }
  uint16_t sd_readCSD(csd_t* csd) {  return sd_readRegister(CMD9, csd); }

 
uint8_t sd_cardAcmd(uint8_t cmd, uint32_t arg) 
{   sd_cardCommand(CMD55, 0);
    return sd_cardCommand(cmd, arg);
}

//------------------------------------------------------------------------------
/**
 * Connect to a SD flash memory card.
 *
 *
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.  The reason for failure
 * can be determined by calling errorCode() and errorData().
 */
 uint8_t sdCommandAndResponse(uint8_t cmd, uint32_t param);
 
int sd_connect()
{ int kk;
  uint16_t t0,t1;
  t0  = (uint16_t)millis();
  uint32_t arg;
  // uint8_t ret;
  // command to go idle in SPI mode
  sd_chipSelect(LOW);

  while (sd_cardCommand(CMD0, 0) != R1_IDLE_STATE) 
  {
    t1=(uint16_t)millis();
    if ((t1- t0) > SD_INIT_TIMEOUT) {
      sd_setError(SD_CARD_ERROR_CMD0);
      goto fail;
    }
   spi_send(0XFF);
  }
  
#ifdef USE_SD_CRC
  if (sd_cardCommand(CMD59, 1) != R1_IDLE_STATE) {
    sd_setError(SD_CARD_ERROR_CMD59);
    goto fail;
  }
#endif  // USE_SD_CRC

  // check SD version
  t0  = (uint16_t)millis();
  while (1) 
  {
    if (sd_cardCommand(CMD8, 0x1AA) == (R1_ILLEGAL_COMMAND | R1_IDLE_STATE)) 
  {
      sd_setType(SD_CARD_TYPE_SD1);
      break;
    }
    for (kk = 0; kk < 4; kk++) m_sd_status = spi_receive(0);
    if (m_sd_status == 0XAA) 
  {
      sd_setType(SD_CARD_TYPE_SD2);
      break;
    }
    if (((uint16_t)millis() - t0) > SD_INIT_TIMEOUT) 
  {
      sd_setError(SD_CARD_ERROR_CMD8);
      goto fail;
    }
  }
  
  // initialize card and send host supports SDHC if SD2
  arg = (sd_getType() == SD_CARD_TYPE_SD2) ? 0X40000000 : 0;

  t0  = (uint16_t)millis();
  while (sd_cardAcmd(ACMD41, arg) != R1_READY_STATE) 
  {
    // check for timeout
  t1  = (uint16_t)millis();
    if ((t1 - t0) > SD_INIT_TIMEOUT) 
  { sd_setError(SD_CARD_ERROR_ACMD41);
      goto fail;
    }
  }
  // if SD2 read OCR register to check for SDHC card
  if (sd_getType() == SD_CARD_TYPE_SD2) {
    if (sd_cardCommand(CMD58, 0)) {
      sd_setError(SD_CARD_ERROR_CMD58);
      goto fail;
    }
    if ((spi_receive(0) & 0XC0) == 0XC0) sd_setType(SD_CARD_TYPE_SDHC);
    // Discard rest of ocr - contains allowed voltage range.
    for (kk = 0; kk < 3; kk++) spi_receive();
  }
  
  sd_chipSelect(HIGH);
  return TRUE;

  fail:
  sd_chipSelect(HIGH);
  return FALSE;
}

//------------------------------------------------------------------------------
/**
 * Determine the size of an SD flash memory card.
 *
 * \return The number of 512 byte data blocks in the card
 *         or zero if an error occurs.
 */
uint32_t sd_cardSize(void) 
{
  csd_t csd;
  uint16_t ret;
  //
  if (!(ret=sd_readCSD(&csd))) return 0;
  //
  if (csd.v1.csd_ver == 0) 
  {
    uint8_t read_bl_len = csd.v1.read_bl_len;
    uint16_t c_size = (csd.v1.c_size_high << 10)
                      | (csd.v1.c_size_mid << 2) | csd.v1.c_size_low;
    uint8_t c_size_mult = (csd.v1.c_size_mult_high << 1)
                          | csd.v1.c_size_mult_low;
    return (uint32_t)(c_size + 1) << (c_size_mult + read_bl_len - 7);
  } 
  else if (csd.v2.csd_ver == 1) 
  {
    uint32_t c_size = 0X10000L * csd.v2.c_size_high + 0X100L
                      * (uint32_t)csd.v2.c_size_mid + csd.v2.c_size_low;
    return (c_size + 1) <<9;
  } 
  else 
  {
    sd_setError(SD_CARD_ERROR_BAD_CSD);
    return 0;
  }
}

//------------------------------------------------------------------------------
/** Determine if card supports single block erase.
 *
 * \return The value one, true, is returned if single block erase is supported.
 * The value zero, false, is returned if single block erase is not supported.
 */
 
uint16_t sd_eraseSingleBlockEnable() {
  csd_t csd;
  return sd_readCSD(&csd) ? csd.v1.erase_blk_en : FALSE;
}

//------------------------------------------------------------------------------
/** Erase a range of blocks.
 *
 * \param[in] firstBlock The address of the first block in the range.
 * \param[in] lastBlock The address of the last block in the range.
 *
 * \note This function requests the SD card to do a flash erase for a
 * range of blocks.  The data on the card after an erase operation is
 * either 0 or 1, depends on the card vendor.  The card must support
 * single block erase.
 *
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
 
uint16_t sd_erase(uint32_t firstBlock, uint32_t lastBlock) 
{
  csd_t csd;
  if (!sd_readCSD(&csd)) goto fail;
  // check for single block erase
  if (!csd.v1.erase_blk_en) {
    // erase size mask
    uint8_t m = (csd.v1.sector_size_high << 1) | csd.v1.sector_size_low;
    if ((firstBlock & m) != 0 || ((lastBlock + 1) & m) != 0) {
      // error card can't erase specified area
      sd_setError(SD_CARD_ERROR_ERASE_SINGLE_BLOCK);
      goto fail;
    }
  }
  if (m_sd_type != SD_CARD_TYPE_SDHC) {
    firstBlock <<= 9;
    lastBlock <<= 9;
  }
  if (sd_cardCommand(CMD32, firstBlock)
    || sd_cardCommand(CMD33, lastBlock)
    || sd_cardCommand(CMD38, 0)) {
      sd_setError(SD_CARD_ERROR_ERASE);
      goto fail;
  }
  if (!sd_waitNotBusy(SD_ERASE_TIMEOUT)) {
    sd_setError(SD_CARD_ERROR_ERASE_TIMEOUT);
    goto fail;
  }
  sd_chipSelect(HIGH);
  return TRUE;

fail:
  sd_chipSelect(HIGH);
  return FALSE;
}

//------------------------------------------------------------------------------
/**
 * Read a 512 byte block from an SD card.
 *
 * \param[in] blockNumber Logical block to be read.
 * \param[out] dst Pointer to the location that will receive the data.

 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
 
uint16_t sd_readBlock(uint32_t blockNumber, uint8_t* dst) 
{
//  SD_TRACE("RB", blockNumber);
  // use address if not SDHC card
  if (sd_getType()!= SD_CARD_TYPE_SDHC) blockNumber <<= 9;
  if (sd_cardCommand(CMD17, blockNumber)) {
    sd_setError(SD_CARD_ERROR_CMD17);
    goto fail;
  }
  return sd_readData(dst, 512);

 fail:
  sd_chipSelect(HIGH);
  return FALSE;
}

//------------------------------------------------------------------------------
/** Read one data block in a multiple block read sequence
 *
 * \param[in] dst Pointer to the location for the data to be read.
 *
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
uint16_t sd_readData2(uint8_t *dst) 
{
  sd_chipSelect(LOW);
  return sd_readData(dst, 512);
}

//------------------------------------------------------------------------------
/** Start a read multiple blocks sequence.
 *
 * \param[in] blockNumber Address of first block in sequence.
 *
 * \note This function is used with readData() and readStop() for optimized
 * multiple block reads.  SPI chipSelect must be low for the entire sequence.
 *
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
uint16_t sd_readStart(uint32_t blockNumber) 
{
  //SD_TRACE("RS", blockNumber);
  if (sd_getType()!= SD_CARD_TYPE_SDHC) blockNumber <<= 9;
  if (sd_cardCommand(CMD18, blockNumber)) {
    sd_setError(SD_CARD_ERROR_CMD18);
    goto fail;
  }
  sd_chipSelect(HIGH);
  return TRUE;

 fail:
  sd_chipSelect(HIGH);
  return FALSE;
}

//------------------------------------------------------------------------------
/** End a read multiple blocks sequence.
 *
* \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */

uint16_t sd_readStop() 
{
  if (sd_cardCommand(CMD12, 0)) {
    sd_setError(SD_CARD_ERROR_CMD12);
    goto fail;
  }
  sd_chipSelect(HIGH);
  return TRUE;

 fail:
  sd_chipSelect(HIGH);
  return FALSE;
}

//------------------------------------------------------------------------------
// send one block of data for write block or write multiple blocks
uint16_t sd_writeData(uint8_t token, const uint8_t* src) 
{
#ifdef USE_SD_CRC
  uint16_t crc = CRC_CCITT(src, 512);
#else  // USE_SD_CRC
  uint16_t crc = 0XFFFF;
#endif  // USE_SD_CRC

  spi_send(token);
  spi_sendBulk(src, 512);
  spi_send(crc >> 8);
  spi_send(crc & 0XFF);
  m_sd_status=spi_receive();

  if ((m_sd_status & DATA_RES_MASK) != DATA_RES_ACCEPTED) {
    sd_setError(SD_CARD_ERROR_WRITE);
//  fprintf(stderr,"write error %x\r\n",m_sd_status & DATA_RES_MASK);
    goto fail;
  }
  return TRUE;

 fail:
  sd_chipSelect(HIGH);
  return FALSE;
}

//------------------------------------------------------------------------------
/**
 * Writes a 512 byte block to an SD card.
 *
 * \param[in] blockNumber Logical block to be written.
 * \param[in] src Pointer to the location of the data to be written.
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
uint16_t sd_writeBlock(uint32_t blockNumber, const uint8_t* src) 
{
  //SD_TRACE("WB", blockNumber);
  // use address if not SDHC card
  if (sd_getType() != SD_CARD_TYPE_SDHC) blockNumber <<= 9;
  if (sd_cardCommand(CMD24, blockNumber)) {
    sd_setError(SD_CARD_ERROR_CMD24);
    goto fail;
  }
  if (!sd_writeData(DATA_START_BLOCK, src)) goto fail;

#if CHECK_PROGRAMMING
  // wait for flash programming to complete
  if (!waitNotBusy(SD_WRITE_TIMEOUT)) {
    error(SD_CARD_ERROR_WRITE_TIMEOUT);
    goto fail;
  }
  // response is r2 so get and check two bytes for nonzero
  if (cardCommand(CMD13, 0) || spi.receive()) {
    error(SD_CARD_ERROR_WRITE_PROGRAMMING);
    goto fail;
  }
#endif  // CHECK_PROGRAMMING

  sd_chipSelect(HIGH);
  return TRUE;

 fail:
  sd_chipSelect(HIGH);
  return FALSE;
}

//------------------------------------------------------------------------------
/** Write one data block in a multiple block write sequence
 * \param[in] src Pointer to the location of the data to be written.
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
uint16_t sd_writeData2(const uint8_t* src) 
{
// unused // static long cnt=0;
  sd_chipSelect(LOW);
  if (!sd_waitNotBusy(SD_WRITE_TIMEOUT)) goto fail;
  if (!sd_writeData(WRITE_MULTIPLE_TOKEN, src)) goto fail;
  sd_chipSelect(HIGH);
  return TRUE;

 fail:
  sd_setError(SD_CARD_ERROR_WRITE_MULTIPLE);
  sd_chipSelect(HIGH);
  return FALSE;
}
//------------------------------------------------------------------------------
/** Start a write multiple blocks sequence.
 *
 * \param[in] blockNumber Address of first block in sequence.
 * \param[in] eraseCount The number of blocks to be pre-erased.
 *
 * \note This function is used with writeData() and writeStop()
 * for optimized multiple block writes.
 *
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
uint16_t sd_writeStart(uint32_t blockNumber, uint32_t eraseCount) 
{
  // send pre-erase count
  if(eraseCount)
  {
    if (sd_cardAcmd(ACMD23, eraseCount)) {
    sd_setError(SD_CARD_ERROR_ACMD23);
    goto fail;
    }
  }
  // use address if not SDHC card
  if (sd_getType() != SD_CARD_TYPE_SDHC) blockNumber <<= 9;
  if (sd_cardCommand(CMD25, blockNumber)) {
    sd_setError(SD_CARD_ERROR_CMD25);
    goto fail;
  }
  sd_chipSelect(HIGH);
  return TRUE;

 fail:
  sd_chipSelect(HIGH);
  return FALSE;
}

//------------------------------------------------------------------------------
/** End a write multiple blocks sequence.
 *
* \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
uint16_t sd_writeStop(void) 
{
  sd_chipSelect(LOW);
  if (!sd_waitNotBusy(SD_WRITE_TIMEOUT)) goto fail;
  spi_send(STOP_TRAN_TOKEN);
  if (!sd_waitNotBusy(SD_WRITE_TIMEOUT)) goto fail;
  sd_chipSelect(HIGH);
  return TRUE;

 fail:
  sd_setError(SD_CARD_ERROR_STOP_TRAN);
  sd_chipSelect(HIGH);
  return FALSE;
}



/*************************************** SPI ********************************************************/

#include <stdint.h>
#include <stdlib.h>

#include <stdio.h>

#include "core_pins.h"
#include "usb_serial.h"

#if defined(__MK20DX128__) || defined(__MK20DX256__) || defined(__MK64FX512__) || defined(__MK66FX1M0__)

	#if defined(__MK20DX128__) || defined(__MK20DX256__)
//		static KINETISK_SPI_t * SPI[] = {(KINETISK_SPI_t *)0x4002C000}; // does not compile with C, inly C++

		uint16_t DMAMUX_SOURCE_SPI_RX[] = {DMAMUX_SOURCE_SPI0_RX};
		uint16_t DMAMUX_SOURCE_SPI_TX[] = {DMAMUX_SOURCE_SPI0_TX};
	#elif defined(__MK64FX512__) || defined(__MK66FX1M0__)
//		static KINETISK_SPI_t * SPI[3] = {(KINETISK_SPI_t *)0x4002C000, (KINETISK_SPI_t *)0x4002D000, (KINETISK_SPI_t *)0x400AC000};

		uint16_t DMAMUX_SOURCE_SPI_RX[] = {DMAMUX_SOURCE_SPI0_RX, DMAMUX_SOURCE_SPI1_RX, DMAMUX_SOURCE_SPI2_RX};
		uint16_t DMAMUX_SOURCE_SPI_TX[] = {DMAMUX_SOURCE_SPI0_TX, DMAMUX_SOURCE_SPI1_TX, DMAMUX_SOURCE_SPI2_TX};
	#endif

	#define ISPI 0
	#if ISPI==0
		#define spi ((KINETISK_SPI_t *)0x4002C000)
	#elif ISPI==1
		#define spi ((KINETISK_SPI_t *)0x4002D000)
	#elif ISPI==1
		#define spi ((KINETISK_SPI_t *)0x400AC000)
	#endif

	//static KINETISK_SPI_t * spi = SPI[ISPI];

	#if defined(__MK20DX256__) || defined(__MK66FX1M0__)
		// following ctarTAB adapted from SPI
		typedef struct {
			uint16_t div;
			uint32_t ctar;
		} CTAR_TAB_t;

	// clock is using F_BUS
	// DBR doubling boudrate (only in master mode)
	// SCK baud rate = (fSYS/PBR) x [(1+DBR)/BR]
	// BR =  [0:15] => [2+2*[0:2] 2^(3:15)] = [2,4,6, 8,16,...]
	// PBR = [0:3]  => [2+0, 1+2*[1:3]] = [2,3,5,7]
	// DBR = [0,1]

	const CTAR_TAB_t ctarTabo[23] =
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
	#else
	#endif

	//============================ SPI ===================================
	volatile static int m_spi_cs;
	volatile static int m_spi_isAsserted=0;

	int spi_isAsserted() {return m_spi_isAsserted;}

	void spi_assertCS(int cs, uint32_t ctar)
	{ 	if(cs>0) m_spi_cs=cs; 
		m_spi_isAsserted=1;
		
		if(0) if (spi->CTAR0 != ctar)
		{
			spi->MCR = SPI_MCR_MDIS | SPI_MCR_HALT | SPI_MCR_PCSIS(0x1F);
			spi->CTAR0 = ctar;
			spi->CTAR1 = ctar | SPI_CTAR_FMSZ(8);
			spi->MCR = SPI_MCR_MSTR | SPI_MCR_PCSIS(0x1F);
		}
		digitalWriteFast(m_spi_cs, LOW);
	}
	void spi_releaseCS(void) 
	{	digitalWriteFast(m_spi_cs, HIGH); //spi_transfer(0xff);
		m_spi_isAsserted=0;
	}

	void spi_init()
	{  if(ISPI==0)
			SIM_SCGC6 |= SIM_SCGC6_SPI0;
		else if(ISPI==1)
			SIM_SCGC6 |= SIM_SCGC6_SPI1;
		else if(ISPI==2)
			SIM_SCGC3 |= SIM_SCGC3_SPI2;
	}

	void spi_configPorts(int iconf)
	{
		if(iconf==1) // PJRC Audio CS = 10
		{
			CORE_PIN14_CONFIG  = PORT_PCR_MUX(2) | PORT_PCR_SRE | PORT_PCR_DSE ; // CLK SRE slew rate enable, DSE drive strength enable 
			CORE_PIN7_CONFIG   = PORT_PCR_MUX(2) | PORT_PCR_SRE | PORT_PCR_DSE ; // MOSI
			CORE_PIN12_CONFIG  = PORT_PCR_MUX(2) | PORT_PCR_SRE | PORT_PCR_DSE;	 // MISO
		}
		else if(iconf==2) //WMXZ_Logger
		{
			CORE_PIN14_CONFIG = PORT_PCR_MUX(2) | PORT_PCR_SRE | PORT_PCR_DSE ; // CLK SRE slew rate enable, DSE drive strength enable 
			CORE_PIN7_CONFIG  = PORT_PCR_MUX(2) | PORT_PCR_SRE | PORT_PCR_DSE ;	// MOSI
			CORE_PIN8_CONFIG  = PORT_PCR_MUX(2) | PORT_PCR_SRE | PORT_PCR_DSE;	// MISO
		}
	}

	#define DO_DEBUG
	#undef DO_DEBUG
	uint32_t spi_setup(uint32_t clk)
	{	int ii;

		spi_init();
		
		//spi SETUP	
		spi->MCR =	SPI_MCR_HALT |   // stop transfer
						SPI_MCR_PCSIS(0x1F) | // set all inactive states high
						SPI_MCR_CLR_TXF | // clear TXFIFO
						SPI_MCR_CLR_RXF | // clear RXFIFO
						SPI_MCR_MSTR;

		int div=F_BUS/clk;
		for(ii=0; (ii<23) && (div > ctarTabo[ii].div); ii++)
		{
		#ifdef DO_DEBUG
			static char text[80]; 
			sprintf(text,"A: %d %d %d\n\r",div,ii,ctarTabo[ii].div);
			usb_serial_write(text,80);
		#endif
		}
		#ifdef DO_DEBUG
			static char text[80];
			sprintf(text,"B: %d %d %d\n\r",div,ii,ctarTabo[ii].div);
			usb_serial_write(text,80);
		#endif

		uint32_t ctar = ctarTabo[ii].ctar | SPI_CTAR_FMSZ(7);
	//	ctar |= SPI_CTAR_CPHA;
		spi->CTAR0 =	ctar;
		spi->CTAR1 =	ctar | SPI_CTAR_FMSZ(8);
		
		spi->MCR = 	SPI_MCR_MSTR | SPI_MCR_DIS_TXF | SPI_MCR_DIS_RXF | SPI_MCR_PCSIS(0x1F);
		//
		return ctar;
	}

	//--------------------------------- used in uSDFS (from SDFAT)-----------------------------------
	// Limit initial fifo to three entries to avoid fifo overrun
	#ifdef __MK20DX256__
	  #define SPI_INITIAL_FIFO_DEPTH 3
	#endif

	#ifdef __MK66FX1M0__
	  #define SPI_INITIAL_FIFO_DEPTH 3
	#endif

	// define some symbols that are not in mk20dx128.h
	#ifndef SPI_SR_RXCTR
		#define SPI_SR_RXCTR 0XF0
	#endif  // SPI_SR_RXCTR

	#ifndef SPI_PUSHR_CONT
		#define SPI_PUSHR_CONT 0X80000000
	#endif   // SPI_PUSHR_CONT

	#ifndef SPI_PUSHR_CTAS
		#define SPI_PUSHR_CTAS(n) (((n) & 7) << 28)
	#endif  // SPI_PUSHR_CTAS

	/** SPI receive a byte */
	uint8_t spi_receive(void)
	{
		spi->MCR |= SPI_MCR_CLR_RXF;
		spi->SR = SPI_SR_TCF;
		spi->PUSHR = 0xFF;
		while (!(spi->SR & SPI_SR_TCF)) {}
		return spi->POPR;
	}
	/** SPI send a byte */
	void spi_send(uint8_t b)
	{
		spi->MCR |= SPI_MCR_CLR_RXF;
		spi->SR = SPI_SR_TCF;
		spi->PUSHR = b;
		while (!(spi->SR & SPI_SR_TCF)) {}
	}

	/** SPI receive multiple bytes */
	uint8_t spi_receiveBulk(uint8_t* buf, size_t n) 
	{
		  // clear any data in RX FIFO
		spi->MCR = SPI_MCR_MSTR | SPI_MCR_CLR_RXF | SPI_MCR_PCSIS(0x1F);
	  
	#if SPI_USE_8BIT_FRAME
	  // initial number of bytes to push into TX FIFO
	  int nf = n < SPI_INITIAL_FIFO_DEPTH ? n : SPI_INITIAL_FIFO_DEPTH;
	  int ii;
	  for (ii = 0; ii < nf; ii++) {
		  spi->PUSHR = 0XFF;
	  }
	  // limit for pushing dummy data into TX FIFO
	  uint8_t* limit = buf + n - nf;
	  while (buf < limit) {
		while (!(spi->SR & SPI_SR_RXCTR)) {}
		spi->PUSHR = 0XFF;
		*buf++ = spi->POPR;
	  }
	  // limit for rest of RX data
	  limit += nf;
	  while (buf < limit) {
		while (!(spi->SR & SPI_SR_RXCTR)) {}
		*buf++ = spi->POPR;
	  }
	  
	#else  // SPI_USE_8BIT_FRAME
	  // use 16 bit frame to avoid TD delay between frames
	  // get one byte if n is odd
	  if (n & 1) {
		*buf++ = spi_receive();
		n--;
	  }
	  // initial number of words to push into TX FIFO
	  int nf = n/2 < SPI_INITIAL_FIFO_DEPTH ? n/2 : SPI_INITIAL_FIFO_DEPTH;
	  for (int i = 0; i < nf; i++) {
		  spi->PUSHR = SPI_PUSHR_CONT | SPI_PUSHR_CTAS(1) | 0XFFFF;
	  }
	  uint8_t* limit = buf + n - 2*nf;
	  while (buf < limit) {
		while (!(spi->SR & SPI_SR_RXCTR)) {}
		spi->PUSHR = SPI_PUSHR_CONT | SPI_PUSHR_CTAS(1) | 0XFFFF;
		uint16_t w = spi->POPR;
		*buf++ = w >> 8;
		*buf++ = w & 0XFF;
	  }
	  // limit for rest of RX data
	  limit += 2*nf;
	  while (buf < limit) {
		while (!(spi->SR & SPI_SR_RXCTR)) {}
		uint16_t w = spi->POPR;
		*buf++ = w >> 8;
		*buf++ = w & 0XFF;
	  }
	#endif  // SPI_USE_8BIT_FRAME
	  return 0;
	}

	/** SPI send multiple bytes */
	void spi_sendBulk(const uint8_t* buf , size_t n) 
	{
	  // clear any data in RX FIFO
		spi->MCR = SPI_MCR_MSTR | SPI_MCR_CLR_RXF | SPI_MCR_PCSIS(0x1F);

	#if SPI_USE_8BIT_FRAME
	  // initial number of bytes to push into TX FIFO
	  int nf = n < SPI_INITIAL_FIFO_DEPTH ? n : SPI_INITIAL_FIFO_DEPTH;
	  // limit for pushing data into TX fifo
	  const uint8_t* limit = buf + n;
	  int ii;
	  for (ii = 0; ii < nf; ii++) {
		  spi->PUSHR = *buf++;
	  }
	  // write data to TX FIFO
	  while (buf < limit) {
		while (!(spi->SR & SPI_SR_RXCTR)) {}
		spi->PUSHR = *buf++;
		spi->POPR;
	  }
	  // wait for data to be sent
	  while (nf) {
		while (!(spi->SR & SPI_SR_RXCTR)) {}
		spi->POPR;
		nf--;
	  }

	#else  // SPI_USE_8BIT_FRAME
	  // use 16 bit frame to avoid TD delay between frames
	  // send one byte if n is odd
	  if (n & 1) {
		spi_send(*buf++);
		n--;
	  }
	  // initial number of words to push into TX FIFO
	  int nf = n/2 < SPI_INITIAL_FIFO_DEPTH ? n/2 : SPI_INITIAL_FIFO_DEPTH;
	  // limit for pushing data into TX fifo
	  const uint8_t* limit = buf + n;
	  int ii;
	  for (ii = 0; ii < nf; ii++) {
		uint16_t w = (*buf++) << 8;
		w |= *buf++;
		spi->PUSHR = SPI_PUSHR_CONT | SPI_PUSHR_CTAS(1) | w;
	  }
	  // write data to TX FIFO
	  while (buf < limit) {
		uint16_t w = *buf++ << 8;
		w |= *buf++;
		while (!(spi->SR & SPI_SR_RXCTR)) {}
		spi->PUSHR = SPI_PUSHR_CONT | SPI_PUSHR_CTAS(1) | w;
		spi->POPR;
	  }
	  // wait for data to be sent
	  while (nf) {
		while (!(spi->SR & SPI_SR_RXCTR)) {}
		spi->POPR;
		nf--;
	  }
	#endif  // SPI_USE_8BIT_FRAME
	}

#elif defined(__IMXRT1052__) || defined(__IMXRT1062__)

	#ifndef LSBFIRST
	#define LSBFIRST 0
	#endif
	#ifndef MSBFIRST
	#define MSBFIRST 1
	#endif

	#define SPI_MODE0 0x00
	#define SPI_MODE1 0x04
	#define SPI_MODE2 0x08
	#define SPI_MODE3 0x0C
	
	void spi_init(void) ;
	void spi_begin(uint32_t clck, uint8_t bitOrder, uint8_t dataMode);
	uint8_t spi_transfer8(uint8_t data) ;
	uint16_t spi_transfer16(uint16_t data) ;
	void spi_transfer(const void * buf, void * retbuf, size_t count);
	
	void spi_configPorts(int iconf) 
	{ return;
	}

	uint32_t spi_setup(uint32_t clk)
	{	spi_init() ;
		spi_begin(clk, MSBFIRST, SPI_MODE0);
		return 0;
	}

	uint8_t spi_receive(void)
	{ return spi_transfer8(0xff) ;
	}

	void spi_send(uint8_t b) 
	{ 	spi_transfer8(b);
		return;
	}

	uint8_t spi_receiveBulk(uint8_t* retbuf, size_t count) 
	{ 	spi_transfer((const void *) 0, (void *) retbuf, count);
		return 0;
	}

	void spi_sendBulk(const uint8_t* buf , size_t count) 
	{ 	spi_transfer((const void *) buf, (void *) 0, count);
		return;
	}

/* does not compile with C ony with C++
	static IMXRT_LPSPI_t *SPIX[]= {( IMXRT_LPSPI_t*)0x40394000, 
									( IMXRT_LPSPI_t*)0x40398000, 
									( IMXRT_LPSPI_t*)0x4039C000, 
									( IMXRT_LPSPI_t*)0x403A0000};
*/
	#define ISPI 3
	#if ISPI==2
		#define spi (( IMXRT_LPSPI_t*)0x4039C000)
	#elif ISPI==3
		#define spi (( IMXRT_LPSPI_t*)0x403A0000)
	#endif
//	static IMXRT_LPSPI_t * spi = (const IMXRT_LPSPI_t *)SPIX[ISPI];

	#define CCM_CCGR1_LPSPIx(m,n)     ((uint32_t)(((n) & 0x03) << (2*m)))
	
	void spi_init(void) 
	{ 
	  // CBCMR[LPSPI_CLK_SEL] - PLL2 = 528 MHz 
	  // CBCMR[LPSPI_PODF] - div4 = 132 MHz 

	  CCM_CCGR1 &= ~CCM_CCGR1_LPSPIx(ISPI,CCM_CCGR_ON); 

	  CCM_CBCMR = (CCM_CBCMR & ~(CCM_CBCMR_LPSPI_PODF_MASK | CCM_CBCMR_LPSPI_CLK_SEL_MASK)) | 
								 CCM_CBCMR_LPSPI_PODF(6) | CCM_CBCMR_LPSPI_CLK_SEL(2); // pg 714 

	  uint32_t fastio = IOMUXC_PAD_SRE | IOMUXC_PAD_DSE(3) | IOMUXC_PAD_SPEED(3); 
	  IOMUXC_SW_PAD_CTL_PAD_GPIO_B0_01 = fastio; 
	  IOMUXC_SW_PAD_CTL_PAD_GPIO_B0_02 = fastio; 
	  IOMUXC_SW_PAD_CTL_PAD_GPIO_B0_03 = fastio; 

	  CCM_CCGR1 |= CCM_CCGR1_LPSPIx(ISPI, CCM_CCGR_ON); 
	  
	  IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_01 = 3 | 0x10; // SDI  //Pin12
	  IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_02 = 3 | 0x10; // SDO  //Pin11
	  IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_03 = 3 | 0x10; // SCK  //Pin13

	  spi->CR = LPSPI_CR_RST; 
	} 

	void spi_begin(uint32_t clck, uint8_t bitOrder, uint8_t dataMode)
	{
		const uint32_t clk_sel[4] = {664615384,  // PLL3 PFD1
				   720000000,  // PLL3 PFD0
				   528000000,  // PLL2
				   396000000}; // PLL2 PFD2       
		uint32_t cbcmr = CCM_CBCMR;
		uint32_t clkhz = clk_sel[(cbcmr >> 4) & 0x03] / (((cbcmr >> 26 ) & 0x07 ) + 1);  // LPSPI peripheral clock
		
		uint32_t d, div;    
		if (clck == 0) clck =1;
		d= clkhz/clck;
		if (d && clkhz/d > clck) d++;
		if (d > 257) d= 257;  // max div
		if (d > 2) {
		  div = d-2;
		} else {
		  div =0;
		}
		uint32_t ccr = LPSPI_CCR_SCKDIV(div) | LPSPI_CCR_DBT(div/2);
		uint32_t tcr = LPSPI_TCR_FRAMESZ(7);    // TCR has polarity and bit order too

		// handle LSB setup 
		if (bitOrder == LSBFIRST) tcr |= LPSPI_TCR_LSBF;

		// Handle Data Mode
		if (dataMode & 0x08) tcr |= LPSPI_TCR_CPOL;

		// PCS to SCK Delay Prescaler into the After SCK Delay Prescaler  
		if (dataMode & 0x04) tcr |= LPSPI_TCR_CPHA; 


	  spi->CR = 0; 
	  spi->CFGR1 = LPSPI_CFGR1_MASTER | LPSPI_CFGR1_SAMPLE; 
	//  spi->CCR = LPSPI_CCR_SCKDIV(4); 
	//  spi->TCR = LPSPI_TCR_FRAMESZ(7); 
	  spi->CCR = ccr; 
	  spi->TCR = tcr; 
	  spi->CR = LPSPI_CR_MEN; 
	}

	uint8_t spi_transfer8(uint8_t data) 
	{ 
	  // TODO: check for space in fifo? 
	  spi->TDR = data; 
	  while (1) 
	  { 
		uint32_t fifo = (spi->FSR >> 16) & 0x1F; 
		if (fifo > 0) return spi->RDR; 
	  } 
	  //spi->SR = SPI_SR_TCF; 
	  //spi->PUSHR = data; 
	  //while (!(spi->SR & SPI_SR_TCF)) ; // wait 
	  //return spi->POPR; 
	} 

	void spi_transfer(const void * buf, void * retbuf, size_t count)
	{

	  if (count == 0) return;
		uint8_t *p_write = (uint8_t*)buf;
		uint8_t *p_read = (uint8_t*)retbuf;
		size_t count_read = count;

	  // Pass 1 keep it simple and don't try packing 8 bits into 16 yet..
	  // Lets clear the reader queue
	  spi->CR = LPSPI_CR_RRF | LPSPI_CR_MEN;  // clear the queue and make sure still enabled. 

	  while (count > 0) {
		// Push out the next byte; 
		spi->TDR = p_write? *p_write++ : 0xff;
		count--; // how many bytes left to output.
		// Make sure queue is not full before pushing next byte out
		do {
		  if ((spi->RSR & LPSPI_RSR_RXEMPTY) == 0)  {
			uint8_t b = spi->RDR;  // Read any pending RX bytes in
			if (p_read) *p_read++ = b; 
			count_read--;
		  }
		} while ((spi->SR & LPSPI_SR_TDF) == 0) ;

	  }

	  // now lets wait for all of the read bytes to be returned...
	  while (count_read) {
		if ((spi->RSR & LPSPI_RSR_RXEMPTY) == 0)  {
		  uint8_t b = spi->RDR;  // Read any pending RX bytes in
		  if (p_read) *p_read++ = b; 
		  count_read--;
		}
	  }
	}

#else // keep following for compiler
 
	void spi_configPorts(int iconf) {return;}
	uint32_t spi_setup(int div){ return 0;}
	uint8_t spi_receive( return 0;)
	void spi_send(uint8_t b) {return;}
	uint8_t spi_receiveBulk(uint8_t* buf, size_t n) {return 0;}
	void spi_sendBulk(const uint8_t* buf , size_t n) {return;}


#endif
#endif
