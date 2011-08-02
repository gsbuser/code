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
//
//Dec 22, 04: SWS, added definitions for MAX5841 and Max 504 dacs
//
//-----------------------------------------------------------------------------
#ifndef dacsh
#define dacsh

#include "reg.h"
#include "Exceptions.h"
#include "II2C.h"

class IDAC {
  public:
    int SettleTime;
    int MaxValue;
    int MinValue;

    virtual void PowerUp(void) = 0;
    virtual void PowerDown(void) = 0;
    virtual void SetDAC(unsigned char chan, unsigned short data) = 0;
    virtual unsigned short ReadDAC(unsigned char chan) = 0;
    
    virtual void WaitForSettle() = 0;
    virtual void SetSettleTime(unsigned char chan, int value) = 0;
};

class Max5306 : public IDAC {
  private:
    TReg *Data;
    TReg *Busy;
    TReg *Select;
    TReg *Select2;
    int SelectValue;
    int SelectValue2;
    int SettleTimes[8];
    int SettleTime;
    int Value[8];
    
  public:
    Max5306(TReg *Data, TReg *Busy, TReg *Sel, TReg *Sel2, int SelVal, int SelVal2);   // Constructor
    virtual ~Max5306();
    
    void PowerUp(void);
    void PowerDown(void);
    void SetDAC(unsigned char chan, unsigned short data);
    unsigned short ReadDAC(unsigned char chan);
    
    void WaitForSettle();
    void SetSettleTime(unsigned char chan, int value); 
};

class Max5841 : public IDAC {
  private:
    II2C *i2c;
    int I2C_address;
    int SettleTimes[4];
    int SettleTime;
    int Value[4];
    
  public:
    Max5841(II2C *ini2c, int address);   // Constructor
    virtual ~Max5841();
    
    void PowerUp(void);
    void PowerDown(void);
    void SetDAC(unsigned char chan, unsigned short data);
    unsigned short ReadDAC(unsigned char chan);
    
    void WaitForSettle();
    void SetSettleTime(unsigned char chan, int value); 
};

class Max504 : public IDAC {
  private:
    TReg *Data;
    TReg *Busy;
    TReg *Select;
    TReg *Select2;
    int SelectValue;
    int SelectValue2;
int SettleTimes[1];
    int SettleTime;
    int Value;
    
  public:
    Max504(TReg *Data, TReg *Busy, TReg *Sel, TReg *Sel2, int SelVal, int SelVal2);   // Constructor
    virtual ~Max504();
    
    void PowerUp(void);
    void PowerDown(void);
    void SetDAC(unsigned char chan, unsigned short data);
    unsigned short ReadDAC(unsigned char chan);
    
    void WaitForSettle();
    void SetSettleTime(unsigned char chan, int value); 
};

#endif
