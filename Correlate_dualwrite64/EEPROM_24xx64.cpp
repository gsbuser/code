//-----------------------------------------------------------------------------
//title: Acquisition Logic A/D Board Kernel Driver Checkout Program
//version: Linux 1.0
//date:  Oct 2004
//designer: Michael Wyrick                                                   
//programmer: Michael Wyrick                                                
//platform: Linux 2.4.x
//language: GCC 3.3.1
//module: EEPROM_24xx64
//-----------------------------------------------------------------------------
//  Purpose: Library for the AL4108
//  Docs:                                  
//-----------------------------------------------------------------------------
// CVS:
// $Id: EEPROM_24xx64.cpp,v 1.1 2005/01/03 16:41:16 mwyrick Exp $
// $Log: EEPROM_24xx64.cpp,v $
// Revision 1.1  2005/01/03 16:41:16  mwyrick
// Check in of Working Version
//
// Revision 1.1.1.1  2004/12/16 14:27:28  mwyrick
// ALLinux 2004
//
//
//-----------------------------------------------------------------------------
#include <stdio.h>
#include "EEPROM_24xx64.h"
#include "Exceptions.h"

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
EEPROM_24xx64::EEPROM_24xx64(II2C *ini2c) {
  EEPROM_SIZE = EEPROM_24xx64_SIZE;
  i2c = ini2c;
  Read();
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
void EEPROM_24xx64::Read(void) {
  // Setup for Write
  i2c->Set(EEPROM_24xx64_WADDR);
  if (!i2c->send_cmd(I2C_CMD_START)) {
    i2c->send_cmd(I2C_CMD_STOP);
    throw exEEPROM_24xx64("EEPROM Read Failed");
  }

  // High Byte of Address
  i2c->Set(0);
  if (!i2c->send_cmd(I2C_CMD_WRITE)) {
    i2c->send_cmd(I2C_CMD_STOP);
    throw exEEPROM_24xx64("EEPROM Write Failed");
  }

  // Low Byte of Address
  i2c->Set(0);
  if (!i2c->send_cmd(I2C_CMD_WRITE)) {
    i2c->send_cmd(I2C_CMD_STOP);
    throw exEEPROM_24xx64("EEPROM Write Failed");
  }

  // Setup for Read
  i2c->Set(EEPROM_24xx64_RADDR);
  if (!i2c->send_cmd(I2C_CMD_START | I2C_CMD_WRITE)) {
    i2c->send_cmd(I2C_CMD_STOP);
    throw exEEPROM_24xx64("EEPROM Read Failed");
  }

  for (int address=0; address < EEPROM_SIZE -1; address++) {
    i2c->send_cmd(I2C_CMD_SEND_ACK | I2C_CMD_READ);
    Data[address] = i2c->Get();
  }

  i2c->send_cmd(I2C_CMD_STOP | I2C_CMD_READ);
  Data[EEPROM_SIZE - 1] = i2c->Get();
}





