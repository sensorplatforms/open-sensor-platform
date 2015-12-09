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
#include "Driver_I2C.h"

uint8_t process_command(uint8_t *rx_buf, uint16_t length);
/*****************************************************************************
 * EXTERNAL VARIABLES & FUNCTIONS
 ****************************************************************************/
void 	MX_I2C2_IRQHandler ( void );
extern 	ARM_DRIVER_I2C Driver_I2C2;
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
static uint32_t      i2c_event = ARM_I2C_EVENT_GENERAL_CALL;

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
/******************************************************************************
 * @fn      i2c2_callback
 *            This function is the registered callback to get the event notifications
 *
 * @param   event - I2C event (Transfer or receive)
 *
 * @return   None
 *
 ******************************************************************************/
void i2c2_callback( uint32_t event )
{
    i2c_event = event;
}

/******************************************************************************
* @fn      I2C_HOSTIF_IRQHandler
*            Host interface IRQ handler, processes command received from host
*            Queues the next buffer for transfer after transmit complete operation
*
* @param   event - I2C event (Transfer or receive)
*
* @return   None
*
******************************************************************************/

void I2C_HOSTIF_IRQHandler(void)
{
    uint8_t rxBuff[RX_LENGTH];

    /* This transfer handler will call one of the registered callback
       to service the I2C event. */
    i2c_event = ARM_I2C_EVENT_GENERAL_CALL;
    MX_I2C2_IRQHandler();

    if ( i2c_event == ARM_I2C_EVENT_SLAVE_RECEIVE )
    {
        uint8_t ret;
        uint8_t read_bytes = 0;
        read_bytes = Driver_I2C2.GetDataCount();
        ASF_assert( RX_LENGTH > read_bytes );
        ASF_assert( ARM_DRIVER_OK == Driver_I2C2.SlaveReceive( rxBuff, read_bytes ));
        ret = process_command( rxBuff,
                               read_bytes );
        ASF_assert( ret == 0 );
    }

    if ( i2c_event == ARM_I2C_EVENT_SLAVE_TRANSMIT )
    {
        /* Assume each transmission will transmit all the requested data
         * so no need to keep track how many been transmitted.
         * check if there is any additional data to send
         */
        Hostif_TxNext( 0x0d000000 );        // Not sure what the magic number mean?
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
    Driver_I2C2.SlaveTransmit( (const uint8_t *)pBuf, (size-1) );
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
    Driver_I2C2.SlaveTransmit( (const uint8_t *)g_hostif.txBuff, i_tx_size );
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
    Driver_I2C2.SlaveTransmit( (const uint8_t *)pBuf, (size-1) );

   // Note: printf in IRQ tends to crash the system so use only in debugging code
}

/** Initialize the Sensor Hub host/AP interface */
void Hostif_Init(void)
{
    /* reset host IF control data structure */
    memset(&g_hostif, 0, sizeof(Hostif_Ctrl_t));

    /* I2C slave initialisation */
    Driver_I2C2.Initialize( i2c2_callback );

    /* Setup slave address to respond to */
    Driver_I2C2.PowerControl( ARM_POWER_FULL );
    /* init host interrupt pin */


    D0_printf("%s initialization done\r\n", __FUNCTION__);
}

