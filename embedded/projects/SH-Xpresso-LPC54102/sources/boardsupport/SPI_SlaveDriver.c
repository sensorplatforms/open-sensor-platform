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
#include "SPI_SlaveDriver.h"
#include "Queue.h"
#include "SensorPackets.h"

/*-------------------------------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/
int16_t DeQueueToBuffer( uint8_t *pBuf, uint32_t *pBufSz, Queue_t *pQ );
extern Queue_t *_HiFNonWakeupQueue;

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/
#define SPI_IF_IDX                      SPIFIFO_IF_IDX //for SPI used in FIFO calls

/* TX & RX FIFO & Thresholds */
#define SPI_RX_FIFO_SZ                  8   //Min 4 and Max 8 allowed for this driver to work
#define SPI_TX_FIFO_SZ                  8   //Min 4 and Max 8 allowed for this driver to work
#define SPI_RX_THRES                    2   //In half words; When num entries in RX-FIFO is > this value
#define SPI_TX_THRES                    2   //In half words; Free entries in TX-FIFO is <= this value

/* Upper 16bits are control bits;No SSEL asserted; DataSz 16bits; EOT, EOF not asserted */
#define TX_CONTROL_BITS                 0x0F0F
#define TX_DUMMY_DATA                   0xAA55
#define TX_FILLER                       0xC0DE

/* Command defines */
#define GET_CAUSE_CMD                   0x8005
#define GET_CAUSE_DATA                  0
#define CONFIGURATION_CMD               0x8006
#define MAX_CONFIG_CMD_SZ               64
#define CAUSE_SENSOR_DATA_READY         8   //TODO: Move to common cause defines

/* Misc... */
#define SPI_TX_BUF_SZ                   256
#define FIFO_PERM_SZ                    6

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   T Y P E   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
typedef enum _CmdState
{
    SPI_IDLE,
    SPI_GETCAUSE_T1,            /* Get-Cause 1st transaction */
    SPI_GETCAUSE_T1_LENGTH,     /* Get-Cause 1nd transaction - send length */
    SPI_GETCAUSE_T2,            /* Get-Cause 2nd transaction - send data */
    SPI_CONFIGURATION,          /* Configuration Command Received */
    SPI_CONFIGURATION_PKT,      /* Configuration Packet Receive */
    NUM_SPI_CMDSTATES
} CmdState_t;

typedef enum _TransactionState
{
    TSTATE_IDLE,
    WAIT_GC_T2_END,         /* Wait for 2nd transaction for Get-Cause to finish */
    WAIT_GC_T2_START,       /* Wait for 2nd transaction for Get-Cause to start */
} TState_t;

typedef struct _SPIState
{
    CmdState_t State;   //Current state of SPI driver
    uint32_t BufLength; //Total Length of data to be transmitted (or received in case of Configuration pkt)
    uint32_t Remain;    //Remaining length to be transmitted/received
    uint16_t Cause;     //Cause identifier for current transaction
    uint32_t BufIdx;    //Current transmit index of SPI buffer
} SPIState_t;

/*-------------------------------------------------------------------------------------------------*\
 |    S T A T I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
/* FIFO configuration */
static LPC_FIFO_CFGSIZE_T _fifoSzCfg;

/* SPI State tracking */
static SPIState_t _SpiState;
static TState_t _TranState;

/* Various buffers */
static uint8_t _SpiTxBuffer[SPI_TX_BUF_SZ];
static uint32_t _BufferSz = sizeof(_SpiTxBuffer);
static uint8_t _ConfigBuffer[MAX_CONFIG_CMD_SZ];

/*-------------------------------------------------------------------------------------------------*\
 |    F O R W A R D   F U N C T I O N   D E C L A R A T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E     F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/
/****************************************************************************************************
 * @fn      SPISetupFIFO
 *          Initializes the FIFO interface for Slave SPI peripheral
 *
 ***************************************************************************************************/
