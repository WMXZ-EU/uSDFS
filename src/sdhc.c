// following code is modified by Walter Zimmer from
// from version provided by
// Petr Gargulak (NXP Employee) 
//https://community.nxp.com/servlet/JiveServlet/download/339474-1-263510/SDHC_K60_Baremetal.ZIP
//see also
//https://community.nxp.com/thread/99202


#include "kinetis.h"
#include "localKinetis.h"

#include "sdhc.h"
#include "sdhc_prv.h"

/* some aux functions for pure c code */
#include "usb_serial.h"
void logg(char c) {usb_serial_putchar(c); usb_serial_flush_output();}
void printb(uint32_t x)
{ char c;
  int ii;
  for(ii=31;ii>=0;ii--)
  { if(!((ii+1)%4))usb_serial_putchar(' ');
    c=(x&1<<ii)?'1':'0'; usb_serial_putchar(c);
  }
  usb_serial_putchar('\r');
  usb_serial_putchar('\n');
  usb_serial_flush_output();
}
/* end aux functions */

SD_CARD_DESCRIPTOR sdCardDesc;

/******************************************************************************
*
*   Public functions
*
******************************************************************************/
//
//-----------------------------------------------------------------------------
// FUNCTION:    SDHC_Baudrate
// SCOPE:       SDHC Controller public related function
// DESCRIPTION: Function to return baudrate
//              
// PARAMETERS:  none
//              
// RETURNS:     baudrate
//-----------------------------------------------------------------------------  
uint32_t SDHC_Baudrate(void)
{ return m_sdhc_baudrate;
}

//-----------------------------------------------------------------------------
// FUNCTION:    SDHC_GetDMAStatus
// SCOPE:       SDHC Controller public related function
// DESCRIPTION: Function to return dma status
//              
// PARAMETERS:  none
//              
// RETURNS:     dma status (0 not finished, 1 finished, -1 error)
//-----------------------------------------------------------------------------  
uint32_t SDHC_GetDMAStatus(void)
{ return m_sdhc_dma_status;
}

//-----------------------------------------------------------------------------
// FUNCTION:    SDHC_ClearDMAStatus
// SCOPE:       SDHC Controller public related function
// DESCRIPTION: Function to clear dma status
//              
// PARAMETERS:  none
//              
// RETURNS:     nil
//-----------------------------------------------------------------------------  
void SDHC_ClearDMAStatus(void)
{ m_sdhc_dma_status = 0;
}

//-----------------------------------------------------------------------------
// FUNCTION:    SDHC_Init
// SCOPE:       SDHC Controller public related function
// DESCRIPTION: Function initialize the SDHC Controller
//              
// PARAMETERS:  none
//              
// RETURNS:     status of initialization(OK, nonInit, noCard, CardProtected)
//-----------------------------------------------------------------------------  
DSTATUS SDHC_Init(void)
{
    // Enable clock to SDHC peripheral
    SIM_SCGC3 |= SIM_SCGC3_SDHC;
    
    // Enable clock to PORT E peripheral (all SDHC BUS signals)
    SIM_SCGC5 |= SIM_SCGC5_PORTE;
    
    SIM_SCGC6 |= SIM_SCGC6_DMAMUX;
    SIM_SCGC7 |= SIM_SCGC7_DMA;
    
    // Switch of MPU unit (maybe bug of silicon)
    MPU_CESR &= ~MPU_CESR_VLD_MASK;
    
    // De-init GPIO - to prevent unwanted clocks on bus
    SDHC_InitGPIO(0);
    
    /* Reset SDHC */
    SDHC_SYSCTL = SDHC_SYSCTL_RSTA_MASK | SDHC_SYSCTL_SDCLKFS(0x80);
    while (SDHC_SYSCTL & SDHC_SYSCTL_RSTA_MASK)
    { };
    
    /* Initial values */ // to do - Check values
    SDHC_VENDOR = 0;
    SDHC_BLKATTR = SDHC_BLKATTR_BLKCNT(1) | SDHC_BLKATTR_BLKSIZE(SDHC_BLOCK_SIZE);
    SDHC_PROCTL = SDHC_PROCTL_EMODE(SDHC_PROCTL_EMODE_INVARIANT) | SDHC_PROCTL_D3CD_MASK; 
    SDHC_WML = SDHC_WML_RDWML(SDHC_FIFO_BUFFER_SIZE) | SDHC_WML_WRWML(SDHC_FIFO_BUFFER_SIZE);

    /* Set the SDHC initial baud rate divider and start */
    SDHC_SetBaudrate(400);

    /* Poll inhibit bits */
    while (SDHC_PRSSTAT & (SDHC_PRSSTAT_CIHB_MASK | SDHC_PRSSTAT_CDIHB_MASK)) { };

    /* Init GPIO again */
    SDHC_InitGPIO(0xFFFF);
    
    /* Enable requests */
    SDHC_IRQSTAT = 0xFFFF;
    SDHC_IRQSTATEN = SDHC_IRQSTATEN_DMAESEN_MASK | SDHC_IRQSTATEN_AC12ESEN_MASK | SDHC_IRQSTATEN_DEBESEN_MASK | SDHC_IRQSTATEN_DCESEN_MASK | SDHC_IRQSTATEN_DTOESEN_MASK 
                         | SDHC_IRQSTATEN_CIESEN_MASK | SDHC_IRQSTATEN_CEBESEN_MASK | SDHC_IRQSTATEN_CCESEN_MASK | SDHC_IRQSTATEN_CTOESEN_MASK 
                         | SDHC_IRQSTATEN_BRRSEN_MASK | SDHC_IRQSTATEN_BWRSEN_MASK | SDHC_IRQSTATEN_DINTSEN_MASK | SDHC_IRQSTATEN_CRMSEN_MASK
                         | SDHC_IRQSTATEN_TCSEN_MASK | SDHC_IRQSTATEN_CCSEN_MASK;
    
    
  #if SDHC_USE_ISR
    SDHC_IRQSIGEN = SDHC_IRQSIGEN_DINTIEN_MASK;
    NVIC_ENABLE_IRQ(IRQ_SDHC);
  #endif

    /* 80 initial clocks */
    SDHC_SYSCTL |= SDHC_SYSCTL_INITA_MASK;
    while (SDHC_SYSCTL & SDHC_SYSCTL_INITA_MASK) { };

    // to do - check if this needed
    SDHC_IRQSTAT |= SDHC_IRQSTAT_CRM_MASK;
    
    // Check card
    if (SDHC_PRSSTAT & SDHC_PRSSTAT_CINS_MASK)
      return 0;
    else
      return STA_NODISK;
}

