//------------------------------------------------------------------------------  
//title: Acquisition Logic A/D Board Kernel Driver
//version: Linux 0.0
//date: July 2002                                                              
//designer: Michael Wyrick                                                      
//programmer: Michael Wyrick                                                    
//platform: Linux 2.4.x
//language: GCC 2.95 and 3.0
//module: aldriver
//------------------------------------------------------------------------------  
//  Purpose: Provide a Kernel Driver to Linux for the ALI A/D Boards
//  Docs:                                  
//    This driver supplies an interface to the raw Registers on the boards.
//    in is upto a user library or program to determine what to do with those
//    registers.
//------------------------------------------------------------------------------  
// RCS:
// $Id: aldriver.h,v 1.9 2005/11/04 15:52:52 mwyrick Exp $
// $Log: aldriver.h,v $
// Revision 1.9  2005/11/04 15:52:52  mwyrick
// Max DMA Speed Testing, Got 190MB/S on 64/66 bus with 4108.
//
// Revision 1.8  2005/01/17 22:21:16  mwyrick
// Removed some debug Printk
//
// Revision 1.7  2004/12/29 20:12:57  mwyrick
// Working well with two acq buffers.  Debugging off
//
// Revision 1.6  2004/12/29 15:21:10  mwyrick
// sending to Pen
//
// Revision 1.5  2004/12/28 16:36:02  mwyrick
// Intem
//
// Revision 1.4  2004/12/23 18:52:35  mwyrick
// Now has board enum in the main control number
//
// Revision 1.3  2004/12/23 14:52:19  mwyrick
// Multi Board is working.
// DMA Buffer is set to 10 Meg
//
// Revision 1.2  2004/12/22 19:29:06  mwyrick
// Working on Multi Board
//
// Revision 1.1.1.1  2004/12/16 14:27:28  mwyrick
// ALLinux 2004
//
// Revision 1.2  2004/10/06 13:53:13  mwyrick
// Added Proper Support for AL212.
// Changed the ResetPLX to reload the config from the EEPROM
// Made DMA Buffer size 8 Megs, there seems to be a problem when its
// larger than that.
//
// Revision 1.1.1.1  2004/04/29 19:20:14  mwyrick
// AcqLog Linux Driver
//
// Revision 1.25  2004/03/18 17:12:59  mwyrick
// Added Shared Interrupt Support
//
// Revision 1.24  2004/03/18 16:12:10  mwyrick
// Check in before starting work on the AL4108
//
// Revision 1.23  2002/12/18 21:50:37  mwyrick
// IOCTL_GET_BRDTYPE
//
// Revision 1.22  2002/12/18 16:35:46  mwyrick
// Removed the Flipped Vendor/Device IDs for 1G
//
// Revision 1.21  2002/12/17 19:46:08  mwyrick
// Started Adding AL1G Support
//
// Revision 1.20  2002/12/09 21:25:54  mwyrick
// Added RBF Upload Status to ioctl
//
// Revision 1.19  2002/11/27 21:14:23  mwyrick
// New memory Structure, DumpFPGA, DumpPLX
// IOCTL_COLLECTDATA
//
// Revision 1.18  2002/11/27 15:32:17  mwyrick
// Good Working Version
//
// Revision 1.17  2002/11/19 21:22:28  mwyrick
// Working with AL8100 to download Waveforms
//
// Revision 1.16  2002/11/19 15:07:35  mwyrick
// Standard Checkin before the 8100
//
// Revision 1.15  2002/11/05 14:53:51  mwyrick
// Trying out kernel 2.4.19 on DELL
//
// Revision 1.14  2002/10/24 17:34:15  mwyrick
// First version working with new ALLOC_DMA_BUFFER code
//
// Revision 1.13  2002/10/24 17:20:18  mwyrick
// Added New IOCTL for NumInts and CircleBuffer Mode
//
// Revision 1.12  2002/10/24 15:59:48  mwyrick
// Clean up under way
// NO boards except ALTLS
//
// Revision 1.11  2002/10/23 15:18:30  mwyrick
// Working on checking for buffer overruns
//
// Revision 1.10  2002/10/23 15:02:48  mwyrick
// Added FRR and MR Registers
//
// Revision 1.9  2002/10/21 18:54:42  mwyrick
// Working in the new ALTLS board
//
// Revision 1.8  2002/10/21 14:47:08  mwyrick
// Working Multi Interrupts and WAITPROGRESS
//
// Revision 1.7  2002/10/18 19:45:13  mwyrick
// Added CLR_CNTREG and fix damn spaces at end of vmem.*
//
// Revision 1.6  2002/10/14 16:00:27  mwyrick
// Have mmap working
// Have Blocking/NonBlocking IO Working
//
// Revision 1.5  2002/10/12 00:38:37  mwyrick
// Giving up on mmap for the night.
//
// Revision 1.4  2002/10/11 18:20:41  mwyrick
// More Registers
//
// Revision 1.3  2002/10/11 14:45:14  mwyrick
// DMA is Working in Simple Mode
// New Structure of Makefile
// Driver is now named ali.o
// new DMA.c file that contains the code for DMA transfers
// .depend works in new Makefile
// PLX constants put into plx9054.h
//
// Revision 1.2  2002/10/10 17:06:14  mwyrick
// Starting the DMA Stuff
//
// Revision 1.1.1.1  2002/08/21 13:37:36  mwyrick
// Acquisition Logic Linux Driver
//
//
//-----------------------------------------------------------------------------
#ifndef _aldriver_H
#define _aldriver_H

