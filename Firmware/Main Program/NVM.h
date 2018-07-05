/* 
 * File:   NVM.h
 * Author: Jackson Snowden
 */

#ifndef _NVM_H_
#define	_NVM_H_

void NVMunlock();

void EEwrite(u8 addr, u8 *buf, u8 length);

void EEread(u8 addr, u8 *buf, u8 length);

#endif  /* _NVM_H_ */