//-----------------------------------------------------------------------------
// FUNCTION:    SDHC_GetStatus
// SCOPE:       SDHC Controller public related function
// DESCRIPTION: Function returns status of card
//              
// PARAMETERS:  none
//              
// RETURNS:     status of the card
//-----------------------------------------------------------------------------  
DSTATUS SDHC_GetStatus(void)
{
  return sdCardDesc.status;
}

//-----------------------------------------------------------------------------
// FUNCTION:    SDHC_GetBlockCnt
// SCOPE:       SDHC Controller public related function
// DESCRIPTION: Function returns count of blocks
//              
// PARAMETERS:  none
//              
// RETURNS:     count of blocks
//-----------------------------------------------------------------------------  
LWord SDHC_GetBlockCnt(void)
{
  if(sdCardDesc.status)
    return 0;
  
  return sdCardDesc.numBlocks;
}

//-----------------------------------------------------------------------------
// FUNCTION:    SDHC_ISR
// SCOPE:       SDHC Controller public related function
// DESCRIPTION: Function provides SDHC interrupt service routine
//              
// PARAMETERS:  none
//              
// RETURNS:     nil
//-----------------------------------------------------------------------------  
void sdhc_isr(void)
{
//  printb(SDHC_IRQSTAT);
  if (SDHC_WaitStatus(SDHC_IRQSTAT_DMAE_MASK | SDHC_IRQSTAT_AC12E_MASK | SDHC_IRQSTAT_DEBE_MASK | SDHC_IRQSTAT_DCE_MASK | SDHC_IRQSTAT_DTOE_MASK | SDHC_IRQSTAT_DINT_MASK | SDHC_IRQSTAT_TC_MASK) & (SDHC_IRQSTAT_DINT_MASK | SDHC_IRQSTAT_TC_MASK))
  { 
    SDHC_IRQSTAT |= SDHC_IRQSTAT_DINT_MASK | SDHC_IRQSTAT_TC_MASK;
    m_sdhc_dma_status=1;
  }
  
  {
      SDHC_IRQSTAT |= SDHC_IRQSTAT_DMAE_MASK | SDHC_IRQSTAT_AC12E_MASK | SDHC_IRQSTAT_DEBE_MASK | SDHC_IRQSTAT_DCE_MASK | SDHC_IRQSTAT_DTOE_MASK | SDHC_IRQSTAT_DINT_MASK;
      (void)SDHC_CMD12_StopTransferWaitForBusy();
      m_sdhc_dma_status=-1;
  }
}

//-----------------------------------------------------------------------------
// FUNCTION:    SDHC_InitCard
// SCOPE:       SDHC Controller public related function
// DESCRIPTION: Function initialize the SDHC Controller and SD Card
//              
// PARAMETERS:  none
//              
// RETURNS:     status of initialization(OK, nonInit, noCard, CardProtected)
//-----------------------------------------------------------------------------  
DSTATUS SDHC_InitCard(uint32_t kbaudrate)
{
  DSTATUS resS;
  DRESULT resR;
  LWord i;
  
  resS = SDHC_Init();
  
  sdCardDesc.status = resS;
  sdCardDesc.address = 0;
  sdCardDesc.highCapacity = 0;
  sdCardDesc.version2 = 0;
  sdCardDesc.numBlocks = 0;
  
  if(resS)
    return resS;
  
  resR = SDHC_CMD0_GoToIdle();
  if(resR)
  {
    sdCardDesc.status = STA_NOINIT; 
    return STA_NOINIT;
  }

  resR = SDHC_CMD8_SetInterface(0x000001AA); // 3.3V and AA check pattern
  if (resR > 0)
  {
    sdCardDesc.status = STA_NOINIT; 
    return STA_NOINIT;
  }
 
  if (resR == 0)
  {
      if (SDHC_CMDRSP0 != 0x000001AA)
      {
        sdCardDesc.status = STA_NOINIT; 
        return STA_NOINIT;
      }
      
      sdCardDesc.highCapacity = 1;              
  }

  if(SDHC_ACMD41_SendOperationCond(0))
  {
    sdCardDesc.status = STA_NOINIT; 
    return STA_NOINIT;
  }

  if (SDHC_CMDRSP0 & 0x300000)
  {
    LWord condition = 0x00300000;
    if (sdCardDesc.highCapacity)
            condition |= 0x40000000;
    i = 0;
    do 
    {
        i++;
        if(SDHC_ACMD41_SendOperationCond(condition))
        {  
          resS = STA_NOINIT;
          break;
        }
        
    }while ((0 == (SDHC_CMDRSP0 & 0x80000000)) && (i < SDHC_INITIALIZATION_MAX_CNT));
   
    
    if (resS)
      return resS;
    
    if ((i >= SDHC_INITIALIZATION_MAX_CNT) || (!(SDHC_CMDRSP0 & 0x40000000)))
        sdCardDesc.highCapacity = 0; 
  }
  
  // Card identify
  if(SDHC_CMD2_Identify())
  {
    sdCardDesc.status = STA_NOINIT; 
    return STA_NOINIT;
  }
  
  // Get card address
  if(SDHC_CMD3_GetAddress())
  {
    sdCardDesc.status = STA_NOINIT; 
    return STA_NOINIT;
  }
 
  sdCardDesc.address = SDHC_CMDRSP0 & 0xFFFF0000;
  
  // Get card parameters 
  if(SDHC_CMD9_GetParameters(sdCardDesc.address))
  {
    sdCardDesc.status = STA_NOINIT; 
    return STA_NOINIT;
  }
  
  if (0 == (SDHC_CMDRSP3 & 0x00C00000))
  {
    LWord read_bl_len, c_size, c_size_mult;
    
    read_bl_len = (SDHC_CMDRSP2 >> 8) & 0x0F;
    c_size = SDHC_CMDRSP2 & 0x03;
    c_size = (c_size << 10) | (SDHC_CMDRSP1 >> 22);
    c_size_mult = (SDHC_CMDRSP1 >> 7) & 0x07;
    sdCardDesc.numBlocks = (c_size + 1) * (1 << (c_size_mult + 2)) * (1 << (read_bl_len - 9));
  }
  else
  {
    LWord c_size;
    
    sdCardDesc.version2 = 1;
    c_size = (SDHC_CMDRSP1 >> 8) & 0x003FFFFF;
    sdCardDesc.numBlocks = (c_size + 1) << 10;
  }
  
  // Select card
  if(SDHC_CMD7_SelectCard(sdCardDesc.address))
  {
    sdCardDesc.status = STA_NOINIT; 
    return STA_NOINIT;
  }
  
  // Set Block Size to 512
  // Block Size in SDHC Controller is already set to 512 by SDHC_Init();
  // Set 512 Block size in SD card
  if(SDHC_CMD16_SetBlockSize(SDHC_BLOCK_SIZE))
  {
    sdCardDesc.status = STA_NOINIT; 
    return STA_NOINIT;
  }

  if(SDHC_DO4BITS)
  {
    // Set 4 bit data bus width
    if(SDHC_ACMD6_SetBusWidth(sdCardDesc.address, 2))
    {
      sdCardDesc.status = STA_NOINIT; 
      return STA_NOINIT;
    }
    
    // Set Data bus width also in SDHC controller
    SDHC_PROCTL &= (~ SDHC_PROCTL_DTW_MASK);
    SDHC_PROCTL |= SDHC_PROCTL_DTW(SDHC_PROCTL_DTW_4BIT);
  }  
  
  
  // De-Init GPIO
  SDHC_InitGPIO(0);

  // Set the SDHC default baud rate
  SDHC_SetBaudrate(kbaudrate);

  // Init GPIO
  SDHC_InitGPIO(0xFFFF);

  
  return sdCardDesc.status;
}

