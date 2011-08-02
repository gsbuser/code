//-----------------------------------------------------------------------------
//title: Acquisition Logic A/D Board Kernel Driver Checkout Program
//version: Linux 1.0
//date:  March 2004
//designer: Michael Wyrick                                                   
//programmer: Michael Wyrick                                                
//platform: Linux 2.4.x
//language: GCC 3.3.1
//module: BoardAL4108
//-----------------------------------------------------------------------------
//  Purpose: Library for the AL4108
//  Docs:                                  
//-----------------------------------------------------------------------------
// CVS:
// $Id: Dacs.cpp,v 1.1 2005/01/03 16:41:16 mwyrick Exp $
// $Log: Dacs.cpp,v $
// Revision 1.1  2005/01/03 16:41:16  mwyrick
// Check in of Working Version
//
// Revision 1.3  2004/12/23 14:51:10  mwyrick
// Multi Board seems to work.  Added Max5841 and Max504.  They are in but untested
//
// Revision 1.2  2004/12/22 19:27:13  sschulz
// added MAX5841 and MAX504 DACs
//
// Revision 1.1.1.1  2004/12/16 14:27:28  mwyrick
// ALLinux 2004
//
// Revision 1.3  2004/04/29 19:41:30  mwyrick
// Removed extra space in header
//
// Revision 1.2  2004/04/29 19:12:12  mwyrick
// Added  and  to Header
//
//-----------------------------------------------------------------------------
#include "Dacs.h"
#include "stdio.h"
#include "unistd.h"
#include "Exceptions.h"

//********************* MAX5306/5307 octal 12 bit D/A converter *************
//
//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
Max5306::Max5306(TReg *Data, TReg *Busy, TReg *Sel, TReg *Sel2, int SelVal, int SelVal2) {
  this->Data      = Data;
  this->Busy      = Busy;
  this->Select    = Sel;
  this->Select2   = Sel2;
  SelectValue     = SelVal;
  SelectValue2    = SelVal2;
  SettleTime      = 0;
  MaxValue        = 4095;
  MinValue        = 0;

  for(int i=0; i < 7; i++) {
    SettleTimes[i] = 80000;     // Default Setting Time
  }

  PowerUp();
}

