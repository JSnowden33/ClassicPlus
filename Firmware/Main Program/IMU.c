/*
 * File:   IMU.c
 * Author: Jackson Snowden
 */

#include <xc.h>
#include "pin_defs.h"
#include "config.h"
#include "MSSP.h"
#include "IMU.h"

u8 IMUen;

u8 IMUinit(u8 aScale, u8 gScale)
{
    if (!SPIisEnabled()) 
    {
        SPImasterInit(IMU_SPI_CLK, 0, 1, 0);
    }
    
    // Give IMU time to initialize
    __delay_ms(50);
    
    /* Disable IMU and return if device is not recognized */
    if (IMUreadReg(ID_REG) != IMU_ID)
    {
        IMUoff();
        return 1;
    }
    
    IMUwriteReg(INT1_CTRL, 0x03);                               // Accel Data Ready and Gyro Data Ready on INT1
    
    if (aScale == XL_OFF) IMUwriteReg(CTRL1_XL, 0x00);          // Accel off
    else IMUwriteReg(CTRL1_XL, 0x80 | (aScale & 0x03));         // Accel 1.66 kHz data rate
    
    if (gScale == G_OFF) IMUwriteReg(CTRL2_G, 0x00);            // Gyro off
    else IMUwriteReg(CTRL2_G, 0x80 | ((gScale & 0x07) << 1));   // Gyro 1.66 kHz data rate
    
    IMUwriteReg(CTRL3_C, 0x04);                                 // Continuous update, interrupt active high, 4-wire SPI
    
    IMUen = 1;
    return 0;
}

void IMUoff()
{
    SPIoff();
    IMUen = 0;
}

u8 IMUisEnabled()
{
    return IMUen;
}

void IMUwriteReg(u8 reg, u8 data)
{
    CS = 0;
    SPItransfer(SPI_WRITE | (IMU_REG_MASK & reg));
    SPItransfer(data);
    CS = 1;
}

void IMUwriteRegMulti(u8 reg, u8 *data, u8 len)
{
    u8 i;

    CS = 0;
    SPItransfer(SPI_WRITE | (IMU_REG_MASK & reg));
    for (i = 0; i < len; i++) SPItransfer(data[i]);
    CS = 1;
}

u8 IMUreadReg(u8 reg)
{
    u8 result;
    
    CS = 0;
    SPItransfer(SPI_READ | (IMU_REG_MASK & reg));
    result = SPItransfer(SPI_NOP);
    CS = 1;
    
    return result;
}

void IMUreadRegMulti(u8 reg, u8 *data, u8 len)
{
    u8 i;
    
    CS = 0;
    SPItransfer(SPI_READ | (IMU_REG_MASK & reg));
    for (i = 0; i < len; i++) data[i] = SPItransfer(SPI_NOP);
    CS = 1;
}

s16 IMUreadAccelX()
{
    u8 data[2];
    IMUreadRegMulti(OUTX_L_XL, data, 2);
    return data[0] | (data[1] << 8);
}

s16 IMUreadAccelY()
{
    u8 data[2];
    IMUreadRegMulti(OUTY_L_XL, data, 2);
    return data[0] | (data[1] << 8);
}

s16 IMUreadAccelZ()
{
    u8 data[2];
    IMUreadRegMulti(OUTZ_L_XL, data, 2);
    return data[0] | (data[1] << 8);
}

s16 IMUreadGyroX()
{
    u8 data[2];
    IMUreadRegMulti(OUTX_L_G, data, 2);
    return data[0] | (data[1] << 8);
}

s16 IMUreadGyroY()
{
    u8 data[2];
    IMUreadRegMulti(OUTY_L_G, data, 2);
    return data[0] | (data[1] << 8);
}

s16 IMUreadGyroZ()
{
    u8 data[2];
    IMUreadRegMulti(OUTZ_L_G, data, 2);
    return data[0] | (data[1] << 8);
}