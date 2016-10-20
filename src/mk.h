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
//mk.h
// contains bare metal definitions (similar to kinetis.h) to allow programmatic changes

#ifndef MK_H
#define MK_H

/******************************************************************************************************/
/* PORT - Peripheral instance base addresses */
#define PORTA_BASE_PTR                           (0x40049000u)
#define PORTB_BASE_PTR                           (0x4004A000u)
#define PORTC_BASE_PTR                           (0x4004B000u)
#define PORTD_BASE_PTR                           (0x4004C000u)
#define PORTE_BASE_PTR                           (0x4004D000u)
/** Array initializer of PORT peripheral base pointers */
#define PORT_BASE_PTRS                           { PORTA_BASE_PTR, PORTB_BASE_PTR, PORTC_BASE_PTR, PORTD_BASE_PTR, PORTE_BASE_PTR }

/******************************************************************************************************/

/** GPIO - Peripheral register structure */
typedef struct GPIO_MemMap {
  uint32_t PDOR;                                   /**< Port Data Output Register, offset: 0x0 */
  uint32_t PSOR;                                   /**< Port Set Output Register, offset: 0x4 */
  uint32_t PCOR;                                   /**< Port Clear Output Register, offset: 0x8 */
  uint32_t PTOR;                                   /**< Port Toggle Output Register, offset: 0xC */
  uint32_t PDIR;                                   /**< Port Data Input Register, offset: 0x10 */
  uint32_t PDDR;                                   /**< Port Data Direction Register, offset: 0x14 */
} volatile *GPIO_MemMapPtr;

/* GPIO - Register accessors */
#define GPIO_PDOR_REG(base)                      ((base)->PDOR)
#define GPIO_PSOR_REG(base)                      ((base)->PSOR)
#define GPIO_PCOR_REG(base)                      ((base)->PCOR)
#define GPIO_PTOR_REG(base)                      ((base)->PTOR)
#define GPIO_PDIR_REG(base)                      ((base)->PDIR)
#define GPIO_PDDR_REG(base)                      ((base)->PDDR)


/* ----------------------------------------------------------------------------
   -- GPIO Register Masks
   ---------------------------------------------------------------------------- */
/* PDOR Bit Fields */
#define GPIO_PDOR_PDO_MASK                       0xFFFFFFFFu
#define GPIO_PDOR_PDO_SHIFT                      0
#define GPIO_PDOR_PDO(x)                         (((uint32_t)(((uint32_t)(x))<<GPIO_PDOR_PDO_SHIFT))&GPIO_PDOR_PDO_MASK)
/* PSOR Bit Fields */
#define GPIO_PSOR_PTSO_MASK                      0xFFFFFFFFu
#define GPIO_PSOR_PTSO_SHIFT                     0
#define GPIO_PSOR_PTSO(x)                        (((uint32_t)(((uint32_t)(x))<<GPIO_PSOR_PTSO_SHIFT))&GPIO_PSOR_PTSO_MASK)
/* PCOR Bit Fields */
#define GPIO_PCOR_PTCO_MASK                      0xFFFFFFFFu
#define GPIO_PCOR_PTCO_SHIFT                     0
#define GPIO_PCOR_PTCO(x)                        (((uint32_t)(((uint32_t)(x))<<GPIO_PCOR_PTCO_SHIFT))&GPIO_PCOR_PTCO_MASK)
/* PTOR Bit Fields */
#define GPIO_PTOR_PTTO_MASK                      0xFFFFFFFFu
#define GPIO_PTOR_PTTO_SHIFT                     0
#define GPIO_PTOR_PTTO(x)                        (((uint32_t)(((uint32_t)(x))<<GPIO_PTOR_PTTO_SHIFT))&GPIO_PTOR_PTTO_MASK)
/* PDIR Bit Fields */
#define GPIO_PDIR_PDI_MASK                       0xFFFFFFFFu
#define GPIO_PDIR_PDI_SHIFT                      0
#define GPIO_PDIR_PDI(x)                         (((uint32_t)(((uint32_t)(x))<<GPIO_PDIR_PDI_SHIFT))&GPIO_PDIR_PDI_MASK)
/* PDDR Bit Fields */
#define GPIO_PDDR_PDD_MASK                       0xFFFFFFFFu
#define GPIO_PDDR_PDD_SHIFT                      0
#define GPIO_PDDR_PDD(x)                         (((uint32_t)(((uint32_t)(x))<<GPIO_PDDR_PDD_SHIFT))&GPIO_PDDR_PDD_MASK)

/* GPIO - Peripheral instance base addresses */
#define PTA_BASE_PTR                             ((GPIO_MemMapPtr)0x400FF000u)
#define PTB_BASE_PTR                             ((GPIO_MemMapPtr)0x400FF040u)
#define PTC_BASE_PTR                             ((GPIO_MemMapPtr)0x400FF080u)
#define PTD_BASE_PTR                             ((GPIO_MemMapPtr)0x400FF0C0u)
#define PTE_BASE_PTR                             ((GPIO_MemMapPtr)0x400FF100u)

