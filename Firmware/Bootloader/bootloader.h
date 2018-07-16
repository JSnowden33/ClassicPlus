/* 
 * File:   bootloader.h  
 * Author: Jackson Snowden
 */

#ifndef _BOOTLOADER_H_
#define	_BOOTLOADER_H_

// Expansion controller 7-bit I2C address
#define EXP_I2C_ADDR    0x52

// I2C register definitions
#define I2C_REG_CAL     0x20    // Calibration values (0x20 to 0x3F)
#define I2C_REG_ID      0xFA    // Device ID (0xFA to 0xFF)
#define I2C_REG_CMD     0x6F    // Command reception from Wiimote
#define I2C_REG_DAT     0xBF    // Data reception from Wiimote
#define I2C_REG_CID     0x82    // Custom device ID  

// Custom I2C commands
#define PGM_ERASE       0x11    // Program erase command
#define PGM_WRITE       0x21    // Program write command
#define PGM_READ        0x31    // Program read command
#define PGM_EN          0x1A    // Enable programming mode
#define PGM_DIS         0x2A    // Disable programming mode

// SSPSTAT register states
#define WRITE_ADDR_ACK  0b00001000
#define WRITE_DAT_ACK   0b00101000
#define READ_ADDR_ACK   0b00001100
#define READ_DAT_ACK    0b00101100

void I2CslaveInit(const u8 addr);

void I2Coff();

void CMDreceive(u8 cmd);

void CMDexec();

void PGMreceive(u8 data);

void PGMsend();

void BeginBootloader();

void ExitBootloader();

#endif  /* _BOOTLOADER_H_ */
