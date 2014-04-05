/* Open Sensor Platform Project
 * https://github.com/sensorplatforms/open-sensor-platform
 *
 * Copyright (C) 2013 Sensor Platforms Inc.
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
#include <string.h>
#include "Alg_Conversion.h"


/*-------------------------------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/
extern SystemDescriptor_t gPlatformDesc;

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/
#define Q32DIFF (32 - QFIXEDPOINTPRECISE)

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   T Y P E   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    S T A T I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
static uint32_t _AccTimeStampExtension = 0;    // we will re-create a larger raw time stamp here
static uint32_t _AccLastTimeStamp = 0;         // keep the last time stamp here, we will use it to check for rollover

static uint32_t _MagTimeStampExtension = 0;    // we will re-create a larger raw time stamp here
static uint32_t _MagLastTimeStamp = 0;         // keep the last time stamp here, we will use it to check for rollover

static uint32_t _GyroTimeStampExtension = 0;    // we will re-create a larger raw time stamp here
static uint32_t _GyroLastTimeStamp = 0;         // keep the last time stamp here, we will use it to check for rollover

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
 * @fn      UMul32
 *          Helper routine for 32-bit saturating multiply
 *
 ***************************************************************************************************/
void UMul32(uint32_t x,uint32_t y, uint32_t * pHigh, uint32_t * pLow) {
    uint16_t xmsb;
    uint16_t ymsb;
    uint16_t xlsb;
    uint16_t ylsb;

    register uint32_t high;
    register uint32_t low;
    register uint32_t temp2;
    register uint32_t temp;


    xmsb = x >> 16;
    ymsb = y >> 16;

    xlsb = x & 0x0000FFFF;
    ylsb = y & 0x0000FFFF;

    high = mult_uint16_uint16(xmsb , ymsb);

    temp = mult_uint16_uint16(ymsb , xlsb);
    high += (temp & 0xFFFF0000) >> 16;

    low = temp << 16;

    temp = mult_uint16_uint16(xmsb , ylsb);
    high += (temp & 0xFFFF0000) >> 16;

    temp2 = low;
    low += temp << 16;

    if (low < temp2) {
        ++high;
    }

    temp = low;
    low += mult_uint16_uint16(xlsb, ylsb);

    if (low < temp) {
        ++high;
    }

    *pHigh = high;
    *pLow = low;
}


/****************************************************************************************************
 * @fn      ScaleSensorData
 *          Helper routine - Apply sign extension, offset and scaling to raw sensor data
 *
 ***************************************************************************************************/
static NTEXTENDED ScaleSensorData(int32_t Data, SensorDataType_t Format, uint32_t Mask, int32_t Offset, NTPRECISE ScaleFactor)
{
    int64_t llTemp;

    // apply offset
    Data -= Offset;

    Data &= Mask;               // mask off non-used data bits
    // sign extend if needed
    if(Format == SENSOR_DATA_SIGNED_TWO_COMP){
        if((Data & (~Mask >> 1)) != 0 )
            Data |= ~Mask;
    }

    llTemp = (int64_t)(Data) * (int64_t)(ScaleFactor);     // do initial scaling in 32Q24 (NTPRECISE)
    llTemp = (llTemp + FIXEDPOINT_ROUNDING_VALUE) >> (QFIXEDPOINTPRECISE-QFIXEDPOINT); //now convert to 32Q12, but leave 64 bit for range check
    if(llTemp > SPI_MAX_INT )
        llTemp = SPI_MAX_INT;               //if overflow, make max
    if(llTemp < SPI_MIN_INT )
        llTemp = SPI_MIN_INT;               //if underflow, make min
    return (NTEXTENDED)llTemp;              //return just the lower 32 bits (NTEXTENDED)
}


/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C     F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/****************************************************************************************************
 * @fn      GetTimeFromCounter
 *          Helper routine for time conversion
 *
 ***************************************************************************************************/
fm_bool_t GetTimeFromCounter(NTTIME * pTime,TIMECOEFFICIENT counterToTimeConversionFactor,uint32_t counterHigh, uint32_t counterLow)
{
    NTTIME ret = 0;
    uint32_t high1,low1;
    uint32_t high2,low2;
    const uint32_t roundfactor = (1 << (Q32DIFF-1)) ;
    if (counterToTimeConversionFactor & 0x80000000) {
        counterToTimeConversionFactor = ~counterToTimeConversionFactor + 1;
    }

    UMul32(counterToTimeConversionFactor,counterLow,&high1,&low1);
    UMul32(counterToTimeConversionFactor,counterHigh,&high2,&low2);

    low2 += high1;
    if (low2 < high1) {
        high2++;
    }


    //round things
    low1 += roundfactor;

    //check overflow
    if (low1 < roundfactor) {
        low2++;
        if (low2 ==0) {
            high2++;
        }
    }

    //right shift by Q32DIFF to make this into a Q24 number from a Q32 number
    low1 >>= Q32DIFF;
    low1 |= (low2 << (32 -  Q32DIFF) );
    low2 >>= Q32DIFF;
    low2 |= (high2 << (32 -  Q32DIFF) );
    high2 >>= Q32DIFF;

    if (high2 || low2 & 0x80000000) {
        //saturation!!!!!
        *pTime = 0x7FFFFFFFFFFFFFFFLL;
        return FALSE;
    }

    ret = low2;
    ret <<= 32;
    ret |= low1;

    *pTime = ret;

    return TRUE;
}