/** Array initializer of GPIO peripheral base pointers */
#define GPIO_BASE_PTRS                           { PTA_BASE_PTR, PTB_BASE_PTR, PTC_BASE_PTR, PTD_BASE_PTR, PTE_BASE_PTR }

/******************************************************************************************************/
/** SPI - Peripheral register structure */
typedef struct SPI_MemMap {
  uint32_t MCR;                                    /**< DSPI Module Configuration Register, offset: 0x0 */
  uint8_t RESERVED_0[4];
  uint32_t TCR;                                    /**< DSPI Transfer Count Register, offset: 0x8 */
  union {                                          /* offset: 0xC */
    uint32_t CTAR[2];                                /**< DSPI Clock and Transfer Attributes Register (In Master Mode), array offset: 0xC, array step: 0x4 */
    uint32_t CTAR_SLAVE[1];                          /**< DSPI Clock and Transfer Attributes Register (In Slave Mode), array offset: 0xC, array step: 0x4 */
  };
  uint8_t RESERVED_1[24];
  uint32_t SR;                                     /**< DSPI Status Register, offset: 0x2C */
  uint32_t RSER;                                   /**< DSPI DMA/Interrupt Request Select and Enable Register, offset: 0x30 */
  union {                                          /* offset: 0x34 */
    uint32_t PUSHR;                                  /**< DSPI PUSH TX FIFO Register In Master Mode, offset: 0x34 */
    uint32_t PUSHR_SLAVE;                            /**< DSPI PUSH TX FIFO Register In Slave Mode, offset: 0x34 */
  };
  uint32_t POPR;                                   /**< DSPI POP RX FIFO Register, offset: 0x38 */
  uint32_t TXFR0;                                  /**< DSPI Transmit FIFO Registers, offset: 0x3C */
  uint32_t TXFR1;                                  /**< DSPI Transmit FIFO Registers, offset: 0x40 */
  uint32_t TXFR2;                                  /**< DSPI Transmit FIFO Registers, offset: 0x44 */
  uint32_t TXFR3;                                  /**< DSPI Transmit FIFO Registers, offset: 0x48 */
  uint8_t RESERVED_2[48];
  uint32_t RXFR0;                                  /**< DSPI Receive FIFO Registers, offset: 0x7C */
  uint32_t RXFR1;                                  /**< DSPI Receive FIFO Registers, offset: 0x80 */
  uint32_t RXFR2;                                  /**< DSPI Receive FIFO Registers, offset: 0x84 */
  uint32_t RXFR3;                                  /**< DSPI Receive FIFO Registers, offset: 0x88 */
} volatile *SPI_MemMapPtr;

/* SPI - Register accessors */
#define SPI_MCR_REG(base)                        ((base)->MCR)
#define SPI_TCR_REG(base)                        ((base)->TCR)
#define SPI_CTAR_REG(base,index2)                ((base)->CTAR[index2])
#define SPI_CTAR_SLAVE_REG(base,index2)          ((base)->CTAR_SLAVE[index2])
#define SPI_SR_REG(base)                         ((base)->SR)
#define SPI_RSER_REG(base)                       ((base)->RSER)
#define SPI_PUSHR_REG(base)                      ((base)->PUSHR)
#define SPI_PUSHR_SLAVE_REG(base)                ((base)->PUSHR_SLAVE)
#define SPI_POPR_REG(base)                       ((base)->POPR)
#define SPI_TXFR0_REG(base)                      ((base)->TXFR0)
#define SPI_TXFR1_REG(base)                      ((base)->TXFR1)
#define SPI_TXFR2_REG(base)                      ((base)->TXFR2)
#define SPI_TXFR3_REG(base)                      ((base)->TXFR3)
#define SPI_RXFR0_REG(base)                      ((base)->RXFR0)
#define SPI_RXFR1_REG(base)                      ((base)->RXFR1)
#define SPI_RXFR2_REG(base)                      ((base)->RXFR2)
#define SPI_RXFR3_REG(base)                      ((base)->RXFR3)

#define SPI_PUSHR_TXDATA_MASK                    0xFFFFu
#define SPI_PUSHR_TXDATA_SHIFT                   0
#define SPI_PUSHR_TXDATA(x)                      (((uint32_t)(((uint32_t)(x))<<SPI_PUSHR_TXDATA_SHIFT))&SPI_PUSHR_TXDATA_MASK)

#define SPI_POPR_RXDATA_MASK                     0xFFFFFFFFu
#define SPI_POPR_RXDATA_SHIFT                    0
#define SPI_POPR_RXDATA(x)                       (((uint32_t)(((uint32_t)(x))<<SPI_POPR_RXDATA_SHIFT))&SPI_POPR_RXDATA_MASK)


/** Peripheral SPI base pointesr */
#define SPI0_BASE_PTR                            ((SPI_MemMapPtr)0x4002C000u)
#define SPI1_BASE_PTR                            ((SPI_MemMapPtr)0x4002D000u)
#define SPI2_BASE_PTR                            ((SPI_MemMapPtr)0x400AC000u)
/** Array initializer of SPI peripheral base pointers */
#define SPI_BASE_PTRS                            { SPI0_BASE_PTR, SPI1_BASE_PTR, SPI2_BASE_PTR }

#endif
