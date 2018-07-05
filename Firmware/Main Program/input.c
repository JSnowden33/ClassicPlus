/*
 * File:   input.c
 * Author: Jackson Snowden
 */

#include <xc.h>
#include "config.h"
#include "pin_defs.h"
#include "ADC.h"
#include "IMU.h"
#include "expansion.h"
#include "input.h"

// Debounced button/axis states
Button buttons;
Axis axes;

// Raw previous/current states of each button
Button buttonsPrev;
Button buttonsCur;

// Variables for counting how long a button is pressed
u8 countA;
u8 countB;
u8 countX;
u8 countY;
u8 countDU;
u8 countDD;
u8 countDR;
u8 countDL;
u8 countL;
u8 countZL;
u8 countR;
u8 countZR;
u8 countPlus;
u8 countMinus;
u8 countHome;
u8 countC;
u8 countZ;

void InputInit()
{
    ADCinit();
    
    buttons.A = 1;
    buttons.B = 1;
    buttons.X = 1;
    buttons.Y = 1;
    buttons.DU = 1;
    buttons.DD = 1;
    buttons.DR = 1;
    buttons.DL = 1;
    buttons.R = 1;
    buttons.ZR = 1;
    buttons.L = 1;
    buttons.ZL = 1;
    buttons.Plus = 1;
    buttons.Minus = 1;
    buttons.Home = 1;
    
    axes.LX = 127;
    axes.LY = 127;
    axes.RX = 127;
    axes.RY = 127;
    axes.LT = 0;
    axes.RT = 0;
}

