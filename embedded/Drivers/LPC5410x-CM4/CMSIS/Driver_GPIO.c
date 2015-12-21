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
#include "common.h"
#include "Driver_GPIO.h"
#include "chip.h"

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/
#define OSP_GPIO_DRV_VERSION ARM_DRIVER_VERSION_MAJOR_MINOR( 1,00 )

/*-------------------------------------------------------------------------------------------------*\
 |    T Y P E   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    S T A T I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
/* Driver Version */
static const ARM_DRIVER_VERSION DriverVersion =
{
    OSP_GPIO_API_VERSION,
    OSP_GPIO_DRV_VERSION
};

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E     F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/
/****************************************************************************************************
 * @fn      OSP_GPIO_GetVersion
 *          Get driver version.
 *
 * @param   gpio_priv   Private argument (Unused)
 *
 * @return  ARM_DRIVER_VERSION
 *
 ***************************************************************************************************/
static ARM_DRIVER_VERSION OSP_GPIO_GetVersion( void *gpio_priv )
{
    return DriverVersion;
}

/****************************************************************************************************
 * @fn      OSP_GPIO_Initialize
 *          Initializes the GPIO Module and Hardware
 *
 * @param   gpio_priv   Private argument (Unused)
 *
 * @return  Return code: On success return ARM_DRIVER_OK
 *
 ***************************************************************************************************/
static int32_t OSP_GPIO_Initialize( void *gpio_priv )
{
    /* Enable the peripheral clock in the PMC */
    Chip_GPIO_Init( LPC_GPIO );

    /* Enable Pin interrupt sources */
    Chip_PININT_Init( NULL );
    return ARM_DRIVER_OK;
}

/****************************************************************************************************
 * @fn      OSP_GPIO_Uninitialize
 *          Unitialize the GPIO Module
 *
 * @param   gpio_priv   Private argument (Unused)
 *
 * @return  Return code: On success return ARM_DRIVER_OK
 *
 ***************************************************************************************************/
static int32_t OSP_GPIO_Uninitialize( void *gpio_priv )
{
    /* Uninitialize GPIO */
    Chip_GPIO_DeInit( LPC_GPIO );

    Chip_PININT_DeInit( NULL );
    return ARM_DRIVER_OK;
}

/****************************************************************************************************
 * @fn      OSP_GPIO_PowerControl
 *          Control the GPIO's power state
 *
 * @param   state       Target GPIO Power state
 * @param   gpio_priv   Private argument (Unused)
 *
 * @return  Return code: On success return ARM_DRIVER_OK
 *
 ***************************************************************************************************/
static int32_t OSP_GPIO_PowerControl( ARM_POWER_STATE state, void *gpio_priv )
{
    switch ( state )
    {
    case ARM_POWER_OFF:
        Chip_GPIO_DeInit( LPC_GPIO );
        break;

    case ARM_POWER_FULL:
        Chip_GPIO_Init( LPC_GPIO );
        break;

    default:
        return ARM_DRIVER_ERROR_UNSUPPORTED;
    }
    return ARM_DRIVER_OK;
}

/****************************************************************************************************
 * @fn      OSP_GPIO_SetDirection
 *          Set the direction of GPIO pin
 *
 * @param   pin         Port pin number (Upper 16-bit is port number and lower 16-bit is pin number)
 * @param   dir         Direction (GPIO_DIR_INPUT or GPIO_DIR_OUTPUT)
 * @param   gpio_priv   Private argument (Unused)
 *
 * @return  Return code: On success return ARM_DRIVER_OK
 *
 ***************************************************************************************************/
static int32_t OSP_GPIO_SetDirection( uint32_t pin, uint32_t dir, void *gpio_priv )
{
    uint32_t port_num = 0, pin_num = 0;

    ASF_assert( pin != (PinName)NC );
    ASF_assert( (dir == ARM_GPIO_DIR_INPUT) || (dir == ARM_GPIO_DIR_OUTPUT) );
    port_num = DECODE_PORT( pin );
    pin_num  = DECODE_PIN( pin );

    ( dir == ARM_GPIO_DIR_OUTPUT ) ? Chip_GPIO_SetPinDIROutput( LPC_GPIO, port_num, pin_num ) : \
                                     Chip_GPIO_SetPinDIRInput( LPC_GPIO, port_num, pin_num );
    return ARM_DRIVER_OK;
}

