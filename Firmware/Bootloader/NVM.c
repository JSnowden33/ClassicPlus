/*
 * File:   NVM.c
 * Author: Jackson
 */

#include <xc.h>
#include "config.h"
#include "NVM.h"

void NVMunlock()
{
    INTCONbits.GIE = 0;
    NVMCON2 = 0x55;
    NVMCON2 = 0xAA;
    NVMCON1bits.WR = 1;
    NOP();
    NOP();
}

void PGMerase(u16 addr)
{
    addr = addr - (addr % 32);
    
    NVMCON1bits.NVMREGS = 0;
    
    NVMADRH = (addr & 0xFF00) >> 8;
    NVMADRL = addr & 0xFF;
    
    NVMCON1bits.FREE = 1;
    NVMCON1bits.WREN = 1;
    
    NVMunlock();
    NVMCON1bits.WREN = 0;
}

void PGMwrite(u16 addr, u16 *buf, u8 length)
{
    // Shorten length if it extends beyond the row containing the address
    if (length + (addr % 32) > 32) length = 32 - (addr % 32);

    NVMCON1bits.NVMREGS = 0;
    NVMCON1bits.WREN = 1;
    NVMCON1bits.LWLO = 1;
        
    NVMADRH = (addr & 0xFF00) >> 8;
        
    u8 i;
    for (i = 0; i < length; i++)
    {
        NVMADRL = (addr + i) & 0xFF;
        
        if (i < length - 1)
        {
            NVMDATH = (buf[i] & 0xFF00) >> 8;
            NVMDATL = buf[i] & 0xFF;
            
            NVMunlock();
        }
        else
        {
            NVMCON1bits.LWLO = 0;
            
            NVMDATH = (buf[i] & 0xFF00) >> 8;
            NVMDATL = buf[i] & 0xFF;
            
            NVMunlock();
        }
    }
    
    NVMCON1bits.WREN = 0;
}

void PGMread(u16 addr, u16 *buf, u16 length)
{
    if (length > 0)
    {
        NVMCON1bits.NVMREGS = 0;
    
        u8 i;
        for (i = 0; i < length; i++)
        {
            NVMADRH = ((addr + i) & 0xFF00) >> 8;
            NVMADRL = (addr + i) & 0xFF;
    
            NVMCON1bits.RD = 1;
            NOP();
            buf[i] = (NVMDATH << 8) | NVMDATL;
        }
    }
}
