
//-----------------------------------------------------------------------------
//title: Acquisition Logic A/D Board Kernel Driver Checkout Program
//version: Linux 1.0
//date:  Oct 2004
//designer: Michael Wyrick                                                   
//programmer: Michael Wyrick                                                
//platform: Linux 2.4.x
//language: GCC 3.3.1
//module: I2C_212
//-----------------------------------------------------------------------------
//  Purpose: Library Header
//  Docs:                                  
//-----------------------------------------------------------------------------
// CVS:
// $Id: I2C_4108.cpp,v 1.1 2005/01/03 16:41:16 mwyrick Exp $
// $Log: I2C_4108.cpp,v $
// Revision 1.1  2005/01/03 16:41:16  mwyrick
// Check in of Working Version
//
// Revision 1.1.1.1  2004/12/16 14:27:27  mwyrick
// ALLinux 2004
//
//
//-----------------------------------------------------------------------------
#include <unistd.h>
#include "I2C_4108.h"
#include "Exceptions.h"

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
I2C_4108::I2C_4108(TReg *inCmd, TReg *inData) {
  CmdReg = inCmd;
  DataReg = inData;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
unsigned char I2C_4108::Get(void) {
  return DataReg->Get();
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
void I2C_4108::Set(unsigned char b) {
  DataReg->Set(b);
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
bool I2C_4108::send_cmd(unsigned char command) {
  unsigned char v = I2C_CMD_BUSY;

  while((v & I2C_CMD_BUSY) == I2C_CMD_BUSY)
      v = (unsigned char)CmdReg->Get();

  CmdReg->Set( (unsigned char)(command | I2C_CMD_EXECUTE));

  v = I2C_CMD_BUSY;

  while((v & I2C_CMD_BUSY) == I2C_CMD_BUSY)
      v = (unsigned char)CmdReg->Get();
  

  return ((v & I2C_CMD_WAS_ACK) == I2C_CMD_WAS_ACK);
}