static void SPISetupFIFO( void )
{
    uint32_t txFifoSz, rxFifoSz;
    LPC_FIFO_CFG_T     fifoCfg;

    /* System FIFO activate */
    Chip_FIFO_Init(LPC_FIFO);

    /* Debug - Print FIFO spaces available */
    txFifoSz = Chip_FIFO_GetFifoSpace(LPC_FIFO, FIFO_SPI, FIFO_TX);
    rxFifoSz = Chip_FIFO_GetFifoSpace(LPC_FIFO, FIFO_SPI, FIFO_RX);
    D1_printf("SPI-TxFIFO Size= %d, RxFIFO Size= %d\r\n", txFifoSz, rxFifoSz);

    /* FIFO Setup for SPI */
    /* Sizes for SPI0 & 1 - TODO Note: Doing combined per existing API, can be reimplemented later */
    _fifoSzCfg.fifoRXSize[0] = 0;       //No allocation of FIFO space to SPI0
    _fifoSzCfg.fifoTXSize[0] = 0;
    _fifoSzCfg.fifoRXSize[SPI_IF_IDX] = rxFifoSz; //All RX FIFO available to SPI1
    _fifoSzCfg.fifoTXSize[SPI_IF_IDX] = txFifoSz; //All TX FIFO available to SPI1
    Chip_FIFO_ConfigFifoSize(LPC_FIFO, FIFO_SPI, &_fifoSzCfg);

    /* Threshold, timeout config... */
    fifoCfg.noTimeoutContEmpty = 0; //Don't care
    fifoCfg.noTimeoutContWrite = 0; //Don't care
    fifoCfg.timeoutBase        = 0; //Don't care
    fifoCfg.timeoutValue       = 0; //Don't care
    fifoCfg.rxThreshold        = SPI_RX_THRES > 0? SPI_RX_THRES-1 : 0;
    fifoCfg.txThreshold        = SPI_TX_THRES > 0? SPI_TX_THRES-1 : 0;
    Chip_FIFOSPI_Configure(LPC_FIFO, SPI_IF_IDX, &fifoCfg);

    /* Flush FIFO - May not be needed (?) since we are resetting on activation */
    Chip_FIFOSPI_FlushFIFO(LPC_FIFO, SPI_IF_IDX, (LPC_PERIPFIFO_INT_RXFLUSH | LPC_PERIPFIFO_INT_TXFLUSH));

    /* Enable TX & RX FIFO function in SYSCON */
    Chip_SYSCON_EnableSysFIFO(SPIFIFO_SYSCON_TXEN | SPIFIFO_SYSCON_RXEN);

    /* Set control bits for TX - only needed once unless changed */
    LPC_FIFO->spi[SPI_IF_IDX].TXDATSPI_CTRL = TX_CONTROL_BITS;
}


/****************************************************************************************************
 * @fn      SPISlaveHardwareSetup
 *          Initializes the SPI interface hardware (function, pin mux, interrupts etc.)
 *
 ***************************************************************************************************/
static void SPISlaveHardwareSetup( void )
{
    SPI_CFGSETUP_T spiSetup;

    /* Connect the SPI1 signals to port pins */
    Chip_IOCON_PinMuxSet(LPC_IOCON, SPI_HOSTIF_SSN);
    Chip_IOCON_PinMuxSet(LPC_IOCON, SPI_HOSTIF_SCK);
    Chip_IOCON_PinMuxSet(LPC_IOCON, SPI_HOSTIF_MOSI);
    Chip_IOCON_PinMuxSet(LPC_IOCON, SPI_HOSTIF_MISO);

    /* Initialize SPI controllers */
    Chip_SPI_Init(SPI_HOSTIF_BUS);

    /* Call to initialize second SPI controller for mode0, slave mode, MSB first */
    Chip_SPI_Enable(SPI_HOSTIF_BUS);
    spiSetup.master = 0;
    spiSetup.lsbFirst = 0;
    spiSetup.mode = SPI_CLOCK_MODE1;
    Chip_SPI_ConfigureSPI(SPI_HOSTIF_BUS, &spiSetup);

    /* Setup slave controller SSEL0 for active low select */
    Chip_SPI_SetCSPolLow(SPI_HOSTIF_BUS, 0);

    /* Setup slave controller for 16-bit transfer. */
    Chip_SPI_SetXferSize(SPI_HOSTIF_BUS, 16);

    /* Note: Data interrupts are handled by FIFO interface. Only enable error & control interrupts */
    Chip_SPI_EnableInts(SPI_HOSTIF_BUS, (SPI_INTENSET_RXOVEN | SPI_INTENSET_TXUREN | SPI_INTENSET_SSAEN |
        SPI_INTENSET_SSDEN));

    /* Enable SPI controller interrupts */
    NVIC_EnableIRQ(SPI_HOSTIF_IRQn);
}


/****************************************************************************************************
 * @fn      PrintPacket
 *          Helper routine for debug display of packet data bytes
 *
 ***************************************************************************************************/
