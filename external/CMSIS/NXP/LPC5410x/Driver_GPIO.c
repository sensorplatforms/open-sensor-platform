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

#include "common.h"
#include "Driver_GPIO.h"
#include "chip.h"

#define OSP_GPIO_DRV_VERSION ARM_DRIVER_VERSION_MAJOR_MINOR(1,00)

/*-------------------------------------------------------------------------------------------------*\
 |    S T A T I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/* Pin muxing table, only items that need changing from their default pin
   state are in this table. Not every pin is mapped. */
STATIC const PINMUX_GRP_T pinmuxing[] = {

    /* I2C1 standard/fast (bridge) */
    {0, 27, (IOCON_FUNC1 | IOCON_MODE_INACT     | IOCON_DIGITAL_EN | IOCON_STDI2C_EN)}, /* BRIDGE_SCL (SCL) */
    {0, 28, (IOCON_FUNC1 | IOCON_MODE_INACT     | IOCON_DIGITAL_EN | IOCON_STDI2C_EN)}, /* BRIDGE_SDA (SDA) */

    /* Sensor related */
    {0, 4,  (IOCON_FUNC0 | IOCON_MODE_PULLDOWN  | IOCON_DIGITAL_EN)},                   /* GYR_INT1 (GPIO input) */
    {0, 18, (IOCON_FUNC0 | IOCON_MODE_INACT     | IOCON_DIGITAL_EN)},                   /* CT32B0_MAT0-ACCL_INT1 */
    {0, 22, (IOCON_FUNC0 | IOCON_MODE_PULLDOWN  | IOCON_DIGITAL_EN)},                   /* MAG_DRDY_INT (GPIO input) */

};

/* Private structure for GPIO */
typedef struct _GPIO_T {
    LPC_GPIO_T    *base;    /* GPIO Base address */
    uint16_t       port;    /* Port number */
    uint16_t       pin;     /* Pin number */
    uint8_t        id;      /* Pin Interrupt Select Value */
} GPIO_T;

/* Private argument */
static GPIO_T gpio_ID = { LPC_GPIO, 0, 0, 0 };

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E     F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/* Driver Version */
static const ARM_DRIVER_VERSION DriverVersion = {
    OSP_GPIO_API_VERSION,
    OSP_GPIO_DRV_VERSION
};

/****************************************************************************************************
 * @fn      GPIOx_GetVersion
 *          Get driver version.
 *
 * @param   gpio_priv    Private object containing GPIO Base address, Interrupt pin select value (id) and Port-Pin numbers
 *
 * @return  ARM_DRIVER_VERSION
 *
 ***************************************************************************************************/
static ARM_DRIVER_VERSION GPIOx_GetVersion (GPIO_T *gpio_priv)
{
    return DriverVersion;
}

/****************************************************************************************************
 * @fn      GPIOx_AINConfig
 *          Configures all IOs as AIN to reduce the power consumption.
 *
 * @param   none
 *
 * @return  none
 *
 ***************************************************************************************************/
static void GPIOx_AINConfig(void)
{
    return;
}

/****************************************************************************************************
 * @fn      GPIOx_Initialize
 *          Initializes the GPIO Module and Hardware
 *
 * @param   gpio_priv    Private object containing GPIO Base address, Interrupt pin select value (id) and Port-Pin numbers
 *
 * @return  Return code: On success return ARM_DRIVER_OK
 *
 ***************************************************************************************************/
static int32_t GPIOx_Initialize(GPIO_T *gpio_priv)
{
    /* Set all GPIOs to analog input mode to begin with */
    GPIOx_AINConfig();

    /* Enable the peripheral clock in the PMC */
    Chip_GPIO_Init(gpio_priv->base);

    /* TODO: Catch ALL for uninitialized pins... should be moved to respective modules */
    Chip_IOCON_SetPinMuxing(LPC_IOCON, pinmuxing, sizeof(pinmuxing) / sizeof(PINMUX_GRP_T));

    /* Setup DMA Common here since its not specific to any peripheral */
    Chip_DMA_Init(LPC_DMA);
    Chip_DMA_Enable(LPC_DMA);
    Chip_DMA_SetSRAMBase(LPC_DMA, DMA_ADDR(Chip_DMA_Table));
    return ARM_DRIVER_OK;
}

/****************************************************************************************************
 * @fn      GPIOx_Uninitialize
 *          Unitialize the GPIO Module
 *
 * @param   gpio_priv    Private object containing GPIO Base address, Interrupt pin select value (id) and Port-Pin numbers
 *
 * @return  Return code: On success return ARM_DRIVER_OK
 *
 ***************************************************************************************************/
static int32_t GPIOx_Uninitialize (GPIO_T *gpio_priv)
{

    /* Uninitialize GPIO */
    Chip_GPIO_DeInit(gpio_priv->base);

    Chip_PININT_DeInit(LPC_PININT);
    return ARM_DRIVER_OK;
}

/****************************************************************************************************
 * @fn      GPIOx_PowerControl
 *          Control the GPIO's power state
 *
 * @param   state       Target GPIO Power state
 * @param   gpio_priv    Private object containing GPIO Base address, Interrupt pin select value (id) and Port-Pin numbers
 *
 * @return  Return code: On success return ARM_DRIVER_OK
 *
 ***************************************************************************************************/
