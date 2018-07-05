/*
 * File:   NVM.c
 * Author: Jackson Snowden
 */

#include <xc.h>
#include "config.h"
#include "NVM.h"

void NVMunlock()
{
    NVMCON2 = 0x55;
    NVMCON2 = 0xAA;
    NVMCON1bits.WR = 1;
    NOP();
    NOP();
}

void EEwrite(u8 addr, u8 *buf, u8 length)
{
    if (length > 0)
    {
        u8 intStat = INTCONbits.GIE;
        INTCONbits.GIE = 0;
        
        NVMCON1bits.NVMREGS = 1;
        
        u8 i;
        for (i = 0; i < length; i++)
        {
            NVMADRH = 0xF0;
            NVMADRL = addr + i;
        
            NVMDATL = buf[i];
    
            NVMCON1bits.WREN = 1;
            NVMunlock();
            while(NVMCON1bits.WR);
            NVMCON1bits.WREN = 0;
        }
        
        if (intStat) INTCONbits.GIE = 1;
    }
}

void EEread(u8 addr, u8 *buf, u8 length)
{
    if (length > 0)
    {
        NVMCON1bits.NVMREGS = 1;
        
        u8 i;
        for (i = 0; i < length; i++)
        {
            NVMADRH = 0xF0;
            NVMADRL = addr + i;
    
            NVMCON1bits.RD = 1;
            buf[i] = NVMDATL;
        }
    }
}