//-----------------------------------------------------------------------------
// FUNCTION:    disk_read
// SCOPE:       SDHC public related function
// DESCRIPTION: Function read block/blocks from disk
//              
// PARAMETERS:  buff - pointer on buffer where read data should be stored
//              sector - index of start sector
//              count - count of sector to read
//              
// RETURNS:     result of operation
//-----------------------------------------------------------------------------
DRESULT SDHC_ReadBlocks(UCHAR* buff, DWORD sector, UCHAR count)
{
  DRESULT result;
  LWord* pData = (LWord*)buff;
  
  // Check if this is ready
  if(sdCardDesc.status != 0)
     return RES_NOTRDY;
  
  // Check the valid Count of block
  if(!count)
    return RES_PARERR; 
  
  // Convert LBA to byte address if needed
  if(!sdCardDesc.highCapacity)
    sector *= 512;
  
  SDHC_IRQSTAT = 0xffff;
  
  
#if SDHC_TRANSFERTYPE == SDHC_TRANSFERTYPE_DMA
  while(SDHC_PRSSTAT & SDHC_PRSSTAT_DLA_MASK) {};
  SDHC_IRQSTAT |= SDHC_IRQSTAT_TC_MASK;
  SDHC_DSADDR  = (LWord)pData;  
	 m_sdhc_dma_status=0;
#endif  
  
  if(count == 1)
  {
    // Just single block mode is needed
    result = SDHC_CMD17_ReadBlock(sector); 
    if(result != RES_OK)
      return result;
    
    result = SDHC_ReadBlock(pData);
  }else
  {
    // Multi Block read access should be used
    result = SDHC_CMD18_ReadBlocks(sector, count); 
    if(result != RES_OK)
      return result;

//    do
//    { 
      result = SDHC_ReadBlock(pData); // simply wait for end of transfer
//      pData += (SDHC_BLOCK_SIZE / sizeof(LWord));
//      count--;
//    }while((result == RES_OK) && count);         
   
    // Auto CMD12 is enabled
    if(result != RES_OK)
      (void)SDHC_CMD12_StopTransferWaitForBusy();        
  }
  
  return result; 
}

//-----------------------------------------------------------------------------
// FUNCTION:    disk_write
// SCOPE:       SDHC public related function
// DESCRIPTION: Function write block/blocks to disk
//              
// PARAMETERS:  buff - pointer on buffer where is stored data
//              sector - index of start sector
//              count - count of sector to write
//              
// RETURNS:     result of operation
//-----------------------------------------------------------------------------
DRESULT SDHC_WriteBlocks(UCHAR* buff, DWORD sector, UCHAR count)
{
  DRESULT result;
  LWord* pData = (LWord*)buff;
  
  // Check if this is ready
  if(sdCardDesc.status != 0)
     return RES_NOTRDY;
  
  // Check the valid Count of block
  if(!count)
    return RES_PARERR; 
  
  // Convert LBA to byte address if needed
  if(!sdCardDesc.highCapacity)
    sector *= 512;

  SDHC_IRQSTAT = 0xffff;
    
#if SDHC_TRANSFERTYPE == SDHC_TRANSFERTYPE_DMA
  while(SDHC_PRSSTAT & SDHC_PRSSTAT_DLA_MASK) {};
  SDHC_IRQSTAT |= SDHC_IRQSTAT_TC_MASK;
  SDHC_DSADDR  = (LWord)pData;  
  
	 m_sdhc_dma_status=0;
#endif   

  
  if(count == 1)
  {
    // Just single block mode is needed
    result = SDHC_CMD24_WriteBlock(sector); 
    if(result != RES_OK)
      return result;
    
    result = SDHC_WriteBlock(pData);
  }else
  {
    // Multi Block write access should be used

    result = SDHC_CMD25_WriteBlocks(sector, count); 
    if(result != RES_OK)
      return result;

//    do
//    {
      result = SDHC_WriteBlock(pData);
//      pData += (SDHC_BLOCK_SIZE / sizeof(LWord));
//      count--;
//    }while((result == RES_OK) && count);         
 
    // Auto CMD12 is enabled
    if(result != RES_OK)
      (void)SDHC_CMD12_StopTransferWaitForBusy();        
  }
  
  return result;
}

/******************************************************************************
*
*   Private functions
*
******************************************************************************/

//-----------------------------------------------------------------------------
// FUNCTION:    SDHC_InitGPIO
// SCOPE:       SDHC Controller private related function
// DESCRIPTION: Function initialize the SDHC Controller GPIO signals
//              
// PARAMETERS:  mask of PCR reg
//              
// RETURNS:     none
//----------------------------------------------------------------------------- 
static void SDHC_InitGPIO(Word init)
{  
  PORTE_PCR0 = init & (PORT_PCR_MUX(4) | PORT_PCR_PS | PORT_PCR_PE | PORT_PCR_DSE);    /* SDHC.D1  */
  PORTE_PCR1 = init & (PORT_PCR_MUX(4) | PORT_PCR_PS | PORT_PCR_PE | PORT_PCR_DSE);    /* SDHC.D0  */
  PORTE_PCR2 = init & (PORT_PCR_MUX(4) | PORT_PCR_DSE);                                          /* SDHC.CLK */
  PORTE_PCR3 = init & (PORT_PCR_MUX(4) | PORT_PCR_PS | PORT_PCR_PE | PORT_PCR_DSE);    /* SDHC.CMD */
  PORTE_PCR4 = init & (PORT_PCR_MUX(4) | PORT_PCR_PS | PORT_PCR_PE | PORT_PCR_DSE);    /* SDHC.D3  */
  PORTE_PCR5 = init & (PORT_PCR_MUX(4) | PORT_PCR_PS | PORT_PCR_PE | PORT_PCR_DSE);    /* SDHC.D2  */  
}

