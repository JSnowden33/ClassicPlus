/* 
 * File:   expansion.h  
 * Author: Jackson Snowden
 */

#ifndef _CAMERA_H_
#define	_CAMERA_H_

// IR camera 7-bit I2C address
#define CAM_I2C_ADDR    0x58

// I2C register definitions
#define CAM_REG_SETUP1  0x00    // Sensitivity data (0x00 to 0x08)
#define CAM_REG_SETUP2  0x1A    // Sensitivity data (0x1A to 0x1B)
#define CAM_REG_ENABLE  0x30
#define CAM_REG_MODE    0x33
#define CAM_REG_DATA    0x37    // Blob data (0x37 to 0x5B)

// Camera reporting modes
#define CAM_BASIC       1
#define CAM_EXTENDED    3
#define CAM_FULL        5

// Default blob settings
#define CAM_OFFSET      70
#define CAM_BLOB_SIZE   8

// Center coordinates from Wii home menu
#define CAM_X_CENTER    525
#define CAM_Y_CENTER    310

// Cycles required for cursor idle timeout
#define CAM_TIMEOUT     150

typedef struct
{
    s32 X;
    s32 Y;
} 
Cursor;

void CamInit();

void CamOff();

void CamSetSensitivity(u8 sens);

void CamCmdRcv(u8 data, u8 addr);

void CamUpdateCursor();

void CamUpdateBlobs();

#endif  /* _CAMERA_H_ */