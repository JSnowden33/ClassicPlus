/* 
 * File:   IMU.h  
 * Author: Jackson Snowden
 */

#ifndef _IMU_H_
#define	_IMU_H_

// SPI commands
#define SPI_WRITE       0x00
#define SPI_READ        0x80
#define SPI_NOP         0xFF
#define IMU_REG_MASK    0x7F

// Master SPI CLK speed in Hz
#define IMU_SPI_CLK     1000000

// +/- accelerometer scales
#define XL_OFF          0xFF
#define XL_SCALE_2G     0b00
#define XL_SCALE_4G     0b10
#define XL_SCALE_8G     0b11
#define XL_SCALE_16G    0b01

// +/- gyroscope scales
#define G_OFF           0xFF
#define G_SCALE_125DPS  0b001
#define G_SCALE_250DPS  0b000
#define G_SCALE_500DPS  0b010
#define G_SCALE_1000DPS 0b100
#define G_SCALE_2000DPS 0b110

// LSM6DS3 register addresses
#define ORIENT_CFG_G    0x0B
#define INT1_CTRL       0x0D
#define INT2_CTRL       0x0E
#define ID_REG          0x0F
#define CTRL1_XL        0x10
#define CTRL2_G         0x11
#define CTRL3_C         0x12
#define CTRL4_C         0x13
#define CTRL5_C         0x14
#define CTRL6_C         0x15
#define CTRL7_G         0x16
#define CTRL8_XL        0x17
#define CTRL9_XL        0x18
#define CTRL10_C        0x19
#define MASTER_CONFIG   0x1A
#define STATUS_REG      0x1E
#define OUTX_L_G        0x22
#define OUTX_H_G        0x23
#define OUTY_L_G        0x24
#define OUTY_H_G        0x25
#define OUTZ_L_G        0x26
#define OUTZ_H_G        0x27
#define OUTX_L_XL       0x28
#define OUTX_H_XL       0x29
#define OUTY_L_XL       0x2A
#define OUTY_H_XL       0x2B
#define OUTZ_L_XL       0x2C
#define OUTZ_H_XL       0x2D

#define IMU_ID          0x69

u8 IMUinit(u8 aScale, u8 gScale);

void IMUoff();

u8 IMUisEnabled();

void IMUwriteReg(u8 reg, u8 data);

void IMUwriteRegMulti(u8 reg, u8 *data, u8 len);

u8 IMUreadReg(u8 reg);

void IMUreadRegMulti(u8 reg, u8 *data, u8 len);

s16 IMUreadAccelX();

s16 IMUreadAccelY();

s16 IMUreadAccelZ();

s16 IMUreadGyroX();

s16 IMUreadGyroY();

s16 IMUreadGyroZ();

#endif  /* _IMU_H_ */