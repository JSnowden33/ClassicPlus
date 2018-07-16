/*
 * File:   expansion.c
 * Author: Jackson Snowden
 */

#include <xc.h>
#include "config.h"
#include "pin_defs.h"
#include "MSSP.h"
#include "crypto.h"
#include "input.h"
#include "NVM.h"
#include "IMU.h"
#include "camera.h"
#include "expansion.h"

// Extension controller IDs recognized by Wii
const u8 classicID[6] = { 0x00, 0x00, 0xA4, 0x20, 0x01, 0x01 };
const u8 nunchukID[6] = { 0x00, 0x00, 0xA4, 0x20, 0x00, 0x00 };

// Calibration data used by Wii
const u8 calDataClassic[32] =
{
    0xE0, 0x20, 0x80,           // Left Stick X: Max, Min, Center
	0xE0, 0x20, 0x80,           // Left Stick Y: Max, Min, Center
	0xE0, 0x20, 0x80,           // Right Stick X: Max, Min, Center
	0xE0, 0x20, 0x80,           // Right Stick Y: Max, Min, Center
	0x20, 0x20,                 // Left Trigger Min, Right Trigger Min
    0x95, 0xEA,                 // Checksum 0x55, Checksum 0xAA

	0xE0, 0x20, 0x80,           // Left Stick X: Max, Min, Center
	0xE0, 0x20, 0x80,           // Left Stick Y: Max, Min, Center
	0xE0, 0x20, 0x80,           // Right Stick X: Max, Min, Center
	0xE0, 0x20, 0x80,           // Right Stick Y: Max, Min, Center
	0x20, 0x20,                 // Left Trigger Min, Right Trigger Min
    0x95, 0xEA                  // Checksum 0x55, Checksum 0xAA
};
const u8 calDataNunchuk[32] =
{
	0x80, 0x80, 0x80,           // Accel X [9:2], Accel Y [9:2], Accel Z [9:2] (at 0g)
    0x00,                       // Accel X [1:0] + Accel Y [1:0] + Accel Z [1:0] (at 0g)
    0xB4, 0xB4, 0xB4,           // Accel X [9:2], Accel Y [9:2], Accel Z [9:2] (at 1g)
    0x00,                       // Accel X [1:0] + Accel Y [1:0] + Accel Z [1:0] (at 1g)
    0xE0, 0x20, 0x80,           // Joystick X: Max, Min, Center
    0xE0, 0x20, 0x80,           // Joystick Y: Max, Min, Center
    0xF1, 0x46,                 // Checksum 0x55, Checksum 0xAA

	0x80, 0x80, 0x80,           // Accel X [9:2], Accel Y [9:2], Accel Z [9:2] (at 0g)
    0x00,                       // Accel X [1:0] + Accel Y [1:0] + Accel Z [1:0] (at 0g)
    0xB4, 0xB4, 0xB4,           // Accel X [9:2], Accel Y [9:2], Accel Z [9:2] (at 1g)
    0x00,                       // Accel X [1:0] + Accel Y [1:0] + Accel Z [1:0] (at 1g)
    0xE0, 0x20, 0x80,           // Joystick X: Max, Min, Center
    0xE0, 0x20, 0x80,           // Joystick Y: Max, Min, Center
    0xF1, 0x46                  // Checksum 0x55, Checksum 0xAA
};

Calibration cal;
Axis axesMapped;

// Look-up-table for all possible joystick inputs (256 per axis)
u8 LUT[1024];

u8 expMode;     // Expansion controller mode
u8 expEn;       // Expansion controller enable
u8 expCmd;      // Special I2C command buffer

u8 encEn;       // Input/output encryption enable
u8 cfgEn;       // Configuration mode enable
u8 fullModeEn;  // Full reporting mode enable

static u32 Map(u32 x, u32 inMin, u32 inMax, u32 outMin, u32 outMax)
{
    // Map a single value onto a different range
    return (((x - inMin) * (outMax - outMin)) / (inMax - inMin)) + outMin;
}

