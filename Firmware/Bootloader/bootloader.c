/*
 * File:   bootloader.c
 * Author: Jackson
 */

#include <xc.h>
#include "pin_defs.h"
#include "config.h"
#include "NVM.h"
#include "bootloader.h"

u8 I2CregAddr;
u8 I2CregAddrFlag;
u16 I2Cstatus;

u8 pgmStatus;
u16 pgmData[32];
u8 pgmDataPos;
u8 pgmDataLength;
u8 pgmDataFlag;
u16 pgmAddr;
u8 pgmAddrFlag;

u8 checksum;

u8 exitFlag;

void I2CslaveInit(const u8 addr)
{    
    SSP1CON1bits.SSPEN = 0;
    SSP1CON1 = 0b01010110;      // 7-bit address slave mode with start/stop interrupts disabled
    SSP1CON2 = 0b00000001;      // Clock stretching enabled
    SSP1CON3 = 0b00000000;      // Address holding disabled, data holding disabled
    SSP1ADD = (addr << 1);
    SSP1MSK = 0xFF;
    SSP1STAT = 0b00000000;
    SSP1CON1bits.SSPEN = 1;
}

void I2Coff()
{
    SSP1CON1bits.SSPEN = 0;
}

void CMDreceive(u8 cmd)
{
    // Command reception
    if (!pgmStatus)
    {
        pgmStatus = cmd;
        switch (pgmStatus)
        {
            case PGM_ERASE:
            case PGM_WRITE:
            case PGM_READ:
                pgmDataPos = 0;
                pgmDataFlag = 0;
                pgmAddrFlag = 0;
                checksum = 0;
                pgmStatus++;
                break;
                                    
            case PGM_DIS:
                pgmStatus = 0;
                exitFlag = 1;
                break;
                                    
            default:
                pgmStatus = 0;
                break;
        }
    }
    else
    {
        switch (pgmStatus)
        {
            case (PGM_WRITE + 2):
            case (PGM_READ + 2):
                checksum += cmd;
                    
                // Byte length -> Word length
                pgmDataLength = cmd / 2;
                pgmStatus++;
                    
                if (pgmStatus == PGM_READ + 3)
                {
                    if (pgmAddr >= APP_ADDR) PGMread(pgmAddr, pgmData, pgmDataLength);
                }
                break;
                            
            case (PGM_ERASE + 1):
            case (PGM_WRITE + 1):
            case (PGM_READ + 1):
                if (!pgmAddrFlag) 
                {
                    // Receive address low byte
                    pgmAddr = cmd;
                    pgmAddrFlag = 1;
                }
                else 
                {
                    // Receive address high byte
                    pgmAddr = (pgmAddr & 0xFF) | (cmd << 8);
                    checksum += pgmAddr & 0xFF;
                    checksum += (pgmAddr & 0xFF00) >> 8;
                        
                    // Byte address -> Word address
                    pgmAddr = pgmAddr / 2;
                    pgmAddrFlag = 0;
                    pgmStatus++;
                }
                break;
                                
            default:
                break;  
        }   
    }  
}

void CMDexec()
{
    // Execute time-consuming commands
    switch(pgmStatus)
    {
        case (PGM_ERASE + 2):
            if (pgmAddr >= APP_ADDR) PGMerase(pgmAddr);
            pgmAddr = 0;
            pgmStatus = 0;
            break;
                    
        case (PGM_WRITE + 5):
            PGMwrite(pgmAddr, pgmData, pgmDataLength);
            pgmAddr = 0;
            pgmDataLength = 0;
            pgmDataPos = 0;
            checksum = 0;
            pgmStatus = 0;
            break;
                    
        default:
            break;
    }
}

void PGMreceive(u8 data)
{
    // Program data reception
    switch (pgmStatus)
    {
        case (PGM_WRITE + 5):                              
            if ((pgmAddr >= APP_ADDR) && (data == PGM_WRITE)) pgmStatus++;
            else
            {
                pgmAddr = 0;
                pgmDataLength = 0;
                pgmDataPos = 0;
                checksum = 0;
                pgmStatus = 0;
            }
            break;
								
        case (PGM_WRITE + 3):
            if (!pgmDataFlag)
            {
                // Write data low byte
                pgmData[pgmDataPos] = data;
                checksum += data;
                pgmDataFlag = 1;
            }
            else
            {
                // Write data high byte
                pgmData[pgmDataPos] = (pgmData[pgmDataPos] & 0xFF) | (data << 8);
                checksum += data;
                pgmDataFlag = 0;
                pgmDataPos++;
            }
                                
            if (pgmDataPos >= pgmDataLength)
            {
                // Calculate checksum two's complement
                checksum = ~checksum + 1;
                pgmStatus++;
            }
            break;
                
        default:
            break;
    }
}

