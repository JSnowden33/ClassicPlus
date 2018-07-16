/*
 * File:   camera.c
 * Author: Jackson Snowden
 */

#include <xc.h>
#include "config.h"
#include "input.h"
#include "MSSP.h"
#include "expansion.h"
#include "camera.h"

// Cursor position
Cursor cur;

u8 camMode;
u8 camEn;

u32 timeoutCount;
u8 cursorIdle;

u8 sensitivity;

void CamInit()
{
    cur.X = CAM_X_CENTER * 8;
    cur.Y = CAM_Y_CENTER * 8;
    
    if (!camMode) camMode = CAM_BASIC;
    
    timeoutCount = 0;
    cursorIdle = 1;
    camEn = 1;
}

void CamOff()
{
    cursorIdle = 1;
    camEn = 0;
}

void CamSetSensitivity(u8 sens)
{
    sensitivity = sens;
}

void CamCmdRcv(u8 data, u8 addr)
{
    switch (addr)
    {
        // Set reporting mode
        case CAM_REG_MODE:
            camMode = data;
            break;
            
        default:
            break;
    }
}

void CamUpdateCursor()
{
    s16 joyX;
    s16 joyY;
    
    if ((((s16)axes.RX - 128)*((s16)axes.RX - 128) + ((s16)axes.RY - 128)*((s16)axes.RY - 128) < ((u16)cal.deadzones[DZ_R]*(u16)cal.deadzones[DZ_R])) || !cal.enable[EN_JOY_R])
    {
        joyX = 0;
        joyY = 0;
    }
    else
    {
        joyX = 128 - LUT[axes.RX + LUT_RX];
        joyY = 128 - LUT[axes.RY + LUT_RY];
    }
    
    // Reset timer if joystick is not centered
    if (joyX || joyY) 
    {
        timeoutCount = 0;
        cursorIdle = 0;
    }
    
    if (!cursorIdle)
    {
        s16 partialX = (joyX * sensitivity) / 256;
        s16 partialY = (joyY * sensitivity) / 256;
        
        cur.X += partialX;
        cur.Y += partialY;
        
        // Set X boundaries
        if (cur.X < CAM_OFFSET) cur.X = CAM_OFFSET;
        else if (cur.X > (0x3FF * 8 - CAM_OFFSET)) cur.X = 0x3FF * 8 - CAM_OFFSET;
        
        // Set Y boundaries
        if (cur.Y < 0) cur.Y = 0;
        else if (cur.Y > (0x3FF * 8)) cur.Y = 0x3FF * 8;
        
        timeoutCount++;
    }
    
    if (timeoutCount > CAM_TIMEOUT)
    {
        timeoutCount = 0;
        cursorIdle = 1;
    }
}

void CamUpdateBlobs()
{
    // { X1, Y1, X2, Y2 }
    u16 pos[4];

    if (cursorIdle)
    {
        pos[0] = 0x3FF;
        pos[1] = 0x3FF;
        pos[2] = 0x3FF;
        pos[3] = 0x3FF;
    }
    else
    {
        pos[0] = (cur.X / 8) + CAM_OFFSET;
        pos[1] = (cur.Y / 8);
        pos[2] = (cur.X / 8) - CAM_OFFSET;
        pos[3] = (cur.Y / 8);
    }
    
    u8 buf[36];
    switch (camMode)
    {
        case CAM_BASIC:
            buf[0] = pos[0];
            buf[1] = pos[1];
            buf[2] = ((pos[1] & 0x300) >> 2) | ((pos[0] & 0x300) >> 4) | ((pos[3] & 0x300) >> 6) | ((pos[2] & 0x300) >> 8);
            buf[3] = pos[2];
            buf[4] = pos[3];
            
            buf[5] = 0xFF;
            buf[6] = 0xFF;
            buf[7] = 0xFF;
            buf[8] = 0xFF;
            buf[9] = 0xFF;
            
            I2CslaveWriteMulti(CAM_REG_DATA + 256, buf, 10);
            break;
            
        case CAM_EXTENDED:
            buf[0] = pos[0];
            buf[1] = pos[1];
            buf[2] = ((pos[1] & 0x300) >> 2) | ((pos[0] & 0x300) >> 4) | CAM_BLOB_SIZE;
            
            buf[3] = pos[2];
            buf[4] = pos[3];
            buf[5] = ((pos[3] & 0x300) >> 2) | ((pos[2] & 0x300) >> 4) | CAM_BLOB_SIZE;
            
            buf[6] = 0xFF;
            buf[7] = 0xFF;
            buf[8] = 0xF0;
            
            buf[9] = 0xFF;
            buf[10] = 0xFF;
            buf[11] = 0xF0;
            
            I2CslaveWriteMulti(CAM_REG_DATA + 256, buf, 12);
            break;
            
        case CAM_FULL:
            buf[0] = pos[0];
            buf[1] = pos[1];
            buf[2] = ((pos[1] & 0x300) >> 2) | ((pos[0] & 0x300) >> 4) | CAM_BLOB_SIZE;
            buf[3] = 0;
            buf[4] = 0x7F;
            buf[5] = 0;
            buf[6] = 0x7F;
            buf[7] = 0;
            buf[8] = 0xFF;
            
            buf[9] = pos[2];
            buf[10] = pos[3];
            buf[11] = ((pos[3] & 0x300) >> 2) | ((pos[2] & 0x300) >> 4) | CAM_BLOB_SIZE;
            buf[12] = 0;
            buf[13] = 0x7F;
            buf[14] = 0;
            buf[15] = 0x7F;
            buf[16] = 0;
            buf[17] = 0xFF;

            buf[18] = 0xFF;
            buf[19] = 0xFF;
            buf[20] = 0xF0;
            buf[21] = 0;
            buf[22] = 0;
            buf[23] = 0;
            buf[24] = 0;
            buf[25] = 0;
            buf[26] = 0xFF;
            
            buf[27] = 0xFF;
            buf[28] = 0xFF;
            buf[29] = 0xF0;
            buf[30] = 0;
            buf[31] = 0;
            buf[32] = 0;
            buf[33] = 0;
            buf[34] = 0;
            buf[35] = 0xFF;
            
            I2CslaveWriteMulti(CAM_REG_DATA + 256, buf, 36);
            break;
        
        default:
            break;
    }
}