static int32_t GPIOx_PowerControl(ARM_POWER_STATE state, GPIO_T *gpio_priv)
{
    switch (state)
    {
        case ARM_POWER_OFF:
            Chip_PININT_ClearIntStatus(LPC_PININT, gpio_priv->pin);
            break;

        case ARM_POWER_FULL:
            Chip_INMUX_PinIntSel(gpio_priv->id, gpio_priv->port, gpio_priv->pin);
            break;

        default:
            return ARM_DRIVER_ERROR_UNSUPPORTED;
    }
    return ARM_DRIVER_OK;
}

/****************************************************************************************************
 * @fn      GPIOx_SetDirection
 *          Set the direction of GPIO pin
 *
 * @param   pin     Port pin number
 * @param   dir     Direction (GPIO_DIR_INPUT or GPIO_DIR_OUTPUT)
 * @param   gpio_priv    Private object containing GPIO Base address, Interrupt pin select value (id) and Port-Pin numbers
 *
 * @return  Return code: On success return ARM_DRIVER_OK
 *
 ***************************************************************************************************/
static int32_t GPIOx_SetDirection (uint32_t pin, uint32_t dir, GPIO_T *gpio_priv)
{
    uint32_t port_num = 0, pin_num = 0;

    ASF_assert(pin != (PinName)NC);
    ASF_assert ((dir == ARM_GPIO_DIR_INPUT) || (dir == ARM_GPIO_DIR_OUTPUT));
    port_num = DECODE_PORT(pin);
    pin_num  = DECODE_PIN(pin);

    dir  ? (gpio_priv->base->DIR[port_num] |=  (1UL << pin_num)) : \
         (gpio_priv->base->DIR[port_num] &= ~(1UL << pin_num));

     return ARM_DRIVER_OK;
}

/****************************************************************************************************
 * @fn      GPIOx_SetTrigger
 *          Set the trigger mode for GPIO
 *
 * @param   pin         Pin Interrupt Channel
 * @param   trigger     Trigger mode
 * @param   gpio_priv    Private object containing GPIO Base address, Interrupt pin select value (id) and Port-Pin numbers
 *
 * @return  Return code: On success return ARM_DRIVER_OK
 *
 ***************************************************************************************************/
static int32_t GPIOx_SetTrigger (uint32_t pin, uint32_t trigger, GPIO_T *gpio_priv)
{
    if (trigger & (1 << ARM_GPIO_TRIGGER_EDGE))
    {
        Chip_PININT_SetPinModeEdge(LPC_PININT, pin); /* edge sensitive */
    }
    else
    {
        Chip_PININT_SetPinModeLevel(LPC_PININT, pin); /* Level sensitive */
    }
    if ((trigger & (1 << ARM_GPIO_TRIGGER_HIGH)) || (trigger & (1 << ARM_GPIO_TRIGGER_RISING)))
    {
        Chip_PININT_EnableIntHigh(LPC_PININT, pin);  /* Rising Edge/High Level */
    }
    else
    {
        Chip_PININT_EnableIntLow(LPC_PININT, pin);   /* Falling Edge/Low Level */
    }
    return ARM_DRIVER_OK;
}

/****************************************************************************************************
 * @fn      GPIOx_WritePin
 *          Set the GPIO Pin state
 *
 * @param   pin     Port pin number
 * @param   val     Value to be written  (0 or 1)
 * @param   gpio_priv    Private object containing GPIO Base address, Interrupt pin select value (id) and Port-Pin numbers
 *
 * @return  Return code: On success return ARM_DRIVER_OK
 *
 ***************************************************************************************************/
static int32_t GPIOx_WritePin (uint32_t pin, uint32_t val, GPIO_T *gpio_priv)
{
    uint32_t port_num = 0, pin_num = 0;

    ASF_assert(pin != (PinName)NC);
    port_num = DECODE_PORT(pin);
    pin_num  = DECODE_PIN(pin);

    gpio_priv->base->B[port_num][pin_num] = val;
    return ARM_DRIVER_OK;
}

/****************************************************************************************************
 * @fn      GPIOx_ReadPin
 *          Get the GPIO Pin value
 *
 * @param   pin     Port pin number
 * @param   gpio_priv    Private object containing GPIO Base address, Interrupt pin select value (id) and Port-Pin numbers
 *
 * @return  pin value
 *
 ***************************************************************************************************/
static int32_t GPIOx_ReadPin (uint32_t pin, GPIO_T *gpio_priv)
{
    uint32_t port_num = 0, pin_num = 0;

    ASF_assert(pin != (PinName)NC);
    port_num = DECODE_PORT(pin);
    pin_num  = DECODE_PIN(pin);
    return ((gpio_priv->base->PIN[port_num] & (1UL << pin_num)) ? (1) : (0));
}

/****************************************************************************************************
 * @fn      GPIOx_SetHandler
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
static int32_t GPIOx_SetHandler (uint32_t pin, ARM_GPIO_Handler_t handler, void *data, GPIO_T *gpio_priv)
{
    return ARM_DRIVER_OK;    //Do nothing
}

/* Driver Definition */
ARM_BUILD_DRIVER_GPIO(GPIO, GPIOx, &gpio_ID);