static void LUTinit()
{   
    u16 pos;
    
    for (pos = 0; pos < 256; pos++)
    {
        // Left Joystick X 
        if (pos < cal.minMax[MIN_LX])
            LUT[pos + LUT_LX] = 0;
        
        else if (pos > cal.minMax[MAX_LX])
            LUT[pos + LUT_LX] = 255;
        
        else LUT[pos + LUT_LX] = Map(pos, cal.minMax[MIN_LX], cal.minMax[MAX_LX], 0, 255);
        
        // Left Joystick Y  
        if (pos < cal.minMax[MIN_LY])
            LUT[pos + LUT_LY] = 0;
        
        else if (pos > cal.minMax[MAX_LY])
            LUT[pos + LUT_LY] = 255;
        
        else LUT[pos + LUT_LY] = Map(pos, cal.minMax[MIN_LY], cal.minMax[MAX_LY], 0, 255);
        
        // Right Joystick X
        if (pos < cal.minMax[MIN_RX])
            LUT[pos + LUT_RX] = 0;
        
        else if (pos > cal.minMax[MAX_RX])
            LUT[pos + LUT_RX] = 255;
        
        else LUT[pos + LUT_RX] = Map(pos, cal.minMax[MIN_RX], cal.minMax[MAX_RX], 0, 255);
        
        // Right Joystick Y
        if (pos < cal.minMax[MIN_RY])
            LUT[pos + LUT_RY] = 0;
        
        else if (pos > cal.minMax[MAX_RY])
            LUT[pos + LUT_RY] = 255;
        
        else LUT[pos + LUT_RY] = Map(pos, cal.minMax[MIN_RY], cal.minMax[MAX_RY], 0, 255);
    }
}

void ExpInit(const u8 *ID)
{
    // Controller disconnect
    DETECT = 0;
    
    // INTOSC = 32 MHz
    OSCCON1bits.NDIV = 0;
    
    // Disable encryption
    encEn = 0;

	// Set controller IDs
    I2CslaveWriteMulti(EXP_REG_ID, (u8*)ID, 6);
    I2CslaveWrite(EXP_REG_CID, CID);

	// Set calibration data
    switch (expMode)
    {
        case MODE_CLASSIC:
            I2CslaveWriteMulti(EXP_REG_CAL, (u8*)calDataClassic, 32);
            break;
        
        case MODE_NUNCHUK:
            I2CslaveWriteMulti(EXP_REG_CAL, (u8*)calDataNunchuk, 32);
            break;
            
        default:
            break;
    }
    
    // Initialize joystick lookup table
    LUTinit();
    
    // Load neutral values into I2C register
    ExpUpdateDefault();
    
    // INTOSC = 8 MHz
    OSCCON1bits.NDIV = 2;
    
    // Controller reconnect
    DETECT = 1;
    
    expCmd = 0;
    cfgEn = 0;
    expEn = 1;
}

void ExpOff()
{
    DETECT = 0;
    expEn = 0;
}

u8 ExpIsEnabled()
{
    return expEn;
}

void ExpSetMode(u8 mode)
{
    expMode = mode;
    
    switch(expMode)
    {
        case MODE_CLASSIC:
            ExpCalLoad();
            I2CslaveInit(EXP_I2C_ADDR, 0);
            IMUoff();
            ExpInit(classicID);
            CamOff();
            ANSELC = 0b01111110;
            break;
        
        case MODE_NUNCHUK:
            ExpCalLoad();
            if (cal.enable[EN_CAM]) I2CslaveInit(EXP_I2C_ADDR, CAM_I2C_ADDR);
            else I2CslaveInit(EXP_I2C_ADDR, 0);
            if (!IMUisEnabled()) IMUinit(XL_SCALE_2G, G_OFF);
            ExpInit(nunchukID);
            CamInit();
            ANSELC = 0b01111110;
            break;
            
        default:
            I2Coff();
            IMUoff();
            ExpOff();
            CamOff();
            ANSELC = 0;
            break;
    }    
}

void ExpCmdRcv(u8 data, u8 addr)
{
    switch (addr)
    {
        // Disable encryption
        case EXP_REG_SETUP2:
            if (data == 0) encEn = 0;
            break;
            
        // Enable full data reporting mode (Classic Controller only)
        case (EXP_REG_ID + 4):
            if (data == 3) 
            {
                if (expMode == MODE_CLASSIC) fullModeEn = 1;
                else I2CslaveWrite(EXP_REG_ID + 4, 0);
            }
            break;
            
        // Generate encryption keys
        case (EXP_REG_KEY + 15):
            expCmd = ENC_EN;
            break;
            
        // Receive custom command
        case EXP_REG_CMD:
            expCmd = data;
            break;
            
        default:
            break;
    }
}

