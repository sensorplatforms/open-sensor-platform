/*
 ***************************************************************************************************
 *
 * (C) All rights reserved by ROBERT BOSCH GMBH
 *
 **************************************************************************************************/
/*  $Date: 2012/11/06
 *  $Revision: 1.3 $
 *
 */

/**************************************************************************************************
* Copyright (C) 2007 Bosch Sensortec GmbH
*
* bma2x2.c
*
* Usage:        Sensor Driver for BMA2x2 Triaxial acceleration sensor
*
**************************************************************************************************/
/*************************************************************************************************/
/*  Disclaimer
*
* Common:
* Bosch Sensortec products are developed for the consumer goods industry. They may only be used
* within the parameters of the respective valid product data sheet.  Bosch Sensortec products are
* provided with the express understanding that there is no warranty of fitness for a particular purpose.
* They are not fit for use in life-sustaining, safety or security sensitive systems or any system or device
* that may lead to bodily harm or property damage if the system or device malfunctions. In addition,
* Bosch Sensortec products are not fit for use in products which interact with motor vehicle systems.
* The resale and/or use of products are at the purchaser’s own risk and his own responsibility. The
* examination of fitness for the intended use is the sole responsibility of the Purchaser.
*
* The purchaser shall indemnify Bosch Sensortec from all third party claims, including any claims for
* incidental, or consequential damages, arising from any product use not covered by the parameters of
* the respective valid product data sheet or not approved by Bosch Sensortec and reimburse Bosch
* Sensortec for all costs in connection with such claims.
*
* The purchaser must monitor the market for the purchased products, particularly with regard to
* product safety and inform Bosch Sensortec without delay of all security relevant incidents.
*
* Engineering Samples are marked with an asterisk (*) or (e). Samples may vary from the valid
* technical specifications of the product series. They are therefore not intended or fit for resale to third
* parties or for use in end products. Their sole purpose is internal client testing. The testing of an
* engineering sample may in no way replace the testing of a product series. Bosch Sensortec
* assumes no liability for the use of engineering samples. By accepting the engineering samples, the
* Purchaser agrees to indemnify Bosch Sensortec from all claims arising from the use of engineering
* samples.
*
* Special:
* This software module (hereinafter called "Software") and any information on application-sheets
* (hereinafter called "Information") is provided free of charge for the sole purpose to support your
* application work. The Software and Information is subject to the following terms and conditions:
*
* The Software is specifically designed for the exclusive use for Bosch Sensortec products by
* personnel who have special experience and training. Do not use this Software if you do not have the
* proper experience or training.
*
* This Software package is provided `` as is `` and without any expressed or implied warranties,
* including without limitation, the implied warranties of merchantability and fitness for a particular
* purpose.
*
* Bosch Sensortec and their representatives and agents deny any liability for the functional impairment
* of this Software in terms of fitness, performance and safety. Bosch Sensortec and their
* representatives and agents shall not be liable for any direct or indirect damages or injury, except as
* otherwise stipulated in mandatory applicable law.
*
* The Information provided is believed to be accurate and reliable. Bosch Sensortec assumes no
* responsibility for the consequences of use of such Information nor for any infringement of patents or
* other rights of third parties which may result from its use. No license is granted by implication or
* otherwise under any patent or patent rights of Bosch. Specifications mentioned in the Information are
* subject to change without notice.
*
* It is not allowed to deliver the source code of the Software to any third party without permission of
* Bosch Sensortec.
*/
/*************************************************************************************************/
/*! \file <BMA2x2 >
    \brief <Sensor driver for BMA2x2> */
#include "bma2x2.h"
/* user defined code to be added here ... */
bma2x2_t * p_bma2x2;
const unsigned char V_BMA2x2RESOLUTION_U8R = BMA2x2_12_RESOLUTION; /* Based on Bit resolution value should be modified */
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API reads the data from the given register continously
 *
 *
 *
 *
 *  \param unsigned char addr, unsigned char *data
 *                       addr -> Address of the register
 *                       data -> address of the variable, read value will be kept
 *  \return  results of bus communication function
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_burst_read(unsigned char addr, unsigned char *data, unsigned int len)
   {
   BMA2x2_RETURN_FUNCTION_TYPE comres;
   if (p_bma2x2 == BMA2x2_NULL)
      {
      comres = E_BMA2x2_NULL_PTR;
      }
   else
      {
      comres = p_bma2x2->BMA2x2_BURST_READ_FUNC(p_bma2x2->dev_addr, addr, data, len);
      }
   return comres;
   }
/*******************************************************************************
 * Description: *//**\brief
 *        This function initialises the structure pointer and assigns the I2C address.
 *
 *
 *
 *
 *
 *  \param  bma2x2_t *bma2x2 structure pointer.
 *
 *
 *
 *  \return communication results.
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma2x2_init(bma2x2_t *bma2x2)
   {
   int comres= C_BMA2x2_Zero_U8X;
   unsigned char data;

   p_bma2x2 = bma2x2;                                                                             /* assign bma2x2 ptr */
   p_bma2x2->dev_addr = BMA2x2_I2C_ADDR;                                                          /* preset bma2x2 I2C_addr */
   comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_CHIP_ID__REG, &data, 1);     /* read Chip Id */
   p_bma2x2->chip_id = data ;                                          /* get bitslice */
   return comres;
   }