static void PrintPacket( uint8_t *pPkt, uint16_t len )
{
    uint16_t i;
    len = (len + 7) & 0xFFF8;

    D1_printf("\t-------------------------\r\n");
    for (i = 0; i < len; i+=8)
    {
        D1_printf("\tData: %02X %02X %02X %02X %02X %02X %02X %02X\r\n",
            pPkt[i], pPkt[i+1], pPkt[i+2], pPkt[i+3],
            pPkt[i+4], pPkt[i+5], pPkt[i+6], pPkt[i+7]);
    }
}


/****************************************************************************************************
 * @fn      SPIResetFIFOandStates
 *          Helper routine for bus recovery/reset due to error or at the end of a transaction
 *
 ***************************************************************************************************/
static void SPIResetFIFOandStates( void )
{
    int32_t i;

    Chip_FIFOSPI_FlushFIFO(LPC_FIFO, SPI_IF_IDX, (LPC_PERIPFIFO_INT_RXFLUSH | LPC_PERIPFIFO_INT_TXFLUSH));
    Chip_SPI_FlushFifos(SPI_HOSTIF_BUS);

    /* Prime 2x32-bit equivalent FIFO slots to avoid TX-Underrun errors */
    for (i = 0; i < FIFO_PERM_SZ; i++)
    {
        LPC_FIFO->spi[SPI_IF_IDX].TXDATSPI_DATA = TX_DUMMY_DATA;
    }

    _SpiState.State = SPI_IDLE;
    _TranState = TSTATE_IDLE;
}


/****************************************************************************************************
 * @fn      AddDataToFIFO
 *          Helper routine for adding transmit data to available FIFO
 *
 ***************************************************************************************************/
void AddDataToFIFO( uint32_t txFifoAvail )
{
    while ((txFifoAvail > 0) && (_SpiState.Remain > 0))
    {
        if ((_SpiState.BufIdx + 1) < _SpiState.BufLength)
        {
            LPC_FIFO->spi[SPI_IF_IDX].TXDATSPI_DATA = (_SpiTxBuffer[_SpiState.BufIdx] << 8) |
                _SpiTxBuffer[_SpiState.BufIdx+1];
            _SpiState.BufIdx += 2;
        }
        else if ((_SpiState.BufIdx + 1) == _SpiState.BufLength)
        {
            LPC_FIFO->spi[SPI_IF_IDX].TXDATSPI_DATA = (_SpiTxBuffer[_SpiState.BufIdx] << 8);
            _SpiState.BufIdx += 1;
        }
        else if (_SpiState.Remain != 0)
        {
            /* This 'else' handling takes care of situation where padding bytes were 3 */
            LPC_FIFO->spi[SPI_IF_IDX].TXDATSPI_DATA = 0; //Send padding data
        }

        txFifoAvail--;
        _SpiState.Remain--;
    }
}


/****************************************************************************************************
 * @fn      SPI_CommandHandler
 *          SPI Slave command handler that implements Get-Cause & Configuration command handling as
 *          described in the Host Interface Protocol document v0.8
 *
 * @param   [IN]rcvDat - 16-bit received data on the bus
 *
 * @return  none
 *
 ***************************************************************************************************/