//-----------------------------------------------------------------------------
// FUNCTION:    SDHC_SetBaudrate
// SCOPE:       SDHC Controller private related function
// DESCRIPTION: Function sets clock on SDHC Peripheral
//              
// PARAMETERS:  requested baudrate
//              
// RETURNS:     result
//----------------------------------------------------------------------------- 
static DRESULT SDHC_SetBaudrate(uint32_t kbaudrate) 
{
  LWord sysctl, time_out;
  
  // Disable SDHC clocks
  SDHC_SYSCTL &= (~ SDHC_SYSCTL_SDCLKEN_MASK);

     // get dividers from requested baud rate 
     uint32_t aux=F_CPU;
     uint32_t ii,jj;
     uint32_t baudrate=kbaudrate*1000;
     for(ii=0;ii<8;ii++)
     {  for(jj=0;jj<16;jj++)  if((aux/((1<<ii)*(jj+1))) <= baudrate) break;
        if(jj<16) break;
     }
     uint32_t minpresc=(1<<ii)>>1;
     uint32_t mindiv=jj;

//     minpresc=0;
//     mindiv=9;

  m_sdhc_baudrate=F_CPU/((1<<minpresc)*(mindiv+1));

  // Change dividers
  sysctl = SDHC_SYSCTL & (~ (SDHC_SYSCTL_DTOCV_MASK | SDHC_SYSCTL_SDCLKFS_MASK | SDHC_SYSCTL_DVS_MASK));
  SDHC_SYSCTL = sysctl | (SDHC_SYSCTL_DTOCV(0x0E) | SDHC_SYSCTL_SDCLKFS(minpresc) | SDHC_SYSCTL_DVS(mindiv));

  time_out = 0xfffff;
  
  /* Wait for stable clock */
  while ((0 == (SDHC_PRSSTAT & SDHC_PRSSTAT_SDSTB_MASK)) && time_out)
  {
    time_out--;
  };
  
  /* Enable SDHC clocks */
  SDHC_SYSCTL |= SDHC_SYSCTL_SDCLKEN_MASK;
  SDHC_IRQSTAT |= SDHC_IRQSTAT_DTOE_MASK;
  
  if(time_out)
    return RES_OK;
  else
    return RES_ERROR;
  
}

//-----------------------------------------------------------------------------
// FUNCTION:    SDHC_WaitStatus
// SCOPE:       SDHC Controller private related function
// DESCRIPTION: Function waits for status bits sets 
//              
// PARAMETERS:  requested staus bits mask
//              
// RETURNS:     result bits set
//----------------------------------------------------------------------------- 
static LWord SDHC_WaitStatus(LWord mask)
{
    LWord             result;
    LWord             timeout = -1;
    do
    {
        result = SDHC_IRQSTAT & mask;
        timeout--;
    } 
    while((0 == result) && (timeout));
    
    if(timeout)
      return result;
    
    return 0;
}   

//-----------------------------------------------------------------------------
// FUNCTION:    SDHC_ReadBlock
// SCOPE:       SDHC Controller private related function
// DESCRIPTION: Function reads one block
//              
// PARAMETERS:  pointer to buffer for readed data
//              
// RETURNS:     result
//----------------------------------------------------------------------------- 
static DRESULT SDHC_ReadBlock(LWord* pData)
{
#if SDHC_TRANSFERTYPE == SDHC_TRANSFERTYPE_SWPOLL 
  LWord i, i_max, j;
  // loop count ((block_count * block_size) / (int_size * fifo_size))
  i_max = ((1 * SDHC_BLOCK_SIZE) / (4 * SDHC_FIFO_BUFFER_SIZE));
  
  while ((SDHC_PRSSTAT & SDHC_PRSSTAT_DLA_MASK)==1){};
  
  for(i = 0; i < i_max; i++)
  {
    SDHC_IRQSTAT |= SDHC_IRQSTAT_BRR_MASK;
    
    if (SDHC_IRQSTAT & (SDHC_IRQSTAT_DEBE_MASK | SDHC_IRQSTAT_DCE_MASK | SDHC_IRQSTAT_DTOE_MASK))
    {
        SDHC_IRQSTAT |= SDHC_IRQSTAT_DEBE_MASK | SDHC_IRQSTAT_DCE_MASK | SDHC_IRQSTAT_DTOE_MASK | SDHC_IRQSTAT_BRR_MASK;
        
        (void)SDHC_CMD12_StopTransferWaitForBusy();
        return RES_ERROR;
    }
            
    while (0 == (SDHC_PRSSTAT  & SDHC_PRSSTAT_BREN_MASK))
    { };
    
    for(j=0;j<SDHC_FIFO_BUFFER_SIZE;j++)
      *pData++ = SDHC_DATPORT;
  }

  while (!(SDHC_IRQSTAT & SDHC_IRQSTAT_TC_MASK)) {                          // wait for transfer to complete
    }
  SDHC_IRQSTAT = (SDHC_IRQSTAT_TC_MASK | SDHC_IRQSTAT_BRR_MASK | SDHC_IRQSTAT_AC12E_MASK);
   return RES_OK;

//--------------------------------------------
#elif SDHC_TRANSFERTYPE == SDHC_TRANSFERTYPE_DMA
  /* Wait for response */

  #if SDHC_USE_ISR
    return RES_OK;
  #endif

  if (SDHC_WaitStatus(SDHC_IRQSTAT_DMAE_MASK | SDHC_IRQSTAT_AC12E_MASK | SDHC_IRQSTAT_DEBE_MASK | SDHC_IRQSTAT_DCE_MASK | SDHC_IRQSTAT_DTOE_MASK | SDHC_IRQSTAT_DINT_MASK | SDHC_IRQSTAT_TC_MASK) & (SDHC_IRQSTAT_DINT_MASK | SDHC_IRQSTAT_TC_MASK))
  { 
     SDHC_IRQSTAT |= SDHC_IRQSTAT_DINT_MASK | SDHC_IRQSTAT_TC_MASK;
	 m_sdhc_dma_status=1;
	 return RES_OK;
  }
  
  {
      SDHC_IRQSTAT |= SDHC_IRQSTAT_DMAE_MASK | SDHC_IRQSTAT_AC12E_MASK | SDHC_IRQSTAT_DEBE_MASK | SDHC_IRQSTAT_DCE_MASK | SDHC_IRQSTAT_DTOE_MASK | SDHC_IRQSTAT_DINT_MASK;
      (void)SDHC_CMD12_StopTransferWaitForBusy();
	 m_sdhc_dma_status=-1;
      return RES_ERROR;
  }
#endif  
}

