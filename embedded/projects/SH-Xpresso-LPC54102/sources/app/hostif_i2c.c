/*
 * @brief Implements I2C slave driver for host interface module
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2014
 * All rights reserved.
 *
 * @par
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * LPC products.  This software is supplied "AS IS" without any warranties of
 * any kind, and NXP Semiconductors and its licensor disclaim any and
 * all warranties, express or implied, including all implied warranties of
 * merchantability, fitness for a particular purpose and non-infringement of
 * intellectual property rights.  NXP Semiconductors assumes no responsibility
 * or liability for the use of the software, conveys no license or rights under any
 * patent, copyright, mask work right, or any other intellectual property rights in
 * or to any products. NXP Semiconductors reserves the right to make changes
 * in the software without notification. NXP Semiconductors also makes no
 * representation or warranty that such application will be suitable for the
 * specified use without further testing or modification.
 *
 * @par
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, under NXP Semiconductors' and its
 * licensor's relevant copyrights in the software, without fee, provided that it
 * is used in conjunction with NXP Semiconductors microcontrollers.  This
 * copyright, permission, and disclaimer notice must appear in all copies of
 * this code.
 */
#include <string.h>
#include <stdint.h>
#include "common.h"
#include "osp-types.h"
#include "i2cs_driver.h"

uint8_t process_command(uint8_t *rx_buf, uint16_t length);

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/
#define RX_LENGTH               16
#define I2C_SLAVE_XFER_DONE     0x10
#define I2C_OVERREACH_VAL       0xCC// This value is sent for access beyond register area

typedef struct __HOSTIF_Ctrl_t {
    int    magic;
    uint16_t rxCount;            /* Bytes so far received  */
    uint16_t rxLength;           /* Expected Rx buffer length */
    uint16_t txCountSet;
    uint16_t txCount;            /* Bytes so far transmitted */
    uint16_t txLength;           /* Total transfer length of Tx buffer */
    uint8_t *txBuff;             /* Tx buffer pointer */
    uint8_t *oldtxBuff;
    uint8_t rxBuff[RX_LENGTH];   /* Rx buffer */
    uint8_t *txBuff_next;
    uint16_t txLength_next;
} Hostif_Ctrl_t;

static Hostif_Ctrl_t g_hostif;
static i2c_t slave_i2c_handle;

#define I2C_MEM_SZ    64 /* Size of memory for I2C Slave ROM driver */

#if defined(I2CS_DMA_BASED)
/* DMA descriptors must be aligned to 16 bytes */
#if defined(__CC_ARM)
__align(16) static DMA_CHDESC_T dmaI2CSDesc;
#endif /* defined (__CC_ARM) */

/* IAR support */
#if defined(__ICCARM__)
#pragma data_alignment=16
static DMA_CHDESC_T dmaI2CSDesc;
#endif /* defined (__ICCARM__) */

#if defined( __GNUC__ )
static DMA_CHDESC_T dmaI2CSDesc __attribute__ ((aligned(16)));
#endif /* defined (__GNUC__) */
#endif

static void Hostif_TxNext(int magic);

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Private functions
 ****************************************************************************/

/*****************************************************************************
 * Public functions
 ****************************************************************************/

/* I2CS Interrupt handler.
 * This function is called whenever there is an I2C interrupt service request
 * on the hostif I2C bus.
 */
void I2C_HOSTIF_IRQHandler(void)
{
    slave_i2c_handle.i2c_operation = 0;

    // This transfer handler will call one of the registered callback to
    // to service the I2C event.
    i2c_slave_receive(&slave_i2c_handle);

    if ( slave_i2c_handle.i2c_operation == 0x1)
    {
        uint8_t ret;
        ret = process_command( slave_i2c_handle.pXfer.rxBuff,
                                 slave_i2c_handle.pXfer.rxSz);
        if ( slave_i2c_handle.rxBuff == slave_i2c_handle.pXfer.rxBuff )
        {
            slave_i2c_handle.pXfer.rxSz  = ret;
        }
    }

    if ( slave_i2c_handle.i2c_operation == 0x2 )
    {
        /* Assume each transmission will transmit all the requested data
         * so no need to keep track how many been transmitted.
         * check if there is any additional data to send
         */
        Hostif_TxNext(0x0d000000);        // Not sure what the magic number mean?
    }
}


