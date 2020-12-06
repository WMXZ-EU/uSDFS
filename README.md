# uSDFS
uSD File system based on ELM-CHaN generic FAT system

The Teensy library contains a port of ELM_CHaN's generic FAT file system for the PJRC Teensy 3.5/6 and Teensy 4 MCU.

# Examples
* uSDFS_test: basic test of uSD File system
* logger_RawWrite: testing writing to SD Cards

# History:

### - 09-aug-2020
* added example with fatfs=malloc(sizeof(FATFS)); (issue11)

### - 01-july-2019
* Version 1.1.2
	* removed compatibility issues with TimeLib library

### - 27-may-2019
* Version 1.1.1
	* added interface to GPT partitions (thanks to KurtE)

### - 07-may-2019
* Version 1.1.0
	* upgraded to ELM-CHaN's ff13c
	* refactored library structure
	* added Teensy 4 low-level interface
	* added MSC (USB disk) interface
		* uses UHSHost_t36 (Teensy build-in library)
		* needs MSC library (https://forum.pjrc.com/threads/55821-USBHost_t36-USB-Mass-Storage-Driver-Experiments) 

### - 12-mar-2018
* Version 1.0.6
	* upgraded to ELM-CHaN's ff13a
		
### - 03-may-2017
* Version 1.0.5
	* upgraded to ELM-CHaN's ff12c
		
### - 29-oct-2106
* work in progress to stabilize use of multi-device settings (e.g. spi and sdhc)

### - 26-oct-2016
* Version 1.0.4
	* added multi record operations for SPI (significant speed increase for large buffer read/write)
	* to be done
		* consolidations of SPI calls
		* variable speed settings for SPI (similar to begin/end transactions)
		* DMA support for SPI

### - 20-oct-2016
* Version 1.0.3
	* added basic SPI support (8/16 bit FIFO)
	* User should edit src/uSDconfig.h to configure multiple uSD cards for both SPI (all teensies) and 4-Bit SDIO (T3.5/3.6 only)
	* multiple disks (mix of spi and sdio) are possible
	* to be done: 
		* multi record operations for SPI
		* variable speed settings for SPI (similar to begin/end transactions)
		* DMA support for SPI

### - 21-sept-2016
* -fshort-wchar  addition in boards.txt is NOT neeeded any more
* changing _T(x) macro to #define _T(x) u ## x in ff.h does the job  (u compiler directive indicate 16 bit)

### - 12-sept-2016
* Version 1.0.2
	* upgraded to ELM-CHaN's ff12b
	* cleaned sdio.c
	* added cmd6 switch for high speed sdio
	* needs -fshort-wchar  in boards.txt added to teensy36.build.flags.common for exFAT operation
	
### - 09-aug-2016
* symbols aligned with Teensyduino_1.29

### - 25-jul-2016
* upgraded to CHaN's ff12a version as of 25-July
* default configuration is now with exFAT
	* implies use of LFN
	* requires use of unicode

### - 13-jul-2016
* corrected unicode char width (is 32 bit not 16 bit on FRDM K64, not sure for Teensy 3.6)

### - 08-july-2016
* Working release
	
### - 30-jun-2016
* First release V1.0.0
	* multi-buffer operation fixed
	
### - 27-jun-2016
* work-around to get f_read/f_write working with buffer sizes >= 1024 bytes
    
### - 26-jun-2016: 
* Initial upload (K66 4-Bit SDIO)
* No speed optimization in FAT system