//-----------------------------------------------------------------------------
// FUNCTION:    SDHC_WriteBlock
// SCOPE:       SDHC Controller private related function
// DESCRIPTION: Function writes one block
//              
// PARAMETERS:  pointer to buffer where are stored data
//              
// RETURNS:     result
//----------------------------------------------------------------------------- 
static DRESULT SDHC_WriteBlock(const LWord* pData)
{

#if SDHC_TRANSFERTYPE == SDHC_TRANSFERTYPE_SWPOLL 
  
  LWord i, i_max, j;
  // loop count ((block_count * block_size) / (int_size * fifo_size))
  i_max = ((1 * SDHC_BLOCK_SIZE) / (4 * SDHC_FIFO_BUFFER_SIZE));
  
  for(i = 0; i < i_max; i++)
  {
    while (0 == (SDHC_IRQSTAT & SDHC_IRQSTAT_BWR_MASK))
    { };
    
    if (SDHC_IRQSTAT & (SDHC_IRQSTAT_DEBE_MASK | SDHC_IRQSTAT_DCE_MASK | SDHC_IRQSTAT_DTOE_MASK))
    {
        SDHC_IRQSTAT |= SDHC_IRQSTAT_DEBE_MASK | SDHC_IRQSTAT_DCE_MASK | SDHC_IRQSTAT_DTOE_MASK | SDHC_IRQSTAT_BRR_MASK;
        
        (void)SDHC_CMD12_StopTransferWaitForBusy();
        return RES_ERROR;
    }
    
    for(j=0;j<SDHC_FIFO_BUFFER_SIZE;j++)
      SDHC_DATPORT = *pData++;
    
    SDHC_IRQSTAT |= SDHC_IRQSTAT_BWR_MASK;
    
    if (SDHC_IRQSTAT & (SDHC_IRQSTAT_DEBE_MASK | SDHC_IRQSTAT_DCE_MASK | SDHC_IRQSTAT_DTOE_MASK))
    {
        SDHC_IRQSTAT |= SDHC_IRQSTAT_DEBE_MASK | SDHC_IRQSTAT_DCE_MASK | SDHC_IRQSTAT_DTOE_MASK | SDHC_IRQSTAT_BRR_MASK;
        
        (void)SDHC_CMD12_StopTransferWaitForBusy();
        return RES_ERROR;
    }
            
    
  }
  return RES_OK;
 
 //--------------------------------------------
#elif SDHC_TRANSFERTYPE == SDHC_TRANSFERTYPE_DMA
  /* Wait for response */

  #if SDHC_USE_ISR
    return RES_OK;
  #endif

  if (SDHC_WaitStatus(SDHC_IRQSTAT_DMAE_MASK | SDHC_IRQSTAT_AC12E_MASK | SDHC_IRQSTAT_DEBE_MASK | SDHC_IRQSTAT_DCE_MASK | SDHC_IRQSTAT_DTOE_MASK | SDHC_IRQSTAT_DINT_MASK | SDHC_IRQSTAT_TC_MASK) & (SDHC_IRQSTAT_DINT_MASK | SDHC_IRQSTAT_TC_MASK))
  {
    SDHC_IRQSTAT |= SDHC_IRQSTAT_DINT_MASK | SDHC_IRQSTAT_TC_MASK;
	 m_sdhc_dma_status=1;
    return RES_OK;
  }
  
  {
      SDHC_IRQSTAT |= SDHC_IRQSTAT_DMAE_MASK | SDHC_IRQSTAT_AC12E_MASK | SDHC_IRQSTAT_DEBE_MASK | SDHC_IRQSTAT_DCE_MASK | SDHC_IRQSTAT_DTOE_MASK | SDHC_IRQSTAT_DINT_MASK;
      (void)SDHC_CMD12_StopTransferWaitForBusy();
	 m_sdhc_dma_status=-1;
      return RES_ERROR;
  }
#endif  
  
}

//-----------------------------------------------------------------------------
// FUNCTION:    SDHC_CMD_Do
// SCOPE:       SDHC Controller private related function
// DESCRIPTION: Function that sends the command to SDcard
//              
// PARAMETERS:  Contains of SFERTYP register
//              
// RETURNS:     result
//----------------------------------------------------------------------------- 
static DRESULT SDHC_CMD_Do(LWord xfertyp)
{

    // Card removal check preparation
    SDHC_IRQSTAT |= SDHC_IRQSTAT_CRM_MASK;

    // Wait for cmd line idle // to do timeout PRSSTAT[CDIHB] and the PRSSTAT[CIHB]
    while ((SDHC_PRSSTAT & SDHC_PRSSTAT_CIHB_MASK) || (SDHC_PRSSTAT & SDHC_PRSSTAT_CDIHB_MASK))
        { };
    
    SDHC_XFERTYP = xfertyp;

    /* Wait for response */
    if (SDHC_WaitStatus(SDHC_IRQSTAT_CIE_MASK | SDHC_IRQSTAT_CEBE_MASK | SDHC_IRQSTAT_CCE_MASK | SDHC_IRQSTAT_CC_MASK) != SDHC_IRQSTAT_CC_MASK)
    {
        SDHC_IRQSTAT |= SDHC_IRQSTAT_CTOE_MASK | SDHC_IRQSTAT_CIE_MASK | SDHC_IRQSTAT_CEBE_MASK | SDHC_IRQSTAT_CCE_MASK | SDHC_IRQSTAT_CC_MASK;
        return RES_ERROR;
    }

    /* Check card removal */
    if (SDHC_IRQSTAT & SDHC_IRQSTAT_CRM_MASK)
    {
        SDHC_IRQSTAT |= SDHC_IRQSTAT_CTOE_MASK | SDHC_IRQSTAT_CC_MASK;
        return RES_NOTRDY;
    }

    /* Get response, if available */
    if (SDHC_IRQSTAT & SDHC_IRQSTAT_CTOE_MASK)
    {
        SDHC_IRQSTAT |= SDHC_IRQSTAT_CTOE_MASK | SDHC_IRQSTAT_CC_MASK;
        return RES_NONRSPNS;
    }
    
    SDHC_IRQSTAT |= SDHC_IRQSTAT_CC_MASK;

    return RES_OK;

}

