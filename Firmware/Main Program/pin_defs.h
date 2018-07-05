/* 
 * File:   pin_defs.h  
 * Author: Jackson Snowden
 */

#ifndef _PIN_DEFS_H_
#define	_PIN_DEFS_H_

// Button pin definitions
#define A_PIN       RA4
#define B_PIN       RA5
#define X_PIN       RE0
#define Y_PIN       RE1
#define DU_PIN      RD6
#define DD_PIN      RD5
#define DR_PIN      RD4
#define DL_PIN      RC7
#define R_PIN       RD0
#define ZR_PIN      RD1
#define L_PIN       RD3
#define ZL_PIN      RD2
#define PLUS_PIN    RE2
#define MINUS_PIN   RA6
#define HOME_PIN    RA7
#define C_PIN       L_PIN
#define Z_PIN       ZL_PIN

// Axis pin definitions
#define LX_PIN  RC6
#define LY_PIN  RC5
#define RX_PIN  RC1
#define RY_PIN  RC2
#define LT_PIN  RC4
#define RT_PIN  RC3

// Axis ADC channel definitions
#define LX_CH   22
#define LY_CH   21
#define RX_CH   17
#define RY_CH   18
#define LT_CH   20
#define RT_CH   19

// Communication pin definitions
#define MOSI    LATB0
#define MISO    RD7
#define SCK     LATB1
#define CS      LATB2
#define SDA     RB3
#define SCL     RB4
#define DETECT  LATB5
#define MODE    RC0
#define ENABLE  RA0

// PPS output signal names
#define PPS_SDA2    0x17
#define PPS_SDO2    0x17
#define PPS_SCL2    0x16
#define PPS_SCK2    0x16
#define PPS_SDA1    0x15
#define PPS_SDO1    0x15
#define PPS_SCL1    0x14
#define PPS_SCK1    0x14

// PPS input pin names
#define PPS_RB0     0x08
#define PPS_RB1     0x09
#define PPS_RB2     0x0A
#define PPS_RB3     0x0B
#define PPS_RB4     0x0C
#define PPS_RD7     0x1F

#endif  /* _PIN_DEFS_H_ */