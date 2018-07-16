/* 
 * File:   NVM.h
 * Author: Jackson Snowden
 */

#ifndef _NVM_H_
#define	_NVM_H_

// Expansion/camera calibration registers
#define EE_REG_LX_MIN   0x00
#define EE_REG_LY_MIN   0x01
#define EE_REG_LX_MAX   0x02
#define EE_REG_LY_MAX   0x03
#define EE_REG_RX_MIN   0x04
#define EE_REG_RY_MIN   0x05
#define EE_REG_RX_MAX   0x06
#define EE_REG_RY_MAX   0x07
#define EE_REG_DZ_L     0x08
#define EE_REG_DZ_R     0x09
#define EE_REG_INVERT1  0x0A
#define EE_REG_INVERT2  0x0B
#define EE_REG_CONFIG   0x0C
#define EE_REG_IR_SENS  0x0D

void NVMunlock();

void EEwrite(u8 addr, u8 *buf, u8 length);

void EEread(u8 addr, u8 *buf, u8 length);

#endif  /* _NVM_H_ */
