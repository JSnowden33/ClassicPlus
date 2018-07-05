/* 
 * File:   input.h
 * Author: Jackson Snowden
 */

#ifndef _INPUT_H_
#define	_INPUT_H_

typedef struct
{
    u8 A;
    u8 B;
    u8 X;
    u8 Y;
    u8 DU;
    u8 DD;
    u8 DR;
    u8 DL;
    u8 R;
    u8 ZR;
    u8 L;
    u8 ZL;
    u8 Plus;
    u8 Minus;
    u8 Home;
    u8 C;
    u8 Z;
} 
Button;

typedef struct
{
    u8 LX;
    u8 LY;
    u8 RX;
    u8 RY;
    u8 LT;
    u8 RT;
    s16 AX;
    s16 AY;
    s16 AZ;
} 
Axis;

// Button/axis states
extern Button buttons;
extern Axis axes;

// Number of cycles required for valid button input
#define DBNC_CONST  5

void InputInit();

void InputGetButtons(u8 mode);

void InputGetAxes(u8 mode);

#endif  /* _INPUT_H_ */