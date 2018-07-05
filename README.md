# Classic+
Classic+ is a hardware emulator that replaces the Wii Classic Controller and Nunchuk.  
  
### Usage
Classic+ normally operates as a Classic Controller. Nunchuk mode can be enabled at any time by connecting the MD pin to GND. The device can be temporarily disabled by connecting the EN pin to GND. The SDA, SCL, and DET pins should be connected to a Wii Remote expansion port.  

Soldering the accelerometer IC is optional. Without it, Nunchuk mode will report neutral values.

Classic+ works with Wii Remotes and SNES Classic. NES Classic support is untested. Unofficial Wii Remotes may not be supported.

PCBs can be ordered here: https://oshpark.com/shared_projects/poSQyyw2
  
### Updating
Place `update.hex` from the Firmware folder on the root of an SD card or USB storage device. Run the Wii homebrew app with a Classic+ board connected to a Wii Remote and select "Update Classic+".

### Safe Mode
Safe mode can be enabled by holding the X and Y buttons when the Classic+ powers on. Safe mode only supports updating through the homebrew app. Buttons, joysticks, and calibration will not work.
  
### Programming
Flash the pre-compiled `firmware.hex` using MPLAB IPE with a PICkit 3.

### Compiling
Make a project in MPLAB X IDE for PIC16F18876 with the XC8 compiler. The Bootloader and Main Program must be compiled as separate projects and flashed onto the same device using the code offset and Preserve Program Memory features or by manually combining the compiled .hex files.
