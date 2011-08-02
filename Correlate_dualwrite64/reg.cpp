//-----------------------------------------------------------------------------
//title: Acquisition Logic A/D Board Kernel Driver Checkout Program
//version: Linux 1.0
//date:  March 2004
//designer: Michael Wyrick                                                   
//programmer: Michael Wyrick                                                
//platform: Linux 2.4.x
//language: GCC 3.3.1
//module: Board212
//-----------------------------------------------------------------------------
//  Purpose: Library Header
//  Docs:                                  
//-----------------------------------------------------------------------------
// CVS:
// $Id: reg.cpp,v 1.1 2005/01/03 16:41:17 mwyrick Exp $
// $Log: reg.cpp,v $
// Revision 1.1  2005/01/03 16:41:17  mwyrick
// Check in of Working Version
//
// Revision 1.1.1.1  2004/12/16 14:27:27  mwyrick
// ALLinux 2004
//
// Revision 1.4  2004/04/29 19:51:15  mwyrick
// Added Comments above function defs
//
// Revision 1.3  2004/04/29 19:48:02  mwyrick
// Removed Extra line in header
// Testing Multi Line Comment
//
// Revision 1.2  2004/04/29 19:12:13  mwyrick
// Added  and  to Header
//
//-----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <string.h>

#include "reg.h"
#include "Exceptions.h"

typedef unsigned char byte;
typedef unsigned short ushort;
typedef unsigned int uint;

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
TReg::TReg(unsigned char *Addr, eRegSize Siz, int Msk, int Sh)
{
  Address = Addr;
  Size    = Siz;
  Mask    = Msk;
  Shift   = Sh;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void TReg::Set(unsigned int value)
{
  unsigned int temp;
  Value = value;
  if ((value & ~(Mask >> Shift)) != 0) {
    char s[255];
    sprintf(s, "Attempt to set a TRegister to a value outside it's size range, %08X=%08X",
            Address, value);
    throw exRegister(s);
  }

  switch (Size) {
    case eREGSIZE_BYTE:
      *(byte*)Address = (byte)(value & 0xFF);
      break;
    case eREGSIZE_RBW_BYTE:
      temp = *(byte*)Address;
      temp &= ~Mask;
      temp |= (value << Shift) & Mask;
      *(byte*)Address = (byte)temp;
      break;
    case eREGSIZE_WORD:
      *(ushort*)Address = (ushort)(value & 0xFFFF);
      break;
    case eREGSIZE_RBW_WORD:
      temp = *(ushort*)Address;
      temp &= ~Mask;
      temp |= (value << Shift) & Mask;
      *(ushort*)Address = (ushort)temp;
      break;
    case eREGSIZE_LONG:
      *(uint*)Address = (uint)(value & 0xFFFFFFFF);
      break;
    case eREGSIZE_RBW_LONG:
      temp = *(uint*)Address;
      temp &= ~Mask;
      temp |= (value << Shift) & Mask;
      *(uint*)Address = (uint)temp;
      break;

  }
  
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
int  TReg::Get()
{
  int temp;

  switch (Size) {
    case eREGSIZE_BYTE:
    case eREGSIZE_RBW_BYTE:
      temp = *(unsigned char*)Address;
      break;
    case eREGSIZE_WORD:
    case eREGSIZE_RBW_WORD:
      temp = *(unsigned short*)Address;
      break;
    case eREGSIZE_LONG:
    case eREGSIZE_RBW_LONG:
      temp = *(unsigned int*)Address;
      break;
    default:
      temp = 0;
      break;
  }  
  temp &= Mask;
  Value = temp >> Shift;
  return Value;
}



