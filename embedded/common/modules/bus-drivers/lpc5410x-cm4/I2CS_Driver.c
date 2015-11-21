/* Open Sensor Platform Project
 * https://github.com/sensorplatforms/open-sensor-platform
 *
 * Copyright (C) 2015 Audience Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/*-------------------------------------------------------------------------------------------------*\
 |    I N C L U D E   F I L E S
\*-------------------------------------------------------------------------------------------------*/
#include "Common.h"
#include "i2cs_driver.h"
#include "chip.h"
#include "hw_setup_xpresso_lpc54102.h"
#include "clock_5410x.h"
#include "syscon_5410x.h"

/*-------------------------------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/
#define I2C_SLV_ADDR_NUM (3)
#define I2C_SLV_ADDR_MSK    (0x7F)
#define I2C_SLV_PCLK_FREQ    (4000000) /* I2C slave peripheral clock set to 4 MHZ */

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   T Y P E   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
typedef struct {
    uint16_t slaveAddr;          /*!< Slave address used for this index in the controller */
    uint8_t SlaveIndex;         /*!< Slave index to use, 0 is first index */
    uint8_t EnableSlave;        /*!< Set to 0 to disable this slave index, or non-0 to enable */
} i2cs_slave_t;

/*-------------------------------------------------------------------------------------------------*\
 |    F O R W A R D   F U N C T I O N   D E C L A R A T I O N S
\*-------------------------------------------------------------------------------------------------*/
static void i2cslave_start(uint8_t addr);
static void i2cslave_done(void);
static uint8_t i2cslave_tx(uint8_t *data);
static uint8_t i2cslave_rx(uint8_t data);

/*-------------------------------------------------------------------------------------------------*\
 |    S T A T I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
static i2c_t   *pi2c_slave_obj = NULL;

/* I2C slave callback functions */
static const I2CS_XFER_T i2cs_HostIfCallBacks = {
    &i2cslave_start,
    &i2cslave_tx,
    &i2cslave_rx,
    &i2cslave_done
};

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E     F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/****************************************************************************************************
 * @fn      i2cs_setup_slave
 *          Helper routine to set up slave
 *
 ***************************************************************************************************/
static void i2cs_setup_slave(i2cs_slave_t *pSlaveSetup)
{
    uint32_t optimalDev;
    /* Limit usable slave address indexes to the maximum the controller can support */
    if (pSlaveSetup->SlaveIndex <= I2C_SLV_ADDR_NUM) {
        Chip_I2CS_SetSlaveAddr(I2C_HOSTIF, pSlaveSetup->SlaveIndex, (pSlaveSetup->slaveAddr & I2C_SLV_ADDR_MSK));
        /* Clear interrupt status */
        Chip_I2CS_ClearStatus(I2C_HOSTIF, I2C_STAT_SLVDESEL);

        if (pSlaveSetup->EnableSlave == 0) {
            Chip_I2CS_DisableSlaveAddr(I2C_HOSTIF, pSlaveSetup->SlaveIndex);
        }
        else {
            Chip_I2C_EnableInt(I2C_HOSTIF, I2C_INTENSET_SLVPENDING | I2C_INTENSET_SLVDESEL);
        }
    }

    optimalDev = Chip_Clock_GetAsyncSyscon_ClockRate()/I2C_SLV_PCLK_FREQ;
    Chip_I2C_SetClockDiv( I2C_HOSTIF, optimalDev );
    /* Enable I2C slave interface */
    Chip_I2CS_Enable(I2C_HOSTIF);
}

/****************************************************************************************************
 * @fn      i2cslave_start
 *          Slave I2C Start callback function
 *
 ***************************************************************************************************/
static void i2cslave_start(uint8_t addr)
{
    /* Do Nothing */
    //D0_printf("Slave Addr %x\n", addr);
}

/****************************************************************************************************
 * @fn      i2cslave_tx
 *          Slave data transmit callback function
 *
 ***************************************************************************************************/
static uint8_t i2cslave_tx(uint8_t *data)
{
    uint8_t *p8;
    i2c_t *obj = pi2c_slave_obj; /* get the record*/

    p8 = (uint8_t *) obj->pXfer.txBuff;
    if (p8 != NULL) {
        *data = p8[obj->pXfer.bytesSent]; /* fetch next byte to be sent */
        //D0_printf("I2CS bytes sent %d from %x\r\n", obj->pXfer.txSz,obj->pXfer.txBuff);
        obj->pXfer.bytesSent++;
    }
    else {
        *data = 0; /* Have to send something, so NAK with 0 */
    }

    if(obj->pXfer.bytesSent >= obj->pXfer.txSz) {
        obj->i2c_operation = 0x2; /* write operation */
    }

    return I2C_SLVCTL_SLVCONTINUE;
}

