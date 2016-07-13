# uSDFS
uSD File system based on ELM-CHaN generic FAT system

The Teensy library contains a port of ELM_CHaN's generic FAT file system for the K66 MCU 

##History:
###- 26-jun-2016: 
    *Initial upload (K66 4-Bit SDIO)
    *No speed optimization in FAT system
###- 27-jun-2016
    *work-around to get f_read/f_write working with buffer sizes >= 1024 bytes
    
###- 30-jun-2016
	*First release V1.0.0
		*multi-buffer operation fixed
	
###- 13-jul-2016
	*corrected unicode char width (is 32 bit not 16 bit)


