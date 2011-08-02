//-----------------------------------------------------------------------------
//title: Acquisition Logic A/D Board Kernel Driver Checkout Program
//version: Linux 1.0
//date:  Oct 2004
//designer: Michael Wyrick                                                   
//programmer: Michael Wyrick                                                
//platform: Linux 2.4.x
//language: GCC 3.3.1
//module: Exceptions
//-----------------------------------------------------------------------------
//  Purpose: Library Header
//  Docs:                                  
//-----------------------------------------------------------------------------
// CVS:
// $Id: Exceptions.h,v 1.1 2005/01/03 16:41:16 mwyrick Exp $
// $Log: Exceptions.h,v $
// Revision 1.1  2005/01/03 16:41:16  mwyrick
// Check in of Working Version
//
// Revision 1.1.1.1  2004/12/16 14:27:27  mwyrick
// ALLinux 2004
//
//
//-----------------------------------------------------------------------------
#ifndef ExceptionsH
#define ExceptionsH

class exHardware {
  public:
  char *s;
  exHardware() {s = "Unknown Hardware Exception";} ;
  exHardware(char *str) {s = str;};
};

class exPLLLock : public exHardware {
  public:
  exPLLLock() : exHardware("PLL did not Lock") {};
  exPLLLock(char *str) : exHardware(str) {};
};

class exI2C : public exHardware {
  public:
  exI2C() : exHardware("Unknown I2C Exception") {};
  exI2C(char *str) : exHardware(str) {};
};                             

class exRegister : public exHardware {
  public:
  exRegister() : exHardware("Unknown Register Exception") {};
  exRegister(char *str) : exHardware(str) {};
};

class exEEPROM_24xx64 : public exHardware {
  public:
  exEEPROM_24xx64() : exHardware("Unknown EEPROM Exception") {};
  exEEPROM_24xx64(char *str) : exHardware(str) {};
};

class exDACError : public exHardware {
  public:
  exDACError() : exHardware("Unknown DAC Error") {};
  exDACError(char *str) : exHardware(str) {};
};

class exDACInvalidChannel : public exDACError {
  public:
  exDACInvalidChannel() : exDACError("Invalid DAC Channel") {};
};

#endif