/* This function get calls directly when processing receiving host command */
void Hostif_StartTx(uint8_t *pBuf, uint16_t size, int magic)
{
    if (magic != -1)
        g_hostif.magic = magic;
    if (magic == -1)
        g_hostif.magic &= 0xffff0000;
    g_hostif.txBuff = pBuf;
    if (size != 0)
        g_hostif.txCountSet = size;
    if (pBuf != NULL)
        g_hostif.oldtxBuff = pBuf;    // keep a copy of the requested transmit buffer pointer
    g_hostif.txCount = 0;             // number of bytes transmitted
    g_hostif.txLength = size;             // number of bytes transmitted
    /* replace g_hostif.txBuff with i2c_write API
     * No actual transmission happens here, but the I2c HAL keeps the buffer for transmission
     * when the master is actually ready to receive.
     */
    i2c_slave_write(&slave_i2c_handle,(const char *)pBuf,size-1);
    g_hostif.txLength_next = 0;
    g_hostif.txBuff_next = 0;
}


/* This function get calls after a transmit has complete. This
 * function will setup another transmit if there are data pending
 * to be sent
 */
void Hostif_TxNext(int magic)
{
    int   i_tx_size = 0;
    g_hostif.magic |= magic;
    g_hostif.txCountSet = g_hostif.txLength_next;
    g_hostif.txCount = 0;

    if (g_hostif.txBuff_next) {
        g_hostif.txBuff = g_hostif.txBuff_next;
        g_hostif.txLength = g_hostif.txLength_next;
        g_hostif.txBuff_next = NULL;
        g_hostif.txLength_next = 0;
    } else {
        g_hostif.txBuff = NULL;
        g_hostif.txLength = 0;
    }

    i_tx_size = g_hostif.txLength > 0 ? g_hostif.txLength -1 : 0;
    /*
     * Replace g_hostif.txBuff with i2c_write API
     * No actual transmission happens here, but the I2c HAL keeps the buffer for transmission
     * when the master is actually ready to receive.
     */
    i2c_slave_write(&slave_i2c_handle,(const char *)g_hostif.txBuff,i_tx_size);
}

void CHostif_StartTxChained(uint8_t *pBuf, uint16_t size,
                     uint8_t *pBuf_next, uint16_t size_next, int magic)
{
    if (magic != -1)
        g_hostif.magic = magic;
    if (magic == -1)
        g_hostif.magic &= 0xffff0000;
    g_hostif.txBuff = pBuf;
    if (size != 0)
        g_hostif.txCountSet = size;
    if (pBuf != NULL)
        g_hostif.oldtxBuff = pBuf;
    g_hostif.txCount = 0;
    g_hostif.txLength = size;
    g_hostif.txBuff_next = pBuf_next;
    g_hostif.txLength_next = size_next;

    /*
      * No actual transmission happens here, but the I2c HAL keeps the buffer for transmission
      * when the master is actually ready to receive.
      */
    i2c_slave_write(&slave_i2c_handle,(const char *)pBuf,size-1);

   // Note: printf in IRQ tends to crash the system so use only in debugging code
}

/** Initialize the Sensor Hub host/AP interface */
void Hostif_Init(void)
{
    /* reset host IF control data structure */
    memset(&g_hostif, 0, sizeof(Hostif_Ctrl_t));

    slave_i2c_handle.pXfer.rxSz = 0;
    slave_i2c_handle.ui_slave_address = I2C_HOSTIF_ADDR;
    slave_i2c_handle.uc_slave_index = 0;
    slave_i2c_handle.i2c = I2C_LPC_2;
    /* I2C slave initialisation */
    i2c_init(&slave_i2c_handle);

    /* Setup slave address to respond to */
    i2c_slave_mode(&slave_i2c_handle,1);
    /* init host interrupt pin */

    Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, HOSTIF_IRQ_PORT, HOSTIF_IRQ_PIN);
    /* de-assert interrupt line to high to indicate Host/AP that
     * there is no data to receive */
    Chip_GPIO_SetPinState(LPC_GPIO_PORT, HOSTIF_IRQ_PORT, HOSTIF_IRQ_PIN, 0);

    /* Enable the interrupt for the I2C */
    NVIC_SetPriority(I2C_HOSTIF_IRQn, HOSTIF_IRQ_PRIORITY);
    NVIC_EnableIRQ(I2C_HOSTIF_IRQn);

    /* enable I2C hostif to wake-up the sensor hub */
    Chip_SYSCON_EnableWakeup(I2C_HOSTIF_WAKE);

    D0_printf("%s initialization done\r\n", __FUNCTION__);
}