//-----------------------------------------------------------------------------
// Public Functions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//  Structs
//-----------------------------------------------------------------------------
typedef struct {
  int ReadType;
  int WriteType;
  long LocalBusAddress;
  long TransferCount;
  int mmapType;      // 0 = Map DMA, 1 = Map FPGA
  int BrdNum;
} TFile_Private;

typedef struct {
  int Size;
  int Address;
  int Value;
  int Mask;    // This is the PRE Shift Mask (not post shift)
  int Shift;   
} TReg_Def;

//-----------------------------------------------------------------------------
//  Constants
//-----------------------------------------------------------------------------
#define ALDMABUFFERSIZE   (2*32 * 1024 * 1024)

#define BRDTYPE_ALGEN     0x0000    // Generic PCI ID board
#define BRDTYPE_NONE      0x0001    // NO BOARD
#define BRDTYPE_AL8100    0x1018
#define BRDTYPE_AL1G	  0x10A8
#define BRDTYPE_AL500     0x1058
#define BRDTYPE_AL212     0x102C
#define BRDTYPE_AL1G4	  0x90A8
#define BRDTYPE_AL5004    0x9058
#define BRDTYPE_AL2124    0x902C
#define BRDTYPE_AL4108    0x4018
#define BRDTYPE_AL2114    0x201E
#define BRDTYPE_ALTLS     0x10A9

#define READTYPE_REG	  1
#define WRITETYPE_RBF 	  2
#define WRITETYPE_REG     3

#define REGSIZE_BYTE	  1
#define REGSIZE_WORD	  2
#define REGSIZE_LONG	  4
#define REGSIZE_RBW_BYTE  5
#define REGSIZE_RBW_WORD  6
#define REGSIZE_RBW_LONG  7

#define ALI_IOCTL_MAGIC		0x49

#define IOCTL_GET_BRDTYPE	_IOR(ALI_IOCTL_MAGIC, 1, 0)
#define IOCTL_INITBRD		_IOW(ALI_IOCTL_MAGIC, 2, 0)
#define IOCTL_GET_NUMBOARDS	_IOW(ALI_IOCTL_MAGIC, 3, 0)
#define IOCTL_GET_BRD0_TYPE	_IOW(ALI_IOCTL_MAGIC, 5, 0)
#define IOCTL_GET_BRD1_TYPE	_IOW(ALI_IOCTL_MAGIC, 6, 0)
#define IOCTL_GET_BRD2_TYPE	_IOW(ALI_IOCTL_MAGIC, 7, 0)
#define IOCTL_GET_BRD3_TYPE	_IOW(ALI_IOCTL_MAGIC, 8, 0)

#define IOCTL_LOCALBUS_ADDR	_IOW(ALI_IOCTL_MAGIC, 10, 0)
#define IOCTL_STARTDMA		_IOW(ALI_IOCTL_MAGIC, 11, 0)
#define IOCTL_ABORTDMA		_IOW(ALI_IOCTL_MAGIC, 12, 0)
#define IOCTL_GET_DMASTATUS	_IOR(ALI_IOCTL_MAGIC, 13, 0)
#define IOCTL_GET_BRDSTATUS	_IOR(ALI_IOCTL_MAGIC, 14, 0)
#define IOCTL_GET_RBFSTATUS	_IOR(ALI_IOCTL_MAGIC, 15, 0)
#define IOCTL_WAITPROGRESS	_IOW(ALI_IOCTL_MAGIC, 16, 0)
#define IOCTL_SET_CIRCLEBUFFER	_IOW(ALI_IOCTL_MAGIC, 17, 0)
#define IOCTL_SET_NUMINTS	_IOW(ALI_IOCTL_MAGIC, 18, 0)
#define IOCTL_ALLOC_DMA_BUFFER	_IOW(ALI_IOCTL_MAGIC, 19, 0)
#define IOCTL_FREE_DMA_BUFFER	_IOW(ALI_IOCTL_MAGIC, 20, 0)
#define IOCTL_SWTRIGGER		_IOW(ALI_IOCTL_MAGIC, 21, 0)
#define IOCTL_GET_FIFOSTATUS	_IOW(ALI_IOCTL_MAGIC, 22, 0)

