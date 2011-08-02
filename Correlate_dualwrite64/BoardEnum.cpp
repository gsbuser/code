//-----------------------------------------------------------------------------
//title: Acquisition Logic A/D Board Kernel Driver Checkout Program
//version: Linux 1.0
//date:  March 2004
//designer: Michael Wyrick                                                   
//programmer: Michael Wyrick                                                
//platform: Linux 2.4.x
//language: GCC 3.3.1
//module: BoardENum
//-----------------------------------------------------------------------------
//  Purpose: Library for the AL4108
//  Docs:                                  
//-----------------------------------------------------------------------------
// CVS:
// $Id: BoardEnum.cpp,v 1.1 2005/01/03 16:41:16 mwyrick Exp $
// $Log: BoardEnum.cpp,v $
// Revision 1.1  2005/01/03 16:41:16  mwyrick
// Check in of Working Version
//
// Revision 1.1  2004/12/23 18:55:18  mwyrick
// Added BoardEnum to report the number of board in the system.
//
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

#include "BoardEnum.h"

//---------------------------------------------------------------------
// Open the main ali Handle
//---------------------------------------------------------------------
BoardEnum::BoardEnum() {
  int BoardHandle = open("//dev//ali_main", O_RDWR, 0);

  if (BoardHandle < 0) {
      NumberOfBoards = -1;
      throw new exHardware("Failed to open ali_main");
  }

  ioctl(BoardHandle, IOCTL_GET_NUMBOARDS, &NumberOfBoards);

  ioctl(BoardHandle, IOCTL_GET_BRD0_TYPE, &BoardTypes[0]);
  ioctl(BoardHandle, IOCTL_GET_BRD1_TYPE, &BoardTypes[1]);
  ioctl(BoardHandle, IOCTL_GET_BRD2_TYPE, &BoardTypes[2]);
  ioctl(BoardHandle, IOCTL_GET_BRD3_TYPE, &BoardTypes[3]);

  close(BoardHandle);
}

//---------------------------------------------------------------------
// 
//---------------------------------------------------------------------
BoardEnum::~BoardEnum() {
}

//---------------------------------------------------------------------
// 
//---------------------------------------------------------------------
char *BoardEnum::TypeToString(int BoardType) 
{
  switch (BoardType) {
    case BRDTYPE_ALGEN: return "ALGEN";
    case BRDTYPE_NONE:  return "NONE";
    case BRDTYPE_AL8100: return "AL8100";
    case BRDTYPE_AL1G: return "AL1G";
    case BRDTYPE_AL500: return "AL500";
    case BRDTYPE_AL212: return "AL212";
    case BRDTYPE_AL1G4: return "AL1G4";
    case BRDTYPE_AL5004: return "AL5004";
    case BRDTYPE_AL2124: return "AL2124";
    case BRDTYPE_AL4108: return "AL4108";
    case BRDTYPE_AL2114: return "AL2114";
    case BRDTYPE_ALTLS: return "ALTLS";
  }

  return "Invalid Board Type";
}