u8 ExpCmdExec()
{
    u8 pgmEn = 0;
    u8 buf[16];
    
    switch (expCmd)
    {
        case PGM_EN:
            expCmd = 0;
            pgmEn = 1;
            break;
                                
        case CFG_EN:
            expCmd = 0;
            cfgEn = 1;
            break;
                                
        case CFG_DIS:
            expCmd = 0;
            cfgEn = 0;
            break;
                                
        case CAL_LOAD:
            expCmd = 0;
            ExpCalLoad();
            break;
                                
        case CAL_STORE:
            expCmd = 0;
            ExpCalStore();
            break;
                                
        case CAL_DEFAULT:
            expCmd = 0;
            ExpCalStoreDefault();
            break;
            
        case ENC_EN:
            expCmd = 0;
            I2CslaveReadMulti(EXP_REG_KEY, buf, 16);
            encEn = InitKeys(buf);
            break;
        
        default:
            break;
    }
    
    return pgmEn;
}

u8 ExpIsEncEnabled()
{
    return encEn;
}

void ExpCalInit(u8 *buf)
{
    cal.minMax[MIN_LX] = buf[0];
    cal.minMax[MIN_LY] = buf[1];
    cal.minMax[MAX_LX] = buf[2];
    cal.minMax[MAX_LY] = buf[3];
    cal.minMax[MIN_RX] = buf[4];
    cal.minMax[MIN_RY] = buf[5];
    cal.minMax[MAX_RX] = buf[6];
    cal.minMax[MAX_RY] = buf[7];
    
    cal.deadzones[DZ_L] = buf[8]; 
    cal.deadzones[DZ_R] = buf[9]; 
    
    cal.invert[INV_JOY_LX] =  buf[10] & 0x01;
    cal.invert[INV_JOY_LY] = (buf[10] & 0x02) >> 1;
    cal.invert[INV_JOY_RX] = (buf[10] & 0x04) >> 2;
    cal.invert[INV_JOY_RY] = (buf[10] & 0x08) >> 3;
    cal.invert[INV_TRIG_L] = (buf[10] & 0x10) >> 4;
    cal.invert[INV_TRIG_R] = (buf[10] & 0x20) >> 5;
    cal.invert[INV_XL_X] =  buf[11] & 0x01;
    cal.invert[INV_XL_Y] = (buf[11] & 0x02) >> 1;
    cal.invert[INV_XL_Z] = (buf[11] & 0x04) >> 2;
    
    cal.enable[EN_JOY_L] =  buf[12] & 0x01;
    cal.enable[EN_JOY_R] = (buf[12] & 0x02) >> 1;
    cal.enable[EN_TRIG] =  (buf[12] & 0x04) >> 2;
    cal.enable[EN_CAM] =   (buf[12] & 0x08) >> 3;
    
    CamSetSensitivity(buf[13]);
}

void ExpCalLoad()
{
    u8 buf[14];
    EEread(EE_REG_LX_MIN, buf, 14);                 // Get EEPROM values
    I2CslaveWriteMulti(EXP_REG_LX_MIN, buf, 14);    // Transfer to I2C register
    ExpCalInit(buf);                                // Transfer to struct
}

void ExpCalStore()
{
    u8 buf[14];
    I2CslaveReadMulti(EXP_REG_LX_MIN, buf, 14);     // Get I2C register values
    EEwrite(EE_REG_LX_MIN, buf, 14);                // Transfer to EEPROM
    ExpCalInit(buf);                                // Transfer to struct
}

void ExpCalStoreDefault()
{
    u8 buf[14];
    
    // Maximum joystick boundaries, deadzone = 10, joysticks enabled, triggers disabled
    buf[0] = 0;
    buf[1] = 0;
    buf[2] = 255;
    buf[3] = 255;
    buf[4] = 0;
    buf[5] = 0;
    buf[6] = 255;
    buf[7] = 255;
    buf[8] = 10;
    buf[9] = 10;
    buf[10] = 0;
    buf[11] = 0;
    buf[12] = 3;
    buf[13] = 50;
    
    EEwrite(EE_REG_LX_MIN, buf, 14);
    ExpCalInit(buf);
}