//-----------------------------------------------------------------------------
// Deconstructor
//-----------------------------------------------------------------------------
Max5306::~Max5306() {
  PowerDown();
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void Max5306::PowerUp(void) {
  while(Busy->Get())
    ;

  Select->Set(SelectValue);
  Select2->Set(SelectValue2);
  Data->Set(0xFFFF);
  Busy->Set(1);

  while(Busy->Get())
    ;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void Max5306::PowerDown(void) {
  while(Busy->Get())
    ;

  Select->Set(SelectValue);
  Select2->Set(SelectValue2);
  Data->Set(0xFFF4);
  Busy->Set(1);

  while(Busy->Get())
    ;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void Max5306::SetDAC(unsigned char chan, unsigned short data) {
  if (chan > 7)
    throw new exDACInvalidChannel;
  
  if (Value[chan] != data) {  // Don't set if we are already set to data
    Value[chan] = data;

    Select->Set(SelectValue);
    Select2->Set(SelectValue2);

    while(Busy->Get())
     ;
    Data->Set( (unsigned short)((data & 0x0FFF) | ((chan + 2) << 12)) );
    Busy->Set(1);
    while(Busy->Get())
     ;
    Data->Set( (unsigned short)(0xE000 | (1 << (chan + 4))) );
    Busy->Set(1);
    while(Busy->Get())
     ;

    if (SettleTime < SettleTimes[chan])
      SettleTime = SettleTimes[chan];
  }
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
unsigned short Max5306::ReadDAC(unsigned char chan) {
  if (chan < 8)
    return Value[chan];
  else
    throw new exDACInvalidChannel;
}

//-----------------------------------------------------------------------------
// Wait for the Dacs To Settle 
//-----------------------------------------------------------------------------
void Max5306::WaitForSettle() {
  if (SettleTime > 0)
    usleep(SettleTime);
  SettleTime = 0;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void Max5306::SetSettleTime(unsigned char chan, int value) {
  if (chan <= 7)
    SettleTimes[chan] = value;
  else
    throw new exDACInvalidChannel;
}


//********************* MAX841MEUB quad 10 bit D/A converter *************
//
//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
Max5841::Max5841(II2C *ini2c, int address) {
  i2c          = ini2c;
  I2C_address  = address;
  SettleTime   = 0;
  MaxValue     = 4095;     /* DAC is programmed with 12 bit values, low 2 bits are ignored */
  MinValue     = 0;

  for(int i=0; i < 3; i++) {
    SettleTimes[i] = 50000;      // Default Setting Time
    Value[i] = -1;               // not initialized
  }

  PowerUp();
}

//-----------------------------------------------------------------------------
// Deconstructor
//-----------------------------------------------------------------------------
Max5841::~Max5841() {
  PowerDown();
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void Max5841::PowerUp(void) {
  i2c->Set(I2C_address);
  if (!i2c->send_cmd(I2C_CMD_START)) {
    i2c->send_cmd(I2C_CMD_STOP);
    throw exI2C("I2C DAC 5841 Adressing Failed");
  }

  // Select extended command mode
  i2c->Set(0xf0);
  if (!i2c->send_cmd(I2C_CMD_WRITE)) {
    i2c->send_cmd(I2C_CMD_STOP);
    throw exI2C("I2C DAC 5841 Write Failed");
  }

  // Power up all four DACs
  i2c->Set(0x3c);
  if (!i2c->send_cmd(I2C_CMD_STOP | I2C_CMD_WRITE)) {
    i2c->send_cmd(I2C_CMD_STOP);
    throw exI2C("I2C DAC 5841 Write Failed");
  }
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void Max5841::PowerDown(void) {
  i2c->Set(I2C_address);
  if (!i2c->send_cmd(I2C_CMD_START)) {
    i2c->send_cmd(I2C_CMD_STOP);
    throw exI2C("I2C DAC 5841 Adressing Failed");
  }

  // Select extended command mode
  i2c->Set(0xf0);
  if (!i2c->send_cmd(I2C_CMD_WRITE)) {
    i2c->send_cmd(I2C_CMD_STOP);
    throw exI2C("I2C DAC 5841 Write Failed");
  }

  // Power down all four DACs
  i2c->Set(0x3d);
  if (!i2c->send_cmd(I2C_CMD_STOP | I2C_CMD_WRITE)) {
    i2c->send_cmd(I2C_CMD_STOP);
    throw exI2C("I2C DAC 5841 Write Failed");
  }
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void Max5841::SetDAC(unsigned char chan, unsigned short data) {
  if (chan > 3)
    throw new exDACInvalidChannel;
  
  if (Value[chan] != data) {  // Don't set if we are already set to data
    Value[chan] = data;
  }

  i2c->Set(I2C_address);
  if (!i2c->send_cmd(I2C_CMD_START)) {
    i2c->send_cmd(I2C_CMD_STOP);
    throw exI2C("I2C DAC 5841 Adressing Failed");
  }

  // Write channel number and upper data nibble
  i2c->Set( (unsigned char) (((chan & 0x03) << 4) | ((data & 0x0fff) >> 8)));
  if (!i2c->send_cmd(I2C_CMD_WRITE)) {
    i2c->send_cmd(I2C_CMD_STOP);
    throw exI2C("I2C DAC 5841 Write Failed");
  }

  // Write lower 8 data bits
  i2c->Set( (unsigned char) (data & 0x00ff));
  if (!i2c->send_cmd(I2C_CMD_STOP | I2C_CMD_WRITE)) {
    i2c->send_cmd(I2C_CMD_STOP);
    throw exI2C("I2C DAC 5841 Write Failed");
  }

  if (SettleTime < SettleTimes[chan])
    SettleTime = SettleTimes[chan];
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
unsigned short Max5841::ReadDAC(unsigned char chan) {
  if (chan < 4)
    return Value[chan];
  else
    throw new exDACInvalidChannel;
}

//-----------------------------------------------------------------------------
// Wait for the Dacs To Settle 
//-----------------------------------------------------------------------------
void Max5841::WaitForSettle() {
  if (SettleTime > 0)
    usleep(SettleTime);
  SettleTime = 0;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void Max5841::SetSettleTime(unsigned char chan, int value) {
  if (chan <= 3)
    SettleTimes[chan] = value;
  else
    throw new exDACInvalidChannel;
}


//********************* MAX504 single channel 10 bit D/A converter *************
//
//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
Max504::Max504(TReg *Data, TReg *Busy, TReg *Sel, TReg *Sel2, int SelVal, int SelVal2) {
  this->Data      = Data;
  this->Busy      = Busy;
  this->Select    = Sel;
  this->Select2   = Sel2;
  SelectValue     = SelVal;
  SelectValue2    = SelVal2;
  SettleTime      = 0;
  MaxValue        = 4095;
  MinValue        = 0;

  SettleTimes[0]  = 80000;     // Default Setting Time

  PowerUp();
}

//-----------------------------------------------------------------------------
// Deconstructor
//-----------------------------------------------------------------------------
Max504::~Max504() {
  PowerDown();
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void Max504::PowerUp(void) {
    Value = -1;               // not initialized
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void Max504::PowerDown(void) {
    Value = -1;               // not initialized
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void Max504::SetDAC(unsigned char chan, unsigned short data) {
  if (chan > 0)
    throw new exDACInvalidChannel;
  
  if (Value != data) {  // Don't set if we are already set to data
    Value = data;

    Select->Set(SelectValue);
    Select2->Set(SelectValue2);

    while(Busy->Get())
     ;
    Data->Set( (unsigned short) (data & 0x0FFF) );
    Busy->Set(1);
    while(Busy->Get())
     ;

    if (SettleTime < SettleTimes[chan])
      SettleTime = SettleTimes[chan];
  }
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
unsigned short Max504::ReadDAC(unsigned char chan) {
  if (chan == 0)
    return Value;
  else
    throw new exDACInvalidChannel;
}

//-----------------------------------------------------------------------------
// Wait for the Dacs To Settle 
//-----------------------------------------------------------------------------
void Max504::WaitForSettle() {
  if (SettleTime > 0)
    usleep(SettleTime);
  SettleTime = 0;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void Max504::SetSettleTime(unsigned char chan, int value) {
  if (chan == 0)
    SettleTimes[chan] = value;
  else
    throw new exDACInvalidChannel;
}