//-----------------------------------------------------------------------------
// FUNCTION:    SDHC_CMD0_GoToIdle
// SCOPE:       SDHC Controller private related function
// DESCRIPTION: Function sends CMD 0 to put SDCARD to idle
//              
// PARAMETERS:  none
//              
// RETURNS:     result
//----------------------------------------------------------------------------- 
static DRESULT SDHC_CMD0_GoToIdle(void)
{
  LWord xfertyp;
  DRESULT result;
  
  SDHC_CMDARG = 0;
  
  xfertyp = (SDHC_XFERTYP_CMDINX(SDHC_CMD0) | SDHC_XFERTYP_RSPTYP(SDHC_XFERTYP_RSPTYP_NO));                   
                   
  result = SDHC_CMD_Do(xfertyp);
    
  if(result == RES_OK)
  {
    // Here can be checked response in RESPONSE register    
        (void)SDHC_CMDRSP0;
  }
  
  return result; 
}

//-----------------------------------------------------------------------------
// FUNCTION:    SDHC_CMD2_Identify
// SCOPE:       SDHC Controller private related function
// DESCRIPTION: Function sends CMD 2 to identify card
//              
// PARAMETERS:  none
//              
// RETURNS:     result
//----------------------------------------------------------------------------- 
static DRESULT SDHC_CMD2_Identify(void)
{
  LWord xfertyp;
  DRESULT result;
  
  SDHC_CMDARG = 0;
  
  xfertyp = (SDHC_XFERTYP_CMDINX(SDHC_CMD2) | SDHC_XFERTYP_CCCEN_MASK | 
             SDHC_XFERTYP_RSPTYP(SDHC_XFERTYP_RSPTYP_136));                   
                   
  result = SDHC_CMD_Do(xfertyp);
    
  if(result == RES_OK)
  {
    // Here can be checked response in RESPONSE register    
        (void)SDHC_CMDRSP0;
  }
  
  return result; 
}

//-----------------------------------------------------------------------------
// FUNCTION:    SDHC_CMD3_GetAddress
// SCOPE:       SDHC Controller private related function
// DESCRIPTION: Function sends CMD 3 to get address
//              
// PARAMETERS:  none
//              
// RETURNS:     result
//----------------------------------------------------------------------------- 
static DRESULT SDHC_CMD3_GetAddress(void)
{
  LWord xfertyp;
  DRESULT result;
  
  SDHC_CMDARG = 0;
  
  xfertyp = (SDHC_XFERTYP_CMDINX(SDHC_CMD3) | SDHC_XFERTYP_CICEN_MASK | 
             SDHC_XFERTYP_CCCEN_MASK | SDHC_XFERTYP_RSPTYP(SDHC_XFERTYP_RSPTYP_48));                   
                   
  result = SDHC_CMD_Do(xfertyp);
    
  if(result == RES_OK)
  {
    // Here can be checked response in RESPONSE register    
        (void)SDHC_CMDRSP0;
  }
  
  return result; 
}

//-----------------------------------------------------------------------------
// FUNCTION:    SDHC_ACMD6_SetBusWidth
// SCOPE:       SDHC Controller private related function
// DESCRIPTION: Function sends ACMD 6 to set bus width
//              
// PARAMETERS:  none
//              
// RETURNS:     result
//----------------------------------------------------------------------------- 
static DRESULT SDHC_ACMD6_SetBusWidth(LWord address, LWord width)
{
  LWord xfertyp;
  DRESULT result;
  
  SDHC_CMDARG = address; 
  // first send CMD 55 Application specific command
  xfertyp = (SDHC_XFERTYP_CMDINX(SDHC_CMD55) | SDHC_XFERTYP_CICEN_MASK | 
             SDHC_XFERTYP_CCCEN_MASK | SDHC_XFERTYP_RSPTYP(SDHC_XFERTYP_RSPTYP_48));                   
  
  result = SDHC_CMD_Do(xfertyp);
  
  if(result == RES_OK)
  {
    // Here can be checked response in RESPONSE register    
        (void)SDHC_CMDRSP0;
  }else
    return result;
  
  SDHC_CMDARG = width;
  
  // Send 6CMD
  xfertyp = (SDHC_XFERTYP_CMDINX(SDHC_CMD6) | SDHC_XFERTYP_CICEN_MASK | 
             SDHC_XFERTYP_CCCEN_MASK | SDHC_XFERTYP_RSPTYP(SDHC_XFERTYP_RSPTYP_48));                   
                   
  result = SDHC_CMD_Do(xfertyp);
    
  if(result == RES_OK)
  {
    // Here can be checked response in RESPONSE register    
        (void)SDHC_CMDRSP0;
  }
  
  return result; 
}

//-----------------------------------------------------------------------------
// FUNCTION:    SDHC_CMD7_SelectCard
// SCOPE:       SDHC Controller private related function
// DESCRIPTION: Function sends CMD 7 to select card
//              
// PARAMETERS:  none
//              
// RETURNS:     result
//----------------------------------------------------------------------------- 
static DRESULT SDHC_CMD7_SelectCard(LWord address)
{
  LWord xfertyp;
  DRESULT result;
  
  SDHC_CMDARG = address; 
  
  xfertyp = (SDHC_XFERTYP_CMDINX(SDHC_CMD7) | SDHC_XFERTYP_CICEN_MASK | 
             SDHC_XFERTYP_CCCEN_MASK | SDHC_XFERTYP_RSPTYP(SDHC_XFERTYP_RSPTYP_48BUSY));                   
                   
  result = SDHC_CMD_Do(xfertyp);
    
  if(result == RES_OK)
  {
    // Here can be checked response in RESPONSE register    
        (void)SDHC_CMDRSP0;
  }
  
  return result; 
}

//-----------------------------------------------------------------------------
// FUNCTION:    SDHC_CMD8_SetInterface
// SCOPE:       SDHC Controller private related function
// DESCRIPTION: Function sends CMD 8 to send interface condition
//              
// PARAMETERS:  none
//              
// RETURNS:     result
//----------------------------------------------------------------------------- 
static DRESULT SDHC_CMD8_SetInterface(LWord cond)
{
  LWord xfertyp;
  DRESULT result;
  
  SDHC_CMDARG = cond; 
  
  xfertyp = (SDHC_XFERTYP_CMDINX(SDHC_CMD8) | SDHC_XFERTYP_CICEN_MASK | 
             SDHC_XFERTYP_CCCEN_MASK | SDHC_XFERTYP_RSPTYP(SDHC_XFERTYP_RSPTYP_48));                   
                   
  result = SDHC_CMD_Do(xfertyp);
    
  if(result == RES_OK)
  {
    // Here can be checked response in RESPONSE register    
        (void)SDHC_CMDRSP0;
  }
  
  return result; 
}

