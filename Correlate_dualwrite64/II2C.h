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
// $Id: II2C.h,v 1.1 2005/01/03 16:41:17 mwyrick Exp $
// $Log: II2C.h,v $
// Revision 1.1  2005/01/03 16:41:17  mwyrick
// Check in of Working Version
//
// Revision 1.1.1.1  2004/12/16 14:27:27  mwyrick
// ALLinux 2004
//
//
//-----------------------------------------------------------------------------
#ifndef II2CH
#define II2CH

#define I2C_CMD_WAS_ACK  0x20
#define I2C_CMD_BUSY     0x10
#define I2C_CMD_EXECUTE  0x10
#define I2C_CMD_START    0x08
#define I2C_CMD_STOP     0x04
#define I2C_CMD_SEND_ACK 0x02
#define I2C_CMD_READ     0x01
#define I2C_CMD_WRITE    0x00

class II2C {
  public:
  virtual unsigned char Get() = 0;
  virtual void Set(unsigned char) = 0;
  virtual bool send_cmd(unsigned char) = 0;
};

#endif