void PGMsend()
{
    // Program data transmission
    switch (pgmStatus)
	{
		case (PGM_WRITE + 4):
            SSP1BUF = checksum;
            pgmStatus++;
            break;
                                
        case (PGM_READ + 4):
            SSP1BUF = checksum;
            pgmAddr = 0;
            pgmDataLength = 0;
            pgmDataPos = 0;
            checksum = 0;
            pgmStatus = 0;
			break;
								
		case (PGM_READ + 3):
			if (!pgmDataFlag)
			{
				// Send data low byte
				SSP1BUF = pgmData[pgmDataPos] & 0xFF;
				checksum += pgmData[pgmDataPos] & 0xFF;
				pgmDataFlag = 1;
            }
			else
			{
				// Send data high byte
				SSP1BUF = (pgmData[pgmDataPos] & 0xFF00) >> 8;
				checksum += (pgmData[pgmDataPos] & 0xFF00) >> 8;
				pgmDataFlag = 0;
				pgmDataPos++;
			}
						
			if (pgmDataPos >= pgmDataLength)
			{
				// Calculate checksum two's complement
				checksum = ~checksum + 1;
				pgmStatus++;
			}
			break;
								
		default:
			SSP1BUF = 0xFF;
			break;
	}
}

void BeginBootloader() @ 0x100
{
    // Disable interrupts
    INTCONbits.GIE = 0;
    
    // INTOSC = 32 MHz
    OSCCON1bits.NDIV = 0;
    
    const u8 classicID[6] = { 0x00, 0x00, 0xA4, 0x20, 0x01, 0x01 };
    u8 IDpos;
    
    exitFlag = 0;
    
    // Enable communication with Wii Remote
    I2CslaveInit(EXP_I2C_ADDR);
    DETECT = 1;
    
    while (1)
    {
        if (SSP1IF)
        {       
            // Store received byte and check bus status
            u8 I2Cdata;
            if (SSP1STATbits.BF) I2Cdata = SSP1BUF;
        
            I2Cstatus = (SSP1STAT & 0b00111100) | (SSP1CON2 & 0b01000000);
        
            switch (I2Cstatus)
            {
                case WRITE_ADDR_ACK: 
                    I2CregAddrFlag = 1;
                    break;
                   
                case WRITE_DAT_ACK:
                    if (!I2CregAddrFlag)
                    {
                        switch (I2CregAddr)
                        {
                            case I2C_REG_CMD:
                                CMDreceive(I2Cdata);
                                break;
                                
                            case I2C_REG_DAT:
                                PGMreceive(I2Cdata);
                                
                            default:
                                break;
                        }
                    }
                    else
                    {   
                        // Set received byte as register address
                        I2CregAddr = I2Cdata;
                        I2CregAddrFlag = 0;
                    }
                    break;
                
                case READ_ADDR_ACK:
                case READ_DAT_ACK:
                    switch (I2CregAddr)
                    {
                        case I2C_REG_DAT:
                            PGMsend();
                            break;
                            
                        case I2C_REG_CID:
                            SSP1BUF = CID;
                            break;
                            
                        case I2C_REG_ID:
                            SSP1BUF = classicID[IDpos];
                            IDpos++;
                            if (IDpos >= 6) IDpos = 0;
                            break;
                            
                        case I2C_REG_CAL:
                        case I2C_REG_CAL + 0x10:
                            SSP1BUF = 0;
                            break;
                            
                        default:
                            SSP1BUF = 0xFF;
                            break;
                    }
					break;
   
                default:
                    break;
            }

            SSP1IF = 0;             // Clear interrupt flag
            SSP1CON1bits.CKP = 1;   // Release SCL line
        }
        
        CMDexec();
            
        if (exitFlag) ExitBootloader();
    }
}

void ExitBootloader()
{
    DETECT = 0;
    I2Coff();
    exitFlag = 0;
    
    // Go to main application
    #asm
        GOTO APP_ADDR;
    #endasm
}
