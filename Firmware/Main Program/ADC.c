/*
 * File:   ADC.c
 * Author: Jackson Snowden
 */

#include <xc.h>
#include "config.h"
#include "ADC.h"

void ADCinit()
{
    ADCON0 = 0b00000100;    // ADGO cleared after each conversion, ADC clock supplied by FOSC, Right-justified result
    ADCON1 = 0b00000000;    // One conversion for each trigger
    ADCON2 = 0b00000000;    // ADC Mode 0 (Basic Mode)
    ADREF = 0b00000000;     // VREF on VSS and VDD 
    ADCLK = 0b00000111;     // ADC running at FOSC/16
    ADCON0bits.ADON = 1;    // Turn on ADC module
}

u16 ADCread(u8 channel)
{
    u16 result;
    
    ADPRE = 0x00;           // No precharge time
    ADACQ = 255;            // Maximum acquisition time (255 TAD cycles)
    ADPCH = channel & 0x3F; // Select ADC channel
    
    ADCON0bits.ADGO = 1;    // Begin conversion
    while(ADCON0bits.ADGO); // Wait for conversion
    
    result = (ADRESH << 8) | ADRESL;
    
    return result;          // Return 10-bit result
}
