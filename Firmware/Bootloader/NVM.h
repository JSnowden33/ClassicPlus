/* 
 * File:   NVM.h
 * Author: Jackson Snowden
 */

#ifndef _NVM_H_
#define	_NVM_H_

void NVMunlock();

void PGMerase(u16 addr);

void PGMwrite(u16 addr, u16 *buf, u8 length);

void PGMread(u16 addr, u16 *buf, u16 length);

#endif  /* _NVM_H_ */