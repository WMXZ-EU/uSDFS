# uSDFS
uSD File system based on ELM-CHaN generic FAT system

The Teensy library contains a port of ELM_CHaN's generic FAT file system for the PJRC Teensy 3.6 K66 MCU 

##History:
###- 26-jun-2016: 
    *Initial upload (K66 4-Bit SDIO)
    *No speed optimization in FAT system

	###- 27-jun-2016
    *work-around to get f_read/f_write working with buffer sizes >= 1024 bytes
    
###- 30-jun-2016
	*First release V1.0.0
		** multi-buffer operation fixed
	
###- 08-july-2016
	*Working release
	
###- 13-jul-2016
	*corrected unicode char width (is 32 bit not 16 bit on FRDM K64, not sure for Teensy 3.6)

###- 25-jul-2016
	*upgraded to CHaN's ff12a version as of 25-July
	*default configuration is now with exFAT
		** implies use of LFN
		** requires use of unicode

###- 09-aug-2016
	* symbols aligned with Teensyduino_1.29

###- 12-sept-2016
	* Version 1.0.2
	* upgraded to ELM-CHaN's ff12b
	* cleaned sdio.c
	* added cmd6 switch for high speed sdio
	* needs -fshort-wchar  in boards.txt added to teensy36.build.flags.common for exFAT operation
	
###- 21-sept-2016
	* -fshort-wchar  addition in boards.txt is NOT neeeded any more
	* changing _T(x) macro to #define _T(x) u ## x in ff.h does the job  (u compiler directive indicate 16 bit)

###- 20-oct-2016
	* Version 1.0.3
	* added basic SPI support (8/16 bit FIFO)
	* User should edit src/uSDconfig.h to configure multiple uSD cards for both SPI (all teensies) and 4-Bit SDIO (T3.5/3.6 only)
	* multiple disks (mix of spi and sdio) are possible
	* do be done: 
		** multi record operations for SPI
		** variable speed settings for SPI (similar to begin/end transactions)
		** DMA support for SPI

###- 26-oct-2016
	* Version 1.0.4
	* added multi record operations for SPI (significant speed increase for large buffer read/write)
	* to be done
		** consolidations of SPI calls
		** variable speed settings for SPI (similar to begin/end transactions)
		** DMA support for SPI

### - 29-oct-2106
	* work in progress to stabilize use of multi-device settings (e.g. spi and sdhc)

		
