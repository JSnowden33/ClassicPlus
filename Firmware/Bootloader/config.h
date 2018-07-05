/* 
 * File:   config.h  
 * Author: Jackson Snowden
 */

#ifndef _CONFIG_H_
#define	_CONFIG_H_

#define _XTAL_FREQ  32000000

#define u8  unsigned char
#define u16 unsigned int
#define u32 unsigned long
#define s8  signed char
#define s16 signed int
#define s32 signed long

// Classic+ Safe Mode ID
#define CID 0xBB

// Starting address of main application code
#define APP_ADDR    0x700

#endif  /* _CONFIG_H_ */