/****************************************************************************************************
 * @fn      i2cslave_rx
 *          Slave data receive callback function
 *
 ***************************************************************************************************/
static uint8_t i2cslave_rx(uint8_t data)
{

    uint8_t *p8;
    uint8_t ret_val = I2C_SLVCTL_SLVCONTINUE;
    i2c_t *obj = pi2c_slave_obj; /* get the record*/

    obj->i2c_operation = 0x1; /* read operation */

    p8 = obj->pXfer.rxBuff;
    p8[obj->pXfer.bytesRecv++] = (uint8_t) data;

    if(obj->pXfer.bytesRecv >= obj->pXfer.rxSz) {
        obj->pXfer.rxBuff = (uint8_t *)obj->pXfer.rxBuff + obj->pXfer.rxSz;
        obj->pXfer.rxSz++;
        obj->pXfer.bytesRecv = 0;
    }

    return ret_val;
}

/****************************************************************************************************
 * @fn      i2cslave_done
 *          I2C Done event callback function. This function will be called after STOP bit is received.
 *
 ***************************************************************************************************/
static void i2cslave_done(void)
{
    i2c_t *obj = pi2c_slave_obj; /* get the record*/

    obj->pXfer.rxBuff = obj->rxBuff;
    obj->pXfer.rxSz = 0;
    obj->pXfer.status = ERR_I2C_BUSY;
    obj->pXfer.bytesSent = 0;
    obj->pXfer.bytesRecv = 0;
}

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C     F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/****************************************************************************************************
 * @fn      i2c_init
 *          Configures the GPIOs, enables clock and also resets the I2C slave
 *
 * @param   obj - reference to the i2c slave data structure
 *
 * @return  none
 *
 ***************************************************************************************************/
void i2c_init(i2c_t *obj)
{
    /* Enable I2C clock and reset I2C peripheral */
    Chip_I2C_Init(I2C_HOSTIF);

    /* Setup I2C pin mux */
    Chip_IOCON_PinMuxSet(LPC_IOCON, HOSTIF_SCL_PIN);
    Chip_IOCON_PinMuxSet(LPC_IOCON, HOSTIF_SDA_PIN);
    pi2c_slave_obj = obj;
}

/****************************************************************************************************
 * @fn      i2c_slave_mode
 *          This function sets up the I2C slave address, the clock rate, registers
 *          the receive buffer to receive data from host and also resets transfer record parameters
 *
 * @param   obj - reference to the i2c slave data structure
 * @param   enable_slave - enable or disable i2c slave
 *
 * @return  none
 *
 ***************************************************************************************************/
void i2c_slave_mode(i2c_t *obj, int enable_slave)
{
   i2cs_slave_t slaveSetup;

    /* Setup slave address to respond to */
    slaveSetup.slaveAddr = obj->ui_slave_address;     /* host address is 7 bit */
    slaveSetup.SlaveIndex = obj->uc_slave_index;
    slaveSetup.EnableSlave = enable_slave;
    i2cs_setup_slave(&slaveSetup);

   /* Init transfer parameters  */
    obj->pXfer.rxBuff = obj->rxBuff;
    obj->pXfer.status = ERR_I2C_BUSY;
    obj->pXfer.bytesSent = 0;
    obj->pXfer.bytesRecv = 0;
}

/****************************************************************************************************
 * @fn      i2c_slave_receive
 *          This function handles data transfer from host. Currently this function is being called
 *          from IRQ handler
 *
 * @param   obj - reference to the i2c slave data structure
 *
 * @return  none
 *
 ***************************************************************************************************/
int i2c_slave_receive( i2c_t *obj )
{
    obj->i2c_operation = 0; /* reset current i2c operation status */
    Chip_I2CS_XferHandler(I2C_HOSTIF, &i2cs_HostIfCallBacks);
    return obj->i2c_operation;
}

/****************************************************************************************************
 * @fn      i2c_slave_write
 *          This function sets up the pointer to the data to be transmitted and also the transmit length.
 *          The bytes get transmitted when the master requests for data
 *
 * @param   obj - reference to the i2c slave data structure
 * @param   data - reference to the data to be transmitted
 * @param   length - length of transmit data
 *
 * @return  none
 *
 ***************************************************************************************************/
int i2c_slave_write( i2c_t *obj, const char *data, int length )
{
    obj->pXfer.txBuff = (uint8_t *)data;
    obj->pXfer.txSz = length;

   /* Return success now, bytes will be written when master is requesting for data */
   return length;
}

/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/

