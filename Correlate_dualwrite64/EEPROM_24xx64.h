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
// $Id: EEPROM_24xx64.h,v 1.1 2005/01/03 16:41:16 mwyrick Exp $
// $Log: EEPROM_24xx64.h,v $
// Revision 1.1  2005/01/03 16:41:16  mwyrick
// Check in of Working Version
//
// Revision 1.1.1.1  2004/12/16 14:27:28  mwyrick
// ALLinux 2004
//
//
//-----------------------------------------------------------------------------
#ifndef EEPROM_24xx64H
#define EEPROM_24xx64H

#include "II2C.h"

#define EEPROM_24xx64_SIZE 8192
#define EEPROM_24xx64_PAGE_SIZE 16
#define EEPROM_24xx64_WADDR 0xA0
#define EEPROM_24xx64_RADDR 0xA1

class EEPROM_24xx64 {
  private:
  II2C *i2c;

  public:
  int EEPROM_SIZE;
  unsigned char Data[EEPROM_24xx64_SIZE];
  EEPROM_24xx64(II2C *ini2c);
  void Read(void);
};

#endif
