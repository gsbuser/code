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
// $Id: BoardEnum.h,v 1.1 2005/01/03 16:41:16 mwyrick Exp $
// $Log: BoardEnum.h,v $
// Revision 1.1  2005/01/03 16:41:16  mwyrick
// Check in of Working Version
//
// Revision 1.1  2004/12/23 18:55:18  mwyrick
// Added BoardEnum to report the number of board in the system.
//
//
//-----------------------------------------------------------------------------
#ifndef BoardEnumH
#define BoardEnumH

#include "aldriver.h"
#include "Exceptions.h"

//----------------------------------------------------------------------
//
// Board Enum Class
//
//----------------------------------------------------------------------
class BoardEnum {
public:
    int NumberOfBoards;
    int BoardTypes[4];

    BoardEnum();
    ~BoardEnum();
    
    char *TypeToString(int BoardType);

};

#endif

                  
                  
                  