//-----------------------------------------------------------------------------
// FUNCTION:    SDHC_CMD9_GetParameters
// SCOPE:       SDHC Controller private related function
// DESCRIPTION: Function sends CMD 8 to send interface condition
//              
// PARAMETERS:  none
//              
// RETURNS:     result
//----------------------------------------------------------------------------- 
static DRESULT SDHC_CMD9_GetParameters(LWord address)
{
  LWord xfertyp;
  DRESULT result;
  
  SDHC_CMDARG = address; 
  
  xfertyp = (SDHC_XFERTYP_CMDINX(SDHC_CMD9) | SDHC_XFERTYP_CCCEN_MASK | 
             SDHC_XFERTYP_RSPTYP(SDHC_XFERTYP_RSPTYP_136));                   
                   
  result = SDHC_CMD_Do(xfertyp);
    
  if(result == RES_OK)
  {
    // Here can be checked response in RESPONSE register    
        (void)SDHC_CMDRSP0;
  }
  
  return result; 
}


//-----------------------------------------------------------------------------
// FUNCTION:    SDHC_CMD12_StopTransfer
// SCOPE:       SDHC Controller private related function
// DESCRIPTION: Function sends CMD 12 to stop transfer
//              
// PARAMETERS:  none
//              
// RETURNS:     result
//----------------------------------------------------------------------------- 
static DRESULT SDHC_CMD12_StopTransfer(void)
{
  LWord xfertyp;
  DRESULT result;
  
  SDHC_CMDARG = 0;
  xfertyp = (SDHC_XFERTYP_CMDINX(SDHC_CMD12) | SDHC_XFERTYP_CMDTYP(SDHC_XFERTYP_CMDTYP_ABORT) |
                 SDHC_XFERTYP_CICEN_MASK | SDHC_XFERTYP_CCCEN_MASK | SDHC_XFERTYP_RSPTYP(SDHC_XFERTYP_RSPTYP_48BUSY));
  
  result = SDHC_CMD_Do(xfertyp);
  
  if(result == RES_OK)
  {
    // Here can be checked response in RESPONSE register    
  }

  return result;
}

//-----------------------------------------------------------------------------
// FUNCTION:    SDHC_CMD12_StopTransferWaitForBusy
// SCOPE:       SDHC Controller private related function
// DESCRIPTION: Function sends CMD 12 to stop transfer and first waits to ready SDCArd
//              
// PARAMETERS:  none
//              
// RETURNS:     result
//----------------------------------------------------------------------------- 
static DRESULT SDHC_CMD12_StopTransferWaitForBusy(void)
{
  LWord timeOut = 100;
  DRESULT result;
  do{
    result = SDHC_CMD12_StopTransfer();
    timeOut--;
  }while(timeOut && (SDHC_PRSSTAT & SDHC_PRSSTAT_DLA_MASK) && result == RES_OK);
  
  if(result != RES_OK)
    return result;
  
  if(!timeOut)
    return RES_NONRSPNS;
  
  return RES_OK;
}

//-----------------------------------------------------------------------------
// FUNCTION:    SDHC_CMD16_SetBlockSize
// SCOPE:       SDHC Controller private related function
// DESCRIPTION: Function sends CMD 8 to set block size
//              
// PARAMETERS:  none
//              
// RETURNS:     result
//----------------------------------------------------------------------------- 
static DRESULT SDHC_CMD16_SetBlockSize(LWord block_size)
{
  LWord xfertyp;
  DRESULT result;
  
  SDHC_CMDARG = block_size; 
  
  xfertyp = (SDHC_XFERTYP_CMDINX(SDHC_CMD16) | SDHC_XFERTYP_CICEN_MASK | 
             SDHC_XFERTYP_CCCEN_MASK | SDHC_XFERTYP_RSPTYP(SDHC_XFERTYP_RSPTYP_48));                   
                   
  result = SDHC_CMD_Do(xfertyp);
    
  if(result == RES_OK)
  {
    // Here can be checked response in RESPONSE register    
        (void)SDHC_CMDRSP0;
  }
  
  return result; 
}

//-----------------------------------------------------------------------------
// FUNCTION:    SDHC_CMD17_ReadBlock
// SCOPE:       SDHC Controller private related function
// DESCRIPTION: Function sends CMD 17 to read one block
//              
// PARAMETERS:  sector - sector on the sdcard
//              
// RETURNS:     result
//----------------------------------------------------------------------------- 
static DRESULT SDHC_CMD17_ReadBlock(LWord sector)
{
  LWord xfertyp;
  DRESULT result;
  
  while ((SDHC_PRSSTAT & SDHC_PRSSTAT_DLA_MASK)==1){};
  
  SDHC_CMDARG = sector;
  
  SDHC_BLKATTR &= ~(SDHC_BLKATTR_BLKCNT_MASK);  
  SDHC_BLKATTR |= SDHC_BLKATTR_BLKCNT(1);
  
  xfertyp = (SDHC_XFERTYP_CMDINX(SDHC_CMD17) | SDHC_XFERTYP_CICEN_MASK | 
                 SDHC_XFERTYP_CCCEN_MASK | SDHC_XFERTYP_RSPTYP(SDHC_XFERTYP_RSPTYP_48) | 
                 SDHC_XFERTYP_DTDSEL_MASK | SDHC_XFERTYP_DPSEL_MASK 
#if SDHC_TRANSFERTYPE == SDHC_TRANSFERTYPE_DMA 
                  | SDHC_XFERTYP_DMAEN_MASK
#endif
                  );
  
  result = SDHC_CMD_Do(xfertyp);
    
  if(result == RES_OK)
  {
    // Here can be checked response in RESPONSE register    
        (void)SDHC_CMDRSP0;
  }
  
  return result;
}

