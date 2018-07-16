/* 
 * File:   expansion.h  
 * Author: Jackson Snowden
 */

#ifndef _EXPANSION_H_
#define	_EXPANSION_H_

// Expansion controller 7-bit I2C address
#define EXP_I2C_ADDR    0x52

// Nintendo I2C register definitions
#define EXP_REG_DATA    0x00    // Button/axis data (0x00 to 0x05)
#define EXP_REG_CAL     0x20    // Calibration values (0x20 to 0x3F)
#define EXP_REG_KEY     0x40    // Encryption keys (0x40 to 0x4F)
#define EXP_REG_ID      0xFA    // Device ID (0xFA to 0xFF)
#define EXP_REG_SETUP1  0xF0
#define EXP_REG_SETUP2  0xFB

// Custom I2C register definitions
#define EXP_REG_LX_MIN  0x60    // Left joystick X minimum
#define EXP_REG_LY_MIN  0x61    // Left joystick Y minimum
#define EXP_REG_LX_MAX  0x62    // Left joystick X maximum
#define EXP_REG_LY_MAX  0x63    // Left joystick Y maximum
#define EXP_REG_RX_MIN  0x64    // Right joystick X minimum
#define EXP_REG_RY_MIN  0x65    // Right joystick Y minimum
#define EXP_REG_RX_MAX  0x66    // Right joystick X maximum
#define EXP_REG_RY_MAX  0x67    // Right joystick Y maximum
#define EXP_REG_DZ_L    0x68    // Left joystick deadzone radius
#define EXP_REG_DZ_R    0x69    // Right joystick deadzone radius
#define EXP_REG_INVERT1 0x6A    // Invert active high ( 0 | 0 | RT | LT | RY | RX | LY | LX )
#define EXP_REG_INVERT2 0x6B    // Invert active high ( 0 | 0 | 0 | 0 | 0 | AZ | AY | AX )
#define EXP_REG_CONFIG  0x6C    // Misc. settings ( 0 | 0 | 0 | 0 | IR Camera Enable | Trigger Enable | Right Joystick Enable | Left Joystick Enable )
#define EXP_REG_IR_SENS 0x6D    // Camera sensitivity
#define EXP_REG_CMD     0x6F    // Command reception from Wii Remote
#define EXP_REG_LX_RAW  0x70    // Raw LX output
#define EXP_REG_LY_RAW  0x71    // Raw LY output
#define EXP_REG_RX_RAW  0x72    // Raw RX output
#define EXP_REG_RY_RAW  0x73    // Raw RY output
#define EXP_REG_LT_RAW  0x74    // Raw LT output
#define EXP_REG_RT_RAW  0x75    // Raw RT output
#define EXP_REG_FW_VER  0x81    // Device firmware version
#define EXP_REG_CID     0x82    // Custom device ID

// Classic+ ID
#define CID 0xCC

// Custom I2C commands
#define PGM_EN          0x1A    // Enable programming mode
#define PGM_DIS         0x2A    // Disable programming mode
#define CAL_LOAD        0x1B	// Load EEPROM data into I2C registers
#define CAL_STORE       0x1C	// Store data loaded into I2C registers (0x60 through 0x6C)
#define CAL_DEFAULT     0x1D    // Reset settngs in EEPROM
#define CFG_EN          0x1E    // Enable configuration mode
#define CFG_DIS         0x2E    // Disable configuration mode
#define ENC_EN          0x1F    // Enable device encryption

extern u8 LUT[1024];

// LUT register positions
#define LUT_LX  0x000
#define LUT_LY  0x100
#define LUT_RX  0x200
#define LUT_RY  0x300

// Classic Controller calibration data register positions
#define CC_CAL_LX_UPPER 0
#define CC_CAL_LX_LOWER 1
#define CC_CAL_LY_UPPER 3
#define CC_CAL_LY_LOWER 4
#define CC_CAL_RX_UPPER 6
#define CC_CAL_RX_LOWER 7
#define CC_CAL_RY_UPPER 9
#define CC_CAL_RY_LOWER 10
#define CC_CAL_LT_LOWER 12
#define CC_CAL_RT_LOWER 13

// Nunchuk calibration data register positions
#define NK_CAL_AX_0G    0
#define NK_CAL_AY_0G    1
#define NK_CAL_AZ_0G    2
#define NK_CAL_A_0G_L   3
#define NK_CAL_AX_1G    4
#define NK_CAL_AY_1G    5
#define NK_CAL_AZ_1G    6
#define NK_CAL_A_1G_L   7
#define NK_CAL_SX_UPPER 8
#define NK_CAL_SX_LOWER 9
#define NK_CAL_SY_UPPER 11
#define NK_CAL_SY_LOWER 12

typedef struct
{
    u8 minMax[8];
    u8 deadzones[2];
    u8 invert[9];
    u8 enable[4];
} 
Calibration;

extern Calibration cal;

// Calibration struct register positions
#define MIN_LX  0
#define MIN_LY  1
#define MAX_LX  2
#define MAX_LY  3
#define MIN_RX  4
#define MIN_RY  5
#define MAX_RX  6
#define MAX_RY  7

#define DZ_L    0
#define DZ_R    1

#define INV_JOY_LX  0
#define INV_JOY_LY  1
#define INV_JOY_RX  2
#define INV_JOY_RY  3
#define INV_TRIG_L  4
#define INV_TRIG_R  5
#define INV_XL_X    6
#define INV_XL_Y    7
#define INV_XL_Z    8

#define EN_JOY_L    0
#define EN_JOY_R    1
#define EN_TRIG     2
#define EN_CAM      3

void ExpInit(const u8 *ID);

void ExpOff();

u8 ExpIsEnabled();

void ExpSetMode(u8 mode);

void ExpCmdRcv(u8 data, u8 addr);

u8 ExpCmdExec();

u8 ExpIsEncEnabled();

void ExpCalInit(u8 *buf);

void ExpCalLoad();

void ExpCalStore();

void ExpCalStoreDefault();

void ExpUpdate();

void ExpUpdateDefault();

#endif  /* _EXPANSION_H_ */