/****************************************************************************************************
 * @fn      ConvertSensorData
 *          Given a pointer to a raw sensor data packet from the input queue of type
 *          _SensorDataBuffer_t and a pointer to a sensor output data packet of type
 *          TriAxisSensorCookedData_t, apply translations and conversions into a format suitable
 *          to feed to the alg
 *
 ***************************************************************************************************/
int32_t ConvertSensorData(SensorDescriptor_t *pSenDesc, TriAxisSensorRawData_t *pRawData, SPI_3AxisResult_t *pResult)
{
    uint16_t i;
    _TriAxisSensorCookedData_t cookedData;

    // apply axis conversion and data width 1st, then offset, then gain (scaling), finally convert the time stamp.

    for(i = 0; i < 3; i++) {
        switch( pSenDesc->AxisMapping[i] ) {

        case AXIS_MAP_UNUSED:
            cookedData.Data[i] = 0;
            break;

        case AXIS_MAP_POSITIVE_X:

            // mask, sign extend (if needed), apply offset and scale factor
            cookedData.Data[0] = ScaleSensorData(pRawData->Data[i],
                pSenDesc->DataType,
                pSenDesc->DataWidthMask,
                pSenDesc->ConversionOffset[i],
                pSenDesc->ConversionScale[i]);
            break;

        case AXIS_MAP_NEGATIVE_X:

            // mask, sign extend (if needed), apply offset and scale factor
            cookedData.Data[0] = ScaleSensorData(pRawData->Data[i],
                pSenDesc->DataType,
                pSenDesc->DataWidthMask,
                pSenDesc->ConversionOffset[i],
                pSenDesc->ConversionScale[i]);
            cookedData.Data[0] = -cookedData.Data[0];
            break;

        case AXIS_MAP_POSITIVE_Y:

            // mask, sign extend (if needed), apply offset and scale factor
            cookedData.Data[1] = ScaleSensorData(pRawData->Data[i],
                pSenDesc->DataType,
                pSenDesc->DataWidthMask,
                pSenDesc->ConversionOffset[i],
                pSenDesc->ConversionScale[i]);
            break;

        case AXIS_MAP_NEGATIVE_Y:

            // mask, sign extend (if needed), apply offset and scale factor
            cookedData.Data[1] = ScaleSensorData(pRawData->Data[i],
                pSenDesc->DataType,
                pSenDesc->DataWidthMask,
                pSenDesc->ConversionOffset[i],
                pSenDesc->ConversionScale[i]);
            cookedData.Data[1] = -cookedData.Data[1];
            break;

        case AXIS_MAP_POSITIVE_Z:


            // mask, sign extend (if needed), apply offset and scale factor
            cookedData.Data[2] = ScaleSensorData(pRawData->Data[i],
                pSenDesc->DataType,
                pSenDesc->DataWidthMask,
                pSenDesc->ConversionOffset[i],
                pSenDesc->ConversionScale[i]);
            break;

        case AXIS_MAP_NEGATIVE_Z:

            // mask, sign extend (if needed), apply offset and scale factor
            cookedData.Data[2] = ScaleSensorData(pRawData->Data[i],
                pSenDesc->DataType,
                pSenDesc->DataWidthMask,
                pSenDesc->ConversionOffset[i],
                pSenDesc->ConversionScale[i]);
            cookedData.Data[2] = -cookedData.Data[2];
            break;


        default:
            return -1;
        }
    }

    // scale time stamp into seconds

    // check for user timestamp rollover, if so bump our timestamp extension word
    // NOTE: We use individual timestamp extensions in case the sensor data comes in out of order from one sensor to another

    switch( pSenDesc->SensorType ) {

    case SENSOR_TYPE_ACCELEROMETER:
        if( ((int32_t)_AccLastTimeStamp < 0) && ((int32_t)pRawData->TimeStamp >= 0) ) {
            _AccTimeStampExtension++;
        }
        _AccLastTimeStamp = pRawData->TimeStamp;
        GetTimeFromCounter(&cookedData.TimeStamp,gPlatformDesc.TstampConversionToSeconds,_AccTimeStampExtension, pRawData->TimeStamp);

        break;

    case SENSOR_TYPE_MAGNETIC_FIELD:
        if( ((int32_t)_MagLastTimeStamp < 0) && ((int32_t)pRawData->TimeStamp >= 0) ) {
            _MagTimeStampExtension++;
        }
        _MagLastTimeStamp = pRawData->TimeStamp;
        GetTimeFromCounter(&cookedData.TimeStamp,gPlatformDesc.TstampConversionToSeconds,_MagTimeStampExtension, pRawData->TimeStamp);
        break;

    case SENSOR_TYPE_GYROSCOPE:
        if( ((int32_t)_GyroLastTimeStamp < 0) && ((int32_t)pRawData->TimeStamp >= 0) ) {
            _GyroTimeStampExtension++;
        }
        _GyroLastTimeStamp = pRawData->TimeStamp;
        GetTimeFromCounter(&cookedData.TimeStamp,gPlatformDesc.TstampConversionToSeconds,_GyroTimeStampExtension, pRawData->TimeStamp);
        break;

    default:
        break;
    }

    memcpy(pResult, &cookedData, sizeof(_TriAxisSensorCookedData_t ));

    return 0;
}



/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