static void SPI_CommandHandler( uint16_t rcvDat )
{
    uint32_t txFifoAvail = Chip_FIFOSPI_GetTxCount(LPC_FIFO, SPI_IF_IDX);
    static uint16_t configPktSz = 0;

    switch (_SpiState.State)
    {
    case SPI_IDLE:
        if (rcvDat == GET_CAUSE_CMD)
        {
            _SpiState.State = SPI_GETCAUSE_T1;
        }
        if (rcvDat == CONFIGURATION_CMD)
        {
            _SpiState.State = SPI_CONFIGURATION;
        }
        break;

    case SPI_GETCAUSE_T1:
        if (rcvDat == GET_CAUSE_DATA)
        {
            _SpiState.State = SPI_GETCAUSE_T2;

            /* Get packets from application and fill the local transmit buffer */
            _BufferSz = sizeof(_SpiTxBuffer);
            DeQueueToBuffer( _SpiTxBuffer, &_BufferSz, _HiFNonWakeupQueue );
            _SpiState.Cause     = CAUSE_SENSOR_DATA_READY;
            _SpiState.BufLength = _BufferSz;
            _SpiState.Remain    = ((_BufferSz + 3)/4) << 1; //16-bit data size in 32-bit multiple
            _SpiState.BufIdx    = 0;

            /* Prime the Tx FIFO with command response and data */
            /* Note: The following handling assumes that the FIFO size is set to at least 4. That
               ensures that at this point we have at least 2 slots in TX FIFO available */
            ASF_assert_var(txFifoAvail >= 2, txFifoAvail, _SpiState.State, _SpiState.Remain);

            LPC_FIFO->spi[SPI_IF_IDX].TXDATSPI_DATA = GET_CAUSE_CMD;
            txFifoAvail--;
            LPC_FIFO->spi[SPI_IF_IDX].TXDATSPI_DATA = _SpiState.Cause;
            txFifoAvail--;
            if (txFifoAvail < 2)
            {
                _SpiState.State = SPI_GETCAUSE_T1_LENGTH;
            }
            else
            {
                /* Send the actual length in bytes */
                LPC_FIFO->spi[SPI_IF_IDX].TXDATSPI_DATA = _SpiState.BufLength >> 16;
                LPC_FIFO->spi[SPI_IF_IDX].TXDATSPI_DATA = _SpiState.BufLength;
                txFifoAvail -= 2;

                /* Make sure there is something to send before expecting 2nd transaction. This situation is possible
                   (and was also observed) if there was a spurious/startup transient on the Host interrupt line that
                   caused the host to issue a Get-Cause command without there being any data available to send */
                if (_SpiState.BufLength > 0)
                {
                    _TranState = WAIT_GC_T2_START;

                    /* At this point if there is any more space in Tx FIFO then start adding data */
                    AddDataToFIFO( txFifoAvail );
                }
                else
                {
                    _TranState = WAIT_GC_T2_END;
                    /* Note: _SpiState.State change not needed since whole transaction will end with T1.
                       Also since _SpiState.Remain = 0 in this case, nothing will happen in SPI_GETCAUSE_T2 state */
                }
            }
        }
        else
        {
            /* Incorrect command - go back to idle state */
            _SpiState.State = SPI_IDLE;
        }
        break;

    case SPI_GETCAUSE_T1_LENGTH:
        if (txFifoAvail < 2)
        {
            /* Wait for at least 2 FIFO space to become available. */
            break;
        }
        /* Send the actual length in bytes */
        LPC_FIFO->spi[SPI_IF_IDX].TXDATSPI_DATA = _SpiState.BufLength >> 16;
        LPC_FIFO->spi[SPI_IF_IDX].TXDATSPI_DATA = _SpiState.BufLength;
        txFifoAvail -= 2;
        _SpiState.State = SPI_GETCAUSE_T2;

        /* Make sure there is something to send before expecting 2nd transaction */
        if (_SpiState.BufLength > 0)
        {
            _TranState = WAIT_GC_T2_START;
        }
        else
        {
            _TranState = WAIT_GC_T2_END;
            /* Note: _SpiState.State change not needed since whole transaction will end with T1. Also since
               _SpiState.Remain = 0in this case, nothing will happen in SPI_GETCAUSE_T2 state */
            break;
        }
        /*=== FALL Through to SPI_GETCAUSE_T2 handling ===*/

    case SPI_GETCAUSE_T2:
        /* We are in middle of 2nd Get-Cause transaction in which data packet is being sent */
        /* Note: It is possible to come to this state with no free slots in the FIFO */

        /* Start adding data to FIFO */
        AddDataToFIFO( txFifoAvail );
        break;

    case SPI_CONFIGURATION:
        LPC_FIFO->spi[SPI_IF_IDX].TXDATSPI_DATA = TX_FILLER; //Send padding data to TX FIFO to avoid TX-Underruns

        _SpiState.BufLength = rcvDat;
        _SpiState.BufIdx    = 0;
        _SpiState.Remain  = ((_SpiState.BufLength + 3)/4) << 1; //16-bit data size in 32-bit multiple

        ASF_assert_var(configPktSz < MAX_CONFIG_CMD_SZ, configPktSz, 0, 0); // Or ignore the packet instead of asserting
        _SpiState.State = SPI_CONFIGURATION_PKT;
        break;

    case SPI_CONFIGURATION_PKT:
        LPC_FIFO->spi[SPI_IF_IDX].TXDATSPI_DATA = TX_FILLER; //Avoid TX-Underruns

        if ((_SpiState.BufIdx >> 1) < _SpiState.Remain)
        {
            _ConfigBuffer[_SpiState.BufIdx++] = rcvDat >> 8;
            _ConfigBuffer[_SpiState.BufIdx++] = rcvDat & 0xFF;
        }
        /* Else: We are done receiving... just wait for transaction to finish now */
        break;

    default:
        LPC_FIFO->spi[SPI_IF_IDX].TXDATSPI_DATA = TX_FILLER; //Avoid TX-Underruns
        break;
    }
}


