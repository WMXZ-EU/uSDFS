# uSDFS
uSD File system based on ELM-SCaN generic FAT system

The Teensy library contains a port of ELM_CHaN's generic FAT file system for the K66 MCU 

CAVEAT: due to a bug of K66 SDIO handling file size should be an odd multiple of 512 (1*512, 3*512, 5*512, ....)


History:
- 26-jun-2016: 

    Initial upload (K66 4-Bit SDIO)

    No speed optimization in FAT system


