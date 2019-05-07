
#ifndef _SD_CONIG_H
#define _SD_CONIG_H

#ifdef __cplusplus
extern "C" {
#endif

/* Definitions of physical drive number for each drive */
#define DEV_SPI   0 /* Example: Map SPI card to physical drive 0 */
#define DEV_SDHC  1 /* Example: Map SDHC card to physical drive 1 */
#define DEV_MSC   2 /* Example: Map MSC card (USB disk) to physical drive 2 */

// SPI
#define CS_PIN 10

#define DMA_TX 0
#define DMA_RX 1
#define DMA_PRIO 6

// use 16-bit frame if SPI_USE_8BIT_FRAME is zero
#define SPI_USE_8BIT_FRAME 1

#define USE_SD_CRC
#define USE_SD_CCITT 2
#define CHECK_PROGRAMMING 0

//SDIO
#define SDHC_USE_ISR	                    1	// must always use Interrupts (needed for CMD6)

#define SDHC_DO4BITS                        1	// use 4 bit bus
//#define SDHC_TRANSFERTYPE                   SDHC_TRANSFERTYPE_SWPOLL
#define SDHC_TRANSFERTYPE                   SDHC_TRANSFERTYPE_DMA // not implemented yet


#ifdef __cplusplus
}
#endif

#endif