void ExpUpdate()
{
    u8 buf[8];
    
    u32 NKcalAXneutral;
    u32 NKcalAYneutral;
    u32 NKcalAZneutral;
    u32 NKcalAXfull;
    u32 NKcalAYfull;
    u32 NKcalAZfull;
    
    switch (expMode)
    {            
        case MODE_CLASSIC:
            // Use neutral position if joystick magnitude is within deadzone or joystick is disabled          
            if ((((s16)axes.LX - 128)*((s16)axes.LX - 128) + ((s16)axes.LY - 128)*((s16)axes.LY - 128) < ((u16)cal.deadzones[DZ_L]*(u16)cal.deadzones[DZ_L])) || !cal.enable[EN_JOY_L])
            {
                axesMapped.LX = 0x7F;
                axesMapped.LY = 0x7F;
            }
            else
            {
                if (cal.invert[INV_JOY_LX]) axesMapped.LX = Map(255 - LUT[axes.LX + LUT_LX], 0, 255, calDataClassic[CC_CAL_LX_LOWER], calDataClassic[CC_CAL_LX_UPPER]);
                else axesMapped.LX = Map(LUT[axes.LX + LUT_LX], 0, 255, calDataClassic[CC_CAL_LX_LOWER], calDataClassic[CC_CAL_LX_UPPER]);
                
                if (cal.invert[INV_JOY_LY]) axesMapped.LY = Map(255 - LUT[axes.LY + LUT_LY], 0, 255, calDataClassic[CC_CAL_LY_LOWER], calDataClassic[CC_CAL_LY_UPPER]);
                else axesMapped.LY = Map(LUT[axes.LY + LUT_LY], 0, 255, calDataClassic[CC_CAL_LY_LOWER], calDataClassic[CC_CAL_LY_UPPER]);
            }
            
            if ((((s16)axes.RX - 128)*((s16)axes.RX - 128) + ((s16)axes.RY - 128)*((s16)axes.RY - 128) < ((u16)cal.deadzones[DZ_R]*(u16)cal.deadzones[DZ_R])) || !cal.enable[EN_JOY_R])
            {
                axesMapped.RX = 0x7F;
                axesMapped.RY = 0x7F;
            }
            else
            {
                if (cal.invert[INV_JOY_RX]) axesMapped.RX = Map(255 - LUT[axes.RX + LUT_RX], 0, 255, calDataClassic[CC_CAL_RX_LOWER], calDataClassic[CC_CAL_RX_UPPER]);  
                else axesMapped.RX = Map(LUT[axes.RX + LUT_RX], 0, 255, calDataClassic[CC_CAL_RX_LOWER], calDataClassic[CC_CAL_RX_UPPER]);
                
                if (cal.invert[INV_JOY_RY]) axesMapped.RY = Map(255 - LUT[axes.RY + LUT_RY], 0, 255, calDataClassic[CC_CAL_RY_LOWER], calDataClassic[CC_CAL_RY_UPPER]);  
                else axesMapped.RY = Map(LUT[axes.RY + LUT_RY], 0, 255, calDataClassic[CC_CAL_RY_LOWER], calDataClassic[CC_CAL_RY_UPPER]); 
            }
            
            if (!cal.enable[EN_TRIG])
            {
                if (!buttons.L) axesMapped.LT = 0xFF;
                else axesMapped.LT = calDataClassic[CC_CAL_LT_LOWER];
                
                if (!buttons.R) axesMapped.RT = 0xFF;
                else axesMapped.RT = calDataClassic[CC_CAL_RT_LOWER];
            }
            else
            {
                if (cal.invert[INV_TRIG_L]) axesMapped.LT = Map(255 - axes.LT, 0, 255, calDataClassic[CC_CAL_LT_LOWER], 255);
                else axesMapped.LT = Map(axes.LT, 0, 255, calDataClassic[CC_CAL_LT_LOWER], 255); 
                
                if (cal.invert[INV_TRIG_R]) axesMapped.RT = Map(255 - axes.RT, 0, 255, calDataClassic[CC_CAL_RT_LOWER], 255);
                else axesMapped.RT = Map(axes.RT, 0, 255, calDataClassic[CC_CAL_RT_LOWER], 255);  
            }
            
            if (fullModeEn)
            {
                buf[0] = axesMapped.LX;
                buf[1] = axesMapped.RX;
                buf[2] = axesMapped.LY;
                buf[3] = axesMapped.RY;
                buf[4] = axesMapped.LT;
                buf[5] = axesMapped.RT;
            
                buf[6] = ((buttons.DR & 1) << 7) |  
                         ((buttons.DD & 1) << 6) |     
                         ((buttons.L & 1) << 5) |      
                         ((buttons.Minus & 1) << 4) |   
                         ((buttons.Home & 1) << 3) |      
                         ((buttons.Plus & 1) << 2) |     
                         ((buttons.R & 1) << 1) | 0x01;  
    
                buf[7] = ((buttons.ZL & 1) << 7) |
                         ((buttons.B & 1) << 6) |
                         ((buttons.Y & 1) << 5) |
                         ((buttons.A & 1) << 4) |
                         ((buttons.X & 1) << 3) |
                         ((buttons.ZR & 1) << 2) |
                         ((buttons.DL & 1) << 1) |
                          (buttons.DU & 1);
            
                I2CslaveWriteMulti(EXP_REG_DATA, buf, 8);
            }
            else
            {
                axesMapped.LX = axesMapped.LX >> 2;
                axesMapped.LY = axesMapped.LY >> 2;
                axesMapped.RX = axesMapped.RX >> 3;
                axesMapped.RY = axesMapped.RY >> 3;
                axesMapped.LT = axesMapped.LT >> 3;
                axesMapped.RT = axesMapped.RT >> 3;
                
                buf[0] = ((axesMapped.RX << 3) & 0xC0) |    // RX [4:3]
                          (axesMapped.LX & 0x3F);           // LX
                buf[1] = ((axesMapped.RX << 5) & 0xC0) |    // RX [2:1]
                          (axesMapped.LY & 0x3F);           // LY
                buf[2] = ((axesMapped.RX << 7) & 0x80) |    // RX [0]
                         ((axesMapped.LT << 2) & 0x60) |    // LT [4:3]
                          (axesMapped.RY & 0x1F);           // RY 
                buf[3] = ((axesMapped.LT << 5) & 0xE0) |    // LT [2:0]
                          (axesMapped.RT & 0x1F);           // RT
            
                buf[4] = ((buttons.DR & 1) << 7) |  
                         ((buttons.DD & 1) << 6) |     
                         ((buttons.L & 1) << 5) |      
                         ((buttons.Minus & 1) << 4) |   
                         ((buttons.Home & 1) << 3) |      
                         ((buttons.Plus & 1) << 2) |     
                         ((buttons.R & 1) << 1) | 0x01;  
    
                buf[5] = ((buttons.ZL & 1) << 7) |
                         ((buttons.B & 1) << 6) |
                         ((buttons.Y & 1) << 5) |
                         ((buttons.A & 1) << 4) |
                         ((buttons.X & 1) << 3) |
                         ((buttons.ZR & 1) << 2) |
                         ((buttons.DL & 1) << 1) |
                          (buttons.DU & 1);
            
                I2CslaveWriteMulti(EXP_REG_DATA, buf, 6);
            }
            break;
            
        case MODE_NUNCHUK:
            // Initialize accelerometer calibration constants
            NKcalAXneutral = (calDataNunchuk[NK_CAL_AX_0G] << 2) | ((calDataNunchuk[NK_CAL_A_0G_L] >> 4) & 0x03);
            NKcalAYneutral = (calDataNunchuk[NK_CAL_AY_0G] << 2) | ((calDataNunchuk[NK_CAL_A_0G_L] >> 2) & 0x03);
            NKcalAZneutral = (calDataNunchuk[NK_CAL_AZ_0G] << 2) | (calDataNunchuk[NK_CAL_A_0G_L] & 0x03);
            NKcalAXfull = (calDataNunchuk[NK_CAL_AX_1G] << 2) | ((calDataNunchuk[NK_CAL_A_1G_L] >> 4) & 0x03);
            NKcalAYfull = (calDataNunchuk[NK_CAL_AY_1G] << 2) | ((calDataNunchuk[NK_CAL_A_1G_L] >> 2) & 0x03);
            NKcalAZfull = (calDataNunchuk[NK_CAL_AZ_1G] << 2) | (calDataNunchuk[NK_CAL_A_1G_L] & 0x03);
            
            // Update IR camera cursor
            if (cal.enable[EN_CAM]) CamUpdateCursor();
            
            // Use neutral position if joystick magnitude is within deadzone or joystick is disabled
            if ((((s16)axes.LX - 128)*((s16)axes.LX - 128) + ((s16)axes.LY - 128)*((s16)axes.LY - 128) < ((u16)cal.deadzones[DZ_L]*(u16)cal.deadzones[DZ_L])) || !cal.enable[EN_JOY_L])
            {
                axesMapped.LX = 0x7F;
                axesMapped.LY = 0x7F;
            }
            else
            {
                if (cal.invert[INV_JOY_LX]) axesMapped.LX = Map(255 - LUT[axes.LX + LUT_LX], 0, 255, calDataNunchuk[NK_CAL_SX_LOWER], calDataNunchuk[NK_CAL_SX_UPPER]);
                else axesMapped.LX = Map(LUT[axes.LX + LUT_LX], 0, 255, calDataNunchuk[NK_CAL_SX_LOWER], calDataNunchuk[NK_CAL_SX_UPPER]);
                
                if (cal.invert[INV_JOY_LY]) axesMapped.LY = Map(255 - LUT[axes.LY + LUT_LY], 0, 255, calDataNunchuk[NK_CAL_SY_LOWER], calDataNunchuk[NK_CAL_SY_UPPER]);
                else axesMapped.LY = Map(LUT[axes.LY + LUT_LY], 0, 255, calDataNunchuk[NK_CAL_SY_LOWER], calDataNunchuk[NK_CAL_SY_UPPER]);
            }
            
            if (cal.invert[INV_XL_X]) axesMapped.AX = (u16)(((-1)*((s32)axes.AX) * (NKcalAXfull - NKcalAXneutral)) / 256) + NKcalAXneutral;
            else axesMapped.AX = (u16)(((s32)axes.AX * (NKcalAXfull - NKcalAXneutral)) / 256) + NKcalAXneutral;
            
            if (cal.invert[INV_XL_Y]) axesMapped.AY = (u16)(((-1)*((s32)axes.AY) * (NKcalAYfull - NKcalAYneutral)) / 256) + NKcalAYneutral;
            else axesMapped.AY = (u16)(((s32)axes.AY * (NKcalAYfull - NKcalAYneutral)) / 256) + NKcalAYneutral;
            
            if (cal.invert[INV_XL_Z]) axesMapped.AZ = (u16)(((-1)*((s32)axes.AZ) * (NKcalAZfull - NKcalAZneutral)) / 256) + NKcalAZneutral;
            else axesMapped.AZ = (u16)(((s32)axes.AZ * (NKcalAZfull - NKcalAZneutral)) / 256) + NKcalAZneutral;
            
            buf[0] =   axesMapped.LX;                   // LX
            buf[1] =   axesMapped.LY;                   // LY
            buf[2] =  (axesMapped.AX >> 2) & 0xFF;      // AccelX [9:2]
            buf[3] =  (axesMapped.AY >> 2) & 0xFF;      // AccelY [9:2]
            buf[4] =  (axesMapped.AZ >> 2) & 0xFF;      // AccelZ [9:2]
            buf[5] = ((axesMapped.AZ << 6) & 0xC0) |    // AccelZ [1:0]
                     ((axesMapped.AY << 4) & 0x30) |    // AccelY [1:0]
                     ((axesMapped.AX << 2) & 0x0C) |    // AccelX [1:0]
                     ((buttons.C & 1) << 1) |
                      (buttons.Z & 1);
            
            I2CslaveWriteMulti(EXP_REG_DATA, buf, 6);
            break;
            
        default:
            break;
    }
    
    if (cfgEn)
    {
        u8 rawBuf[6];
        
        rawBuf[0] = axes.LX;
        rawBuf[1] = axes.LY;
        rawBuf[2] = axes.RX;
        rawBuf[3] = axes.RY;
        rawBuf[4] = axes.LT;
        rawBuf[5] = axes.RT;
        
        I2CslaveWriteMulti(EXP_REG_LX_RAW, rawBuf, 6);
    }
}

