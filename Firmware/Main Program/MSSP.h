/* 
 * File:   MSSP.h  
 * Author: Jackson Snowden
 */

#ifndef _MSSP_H_
#define	_MSSP_H_

// SSPSTAT register states
#define WRITE_ADDR_ACK  0b00001000
#define WRITE_DAT_ACK   0b00101000
#define READ_ADDR_ACK   0b00001100
#define READ_DAT_ACK    0b00101100

void I2CslaveInit(const u8 addr);

void I2Coff();

u8 I2CisEnabled();

u8 I2CslaveRead(u8 addr);

void I2CslaveReadMulti(u8 addr, u8 *buf, u8 length);

void I2CslaveWrite(u8 addr, u8 data);

void I2CslaveWriteMulti(u8 addr, u8 *buf, u8 length);

void I2CslaveRelease();

void I2CslaveHandle();

void SPImasterInit(const u32 clk, const u8 CKP, const u8 CKE, const u8 SMP);

void SPIoff();

u8 SPIisEnabled();

u8 SPIwait();

void SPIwrite(u8 data);

u8 SPIread();

u8 SPItransfer(u8 data);

#endif  /* _MSSP_H_ */