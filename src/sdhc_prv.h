
#ifndef _SDHC_PRV_H
#define _SDHC_PRV_H
/******************************************************************************
* Includes
******************************************************************************/

/******************************************************************************
* Constants
******************************************************************************/
#define IO_SDHC_ATTRIBS (IO_DEV_ATTR_READ | IO_DEV_ATTR_REMOVE | IO_DEV_ATTR_SEEK | IO_DEV_ATTR_WRITE | IO_DEV_ATTR_BLOCK_MODE)

#define SDHC_XFERTYP_RSPTYP_NO              (0x00)
#define SDHC_XFERTYP_RSPTYP_136             (0x01)
#define SDHC_XFERTYP_RSPTYP_48              (0x02)
#define SDHC_XFERTYP_RSPTYP_48BUSY          (0x03)

#define SDHC_XFERTYP_CMDTYP_ABORT           (0x03)

#define SDHC_PROCTL_EMODE_INVARIANT         (0x02)

#define SDHC_PROCTL_DTW_1BIT                (0x00)
#define SDHC_PROCTL_DTW_4BIT                (0x01)
#define SDHC_PROCTL_DTW_8BIT                (0x10)


#define SDHC_INITIALIZATION_MAX_CNT 100000

/******************************************************************************
* Macros 
******************************************************************************/


/******************************************************************************
* Types
******************************************************************************/



/******************************************************************************
* Private variables
******************************************************************************/
uint32_t m_sdhc_baudrate;
uint32_t m_sdhc_dma_status;

/******************************************************************************
* Private functions
******************************************************************************/

static void SDHC_InitGPIO(Word init);
static DRESULT SDHC_SetBaudrate(uint32_t kbaudrate);
static LWord SDHC_WaitStatus(LWord mask);

static DRESULT SDHC_ReadBlock(LWord* pData);
static DRESULT SDHC_WriteBlock(const LWord* pData);


static DRESULT SDHC_CMD_Do(LWord xfertyp);
static DRESULT SDHC_CMD0_GoToIdle(void);
static DRESULT SDHC_CMD2_Identify(void);
static DRESULT SDHC_CMD3_GetAddress(void);
static DRESULT SDHC_ACMD6_SetBusWidth(LWord address, LWord width);
static DRESULT SDHC_CMD7_SelectCard(LWord address);
static DRESULT SDHC_CMD8_SetInterface(LWord cond);
static DRESULT SDHC_CMD9_GetParameters(LWord address);
static DRESULT SDHC_CMD12_StopTransfer(void);
static DRESULT SDHC_CMD12_StopTransferWaitForBusy(void);
static DRESULT SDHC_CMD13_SendStatus(void);
static DRESULT SDHC_CMD16_SetBlockSize(LWord block_size);
static DRESULT SDHC_CMD17_ReadBlock(LWord sector);
static DRESULT SDHC_CMD18_ReadBlocks(LWord sector, LWord count);
static DRESULT SDHC_CMD24_WriteBlock(LWord sector);
static DRESULT SDHC_CMD25_WriteBlocks(LWord sector, LWord count);
static DRESULT SDHC_ACMD41_SendOperationCond(LWord cond);


#endif