/****************************************************************************************************
 * @fn      OSP_GPIO_SetTrigger
 *          Set the trigger mode for GPIO
 *
 * @param   pin         Port pin number (Upper 16-bit is port number and lower 16-bit is pin number)
 * @param   trigger     Trigger mode
 * @param   gpio_priv   Private argument (Unused)
 *
 * @return  Return code: On success return ARM_DRIVER_OK
 *
 ***************************************************************************************************/
static int32_t OSP_GPIO_SetTrigger( uint32_t pin, uint32_t trigger, void *gpio_priv )
{
    uint8_t index = 0;
    uint8_t pinInterruptChannel = 0;

    ASF_assert( pin != (PinName)NC );

    for ( ; index < MAX_PIN_INTERRUPT_CHANNEL; index++ )
    {
        if ( GPIO_PinMap[index].pin == pin )
        {
            pinInterruptChannel = GPIO_PinMap[index].pinInterruptChannel;
            break;
        }
    }

    ASF_assert( index < MAX_PIN_INTERRUPT_CHANNEL );

    if ( trigger & (1 << ARM_GPIO_TRIGGER_EDGE) )
    {
        Chip_PININT_SetPinModeEdge( LPC_PININT, pinInterruptChannel ); /* edge sensitive */
    }
    else
    {
        Chip_PININT_SetPinModeLevel( LPC_PININT, pinInterruptChannel ); /* Level sensitive */
    }

    if ( (trigger & (1 << ARM_GPIO_TRIGGER_HIGH)) || (trigger & (1 << ARM_GPIO_TRIGGER_RISING)) )
    {
        Chip_PININT_EnableIntHigh( LPC_PININT, pinInterruptChannel );  /* Rising Edge/High Level */
    }
    else
    {
        Chip_PININT_EnableIntLow( LPC_PININT, pinInterruptChannel );   /* Falling Edge/Low Level */
    }
    return ARM_DRIVER_OK;
}

/****************************************************************************************************
 * @fn      OSP_GPIO_WritePin
 *          Set the GPIO Pin state
 *
 * @param   pin         Port pin number (Upper 16-bit is port number and lower 16-bit is pin number)
 * @param   val         Value to be written  (0 or 1)
 * @param   gpio_priv   Private argument (Unused)
 *
 * @return  Return code: On success return ARM_DRIVER_OK
 *
 ***************************************************************************************************/
static int32_t OSP_GPIO_WritePin( uint32_t pin, uint32_t val, void *gpio_priv )
{
    uint32_t port_num = 0, pin_num = 0;

    ASF_assert( pin != (PinName)NC );
    ASF_assert( val <= 1 );
    port_num = DECODE_PORT( pin );
    pin_num  = DECODE_PIN( pin );

    Chip_GPIO_WritePortBit( LPC_GPIO, port_num, pin_num, (bool) val );
    return ARM_DRIVER_OK;
}

/****************************************************************************************************
 * @fn      OSP_GPIO_ReadPin
 *          Get the GPIO Pin value
 *
 * @param   pin         Port pin number (Upper 16-bit is port number and lower 16-bit is pin number)
 * @param   gpio_priv   Private argument (Unused)
 *
 * @return  pin value
 *
 ***************************************************************************************************/
static int32_t OSP_GPIO_ReadPin( uint32_t pin, void *gpio_priv )
{
    uint32_t port_num = 0, pin_num = 0;

    ASF_assert( pin != (PinName)NC );
    port_num = DECODE_PORT( pin );
    pin_num  = DECODE_PIN( pin );

    return ( Chip_GPIO_ReadPortBit( LPC_GPIO, port_num, pin_num ) );
}

/****************************************************************************************************
 * @fn      OSP_GPIO_SetHandler
 *          Handler function
 *
 * @param   pin         Port pin number (Upper 16-bit is port number and lower 16-bit is pin number)
 * @param   handler     Pointer to handler function
 * @param   data        Data to pass to the handler
 * @param   gpio_priv   Private argument (Unused)
 *
 * @return  Return code: On success return ARM_DRIVER_OK
 *
 ***************************************************************************************************/
static int32_t OSP_GPIO_SetHandler( uint32_t pin, ARM_GPIO_Handler_t handler, void *data, void *gpio_priv )
{
    return ARM_DRIVER_ERROR_UNSUPPORTED;    //Not Supported
}

/* Driver Definition */
ARM_BUILD_DRIVER_GPIO( GPIO, OSP_GPIO, NULL );

/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
