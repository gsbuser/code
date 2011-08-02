//-----------------------------------------------------------------------------
//title: Acquisition Logic A/D Board Kernel Driver Checkout Program
//version: Linux 1.0
//date:  Oct 2004
//designer: Michael Wyrick                                                   
//programmer: Michael Wyrick                                                
//platform: Linux 2.4.x
//language: GCC 3.3.1
//module: II2C
//-----------------------------------------------------------------------------
//  Purpose: Library Header
//  Docs:                                  
//-----------------------------------------------------------------------------
// CVS:
// $Id: I2C_4108.h,v 1.1 2005/01/03 16:41:16 mwyrick Exp $
// $Log: I2C_4108.h,v $
// Revision 1.1  2005/01/03 16:41:16  mwyrick
// Check in of Working Version
//
// Revision 1.1.1.1  2004/12/16 14:27:27  mwyrick
// ALLinux 2004
//
//
//-----------------------------------------------------------------------------
#ifndef I2C_4108H
#define I2C_4108H

#include "reg.h"
#include "II2C.h"

class I2C_4108 : public II2C {
  private:
  TReg *CmdReg;
  TReg *DataReg;

  public:
  I2C_4108(TReg*, TReg*);
  virtual unsigned char Get();
  virtual void Set(unsigned char);
  virtual bool send_cmd(unsigned char);
};

#endif