/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C     F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/****************************************************************************************************
 * @fn      SPISlaveSetup
 *          Setup the SPI Slave hardware & FIFOs.
 *
 * @param   none
 *
 * @return  none
 *
 ***************************************************************************************************/
void SPISlaveSetup( void )
{
    SPISetupFIFO();
    SPISlaveHardwareSetup();

    /* Setup FIFO interrupt handling. Only RX threshold interrupt is enabled since in
       Slave mode enabling TX threshold with cause continuous interrupt (as observed) */
    Chip_FIFOSPI_EnableInts(LPC_FIFO, SPI_IF_IDX, LPC_PERIPFIFO_INT_RXTH);

    /* Enable FIFO operation */
    Chip_FIFO_UnpauseFifo(LPC_FIFO, FIFO_SPI, FIFO_RX);
    Chip_FIFO_UnpauseFifo(LPC_FIFO, FIFO_SPI, FIFO_TX);

    /* Perm FIFO and reset states */
    SPIResetFIFOandStates();
}


/****************************************************************************************************
 * @fn      SPISlaveIRQHandler
 *          SPI Interrupt handler
 *
 * @param   none
 *
 * @return  none
 *
 ***************************************************************************************************/
void SPISlaveIRQHandler(void)
{
    SensorPacketTypes_t Out;
    uint32_t ints = Chip_SPI_GetPendingInts(SPI_HOSTIF_BUS);
    uint32_t intFifo = Chip_FIFOSPI_GetIntStatus(LPC_FIFO, SPI_IF_IDX);

    /* Check for control & Error interrupts */
    if ((ints & (SPI_INTENSET_RXOVEN | SPI_INTENSET_TXUREN | SPI_INTENSET_SSAEN | SPI_INTENSET_SSDEN)) != 0)
    {
        if (ints & SPI_INTENSET_RXOVEN)
        {
            Chip_SPI_ClearStatus(SPI_HOSTIF_BUS, SPI_STAT_RXOV);
            D0_printf("SPIS-ISR: RX Overrun!\r\n"); /* If all is well we should never see this! */
        }

        if (ints & SPI_INTENSET_TXUREN)
        {
            Chip_SPI_ClearStatus(SPI_HOSTIF_BUS, SPI_STAT_TXUR);
            D0_printf("SPIS-ISR: TX Underrun!\r\n"); /* If all is well we should never see this! */

            /* At this point the best solution is to try and recover the driver state and let host retry
               the last transaction if needed */
            SPIResetFIFOandStates();
        }

        if (ints & SPI_INTENSET_SSAEN)
        {
            Chip_SPI_ClearStatus(SPI_HOSTIF_BUS, SPI_STAT_SSA);
            if (_TranState == WAIT_GC_T2_START)
            {
                _TranState = WAIT_GC_T2_END;
            }
        }

        if (ints & SPI_INTENSET_SSDEN)
        {
            Chip_SPI_ClearStatus(SPI_HOSTIF_BUS, SPI_STAT_SSD);
            if ((_TranState == WAIT_GC_T2_END) || (_SpiState.State == SPI_IDLE) || (_SpiState.State == SPI_CONFIGURATION_PKT))
            {
                //D0_printf("\tEoT FIFO Flush.\r\n");
                if (_SpiState.State == SPI_CONFIGURATION_PKT)
                {
                    PrintPacket(_ConfigBuffer, _SpiState.BufLength);
                    ParseHostInterfacePkt( &Out, _ConfigBuffer, _SpiState.BufLength );
                }
                /* Reset FIFOs & States */
                SPIResetFIFOandStates();
            }
        }
    }

    /* For slave mode only RX Threshold needs to be enabled & handled */
    if (intFifo & LPC_PERIPFIFO_INT_RXTH)
    {
        uint32_t rxCount = Chip_FIFOSPI_GetRxCount(LPC_FIFO, SPI_IF_IDX);
        uint16_t rxData;

        while (rxCount > 0)
        {
            rxData = LPC_FIFO->spi[SPI_IF_IDX].RXDAT & 0xFFFF;
            rxCount--;
            SPI_CommandHandler( rxData );
        }
    }
}


/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