void ExpUpdateDefault()
{
    u8 buf[8];
    
    // All buttons unpressed, all axes centered
    switch (expMode)
    {
        case MODE_CLASSIC:
            if (fullModeEn)
            {
                buf[0] = 0x7F;
                buf[1] = 0x7F;
                buf[2] = 0x7F;
                buf[3] = 0x7F;
                buf[4] = 0x7F;
                buf[5] = 0x7F;
                buf[6] = 0xFF;
                buf[7] = 0xFF;
                I2CslaveWriteMulti(EXP_REG_DATA, buf, 8);
            }
            else
            {
                buf[0] = 0x5F;
                buf[1] = 0xDF;
                buf[2] = 0x8F;
                buf[3] = 0x00;
                buf[4] = 0xFF;
                buf[5] = 0xFF;
                I2CslaveWriteMulti(EXP_REG_DATA, buf, 6);
            }
            break;
        
        case MODE_NUNCHUK:
            buf[0] = 0x7F;
            buf[1] = 0x7F;
            buf[2] = 0x7F;
            buf[3] = 0x7F;
            buf[4] = 0xB4;
            buf[5] = 0x3F;
            I2CslaveWriteMulti(EXP_REG_DATA, buf, 6);
            break;
        
        default:
            break;
    }   
}