void InputGetButtons(u8 mode)
{
    switch (mode)
    {
        case MODE_CLASSIC:
            // Store most recent button state
            buttonsCur.A = A_PIN;
            buttonsCur.B = B_PIN;
            buttonsCur.X = X_PIN;
            buttonsCur.Y = Y_PIN;
            buttonsCur.DU = DU_PIN;
            buttonsCur.DD = DD_PIN;
            buttonsCur.DR = DR_PIN;
            buttonsCur.DL = DL_PIN;
            buttonsCur.R = R_PIN;
            buttonsCur.ZR = ZR_PIN;
            buttonsCur.L = L_PIN;
            buttonsCur.ZL = ZL_PIN;
            buttonsCur.Plus = PLUS_PIN;
            buttonsCur.Minus = MINUS_PIN;
            buttonsCur.Home = HOME_PIN;
            
            // Increase count if button is pressed
            if (!buttonsCur.A) countA++;
            if (!buttonsCur.B) countB++;
            if (!buttonsCur.X) countX++;
            if (!buttonsCur.Y) countY++;
            if (!buttonsCur.DU) countDU++;
            if (!buttonsCur.DD) countDD++;
            if (!buttonsCur.DR) countDR++;
            if (!buttonsCur.DL) countDL++;
            if (!buttonsCur.R) countR++;
            if (!buttonsCur.ZR) countZR++;
            if (!buttonsCur.L) countL++;
            if (!buttonsCur.ZL) countZL++;
            if (!buttonsCur.Plus) countPlus++;
            if (!buttonsCur.Minus) countMinus++;
            if (!buttonsCur.Home) countHome++;
            
            // Clear button if button state has changed
            if (buttonsPrev.A ^ buttonsCur.A)
            {
                countA = 0;
                buttons.A = 1;
            }
            if (buttonsPrev.B ^ buttonsCur.B)
            {
                countB = 0;
                buttons.B = 1;
            }
            if (buttonsPrev.X ^ buttonsCur.X)
            {
                countX = 0;
                buttons.X = 1;
            }
            if (buttonsPrev.Y ^ buttonsCur.Y)
            {
                countY = 0;
                buttons.Y = 1;
            }
            if (buttonsPrev.DU ^ buttonsCur.DU)
            {
                countDU = 0;
                buttons.DU = 1;
            }
            if (buttonsPrev.DD ^ buttonsCur.DD)
            {
                countDD = 0;
                buttons.DD = 1;
            }
            if (buttonsPrev.DR ^ buttonsCur.DR)
            {
                countDR = 0;
                buttons.DR = 1;
            }
            if (buttonsPrev.DL ^ buttonsCur.DL)
            {
                countDL = 0;
                buttons.DL = 1;
            }
            if (buttonsPrev.R ^ buttonsCur.R)
            {
                countR = 0;
                buttons.R = 1;
            }
            if (buttonsPrev.ZR ^ buttonsCur.ZR)
            {
                countZR = 0;
                buttons.ZR = 1;
            }
            if (buttonsPrev.L ^ buttonsCur.L)
            {
                countL = 0;
                buttons.L = 1;
            }
            if (buttonsPrev.ZL ^ buttonsCur.ZL)
            {
                countZL = 0;
                buttons.ZL = 1;
            }
            if (buttonsPrev.Plus ^ buttonsCur.Plus)
            {
                countPlus = 0;
                buttons.Plus = 1;
            }
            if (buttonsPrev.Minus ^ buttonsCur.Minus)
            {
                countMinus = 0;
                buttons.Minus = 1;
            }
            if (buttonsPrev.Home ^ buttonsCur.Home)
            {
                countHome = 0;
                buttons.Home = 1;
            }
            
            // Store current button states for next check
            buttonsPrev.A = buttonsCur.A;
            buttonsPrev.B = buttonsCur.B;
            buttonsPrev.X = buttonsCur.X;
            buttonsPrev.Y = buttonsCur.Y;
            buttonsPrev.DU = buttonsCur.DU;
            buttonsPrev.DD = buttonsCur.DD;
            buttonsPrev.DR = buttonsCur.DR;
            buttonsPrev.DL = buttonsCur.DL;
            buttonsPrev.R = buttonsCur.R;
            buttonsPrev.ZR = buttonsCur.ZR;
            buttonsPrev.L = buttonsCur.L;
            buttonsPrev.ZL = buttonsCur.ZL;
            buttonsPrev.Plus = buttonsCur.Plus;
            buttonsPrev.Minus = buttonsCur.Minus;
            buttonsPrev.Home = buttonsCur.Home;
                
            // Set button if count has reached threshold
            if (countA >= DBNC_CONST) buttons.A = 0;
            if (countB >= DBNC_CONST) buttons.B = 0;
            if (countX >= DBNC_CONST) buttons.X = 0;
            if (countY >= DBNC_CONST) buttons.Y = 0;
            if (countDU >= DBNC_CONST) buttons.DU = 0;
            if (countDD >= DBNC_CONST) buttons.DD = 0;
            if (countDR >= DBNC_CONST) buttons.DR = 0;
            if (countDL >= DBNC_CONST) buttons.DL = 0;
            if (countR >= DBNC_CONST) buttons.R = 0;
            if (countZR >= DBNC_CONST) buttons.ZR = 0;
            if (countL >= DBNC_CONST) buttons.L = 0;
            if (countZL >= DBNC_CONST) buttons.ZL = 0;
            if (countPlus >= DBNC_CONST) buttons.Plus = 0;
            if (countMinus >= DBNC_CONST) buttons.Minus = 0;
            if (countHome >= DBNC_CONST) buttons.Home = 0;
            break;
            
        case MODE_NUNCHUK:
            // Store most recent button state
            buttonsCur.C = C_PIN;
            buttonsCur.Z = Z_PIN;
            
            // Increase count if button is pressed
            if (!buttonsCur.C) countC++;
            if (!buttonsCur.Z) countZ++;
            
            // Clear button if button state has changed
            if (buttonsPrev.C ^ buttonsCur.C)
            {
                countC = 0;
                buttons.C = 1;
            }
            if (buttonsPrev.Z ^ buttonsCur.Z)
            {
                countZ = 0;
                buttons.Z = 1;
            }
            
            // Store current button states for next check
            buttonsPrev.C = buttonsCur.C;
            buttonsPrev.Z = buttonsCur.Z;
                
            // Set button if count has reached threshold
            if (countC >= DBNC_CONST) buttons.C = 0;
            if (countZ >= DBNC_CONST) buttons.Z = 0;
            break;
            
        default:
            break;
    }
}

void InputGetAxes(u8 mode)
{      
    switch (mode)
    {
        case MODE_CLASSIC:
            axes.LX = ADCread(LX_CH) >> 2;
            axes.LY = ADCread(LY_CH) >> 2;
            axes.RX = ADCread(RX_CH) >> 2;
            axes.RY = ADCread(RY_CH) >> 2;
            axes.LT = ADCread(LT_CH) >> 2;
            axes.RT = ADCread(RT_CH) >> 2;
            break;
            
        case MODE_NUNCHUK: 
            axes.LX = ADCread(LX_CH) >> 2;
            axes.LY = ADCread(LY_CH) >> 2;
            
            // Get all axis values in configuration mode
            if (ExpIsCfgEnabled())
            {
                axes.RX = ADCread(RX_CH) >> 2;
                axes.RY = ADCread(RY_CH) >> 2;
                axes.LT = ADCread(LT_CH) >> 2;
                axes.RT = ADCread(RT_CH) >> 2;
            }
            
            // Read accelerometer values if IMU is initialized
            if (IMUisEnabled())
            {
                axes.AX = IMUreadAccelX() >> 6;
                axes.AY = IMUreadAccelY() >> 6;
                axes.AZ = IMUreadAccelZ() >> 6;
            }
            else
            {
                axes.AX = 0;    // Neutral X (0g)
                axes.AY = 0;    // Neutral Y (0g)
                axes.AZ = 255;  // Neutral Z (1g)
            }
            break;
            
        default:
            break;
    }
}