/*******************************************************************************
 * Description: *//**\brief This API gives data to the given register and
 *                          the data is written in the corresponding register address
 *
 *
 *
 *
 *  \param unsigned char addr, unsigned char data, unsigned char len
 *          addr -> Address of the register
 *          data -> Data to be written to the register
 *          len  -> Length of the Data
 *
 *
 *
 *  \return communication results.
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma2x2_write_reg(unsigned char addr, unsigned char *data, unsigned char len)
   {
   int comres = C_BMA2x2_Zero_U8X ;
   if (p_bma2x2 == C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, addr, data, len);
      }
   return comres;
   }
/*******************************************************************************
 * Description: *//**\brief This API reads the data from the given register address
 *
 *
 *
 *
 *  \param unsigned char addr, unsigned char *data, unsigned char len
 *         addr -> Address of the register
 *         data -> address of the variable, read value will be kept
 *         len  -> Length of the data
 *
 *
 *
 *
 *  \return results of communication routine
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma2x2_read_reg(unsigned char addr, unsigned char *data, unsigned char len)
   {
   int comres = C_BMA2x2_Zero_U8X ;
   if (p_bma2x2 == C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, addr, data, len);
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API reads acceleration data X values
 *                          from location 02h and 03h
 *
 *
 *
 *
 *  \param short  *a_x   :  Address of a_x
 *
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma2x2_read_accel_x(short *a_x)
   {
   int comres = 0;
   unsigned char data[2];
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      switch (V_BMA2x2RESOLUTION_U8R)
         {
         case BMA2x2_12_RESOLUTION:
            comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_ACC_X12_LSB__REG, data, 2);
            *a_x = BMA2x2_GET_BITSLICE(data[0],BMA2x2_ACC_X12_LSB)| (BMA2x2_GET_BITSLICE(data[1],BMA2x2_ACC_X_MSB)<<(BMA2x2_ACC_X12_LSB__LEN));
            *a_x = *a_x << (sizeof(short)*8-(BMA2x2_ACC_X12_LSB__LEN + BMA2x2_ACC_X_MSB__LEN));
            *a_x = *a_x >> (sizeof(short)*8-(BMA2x2_ACC_X12_LSB__LEN + BMA2x2_ACC_X_MSB__LEN));
            break;
         case BMA2x2_10_RESOLUTION:
            comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_ACC_X10_LSB__REG, data, 2);
            *a_x = BMA2x2_GET_BITSLICE(data[0],BMA2x2_ACC_X10_LSB)| (BMA2x2_GET_BITSLICE(data[1],BMA2x2_ACC_X_MSB)<<(BMA2x2_ACC_X10_LSB__LEN));
            *a_x = *a_x << (sizeof(short)*8-(BMA2x2_ACC_X10_LSB__LEN + BMA2x2_ACC_X_MSB__LEN));
            *a_x = *a_x >> (sizeof(short)*8-(BMA2x2_ACC_X10_LSB__LEN + BMA2x2_ACC_X_MSB__LEN));
            break;
         case BMA2x2_8_RESOLUTION:
            comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_ACC_X8_LSB__REG, data, 2);
            *a_x = BMA2x2_GET_BITSLICE(data[0],BMA2x2_ACC_X8_LSB)| (BMA2x2_GET_BITSLICE(data[1],BMA2x2_ACC_X_MSB)<<(BMA2x2_ACC_X8_LSB__LEN));
            *a_x = *a_x << (sizeof(short)*8-(BMA2x2_ACC_X8_LSB__LEN + BMA2x2_ACC_X_MSB__LEN));
            *a_x = *a_x >> (sizeof(short)*8-(BMA2x2_ACC_X8_LSB__LEN + BMA2x2_ACC_X_MSB__LEN));
            break;
         case BMA2x2_14_RESOLUTION:
            comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_ACC_X14_LSB__REG, data, 2);
            *a_x = BMA2x2_GET_BITSLICE(data[0],BMA2x2_ACC_X14_LSB)| (BMA2x2_GET_BITSLICE(data[1],BMA2x2_ACC_X_MSB)<<(BMA2x2_ACC_X14_LSB__LEN));
            *a_x = *a_x << (sizeof(short)*8-(BMA2x2_ACC_X14_LSB__LEN + BMA2x2_ACC_X_MSB__LEN));
            *a_x = *a_x >> (sizeof(short)*8-(BMA2x2_ACC_X14_LSB__LEN + BMA2x2_ACC_X_MSB__LEN));
            break;
         default:
            break;
         }
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API reads acceleration data Y values
 *                          from location 04h and 05h
 *
 *
 *
 *
 *  \param short  *a_y   :  Address of a_y
 *
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma2x2_read_accel_y(short *a_y)
   {
   int comres = 0;
   unsigned char data[2];
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      switch (V_BMA2x2RESOLUTION_U8R)
         {
         case BMA2x2_12_RESOLUTION:
            comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_ACC_Y12_LSB__REG, data, 2);
            *a_y = BMA2x2_GET_BITSLICE(data[0],BMA2x2_ACC_Y12_LSB)| (BMA2x2_GET_BITSLICE(data[1],BMA2x2_ACC_Y_MSB)<<(BMA2x2_ACC_Y12_LSB__LEN ));
            *a_y = *a_y << (sizeof(short)*8-(BMA2x2_ACC_Y12_LSB__LEN + BMA2x2_ACC_Y_MSB__LEN));
            *a_y = *a_y >> (sizeof(short)*8-(BMA2x2_ACC_Y12_LSB__LEN + BMA2x2_ACC_Y_MSB__LEN));
            break;
         case BMA2x2_10_RESOLUTION:
            comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_ACC_Y10_LSB__REG, data, 2);
            *a_y = BMA2x2_GET_BITSLICE(data[0],BMA2x2_ACC_Y10_LSB)| (BMA2x2_GET_BITSLICE(data[1],BMA2x2_ACC_Y_MSB)<<(BMA2x2_ACC_Y10_LSB__LEN ));
            *a_y = *a_y << (sizeof(short)*8-(BMA2x2_ACC_Y10_LSB__LEN + BMA2x2_ACC_Y_MSB__LEN));
            *a_y = *a_y >> (sizeof(short)*8-(BMA2x2_ACC_Y10_LSB__LEN + BMA2x2_ACC_Y_MSB__LEN));
            break;
         case BMA2x2_8_RESOLUTION:
            comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_ACC_Y8_LSB__REG, data, 2);
            *a_y = BMA2x2_GET_BITSLICE(data[0],BMA2x2_ACC_Y8_LSB)| (BMA2x2_GET_BITSLICE(data[1],BMA2x2_ACC_Y_MSB)<<(BMA2x2_ACC_Y8_LSB__LEN ));
            *a_y = *a_y << (sizeof(short)*8-(BMA2x2_ACC_Y8_LSB__LEN + BMA2x2_ACC_Y_MSB__LEN));
            *a_y = *a_y >> (sizeof(short)*8-(BMA2x2_ACC_Y8_LSB__LEN + BMA2x2_ACC_Y_MSB__LEN));
            break;
         case BMA2x2_14_RESOLUTION:
            comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_ACC_Y14_LSB__REG, data, 2);
            *a_y = BMA2x2_GET_BITSLICE(data[0],BMA2x2_ACC_Y14_LSB)| (BMA2x2_GET_BITSLICE(data[1],BMA2x2_ACC_Y_MSB)<<(BMA2x2_ACC_Y14_LSB__LEN ));
            *a_y = *a_y << (sizeof(short)*8-(BMA2x2_ACC_Y14_LSB__LEN + BMA2x2_ACC_Y_MSB__LEN));
            *a_y = *a_y >> (sizeof(short)*8-(BMA2x2_ACC_Y14_LSB__LEN + BMA2x2_ACC_Y_MSB__LEN));
            break;
         default:
            break;
         }
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API reads acceleration data Z values
 *                          from location 06h and 07h
 *
 *
 *
 *
 *  \param short  *a_z   :  Address of a_z
 *
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma2x2_read_accel_z(short *a_z)
   {
   int comres = 0;
   unsigned char data[2];
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      switch (V_BMA2x2RESOLUTION_U8R)
         {
         case BMA2x2_12_RESOLUTION:
            comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_ACC_Z12_LSB__REG, data, 2);
            *a_z = BMA2x2_GET_BITSLICE(data[0],BMA2x2_ACC_Z12_LSB)| (BMA2x2_GET_BITSLICE(data[1],BMA2x2_ACC_Z_MSB)<<(BMA2x2_ACC_Z12_LSB__LEN));
            *a_z = *a_z << (sizeof(short)*8-(BMA2x2_ACC_Z12_LSB__LEN + BMA2x2_ACC_Z_MSB__LEN));
            *a_z = *a_z >> (sizeof(short)*8-(BMA2x2_ACC_Z12_LSB__LEN + BMA2x2_ACC_Z_MSB__LEN));
            break;
         case BMA2x2_10_RESOLUTION:
            comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_ACC_Z10_LSB__REG, data, 2);
            *a_z = BMA2x2_GET_BITSLICE(data[0],BMA2x2_ACC_Z10_LSB)| (BMA2x2_GET_BITSLICE(data[1],BMA2x2_ACC_Z_MSB)<<(BMA2x2_ACC_Z10_LSB__LEN));
            *a_z = *a_z << (sizeof(short)*8-(BMA2x2_ACC_Z10_LSB__LEN + BMA2x2_ACC_Z_MSB__LEN));
            *a_z = *a_z >> (sizeof(short)*8-(BMA2x2_ACC_Z10_LSB__LEN + BMA2x2_ACC_Z_MSB__LEN));
            break;
         case BMA2x2_8_RESOLUTION:
            comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_ACC_Z8_LSB__REG, data, 2);
            *a_z = BMA2x2_GET_BITSLICE(data[0],BMA2x2_ACC_Z8_LSB)| (BMA2x2_GET_BITSLICE(data[1],BMA2x2_ACC_Z_MSB)<<(BMA2x2_ACC_Z8_LSB__LEN));
            *a_z = *a_z << (sizeof(short)*8-(BMA2x2_ACC_Z8_LSB__LEN + BMA2x2_ACC_Z_MSB__LEN));
            *a_z = *a_z >> (sizeof(short)*8-(BMA2x2_ACC_Z8_LSB__LEN + BMA2x2_ACC_Z_MSB__LEN));
            break;
         case BMA2x2_14_RESOLUTION:
            comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_ACC_Z14_LSB__REG, data, 2);
            *a_z = BMA2x2_GET_BITSLICE(data[0],BMA2x2_ACC_Z14_LSB)| (BMA2x2_GET_BITSLICE(data[1],BMA2x2_ACC_Z_MSB)<<(BMA2x2_ACC_Z14_LSB__LEN));
            *a_z = *a_z << (sizeof(short)*8-(BMA2x2_ACC_Z14_LSB__LEN + BMA2x2_ACC_Z_MSB__LEN));
            *a_z = *a_z >> (sizeof(short)*8-(BMA2x2_ACC_Z14_LSB__LEN + BMA2x2_ACC_Z_MSB__LEN));
            break;
         default:
            break;
         }
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API reads acceleration data X,Y,Z values
 *                          from location 02h to 07h
 *
 *
 *
 *
 *  \param bma2x2acc_t * acc : Address of bma2x2acc_t
 *
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma2x2_read_accel_xyz(bma2x2acc_t * acc)
   {
   int comres = 0;
   unsigned char data[6];
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      switch (V_BMA2x2RESOLUTION_U8R)
         {
         case BMA2x2_12_RESOLUTION:
            comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_ACC_X12_LSB__REG, data, 6);
            acc->x = BMA2x2_GET_BITSLICE(data[0],BMA2x2_ACC_X12_LSB)| (BMA2x2_GET_BITSLICE(data[1],BMA2x2_ACC_X_MSB)<<(BMA2x2_ACC_X12_LSB__LEN));
            acc->x = acc->x << (sizeof(short)*8-(BMA2x2_ACC_X12_LSB__LEN + BMA2x2_ACC_X_MSB__LEN));
            acc->x = acc->x >> (sizeof(short)*8-(BMA2x2_ACC_X12_LSB__LEN + BMA2x2_ACC_X_MSB__LEN));

            acc->y = BMA2x2_GET_BITSLICE(data[2],BMA2x2_ACC_Y12_LSB)| (BMA2x2_GET_BITSLICE(data[3],BMA2x2_ACC_Y_MSB)<<(BMA2x2_ACC_Y12_LSB__LEN ));
            acc->y = acc->y << (sizeof(short)*8-(BMA2x2_ACC_Y12_LSB__LEN + BMA2x2_ACC_Y_MSB__LEN));
            acc->y = acc->y >> (sizeof(short)*8-(BMA2x2_ACC_Y12_LSB__LEN + BMA2x2_ACC_Y_MSB__LEN));

            acc->z = BMA2x2_GET_BITSLICE(data[4],BMA2x2_ACC_Z12_LSB)| (BMA2x2_GET_BITSLICE(data[5],BMA2x2_ACC_Z_MSB)<<(BMA2x2_ACC_Z12_LSB__LEN));
            acc->z = acc->z << (sizeof(short)*8-(BMA2x2_ACC_Z12_LSB__LEN + BMA2x2_ACC_Z_MSB__LEN));
            acc->z = acc->z >> (sizeof(short)*8-(BMA2x2_ACC_Z12_LSB__LEN + BMA2x2_ACC_Z_MSB__LEN));
            break;
         case BMA2x2_10_RESOLUTION:
            comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_ACC_X10_LSB__REG, data, 6);
            acc->x = BMA2x2_GET_BITSLICE(data[0],BMA2x2_ACC_X10_LSB)| (BMA2x2_GET_BITSLICE(data[1],BMA2x2_ACC_X_MSB)<<(BMA2x2_ACC_X10_LSB__LEN));
            acc->x = acc->x << (sizeof(short)*8-(BMA2x2_ACC_X10_LSB__LEN + BMA2x2_ACC_X_MSB__LEN));
            acc->x = acc->x >> (sizeof(short)*8-(BMA2x2_ACC_X10_LSB__LEN + BMA2x2_ACC_X_MSB__LEN));

            acc->y = BMA2x2_GET_BITSLICE(data[2],BMA2x2_ACC_Y10_LSB)| (BMA2x2_GET_BITSLICE(data[3],BMA2x2_ACC_Y_MSB)<<(BMA2x2_ACC_Y10_LSB__LEN ));
            acc->y = acc->y << (sizeof(short)*8-(BMA2x2_ACC_Y10_LSB__LEN + BMA2x2_ACC_Y_MSB__LEN));
            acc->y = acc->y >> (sizeof(short)*8-(BMA2x2_ACC_Y10_LSB__LEN + BMA2x2_ACC_Y_MSB__LEN));

            acc->z = BMA2x2_GET_BITSLICE(data[4],BMA2x2_ACC_Z10_LSB)| (BMA2x2_GET_BITSLICE(data[5],BMA2x2_ACC_Z_MSB)<<(BMA2x2_ACC_Z10_LSB__LEN));
            acc->z = acc->z << (sizeof(short)*8-(BMA2x2_ACC_Z10_LSB__LEN + BMA2x2_ACC_Z_MSB__LEN));
            acc->z = acc->z >> (sizeof(short)*8-(BMA2x2_ACC_Z10_LSB__LEN + BMA2x2_ACC_Z_MSB__LEN));
            break;
         case BMA2x2_8_RESOLUTION:
            comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_ACC_X8_LSB__REG, data, 6);
            acc->x = BMA2x2_GET_BITSLICE(data[0],BMA2x2_ACC_X8_LSB)| (BMA2x2_GET_BITSLICE(data[1],BMA2x2_ACC_X_MSB)<<(BMA2x2_ACC_X8_LSB__LEN));
            acc->x = acc->x << (sizeof(short)*8-(BMA2x2_ACC_X8_LSB__LEN + BMA2x2_ACC_X_MSB__LEN));
            acc->x = acc->x >> (sizeof(short)*8-(BMA2x2_ACC_X8_LSB__LEN + BMA2x2_ACC_X_MSB__LEN));

            acc->y = BMA2x2_GET_BITSLICE(data[2],BMA2x2_ACC_Y8_LSB)| (BMA2x2_GET_BITSLICE(data[3],BMA2x2_ACC_Y_MSB)<<(BMA2x2_ACC_Y8_LSB__LEN ));
            acc->y = acc->y << (sizeof(short)*8-(BMA2x2_ACC_Y8_LSB__LEN + BMA2x2_ACC_Y_MSB__LEN));
            acc->y = acc->y >> (sizeof(short)*8-(BMA2x2_ACC_Y8_LSB__LEN + BMA2x2_ACC_Y_MSB__LEN));

            acc->z = BMA2x2_GET_BITSLICE(data[4],BMA2x2_ACC_Z8_LSB)| (BMA2x2_GET_BITSLICE(data[5],BMA2x2_ACC_Z_MSB)<<(BMA2x2_ACC_Z8_LSB__LEN));
            acc->z = acc->z << (sizeof(short)*8-(BMA2x2_ACC_Z8_LSB__LEN + BMA2x2_ACC_Z_MSB__LEN));
            acc->z = acc->z >> (sizeof(short)*8-(BMA2x2_ACC_Z8_LSB__LEN + BMA2x2_ACC_Z_MSB__LEN));
            break;
         case BMA2x2_14_RESOLUTION:
            comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_ACC_X14_LSB__REG, data, 6);
            acc->x = BMA2x2_GET_BITSLICE(data[0],BMA2x2_ACC_X14_LSB)| (BMA2x2_GET_BITSLICE(data[1],BMA2x2_ACC_X_MSB)<<(BMA2x2_ACC_X14_LSB__LEN));
            acc->x = acc->x << (sizeof(short)*8-(BMA2x2_ACC_X14_LSB__LEN + BMA2x2_ACC_X_MSB__LEN));
            acc->x = acc->x >> (sizeof(short)*8-(BMA2x2_ACC_X14_LSB__LEN + BMA2x2_ACC_X_MSB__LEN));

            acc->y = BMA2x2_GET_BITSLICE(data[2],BMA2x2_ACC_Y14_LSB)| (BMA2x2_GET_BITSLICE(data[3],BMA2x2_ACC_Y_MSB)<<(BMA2x2_ACC_Y14_LSB__LEN ));
            acc->y = acc->y << (sizeof(short)*8-(BMA2x2_ACC_Y14_LSB__LEN + BMA2x2_ACC_Y_MSB__LEN));
            acc->y = acc->y >> (sizeof(short)*8-(BMA2x2_ACC_Y14_LSB__LEN + BMA2x2_ACC_Y_MSB__LEN));

            acc->z = BMA2x2_GET_BITSLICE(data[4],BMA2x2_ACC_Z14_LSB)| (BMA2x2_GET_BITSLICE(data[5],BMA2x2_ACC_Z_MSB)<<(BMA2x2_ACC_Z14_LSB__LEN));
            acc->z = acc->z << (sizeof(short)*8-(BMA2x2_ACC_Z14_LSB__LEN + BMA2x2_ACC_Z_MSB__LEN));
            acc->z = acc->z >> (sizeof(short)*8-(BMA2x2_ACC_Z14_LSB__LEN + BMA2x2_ACC_Z_MSB__LEN));
            break;
         default:
            break;
         }
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API Reads tap slope status register byte
 *                          from location 0Bh
 *
 *
 *
 *
 *  \param unsigned char * status_tap : Address of status_tap register
 *
 *
 *
 *  \return Result of bus communication function
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma2x2_get_int_tap_status(unsigned char * status_tap)
   {
   int comres = C_BMA2x2_Zero_U8X;
   if (p_bma2x2 == C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_STATUS_TAP_SLOPE_REG, status_tap, C_BMA2x2_One_U8X);
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API Reads orient status register byte
 *                          from location 0Ch
 *
 *
 *
 *
 *  \param unsigned char *status_orient : Address of status_orient register
 *
 *
 *
 *  \return Result of bus communication function
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma2x2_get_int_orient_status(unsigned char *status_orient)
   {
   int comres = C_BMA2x2_Zero_U8X;
   if (p_bma2x2 == C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_STATUS_ORIENT_HIGH_REG, status_orient, C_BMA2x2_One_U8X);
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API Reads fifo status register byte
 *                          from location 0Eh
 *
 *
 *
 *
 *  \param unsigned char *status_fifo : Address of status_fifo register
 *
 *
 *
 *  \return Result of bus communication function
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma2x2_get_fifo_status(unsigned char *status_fifo)
   {
   int comres = C_BMA2x2_Zero_U8X;
   if (p_bma2x2 == C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_STATUS_FIFO_REG, status_fifo, C_BMA2x2_One_U8X);
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API Reads fifo framecount bits from location 0Eh
 *
 *
 *
 *
 *  \param unsigned char *framecount : Address of framecount
 *
 *
 *
 *  \return Result of bus communication function
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma2x2_get_fifo_framecount(unsigned char *framecount)
   {
   int comres = C_BMA2x2_Zero_U8X;
   unsigned char data;
   if (p_bma2x2 == C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_FIFO_FRAME_COUNTER_S__REG, &data, C_BMA2x2_One_U8X);
      *framecount = BMA2x2_GET_BITSLICE(data,BMA2x2_FIFO_FRAME_COUNTER_S);
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API Reads fifo overrun bits from location 0Eh
 *
 *
 *
 *
 *  \param unsigned char *overrun : Address of overrun
 *
 *
 *
 *  \return Result of bus communication function
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma2x2_get_fifo_overrun(unsigned char *overrun)
   {
   int comres = C_BMA2x2_Zero_U8X;
   unsigned char data;
   if (p_bma2x2 == C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_FIFO_OVERRUN_S__REG, &data, C_BMA2x2_One_U8X);
      *overrun = BMA2x2_GET_BITSLICE(data,BMA2x2_FIFO_OVERRUN_S);
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API Reads interrupt status register byte
 *                          from location 09h
 *
 *
 *
 *
 *  \param unsigned char * status : Address of status register
 *
 *
 *
 *  \return Result of bus communication function
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma2x2_get_interrupt_status(unsigned char * status)
   {
   int comres = C_BMA2x2_Zero_U8X;
   if (p_bma2x2 == C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_STATUS1_REG, status, C_BMA2x2_Four_U8X);
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/**************************************************************************************
 * Description: *//**\brief This API is used to get the Ranges(g values) of the sensor
 *
 *
 *
 *
 *  \param unsigned char * Range : Address of Range
 *                        3 -> 2G
 *                        5 -> 4G
 *                        8 -> 8G
 *                       12 -> 16G
 *
 *
 *
 *  \return
 *
 *
 ***************************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma2x2_get_range(unsigned char * Range )
   {
   int comres = C_BMA2x2_Zero_U8X ;
   unsigned char data;
   if (p_bma2x2 == C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_RANGE_SEL__REG, &data, C_BMA2x2_One_U8X );
      data = BMA2x2_GET_BITSLICE(data, BMA2x2_RANGE_SEL);
      *Range = data;
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/***********************************************************************************
 * Description: *//**\brief This API is used to set Ranges(g value) of the sensor
 *
 *
 *
 *
 *  \param unsigned char Range
 *                        3 -> 2G
 *                        5 -> 4G
 *                        8 -> 8G
 *                       12 -> 16G
 *
 *  \return communication results
 *
 *
 ************************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma2x2_set_range(unsigned char Range)
   {
   int comres=C_BMA2x2_Zero_U8X ;
   unsigned char data1;
   if (p_bma2x2 == C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      if ((Range == C_BMA2x2_Three_U8X) || (Range == C_BMA2x2_Five_U8X) || (Range == C_BMA2x2_Eight_U8X) || (Range == C_BMA2x2_Twelve_U8X))
         {
         comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_RANGE_SEL_REG, &data1, C_BMA2x2_One_U8X );
         switch (Range)
            {
            case BMA2x2_RANGE_2G:
               data1  = BMA2x2_SET_BITSLICE(data1, BMA2x2_RANGE_SEL, C_BMA2x2_Three_U8X);
               break;
            case BMA2x2_RANGE_4G:
               data1  = BMA2x2_SET_BITSLICE(data1, BMA2x2_RANGE_SEL, C_BMA2x2_Five_U8X);
               break;
            case BMA2x2_RANGE_8G:
               data1  = BMA2x2_SET_BITSLICE(data1, BMA2x2_RANGE_SEL, C_BMA2x2_Eight_U8X);
               break;
            case BMA2x2_RANGE_16G:
               data1  = BMA2x2_SET_BITSLICE(data1, BMA2x2_RANGE_SEL, C_BMA2x2_Twelve_U8X);
               break;
            default:
               break;
            }
         comres += p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_RANGE_SEL_REG, &data1, C_BMA2x2_One_U8X);
         }
      else
         {
         comres = E_OUT_OF_RANGE ;
         }
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/***********************************************************************************
 * Description: *//**\brief This API is used to get the bandwidth of the sensor
 *
 *
 *
 *
 *  \param  unsigned char * BW : Address of * BW
 *                       8 -> 7.81HZ
 *                       9 -> 15.63HZ
 *                      10 -> 31.25HZ
 *                      11 -> 62.50HZ
 *                      12 -> 125HZ
 *                      13 -> 250HZ
 *                      14 -> 500HZ
 *                      15 -> 1000HZ
 *
 *
 *
 *  \return
 *
 *
 ************************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma2x2_get_bandwidth(unsigned char * BW)
   {
   int comres = C_BMA2x2_Zero_U8X ;
   unsigned char data;
   if (p_bma2x2 == C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_BANDWIDTH__REG, &data, C_BMA2x2_One_U8X );
      data = BMA2x2_GET_BITSLICE(data, BMA2x2_BANDWIDTH);
      *BW = data ;
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to set Bandwidth of the sensor
 *
 *
 *
 *
 *  \param unsigned char BW
 *                       8 -> 7.81HZ
 *                       9 -> 15.63HZ
 *                      10 -> 31.25HZ
 *                      11 -> 62.50HZ
 *                      12 -> 125HZ
 *                      13 -> 250HZ
 *                      14 -> 500HZ
 *                      15 -> 1000HZ
 *
 *
 *
 *
 *
 *  \return communication results
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma2x2_set_bandwidth(unsigned char BW)
   {
   int comres = C_BMA2x2_Zero_U8X ;
   unsigned char data;
   int Bandwidth ;
   if (p_bma2x2 == C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      if (p_bma2x2->chip_id == 0xFB)
         {
         if (BW > C_BMA2x2_Seven_U8X && BW < C_BMA2x2_Fifteen_U8X)
            {
            switch (BW)
               {
               case BMA2x2_BW_7_81HZ:
                  Bandwidth = BMA2x2_BW_7_81HZ;

                  /*  7.81 Hz      64000 uS   */
                  break;
               case BMA2x2_BW_15_63HZ:
                  Bandwidth = BMA2x2_BW_15_63HZ;

                  /*  15.63 Hz     32000 uS   */
                  break;
               case BMA2x2_BW_31_25HZ:
                  Bandwidth = BMA2x2_BW_31_25HZ;

                  /*  31.25 Hz     16000 uS   */
                  break;
               case BMA2x2_BW_62_50HZ:
                  Bandwidth = BMA2x2_BW_62_50HZ;

                  /*  62.50 Hz     8000 uS   */
                  break;
               case BMA2x2_BW_125HZ:
                  Bandwidth = BMA2x2_BW_125HZ;

                  /*  125 Hz       4000 uS   */
                  break;
               case BMA2x2_BW_250HZ:
                  Bandwidth = BMA2x2_BW_250HZ;

                  /*  250 Hz       2000 uS   */
                  break;
               case BMA2x2_BW_500HZ:
                  Bandwidth = BMA2x2_BW_500HZ;

                  /*  500 Hz       1000 uS   */
                  break;
               default:
                  break;
               }
            comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_BANDWIDTH__REG, &data, C_BMA2x2_One_U8X );
            data = BMA2x2_SET_BITSLICE(data, BMA2x2_BANDWIDTH, Bandwidth );
            comres += p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_BANDWIDTH__REG, &data, C_BMA2x2_One_U8X );
            }
         else
            {
            comres = E_OUT_OF_RANGE ;
            }
         }
      else
         {
         if (BW > C_BMA2x2_Seven_U8X && BW < C_BMA2x2_Sixteen_U8X)
            {
            switch (BW)
               {
               case BMA2x2_BW_7_81HZ:
                  Bandwidth = BMA2x2_BW_7_81HZ;

                  /*  7.81 Hz      64000 uS   */
                  break;
               case BMA2x2_BW_15_63HZ:
                  Bandwidth = BMA2x2_BW_15_63HZ;

                  /*  15.63 Hz     32000 uS   */
                  break;
               case BMA2x2_BW_31_25HZ:
                  Bandwidth = BMA2x2_BW_31_25HZ;

                  /*  31.25 Hz     16000 uS   */
                  break;
               case BMA2x2_BW_62_50HZ:
                  Bandwidth = BMA2x2_BW_62_50HZ;

                  /*  62.50 Hz     8000 uS   */
                  break;
               case BMA2x2_BW_125HZ:
                  Bandwidth = BMA2x2_BW_125HZ;

                  /*  125 Hz       4000 uS   */
                  break;
               case BMA2x2_BW_250HZ:
                  Bandwidth = BMA2x2_BW_250HZ;

                  /*  250 Hz       2000 uS   */
                  break;
               case BMA2x2_BW_500HZ:
                  Bandwidth = BMA2x2_BW_500HZ;

                  /*  500 Hz       1000 uS   */
                  break;
               case BMA2x2_BW_1000HZ:
                  Bandwidth = BMA2x2_BW_1000HZ;

                  /*  1000 Hz      500 uS   */
                  break;
               default:
                  break;
               }
            comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_BANDWIDTH__REG, &data, C_BMA2x2_One_U8X );
            data = BMA2x2_SET_BITSLICE(data, BMA2x2_BANDWIDTH, Bandwidth );
            comres += p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_BANDWIDTH__REG, &data, C_BMA2x2_One_U8X );
            }
         else
            {
            comres = E_OUT_OF_RANGE ;
            }
         }
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/***************************************************************************************
 * Description: *//**\brief This API is used to get the operating modes of the sensor
 *
 *
 *
 *
 *  \param unsigned char * Mode : Address of Mode
 *                       0 -> NORMAL
 *                       1 -> LOWPOWER1
 *                       2 -> SUSPEND
 *                       3 -> DEEP_SUSPEND
 *                       4 -> LOWPOWER2
 *                       5 -> STANDBY
 *
 *
 *
 *  \return
 *
 *
 **************************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma2x2_get_mode(unsigned char * Mode )
   {
   int comres=C_BMA2x2_Zero_U8X ;
   unsigned char data1,data2;
   if (p_bma2x2 == C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_MODE_CTRL_REG, &data1, C_BMA2x2_One_U8X );
      comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_LOW_NOISE_CTRL_REG, &data2, C_BMA2x2_One_U8X );

      data1  = (data1 & 0xE0) >> 5;
      data2  = (data2 & 0x40) >> 6;

      //*Mode  = (*Mode) >> C_Six_U8X;
      if ((data1 == 0x00) && (data2 == 0x00))
         {
         *Mode  = BMA2x2_MODE_NORMAL;
         }
      else
         {
         if ((data1 == 0x02) && (data2 == 0x00))
            {
            *Mode  = BMA2x2_MODE_LOWPOWER1;
            }
         else
            {
            if ((data1 == 0x04 || data1 == 0x06) && (data2 == 0x00))
               {
               *Mode  = BMA2x2_MODE_SUSPEND;
               }
            else
               {
               if (((data1 & 0x01) == 0x01))
                  {
                  *Mode  = BMA2x2_MODE_DEEP_SUSPEND;
                  }
               else
                  {
                  if ((data1 == 0x02) && (data2 == 0x01))
                     {
                     *Mode  = BMA2x2_MODE_LOWPOWER2;
                     }
                  else
                     {
                     if ((data1 == 0x04) && (data2 == 0x01))
                        {
                        *Mode  = BMA2x2_MODE_STANDBY;
                        }
                     else
                        {
                        *Mode = BMA2x2_MODE_DEEP_SUSPEND ;
                        }
                     }
                  }
               }
            }
         }
      }
   p_bma2x2->mode = *Mode;
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/***************************************************************************************
 * Description: *//**\brief This API is used to set the operating Modes of the sensor
 *
 *
 *
 *
 *  \param unsigned char Mode
 *                       0 -> NORMAL
 *                       1 -> LOWPOWER1
 *                       2 -> SUSPEND
 *                       3 -> DEEP_SUSPEND
 *                       4 -> LOWPOWER2
 *                       5 -> STANDBY
 *
 *  \return communication results
 *
 *
 ***************************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
unsigned char bma2x2_set_mode(unsigned char Mode)
   {
   int comres=C_BMA2x2_Zero_U8X ;
   unsigned char data1,data2;
   if (p_bma2x2 == C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      if (Mode < C_BMA2x2_Six_U8X)
         {
         comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_MODE_CTRL_REG, &data1, C_BMA2x2_One_U8X );
         comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_LOW_NOISE_CTRL_REG, &data2, C_BMA2x2_One_U8X );
         switch (Mode)
            {
            case BMA2x2_MODE_NORMAL:
				data1  = BMA2x2_SET_BITSLICE(data1, BMA2x2_MODE_CTRL, C_BMA2x2_Zero_U8X);
				data2  = BMA2x2_SET_BITSLICE(data2, BMA2x2_LOW_POWER_MODE, C_BMA2x2_Zero_U8X);
				comres += p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_MODE_CTRL_REG, &data1, C_BMA2x2_One_U8X);
				p_bma2x2->delay_msec(1);/*A minimum delay of atleast 450us is required for the low power modes, as per the data sheet.*/
				comres += p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_LOW_NOISE_CTRL_REG, &data2, C_BMA2x2_One_U8X);
				break;
            case BMA2x2_MODE_LOWPOWER1:
				data1  = BMA2x2_SET_BITSLICE(data1, BMA2x2_MODE_CTRL, C_BMA2x2_Two_U8X);
				data2  = BMA2x2_SET_BITSLICE(data2, BMA2x2_LOW_POWER_MODE, C_BMA2x2_Zero_U8X);
				comres += p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_MODE_CTRL_REG, &data1, C_BMA2x2_One_U8X);
				p_bma2x2->delay_msec(1);/*A minimum delay of atleast 450us is required for the low power modes, as per the data sheet.*/
				comres += p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_LOW_NOISE_CTRL_REG, &data2, C_BMA2x2_One_U8X);
				break;
				case BMA2x2_MODE_SUSPEND:
				data1  = BMA2x2_SET_BITSLICE(data1, BMA2x2_MODE_CTRL, C_BMA2x2_Four_U8X);
				data2  = BMA2x2_SET_BITSLICE(data2, BMA2x2_LOW_POWER_MODE, C_BMA2x2_Zero_U8X);
				comres += p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_LOW_NOISE_CTRL_REG, &data2, C_BMA2x2_One_U8X);
				p_bma2x2->delay_msec(1);/*A minimum delay of atleast 450us is required for the low power modes, as per the data sheet.*/
				comres += p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_MODE_CTRL_REG, &data1, C_BMA2x2_One_U8X);
               break;
				case BMA2x2_MODE_DEEP_SUSPEND:
				data1  = BMA2x2_SET_BITSLICE(data1, BMA2x2_MODE_CTRL, C_BMA2x2_One_U8X);
				data2  = BMA2x2_SET_BITSLICE(data2, BMA2x2_LOW_POWER_MODE, C_BMA2x2_One_U8X);
				comres += p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_MODE_CTRL_REG, &data1, C_BMA2x2_One_U8X);
				p_bma2x2->delay_msec(1);/*A minimum delay of atleast 450us is required for the low power modes, as per the data sheet.*/
				comres += p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_LOW_NOISE_CTRL_REG, &data2, C_BMA2x2_One_U8X);
				break;
				case BMA2x2_MODE_LOWPOWER2:
				data1  = BMA2x2_SET_BITSLICE(data1, BMA2x2_MODE_CTRL, C_BMA2x2_Two_U8X);
				data2  = BMA2x2_SET_BITSLICE(data2, BMA2x2_LOW_POWER_MODE, C_BMA2x2_One_U8X);
				comres += p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_MODE_CTRL_REG, &data1, C_BMA2x2_One_U8X);
				p_bma2x2->delay_msec(1);/*A minimum delay of atleast 450us is required for the low power modes, as per the data sheet.*/
				comres += p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_LOW_NOISE_CTRL_REG, &data2, C_BMA2x2_One_U8X);
				break;
				case BMA2x2_MODE_STANDBY:
               data1  = BMA2x2_SET_BITSLICE(data1, BMA2x2_MODE_CTRL, C_BMA2x2_Four_U8X);
               data2  = BMA2x2_SET_BITSLICE(data2, BMA2x2_LOW_POWER_MODE, C_BMA2x2_One_U8X);
				comres += p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_LOW_NOISE_CTRL_REG, &data2, C_BMA2x2_One_U8X);
				p_bma2x2->delay_msec(1);/*A minimum delay of atleast 450us is required for the low power modes, as per the data sheet.*/
				comres += p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_MODE_CTRL_REG, &data1, C_BMA2x2_One_U8X);
               break;
				}
				}
      else
         {
         comres = E_OUT_OF_RANGE ;
         }
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/********************************************************************************************
 * Description: *//**\brief This API is used to get the sleep duration status of the sensor
 *
 *
 *
 *
 *  \param  unsigned char *sleep_dur : Address of sleep_dur
 *                       5 -> 0.5MS
 *                       6 -> 1MS
 *                       7 -> 2MS
 *                       8 -> 4MS
 *                       9 -> 6MS
 *                      10 -> 10MS
 *                      11 -> 25MS
 *                      12 -> 50MS
 *                      13 -> 100MS
 *                      14 -> 500MS
 *                      15 -> 1S
 *
 *
 *
 *  \return
 *
 *
 *********************************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_sleep_dur(unsigned char *sleep_dur)
   {
   unsigned char data;
   int comres = C_BMA2x2_Zero_U8X;
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      //SLEEP DURATION
      comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_SLEEP_DUR__REG,&data,C_BMA2x2_One_U8X);
      *sleep_dur = BMA2x2_GET_BITSLICE(data,BMA2x2_SLEEP_DUR);
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/************************************************************************************
 * Description: *//**\brief This API is used to set Sleep Duration of the sensor
 *
 *
 *
 *
 *  \param unsigned char sleep_dur
 *                       5 -> 0.5MS
 *                       6 -> 1MS
 *                       7 -> 2MS
 *                       8 -> 4MS
 *                       9 -> 6MS
 *                      10 -> 10MS
 *                      11 -> 25MS
 *                      12 -> 50MS
 *                      13 -> 100MS
 *                      14 -> 500MS
 *                      15 -> 1S
 *
 *
 *  \return communication results
 *
 *
 **************************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_sleep_dur (unsigned char sleep_dur)
   {
   unsigned char data;
   int comres = C_BMA2x2_Zero_U8X ;
   int sleep_duration = 0;
   if (p_bma2x2 == C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      if (sleep_dur > C_BMA2x2_Four_U8X && sleep_dur < C_BMA2x2_Sixteen_U8X)
         {
         switch (sleep_dur)
            {
            case BMA2x2_SLEEP_DUR_0_5MS:
               sleep_duration = BMA2x2_SLEEP_DUR_0_5MS;

               /*  0.5 MS   */
               break;
            case BMA2x2_SLEEP_DUR_1MS:
               sleep_duration = BMA2x2_SLEEP_DUR_1MS;

               /*  1 MS  */
               break;
            case BMA2x2_SLEEP_DUR_2MS:
               sleep_duration = BMA2x2_SLEEP_DUR_2MS;

               /*  2 MS  */
               break;
            case BMA2x2_SLEEP_DUR_4MS:
               sleep_duration = BMA2x2_SLEEP_DUR_4MS;

               /*  4 MS   */
               break;
            case BMA2x2_SLEEP_DUR_6MS:
               sleep_duration = BMA2x2_SLEEP_DUR_6MS;

               /*  6 MS  */
               break;
            case BMA2x2_SLEEP_DUR_10MS:
               sleep_duration = BMA2x2_SLEEP_DUR_10MS;

               /*  10 MS  */
               break;
            case BMA2x2_SLEEP_DUR_25MS:
               sleep_duration = BMA2x2_SLEEP_DUR_25MS;

               /*  25 MS  */
               break;
            case BMA2x2_SLEEP_DUR_50MS:
               sleep_duration = BMA2x2_SLEEP_DUR_50MS;

               /*  50 MS   */
               break;
            case BMA2x2_SLEEP_DUR_100MS:
               sleep_duration = BMA2x2_SLEEP_DUR_100MS;

               /*  100 MS  */
               break;
            case BMA2x2_SLEEP_DUR_500MS:
               sleep_duration = BMA2x2_SLEEP_DUR_500MS;

               /*  500 MS   */
               break;
            case BMA2x2_SLEEP_DUR_1S:
               sleep_duration = BMA2x2_SLEEP_DUR_1S;

               /*  1 SECS   */
               break;
            default:
               break;
            }
         //SLEEP DURATION
         comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_SLEEP_DUR__REG, &data, C_BMA2x2_One_U8X);
         data = BMA2x2_SET_BITSLICE(data, BMA2x2_SLEEP_DUR, sleep_duration);
         comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_SLEEP_DUR__REG, &data, C_BMA2x2_One_U8X);
         }
      else
         {
         comres = E_OUT_OF_RANGE ;
         }
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to get the sleep timer mode status
 *
 *
 *
 *
 *  \param  unsigned char *sleep_tmr : Address of sleep_tmr
 *                  sleep_tmr -> [0:1]
 *                  0 => enable EventDrivenSampling(EDT)
 *                  1 => enable Eqidistant sampling mode(EST)
 *
 *
 *
 *  \return
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_sleeptmr_mode(unsigned char *sleep_tmr)
   {
   unsigned char data;
   int comres = C_BMA2x2_Zero_U8X;
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      //SLEEP TIMER MODE
      comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_SLEEP_TIMER__REG,&data,C_BMA2x2_One_U8X);
      *sleep_tmr = BMA2x2_GET_BITSLICE(data,BMA2x2_SLEEP_TIMER);
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to set the sleep timer mode status
 *
 *
 *
 *
 *  \param unsigned char sleep_tmr
 *                  sleep_tmr -> [0:1]
 *                  0 => enable EventDrivenSampling(EDT)
 *                  1 => enable Eqidistant sampling mode(EST)
 *
 *
 *
 *  \return communication results
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_sleeptmr_mode (unsigned char sleep_tmr)
   {
   unsigned char data;
   int comres = C_BMA2x2_Zero_U8X;
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      if (sleep_tmr < C_BMA2x2_Two_U8X)
         {
         //SLEEP TIMER MODE
         comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_SLEEP_TIMER__REG, &data, C_BMA2x2_One_U8X);
         data = BMA2x2_SET_BITSLICE(data, BMA2x2_SLEEP_TIMER, sleep_tmr);
         comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_SLEEP_TIMER__REG, &data, C_BMA2x2_One_U8X);
         }
      else
         {
         comres = E_OUT_OF_RANGE ;
         }
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to get high bandwidth
 *
 *
 *
 *
 *  \param unsigned char *high_bw : Address of high_bw
 *                   1 -> Unfiltered High Bandwidth
 *                   0 -> Filtered Low Bandwidth
 *
 *
 *  \return
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_high_bw (unsigned char *high_bw)
   {
      {
      char comres;
      unsigned char data;
      if (p_bma2x2==C_BMA2x2_Zero_U8X)
         {
         comres = E_SMB_NULL_PTR;
         }
      else
         {
         comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_EN_DATA_HIGH_BW__REG,&data,C_BMA2x2_One_U8X);
         *high_bw = BMA2x2_GET_BITSLICE(data,BMA2x2_EN_DATA_HIGH_BW);
         }
      return comres;
      }
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to set high bandwidth
 *
 *
 *
 *
 *  \param unsigned char high_bw
 *                    1 -> Unfiltered High Bandwidth
 *                    0 -> Filtered Low Bandwidth
 *
 *
 *
 *
 *  \return communication results
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_high_bw (unsigned char high_bw)
   {
      {
      int comres = C_BMA2x2_Zero_U8X;
      unsigned char data;
      if (p_bma2x2==C_BMA2x2_Zero_U8X)
         {
         comres = E_SMB_NULL_PTR;
         }
      else
         {
         comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_EN_DATA_HIGH_BW__REG, &data, C_BMA2x2_One_U8X);
         data = BMA2x2_SET_BITSLICE(data, BMA2x2_EN_DATA_HIGH_BW, high_bw);
         comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_EN_DATA_HIGH_BW__REG, &data, C_BMA2x2_One_U8X);
         }
      return comres;
      }
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to get shadow dis
 *
 *
 *
 *
 *  \param unsigned char *shadow_dis : Address of shadow_dis
 *                    1 -> No MSB Locking
 *                    0 -> MSB is Locked
 *
 *
 *
 *  \return
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_shadow_dis (unsigned char *shadow_dis)
   {
      {
      int comres = C_BMA2x2_Zero_U8X;
      unsigned char data;
      if (p_bma2x2==C_BMA2x2_Zero_U8X)
         {
         comres = E_SMB_NULL_PTR;
         }
      else
         {
         comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_DIS_SHADOW_PROC__REG,&data,C_BMA2x2_One_U8X);
         *shadow_dis = BMA2x2_GET_BITSLICE(data,BMA2x2_DIS_SHADOW_PROC);
         }
      return comres;
      }
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to set shadow dis
 *
 *
 *
 *
 *  \param unsigned char shadow_dis
 *                   1 -> No MSB Locking
 *                   0 -> MSB is Locked
 *
 *
 *
 *
 *  \return communication results
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_shadow_dis (unsigned char shadow_dis)
   {
      {
      int comres = C_BMA2x2_Zero_U8X;
      unsigned char data;
      if (p_bma2x2==C_BMA2x2_Zero_U8X)
         {
         comres = E_SMB_NULL_PTR;
         }
      else
         {
         comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_DIS_SHADOW_PROC__REG, &data, C_BMA2x2_One_U8X);
         data = BMA2x2_SET_BITSLICE(data, BMA2x2_DIS_SHADOW_PROC, shadow_dis);
         comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_DIS_SHADOW_PROC__REG, &data, C_BMA2x2_One_U8X);
         }
      return comres;
      }
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief
 *                      This function is used for the soft reset
 *     The soft reset register will be written with 0xB6.
 *
 *
 *
 *  \param None
 *
 *
 *
 *  \return Communication results.
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma2x2_soft_reset(void)
   {
   int comres = C_BMA2x2_Zero_U8X ;
   unsigned char data = BMA2x2_EN_SOFT_RESET_VALUE ;
   if (p_bma2x2 == C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_RESET_REG,&data ,C_BMA2x2_One_U8X);/* To reset the sensor 0xB6 value will be written */
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to update the register values
 *
 *
 *
 *
 *  \param
 *
 *
 *
 *  \return communication results
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma2x2_update_image(void)
   {
   int comres;
   unsigned char data=0;
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_UPDATE_IMAGE__REG, &data, C_BMA2x2_One_U8X);
      data = BMA2x2_SET_BITSLICE(data, BMA2x2_UPDATE_IMAGE, C_BMA2x2_One_U8X);
      comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_UPDATE_IMAGE__REG, &data,C_BMA2x2_One_U8X);
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/****************************************************************************************
 * Description: *//**\brief This API is used to set interrupt enable bits of the sensor
 *
 *
 *
 *
 *  \param unsigned char InterruptType , unsigned char value
 *                        0 -> Low_G_Interrupt
 *                        1 -> High_G_X_Interrupt
 *                        2 -> High_G_Y_Interrupt
 *                        3 -> High_G_Z_Interrupt
 *                        4 -> DATA_EN
 *                        5 -> Slope_X_Interrupt
 *                        6 -> Slope_Y_Interrupt
 *                        7 -> Slope_Z_Interrupt
 *                        8 -> Single_Tap_Interrupt
 *                        9 -> Double_Tap_Interrupt
 *                       10 -> Orient_Interrupt
 *                       11 -> Flat_Interrupt
 *
 *
 *
 *
 *  \return communication results
 *
 *
 ****************************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma2x2_set_Int_Enable(unsigned char InterruptType , unsigned char value )
   {
   int comres=C_BMA2x2_Zero_U8X;
   unsigned char data1,data2;
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_INT_ENABLE1_REG, &data1, C_BMA2x2_One_U8X );
      comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_INT_ENABLE2_REG, &data2, C_BMA2x2_One_U8X );

      value = value & C_BMA2x2_One_U8X;
      switch (InterruptType)
         {
         case BMA2x2_Low_G_Interrupt:
            /* Low G Interrupt  */
            data2 = BMA2x2_SET_BITSLICE(data2, BMA2x2_EN_LOWG_INT, value );
            break;
         case BMA2x2_High_G_X_Interrupt:
            /* High G X Interrupt */
            data2 = BMA2x2_SET_BITSLICE(data2, BMA2x2_EN_HIGHG_X_INT, value );
            break;
         case BMA2x2_High_G_Y_Interrupt:
            /* High G Y Interrupt */
            data2 = BMA2x2_SET_BITSLICE(data2, BMA2x2_EN_HIGHG_Y_INT, value );
            break;
         case BMA2x2_High_G_Z_Interrupt:
            /* High G Z Interrupt */
            data2 = BMA2x2_SET_BITSLICE(data2, BMA2x2_EN_HIGHG_Z_INT, value );
            break;
         case BMA2x2_DATA_EN:
            /*Data En Interrupt  */
            data2 = BMA2x2_SET_BITSLICE(data2, BMA2x2_EN_NEW_DATA_INT, value );
            break;
         case BMA2x2_Slope_X_Interrupt:
            /* Slope X Interrupt */
            data1 = BMA2x2_SET_BITSLICE(data1, BMA2x2_EN_SLOPE_X_INT, value );
            break;
         case BMA2x2_Slope_Y_Interrupt:
            /* Slope Y Interrupt */
            data1 = BMA2x2_SET_BITSLICE(data1, BMA2x2_EN_SLOPE_Y_INT, value );
            break;
         case BMA2x2_Slope_Z_Interrupt:
            /* Slope Z Interrupt */
            data1 = BMA2x2_SET_BITSLICE(data1, BMA2x2_EN_SLOPE_Z_INT, value );
            break;
         case BMA2x2_Single_Tap_Interrupt:
            /* Single Tap Interrupt */
            data1 = BMA2x2_SET_BITSLICE(data1, BMA2x2_EN_SINGLE_TAP_INT, value );
            break;
         case BMA2x2_Double_Tap_Interrupt:
            /* Double Tap Interrupt */
            data1 = BMA2x2_SET_BITSLICE(data1, BMA2x2_EN_DOUBLE_TAP_INT, value );
            break;
         case BMA2x2_Orient_Interrupt:
            /* Orient Interrupt  */
            data1 = BMA2x2_SET_BITSLICE(data1, BMA2x2_EN_ORIENT_INT, value );
            break;
         case BMA2x2_Flat_Interrupt:
            /* Flat Interrupt */
            data1 = BMA2x2_SET_BITSLICE(data1, BMA2x2_EN_FLAT_INT, value );
            break;
         default:
            comres = E_BMA2x2_OUT_OF_RANGE;
            break;
         }
      comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_INT_ENABLE1_REG, &data1, C_BMA2x2_One_U8X );
      comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_INT_ENABLE2_REG, &data2, C_BMA2x2_One_U8X );
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to get the interrupt ffull enable status
 *
 *
 *
 *
 *  \param unsigned char *ffull : Address of ffull
 *                    ffull -> [0:1]
 *                              0 --> Clear
 *                              1 --> Set
 *
 *
 *
 *  \return
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_int_ffull (unsigned char *ffull)
   {
   unsigned char data;
   int comres = C_BMA2x2_Zero_U8X;
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_INT_FFULL_EN_INT__REG,&data,C_BMA2x2_One_U8X);
      *ffull = BMA2x2_GET_BITSLICE(data,BMA2x2_INT_FFULL_EN_INT);
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to set the interrupt ffull enable status
 *
 *
 *
 *
 *  \param unsigned char ffull
 *            ffull -> [0:1]
 *                      0 --> Clear
 *                      1 --> Set
 *
 *
 *  \return communication results
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_int_ffull (unsigned char ffull)
   {
   unsigned char data;
   int comres = C_BMA2x2_Zero_U8X;
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      if (ffull < C_BMA2x2_Two_U8X)
         {
         comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_INT_FFULL_EN_INT__REG, &data, C_BMA2x2_One_U8X);
         data = BMA2x2_SET_BITSLICE(data,BMA2x2_INT_FFULL_EN_INT, ffull);
         comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_INT_FFULL_EN_INT__REG, &data, C_BMA2x2_One_U8X);
         }
      else
         {
         comres = E_OUT_OF_RANGE ;
         }
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/**************************************************************************************
 * Description: *//**\brief This API is used to get the interrupt fwm enable status
 *
 *
 *
 *
 *  \param unsigned char *fwm : Address of fwm
 *                   fwm -> [0:1]
 *                           0 --> Clear
 *                           1 --> Set
 *
 *
 *
 *  \return
 *
 *
 *************************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_int_fwm (unsigned char *fwm)
   {
   unsigned char data;
   int comres = C_BMA2x2_Zero_U8X;
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_INT_FWM_EN_INT__REG,&data,C_BMA2x2_One_U8X);
      *fwm = BMA2x2_GET_BITSLICE(data,BMA2x2_INT_FWM_EN_INT);
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*************************************************************************************
 * Description: *//**\brief This API is used to set the interrupt fwm enable status
 *
 *
 *
 *
 *  \param unsigned char fwm
 *        fwm -> [0:1]
 *                0 --> Clear
 *                1 --> Set
 *
 *
 *
 *  \return communication results
 *
 *
 *************************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_int_fwm (unsigned char fwm)
   {
   unsigned char data;
   int comres = C_BMA2x2_Zero_U8X;
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      if (fwm < C_BMA2x2_Two_U8X)
         {
         comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_INT_FWM_EN_INT__REG, &data, C_BMA2x2_One_U8X);
         data = BMA2x2_SET_BITSLICE(data,BMA2x2_INT_FWM_EN_INT, fwm);
         comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_INT_FWM_EN_INT__REG, &data, C_BMA2x2_One_U8X);
         }
      else
         {
         comres = E_OUT_OF_RANGE ;
         }
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*********************************************************************************************
 * Description: *//**\brief This API is used to get the status of slow/no motion interrupt
 *
 *
 *
 *
 *  \param unsigned char channel,unsigned char *slo_data
 *           channel -->
 *           BMA2x2_SLO_NO_MOT_EN_X     ->     0
 *           BMA2x2_SLO_NO_MOT_EN_Y     ->     1
 *           BMA2x2_SLO_NO_MOT_EN_Z     ->     2
 *           BMA2x2_SLO_NO_MOT_EN_SEL   ->     3
 *           slo_data --> 1
 *
 *
 *
 *  \return
 *
 *
 **********************************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_slo_no_mot (unsigned char channel,unsigned char *slo_data)
   {
      {
      unsigned char data;
      int comres=C_BMA2x2_Zero_U8X;
      if (p_bma2x2==C_BMA2x2_Zero_U8X)
         {
         comres = E_SMB_NULL_PTR;
         }
      else
         {
         switch (channel)
            {
            case BMA2x2_SLO_NO_MOT_EN_X:
               comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_INT_SLO_NO_MOT_EN_X_INT__REG, &data, C_BMA2x2_One_U8X);
               *slo_data = BMA2x2_GET_BITSLICE(data, BMA2x2_INT_SLO_NO_MOT_EN_X_INT);
               break;
            case BMA2x2_SLO_NO_MOT_EN_Y:
               comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_INT_SLO_NO_MOT_EN_Y_INT__REG, &data, C_BMA2x2_One_U8X);
               *slo_data = BMA2x2_GET_BITSLICE(data, BMA2x2_INT_SLO_NO_MOT_EN_Y_INT);
               break;
            case BMA2x2_SLO_NO_MOT_EN_Z:
               comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_INT_SLO_NO_MOT_EN_Z_INT__REG, &data, C_BMA2x2_One_U8X);
               *slo_data = BMA2x2_GET_BITSLICE(data, BMA2x2_INT_SLO_NO_MOT_EN_Z_INT);
               break;
            case BMA2x2_SLO_NO_MOT_EN_SEL:
               comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_INT_SLO_NO_MOT_EN_SEL_INT__REG, &data, C_BMA2x2_One_U8X);
               *slo_data = BMA2x2_GET_BITSLICE(data, BMA2x2_INT_SLO_NO_MOT_EN_SEL_INT);
               break;
            default:
               comres = E_BMA2x2_OUT_OF_RANGE;
               break;
            }
         }
      return comres;
      }
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/********************************************************************************************
 * Description: *//**\brief This API is used to set the status of slow/no motion interrupt
 *
 *
 *
 *
 *  \param unsigned char channel,unsigned char slo_data
 *           channel -->
 *           BMA2x2_SLO_NO_MOT_EN_X     ->     0
 *           BMA2x2_SLO_NO_MOT_EN_Y     ->     1
 *           BMA2x2_SLO_NO_MOT_EN_Z     ->     2
 *           BMA2x2_SLO_NO_MOT_EN_SEL   ->     3
 *           slo_data --> 1
 *
 *
 *
 *  \return communication results
 *
 *
 *******************************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_slo_no_mot (unsigned char channel,unsigned char slo_data)
   {
      {
      unsigned char data;
      int comres=C_BMA2x2_Zero_U8X;
      if (p_bma2x2==C_BMA2x2_Zero_U8X)
         {
         comres = E_SMB_NULL_PTR;
         }
      else
         {
         switch (channel)
            {
            case BMA2x2_SLO_NO_MOT_EN_X:
               comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_INT_SLO_NO_MOT_EN_X_INT__REG, &data, C_BMA2x2_One_U8X);
               data = BMA2x2_SET_BITSLICE(data, BMA2x2_INT_SLO_NO_MOT_EN_X_INT, slo_data);
               comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_INT_SLO_NO_MOT_EN_X_INT__REG, &data, C_BMA2x2_One_U8X);
               break;
            case BMA2x2_SLO_NO_MOT_EN_Y:
               comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_INT_SLO_NO_MOT_EN_Y_INT__REG, &data, C_BMA2x2_One_U8X);
               data = BMA2x2_SET_BITSLICE(data, BMA2x2_INT_SLO_NO_MOT_EN_Y_INT, slo_data);
               comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_INT_SLO_NO_MOT_EN_Y_INT__REG, &data, C_BMA2x2_One_U8X);
               break;
            case BMA2x2_SLO_NO_MOT_EN_Z:
               comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_INT_SLO_NO_MOT_EN_Z_INT__REG, &data, C_BMA2x2_One_U8X);
               data = BMA2x2_SET_BITSLICE(data, BMA2x2_INT_SLO_NO_MOT_EN_Z_INT, slo_data);
               comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_INT_SLO_NO_MOT_EN_Z_INT__REG, &data, C_BMA2x2_One_U8X);
               break;
            case BMA2x2_SLO_NO_MOT_EN_SEL:
               comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_INT_SLO_NO_MOT_EN_SEL_INT__REG, &data, C_BMA2x2_One_U8X);
               data = BMA2x2_SET_BITSLICE(data, BMA2x2_INT_SLO_NO_MOT_EN_SEL_INT, slo_data);
               comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_INT_SLO_NO_MOT_EN_SEL_INT__REG, &data, C_BMA2x2_One_U8X);
               break;
            default:
               comres = E_BMA2x2_OUT_OF_RANGE;
               break;
            }
         }
      return comres;
      }
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*********************************************************************************
 * Description: *//**\brief  This API is used to get the status of low interrupt
 *
 *
 *
 *
 *  \param unsigned char channel,unsigned char *int_low
 *                       channel -->
 *                       BMA2x2_INT1_LOWG     ->    0
 *                       BMA2x2_INT2_LOWG     ->    1
 *                       int_low --> 1
 *
 *
 *
 *  \return
 *
 *
 ********************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_int_low (unsigned char channel,unsigned char *int_low)
   {
      {
      unsigned char data;
      int comres=C_BMA2x2_Zero_U8X;
      if (p_bma2x2==C_BMA2x2_Zero_U8X)
         {
         comres = E_SMB_NULL_PTR;
         }
      else
         {
         switch (channel)
            {
            case BMA2x2_INT1_LOWG:
               comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_EN_INT1_PAD_LOWG__REG,&data,C_BMA2x2_One_U8X);
               *int_low = BMA2x2_GET_BITSLICE(data,BMA2x2_EN_INT1_PAD_LOWG);
               break;
            case BMA2x2_INT2_LOWG:
               comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_EN_INT2_PAD_LOWG__REG,&data,C_BMA2x2_One_U8X);
               *int_low = BMA2x2_GET_BITSLICE(data,BMA2x2_EN_INT2_PAD_LOWG);
               break;
            default:
               comres = E_BMA2x2_OUT_OF_RANGE;
               break;
            }
         }
      return comres;
      }
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*********************************************************************************
 * Description: *//**\brief This API is used to set the status of low interrupt
 *
 *
 *
 *
 *  \param unsigned char channel,unsigned char int_low
 *                       channel -->
 *                       BMA2x2_INT1_LOWG     ->    0
 *                       BMA2x2_INT2_LOWG     ->    1
 *                       int_low --> 1
 *
 *
 *
 *  \return communication results
 *
 *
 ********************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_int_low (unsigned char channel,unsigned char int_low)
   {
      {
      unsigned char data;
      int comres=C_BMA2x2_Zero_U8X;
      if (p_bma2x2==C_BMA2x2_Zero_U8X)
         {
         comres = E_SMB_NULL_PTR;
         }
      else
         {
         switch (channel)
            {
            case BMA2x2_INT1_LOWG:
               comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_EN_INT1_PAD_LOWG__REG, &data, C_BMA2x2_One_U8X);
               data = BMA2x2_SET_BITSLICE(data, BMA2x2_EN_INT1_PAD_LOWG, int_low);
               comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_EN_INT1_PAD_LOWG__REG, &data, C_BMA2x2_One_U8X);
               break;
            case BMA2x2_INT2_LOWG:
               comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_EN_INT2_PAD_LOWG__REG, &data, C_BMA2x2_One_U8X);
               data = BMA2x2_SET_BITSLICE(data, BMA2x2_EN_INT2_PAD_LOWG, int_low);
               comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_EN_INT2_PAD_LOWG__REG, &data, C_BMA2x2_One_U8X);
               break;
            default:
               comres = E_BMA2x2_OUT_OF_RANGE;
               break;
            }
         }
      return comres;
      }
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*********************************************************************************
 * Description: *//**\brief This API is used to get the status of high interrupt
 *
 *
 *
 *
 *  \param unsigned char channel,unsigned char *int_high
 *                           channel -->
 *                           BMA2x2_INT1_HIGHG     ->    0
 *                           BMA2x2_INT2_HIGHG     ->    1
 *                           int_high --> 1
 *
 *
 *
 *  \return
 *
 *
 ********************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_int_high(unsigned char channel,unsigned char *int_high)
   {
      {
      unsigned char data;
      int comres=C_BMA2x2_Zero_U8X;
      if (p_bma2x2==C_BMA2x2_Zero_U8X)
         {
         comres = E_SMB_NULL_PTR;
         }
      else
         {
         switch (channel)
            {
            case BMA2x2_INT1_HIGHG:
               comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_EN_INT1_PAD_HIGHG__REG,&data,C_BMA2x2_One_U8X);
               *int_high = BMA2x2_GET_BITSLICE(data,BMA2x2_EN_INT1_PAD_HIGHG);
               break;
            case BMA2x2_INT2_HIGHG:
               comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_EN_INT2_PAD_HIGHG__REG,&data,C_BMA2x2_One_U8X);
               *int_high = BMA2x2_GET_BITSLICE(data,BMA2x2_EN_INT2_PAD_HIGHG);
               break;
            default:
               comres = E_BMA2x2_OUT_OF_RANGE;
               break;
            }
         }
      return comres;
      }
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*********************************************************************************
 * Description: *//**\brief This API is used to set the status of high interrupt
 *
 *
 *
 *
 *  \param unsigned char channel,unsigned char int_high
 *                    channel -->
 *                    BMA2x2_INT1_HIGHG     ->    0
 *                    BMA2x2_INT2_HIGHG     ->    1
 *                    int_high --> 1
 *
 *
 *
 *  \return communication results
 *
 *
 ********************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_int_high(unsigned char channel,unsigned char int_high)
   {
      {
      unsigned char data;
      int comres=C_BMA2x2_Zero_U8X;
      if (p_bma2x2==C_BMA2x2_Zero_U8X)
         {
         comres = E_SMB_NULL_PTR;
         }
      else
         {
         switch (channel)
            {
            case BMA2x2_INT1_HIGHG:
               comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_EN_INT1_PAD_HIGHG__REG, &data, C_BMA2x2_One_U8X);
               data = BMA2x2_SET_BITSLICE(data, BMA2x2_EN_INT1_PAD_HIGHG, int_high);
               comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_EN_INT1_PAD_HIGHG__REG, &data, C_BMA2x2_One_U8X);
               break;
            case BMA2x2_INT2_HIGHG:
               comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_EN_INT2_PAD_HIGHG__REG, &data, C_BMA2x2_One_U8X);
               data = BMA2x2_SET_BITSLICE(data, BMA2x2_EN_INT2_PAD_HIGHG, int_high);
               comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_EN_INT2_PAD_HIGHG__REG, &data, C_BMA2x2_One_U8X);
               break;
            default:
               comres = E_BMA2x2_OUT_OF_RANGE;
               break;
            }
         }
      return comres;
      }
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/**********************************************************************************
 * Description: *//**\brief This API is used to get the status of slope interrupt
 *
 *
 *
 *
 *  \param unsigned char channel,unsigned char *int_slope
 *                               channel -->
 *                               BMA2x2_INT1_SLOPE     ->    0
 *                               BMA2x2_INT2_SLOPE     ->    1
 *                               int_slope --> 1
 *
 *
 *
 *  \return
 *
 *
 *********************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_int_slope(unsigned char channel,unsigned char *int_slope)
   {
      {
      unsigned char data;
      int comres=C_BMA2x2_Zero_U8X;
      if (p_bma2x2==C_BMA2x2_Zero_U8X)
         {
         comres = E_SMB_NULL_PTR;
         }
      else
         {
         switch (channel)
            {
            case BMA2x2_INT1_SLOPE:
               comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_EN_INT1_PAD_SLOPE__REG,&data,C_BMA2x2_One_U8X);
               *int_slope = BMA2x2_GET_BITSLICE(data,BMA2x2_EN_INT1_PAD_SLOPE);
               break;
            case BMA2x2_INT2_SLOPE:
               comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_EN_INT2_PAD_SLOPE__REG,&data,C_BMA2x2_One_U8X);
               *int_slope = BMA2x2_GET_BITSLICE(data,BMA2x2_EN_INT2_PAD_SLOPE);
               break;
            default:
               comres = E_BMA2x2_OUT_OF_RANGE;
               break;
            }
         }
      return comres;
      }
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/**********************************************************************************
 * Description: *//**\brief This API is used to set the status of slope interrupt
 *
 *
 *
 *
 *  \param unsigned char channel,unsigned char int_slope
 *                      channel -->
 *                      BMA2x2_INT1_SLOPE     ->    0
 *                      BMA2x2_INT2_SLOPE     ->    1
 *                      int_slope --> 1
 *
 *
 *  \return communication results
 *
 *
 *********************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_int_slope(unsigned char channel,unsigned char int_slope)
   {
      {
      unsigned char data;
      int comres=C_BMA2x2_Zero_U8X;
      if (p_bma2x2==C_BMA2x2_Zero_U8X)
         {
         comres = E_SMB_NULL_PTR;
         }
      else
         {
         switch (channel)
            {
            case BMA2x2_INT1_SLOPE:
               comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_EN_INT1_PAD_SLOPE__REG, &data, C_BMA2x2_One_U8X);
               data = BMA2x2_SET_BITSLICE(data, BMA2x2_EN_INT1_PAD_SLOPE, int_slope);
               comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_EN_INT1_PAD_SLOPE__REG, &data, C_BMA2x2_One_U8X);
               break;
            case BMA2x2_INT2_SLOPE:
               comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_EN_INT2_PAD_SLOPE__REG, &data, C_BMA2x2_One_U8X);
               data = BMA2x2_SET_BITSLICE(data, BMA2x2_EN_INT2_PAD_SLOPE, int_slope);
               comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_EN_INT2_PAD_SLOPE__REG, &data, C_BMA2x2_One_U8X);
               break;
            default:
               comres = E_BMA2x2_OUT_OF_RANGE;
               break;
            }
         }
      return comres;
      }
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*********************************************************************************************
 * Description: *//**\brief This API is used to get the status of slow/no motion interrupt
 *
 *
 *
 *
 *  \param unsigned char channel,unsigned char *int_slo_no_mot
 *                                 channel -->
 *                                 BMA2x2_INT1_SLO_NO_MOT     ->    0
 *                                 BMA2x2_INT2_SLO_NO_MOT     ->    1
 *                                 int_slo_no_mot --> 1
 *
 *
 *
 *  \return
 *
 *
 *********************************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_int_slo_no_mot (unsigned char channel,unsigned char *int_slo_no_mot)
   {
      {
      unsigned char data;
      int comres=C_BMA2x2_Zero_U8X;
      if (p_bma2x2==C_BMA2x2_Zero_U8X)
         {
         comres = E_SMB_NULL_PTR;
         }
      else
         {
         switch (channel)
            {
            case BMA2x2_INT1_SLO_NO_MOT:
               comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_EN_INT1_PAD_SLO_NO_MOT__REG,&data,C_BMA2x2_One_U8X);
               *int_slo_no_mot = BMA2x2_GET_BITSLICE(data,BMA2x2_EN_INT1_PAD_SLO_NO_MOT);
               break;
            case BMA2x2_INT2_SLO_NO_MOT:
               comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_EN_INT2_PAD_SLO_NO_MOT__REG,&data,C_BMA2x2_One_U8X);
               *int_slo_no_mot = BMA2x2_GET_BITSLICE(data,BMA2x2_EN_INT2_PAD_SLO_NO_MOT);
               break;
            default:
               comres = E_BMA2x2_OUT_OF_RANGE;
               break;
            }
         }
      return comres;
      }
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*********************************************************************************************
 * Description: *//**\brief This API is used to set the status of slow/no motion interrupt
 *
 *
 *
 *
 *  \param unsigned char channel,unsigned char int_slo_no_mot
 *                            channel -->
 *                            BMA2x2_INT1_SLO_NO_MOT     ->    0
 *                            BMA2x2_INT2_SLO_NO_MOT     ->    1
 *                            int_slo_no_mot --> 1
 *
 *
 *
 *  \return communication results
 *
 *
 **********************************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_int_slo_no_mot (unsigned char channel,unsigned char int_slo_no_mot)
   {
      {
      unsigned char data;
      int comres=C_BMA2x2_Zero_U8X;
      if (p_bma2x2==C_BMA2x2_Zero_U8X)
         {
         comres = E_SMB_NULL_PTR;
         }
      else
         {
         switch (channel)
            {
            case BMA2x2_INT1_SLO_NO_MOT:
               comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_EN_INT1_PAD_SLO_NO_MOT__REG, &data, C_BMA2x2_One_U8X);
               data = BMA2x2_SET_BITSLICE(data, BMA2x2_EN_INT1_PAD_SLO_NO_MOT, int_slo_no_mot);
               comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_EN_INT1_PAD_SLO_NO_MOT__REG, &data, C_BMA2x2_One_U8X);
               break;
            case BMA2x2_INT2_SLO_NO_MOT:
               comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_EN_INT2_PAD_SLO_NO_MOT__REG, &data, C_BMA2x2_One_U8X);
               data = BMA2x2_SET_BITSLICE(data, BMA2x2_EN_INT2_PAD_SLO_NO_MOT, int_slo_no_mot);
               comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_EN_INT2_PAD_SLO_NO_MOT__REG, &data, C_BMA2x2_One_U8X);
               break;
            default:
               comres = E_BMA2x2_OUT_OF_RANGE;
               break;
            }
         }
      return comres;
      }
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*****************************************************************************************
 * Description: *//**\brief This API is used to get the status of double tap interrupt
 *
 *
 *
 *
 *  \param unsigned char channel,unsigned char *int_d_tap
 *                       channel -->
 *                       BMA2x2_INT1_DTAP     ->    0
 *                       BMA2x2_INT2_DTAP     ->    1
 *                       int_d_tap --> 1
 *
 *
 *
 *  \return
 *
 *
 ****************************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_int_d_tap (unsigned char channel,unsigned char *int_d_tap)
   {
      {
      unsigned char data;
      int comres=C_BMA2x2_Zero_U8X;
      if (p_bma2x2==C_BMA2x2_Zero_U8X)
         {
         comres = E_SMB_NULL_PTR;
         }
      else
         {
         switch (channel)
            {
            case BMA2x2_INT1_DTAP:
               comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_EN_INT1_PAD_DB_TAP__REG,&data,C_BMA2x2_One_U8X);
               *int_d_tap = BMA2x2_GET_BITSLICE(data,BMA2x2_EN_INT1_PAD_DB_TAP);
               break;
            case BMA2x2_INT2_DTAP:
               comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_EN_INT2_PAD_DB_TAP__REG,&data,C_BMA2x2_One_U8X);
               *int_d_tap = BMA2x2_GET_BITSLICE(data,BMA2x2_EN_INT2_PAD_DB_TAP);
               break;
            default:
               comres = E_BMA2x2_OUT_OF_RANGE;
               break;
            }
         }
      return comres;
      }
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/****************************************************************************************
 * Description: *//**\brief This API is used to set the status of double tap interrupt
 *
 *
 *
 *
 *  \param unsigned char channel,unsigned char int_d_tap
 *                       channel -->
 *                       BMA2x2_INT1_DTAP     ->    0
 *                       BMA2x2_INT2_DTAP     ->    1
 *                       int_d_tap --> 1
 *
 *
 *
 *  \return communication results
 *
 *
 *****************************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_int_d_tap (unsigned char channel,unsigned char int_d_tap)
   {
      {
      unsigned char data;
      int comres=C_BMA2x2_Zero_U8X;
      if (p_bma2x2==C_BMA2x2_Zero_U8X)
         {
         comres = E_SMB_NULL_PTR;
         }
      else
         {
         switch (channel)
            {
            case BMA2x2_INT1_DTAP:
               comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_EN_INT1_PAD_DB_TAP__REG, &data, C_BMA2x2_One_U8X);
               data = BMA2x2_SET_BITSLICE(data, BMA2x2_EN_INT1_PAD_DB_TAP, int_d_tap);
               comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_EN_INT1_PAD_DB_TAP__REG, &data, C_BMA2x2_One_U8X);
               break;
            case BMA2x2_INT2_DTAP:
               comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_EN_INT2_PAD_DB_TAP__REG, &data, C_BMA2x2_One_U8X);
               data = BMA2x2_SET_BITSLICE(data, BMA2x2_EN_INT2_PAD_DB_TAP, int_d_tap);
               comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_EN_INT2_PAD_DB_TAP__REG, &data, C_BMA2x2_One_U8X);
               break;
            default:
               comres = E_BMA2x2_OUT_OF_RANGE;
               break;
            }
         }
      return comres;
      }
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/***************************************************************************************
 * Description: *//**\brief This API is used to get the status of single tap interrupt
 *
 *
 *
 *
 *  \param unsigned char channel,unsigned char *int_s_tap
 *                       channel -->
 *                       BMA2x2_INT1_STAP     ->    0
 *                       BMA2x2_INT2_STAP     ->    1
 *                       int_s_tap --> 1
 *
 *
 *
 *  \return
 *
 *
 ***************************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_int_s_tap (unsigned char channel,unsigned char *int_s_tap)
   {
      {
      unsigned char data;
      int comres=C_BMA2x2_Zero_U8X;
      if (p_bma2x2==C_BMA2x2_Zero_U8X)
         {
         comres = E_SMB_NULL_PTR;
         }
      else
         {
         switch (channel)
            {
            case BMA2x2_INT1_STAP:
               comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_EN_INT1_PAD_SNG_TAP__REG,&data,C_BMA2x2_One_U8X);
               *int_s_tap = BMA2x2_GET_BITSLICE(data,BMA2x2_EN_INT1_PAD_SNG_TAP);
               break;
            case BMA2x2_INT2_STAP:
               comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_EN_INT2_PAD_SNG_TAP__REG,&data,C_BMA2x2_One_U8X);
               *int_s_tap = BMA2x2_GET_BITSLICE(data,BMA2x2_EN_INT2_PAD_SNG_TAP);
               break;
            default:
               comres = E_BMA2x2_OUT_OF_RANGE;
               break;
            }
         }
      return comres;
      }
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/****************************************************************************************
 * Description: *//**\brief This API is used to set the status of single tap interrupt
 *
 *
 *
 *
 *  \param unsigned char channel,unsigned char int_s_tap
 *                       channel -->
 *                       BMA2x2_INT1_STAP     ->    0
 *                       BMA2x2_INT2_STAP     ->    1
 *                       int_s_tap --> 1
 *
 *
 *
 *  \return communication results
 *
 *
 ****************************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_int_s_tap (unsigned char channel,unsigned char int_s_tap)
   {
      {
      unsigned char data;
      int comres=C_BMA2x2_Zero_U8X;
      if (p_bma2x2==C_BMA2x2_Zero_U8X)
         {
         comres = E_SMB_NULL_PTR;
         }
      else
         {
         switch (channel)
            {
            case BMA2x2_INT1_STAP:
               comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_EN_INT1_PAD_SNG_TAP__REG, &data, C_BMA2x2_One_U8X);
               data = BMA2x2_SET_BITSLICE(data, BMA2x2_EN_INT1_PAD_SNG_TAP, int_s_tap);
               comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_EN_INT1_PAD_SNG_TAP__REG, &data, C_BMA2x2_One_U8X);
               break;
            case BMA2x2_INT2_STAP:
               comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_EN_INT2_PAD_SNG_TAP__REG, &data, C_BMA2x2_One_U8X);
               data = BMA2x2_SET_BITSLICE(data, BMA2x2_EN_INT2_PAD_SNG_TAP, int_s_tap);
               comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_EN_INT2_PAD_SNG_TAP__REG, &data, C_BMA2x2_One_U8X);
               break;
            default:
               comres = E_BMA2x2_OUT_OF_RANGE;
               break;
            }
         }
      return comres;
      }
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/**************************************************************************************
 * Description: *//**\brief This API is used to get the status of orient interrupt
 *
 *
 *
 *
 *  \param unsigned char channel,unsigned char *int_orient
 *                           channel -->
 *                           BMA2x2_INT1_ORIENT     ->    0
 *                           BMA2x2_INT2_ORIENT     ->    1
 *                           int_orient --> 1
 *
 *
 *
 *  \return
 *
 *
 **************************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_int_orient (unsigned char channel,unsigned char *int_orient)
   {
      {
      unsigned char data;
      int comres=C_BMA2x2_Zero_U8X;
      if (p_bma2x2==C_BMA2x2_Zero_U8X)
         {
         comres = E_SMB_NULL_PTR;
         }
      else
         {
         switch (channel)
            {
            case BMA2x2_INT1_ORIENT:
               comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_EN_INT1_PAD_ORIENT__REG,&data,C_BMA2x2_One_U8X);
               *int_orient = BMA2x2_GET_BITSLICE(data,BMA2x2_EN_INT1_PAD_ORIENT);
               break;
            case BMA2x2_INT2_ORIENT:
               comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_EN_INT2_PAD_ORIENT__REG,&data,C_BMA2x2_One_U8X);
               *int_orient = BMA2x2_GET_BITSLICE(data,BMA2x2_EN_INT2_PAD_ORIENT);
               break;
            default:
               comres = E_BMA2x2_OUT_OF_RANGE;
               break;
            }
         }
      return comres;
      }
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*************************************************************************************
 * Description: *//**\brief This API is used to set the status of orient interrupt
 *
 *
 *
 *
 *  \param unsigned char channel,unsigned char int_orient
 *                        channel -->
 *                        BMA2x2_INT1_ORIENT     ->    0
 *                        BMA2x2_INT2_ORIENT     ->    1
 *                        int_orient --> 1
 *
 *
 *
 *  \return communication results
 *
 *
 ************************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_int_orient (unsigned char channel,unsigned char int_orient)
   {
      {
      unsigned char data;
          int comres=C_BMA2x2_Zero_U8X;
      if (p_bma2x2==C_BMA2x2_Zero_U8X)
         {
         comres = E_SMB_NULL_PTR;
         }
      else
         {
         switch (channel)
            {
            case BMA2x2_INT1_ORIENT:
               comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_EN_INT1_PAD_ORIENT__REG, &data, C_BMA2x2_One_U8X);
               data = BMA2x2_SET_BITSLICE(data, BMA2x2_EN_INT1_PAD_ORIENT, int_orient);
               comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_EN_INT1_PAD_ORIENT__REG, &data, C_BMA2x2_One_U8X);
               break;
            case BMA2x2_INT2_ORIENT:
               comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_EN_INT2_PAD_ORIENT__REG, &data, C_BMA2x2_One_U8X);
               data = BMA2x2_SET_BITSLICE(data, BMA2x2_EN_INT2_PAD_ORIENT, int_orient);
               comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_EN_INT2_PAD_ORIENT__REG, &data, C_BMA2x2_One_U8X);
               break;
            default:
               comres = E_BMA2x2_OUT_OF_RANGE;
               break;
            }
         }
      return comres;
      }
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/**********************************************************************************
 * Description: *//**\brief This API is used to get the status of flat interrupt
 *
 *
 *
 *
 *  \param unsigned char channel,unsigned char *int_flat
 *                         channel -->
 *                         BMA2x2_INT1_FLAT     ->    0
 *                         BMA2x2_INT2_FLAT     ->    1
 *                         int_flat --> 1
 *
 *
 *
 *  \return
 *
 *
 *********************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_int_flat (unsigned char channel,unsigned char *int_flat)
   {
      {
      unsigned char data;
      int comres=C_BMA2x2_Zero_U8X;
      if (p_bma2x2==C_BMA2x2_Zero_U8X)
         {
         comres = E_SMB_NULL_PTR;
         }
      else
         {
         switch (channel)
            {
            case BMA2x2_INT1_FLAT:
               comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_EN_INT1_PAD_FLAT__REG,&data,C_BMA2x2_One_U8X);
               *int_flat = BMA2x2_GET_BITSLICE(data,BMA2x2_EN_INT1_PAD_FLAT);
               break;
            case BMA2x2_INT2_FLAT:
               comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_EN_INT2_PAD_FLAT__REG,&data,C_BMA2x2_One_U8X);
               *int_flat = BMA2x2_GET_BITSLICE(data,BMA2x2_EN_INT2_PAD_FLAT);
               break;
            default:
               comres = E_BMA2x2_OUT_OF_RANGE;
               break;
            }
         }
      return comres;
      }
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to set the status of flat interrupt
 *
 *
 *
 *
 *  \param unsigned char channel,unsigned char int_flat
 *                        channel -->
 *                        BMA2x2_INT1_FLAT     ->    0
 *                        BMA2x2_INT2_FLAT     ->    1
 *                        int_flat --> 1
 *
 *
 *
 *  \return communication results
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_int_flat (unsigned char channel,unsigned char int_flat)
   {
      {
      unsigned char data;
      int comres=C_BMA2x2_Zero_U8X;
      if (p_bma2x2==C_BMA2x2_Zero_U8X)
         {
         comres = E_SMB_NULL_PTR;
         }
      else
         {
         switch (channel)
            {
            case BMA2x2_INT1_FLAT:
               comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_EN_INT1_PAD_FLAT__REG, &data, C_BMA2x2_One_U8X);
               data = BMA2x2_SET_BITSLICE(data, BMA2x2_EN_INT1_PAD_FLAT, int_flat);
               comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_EN_INT1_PAD_FLAT__REG, &data, C_BMA2x2_One_U8X);
               break;
            case BMA2x2_INT2_FLAT:
               comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_EN_INT2_PAD_FLAT__REG, &data, C_BMA2x2_One_U8X);
               data = BMA2x2_SET_BITSLICE(data, BMA2x2_EN_INT2_PAD_FLAT, int_flat);
               comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_EN_INT2_PAD_FLAT__REG, &data, C_BMA2x2_One_U8X);
               break;
            default:
               comres = E_BMA2x2_OUT_OF_RANGE;
               break;
            }
         }
      return comres;
      }
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to get the status of new data
 *
 *
 *
 *
 *  \param unsigned char channel,unsigned char *int_newdata
 *                         channel -->
 *                         BMA2x2_INT1_NDATA     ->    0
 *                         BMA2x2_INT2_NDATA     ->    1
 *                         int_newdata --> 1
 *
 *
 *
 *  \return
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_newdata (unsigned char channel,unsigned char *int_newdata)
   {
      {
      unsigned char data;
      int comres=C_BMA2x2_Zero_U8X;
      if (p_bma2x2==C_BMA2x2_Zero_U8X)
         {
         comres = E_SMB_NULL_PTR;
         }
      else
         {
         switch (channel)
            {
            case BMA2x2_INT1_NDATA:
               comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_EN_INT1_PAD_NEWDATA__REG,&data,C_BMA2x2_One_U8X);
               *int_newdata = BMA2x2_GET_BITSLICE(data,BMA2x2_EN_INT1_PAD_NEWDATA);
               break;
            case BMA2x2_INT2_NDATA:
               comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_EN_INT2_PAD_NEWDATA__REG,&data,C_BMA2x2_One_U8X);
               *int_newdata = BMA2x2_GET_BITSLICE(data,BMA2x2_EN_INT2_PAD_NEWDATA);
               break;
            default:
               comres = E_BMA2x2_OUT_OF_RANGE;
               break;
            }
         }
      return comres;
      }
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to set the status of new data
 *
 *
 *
 *
 *  \param unsigned char channel,unsigned char int_newdata
 *                        channel -->
 *                        BMA2x2_INT1_NDATA     ->    0
 *                        BMA2x2_INT2_NDATA     ->    1
 *                        int_newdata --> 1
 *
 *
 *
 *  \return communication results
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_newdata (unsigned char channel,unsigned char int_newdata)
   {
      {
      unsigned char data;
      int comres=C_BMA2x2_Zero_U8X;
      if (p_bma2x2==C_BMA2x2_Zero_U8X)
         {
         comres = E_SMB_NULL_PTR;
         }
      else
         {
         switch (channel)
            {
            case BMA2x2_INT1_NDATA:
               comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_EN_INT1_PAD_NEWDATA__REG, &data, C_BMA2x2_One_U8X);
               data = BMA2x2_SET_BITSLICE(data, BMA2x2_EN_INT1_PAD_NEWDATA, int_newdata);
               comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_EN_INT1_PAD_NEWDATA__REG, &data, C_BMA2x2_One_U8X);
               break;
            case BMA2x2_INT2_NDATA:
               comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_EN_INT2_PAD_NEWDATA__REG, &data, C_BMA2x2_One_U8X);
               data = BMA2x2_SET_BITSLICE(data, BMA2x2_EN_INT2_PAD_NEWDATA, int_newdata);
               comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_EN_INT2_PAD_NEWDATA__REG, &data, C_BMA2x2_One_U8X);
               break;
            default:
               comres = E_BMA2x2_OUT_OF_RANGE;
               break;
            }
         }
      return comres;
      }
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to get the fwm interrupt1 data
 *
 *
 *
 *
 *  \param  unsigned char *int1_fwm : Address of int1_fwm
 *                  int1_fwm --> [0:1]
 *                                0 --> Clear
 *                                1 --> Set
 *
 *
 *
 *  \return
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_int1_fwm (unsigned char *int1_fwm)
   {
      {
      unsigned char data;
      int comres=C_BMA2x2_Zero_U8X;
      if (p_bma2x2==C_BMA2x2_Zero_U8X)
         {
         comres = E_SMB_NULL_PTR;
         }
      else
         {
         comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_EN_INT1_PAD_FWM__REG,&data,C_BMA2x2_One_U8X);
         *int1_fwm = BMA2x2_GET_BITSLICE(data,BMA2x2_EN_INT1_PAD_FWM);
         }
      return comres;
      }
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to set the fwm interrupt1 data
 *
 *
 *
 *
 *  \param unsigned char int1_fwm
 *         int1_fwm --> [0:1]
 *          0 --> Clear
 *          1 --> Set
 *
 *
 *
 *  \return communication results
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_int1_fwm (unsigned char int1_fwm)
   {
   unsigned char data;
   int comres = C_BMA2x2_Zero_U8X;
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      if (int1_fwm < C_BMA2x2_Two_U8X)
         {
         comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_EN_INT1_PAD_FWM__REG, &data, C_BMA2x2_One_U8X);
         data = BMA2x2_SET_BITSLICE(data, BMA2x2_EN_INT1_PAD_FWM, int1_fwm);
         comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_EN_INT1_PAD_FWM__REG, &data, C_BMA2x2_One_U8X);
         }
      else
         {
         comres = E_OUT_OF_RANGE ;
         }
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to get the fwm interrupt2 data
 *
 *
 *
 *
 *  \param  unsigned char*int2_fwm : Address of int2_fwm
 *                 int2_fwm[0:1]
 *                          0 --> Clear
 *                          1 --> Set
 *
 *
 *
 *  \return
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_int2_fwm (unsigned char *int2_fwm)
   {
      {
      unsigned char data;
      int comres=C_BMA2x2_Zero_U8X;
      if (p_bma2x2==C_BMA2x2_Zero_U8X)
         {
         comres = E_SMB_NULL_PTR;
         }
      else
         {
         comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_EN_INT2_PAD_FWM__REG,&data,C_BMA2x2_One_U8X);
         *int2_fwm = BMA2x2_GET_BITSLICE(data,BMA2x2_EN_INT2_PAD_FWM);
         }
      return comres;
      }
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to set the fwm interrupt2 data
 *
 *
 *
 *
 *  \param unsigned char int2_fwm
 *              int2_fwm --> [0:1]
 *              0 --> Clear
 *              1 --> Set
 *
 *
 *
 *  \return communication results
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_int2_fwm (unsigned char int2_fwm)
   {
   unsigned char data;
   int comres = C_BMA2x2_Zero_U8X;
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      if (int2_fwm < C_BMA2x2_Two_U8X)
         {
         comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_EN_INT2_PAD_FWM__REG, &data, C_BMA2x2_One_U8X);
         data = BMA2x2_SET_BITSLICE(data, BMA2x2_EN_INT2_PAD_FWM, int2_fwm);
         comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_EN_INT2_PAD_FWM__REG, &data, C_BMA2x2_One_U8X);
         }
      else
         {
         comres = E_OUT_OF_RANGE ;
         }
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to get the ffull interrupt1 data
 *
 *
 *
 *
 *  \param unsigned char *int1_ffull : Address of int1_ffull
 *                    int1_ffull --> [0:1]
 *                                    0 --> Clear
 *                                    1 --> Set
 *
 *
 *
 *  \return
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_int1_ffull (unsigned char *int1_ffull)
   {
      {
      unsigned char data;
      int comres=C_BMA2x2_Zero_U8X;
      if (p_bma2x2==C_BMA2x2_Zero_U8X)
         {
         comres = E_SMB_NULL_PTR;
         }
      else
         {
         comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_EN_INT1_PAD_FFULL__REG,&data,C_BMA2x2_One_U8X);
         *int1_ffull = BMA2x2_GET_BITSLICE(data,BMA2x2_EN_INT1_PAD_FFULL);
         }
      return comres;
      }
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to set the ffull interrupt1 data
 *
 *
 *
 *
 *  \param unsigned char int1_ffull
 *             int1_ffull --> [0:1]
 *             0 --> Clear
 *             1 --> Set
 *
 *
 *
 *  \return communication results
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_int1_ffull (unsigned char int1_ffull)
   {
   unsigned char data;
   int comres = C_BMA2x2_Zero_U8X;
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      if (int1_ffull < C_BMA2x2_Two_U8X)
         {
         comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_EN_INT1_PAD_FFULL__REG, &data, C_BMA2x2_One_U8X);
         data = BMA2x2_SET_BITSLICE(data, BMA2x2_EN_INT1_PAD_FFULL, int1_ffull);
         comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_EN_INT1_PAD_FFULL__REG, &data, C_BMA2x2_One_U8X);
         }
      else
         {
         comres = E_OUT_OF_RANGE ;
         }
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to get the ffull interrupt2 data
 *
 *
 *
 *
 *  \param unsigned char *int2_ffull : Address of int2_ffull
 *             int2_ffull --> [0:1]
 *                             0 --> Clear
 *                             1 --> Set
 *
 *
 *
 *  \return
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_int2_ffull (unsigned char *int2_ffull)
   {
      {
      unsigned char data;
      int comres=C_BMA2x2_Zero_U8X;
      if (p_bma2x2==C_BMA2x2_Zero_U8X)
         {
         comres = E_SMB_NULL_PTR;
         }
      else
         {
         comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_EN_INT2_PAD_FFULL__REG,&data,C_BMA2x2_One_U8X);
         *int2_ffull = BMA2x2_GET_BITSLICE(data,BMA2x2_EN_INT2_PAD_FFULL);
         }
      return comres;
      }
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to set the ffull interrupt2 data
 *
 *
 *
 *
 *  \param unsigned char int2_ffull
 *         int2_ffull --> [0:1]
 *         0 --> Clear
 *         1 --> Set
 *
 *
 *
 *  \return communication results
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_int2_ffull (unsigned char int2_ffull)
   {
   unsigned char data;
   int comres = C_BMA2x2_Zero_U8X;
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      if (int2_ffull < C_BMA2x2_Two_U8X)
         {
         comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_EN_INT2_PAD_FFULL__REG, &data, C_BMA2x2_One_U8X);
         data = BMA2x2_SET_BITSLICE(data, BMA2x2_EN_INT2_PAD_FFULL, int2_ffull);
         comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_EN_INT2_PAD_FFULL__REG, &data, C_BMA2x2_One_U8X);
         }
      else
         {
         comres = E_OUT_OF_RANGE ;
         }
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to get the source status data
 *
 *
 *
 *
 *  \param unsigned char channel,unsigned char *int_source
 *                     Channel ->
 *                  BMA2x2_SRC_LOWG         0
 *                  BMA2x2_SRC_HIGHG        1
 *                  BMA2x2_SRC_SLOPE        2
 *                  BMA2x2_SRC_SLO_NO_MOT   3
 *                  BMA2x2_SRC_TAP          4
 *                  BMA2x2_SRC_DATA         5
 *                  int_source -> 1
 *
 *
 *
 *  \return
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_source (unsigned char channel,unsigned char *int_source)
   {
      {
      unsigned char data;
      int comres=C_BMA2x2_Zero_U8X;
      if (p_bma2x2==C_BMA2x2_Zero_U8X)
         {
         comres = E_SMB_NULL_PTR;
         }
      else
         {
         switch (channel)
            {
            case BMA2x2_SRC_LOWG:
               comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_UNFILT_INT_SRC_LOWG__REG,&data,C_BMA2x2_One_U8X);
               *int_source = BMA2x2_GET_BITSLICE(data,BMA2x2_UNFILT_INT_SRC_LOWG);
               break;
            case BMA2x2_SRC_HIGHG:
               comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_UNFILT_INT_SRC_HIGHG__REG,&data,C_BMA2x2_One_U8X);
               *int_source = BMA2x2_GET_BITSLICE(data,BMA2x2_UNFILT_INT_SRC_HIGHG);
               break;
            case BMA2x2_SRC_SLOPE:
               comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_UNFILT_INT_SRC_SLOPE__REG,&data,C_BMA2x2_One_U8X);
               *int_source = BMA2x2_GET_BITSLICE(data,BMA2x2_UNFILT_INT_SRC_SLOPE);
               break;
            case BMA2x2_SRC_SLO_NO_MOT:
               comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_UNFILT_INT_SRC_SLO_NO_MOT__REG,&data,C_BMA2x2_One_U8X);
               *int_source = BMA2x2_GET_BITSLICE(data,BMA2x2_UNFILT_INT_SRC_SLO_NO_MOT);
               break;
            case BMA2x2_SRC_TAP:
               comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_UNFILT_INT_SRC_TAP__REG,&data,C_BMA2x2_One_U8X);
               *int_source = BMA2x2_GET_BITSLICE(data,BMA2x2_UNFILT_INT_SRC_TAP);
               break;
            case BMA2x2_SRC_DATA:
               comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_UNFILT_INT_SRC_DATA__REG,&data,C_BMA2x2_One_U8X);
               *int_source = BMA2x2_GET_BITSLICE(data,BMA2x2_UNFILT_INT_SRC_DATA);
               break;
            default:
               comres = E_BMA2x2_OUT_OF_RANGE;
               break;
            }
         }
      return comres;
      }
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief  This API is used to set source status data
 *
 *
 *
 *
 *  \param unsigned char channel,unsigned char int_source
 *                  Channel ->
 *                  BMA2x2_SRC_LOWG         0
 *                  BMA2x2_SRC_HIGHG        1
 *                  BMA2x2_SRC_SLOPE        2
 *                  BMA2x2_SRC_SLO_NO_MOT   3
 *                  BMA2x2_SRC_TAP          4
 *                  BMA2x2_SRC_DATA         5
 *                  int_source -> 1
 *
 *
 *  \return communication results
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_source (unsigned char channel,unsigned char int_source)
   {
      {
      unsigned char data;
      int comres=C_BMA2x2_Zero_U8X;
      if (p_bma2x2==C_BMA2x2_Zero_U8X)
         {
         comres = E_SMB_NULL_PTR;
         }
      else
         {
         switch (channel)
            {
            case BMA2x2_SRC_LOWG:
               comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_UNFILT_INT_SRC_LOWG__REG, &data, C_BMA2x2_One_U8X);
               data = BMA2x2_SET_BITSLICE(data, BMA2x2_UNFILT_INT_SRC_LOWG, int_source);
               comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_UNFILT_INT_SRC_LOWG__REG, &data, C_BMA2x2_One_U8X);
               break;
            case BMA2x2_SRC_HIGHG:
               comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_UNFILT_INT_SRC_HIGHG__REG, &data, C_BMA2x2_One_U8X);
               data = BMA2x2_SET_BITSLICE(data, BMA2x2_UNFILT_INT_SRC_HIGHG, int_source);
               comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_UNFILT_INT_SRC_HIGHG__REG, &data, C_BMA2x2_One_U8X);
               break;
            case BMA2x2_SRC_SLOPE:
               comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_UNFILT_INT_SRC_SLOPE__REG, &data, C_BMA2x2_One_U8X);
               data = BMA2x2_SET_BITSLICE(data, BMA2x2_UNFILT_INT_SRC_SLOPE, int_source);
               comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_UNFILT_INT_SRC_SLOPE__REG, &data, C_BMA2x2_One_U8X);
               break;
            case BMA2x2_SRC_SLO_NO_MOT:
               comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_UNFILT_INT_SRC_SLO_NO_MOT__REG, &data, C_BMA2x2_One_U8X);
               data = BMA2x2_SET_BITSLICE(data, BMA2x2_UNFILT_INT_SRC_SLO_NO_MOT, int_source);
               comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_UNFILT_INT_SRC_SLO_NO_MOT__REG, &data, C_BMA2x2_One_U8X);
               break;
            case BMA2x2_SRC_TAP:
               comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_UNFILT_INT_SRC_TAP__REG, &data, C_BMA2x2_One_U8X);
               data = BMA2x2_SET_BITSLICE(data, BMA2x2_UNFILT_INT_SRC_TAP, int_source);
               comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_UNFILT_INT_SRC_TAP__REG, &data, C_BMA2x2_One_U8X);
               break;
            case BMA2x2_SRC_DATA:
               comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_UNFILT_INT_SRC_DATA__REG, &data, C_BMA2x2_One_U8X);
               data = BMA2x2_SET_BITSLICE(data, BMA2x2_UNFILT_INT_SRC_DATA, int_source);
               comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_UNFILT_INT_SRC_DATA__REG, &data, C_BMA2x2_One_U8X);
               break;
            default:
               comres = E_BMA2x2_OUT_OF_RANGE;
               break;
            }
         }
      return comres;
      }
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to get the output type status
 *
 *
 *
 *
 *  \param unsigned char channel,unsigned char *int_od
 *                  BMA2x2_INT1_OUTPUT    ->   0
 *                  BMA2x2_INT2_OUTPUT    ->   1
 *                  int_od : open drain   ->   1
 *                           push pull    ->   0
 *
 *
 *
 *  \return
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_int_od (unsigned char channel,unsigned char *int_od)
   {
      {
      unsigned char data;
      int comres=C_BMA2x2_Zero_U8X;
      if (p_bma2x2==C_BMA2x2_Zero_U8X)
         {
         comres = E_SMB_NULL_PTR;
         }
      else
         {
         switch (channel)
            {
            case BMA2x2_INT1_OUTPUT:
               comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_INT1_PAD_OUTPUT_TYPE__REG,&data,C_BMA2x2_One_U8X);
               *int_od = BMA2x2_GET_BITSLICE(data,BMA2x2_INT1_PAD_OUTPUT_TYPE);
               break;
            case BMA2x2_INT2_OUTPUT:
               comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_INT2_PAD_OUTPUT_TYPE__REG,&data,C_BMA2x2_One_U8X);
               *int_od = BMA2x2_GET_BITSLICE(data,BMA2x2_INT2_PAD_OUTPUT_TYPE);
               break;
            default:
               comres = E_BMA2x2_OUT_OF_RANGE;
               break;
            }
         }
      return comres;
      }
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to set the output type status
 *
 *
 *
 *
 *  \param unsigned char channel,unsigned char int_od
 *                  BMA2x2_INT1_OUTPUT    ->   0
 *                  BMA2x2_INT2_OUTPUT    ->   1
 *                  int_od : open drain   ->   1
 *                           push pull    ->   0
 *
 *
 *
 *  \return communication results
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_int_od (unsigned char channel,unsigned char int_od)
   {
      {
      unsigned char data;
      int comres=C_BMA2x2_Zero_U8X;
      if (p_bma2x2==C_BMA2x2_Zero_U8X)
         {
         comres = E_SMB_NULL_PTR;
         }
      else
         {
         switch (channel)
            {
            case BMA2x2_INT1_OUTPUT:
               comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_INT1_PAD_OUTPUT_TYPE__REG, &data, C_BMA2x2_One_U8X);
               data = BMA2x2_SET_BITSLICE(data, BMA2x2_INT1_PAD_OUTPUT_TYPE, int_od);
               comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_INT1_PAD_OUTPUT_TYPE__REG, &data, C_BMA2x2_One_U8X);
               break;
            case BMA2x2_INT2_OUTPUT:
               comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_INT2_PAD_OUTPUT_TYPE__REG, &data, C_BMA2x2_One_U8X);
               data = BMA2x2_SET_BITSLICE(data, BMA2x2_INT2_PAD_OUTPUT_TYPE, int_od);
               comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_INT2_PAD_OUTPUT_TYPE__REG, &data, C_BMA2x2_One_U8X);
               break;
            default:
               comres = E_BMA2x2_OUT_OF_RANGE;
               break;
            }
         }
      return comres;
      }
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to get Active Level status
 *
 *
 *
 *
 *  \param unsigned char channel,unsigned char *int_lvl
 *                  BMA2x2_INT1_LEVEL    ->    0
 *                  BMA2x2_INT2_LEVEL    ->    1
 *                  int_lvl : Active HI   ->   1
 *                            Active LO   ->   0
 *
 *
 *
 *  \return
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_int_lvl (unsigned char channel,unsigned char *int_lvl)
   {
      {
      unsigned char data;
      int comres=C_BMA2x2_Zero_U8X;
      if (p_bma2x2==C_BMA2x2_Zero_U8X)
         {
         comres = E_SMB_NULL_PTR;
         }
      else
         {
         switch (channel)
            {
            case BMA2x2_INT1_LEVEL:
               comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_INT1_PAD_ACTIVE_LEVEL__REG,&data,C_BMA2x2_One_U8X);
               *int_lvl = BMA2x2_GET_BITSLICE(data,BMA2x2_INT1_PAD_ACTIVE_LEVEL);
               break;
            case BMA2x2_INT2_LEVEL:
               comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_INT2_PAD_ACTIVE_LEVEL__REG,&data,C_BMA2x2_One_U8X);
               *int_lvl = BMA2x2_GET_BITSLICE(data,BMA2x2_INT2_PAD_ACTIVE_LEVEL);
               break;
            default:
               comres = E_BMA2x2_OUT_OF_RANGE;
               break;
            }
         }
      return comres;
      }
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to set Active Level status
 *
 *
 *
 *
 *  \param (unsigned char channel,unsigned char int_lvl)
 *                  BMA2x2_INT1_LEVEL    ->    0
 *                  BMA2x2_INT2_LEVEL    ->    1
 *                  int_lvl : Active HI   ->   1
 *                            Active LO   ->   0
 *
 *
 *
 *  \return communication results
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_int_lvl (unsigned char channel,unsigned char int_lvl)
   {
      {
      unsigned char data;
      int comres=C_BMA2x2_Zero_U8X;
      if (p_bma2x2==C_BMA2x2_Zero_U8X)
         {
         comres = E_SMB_NULL_PTR;
         }
      else
         {
         switch (channel)
            {
            case BMA2x2_INT1_LEVEL:
               comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_INT1_PAD_ACTIVE_LEVEL__REG, &data, C_BMA2x2_One_U8X);
               data = BMA2x2_SET_BITSLICE(data, BMA2x2_INT1_PAD_ACTIVE_LEVEL, int_lvl);
               comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_INT1_PAD_ACTIVE_LEVEL__REG, &data, C_BMA2x2_One_U8X);
               break;
            case BMA2x2_INT2_LEVEL:
               comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_INT2_PAD_ACTIVE_LEVEL__REG, &data, C_BMA2x2_One_U8X);
               data = BMA2x2_SET_BITSLICE(data, BMA2x2_INT2_PAD_ACTIVE_LEVEL, int_lvl);
               comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_INT2_PAD_ACTIVE_LEVEL__REG, &data, C_BMA2x2_One_U8X);
               break;
            default:
               comres = E_BMA2x2_OUT_OF_RANGE;
               break;
            }
         }
      return comres;
      }
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to set the reset interrupt
 *
 *
 *
 *
 *  \param unsigned char reset_int
 *
 *
 *
 *  \return communication results
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_reset_interrupt (unsigned char reset_int)
   {
      {
      unsigned char data;
      int comres=C_BMA2x2_Zero_U8X;
      if (p_bma2x2==C_BMA2x2_Zero_U8X)
         {
         comres = E_SMB_NULL_PTR;
         }
      else
         {
         comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_RESET_INT__REG, &data, C_BMA2x2_One_U8X);
         data = BMA2x2_SET_BITSLICE(data, BMA2x2_RESET_INT, reset_int);
         comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_RESET_INT__REG, &data, C_BMA2x2_One_U8X);
         }
      return comres;
      }
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to get the latch duration
 *
 *
 *
 *
 *  \param unsigned char *latch_int : Address of latch_int
 *                  0 -> NON_LATCH
 *                  1 -> 250MS
 *                  2 -> 500MS
 *                  3 -> 1S
 *                  4 -> 2S
 *                  5 -> 4S
 *                  6 -> 8S
 *                  7 -> LATCH
 *                  8 -> NON_LATCH1
 *                  9 -> 250US
 *                 10 -> 500US
 *                 11 -> 1MS
 *                 12 -> 12.5MS
 *                 13 -> 25MS
 *                 14 -> 50MS
 *                 15 -> LATCH1
 *
 *
 *
 *  \return
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_latch_int (unsigned char *latch_int)
   {
      {
      unsigned char data;
      int comres=C_BMA2x2_Zero_U8X;
      if (p_bma2x2==C_BMA2x2_Zero_U8X)
         {
         comres = E_SMB_NULL_PTR;
         }
      else
         {
         comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_LATCH_INT__REG,&data,C_BMA2x2_One_U8X);
         *latch_int = BMA2x2_GET_BITSLICE(data,BMA2x2_LATCH_INT);
         }
      return comres;
      }
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to set the latch duration
 *
 *
 *
 *
 *  \param unsigned char latch_int
 *                  0 -> NON_LATCH
 *                  1 -> 250MS
 *                  2 -> 500MS
 *                  3 -> 1S
 *                  4 -> 2S
 *                  5 -> 4S
 *                  6 -> 8S
 *                  7 -> LATCH
 *                  8 -> NON_LATCH1
 *                  9 -> 250US
 *                 10 -> 500US
 *                 11 -> 1MS
 *                 12 -> 12.5MS
 *                 13 -> 25MS
 *                 14 -> 50MS
 *                 15 -> LATCH1
 *
 *  \return communication results
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_latch_int (unsigned char latch_int)
   {
   unsigned char data;
   int comres = C_BMA2x2_Zero_U8X ;
   int latch_duration = 0;
   if (p_bma2x2 == C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      if (latch_int < C_BMA2x2_Sixteen_U8X)
         {
         switch (latch_int)
            {
            case BMA2x2_LATCH_DUR_NON_LATCH:
               latch_duration = BMA2x2_LATCH_DUR_NON_LATCH;

               /*  NON LATCH   */
               break;
            case BMA2x2_LATCH_DUR_250MS:
               latch_duration = BMA2x2_LATCH_DUR_250MS;

               /*  250 MS  */
               break;
            case BMA2x2_LATCH_DUR_500MS:
               latch_duration = BMA2x2_LATCH_DUR_500MS;

               /*  500 MS  */
               break;
            case BMA2x2_LATCH_DUR_1S:
               latch_duration = BMA2x2_LATCH_DUR_1S;

               /*  1 S   */
               break;
            case BMA2x2_LATCH_DUR_2S:
               latch_duration = BMA2x2_LATCH_DUR_2S;

               /*  2 S  */
               break;
            case BMA2x2_LATCH_DUR_4S:
               latch_duration = BMA2x2_LATCH_DUR_4S;

               /*  4 S  */
               break;
            case BMA2x2_LATCH_DUR_8S:
               latch_duration = BMA2x2_LATCH_DUR_8S;

               /*  8 S  */
               break;
            case BMA2x2_LATCH_DUR_LATCH:
               latch_duration = BMA2x2_LATCH_DUR_LATCH;

               /*  LATCH  */
               break;
            case BMA2x2_LATCH_DUR_NON_LATCH1:
               latch_duration = BMA2x2_LATCH_DUR_NON_LATCH1;

               /*  NON LATCH1  */
               break;
            case BMA2x2_LATCH_DUR_250US:
               latch_duration = BMA2x2_LATCH_DUR_250US;

               /*  250 US   */
               break;
            case BMA2x2_LATCH_DUR_500US:
               latch_duration = BMA2x2_LATCH_DUR_500US;

               /*  500 US   */
               break;
            case BMA2x2_LATCH_DUR_1MS:
               latch_duration = BMA2x2_LATCH_DUR_1MS;

               /*  1 MS   */
               break;
            case BMA2x2_LATCH_DUR_12_5MS:
               latch_duration = BMA2x2_LATCH_DUR_12_5MS;

               /*  12.5 MS   */
               break;
            case BMA2x2_LATCH_DUR_25MS:
               latch_duration = BMA2x2_LATCH_DUR_25MS;

               /*  25 MS   */
               break;
            case BMA2x2_LATCH_DUR_50MS:
               latch_duration = BMA2x2_LATCH_DUR_50MS;

               /*  50 MS   */
               break;
            case BMA2x2_LATCH_DUR_LATCH1:
               latch_duration = BMA2x2_LATCH_DUR_LATCH1;

               /*  LATCH1   */
               break;
            default:
               break;
            }
         comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_LATCH_INT__REG, &data, C_BMA2x2_One_U8X);
         data = BMA2x2_SET_BITSLICE(data, BMA2x2_LATCH_INT, latch_duration);
         comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_LATCH_INT__REG, &data, C_BMA2x2_One_U8X);
         }
      else
         {
         comres = E_OUT_OF_RANGE ;
         }
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to get duration
 *
 *
 *
 *
 *  \param unsigned char channel,unsigned char *dur
 *          BMA2x2_LOW_DURATION            0,1
 *          BMA2x2_HIGH_DURATION           1,2
 *          BMA2x2_SLOPE_DURATION          2,3
 *          BMA2x2_SLO_NO_MOT_DURATION     3,4
 *
 *
 *
 *  \return
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_dur(unsigned char channel,unsigned char *dur)
   {
   unsigned char data;
   int comres = C_BMA2x2_Zero_U8X;
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      switch (channel)
         {
         case BMA2x2_LOW_DURATION:
            //LOW DURATION
            comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_LOW_DURN_REG,&data,C_BMA2x2_One_U8X);
            *dur = data;
            break;
         case BMA2x2_HIGH_DURATION:
            //HIGH DURATION
            comres =p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_HIGH_DURN_REG,&data,C_BMA2x2_One_U8X);
            *dur = data;
            break;
         case BMA2x2_SLOPE_DURATION:
            //SLOPE DURATION
            comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_SLOPE_DUR__REG,&data,C_BMA2x2_One_U8X);
            *dur = BMA2x2_GET_BITSLICE(data,BMA2x2_SLOPE_DUR);
            break;
         case BMA2x2_SLO_NO_MOT_DURATION:
            //SLO NO MOT DURATION
            comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_SLO_NO_MOT_DUR__REG,&data,C_BMA2x2_One_U8X);
            *dur = BMA2x2_GET_BITSLICE(data,BMA2x2_SLO_NO_MOT_DUR);
            break;
         default:
            comres = E_BMA2x2_OUT_OF_RANGE;
            break;
         }
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to set duration
 *
 *
 *
 *
 *  \param unsigned char channel,unsigned char dur
 *          BMA2x2_LOW_DURATION            0,1
 *          BMA2x2_HIGH_DURATION           1,2
 *          BMA2x2_SLOPE_DURATION          2,3
 *          BMA2x2_SLO_NO_MOT_DURATION     3,4
 *
 *
 *
 *  \return communication results
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_dur (unsigned char channel,unsigned char dur)
   {
   unsigned char data;
   int comres = C_BMA2x2_Zero_U8X;
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      switch (channel)
         {
         case BMA2x2_LOW_DURATION:
            //LOW DURATION
            data = dur;
            comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_LOW_DURN_REG, &data, C_BMA2x2_One_U8X);
            break;
         case BMA2x2_HIGH_DURATION:
            //HIGH DURATION
            data = dur;
            comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_HIGH_DURN_REG, &data,C_BMA2x2_One_U8X);
            break;
         case BMA2x2_SLOPE_DURATION:
            //SLOPE DURATION
            comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_SLOPE_DUR__REG, &data, C_BMA2x2_One_U8X);
            data = BMA2x2_SET_BITSLICE(data, BMA2x2_SLOPE_DUR, dur);
            comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_SLOPE_DUR__REG, &data, C_BMA2x2_One_U8X);
            break;
         case BMA2x2_SLO_NO_MOT_DURATION:
            //SLO NO MOT DURATION
            comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_SLO_NO_MOT_DUR__REG, &data, C_BMA2x2_One_U8X);
            data = BMA2x2_SET_BITSLICE(data, BMA2x2_SLO_NO_MOT_DUR, dur);
            comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_SLO_NO_MOT_DUR__REG, &data, C_BMA2x2_One_U8X);
            break;
         default:
            comres = E_BMA2x2_OUT_OF_RANGE;
            break;
         }
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to get threshold
 *
 *
 *
 *
 *  \param unsigned char channel,unsigned char *thr
 *               BMA2x2_LOW_THRESHOLD            0,FE
 *               BMA2x2_HIGH_THRESHOLD           1,01
 *               BMA2x2_SLOPE_THRESHOLD          2,01
 *               BMA2x2_SLO_NO_MOT_THRESHOLD     3,00
 *
 *
 *
 *  \return
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_thr(unsigned char channel,unsigned char *thr)
   {
   unsigned char data;
   int comres = C_BMA2x2_Zero_U8X;
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      switch (channel)
         {
         case BMA2x2_LOW_THRESHOLD:
            //LOW THRESHOLD
            comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_LOW_THRES_REG,&data,C_BMA2x2_One_U8X);
            *thr = data;
            break;
         case BMA2x2_HIGH_THRESHOLD:
            //HIGH THRESHOLD
            comres =p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_HIGH_THRES_REG,&data,C_BMA2x2_One_U8X);
            *thr = data;
            break;
         case BMA2x2_SLOPE_THRESHOLD:
            //SLOPE THRESHOLD
            comres =p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_SLOPE_THRES_REG,&data,C_BMA2x2_One_U8X);
            *thr = data;
            break;
         case BMA2x2_SLO_NO_MOT_THRESHOLD:
            //SLO NO MOT THRESHOLD
            comres =p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_SLO_NO_MOT_THRES_REG,&data,C_BMA2x2_One_U8X);
            *thr = data;
            break;
         default:
            comres = E_BMA2x2_OUT_OF_RANGE;
            break;
         }
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to set threshold
 *
 *
 *
 *
 *  \param unsigned char channel,unsigned char thr
 *               BMA2x2_LOW_THRESHOLD            0,FE
 *               BMA2x2_HIGH_THRESHOLD           1,01
 *               BMA2x2_SLOPE_THRESHOLD          2,01
 *               BMA2x2_SLO_NO_MOT_THRESHOLD     3,00
 *
 *
 *
 *  \return communication results
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_thr (unsigned char channel,unsigned char thr)
   {
   unsigned char data;
   int comres = C_BMA2x2_Zero_U8X;
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      switch (channel)
         {
         case BMA2x2_LOW_THRESHOLD:
            //LOW THRESHOLD
            data = thr;
            comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_LOW_THRES_REG, &data, C_BMA2x2_One_U8X);
            break;
         case BMA2x2_HIGH_THRESHOLD:
            //HIGH THRESHOLD
            data = thr;
            comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_HIGH_THRES_REG, &data,C_BMA2x2_One_U8X);
            break;
         case BMA2x2_SLOPE_THRESHOLD:
            //SLOPE THRESHOLD
            data = thr;
            comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr,BMA2x2_SLOPE_THRES_REG, &data,C_BMA2x2_One_U8X);
            break;
         case BMA2x2_SLO_NO_MOT_THRESHOLD:
            //SLO NO MOT THRESHOLD
            data = thr;
            comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr,BMA2x2_SLO_NO_MOT_THRES_REG, &data,C_BMA2x2_One_U8X);
            break;
         default:
            comres = E_BMA2x2_OUT_OF_RANGE;
            break;
         }
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to get low high hysteresis
 *
 *
 *
 *
 *  \param unsigned char channel,unsigned char *hyst
 *                  channel -->
 *                  BMA2x2_LOWG_HYST                0
 *                  BMA2x2_HIGHG_HYST               1
 *                  hyst --> 1
 *
 *
 *
 *  \return
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_low_high_hyst(unsigned char channel,unsigned char *hyst)
   {
   unsigned char data;
   int comres = C_BMA2x2_Zero_U8X;
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      switch (channel)
         {
         case BMA2x2_LOWG_HYST:
            comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_LOWG_HYST__REG,&data,C_BMA2x2_One_U8X);
            *hyst = BMA2x2_GET_BITSLICE(data,BMA2x2_LOWG_HYST);
            break;
         case BMA2x2_HIGHG_HYST:
            comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_HIGHG_HYST__REG,&data,C_BMA2x2_One_U8X);
            *hyst = BMA2x2_GET_BITSLICE(data,BMA2x2_HIGHG_HYST);
            break;
         default:
            comres = E_BMA2x2_OUT_OF_RANGE;
            break;
         }
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to set low high hysteresis
 *
 *
 *
 *
 *  \param (unsigned char channel,unsigned char hyst)
 *                      channel -->
 *                      BMA2x2_LOWG_HYST                0
 *                      BMA2x2_HIGHG_HYST               1
 *                      hyst --> 1
 *
 *
 *
 *  \return communication results
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_low_high_hyst (unsigned char channel,unsigned char hyst)
   {
   unsigned char data;
   int comres = C_BMA2x2_Zero_U8X;
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      switch (channel)
         {
         case BMA2x2_LOWG_HYST:
            comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_LOWG_HYST__REG, &data, C_BMA2x2_One_U8X);
            data = BMA2x2_SET_BITSLICE(data,BMA2x2_LOWG_HYST, hyst);
            comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_LOWG_HYST__REG, &data, C_BMA2x2_One_U8X);
            break;
         case BMA2x2_HIGHG_HYST:
            comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_HIGHG_HYST__REG, &data,  C_BMA2x2_One_U8X);
            data = BMA2x2_SET_BITSLICE(data, BMA2x2_HIGHG_HYST, hyst);
            comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_HIGHG_HYST__REG, &data,  C_BMA2x2_One_U8X);
            break;
         default:
            comres = E_BMA2x2_OUT_OF_RANGE;
            break;
         }
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to get low high hysteresis mode
 *
 *
 *
 *
 *  \param unsigned char *mode
 *           0 -> single mode
 *           1 -> sum mode
 *
 *
 *
 *  \return
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_low_high_hyst_mode(unsigned char *mode)
   {
   unsigned char data;
   int comres = C_BMA2x2_Zero_U8X;
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_LOWG_INT_MODE__REG,&data,C_BMA2x2_One_U8X);
      *mode = BMA2x2_GET_BITSLICE(data,BMA2x2_LOWG_INT_MODE);
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to set low high hysteresis mode
 *
 *
 *
 *
 *  \param (unsigned char mode)
 *             0 -> single mode
 *             1 -> sum mode
 *
 *
 *
 *  \return communication results
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_low_high_hyst_mode (unsigned char mode)
   {
   unsigned char data;
   int comres = C_BMA2x2_Zero_U8X;
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_LOWG_INT_MODE__REG, &data, C_BMA2x2_One_U8X);
      data = BMA2x2_SET_BITSLICE(data,BMA2x2_LOWG_INT_MODE, mode);
      comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_LOWG_INT_MODE__REG, &data, C_BMA2x2_One_U8X);
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to get tap duration
 *
 *
 *
 *
 *  \param unsigned char *tap_dur : Address of tap_dur
 *        0 -> 50ms
 *        1 -> 100ms
 *        2 -> 150ms
 *        3 -> 200ms
 *        4 -> 250ms
 *        5 -> 375ms
 *        6 -> 500ms
 *        7 -> 700ms
 *
 *
 *
 *  \return
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_tap_dur (unsigned char *tap_dur)
   {
   unsigned char data;
   int comres = C_BMA2x2_Zero_U8X;
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_TAP_DUR__REG,&data,C_BMA2x2_One_U8X);
      *tap_dur = BMA2x2_GET_BITSLICE(data,BMA2x2_TAP_DUR);
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to set tap duration
 *
 *
 *
 *
 *  \param unsigned char tap_dur
 *        0 -> 50ms
 *        1 -> 100ms
 *        2 -> 150ms
 *        3 -> 200ms
 *        4 -> 250ms
 *        5 -> 375ms
 *        6 -> 500ms
 *        7 -> 700ms
 *
 *
 *
 *  \return communication results
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_tap_dur (unsigned char tap_dur)
   {
   unsigned char data;
   int comres = C_BMA2x2_Zero_U8X;
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_TAP_DUR__REG, &data, C_BMA2x2_One_U8X);
      data = BMA2x2_SET_BITSLICE(data,BMA2x2_TAP_DUR, tap_dur);
      comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_TAP_DUR__REG, &data, C_BMA2x2_One_U8X);
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to get tap shock
 *
 *
 *
 *
 *  \param unsigned char *tap_shock : Address of tap_shock
 *                      0 -> 50ms
 *                      1 -> 75ms
 *
 *
 *
 *  \return
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_tap_shock (unsigned char *tap_shock)
   {
   unsigned char data;
   int comres = C_BMA2x2_Zero_U8X;
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_TAP_SHOCK_DURN__REG,&data,C_BMA2x2_One_U8X);
      *tap_shock = BMA2x2_GET_BITSLICE(data,BMA2x2_TAP_SHOCK_DURN);
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to set tap shock
 *
 *
 *
 *
 *  \param unsigned char tap_shock
 *            0 -> 50ms
 *            1 -> 75ms
 *
 *
 *
 *  \return communication results
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_tap_shock (unsigned char tap_shock)
   {
   unsigned char data;
   int comres = C_BMA2x2_Zero_U8X;
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_TAP_SHOCK_DURN__REG, &data, C_BMA2x2_One_U8X);
      data = BMA2x2_SET_BITSLICE(data,BMA2x2_TAP_SHOCK_DURN, tap_shock);
      comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_TAP_SHOCK_DURN__REG, &data, C_BMA2x2_One_U8X);
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to get tap quiet
 *
 *
 *
 *
 *  \param  unsigned char *tap_quiet : Address of tap_quiet
 *              0 -> 30ms
 *              1 -> 20ms
 *
 *
 *
 *  \return
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_tap_quiet (unsigned char *tap_quiet)
   {
   unsigned char data;
   int comres = C_BMA2x2_Zero_U8X;
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_TAP_QUIET_DURN__REG,&data,C_BMA2x2_One_U8X);
      *tap_quiet = BMA2x2_GET_BITSLICE(data,BMA2x2_TAP_QUIET_DURN);
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to set tap quiet
 *
 *
 *
 *
 *  \param unsigned char tap_quiet
 *                0 -> 30ms
 *                1 -> 20ms
 *
 *
 *
 *  \return communication results
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_tap_quiet (unsigned char tap_quiet)
   {
   unsigned char data;
   int comres = C_BMA2x2_Zero_U8X;
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_TAP_QUIET_DURN__REG, &data, C_BMA2x2_One_U8X);
      data = BMA2x2_SET_BITSLICE(data,BMA2x2_TAP_QUIET_DURN, tap_quiet);
      comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_TAP_QUIET_DURN__REG, &data, C_BMA2x2_One_U8X);
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to get tap threshold
 *
 *
 *
 *
 *  \param unsigned char *tap_thr : Address of tap_thr
 *
 *
 *
 *  \return
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_tap_thr (unsigned char *tap_thr)
   {
   unsigned char data;
   int comres = C_BMA2x2_Zero_U8X;
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_TAP_THRES__REG,&data,C_BMA2x2_One_U8X);
      *tap_thr = BMA2x2_GET_BITSLICE(data,BMA2x2_TAP_THRES);
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to set tap threshold
 *
 *
 *
 *
 *  \param unsigned char tap_thr
 *
 *
 *
 *  \return communication results
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_tap_thr (unsigned char tap_thr)
   {
   unsigned char data;
   int comres = C_BMA2x2_Zero_U8X;
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_TAP_THRES__REG, &data, C_BMA2x2_One_U8X);
      data = BMA2x2_SET_BITSLICE(data,BMA2x2_TAP_THRES, tap_thr);
      comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_TAP_THRES__REG, &data, C_BMA2x2_One_U8X);
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to get tap sample
 *
 *
 *
 *
 *  \param unsigned char  *tap_sample : Address of tap_sample
 *
 *
 *
 *  \return
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_tap_sample (unsigned char *tap_sample)
   {
   unsigned char data;
   int comres = C_BMA2x2_Zero_U8X;
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_TAP_SAMPLES__REG,&data,C_BMA2x2_One_U8X);
      *tap_sample = BMA2x2_GET_BITSLICE(data,BMA2x2_TAP_SAMPLES);
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to set tap sample
 *
 *
 *
 *
 *  \param unsigned char tap_sample
 *
 *
 *
 *  \return communication results
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_tap_sample (unsigned char tap_sample)
   {
   unsigned char data;
   int comres = C_BMA2x2_Zero_U8X;
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_TAP_SAMPLES__REG, &data, C_BMA2x2_One_U8X);
      data = BMA2x2_SET_BITSLICE(data,BMA2x2_TAP_SAMPLES, tap_sample);
      comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_TAP_SAMPLES__REG, &data, C_BMA2x2_One_U8X);
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to get orient mode
 *
 *
 *
 *
 *  \param unsigned char *orient_mode : Address of orient_mode
 *                00 -> 45' symmetrical
 *                01 -> 63' high asymmetrical
 *                10 -> 27' low asymmetrical
 *                11 -> reserved
 *
 *
 *
 *  \return
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_orient_mode (unsigned char *orient_mode)
   {
   unsigned char data;
   int comres = C_BMA2x2_Zero_U8X;
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_ORIENT_MODE__REG,&data,C_BMA2x2_One_U8X);
      *orient_mode = BMA2x2_GET_BITSLICE(data,BMA2x2_ORIENT_MODE);
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to set orient mode
 *
 *
 *
 *
 *  \param unsigned char orient_mode
 *                00 -> 45' symmetrical
 *                01 -> 63' high asymmetrical
 *                10 -> 27' low asymmetrical
 *                11 -> reserved
 *
 *
 *
 *  \return communication results
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_orient_mode (unsigned char orient_mode)
   {
   unsigned char data;
   int comres = C_BMA2x2_Zero_U8X;
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_ORIENT_MODE__REG, &data, C_BMA2x2_One_U8X);
      data = BMA2x2_SET_BITSLICE(data,BMA2x2_ORIENT_MODE, orient_mode);
      comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_ORIENT_MODE__REG, &data, C_BMA2x2_One_U8X);
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to get orient block
 *
 *
 *
 *
 *  \param unsigned char *orient_block : Address of orient_block
 *               00 -> disabled
 *               01 -> horizontal position or acc >1.75g
 *               10 -> horizontal position or acc >1.75g or slope > 0.2g
 *               11 -> horizontal position or acc >1.75g or slope > 0.4g or wait 100ms
 *
 *
 *  \return
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_orient_block (unsigned char *orient_block)
   {
   unsigned char data;
   int comres = C_BMA2x2_Zero_U8X;
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_ORIENT_BLOCK__REG,&data,C_BMA2x2_One_U8X);
      *orient_block = BMA2x2_GET_BITSLICE(data,BMA2x2_ORIENT_BLOCK);
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to set orient block
 *
 *
 *
 *
 *  \param unsigned char orient_block
 *               00 -> disabled
 *               01 -> horizontal position or acc >1.75g
 *               10 -> horizontal position or acc >1.75g or slope > 0.2g
 *               11 -> horizontal position or acc >1.75g or slope > 0.4g or wait 100ms
 *
 *
 *  \return communication results
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_orient_block (unsigned char orient_block)
   {
   unsigned char data;
   int comres = C_BMA2x2_Zero_U8X;
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_ORIENT_BLOCK__REG, &data, C_BMA2x2_One_U8X);
      data = BMA2x2_SET_BITSLICE(data,BMA2x2_ORIENT_BLOCK, orient_block);
      comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_ORIENT_BLOCK__REG, &data, C_BMA2x2_One_U8X);
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to get orient hysteresis
 *
 *
 *
 *
 *  \param unsigned char *orient_hyst : Address of orient_hyst
 *
 *
 *
 *  \return
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_orient_hyst (unsigned char *orient_hyst)
   {
   unsigned char data;
   int comres = C_BMA2x2_Zero_U8X;
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_ORIENT_HYST__REG,&data,C_BMA2x2_One_U8X);
      *orient_hyst = BMA2x2_GET_BITSLICE(data,BMA2x2_ORIENT_HYST);
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to set orient hysteresis
 *
 *
 *
 *
 *  \param unsigned char orient_hyst
 *
 *
 *
 *  \return communication results
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_orient_hyst (unsigned char orient_hyst)
   {
   unsigned char data;
   int comres = C_BMA2x2_Zero_U8X;
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_ORIENT_HYST__REG, &data, C_BMA2x2_One_U8X);
      data = BMA2x2_SET_BITSLICE(data,BMA2x2_ORIENT_HYST, orient_hyst);
      comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_ORIENT_HYST__REG, &data, C_BMA2x2_One_U8X);
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief  This API is used to get theta
 *
 *
 *
 *
 *  \param unsigned char channel,unsigned char *theta
 *              channel -->
 *              BMA2x2_ORIENT_THETA             0
 *              BMA2x2_FLAT_THETA               1
 *              theta --> Any valid value
 *
 *
 *
 *  \return
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_theta(unsigned char channel,unsigned char *theta)
   {
   unsigned char data;
   int comres = C_BMA2x2_Zero_U8X;
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      switch (channel)
         {
         case BMA2x2_ORIENT_THETA:
            //ORIENT THETA
            comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_THETA_BLOCK__REG,&data,C_BMA2x2_One_U8X);
            *theta = BMA2x2_GET_BITSLICE(data,BMA2x2_THETA_BLOCK);
            break;
         case BMA2x2_FLAT_THETA:
            //FLAT THETA
            comres =p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_THETA_FLAT__REG,&data,C_BMA2x2_One_U8X);
            *theta = data;
            break;
         default:
            comres = E_BMA2x2_OUT_OF_RANGE;
            break;
         }
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to set theta
 *
 *
 *
 *
 *  \param unsigned char channel,unsigned char theta
 *               channel -->
 *               BMA2x2_ORIENT_THETA             0
 *               BMA2x2_FLAT_THETA               1
 *               theta --> Any valid value
 *
 *
 *
 *  \return communication results
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_theta (unsigned char channel,unsigned char theta)
   {
   unsigned char data;
   int comres = C_BMA2x2_Zero_U8X;
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      switch (channel)
         {
         case BMA2x2_ORIENT_THETA:
            //ORIENT THETA
            comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_THETA_BLOCK__REG, &data, C_BMA2x2_One_U8X);
            data = BMA2x2_SET_BITSLICE(data, BMA2x2_THETA_BLOCK, theta);
            comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_THETA_BLOCK__REG, &data, C_BMA2x2_One_U8X);
            break;
         case BMA2x2_FLAT_THETA:
            //FLAT THETA
            data = theta;
            comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_THETA_FLAT__REG, &data,C_BMA2x2_One_U8X);
            break;
         default:
            comres = E_BMA2x2_OUT_OF_RANGE;
            break;
         }
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to get the status of orient enable
 *
 *
 *
 *
 *  \param unsigned char *orient_en : Address of orient_en
 *                        1 -> Generates Interrupt
 *                        0 -> Do not generate interrupt
 *
 *
 *
 *  \return
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_orient_en (unsigned char *orient_en)
   {
   unsigned char data;
   int comres = C_BMA2x2_Zero_U8X;
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_ORIENT_UD_EN__REG,&data,C_BMA2x2_One_U8X);
      *orient_en = BMA2x2_GET_BITSLICE(data,BMA2x2_ORIENT_UD_EN);
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to set the status of orient enable
 *
 *
 *
 *
 *  \param unsigned char orient_en
 *         1 -> Generates Interrupt
 *         0 -> Do not generate interrupt
 *
 *
 *
 *  \return communication results
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_orient_en (unsigned char orient_en)
   {
   unsigned char data;
   int comres = C_BMA2x2_Zero_U8X;
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_ORIENT_UD_EN__REG, &data, C_BMA2x2_One_U8X);
      data = BMA2x2_SET_BITSLICE(data,BMA2x2_ORIENT_UD_EN, orient_en);
      comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_ORIENT_UD_EN__REG, &data, C_BMA2x2_One_U8X);
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to get the status of flat hyst
 *
 *
 *
 *
 *  \param unsigned char *flat_hyst : Address of flat_hyst
 *
 *
 *
 *  \return
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_flat_hyst (unsigned char *flat_hyst)
   {
   unsigned char data;
   int comres = C_BMA2x2_Zero_U8X;
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_FLAT_HYS__REG,&data,C_BMA2x2_One_U8X);
      *flat_hyst = BMA2x2_GET_BITSLICE(data,BMA2x2_FLAT_HYS);
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to set the status of flat hyst
 *
 *
 *
 *
 *  \param unsigned char flat_hyst
 *
 *
 *
 *  \return communication results
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_flat_hyst (unsigned char flat_hyst)
   {
   unsigned char data;
   int comres = C_BMA2x2_Zero_U8X;
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_FLAT_HYS__REG, &data, C_BMA2x2_One_U8X);
      data = BMA2x2_SET_BITSLICE(data,BMA2x2_FLAT_HYS, flat_hyst);
      comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_FLAT_HYS__REG, &data, C_BMA2x2_One_U8X);
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to get the status of flat hold time
 *
 *
 *
 *
 *  \param  unsigned char *flat_hold_time : Address of flat_hold_time
 *               00 -> disabled
 *               01 -> 512ms
 *               10 -> 1024ms
 *               11 -> 2048ms
 *
 *
 *
 *  \return
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_flat_hold_time (unsigned char *flat_hold_time)
   {
   unsigned char data;
   int comres = C_BMA2x2_Zero_U8X;
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_FLAT_HOLD_TIME__REG,&data,C_BMA2x2_One_U8X);
      *flat_hold_time = BMA2x2_GET_BITSLICE(data,BMA2x2_FLAT_HOLD_TIME);
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to set the status of flat hold time
 *
 *
 *
 *
 *  \param unsigned char flat_hold_time
 *               00 -> disabled
 *               01 -> 512ms
 *               10 -> 1024ms
 *               11 -> 2048ms
 *
 *
 *
 *  \return communication results
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_flat_hold_time (unsigned char flat_hold_time)
   {
   unsigned char data;
   int comres = C_BMA2x2_Zero_U8X;
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_FLAT_HOLD_TIME__REG, &data, C_BMA2x2_One_U8X);
      data = BMA2x2_SET_BITSLICE(data,BMA2x2_FLAT_HOLD_TIME, flat_hold_time);
      comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_FLAT_HOLD_TIME__REG, &data, C_BMA2x2_One_U8X);
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/***********************************************************************************************
 * Description: *//**\brief This API is used to get the fifo water mark level trigger status
 *
 *
 *
 *
 *  \param fifo_wml_trig
 *
 *
 *
 *  \return
 *
 *
 ***********************************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_fifo_wml_trig (unsigned char *fifo_wml_trig)
   {
   unsigned char data;
   int comres = C_BMA2x2_Zero_U8X;
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_FIFO_WML_TRIG_RETAIN__REG,&data,C_BMA2x2_One_U8X);
      *fifo_wml_trig = BMA2x2_GET_BITSLICE(data,BMA2x2_FIFO_WML_TRIG_RETAIN);
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/**********************************************************************************************
 * Description: *//**\brief This API is used to set the fifo water mark level trigger status
 *
 *
 *
 *
 *  \param unsigned char fifo_wml_trig
 *                    0-31
 *
 *
 *
 *  \return
 *
 *
 **********************************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_fifo_wml_trig (unsigned char fifo_wml_trig)
   {
   unsigned char data;
   int comres = C_BMA2x2_Zero_U8X;
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      if (fifo_wml_trig < C_BMA2x2_ThirtyTwo_U8X)
         {
         comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_FIFO_WML_TRIG_RETAIN__REG, &data, C_BMA2x2_One_U8X);
         data = BMA2x2_SET_BITSLICE(data,BMA2x2_FIFO_WML_TRIG_RETAIN, fifo_wml_trig);
         comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_FIFO_WML_TRIG_RETAIN__REG, &data, C_BMA2x2_One_U8X);
         }
      else
         {
         comres = E_OUT_OF_RANGE ;
         }
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is for to get Self Test Axis
 *
 *
 *
 *
 *  \param unsigned char *self_test_axis : Address of self_test_axis
 *                          Possible values [1:0] are 0 to 3.
 *
 *
 *
 *  \return
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_self_test_axis (unsigned char *self_test_axis)
   {
   unsigned char data;
   int comres = C_BMA2x2_Zero_U8X;
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_EN_SELF_TEST__REG,&data,C_BMA2x2_One_U8X);
      *self_test_axis = BMA2x2_GET_BITSLICE(data,BMA2x2_EN_SELF_TEST);
      }
   return comres;
   }
/*******************************************************************************
 * Description: *//**\brief This API is for to Set Self Test Axis
 *
 *
 *
 *
 *  \param unsigned char self_test_axis
 *
 *                      Possible values [1:0] are 0 to 3.
 *
 *
 *
 *
 *
 *
 *
 *  \return Communication Results
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_self_test_axis (unsigned char self_test_axis)
   {
   unsigned char data;
   int comres = C_BMA2x2_Zero_U8X;
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      if (self_test_axis < C_BMA2x2_Four_U8X)
         {
         comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_EN_SELF_TEST__REG, &data, C_BMA2x2_One_U8X);
         data = BMA2x2_SET_BITSLICE(data,BMA2x2_EN_SELF_TEST, self_test_axis);
         comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_EN_SELF_TEST__REG, &data, C_BMA2x2_One_U8X);
         }
      else
         {
         comres = E_OUT_OF_RANGE ;
         }
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is for to get Self Test sign
 *
 *
 *
 *
 *  \param unsigned char *self_test_sign : Address of self_test_sign
 *                              0 => '+'ve sign
 *                              1 => '-'ve sign
 *
 *
 *
 *  \return
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_self_test_sign (unsigned char *self_test_sign)
   {
   unsigned char data;
   int comres = C_BMA2x2_Zero_U8X;
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_NEG_SELF_TEST__REG,&data,C_BMA2x2_One_U8X);
      *self_test_sign = BMA2x2_GET_BITSLICE(data,BMA2x2_NEG_SELF_TEST);
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is for to set Self Test sign
 *
 *
 *
 *
 *  \param  unsigned char self_test_sign
 *          0 => '+'ve sign
 *          1 => '-'ve sign
 *
 *
 *  \return communication results
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_self_test_sign (unsigned char self_test_sign)
   {
   unsigned char data;
   int comres = C_BMA2x2_Zero_U8X;
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      if (self_test_sign < C_BMA2x2_Two_U8X)
         {
         comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_NEG_SELF_TEST__REG, &data, C_BMA2x2_One_U8X);
         data = BMA2x2_SET_BITSLICE(data,BMA2x2_NEG_SELF_TEST, self_test_sign);
         comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_NEG_SELF_TEST__REG, &data, C_BMA2x2_One_U8X);
         }
      else
         {
         comres = E_OUT_OF_RANGE ;
         }
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to get the status of nvm program mode
 *
 *
 *
 *
 *  \param  unsigned char *nvmprog_mode : Address of *nvmprog_mode
 *                  1 -> Enable program mode
 *                   0 -> Disable program mode
 *
 *
 *
 *  \return
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_nvmprog_mode (unsigned char *nvmprog_mode)
   {
   unsigned char data;
   int comres = C_BMA2x2_Zero_U8X;
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_UNLOCK_EE_PROG_MODE__REG,&data,C_BMA2x2_One_U8X);
      *nvmprog_mode = BMA2x2_GET_BITSLICE(data,BMA2x2_UNLOCK_EE_PROG_MODE);
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to set the status of nvmprogram mode
 *
 *
 *
 *
 *  \param (unsigned char prgmode)
 *                   1 -> Enable program mode
 *                   0 -> Disable program mode
 *
 *
 *
 *  \return communication results
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma2x2_set_nvmprog_mode(unsigned char prgmode)
   {
   unsigned char data;
         int comres;
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR ;
      }
   else
      {
      comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_UNLOCK_EE_PROG_MODE__REG,&data, C_BMA2x2_One_U8X);
      data = BMA2x2_SET_BITSLICE(data, BMA2x2_UNLOCK_EE_PROG_MODE, prgmode);
      comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_UNLOCK_EE_PROG_MODE__REG, &data, C_BMA2x2_One_U8X);
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to set the status of nvm program trig
 *
 *
 *
 *
 *  \param unsigned char trig
 *            1 -> trig program seq (wo)
 *            0 -> No Action
 *
 *
 *
 *  \return communication results
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma2x2_set_nvprog_trig(unsigned char trig)
   {
   unsigned char data;
         int comres;
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR ;
      }
   else
      {
      comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_START_EE_PROG_TRIG__REG,&data, C_BMA2x2_One_U8X);
      data = BMA2x2_SET_BITSLICE(data, BMA2x2_START_EE_PROG_TRIG, trig);
      comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_START_EE_PROG_TRIG__REG, &data, C_BMA2x2_One_U8X);
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to get the status of nvmprogram ready
 *
 *
 *
 *
 *  \param unsigned char *ready
 *             1 -> program seq finished
 *             0 -> program seq in progress
 *
 *
 *
 *  \return
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma2x2_get_nvmprog_ready(unsigned char *ready)
   {
   int comres;
   unsigned char data;
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres =E_SMB_NULL_PTR ;
      }
   else
      {
      comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_EE_PROG_READY__REG,&data, C_BMA2x2_One_U8X);
      *ready = BMA2x2_GET_BITSLICE(data, BMA2x2_EE_PROG_READY);
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to get the status of nvm program remain
 *
 *
 *
 *
 *  \param unsigned char *remain
 *
 *
 *
 *  \return
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma2x2_get_nvmprog_remain(unsigned char *remain)
   {
   int comres;
   unsigned char data;
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres =E_SMB_NULL_PTR ;
      }
   else
      {
      comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_EE_REMAIN__REG,&data, C_BMA2x2_One_U8X);
      *remain = BMA2x2_GET_BITSLICE(data, BMA2x2_EE_REMAIN);
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to get the status of spi3
 *
 *
 *
 *
 *  \param  unsigned char *spi3 : Address of spi3
 *              0 -> spi3
 *              1 -> spi4(default)
 *
 *
 *
 *  \return
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_spi3 (unsigned char *spi3)
   {
   unsigned char data;
   int comres = C_BMA2x2_Zero_U8X;
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_EN_SPI_MODE_3__REG,&data,C_BMA2x2_One_U8X);
      *spi3 = BMA2x2_GET_BITSLICE(data,BMA2x2_EN_SPI_MODE_3);
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to set the status of spi3
 *
 *
 *
 *
 *  \param unsigned char spi3
 *        0 -> spi3
 *        1 -> spi4(default)
 *
 *
 *  \return communication results
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_spi3 (unsigned char spi3)
   {
   unsigned char data;
   int comres = C_BMA2x2_Zero_U8X;
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_EN_SPI_MODE_3__REG, &data, C_BMA2x2_One_U8X);
      data = BMA2x2_SET_BITSLICE(data,BMA2x2_EN_SPI_MODE_3, spi3);
      comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_EN_SPI_MODE_3__REG, &data, C_BMA2x2_One_U8X);
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to get the status of i2c wdt
 *
 *
 *
 *
 *  \param unsigned char channel,unsigned char *prog_mode
 *            BMA2x2_I2C_SELECT               0
 *            BMA2x2_I2C_EN                   1
 *
 *           sel,en - x,0 ->OFF
 *                    0,1 ->1 ms
 *                    1,1 ->50ms
 *
 *
 *
 *  \return
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_i2c_wdt (unsigned char channel,unsigned char *prog_mode)
   {
   unsigned char data;
   int comres = C_BMA2x2_Zero_U8X;
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      switch (channel)
         {
         case BMA2x2_I2C_SELECT:
            comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_I2C_WATCHDOG_PERIOD__REG,&data,C_BMA2x2_One_U8X);
            *prog_mode = BMA2x2_GET_BITSLICE(data,BMA2x2_I2C_WATCHDOG_PERIOD);
            break;
         case BMA2x2_I2C_EN:
            comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_EN_I2C_WATCHDOG__REG,&data,C_BMA2x2_One_U8X);
            *prog_mode = BMA2x2_GET_BITSLICE(data,BMA2x2_EN_I2C_WATCHDOG);
            break;
         default:
            comres = E_OUT_OF_RANGE ;
            break;
         }
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to set the status of i2c wdt
 *
 *
 *
 *
 *  \param unsigned char channel,unsigned char prog_mode
 *               BMA2x2_I2C_SELECT               0
 *               BMA2x2_I2C_EN                   1
 *
 *           sel,en - x,0 ->OFF
 *                    0,1 ->1 ms
 *                    1,1 ->50ms
 *
 *
 *  \return communication results
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_i2c_wdt (unsigned char channel,unsigned char prog_mode)
   {
   unsigned char data;
   int comres = C_BMA2x2_Zero_U8X;
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      switch (channel)
         {
         case BMA2x2_I2C_SELECT:
            comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_I2C_WATCHDOG_PERIOD__REG, &data, C_BMA2x2_One_U8X);
            data = BMA2x2_SET_BITSLICE(data, BMA2x2_I2C_WATCHDOG_PERIOD, prog_mode);
            comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_I2C_WATCHDOG_PERIOD__REG, &data, C_BMA2x2_One_U8X);
            break;
         case BMA2x2_I2C_EN:
            comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_EN_I2C_WATCHDOG__REG, &data, C_BMA2x2_One_U8X);
            data = BMA2x2_SET_BITSLICE(data, BMA2x2_EN_I2C_WATCHDOG, prog_mode);
            comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_EN_I2C_WATCHDOG__REG, &data, C_BMA2x2_One_U8X);
            break;
         default:
            comres = E_OUT_OF_RANGE ;
            break;
         }
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to get the status slow compensation
 *
 *
 *
 *
 *  \param unsigned char channel,unsigned char *slow_comp
 *                     BMA2x2_SLOW_COMP_X              0
 *                     BMA2x2_SLOW_COMP_Y              1
 *                     BMA2x2_SLOW_COMP_Z              2
 *
 *
 *            slow_comp : 1 -> enable
 *                        0 -> disable slow offset
 *
 *
 *  \return
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_slow_comp(unsigned char channel,unsigned char *slow_comp)
   {
   unsigned char data;
   int comres = C_BMA2x2_Zero_U8X;
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      switch (channel)
         {
         case BMA2x2_SLOW_COMP_X:
            //SLOW COMP X
            comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_EN_SLOW_COMP_X__REG,&data,C_BMA2x2_One_U8X);
            *slow_comp = BMA2x2_GET_BITSLICE(data,BMA2x2_EN_SLOW_COMP_X);
            break;
         case BMA2x2_SLOW_COMP_Y:
            //SLOW COMP Y
            comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_EN_SLOW_COMP_Y__REG,&data,C_BMA2x2_One_U8X);
            *slow_comp = BMA2x2_GET_BITSLICE(data,BMA2x2_EN_SLOW_COMP_Y);
            break;
         case BMA2x2_SLOW_COMP_Z:
            //SLOW COMP Z
            comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_EN_SLOW_COMP_Z__REG,&data,C_BMA2x2_One_U8X);
            *slow_comp = BMA2x2_GET_BITSLICE(data,BMA2x2_EN_SLOW_COMP_Z);
            break;
         default:
            comres = E_BMA2x2_OUT_OF_RANGE;
            break;
         }
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to set the status slow compensation
 *
 *
 *
 *
 *  \param unsigned char channel,unsigned char slow_comp
 *          BMA2x2_SLOW_COMP_X              0
 *          BMA2x2_SLOW_COMP_Y              1
 *          BMA2x2_SLOW_COMP_Z              2
 *
 *
 *            slow_comp : 1 -> enable
 *                        0 -> disable slow offset
 *
 *  \return communication results
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_slow_comp (unsigned char channel,unsigned char slow_comp)
   {
   unsigned char data;
   int comres = C_BMA2x2_Zero_U8X;
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      switch (channel)
         {
         case BMA2x2_SLOW_COMP_X:
            //SLOW COMP X
            comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_EN_SLOW_COMP_X__REG, &data, C_BMA2x2_One_U8X);
            data = BMA2x2_SET_BITSLICE(data, BMA2x2_EN_SLOW_COMP_X, slow_comp);
            comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_EN_SLOW_COMP_X__REG, &data, C_BMA2x2_One_U8X);
            break;
         case BMA2x2_SLOW_COMP_Y:
            //SLOW COMP Y
            comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_EN_SLOW_COMP_Y__REG, &data, C_BMA2x2_One_U8X);
            data = BMA2x2_SET_BITSLICE(data, BMA2x2_EN_SLOW_COMP_Y, slow_comp);
            comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_EN_SLOW_COMP_Y__REG, &data, C_BMA2x2_One_U8X);
            break;
         case BMA2x2_SLOW_COMP_Z:
            //SLOW COMP Z
            comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_EN_SLOW_COMP_Z__REG, &data, C_BMA2x2_One_U8X);
            data = BMA2x2_SET_BITSLICE(data, BMA2x2_EN_SLOW_COMP_Z, slow_comp);
            comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_EN_SLOW_COMP_Z__REG, &data, C_BMA2x2_One_U8X);
            break;
         default:
            comres = E_BMA2x2_OUT_OF_RANGE;
            break;
         }
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/***************************************************************************************************
 * Description: *//**\brief This API is used to get the status of fast offset compensation(cal rdy)
 *
 *
 *
 *
 *  \param unsigned char *rdy
 *                 Read Only Possible
 *
 *
 *
 *  \return
 *
 *
 ****************************************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_cal_rdy (unsigned char *rdy)
   {
   unsigned char data;
   int comres = C_BMA2x2_Zero_U8X;
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_FAST_CAL_RDY_S__REG,&data,C_BMA2x2_One_U8X);
      *rdy = BMA2x2_GET_BITSLICE(data,BMA2x2_FAST_CAL_RDY_S);
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to set the status of cal rdy
 *
 *
 *
 *
 *  \param unsigned char rdy
 *
 *
 *
 *  \return communication results
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_cal_rdy (unsigned char rdy)
   {
   unsigned char data;
   int comres = C_BMA2x2_Zero_U8X;
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_FAST_CAL_RDY_S__REG, &data, C_BMA2x2_One_U8X);
      data = BMA2x2_SET_BITSLICE(data,BMA2x2_FAST_CAL_RDY_S, rdy);
      comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_FAST_CAL_RDY_S__REG, &data, C_BMA2x2_One_U8X);
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to get the status of cal trig
 *
 *
 *
 *
 *  \param unsigned char *cal_trig
 *
 *
 *
 *  \return
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_cal_trig (unsigned char *cal_trig)
   {
   unsigned char data;
   int comres = C_BMA2x2_Zero_U8X;
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_CAL_TRIGGER__REG,&data,C_BMA2x2_One_U8X);
      *cal_trig = BMA2x2_GET_BITSLICE(data,BMA2x2_CAL_TRIGGER);
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*****************************************************************************************************
 * Description: *//**\brief This API is used to set the status of fast offset calculation(cal trig)
 *
 *
 *
 *
 *  \param unsigned char cal_trig
 *                  Write only possible
 *
 *
 *
 *  \return communication results
 *
 *
 *****************************************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_cal_trig (unsigned char cal_trig)
   {
   unsigned char data;
   int comres = C_BMA2x2_Zero_U8X;
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_CAL_TRIGGER__REG, &data, C_BMA2x2_One_U8X);
      data = BMA2x2_SET_BITSLICE(data,BMA2x2_CAL_TRIGGER, cal_trig);
      comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_CAL_TRIGGER__REG, &data, C_BMA2x2_One_U8X);
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to set the status of offset reset
 *
 *
 *
 *
 *  \param unsigned char offset_reset
 *
 *
 *
 *  \return communication results
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_offset_reset (unsigned char offset_reset)
   {
   unsigned char data;
   int comres = C_BMA2x2_Zero_U8X;
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_RESET_OFFSET_REGS__REG, &data, C_BMA2x2_One_U8X);
      data = BMA2x2_SET_BITSLICE(data,BMA2x2_RESET_OFFSET_REGS, offset_reset);
      comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_RESET_OFFSET_REGS__REG, &data, C_BMA2x2_One_U8X);
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to get the status of offset reset
 *
 *
 *
 *
 *  \param unsigned char channel,unsigned char *offset
 *                     Channel ->
 *               BMA2x2_CUT_OFF              ->    0
 *               BMA2x2_OFFSET_TRIGGER_X     ->    1
 *               BMA2x2_OFFSET_TRIGGER_Y     ->    2
 *               BMA2x2_OFFSET_TRIGGER_Z     ->    3
 *               offset ->
 *               CUT_OFF -> 0 or 1
 *               BMA2x2_OFFSET_TRIGGER_X   -> 0,1,2,3
 *               BMA2x2_OFFSET_TRIGGER_Y   -> 0,1,2,3
 *               BMA2x2_OFFSET_TRIGGER_Z   -> 0,1,2,3
 *
 *
 *
 *  \return
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_offset_target(unsigned char channel,unsigned char *offset)
   {
   unsigned char data;
   int comres = C_BMA2x2_Zero_U8X;
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      switch (channel)
         {
         case BMA2x2_CUT_OFF:
            //CUT-OFF
            comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_COMP_CUTOFF__REG,&data,C_BMA2x2_One_U8X);
            *offset = BMA2x2_GET_BITSLICE(data,BMA2x2_COMP_CUTOFF);
            break;
         case BMA2x2_OFFSET_TRIGGER_X:
            //OFFSET TRIGGER X
            comres =p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_COMP_TARGET_OFFSET_X__REG,&data,C_BMA2x2_One_U8X);
            *offset = BMA2x2_GET_BITSLICE(data,BMA2x2_COMP_TARGET_OFFSET_X);
            break;
         case BMA2x2_OFFSET_TRIGGER_Y:
            //OFFSET TRIGGER Y
            comres =p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_COMP_TARGET_OFFSET_Y__REG,&data,C_BMA2x2_One_U8X);
            *offset = BMA2x2_GET_BITSLICE(data,BMA2x2_COMP_TARGET_OFFSET_Y);
            break;
         case BMA2x2_OFFSET_TRIGGER_Z:
            //OFFSET TRIGGER Z
            comres =p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_COMP_TARGET_OFFSET_Z__REG,&data,C_BMA2x2_One_U8X);
            *offset = BMA2x2_GET_BITSLICE(data,BMA2x2_COMP_TARGET_OFFSET_Z);
            break;
         default:
            comres = E_BMA2x2_OUT_OF_RANGE;
            break;
         }
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to set the status of offset reset
 *
 *
 *
 *
 *  \param unsigned char channel,unsigned char offset
 *               Channel ->
 *               BMA2x2_CUT_OFF              ->    0
 *               BMA2x2_OFFSET_TRIGGER_X     ->    1
 *               BMA2x2_OFFSET_TRIGGER_Y     ->    2
 *               BMA2x2_OFFSET_TRIGGER_Z     ->    3
 *               offset ->
 *               CUT_OFF -> 0 or 1
 *               BMA2x2_OFFSET_TRIGGER_X   -> 0,1,2,3
 *               BMA2x2_OFFSET_TRIGGER_Y   -> 0,1,2,3
 *               BMA2x2_OFFSET_TRIGGER_Z   -> 0,1,2,3
 *
 *  \return communication results
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_offset_target (unsigned char channel,unsigned char offset)
   {
   unsigned char data;
   int comres = C_BMA2x2_Zero_U8X;
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      switch (channel)
         {
         case BMA2x2_CUT_OFF:
            //CUT-OFF
            comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_COMP_CUTOFF__REG, &data, C_BMA2x2_One_U8X);
            data = BMA2x2_SET_BITSLICE(data, BMA2x2_COMP_CUTOFF, offset);
            comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_COMP_CUTOFF__REG, &data, C_BMA2x2_One_U8X);
            break;
         case BMA2x2_OFFSET_TRIGGER_X:
            //OFFSET TARGET X
            comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_COMP_TARGET_OFFSET_X__REG, &data, C_BMA2x2_One_U8X);
            data = BMA2x2_SET_BITSLICE(data, BMA2x2_COMP_TARGET_OFFSET_X, offset);
            comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_COMP_TARGET_OFFSET_X__REG, &data, C_BMA2x2_One_U8X);
            break;
         case BMA2x2_OFFSET_TRIGGER_Y:
            //OFFSET TARGET Y
            comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_COMP_TARGET_OFFSET_Y__REG, &data, C_BMA2x2_One_U8X);
            data = BMA2x2_SET_BITSLICE(data, BMA2x2_COMP_TARGET_OFFSET_Y, offset);
            comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_COMP_TARGET_OFFSET_Y__REG, &data, C_BMA2x2_One_U8X);
            break;
         case BMA2x2_OFFSET_TRIGGER_Z:
            //OFFSET TARGET Z
            comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_COMP_TARGET_OFFSET_Z__REG, &data, C_BMA2x2_One_U8X);
            data = BMA2x2_SET_BITSLICE(data, BMA2x2_COMP_TARGET_OFFSET_Z, offset);
            comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_COMP_TARGET_OFFSET_Z__REG, &data, C_BMA2x2_One_U8X);
            break;
         default:
            comres = E_BMA2x2_OUT_OF_RANGE;
            break;
         }
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to get the status of offset
 *
 *
 *
 *
 *  \param unsigned char channel,unsigned char *offset
 *                         Channel ->
 *                   BMA2x2_X_AXIS     ->      0
 *                   BMA2x2_Y_AXIS     ->      1
 *                   BMA2x2_Z_AXIS     ->      2
 *                   offset -> Any valid value
 *
 *
 *
 *  \return
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_offset(unsigned char channel,unsigned char *offset)
   {
   unsigned char data;
   int comres = C_BMA2x2_Zero_U8X;
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      switch (channel)
         {
         case BMA2x2_X_AXIS:
            comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_OFFSET_X_AXIS_REG,&data,C_BMA2x2_One_U8X);
            *offset = data;
            break;
         case BMA2x2_Y_AXIS:
            comres =p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_OFFSET_Y_AXIS_REG,&data,C_BMA2x2_One_U8X);
            *offset = data;
            break;
         case BMA2x2_Z_AXIS:
            comres =p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_OFFSET_Z_AXIS_REG,&data,C_BMA2x2_One_U8X);
            *offset = data;
            break;
         default:
            comres = E_BMA2x2_OUT_OF_RANGE;
            break;
         }
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to set the status of offset
 *
 *
 *
 *
 *  \param unsigned char channel,unsigned char offset
 *                   Channel ->
 *                   BMA2x2_X_AXIS     ->      0
 *                   BMA2x2_Y_AXIS     ->      1
 *                   BMA2x2_Z_AXIS     ->      2
 *                   offset -> Any valid value
 *
 *
 *
 *  \return communication results
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_offset (unsigned char channel,unsigned char offset)
   {
   unsigned char data;
   int comres = C_BMA2x2_Zero_U8X;
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      switch (channel)
         {
         case BMA2x2_X_AXIS:
            data = offset;
            comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_OFFSET_X_AXIS_REG, &data, C_BMA2x2_One_U8X);
            break;
         case BMA2x2_Y_AXIS:
            data = offset;
            comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_OFFSET_Y_AXIS_REG, &data,C_BMA2x2_One_U8X);
            break;
         case BMA2x2_Z_AXIS:
            data = offset;
            comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_OFFSET_Z_AXIS_REG, &data,C_BMA2x2_One_U8X);
            break;
         default:
            comres = E_BMA2x2_OUT_OF_RANGE;
            break;
         }
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/********************************************************************************************
 * Description: *//**\brief This API is used to get the status of general purpose register
 *
 *
 *
 *
 *  \param unsigned char channel,unsigned char *gp
 *             channel ->
 *              BMA2x2_GP0                      0
 *              BMA2x2_GP1                      1
 *               gp -> Any valid value
 *
 *
 *
 *  \return
 *
 *
 *******************************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_gp(unsigned char channel,unsigned char *gp)
   {
   unsigned char data;
   int comres = C_BMA2x2_Zero_U8X;
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      switch (channel)
         {
         case BMA2x2_GP0:
            //GP0
            comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_GP0_REG,&data,C_BMA2x2_One_U8X);
            *gp = data;
            break;
         case BMA2x2_GP1:
            //GP1
            comres =p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_GP1_REG,&data,C_BMA2x2_One_U8X);
            *gp = data;
            break;
         default:
            comres = E_BMA2x2_OUT_OF_RANGE;
            break;
         }
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*********************************************************************************************
 * Description: *//**\brief This API is used to set the status of general purpose register
 *
 *
 *
 *
 *  \param unsigned char channel,unsigned char gp
 *              channel ->
 *              BMA2x2_GP0                      0
 *              BMA2x2_GP1                      1
 *               gp -> Any valid value
 *
 *
 *
 *  \return communication results
 *
 *
 *********************************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_gp (unsigned char channel,unsigned char gp)
   {
   unsigned char data;
   int comres = C_BMA2x2_Zero_U8X;
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      switch (channel)
         {
         case BMA2x2_GP0:
            //GP0
            data = gp;
            comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_GP0_REG, &data, C_BMA2x2_One_U8X);
            break;
         case BMA2x2_GP1:
            //GP1
            data = gp;
            comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_GP1_REG, &data,C_BMA2x2_One_U8X);
            break;
         default:
            comres = E_BMA2x2_OUT_OF_RANGE;
            break;
         }
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to get the status of fifo mode
 *
 *
 *
 *
 *  \param unsigned char *fifo_mode : Address of fifo_mode
 *                         fifo_mode  0 --> Bypass
 *                         1 --> FIFO
 *                         2 --> Stream
 *                         3 --> Reserved
 *
 *
 *
 *  \return
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_fifo_mode (unsigned char *fifo_mode)
   {
   int comres;
   unsigned char data;
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres =E_SMB_NULL_PTR ;
      }
   else
      {
      comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_FIFO_MODE__REG,&data, C_BMA2x2_One_U8X);
      *fifo_mode = BMA2x2_GET_BITSLICE(data, BMA2x2_FIFO_MODE);
      }
   return comres;
   }
/*******************************************************************************
 * Description: *//**\brief This API is used set to FIFO mode
 *
 *
 *
 *
 *  \param unsigned char fifo_mode
 *
 *
 *              fifo_mode  0 --> Bypass
 *                         1 --> FIFO
 *                         2 --> Stream
 *                         3 --> Reserved
 *
 *
 *
 *
 *  \return Communication Results
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_fifo_mode (unsigned char fifo_mode)
   {
   unsigned char data;
   int comres = C_BMA2x2_Zero_U8X;
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      if (fifo_mode < C_BMA2x2_Four_U8X)
         {
         comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_FIFO_MODE__REG, &data, C_BMA2x2_One_U8X);
         data = BMA2x2_SET_BITSLICE(data,BMA2x2_FIFO_MODE, fifo_mode);
         comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_FIFO_MODE__REG, &data, C_BMA2x2_One_U8X);
         }
      else
         {
         comres = E_OUT_OF_RANGE ;
         }
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to get the status of fifo data sel
 *
 *
 *
 *
 *  \param unsigned char *data_sel : Address of data_sel
 *         data_sel --> [0:3]
 *         0 --> X,Y and Z (DEFAULT)
 *         1 --> X only
 *         2 --> Y only
 *         3 --> Z only
 *
 *
 *
 *  \return
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_fifo_data_sel (unsigned char *data_sel)
   {
   int comres;
   unsigned char data;
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres =E_SMB_NULL_PTR ;
      }
   else
      {
      comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_FIFO_DATA_SELECT__REG,&data, C_BMA2x2_One_U8X);
      *data_sel = BMA2x2_GET_BITSLICE(data, BMA2x2_FIFO_DATA_SELECT);
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to set the status of fifo data sel
 *
 *
 *
 *
 *  \param unsigned char data_sel
 *         data_sel --> [0:3]
 *         0 --> X,Y and Z (DEFAULT)
 *         1 --> X only
 *         2 --> Y only
 *         3 --> Z only
 *
 *
 *
 *  \return communication results
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_fifo_data_sel (unsigned char data_sel)
   {
   unsigned char data;
   int comres = C_BMA2x2_Zero_U8X;
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      if (data_sel < C_BMA2x2_Four_U8X)
         {
         comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_FIFO_DATA_SELECT__REG, &data, C_BMA2x2_One_U8X);
         data = BMA2x2_SET_BITSLICE(data,BMA2x2_FIFO_DATA_SELECT, data_sel);
         comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_FIFO_DATA_SELECT__REG, &data, C_BMA2x2_One_U8X);
         }
      else
         {
         comres = E_OUT_OF_RANGE ;
         }
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to get the status of fifo data out reg
 *
 *
 *
 *
 *  \param  unsigned char *out_reg : Address of out_reg
 *                         Any Valid Value
 *
 *
 *
 *  \return
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_fifo_data_out_reg(unsigned char *out_reg)
   {
   unsigned char data;
   int comres = C_BMA2x2_Zero_U8X;
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      //GET FIFO DATA OUTPUT REGISTER
      comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_FIFO_DATA_OUTPUT_REG,&data,C_BMA2x2_One_U8X);
      *out_reg = data;
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to set the status of fifo data out reg
 *
 *
 *
 *
 *  \param unsigned char out_reg
 *                 Any Valid Value
 *
 *
 *
 *  \return communication results
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_fifo_data_out_reg (unsigned char out_reg)
   {
   unsigned char data;
   int comres = C_BMA2x2_Zero_U8X;
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      //SET FIFO DATA REGISTER
      data = out_reg;
      comres = p_bma2x2->BMA2x2_BUS_WRITE_FUNC(p_bma2x2->dev_addr, BMA2x2_FIFO_DATA_OUTPUT_REG, &data, C_BMA2x2_One_U8X);
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API is used to read the temperature
 *
 *
 *
 *
 *  \param signed char temperature
 *                 Pointer to temperature
 *
 *
 *
 *  \return communication results
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_read_temperature (signed char *temperature)
   {
   unsigned char data;
   int comres = C_BMA2x2_Zero_U8X;
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
	  comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr,BMA2x2_TEMPERATURE_REG,&data,C_BMA2x2_One_U8X);
      *temperature = (signed char)data;
      }
   return comres;
   }
/* Compiler Switch if applicable
#ifdef

#endif
*/
/*******************************************************************************
 * Description: *//**\brief This API reads acceleration data X,Y,Z values and
 *                          Temperature data from location 02h to 08h
 *
 *
 *
 *
 *  \param bma2x2acc_data * acc : Address of bma2x2acc_data
 *
 *
 *
 *  \return result of communication routines
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
int bma2x2_read_accel_xyzt(bma2x2acc_data * acc)
   {
   int comres = 0;
   unsigned char data[7];
   if (p_bma2x2==C_BMA2x2_Zero_U8X)
      {
      comres = E_SMB_NULL_PTR;
      }
   else
      {
      switch (V_BMA2x2RESOLUTION_U8R)
         {
         case BMA2x2_12_RESOLUTION:
            comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_ACC_X12_LSB__REG, data, 7);
            acc->x = BMA2x2_GET_BITSLICE(data[0],BMA2x2_ACC_X12_LSB)| (BMA2x2_GET_BITSLICE(data[1],BMA2x2_ACC_X_MSB)<<(BMA2x2_ACC_X12_LSB__LEN));
            acc->x = acc->x << (sizeof(short)*8-(BMA2x2_ACC_X12_LSB__LEN + BMA2x2_ACC_X_MSB__LEN));
            acc->x = acc->x >> (sizeof(short)*8-(BMA2x2_ACC_X12_LSB__LEN + BMA2x2_ACC_X_MSB__LEN));

            acc->y = BMA2x2_GET_BITSLICE(data[2],BMA2x2_ACC_Y12_LSB)| (BMA2x2_GET_BITSLICE(data[3],BMA2x2_ACC_Y_MSB)<<(BMA2x2_ACC_Y12_LSB__LEN ));
            acc->y = acc->y << (sizeof(short)*8-(BMA2x2_ACC_Y12_LSB__LEN + BMA2x2_ACC_Y_MSB__LEN));
            acc->y = acc->y >> (sizeof(short)*8-(BMA2x2_ACC_Y12_LSB__LEN + BMA2x2_ACC_Y_MSB__LEN));

            acc->z = BMA2x2_GET_BITSLICE(data[4],BMA2x2_ACC_Z12_LSB)| (BMA2x2_GET_BITSLICE(data[5],BMA2x2_ACC_Z_MSB)<<(BMA2x2_ACC_Z12_LSB__LEN));
            acc->z = acc->z << (sizeof(short)*8-(BMA2x2_ACC_Z12_LSB__LEN + BMA2x2_ACC_Z_MSB__LEN));
            acc->z = acc->z >> (sizeof(short)*8-(BMA2x2_ACC_Z12_LSB__LEN + BMA2x2_ACC_Z_MSB__LEN));
			acc->temperature = (signed char)data[6];
            break;
         case BMA2x2_10_RESOLUTION:
            comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_ACC_X10_LSB__REG, data, 7);
            acc->x = BMA2x2_GET_BITSLICE(data[0],BMA2x2_ACC_X10_LSB)| (BMA2x2_GET_BITSLICE(data[1],BMA2x2_ACC_X_MSB)<<(BMA2x2_ACC_X10_LSB__LEN));
            acc->x = acc->x << (sizeof(short)*8-(BMA2x2_ACC_X10_LSB__LEN + BMA2x2_ACC_X_MSB__LEN));
            acc->x = acc->x >> (sizeof(short)*8-(BMA2x2_ACC_X10_LSB__LEN + BMA2x2_ACC_X_MSB__LEN));

            acc->y = BMA2x2_GET_BITSLICE(data[2],BMA2x2_ACC_Y10_LSB)| (BMA2x2_GET_BITSLICE(data[3],BMA2x2_ACC_Y_MSB)<<(BMA2x2_ACC_Y10_LSB__LEN ));
            acc->y = acc->y << (sizeof(short)*8-(BMA2x2_ACC_Y10_LSB__LEN + BMA2x2_ACC_Y_MSB__LEN));
            acc->y = acc->y >> (sizeof(short)*8-(BMA2x2_ACC_Y10_LSB__LEN + BMA2x2_ACC_Y_MSB__LEN));

            acc->z = BMA2x2_GET_BITSLICE(data[4],BMA2x2_ACC_Z10_LSB)| (BMA2x2_GET_BITSLICE(data[5],BMA2x2_ACC_Z_MSB)<<(BMA2x2_ACC_Z10_LSB__LEN));
            acc->z = acc->z << (sizeof(short)*8-(BMA2x2_ACC_Z10_LSB__LEN + BMA2x2_ACC_Z_MSB__LEN));
            acc->z = acc->z >> (sizeof(short)*8-(BMA2x2_ACC_Z10_LSB__LEN + BMA2x2_ACC_Z_MSB__LEN));
			acc->temperature = (signed char)data[6];
            break;
         case BMA2x2_8_RESOLUTION:
            comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_ACC_X8_LSB__REG, data, 7);
            acc->x = BMA2x2_GET_BITSLICE(data[0],BMA2x2_ACC_X8_LSB)| (BMA2x2_GET_BITSLICE(data[1],BMA2x2_ACC_X_MSB)<<(BMA2x2_ACC_X8_LSB__LEN));
            acc->x = acc->x << (sizeof(short)*8-(BMA2x2_ACC_X8_LSB__LEN + BMA2x2_ACC_X_MSB__LEN));
            acc->x = acc->x >> (sizeof(short)*8-(BMA2x2_ACC_X8_LSB__LEN + BMA2x2_ACC_X_MSB__LEN));

            acc->y = BMA2x2_GET_BITSLICE(data[2],BMA2x2_ACC_Y8_LSB)| (BMA2x2_GET_BITSLICE(data[3],BMA2x2_ACC_Y_MSB)<<(BMA2x2_ACC_Y8_LSB__LEN ));
            acc->y = acc->y << (sizeof(short)*8-(BMA2x2_ACC_Y8_LSB__LEN + BMA2x2_ACC_Y_MSB__LEN));
            acc->y = acc->y >> (sizeof(short)*8-(BMA2x2_ACC_Y8_LSB__LEN + BMA2x2_ACC_Y_MSB__LEN));

            acc->z = BMA2x2_GET_BITSLICE(data[4],BMA2x2_ACC_Z8_LSB)| (BMA2x2_GET_BITSLICE(data[5],BMA2x2_ACC_Z_MSB)<<(BMA2x2_ACC_Z8_LSB__LEN));
            acc->z = acc->z << (sizeof(short)*8-(BMA2x2_ACC_Z8_LSB__LEN + BMA2x2_ACC_Z_MSB__LEN));
            acc->z = acc->z >> (sizeof(short)*8-(BMA2x2_ACC_Z8_LSB__LEN + BMA2x2_ACC_Z_MSB__LEN));
			acc->temperature = (signed char)data[6];
            break;
         case BMA2x2_14_RESOLUTION:
            comres = p_bma2x2->BMA2x2_BUS_READ_FUNC(p_bma2x2->dev_addr, BMA2x2_ACC_X14_LSB__REG, data, 7);
            acc->x = BMA2x2_GET_BITSLICE(data[0],BMA2x2_ACC_X14_LSB)| (BMA2x2_GET_BITSLICE(data[1],BMA2x2_ACC_X_MSB)<<(BMA2x2_ACC_X14_LSB__LEN));
            acc->x = acc->x << (sizeof(short)*8-(BMA2x2_ACC_X14_LSB__LEN + BMA2x2_ACC_X_MSB__LEN));
            acc->x = acc->x >> (sizeof(short)*8-(BMA2x2_ACC_X14_LSB__LEN + BMA2x2_ACC_X_MSB__LEN));

            acc->y = BMA2x2_GET_BITSLICE(data[2],BMA2x2_ACC_Y14_LSB)| (BMA2x2_GET_BITSLICE(data[3],BMA2x2_ACC_Y_MSB)<<(BMA2x2_ACC_Y14_LSB__LEN ));
            acc->y = acc->y << (sizeof(short)*8-(BMA2x2_ACC_Y14_LSB__LEN + BMA2x2_ACC_Y_MSB__LEN));
            acc->y = acc->y >> (sizeof(short)*8-(BMA2x2_ACC_Y14_LSB__LEN + BMA2x2_ACC_Y_MSB__LEN));

            acc->z = BMA2x2_GET_BITSLICE(data[4],BMA2x2_ACC_Z14_LSB)| (BMA2x2_GET_BITSLICE(data[5],BMA2x2_ACC_Z_MSB)<<(BMA2x2_ACC_Z14_LSB__LEN));
            acc->z = acc->z << (sizeof(short)*8-(BMA2x2_ACC_Z14_LSB__LEN + BMA2x2_ACC_Z_MSB__LEN));
            acc->z = acc->z >> (sizeof(short)*8-(BMA2x2_ACC_Z14_LSB__LEN + BMA2x2_ACC_Z_MSB__LEN));
			acc->temperature = (signed char)data[6];
            break;
         default:
            break;
         }
      }
   return comres;
   }
