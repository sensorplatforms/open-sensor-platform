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
/* Private structure for GPIO */
typedef struct _GPIO_T
{
    LPC_GPIO_T    *base;    /* GPIO Base address */
    uint16_t       port;    /* Port number */
    uint16_t       pin;     /* Pin number */
    uint8_t        id;      /* Pin Interrupt Select Value */
} GPIO_T;

/*-------------------------------------------------------------------------------------------------*\
 |    S T A T I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
/* Private argument */
static GPIO_T gpio_ID = { LPC_GPIO, 0, 0, 0 };

/* Driver Version */
static const ARM_DRIVER_VERSION DriverVersion = {
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
 * @param   gpio_priv    Private object containing GPIO Base address, Interrupt pin select value (id) and Port-Pin numbers
 *
 * @return  ARM_DRIVER_VERSION
 *
 ***************************************************************************************************/
static ARM_DRIVER_VERSION OSP_GPIO_GetVersion( GPIO_T *gpio_priv )
{
    return DriverVersion;
}

/****************************************************************************************************
 * @fn      OSP_GPIO_AINConfig
 *          Configures all IOs as AIN to reduce the power consumption.
 *
 * @param   none
 *
 * @return  none
 *
 ***************************************************************************************************/
static void OSP_GPIO_AINConfig( void )
{
    return;
}

/****************************************************************************************************
 * @fn      OSP_GPIO_Initialize
 *          Initializes the GPIO Module and Hardware
 *
 * @param   gpio_priv    Private object containing GPIO Base address, Interrupt pin select value (id) and Port-Pin numbers
 *
 * @return  Return code: On success return ARM_DRIVER_OK
 *
 ***************************************************************************************************/
static int32_t OSP_GPIO_Initialize( GPIO_T *gpio_priv )
{
    /* Set all GPIOs to analog input mode to begin with */
    OSP_GPIO_AINConfig();

    /* Enable the peripheral clock in the PMC */
    Chip_GPIO_Init( gpio_priv->base );
    return ARM_DRIVER_OK;
}

/****************************************************************************************************
 * @fn      OSP_GPIO_Uninitialize
 *          Unitialize the GPIO Module
 *
 * @param   gpio_priv    Private object containing GPIO Base address, Interrupt pin select value (id) and Port-Pin numbers
 *
 * @return  Return code: On success return ARM_DRIVER_OK
 *
 ***************************************************************************************************/
static int32_t OSP_GPIO_Uninitialize( GPIO_T *gpio_priv )
{
    /* Uninitialize GPIO */
    Chip_GPIO_DeInit( gpio_priv->base );

    Chip_PININT_DeInit( LPC_PININT );
    return ARM_DRIVER_OK;
}

/****************************************************************************************************
 * @fn      OSP_GPIO_PowerControl
 *          Control the GPIO's power state
 *
 * @param   state       Target GPIO Power state
 * @param   gpio_priv    Private object containing GPIO Base address, Interrupt pin select value (id) and Port-Pin numbers
 *
 * @return  Return code: On success return ARM_DRIVER_OK
 *
 ***************************************************************************************************/
static int32_t OSP_GPIO_PowerControl( ARM_POWER_STATE state, GPIO_T *gpio_priv )
{
    switch ( state )
    {
        case ARM_POWER_OFF:
            Chip_PININT_ClearIntStatus( LPC_PININT, gpio_priv->pin );
            break;

        case ARM_POWER_FULL:
            Chip_INMUX_PinIntSel( gpio_priv->id, gpio_priv->port, gpio_priv->pin );
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
 * @param   pin     Port pin number
 * @param   dir     Direction (GPIO_DIR_INPUT or GPIO_DIR_OUTPUT)
 * @param   gpio_priv    Private object containing GPIO Base address, Interrupt pin select value (id) and Port-Pin numbers
 *
 * @return  Return code: On success return ARM_DRIVER_OK
 *
 ***************************************************************************************************/
static int32_t OSP_GPIO_SetDirection( uint32_t pin, uint32_t dir, GPIO_T *gpio_priv )
{
    uint32_t port_num = 0, pin_num = 0;

    ASF_assert( pin != (PinName)NC );
    ASF_assert( (dir == ARM_GPIO_DIR_INPUT) || (dir == ARM_GPIO_DIR_OUTPUT) );
    port_num = DECODE_PORT( pin );
    pin_num  = DECODE_PIN( pin );

    dir  ? ( gpio_priv->base->DIR[port_num] |=  ( 1UL << pin_num) ) : \
         ( gpio_priv->base->DIR[port_num] &= ~( 1UL << pin_num) );

    return ARM_DRIVER_OK;
}

/****************************************************************************************************
 * @fn      OSP_GPIO_SetTrigger
 *          Set the trigger mode for GPIO
 *
 * @param   pin         Pin Interrupt Channel
 * @param   trigger     Trigger mode
 * @param   gpio_priv    Private object containing GPIO Base address, Interrupt pin select value (id) and Port-Pin numbers
 *
 * @return  Return code: On success return ARM_DRIVER_OK
 *
 ***************************************************************************************************/
static int32_t OSP_GPIO_SetTrigger( uint32_t pin, uint32_t trigger, GPIO_T *gpio_priv )
{
    if ( trigger & (1 << ARM_GPIO_TRIGGER_EDGE) )
    {
        Chip_PININT_SetPinModeEdge( LPC_PININT, pin ); /* edge sensitive */
    }
    else
    {
        Chip_PININT_SetPinModeLevel( LPC_PININT, pin ); /* Level sensitive */
    }
    if ( (trigger & (1 << ARM_GPIO_TRIGGER_HIGH)) || (trigger & (1 << ARM_GPIO_TRIGGER_RISING)) )
    {
        Chip_PININT_EnableIntHigh( LPC_PININT, pin );  /* Rising Edge/High Level */
    }
    else
    {
        Chip_PININT_EnableIntLow( LPC_PININT, pin );   /* Falling Edge/Low Level */
    }
    return ARM_DRIVER_OK;
}

/****************************************************************************************************
 * @fn      OSP_GPIO_WritePin
 *          Set the GPIO Pin state
 *
 * @param   pin     Port pin number
 * @param   val     Value to be written  (0 or 1)
 * @param   gpio_priv    Private object containing GPIO Base address, Interrupt pin select value (id) and Port-Pin numbers
 *
 * @return  Return code: On success return ARM_DRIVER_OK
 *
 ***************************************************************************************************/
static int32_t OSP_GPIO_WritePin( uint32_t pin, uint32_t val, GPIO_T *gpio_priv )
{
    uint32_t port_num = 0, pin_num = 0;

    ASF_assert( pin != (PinName)NC );
    port_num = DECODE_PORT( pin );
    pin_num  = DECODE_PIN( pin );

    gpio_priv->base->B[port_num][pin_num] = val;
    return ARM_DRIVER_OK;
}

/****************************************************************************************************
 * @fn      OSP_GPIO_ReadPin
 *          Get the GPIO Pin value
 *
 * @param   pin     Port pin number
 * @param   gpio_priv    Private object containing GPIO Base address, Interrupt pin select value (id) and Port-Pin numbers
 *
 * @return  pin value
 *
 ***************************************************************************************************/
static int32_t OSP_GPIO_ReadPin( uint32_t pin, GPIO_T *gpio_priv )
{
    uint32_t port_num = 0, pin_num = 0;

    ASF_assert( pin != (PinName)NC );
    port_num = DECODE_PORT( pin );
    pin_num  = DECODE_PIN( pin );
    return ( (gpio_priv->base->PIN[port_num] & (1UL << pin_num)) ? (1) : (0) );
}

/****************************************************************************************************
 * @fn      OSP_GPIO_SetHandler
 *          Handler function
 *
 * @param   pin     Port pin number
 * @param   handler Pointer to handler function
 * @param   data    Data to pass to the handler
 * @param   gpio_priv    Private object containing GPIO Base address, Interrupt pin select value (id) and Port-Pin numbers
 *
 * @return  Return code: On success return ARM_DRIVER_OK
 *
 ***************************************************************************************************/
static int32_t OSP_GPIO_SetHandler( uint32_t pin, ARM_GPIO_Handler_t handler, void *data, GPIO_T *gpio_priv )
{
    return ARM_DRIVER_OK;    //Do nothing
}

/* Driver Definition */
ARM_BUILD_DRIVER_GPIO( GPIO, OSP_GPIO, &gpio_ID );

/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
