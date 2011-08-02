//-----------------------------------------------------------------------------
//title: Acquisition Logic A/D Board Kernel Driver Checkout Program
//version: Linux 1.0
//date:  March 2004
//designer: Michael Wyrick                                                   
//programmer: Michael Wyrick                                                
//platform: Linux 2.4.x
//language: GCC 3.3.1
//module: Board4108
//-----------------------------------------------------------------------------
//  Purpose: Library Header
//  Docs:                                  
//-----------------------------------------------------------------------------
// CVS:
// $Id: reg.h,v 1.1 2005/01/03 16:41:17 mwyrick Exp $
// $Log: reg.h,v $
// Revision 1.1  2005/01/03 16:41:17  mwyrick
// Check in of Working Version
//
// Revision 1.1.1.1  2004/12/16 14:27:27  mwyrick
// ALLinux 2004
//
// Revision 1.2  2004/04/29 19:12:14  mwyrick
// Added  and  to Header
//
//
//-----------------------------------------------------------------------------
#ifndef regH
#define regH

enum eRegSize { eREGSIZE_BYTE, 
                eREGSIZE_WORD, 
                eREGSIZE_LONG,
                eREGSIZE_RBW_BYTE, 
                eREGSIZE_RBW_WORD, 
                eREGSIZE_RBW_LONG };

class TReg {
  unsigned char *Address;
  eRegSize Size;
  int Mask;
  int Shift;
  int Value;
    
public:
  TReg(unsigned char *Addr, eRegSize Siz, int Msk, int Sh);  
  void Set(unsigned int value);
  int Get();  
};

class TRegByte : public TReg {
  public:
  TRegByte(unsigned char *Addr) : TReg(Addr, eREGSIZE_BYTE, 0xFF, 0) {}
};

class TRegWord : public TReg {
  public:
  TRegWord(unsigned char *Addr) : TReg(Addr, eREGSIZE_WORD, 0xFFFF, 0) {}
};

class TRegLong : public TReg {
  public:
  TRegLong(unsigned char *Addr) : TReg(Addr, eREGSIZE_LONG, 0xFFFFFFFF, 0) {}
};

#endif