//-----------------------------------------------------------------------------
// FUNCTION:    SDHC_CMD18_ReadBlocks
// SCOPE:       SDHC Controller private related function
// DESCRIPTION: Function sends CMD 18 to read multiple blocks
//              
// PARAMETERS:  sector - start sector on the sdcard
//              count - count of sector to read
//              
// RETURNS:     result
//----------------------------------------------------------------------------- 
static DRESULT SDHC_CMD18_ReadBlocks(LWord sector, LWord count)
{
  LWord xfertyp;
  DRESULT result;
  
  while ((SDHC_PRSSTAT & SDHC_PRSSTAT_DLA_MASK)==1){};
  
  SDHC_CMDARG = sector;
//  SDHC_BLKATTR &= ~(SDHC_BLKATTR_BLKCNT_MASK);  
//  SDHC_BLKATTR |= SDHC_BLKATTR_BLKCNT(count);
  SDHC_BLKATTR = SDHC_BLKATTR_BLKCNT(count) | 512;
  
  xfertyp = (SDHC_XFERTYP_CMDINX(SDHC_CMD18) | SDHC_XFERTYP_CICEN_MASK | 
                 SDHC_XFERTYP_CCCEN_MASK | SDHC_XFERTYP_RSPTYP(SDHC_XFERTYP_RSPTYP_48) | 
                 SDHC_XFERTYP_DTDSEL_MASK | SDHC_XFERTYP_DPSEL_MASK | 
                 SDHC_XFERTYP_MSBSEL_MASK | SDHC_XFERTYP_AC12EN_MASK | SDHC_XFERTYP_BCEN_MASK
#if SDHC_TRANSFERTYPE == SDHC_TRANSFERTYPE_DMA 
                  | SDHC_XFERTYP_DMAEN_MASK
#endif
                  );
  result = SDHC_CMD_Do(xfertyp);
    
  if(result == RES_OK)
  {
    // Here can be checked response in RESPONSE register    
        (void)SDHC_CMDRSP0;
  }
  
  return result;
}

//-----------------------------------------------------------------------------
// FUNCTION:    SDHC_CMD24_WriteBlock
// SCOPE:       SDHC Controller private related function
// DESCRIPTION: Function sends CMD 24 to write one block
//              
// PARAMETERS:  sector - start sector on the sdcard
//              
// RETURNS:     result
//----------------------------------------------------------------------------- 
static DRESULT SDHC_CMD24_WriteBlock(LWord sector)
{
  LWord xfertyp;
  DRESULT result;
  
  SDHC_CMDARG = sector;
  SDHC_BLKATTR &= ~(SDHC_BLKATTR_BLKCNT_MASK);  
  SDHC_BLKATTR |= SDHC_BLKATTR_BLKCNT(1);
  
  xfertyp = (SDHC_XFERTYP_CMDINX(SDHC_CMD24) | SDHC_XFERTYP_CICEN_MASK | 
                 SDHC_XFERTYP_CCCEN_MASK | SDHC_XFERTYP_RSPTYP(SDHC_XFERTYP_RSPTYP_48) | 
                 SDHC_XFERTYP_DPSEL_MASK
#if SDHC_TRANSFERTYPE == SDHC_TRANSFERTYPE_DMA 
                  | SDHC_XFERTYP_DMAEN_MASK
#endif
                  );                   
  
  result = SDHC_CMD_Do(xfertyp);
    
  if(result == RES_OK)
  {
    // Here can be checked response in RESPONSE register    
    (void)SDHC_CMDRSP0;
  }
  
  return result;
}

//-----------------------------------------------------------------------------
// FUNCTION:    SDHC_CMD25_WriteBlocks
// SCOPE:       SDHC Controller private related function
// DESCRIPTION: Function sends CMD 25 to write multiple blocks
//              
// PARAMETERS:  sector - start sector on the sdcard
//              count - count of sector to write
//              
// RETURNS:     result
//----------------------------------------------------------------------------- 
static DRESULT SDHC_CMD25_WriteBlocks(LWord sector, LWord count)
{
  LWord xfertyp;
  DRESULT result;
  
  SDHC_CMDARG = sector;
//  SDHC_BLKATTR &= ~(SDHC_BLKATTR_BLKCNT_MASK);  
//  SDHC_BLKATTR |= SDHC_BLKATTR_BLKCNT(count);
  SDHC_BLKATTR = SDHC_BLKATTR_BLKCNT(count) | 512;
  
  xfertyp = (SDHC_XFERTYP_CMDINX(SDHC_CMD25) | SDHC_XFERTYP_CICEN_MASK | 
                 SDHC_XFERTYP_CCCEN_MASK | SDHC_XFERTYP_RSPTYP(SDHC_XFERTYP_RSPTYP_48) | 
                 SDHC_XFERTYP_DPSEL_MASK | 
                 SDHC_XFERTYP_MSBSEL_MASK | SDHC_XFERTYP_AC12EN_MASK | SDHC_XFERTYP_BCEN_MASK
#if SDHC_TRANSFERTYPE == SDHC_TRANSFERTYPE_DMA 
                  | SDHC_XFERTYP_DMAEN_MASK
#endif
                  );                   
                   
  result = SDHC_CMD_Do(xfertyp);
    
  if(result == RES_OK)
  {
    // Here can be checked response in RESPONSE register    
        (void)SDHC_CMDRSP0;
  }
  
  return result; 
}

//-----------------------------------------------------------------------------
// FUNCTION:    SDHC_ACMD41_SendOperationCond
// SCOPE:       SDHC Controller private related function
// DESCRIPTION: Function sends ACMD 41 to send operation condition
//              
// PARAMETERS:  none
//              
// RETURNS:     result
//----------------------------------------------------------------------------- 
static DRESULT SDHC_ACMD41_SendOperationCond(LWord cond)
{
  LWord xfertyp;
  DRESULT result;
  
  SDHC_CMDARG = 0; 
  // first send CMD 55 Application specific command
  xfertyp = (SDHC_XFERTYP_CMDINX(SDHC_CMD55) | SDHC_XFERTYP_CICEN_MASK | 
             SDHC_XFERTYP_CCCEN_MASK | SDHC_XFERTYP_RSPTYP(SDHC_XFERTYP_RSPTYP_48));                   
  
  result = SDHC_CMD_Do(xfertyp);
  
  if(result == RES_OK)
  {
    // Here can be checked response in RESPONSE register    
        (void)SDHC_CMDRSP0;
  }else
    return result;
  
  SDHC_CMDARG = cond;
  
  // Send 41CMD
  xfertyp = (SDHC_XFERTYP_CMDINX(SDHC_ACMD41) | SDHC_XFERTYP_RSPTYP(SDHC_XFERTYP_RSPTYP_48));                   
                   
  result = SDHC_CMD_Do(xfertyp);
    
  if(result == RES_OK)
  {
    // Here can be checked response in RESPONSE register    
        (void)SDHC_CMDRSP0;
  }
  
  return result; 
}

