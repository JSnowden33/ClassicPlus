/*
 * File:   main.c
 * Author: Jackson Snowden
 */

// PIC16F18876 Configuration Bit Settings

// 'C' source line config statements

// CONFIG1
#pragma config FEXTOSC = OFF    // External Oscillator mode selection bits (Oscillator not enabled)
#pragma config RSTOSC = HFINT32 // Power-up default value for COSC bits (HFINTOSC with OSCFRQ = 32 MHz and CDIV = 1:1)
#pragma config CLKOUTEN = OFF   // Clock Out Enable bit (CLKOUT function is disabled; i/o or oscillator function on OSC2)
#pragma config CSWEN = ON       // Clock Switch Enable bit (Writing to NOSC and NDIV is allowed)
#pragma config FCMEN = ON       // Fail-Safe Clock Monitor Enable bit (FSCM timer enabled)

// CONFIG2
#pragma config MCLRE = ON       // Master Clear Enable bit (MCLR pin is Master Clear function)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config LPBOREN = OFF    // Low-Power BOR enable bit (ULPBOR disabled)
#pragma config BOREN = ON       // Brown-out reset enable bits (Brown-out Reset Enabled, SBOREN bit is ignored)
#pragma config BORV = LO        // Brown-out Reset Voltage Selection (Brown-out Reset Voltage (VBOR) set to 1.9V on LF, and 2.45V on F Devices)
#pragma config ZCD = OFF        // Zero-cross detect disable (Zero-cross detect circuit is disabled at POR.)
#pragma config PPS1WAY = OFF    // Peripheral Pin Select one-way control (The PPSLOCK bit can be set and cleared repeatedly by software)
#pragma config STVREN = ON      // Stack Overflow/Underflow Reset Enable bit (Stack Overflow or Underflow will cause a reset)

// CONFIG3
#pragma config WDTCPS = WDTCPS_31// WDT Period Select bits (Divider ratio 1:65536; software control of WDTPS)
#pragma config WDTE = OFF       // WDT operating mode (WDT Disabled, SWDTEN is ignored)
#pragma config WDTCWS = WDTCWS_7// WDT Window Select bits (window always open (100%); software control; keyed access not required)
#pragma config WDTCCS = LFINTOSC// WDT input clock selector (WDT reference clock is the 31.0kHz LFINTOSC output)

// CONFIG4
#pragma config WRT = OFF        // UserNVM self-write protection bits (Write protection off)
#pragma config SCANE = not_available// Scanner Enable bit (Scanner module is not available for use)
#pragma config LVP = ON         // Low Voltage Programming Enable bit (Low Voltage programming enabled. MCLR/Vpp pin function is MCLR.)

// CONFIG5
#pragma config CP = OFF         // UserNVM Program memory code protection bit (Program Memory code protection disabled)
#pragma config CPD = OFF        // DataNVM code protection bit (Data EEPROM code protection disabled)

#include <xc.h>
#include "config.h"
#include "pin_defs.h"
#include "input.h"
#include "ADC.h"
#include "MSSP.h"
#include "expansion.h"

u8 mode;
u8 newMode;

void PPSunlock()
{
    PPSLOCK = 0x55;
    PPSLOCK = 0xAA;
    PPSLOCKbits.PPSLOCKED = 0;
}

void PPSlock()
{
    PPSLOCK = 0x55;
    PPSLOCK = 0xAA;
    PPSLOCKbits.PPSLOCKED = 1;
}

void PICinit()
{
    // INTOSC = 8 MHz
    OSCCON1bits.NDIV = 2;
    
    INTCONbits.GIE = 0;
    PPSunlock();
    
    // I2C pin mapping
    SSP1DATPPS = PPS_RB3;   // SDA input on RB3
    RB3PPS = PPS_SDA1;      // SDA output on RB3
    SSP1CLKPPS = PPS_RB4;   // SCL input on RB4
    RB4PPS = PPS_SCL1;      // SCL output on RB4
    
    // SPI pin mapping
    RB0PPS = PPS_SDO2;      // SDO on RB0
    SSP2DATPPS = PPS_RD7;   // SDI on RD7
    RB1PPS = PPS_SCK2;      // SCK output on RB1
    
    PPSlock();
    
    // Enable MSSP1 interrupt
    SSP1IF = 0;
    PIE3 = 0b00000001;
    INTCON = 0b11000000;  
    
    // Pull-ups on button inputs
    WPUA = 0b11110000;    
    WPUB = 0b00000000;
    WPUC = 0b10000000;  
    WPUD = 0b01111111;  
    WPUE = 0b00000111;  
    
    // Analog pins: RC1, RC2, RC3, RC4, RC5, RC6
    ANSELA = 0b00000000;
    ANSELB = 0b00000000;
    ANSELC = 0b01111110;
    ANSELD = 0b00000000;
    ANSELE = 0b00000000;
    
    // Output pins: RB0, RB1, RB2, RB5
    TRISA = 0b11111111;   
    TRISB = 0b11011000;
    TRISC = 0b11111111;
    TRISD = 0b11111111;
    TRISE = 0b11111111;
    
    // Initial output pin states
    DETECT = 0;
    CS = 1;
}

void BeginBootloader()
{
    // Go to bootloader address
    #asm
        GOTO BOOT_ADDR;
    #endasm
}

void interrupt ISR()
{   
    if (SSP1IF)
    {       
        I2CslaveHandle();
        I2CslaveRelease();
        
        SSP1IF = 0;
    }
}

void main()
{
    PICinit();
    InputInit();
    
    mode = 0xFF;
    newMode = 0;
    
    while(1)
    {
        // Poll mode/enable pins
        if (ENABLE)
        {
            if (MODE) newMode = MODE_CLASSIC;
            else newMode = MODE_NUNCHUK;
        }
        else newMode = MODE_OFF;
            
        // Update mode
        if (newMode != mode) 
        {
            ExpSetMode(newMode);
            mode = newMode;
        }
        
        if (ExpIsEnabled())
        {
            InputGetButtons(mode);  // Get current button states
            InputGetAxes(mode);     // Get current axis states
            ExpUpdate();            // Send data to Wii Remote
            
            // Execute special commands, go to bootloader if triggered
            if (ExpCmdExec()) BeginBootloader();
        }
    }
}
