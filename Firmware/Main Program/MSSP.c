/*
 * File:   MSSP.c
 * Author: Jackson Snowden
 */

#include <xc.h>
#include "config.h"
#include "crypto.h"
#include "expansion.h"
#include "camera.h"
#include "MSSP.h"

u8 I2Caddr;
u8 I2CaddrSet;

u8 I2Creg[512];
u8 I2CregAddr;
u8 I2CregAddrSet;

u16 I2Cstatus;

void I2CslaveInit(const u8 addr1, const u8 addr2)
{    
    SSP1CON1bits.SSPEN = 0;
    
    SSP1CON1 = 0b01010110;      // 7-bit address slave mode with start/stop interrupts disabled
    SSP1CON2 = 0b00000001;      // Clock stretching enabled
    SSP1CON3 = 0b01000000;      // Stop interrupt enabled, address holding disabled, data holding disabled
    SSP1ADD = (addr1 << 1);
    if (addr2) SSP1MSK = ~(addr1 ^ addr2) << 1; // Respond to both slave addresses
    else SSP1MSK = 0xFF;
    SSP1STAT = 0b00000000;
    
    SSP1CON1bits.SSPEN = 1;
}

void I2Coff()
{
    SSP1CON1bits.SSPEN = 0;
}

u8 I2CisEnabled()
{
    return SSP1CON1bits.SSPEN;
}

u8 I2CslaveRead(u16 addr)
{
    return I2Creg[addr & 0x1FF];
}

void I2CslaveReadMulti(u16 addr, u8 *buf, u8 length)
{
    if (length > 0)
    {
        u8 i;
        for (i = 0; i < length; i++)
        {
            buf[i] = I2Creg[(addr & 0x1FF) + i];
        }
    }
}

void I2CslaveWrite(u16 addr, u8 data)
{
    I2Creg[addr & 0x1FF] = data;
}

void I2CslaveWriteMulti(u16 addr, u8 *buf, u8 length)
{
    if (length > 0)
    {
        u8 i;
        for (i = 0; i < length; i++)
        {
            I2Creg[(addr & 0x1FF) + i] = buf[i];
        }
    }
}

void I2CslaveRelease()
{
    SSP1CON1bits.CKP = 1;
}

void I2CslaveHandle()
{
    u8 enc = ExpIsEncEnabled();
        
    if (SSP1STATbits.P) 
    {
        I2Caddr = 0;
        I2CaddrSet = 0;
        I2CregAddrSet = 0;
        
        // Clear pending overflow
        u8 dummy;
        if (SSP1STATbits.BF) dummy = SSP1BUF;
        SSP1CON1bits.SSPOV = 0;
    }
    else
    {
        u8 I2Cdata;
        I2Cstatus = (SSP1STAT & 0b00111100) | (SSP1CON2 & 0b01000000);
        
        // Accept write requests only if desired addresses match
        if (!I2CaddrSet || (I2Caddr == EXP_I2C_ADDR) || (I2Caddr == CAM_I2C_ADDR))
        {
            // Store received byte (not reading will cause NACK)
            if (SSP1STATbits.BF) I2Cdata = SSP1BUF;
            SSP1CON1bits.SSPOV = 0;
            
            switch (I2Cstatus)
            {
                case WRITE_ADDR_ACK: 
                    I2Caddr = I2Cdata >> 1;
                    I2CaddrSet = 1;
                    break;
                  
                case WRITE_DAT_ACK:
                    if(I2CregAddrSet)
                    {   
                        if (I2Caddr == EXP_I2C_ADDR)
                        {
                            // Send data to expansion controller emulator to check for commands
                            ExpCmdRcv(I2Cdata, I2CregAddr);
                            if (enc) I2Creg[I2CregAddr] = Decrypt(I2CregAddr, I2Cdata);
                            else I2Creg[I2CregAddr] = I2Cdata;
                        }
                        else if (I2Caddr == CAM_I2C_ADDR)
                        {
                            // Send data to camera emulator to check for commands
                            CamCmdRcv(I2Cdata, I2CregAddr);
                            I2Creg[I2CregAddr + 256] = I2Cdata;
                        }
                
                        I2CregAddr++;
                    }
                    else
                    {   
                        I2CregAddr = I2Cdata;
                        I2CregAddrSet = 1;
                    }
                    break;
                    
                default:
                    break;
            }
        }
            
        // Respond to all read requests
        switch (I2Cstatus)
        { 
            case READ_ADDR_ACK: 
                I2Caddr = I2Cdata >> 1;
            
            case READ_DAT_ACK:
                if (I2Caddr == EXP_I2C_ADDR)
                {
                    if (enc) SSP1BUF = Encrypt(I2CregAddr, I2Creg[I2CregAddr]);
                    else SSP1BUF = I2Creg[I2CregAddr];  
                }
                else if (I2Caddr == CAM_I2C_ADDR) SSP1BUF = I2Creg[I2CregAddr + 256];
                else SSP1BUF = 0xFF;
                    
                I2CregAddr++;
                break;
                
            default:
                break;
        }
    }
}

// - - - - - - - - - - //

void SPImasterInit(const u32 clk, const u8 CKP, const u8 CKE, const u8 SMP)
{
    SSP2CON1bits.SSPEN = 0;
    SSP2CON1 = 0b00001010;                  // SPI master mode with custom clock
    SSP2CON3 = 0b00010000;                  // Buffer overwrite enabled
    SSP2ADD = (_XTAL_FREQ / (4 * clk)) - 1; // Set CLK speed
    SSP2CON1bits.CKP = CKP & 0x01;          // CLK polarity (0 = idle low, 1 = idle high)
    SSP2STATbits.CKE = CKE & 0x01;          // SDO transmission edge (0 = idle-to-active CLK, 1 = active-to-idle CLK)
    SSP2STATbits.SMP = SMP & 0x01;          // SDI sampling time (0 = middle, 1 = end)
    SSP2CON1bits.SSPEN = 1;
}

void SPIoff()
{
    SSP2CON1bits.SSPEN = 0;
}

u8 SPIisEnabled()
{
    return SSP2CON1bits.SSPEN;
}

u8 SPIwait()
{
    // Wait until buffer is full
    u8 timeout = 0;
    while (!SSP2STATbits.BF && (timeout < 255)) timeout++;
    if (timeout < 255) return 0;
    return 1;
}

void SPIwrite(u8 data)
{
    SSP2BUF = data;
}

u8 SPIread()
{
    return SSP2BUF;
}

u8 SPItransfer(u8 data)
{
    SPIwrite(data);                     // Begin transfer
    if (!SPIwait()) return SPIread();   // Read slave data
    return 0;
}
