/* 
 * File:   config.h  
 * Author: Jackson Snowden
 */

#ifndef _CONFIG_H_
#define	_CONFIG_H_

#define _XTAL_FREQ  8000000

#define u8  unsigned char
#define u16 unsigned int
#define u32 unsigned long
#define s8  signed char
#define s16 signed int
#define s32 signed long

#define MODE_CLASSIC    0xA
#define MODE_NUNCHUK    0xB
#define MODE_OFF        0x0

#define BOOT_ADDR       0x100

#endif  /* _CONFIG_H_ */