#define IOCTL_SET_FRR		_IOW(ALI_IOCTL_MAGIC, 30, 0)
#define IOCTL_GET_FRR		_IOR(ALI_IOCTL_MAGIC, 31, 0)
#define IOCTL_SET_MR		_IOW(ALI_IOCTL_MAGIC, 32, 0)
#define IOCTL_GET_MR		_IOR(ALI_IOCTL_MAGIC, 33, 0)
#define IOCTL_CLR_CNTREG	_IOW(ALI_IOCTL_MAGIC, 34, 0)

#define IOCTL_READ_REG		_IOR(ALI_IOCTL_MAGIC, 40, 0)
#define IOCTL_WRITE_REG		_IOW(ALI_IOCTL_MAGIC, 41, 0)

#define IOCTL_TRANSFERCOUNT	_IOW(ALI_IOCTL_MAGIC, 50, 0)
#define IOCTL_SETDMATIMEOUT	_IOW(ALI_IOCTL_MAGIC, 51, 0)
#define IOCTL_PRINTPLX		_IOW(ALI_IOCTL_MAGIC, 52, 0)
#define IOCTL_PRINTFPGA		_IOW(ALI_IOCTL_MAGIC, 53, 0)
#define IOCTL_COLLECTDATA	_IOW(ALI_IOCTL_MAGIC, 54, 0)

#define IOCTL_SETMEMTYPE	_IOW(ALI_IOCTL_MAGIC, 60, 0)
#define IOCTL_RELOADCNTMODE	_IOW(ALI_IOCTL_MAGIC, 61, 0)
#define IOCTL_GETBUFFERSTATUS	_IOW(ALI_IOCTL_MAGIC, 62, 0)
#define IOCTL_CLEARBUFFERSTATUS	_IOW(ALI_IOCTL_MAGIC, 63, 0)

#ifndef PCI_VENDOR_ID_ACQUISITIONLOGIC
#define PCI_VENDOR_ID_ACQUISITIONLOGIC 0x1832
#endif

#ifndef PCI_DEVICE_ID_ACQUISITIONLOGIC_AL8100
#define PCI_DEVICE_ID_ACQUISITIONLOGIC_AL8100 0x1018
#endif

#ifndef PCI_DEVICE_ID_ACQUISITIONLOGIC_AL1G
#define PCI_DEVICE_ID_ACQUISITIONLOGIC_AL1G 0x10A8
#endif

#ifndef PCI_DEVICE_ID_ACQUISITIONLOGIC_AL1G4
#define PCI_DEVICE_ID_ACQUISITIONLOGIC_AL1G4 0x90A8
#endif

#ifndef PCI_DEVICE_ID_ACQUISITIONLOGIC_AL500
#define PCI_DEVICE_ID_ACQUISITIONLOGIC_AL500 0x1058
#endif

#ifndef PCI_DEVICE_ID_ACQUISITIONLOGIC_AL5004
#define PCI_DEVICE_ID_ACQUISITIONLOGIC_AL5004 0x9058
#endif

#ifndef PCI_DEVICE_ID_ACQUISITIONLOGIC_AL212
#define PCI_DEVICE_ID_ACQUISITIONLOGIC_AL212 0x102C
#endif

#ifndef PCI_DEVICE_ID_ACQUISITIONLOGIC_AL2124
#define PCI_DEVICE_ID_ACQUISITIONLOGIC_AL2124 0x902C
#endif

#ifndef PCI_DEVICE_ID_ACQUISITIONLOGIC_AL4108
#define PCI_DEVICE_ID_ACQUISITIONLOGIC_AL4108 0x4018
#endif

#ifndef PCI_DEVICE_ID_ACQUISITIONLOGIC_AL2114
#define PCI_DEVICE_ID_ACQUISITIONLOGIC_AL2114 0x201E
#endif

#ifndef PCI_DEVICE_ID_ACQUISITIONLOGIC_ALTLS
#define PCI_DEVICE_ID_ACQUISITIONLOGIC_ALTLS 0x10A9
#endif

#endif
