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
// $Id: Board4108.cpp,v 1.2 2005/01/03 20:29:03 mwyrick Exp $
// $Log: Board4108.cpp,v $
// Revision 1.2  2005/01/03 20:29:03  mwyrick
// Added Assembly Revision 9 PLL Programming Value
//
// Revision 1.1  2005/01/03 16:41:16  mwyrick
// Check in of Working Version
//
// Revision 1.16  2004/12/29 15:20:57  mwyrick
// Preparing to send to Pen
//
// Revision 1.15  2004/12/28 18:04:45  mwyrick
// interm
//
// Revision 1.14  2004/12/28 16:36:14  mwyrick
// Working
//
// Revision 1.13  2004/12/23 20:27:11  mwyrick
// Added Serial Number
//
// Revision 1.12  2004/12/23 20:02:21  sschulz
// fixed FineGain and ADOffset values for 100MS/sec
//
// Revision 1.11  2004/12/23 19:59:44  mwyrick
// Working on 400 and 200 MHz mode
//
// Revision 1.10  2004/12/23 19:39:57  sschulz
// added TGC registers and TGC set without profile, forced 100MS/sec to TGC path on all channels

//
// Revision 1.9  2004/12/23 19:18:51  sschulz
// merged in changes to DAC setting and use of calibration data
//
// Revision 1.8  2004/12/23 18:55:18  mwyrick
// Added BoardEnum to report the number of board in the system.
//
// Revision 1.7  2004/12/23 16:42:55  mwyrick
// Fixed Missing Para
//
// Revision 1.6  2004/12/23 16:39:04  sschulz
// merged in changes to DAC setting and use of calibration data
//
// Revision 1.5  2004/12/23 16:12:47  mwyrick
// Working Well with StartAcq and then Multiple DMAs to collect data.
//
// Revision 1.4  2004/12/23 14:51:10  mwyrick
// Multi Board seems to work.  Added Max5841 and Max504.  They are in but untested
//
// Revision 1.3  2004/12/22 19:27:13  sschulz
// added MAX5841 and MAX504 DACs
//
// Revision 1.2  2004/12/16 14:48:30  mwyrick
// Readme file for the Record Program
// Changed download register to 0x40 (data) from 0x78 (xfercnt)
//
// Revision 1.1.1.1  2004/12/16 14:27:28  mwyrick
// ALLinux 2004
//
// Revision 1.4  2004/04/29 19:41:30  mwyrick
// Removed extra space in header
//
// Revision 1.3  2004/04/29 19:15:29  mwyrick
// added to header
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
#define MIN(x,y)  ((x<y)?x:y)

// Define this to transfer data from the Counting register insted of real data
#define DEBUGGING
#undef DEBUGGING

#include "aldriver.h"
#include "Board4108.h"
#include "I2C_4108.h"

long BufferSize = ALDMABUFFERSIZE;

//---------------------------------------------------------------------
// Destructor
//---------------------------------------------------------------------
AL4108::~AL4108() {
  close(BoardHandle);
}

//------------------------------------------------------------------------
// Returns the Short from the EEPROM at location address (byte address)
//------------------------------------------------------------------------
unsigned short AL4108::GetEEPROMShort(int address) {
    unsigned short val = (unsigned short)(((unsigned short)userEEPROM->Data[address + 1] << 8) +
                                                  userEEPROM->Data[address]);
    return val;
}

//---------------------------------------------------------------------
// Return the Board Serial Number
//---------------------------------------------------------------------
unsigned int AL4108::SerialNumber() {
  unsigned int tmp = (((unsigned int)userEEPROM->Data[8]) << 24) +
                     (((unsigned int)userEEPROM->Data[7]) << 16) +
                     (((unsigned int)userEEPROM->Data[6]) << 8) +
                     ((unsigned int)userEEPROM->Data[5]);

  return tmp;
}                                                            

//---------------------------------------------------------------------
// Open the Board and Init it
//---------------------------------------------------------------------
AL4108::AL4108(bool UploadRBF = true, int BrdNum = 0) { 
  int j;
  OldAcqMode = 0;  // Clear the AcqMode

  switch (BrdNum) {
  case 0: 
      BoardHandle = open("//dev//ali_brd1", O_RDWR, 0);
      break;
  case 1: 
      BoardHandle = open("//dev//ali_brd2", O_RDWR, 0);
      break;
  case 2: 
      BoardHandle = open("//dev//ali_brd3", O_RDWR, 0);
      break;
  case 3: 
      BoardHandle = open("//dev//ali_brd4", O_RDWR, 0);
      break;
  }

  if (BoardHandle < 0) {
    printf("Open on //dev//ali_brdX failed.");
    exit(1);
  }

  // Get a Pointer to the DMA Memory
  ioctl(BoardHandle, IOCTL_SETMEMTYPE, 0);
  Data = (unsigned char *)mmap(0, BufferSize, PROT_READ | PROT_WRITE, MAP_SHARED, BoardHandle, 0);

  if ((int)Data == -1) {
    printf("Error during mmap, errno=%i\n",errno);
  }

  // Get a Pointer to the FPGA Registers
  ioctl(BoardHandle, IOCTL_SETMEMTYPE, 1);
  FPGAaddr = (unsigned char *)mmap(0, 256, PROT_READ | PROT_WRITE, MAP_SHARED, BoardHandle, 0);
//  FPGAaddr = (unsigned char *)mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, BoardHandle, 0);
  printf("FPGAaddr = 0x%x\n",FPGAaddr);
/*
    The mmap always returns a page aligned address on some systems,
    e.g. SE7320EP2.  Then we need to search the whole page for
    the actual location of the FPGA registers.
  for (j=0;j<256*16;j+=256) {
        if (*((unsigned short *)(FPGAaddr+j+0x7c)) == 0x020D) break;
  }
  FPGAaddr+=j;
  printf("FPGAaddr = 0x%x\n",FPGAaddr);
  for (j=0;j<256;j+=16) {
        for (int i=0;i<16;i++) printf("%x ",FPGAaddr[i+j]);
        printf("%x\n",j);
  }

 */

  if ((int)FPGAaddr == -1) {
      printf("Error during mmap for FPGA Memory, errno=%i\n", errno);
  }

  fcntl(BoardHandle, F_SETFL, fcntl(BoardHandle, F_GETFL) & ~O_NONBLOCK);
  ioctl(BoardHandle, IOCTL_INITBRD);
#ifdef DEBUGGING
  ioctl(BoardHandle, IOCTL_LOCALBUS_ADDR, 0x78); 
#else
  ioctl(BoardHandle, IOCTL_LOCALBUS_ADDR, 0x40);
#endif
  ioctl(BoardHandle, IOCTL_SET_CIRCLEBUFFER, 0);

  // No Interrupts Until Done
  ioctl(BoardHandle, IOCTL_SET_NUMINTS, 0);
  // DMA Timeout in Seconds
  ioctl(BoardHandle, IOCTL_SETDMATIMEOUT, 3);

  // Setup the Register vars
  SetupRegisters(FPGAaddr);

  // See if we should upload the RBF file
  if (UploadRBF == true) {
    UploadDefault();
  }

  // Build a i2cBus object for the userEEPROM and Dacs
  i2cBus  = new I2C_4108(I2CCMD, I2CDATA);
  userEEPROM = new EEPROM_24xx64(i2cBus);

  // dec 17, 04  SWS
  PCBREV->Set(userEEPROM->Data[9]);    // set board revision 
  dacs[0] = new Max5306(DACDATA, DACBUSY, DACSEL, DACSEL2, 0, 0);
  dacs[1] = new Max5306(DACDATA, DACBUSY, DACSEL, DACSEL2, 1, 0);
  dacs[2] = new Max5841(i2cBus, 0xb8);
  dacs[3] = new Max5841(i2cBus, 0xba);
  dacs[4] = new Max504(DACDATA, DACBUSY, DACSEL, DACSEL2, 0, 1);

#ifndef OLDCLOCK
  // Setup the PLL
  ProgramPLL(0x000032);
  ProgramPLL(0x000072);
  ProgramPLL(0x00001A);
  ProgramPLL(0x000093);
  ProgramPLL(0x100028);

  // Check the Assembly Revision for the PLL AB Load Value
  if (userEEPROM->Data[0x0A] >= 0x09) {
      ProgramPLL(0x001901);
  } else {
      ProgramPLL(0x006401);
  }
#endif

  // Setup the FPGA Registers to known Values
  TRIGPOL->Set(1);     // Rising Edge
  TRIGSRC->Set(0);     // Software

  AINRANGE->Set(0);    // One Volt
  AACCOUPLE->Set(0);   // AC Coupled
  A50TERM->Set(1);     // 50Ohm, Needed for Direct Path
  AADSRC->Set(2);      // Direct Path

  BINRANGE->Set(0);    // One Volt
  BACCOUPLE->Set(0);   // AC Coupled
  B50TERM->Set(1);     // 50Ohm, Needed for Direct Path
  BADSRC->Set(2);      // Direct Path
  
  CACCOUPLE->Set(0);   // AC Coupled
  C50TERM->Set(1);     // 50Ohm, Needed for Direct Path
  CADSRC->Set(0);      // Direct Path

  DACCOUPLE->Set(0);   // AC Coupled
  D50TERM->Set(1);     // 50Ohm, Needed for Direct Path
  DADSRC->Set(0);      // Direct Path

  BNCOUTEN->Set(0x01); // Enable the BNC Out
  BNCOUT->Set(0x03);   // Turn on BNC Out
  BNCPOL->Set(0x01);   // POL is Normal
  BNCSRC->Set(0x00);   // Source is Software

  // dec 22, 04 SWS
  // set D/A converters to common calibration values for 1000mV input range
  // these voltages are not needed when using a direct path but will avoid
  // driving any amplifier stage to its limits
   
  SetDAC("A_InputOffset", GetEEPROMShort(0x010));
  SetDAC("B_InputOffset", GetEEPROMShort(0x054));
  SetDAC("C_InputOffset", GetEEPROMShort(0x098));
  SetDAC("D_InputOffset", GetEEPROMShort(0x0dc));
   
  SetDAC("A_VideoBalance", GetEEPROMShort(0x038));
  SetDAC("B_VideoBalance", GetEEPROMShort(0x07c));
  SetDAC("C_VideoBalance", GetEEPROMShort(0x0c0));
  SetDAC("D_VideoBalance", GetEEPROMShort(0x104));

  SetUpTGC(0, 0, 0, 0);    // minimum TGC gain on all channels

  // Wait for all the Dacs to settle
  WaitForDACSettle();
}

//------------------------------------------------------------------------
// Turns on/off the DMA Circle Buffer
//------------------------------------------------------------------------
void AL4108::TurnOnOffCircleBuffer(int b, int numints)
{
  ioctl(BoardHandle, IOCTL_SET_CIRCLEBUFFER, b);
  ioctl(BoardHandle, IOCTL_SET_NUMINTS, numints);
}

//------------------------------------------------------------------------
// Changes the InputOffset for a Given Channel
//------------------------------------------------------------------------
void AL4108::SetInputOffset(int Chan, short value)
{
    switch(Chan) {
    case 0 :
        SetDAC("A_InputOffset", GetEEPROMShort(0x010) + value);
        break;
    case 1 :
        SetDAC("B_InputOffset", GetEEPROMShort(0x054) + value);
        break;
    case 2 :
        SetDAC("C_InputOffset", GetEEPROMShort(0x098) + value);
        break;
    case 3 :
        SetDAC("D_InputOffset", GetEEPROMShort(0x0dc) + value);
        break;
    }
}

//------------------------------------------------------------------------
// Setup the TGC gain
//   gain profile not implemented yet
//   this is the static TGC set routine from the lab system
//------------------------------------------------------------------------
void AL4108::SetUpTGC(char GainA, char GainB, char GainC, char GainD)
{
   TGCENABLE->Set(0);
   TGCRESET->Set(1);
   TGCAPADDR->Set(0);
   TGCRESET->Set(0);

   TGCAPLEN->Set(0);
   TGCFOLLOW->Set(0);    // fixed to absolute mode
   TGCAPTW->Set(100);    // anything larger than 4
   TGCAPH ->Set(0);      // no delay

   // Write the Gains
   TGCAPGAIN->Set((uint)(GainA << 24) + 
   			   (uint)(GainB << 16) + 
   			   ((0xFF - (uint)GainC) << 8) + 
   			   (0xFF - (uint)GainD));

   TGCENABLE->Set(1);
}

//------------------------------------------------------------------------
// Setup the Clock Select registers for different modes
//------------------------------------------------------------------------
void AL4108::SetUpClockSelectregs(int AcqMode) {

  if (AcqMode & 0x0200) {        // Setup for 400Mhz
    ADIMODE->Set(0x3);           // 400 MHz interleave mode
    AADSRC->Set(2);              // select A direct for A/D converter A
    BADSRC->Set(3);              // select A direct for A/D converter B
    CADSRC->Set(2);              // select A direct for A/D converter C
    DADSRC->Set(3);              // select A direct for A/D converter D

    SetDAC("A_ClockSkew", GetEEPROMShort(0x178));
    SetDAC("B_ClockSkew", GetEEPROMShort(0x18A));
    SetDAC("C_ClockSkew", GetEEPROMShort(0x19C));
    SetDAC("D_ClockSkew", GetEEPROMShort(0x1AE));

    SetDAC("A_FineGain", GetEEPROMShort(0x168));
    SetDAC("B_FineGain", GetEEPROMShort(0x17A));
    SetDAC("C_FineGain", GetEEPROMShort(0x18C));
    SetDAC("D_FineGain", GetEEPROMShort(0x19E));

    SetDAC("A_ADOffset", GetEEPROMShort(0x170));
    SetDAC("B_ADOffset", GetEEPROMShort(0x182));
    SetDAC("C_ADOffset", GetEEPROMShort(0x194));
    SetDAC("D_ADOffset", GetEEPROMShort(0x1A6));
  
  } else if (AcqMode & 0x0100) { // Setup for 200Mhz
    ADIMODE->Set(0x1);           // 200 MHz interleave mode
    AADSRC->Set(2);              // select A direct for A/D converter A
    BADSRC->Set(2);              // select B direct for A/D converter B
    CADSRC->Set(2);              // select A direct for A/D converter C
    DADSRC->Set(2);              // select B direct for A/D converter D

    SetDAC("A_ClockSkew", GetEEPROMShort(0x130));
    SetDAC("B_ClockSkew", GetEEPROMShort(0x142));
    SetDAC("C_ClockSkew", GetEEPROMShort(0x154));
    SetDAC("D_ClockSkew", GetEEPROMShort(0x166));

    SetDAC("A_FineGain", GetEEPROMShort(0x120));
    SetDAC("B_FineGain", GetEEPROMShort(0x132));
    SetDAC("C_FineGain", GetEEPROMShort(0x144));
    SetDAC("D_FineGain", GetEEPROMShort(0x156));

    SetDAC("A_ADOffset", GetEEPROMShort(0x128));
    SetDAC("B_ADOffset", GetEEPROMShort(0x13a));
    SetDAC("C_ADOffset", GetEEPROMShort(0x14c));
    SetDAC("D_ADOffset", GetEEPROMShort(0x15e));

  } else {
    ADIMODE->Set(0x0);           // 100 MHz non-interleaved mode
    AADSRC->Set(0);              // TGC path on all channels
    BADSRC->Set(0);
    CADSRC->Set(0);
    DADSRC->Set(0);

    SetDAC("A_ClockSkew", 2048);
    SetDAC("B_ClockSkew", 2048);
    SetDAC("C_ClockSkew", 2048);
    SetDAC("D_ClockSkew", 2048);

    SetDAC("A_FineGain", GetEEPROMShort(0x014));
    SetDAC("B_FineGain", GetEEPROMShort(0x058));
    SetDAC("C_FineGain", GetEEPROMShort(0x09c));
    SetDAC("D_FineGain", GetEEPROMShort(0x0e0));

    SetDAC("A_ADOffset", GetEEPROMShort(0x012));
    SetDAC("B_ADOffset", GetEEPROMShort(0x056));
    SetDAC("C_ADOffset", GetEEPROMShort(0x09a));
    SetDAC("D_ADOffset", GetEEPROMShort(0x0de));

    SetUpTGC( (char) (GetEEPROMShort(0x018) & 0x0ff),    // A_RF_1000mV
              (char) (GetEEPROMShort(0x05c) & 0x0ff),    // B_RF_1000mV
              (char) (GetEEPROMShort(0x0a0) & 0x0ff),    // C_RF_1000mV
              (char) (GetEEPROMShort(0x0e4) & 0x0ff)     // D_RF_1000mV
            );   
  }

  // Set the Acqmode
  AMODE->Set(AcqMode & 0x3F);

  // Wait for all the Dacs to Settle
  WaitForDACSettle();
}


//------------------------------------------------------------------------
// Start Acq
//------------------------------------------------------------------------
void AL4108::SetupAcq(int Samples) {
  AENABLE->Set(0); 	// Acq Is Disabled
  GENABLE->Set(0);  // Global is Disabled

  TRIGCNT->Set(0);  // Clear the Trigger Counter
 
  // Do a General Clear
  GENCLR->Set(1);
  GENCLR->Set(0); 

  // Clear all FIFOs
  FIFOCLR->Set(1);
  FIFOCLR->Set(0);

  // Clear Input FIFO overflow flag
  FOVERRUN->Set(1);

  // Clear Output FIFO Overflow flag
  OFOVERFLOW->Set(1);

  // Clear the transfer Count Register so we can check data
  XFERCNT->Set(0);

  int al = Samples;
  int clen = (al + 127 + 16) / 64;
  CACQLEN->Set(clen);
  
  ACQLEN->Set(al - 3);

  NUMWF->Set(524288 / (clen+1));

  WDGLEN->Set(Samples - 1);

  GENABLE->Set(1); // Global is Enabled
}

//------------------------------------------------------------------------
// Enable Acquisition
//------------------------------------------------------------------------
void AL4108::EnableAcq() {
    AENABLE->Set(1);   // Acq Enable
}

//------------------------------------------------------------------------
// Trigger
//------------------------------------------------------------------------
void AL4108::TriggerAcq() {
  if (TRIGREADY->Get() == 1) {
    SWTRIG->Set(1);    // Software Trigger
  } else {
    throw new int(1);
  }
}

//------------------------------------------------------------------------
// End Acquisitiion
//------------------------------------------------------------------------
void AL4108::EndAcq() {
    AENABLE->Set(0);   // Acq Disable
    GENABLE->Set(0);   // Global Disable
}

//------------------------------------------------------------------------
// Start DMA
//------------------------------------------------------------------------
void AL4108::StartDMA(int SizeInBytes) {
  int SizeInDWords = SizeInBytes / 4;
  DMACNT->Set(SizeInDWords);               // Set the DMA Count

  ioctl(BoardHandle, IOCTL_TRANSFERCOUNT, SizeInBytes);
  ioctl(BoardHandle, IOCTL_STARTDMA);
}

//------------------------------------------------------------------------
// WaitForData
//------------------------------------------------------------------------
void AL4108::WaitForDMAInterrupt() {
  ioctl(BoardHandle, IOCTL_WAITPROGRESS);
}

//------------------------------------------------------------------------
// EndDMA
//------------------------------------------------------------------------
void AL4108::EndDMA() {
  ioctl(BoardHandle, IOCTL_ABORTDMA);
}

//------------------------------------------------------------------------
// DMA Status
//------------------------------------------------------------------------
long AL4108::DMAStatus() {
  long Count;
  ioctl(BoardHandle, IOCTL_GET_DMASTATUS, &Count);
  return Count;
}

//------------------------------------------------------------------------
// Board Status
//------------------------------------------------------------------------
long AL4108::BoardStatus() {
  long Status;
  ioctl(BoardHandle, IOCTL_GET_BRDSTATUS, &Status);
  return Status;
}

//------------------------------------------------------------------------
// ProgramPLL
//------------------------------------------------------------------------
bool AL4108::ProgramPLL(int data) {
  int bit_num;
#ifndef OLDCLOCK
  PLLLATCH->Set(0);
  PLLCLK->Set(0);

  for(bit_num = 23; bit_num >= 0; --bit_num) {
    PLLDATA->Set( (data >> bit_num) & 0x01 );
    PLLCLK->Set(1);
    PLLCLK->Set(0);
  }

  PLLLATCH->Set(1);
  PLLLATCH->Set(0);
#endif

  return true;
}

//------------------------------------------------------------------------
//  Returns the PLL Status
//------------------------------------------------------------------------
bool AL4108::GetPLLStatus(void)
{
    return PLLSTATUS->Get();
}

//------------------------------------------------------------------------
// ReadTestAD
//------------------------------------------------------------------------
double AL4108::ReadTestAD(int channel)
{
  double vdf[] = {11.20, 11.20, 1.665, 1.0, 1.0, 1.0, 1.665, 4.65, 1.0, 1.665, 1.665, 2.458, 1.35, 1.665, 0, 0};
  double retval = 0.0;

  if (channel > 15) {
      throw new exHardware("Invalid Test A/D Channel\n");
  }

  ADCSEL->Set(channel);
  int to = 0;
  while( ADCBUSY->Get() == 1) {
      to++;
      if (to > 1000000) {
          throw new exHardware("Timeout waiting for Test A/D\n");
      }
  }

  ADCSEL->Set(channel);
  to = 0;
  while( ADCBUSY->Get() == 1) {
      to++;
      if (to > 1000000) {
          throw new exHardware("Timeout waiting for Test A/D\n");
      }
  }

  int v = -1 * (ADCDATA->Get() - 0x800);
  retval = ((double)v * (vdf[channel] / 1000.0)) + 2.048;

  return retval;
}

//------------------------------------------------------------------------
// SetupRegisters
//------------------------------------------------------------------------
void AL4108::SetupRegisters(unsigned char *FPGAaddr)
{
  MCR 				= new TRegByte(FPGAaddr+0x00);
  FIFOCLR			= new TReg(FPGAaddr+0x00, eREGSIZE_RBW_BYTE, 0x10, 4);
  GENABLE			= new TReg(FPGAaddr+0x00, eREGSIZE_RBW_BYTE, 0x04, 2);

  FOVERRUN          = new TReg(FPGAaddr+0x01, eREGSIZE_RBW_BYTE, 0x20, 5);

  WDGLEN            = new TRegLong(FPGAaddr+0x08);

  ENCDATA	= new TRegWord(FPGAaddr+0x34);
  ENCHI		= new TRegByte(FPGAaddr+0x35);
  
  DATA 				= new TRegLong(FPGAaddr+0x40);
  TRIGCNT 			= new TRegLong(FPGAaddr+0x68);
  TIMER 			= new TRegLong(FPGAaddr+0x6C);

  DMACNT			= new TRegLong(FPGAaddr+0x70);
  XFERCNT			= new TRegLong(FPGAaddr+0x78);
  FPGAVERSION		= new TRegLong(FPGAaddr+0x7C);

  ACR 				= new TRegByte(FPGAaddr+0x80);
  TRIGREADY			= new TReg(FPGAaddr+0x80, eREGSIZE_RBW_BYTE, 0x80, 7);
  DECIMATE3                     = new TReg(FPGAaddr+0x80, eREGSIZE_RBW_BYTE, 0x40, 6);
  GENCLR			= new TReg(FPGAaddr+0x80, eREGSIZE_RBW_BYTE, 0x10, 4);
  AENABLE			= new TReg(FPGAaddr+0x80, eREGSIZE_RBW_BYTE, 0x04, 2);

  AMR 				= new TRegByte(FPGAaddr+0x81);
  OFOVERFLOW	 	= new TReg(FPGAaddr+0x81, eREGSIZE_RBW_BYTE, 0x80, 7);
  OFEMPTY			= new TReg(FPGAaddr+0x81, eREGSIZE_RBW_BYTE, 0x40, 6);
  ACS               = new TReg(FPGAaddr+0x81, eREGSIZE_RBW_BYTE, 0x3C, 2);
  AMODE				= new TReg(FPGAaddr+0x81, eREGSIZE_RBW_BYTE, 0x3F, 0);

  FINEPOST          = new TRegWord(FPGAaddr + 0x82);
  CPOST             = new TRegLong(FPGAaddr + 0x84);
  
  CACQLEN           = new TRegLong(FPGAaddr + 0x88);
  ACQLEN            = new TRegLong(FPGAaddr + 0x8C);
  NUMWF             = new TRegLong(FPGAaddr + 0x90);
  
  SWTRIG			= new TReg(FPGAaddr+0x94, eREGSIZE_RBW_BYTE, 0x80, 7);
  TRIGPOL			= new TReg(FPGAaddr+0x94, eREGSIZE_RBW_BYTE, 0x08, 3);
  TRIGSRC			= new TReg(FPGAaddr+0x94, eREGSIZE_RBW_BYTE, 0x07, 0);

  BNCTERM			= new TReg(FPGAaddr+0x95, eREGSIZE_RBW_BYTE, 0x80, 7);
  BNCOUTEN		 	= new TReg(FPGAaddr+0x95, eREGSIZE_RBW_BYTE, 0x40, 6);
  BNCOUT			= new TReg(FPGAaddr+0x95, eREGSIZE_RBW_BYTE, 0x30, 4);
  BNCPOL			= new TReg(FPGAaddr+0x95, eREGSIZE_RBW_BYTE, 0x08, 3);
  BNCSRC			= new TReg(FPGAaddr+0x95, eREGSIZE_RBW_BYTE, 0x07, 0);

  DACBUSY		    = new TReg(FPGAaddr+0x96, eREGSIZE_RBW_BYTE,0x80, 7);
  DACSEL		    = new TReg(FPGAaddr+0x96, eREGSIZE_RBW_BYTE,0x40, 6);
  SMCOUT                    = new TReg(FPGAaddr+0x96, eREGSIZE_RBW_BYTE,0x30, 4) ;
  SMCPOL                    = new TReg(FPGAaddr+0x96, eREGSIZE_RBW_BYTE,0x08, 3) ;
  SMCSRC                    = new TReg(FPGAaddr+0x96, eREGSIZE_RBW_BYTE,0x07, 0) ;
  DACDATA			= new TRegWord(FPGAaddr+0x98);
  DACSEL2	        = new TReg(FPGAaddr+0x9d, eREGSIZE_RBW_BYTE,0x40, 6);

  PCBREV			= new TRegByte(FPGAaddr+0xBF);

  AINRANGE		 	= new TReg(FPGAaddr+0xC0, eREGSIZE_RBW_BYTE, 0xC0, 6);
  AACCOUPLE		 	= new TReg(FPGAaddr+0xC0, eREGSIZE_RBW_BYTE, 0x20, 5);
  A50TERM			= new TReg(FPGAaddr+0xC0, eREGSIZE_RBW_BYTE, 0x10, 4);
  AADSRC			= new TReg(FPGAaddr+0xC0, eREGSIZE_RBW_BYTE, 0x03, 0);

  BINRANGE		 	= new TReg(FPGAaddr+0xC1, eREGSIZE_RBW_BYTE, 0xC0, 6);
  BACCOUPLE		 	= new TReg(FPGAaddr+0xC1, eREGSIZE_RBW_BYTE, 0x20, 5);
  B50TERM			= new TReg(FPGAaddr+0xC1, eREGSIZE_RBW_BYTE, 0x10, 4);
  BADSRC			= new TReg(FPGAaddr+0xC1, eREGSIZE_RBW_BYTE, 0x03, 0);

  CACCOUPLE		 	= new TReg(FPGAaddr+0xC2, eREGSIZE_RBW_BYTE, 0x20, 5);
  C50TERM			= new TReg(FPGAaddr+0xC2, eREGSIZE_RBW_BYTE, 0x10, 4);
  CADSRC			= new TReg(FPGAaddr+0xC2, eREGSIZE_RBW_BYTE, 0x03, 0);

  DACCOUPLE		 	= new TReg(FPGAaddr+0xC3, eREGSIZE_RBW_BYTE, 0x20, 5);
  D50TERM			= new TReg(FPGAaddr+0xC3, eREGSIZE_RBW_BYTE, 0x10, 4);
  DADSRC			= new TReg(FPGAaddr+0xC3, eREGSIZE_RBW_BYTE, 0x03, 0);

  CCR				= new TRegByte(FPGAaddr+0xC4);
  PLLSTATUS		 	= new TReg(FPGAaddr+0xC4, eREGSIZE_RBW_BYTE, 0x80, 7);
  PLLLATCH		 	= new TReg(FPGAaddr+0xC4, eREGSIZE_RBW_BYTE, 0x40, 6);
  PLLDATA		 	= new TReg(FPGAaddr+0xC4, eREGSIZE_RBW_BYTE, 0x20, 5);
  PLLCLK		 	= new TReg(FPGAaddr+0xC4, eREGSIZE_RBW_BYTE, 0x10, 4);
	
  ADIMODE			= new TReg(FPGAaddr+0xC4, eREGSIZE_RBW_BYTE, 0x0C, 2);
  CMS				= new TReg(FPGAaddr+0xC4, eREGSIZE_RBW_BYTE, 0x03, 0);

  // User EEPROM
  I2CCMD			= new TRegByte(FPGAaddr+0xAC);
  I2CDATA			= new TRegByte(FPGAaddr+0xAD);

  // Test AD Convert
  ADCSEL	    = new TReg(FPGAaddr + 0x9E, eREGSIZE_RBW_BYTE, 0x0F, 0);
  ADCBUSY	    = new TReg(FPGAaddr + 0x9C, eREGSIZE_RBW_WORD, 0x8000, 15);
  ADCDATA	    = new TReg(FPGAaddr + 0x9C, eREGSIZE_RBW_WORD, 0x0FFF, 0);

  // Time-Gain Control
  TGCRESET		= new TReg(FPGAaddr + 0xD0, eREGSIZE_RBW_BYTE, 0x08, 3);
  TGCENABLE		= new TReg(FPGAaddr + 0xD0, eREGSIZE_RBW_BYTE, 0x04, 2);
  TGCFOLLOW		= new TReg(FPGAaddr + 0xD0, eREGSIZE_RBW_BYTE, 0x02, 1);

  TGCAPH			= new TRegWord(FPGAaddr + 0xD2);
  TGCAPTW		= new TRegLong(FPGAaddr + 0xD4);
  TGCAPLEN		= new TRegWord(FPGAaddr + 0xD8);
  TGCAPADDR		= new TRegWord(FPGAaddr + 0xDA);
  TGCAPGAIN		= new TRegLong(FPGAaddr + 0xDC);
  TGCGAINA		= new TRegByte(FPGAaddr + 0xDF);
  TGCGAINB		= new TRegByte(FPGAaddr + 0xDE);
  TGCGAINC		= new TRegByte(FPGAaddr + 0xDD);
  TGCGAIND		= new TRegByte(FPGAaddr + 0xDC);

  // Init Control
  FCC		      = new TRegByte(FPGAaddr + 0xFC);
  FCCDONE		= new TReg(FPGAaddr + 0xFC, eREGSIZE_RBW_BYTE, 0x08, 3);
  FCCSTATUS	   = new TReg(FPGAaddr + 0xFC, eREGSIZE_RBW_BYTE, 0x04, 2);
  FCCREADY		= new TReg(FPGAaddr + 0xFC, eREGSIZE_RBW_BYTE, 0x02, 1);
  FCCCONF		= new TReg(FPGAaddr + 0xFC, eREGSIZE_RBW_BYTE, 0x01, 0);
  FCCDATA		= new TRegByte(FPGAaddr + 0x00);
}

//------------------------------------------------------------------------
// DACNameToChannel
//------------------------------------------------------------------------
int AL4108::DACNameToChannel(char *s) {
  int tmp = -1;

  if (!strcmp(s, "A_InputOffset")) 
    tmp = 0;
  else if (!strcmp(s, "A_VideoBalance")) 
    tmp = 1;
  else if (!strcmp(s, "A_ADOffset")) 
    tmp = 2;
  else if (!strcmp(s, "A_ThresholdTrigRef")) 
    tmp = 3;

  else if (!strcmp(s, "B_InputOffset")) 
    tmp = 4;
  else if (!strcmp(s, "B_VideoBalance")) 
    tmp = 5;
  else if (!strcmp(s, "B_ADOffset")) 
    tmp = 6;
  else if (!strcmp(s, "B_ThresholdTrigRef")) 
    tmp = 7;

  else if (!strcmp(s, "C_InputOffset")) 
    tmp = 8;
  else if (!strcmp(s, "C_VideoBalance")) 
    tmp = 9;
  else if (!strcmp(s, "C_ADOffset")) 
    tmp = 10;
  else if (!strcmp(s, "C_ThresholdTrigRef")) 
    tmp = 11;

  else if (!strcmp(s, "D_InputOffset")) 
    tmp = 12;
  else if (!strcmp(s, "D_VideoBalance")) 
    tmp = 13;
  else if (!strcmp(s, "D_ADOffset")) 
    tmp = 14;
  else if (!strcmp(s, "D_ThresholdTrigRef")) 
    tmp = 15;

// dec 22, 04  SWS
  else if (!strcmp(s, "BNCTriggerRef"))
    tmp = (PCBREV->Get() != 0) ? 24 : 15;

  else if (!strcmp(s, "D_ClockSkew")) 
    tmp = 16;
  else if (!strcmp(s, "C_ClockSkew")) 
    tmp = 17;
  else if (!strcmp(s, "B_ClockSkew")) 
    tmp = 18;
  else if (!strcmp(s, "A_ClockSkew")) 
    tmp = 19;

  else if (!strcmp(s, "A_FineGain")) 
    tmp = 20;
  else if (!strcmp(s, "B_FineGain")) 
    tmp = 21;
  else if (!strcmp(s, "C_FineGain")) 
    tmp = 22;
  else if (!strcmp(s, "D_FineGain")) 
    tmp = 23;
  else {
    char t[255];
    sprintf(t, "Unknown D/A converter name:  %s", s);
    throw exDACError(t);
  }

  return tmp;
}

//------------------------------------------------------------------------
// SetDAC
//  Takes a DAC Name And Converts it to a Channel then Sets it
//------------------------------------------------------------------------
void AL4108::SetDAC(char *s, unsigned short value) {
  int Chan = DACNameToChannel(s);

  if ((Chan < 25) && (Chan >= 0)) {
    if (Chan < 8) {
      dacs[0]->SetDAC(Chan, value);
    } else if (Chan < 16) {
      dacs[1]->SetDAC(Chan - 8, value);
// dec 22, 04  SWS
    } else if (Chan < 20) {
      dacs[2]->SetDAC(Chan - 16, value);
    } else if (Chan < 24) {
      dacs[3]->SetDAC(Chan - 20, value);
    } else if (Chan == 24) {
      dacs[4]->SetDAC(0, value);
    } else {
      char t[255];
      sprintf(t, "Undefined D/A converter channel %i", Chan);
      printf("%s", t);
      throw exDACError(t);
    }
  }
}

//------------------------------------------------------------------------
// FifoOverFlow
//------------------------------------------------------------------------
bool AL4108::FifoOverFlow() {
  long fifostatus;
  ioctl(BoardHandle, IOCTL_GET_FIFOSTATUS, &fifostatus);
  return (fifostatus > 0) ? true : false;
}

//------------------------------------------------------------------------
// Wait for all DACS to Settle
//  This will find the greatest of the settle times and wait that amount
//------------------------------------------------------------------------
void AL4108::WaitForDACSettle() {
  int SettleTime = 0;

  for(int x=0;x < 5;x++) {
      if (dacs[x]->SettleTime > SettleTime)
          SettleTime = dacs[x]->SettleTime;
      dacs[x]->SettleTime = 0;
  }
  SettleTime=MIN(SettleTime,1000000);

  if (SettleTime > 0)
    usleep(SettleTime);
}

//------------------------------------------------------------------------
// Upload the Default RBF File Name
//------------------------------------------------------------------------
void AL4108::UploadDefault() {
  #define MAXRBFSIZE 900000
  char rbfdata[MAXRBFSIZE];
  char *filename;
  
  filename = "al4108_c.rbf";
  
  int infile = open(filename, O_RDONLY, 0);

  if (infile == -1) {
    printf("Can't open RBF File %s\n",filename);
    exit(1);
  }

  int count = 0;
  count = read(infile, rbfdata, MAXRBFSIZE);
  close(infile);

  UploadRBF(rbfdata, count);
}

//------------------------------------------------------------------------
// Use a PCi Register Access to Generate a Reliable Delay
//------------------------------------------------------------------------
void AL4108::RBFdelay() {
  for (int i=0; i < 8; i++) {
    FCCCONF->Get();
  }
}

//------------------------------------------------------------------------
//  Upload a RBF file to the board using Config Registers
//------------------------------------------------------------------------
#define AL4108_CONF_OFFSET	0xFC
#define AL4108_CONF_MASK	0x0D
#define AL4108_CONF_START	0x00
#define AL4108_CONF_NEXT	0x05
#define AL4108_CONF_ERROR	0x01
#define AL4108_CONF_DONE	0x0D

int AL4108::UploadRBF(char *data, long size) {
  // Point to the Start of the FPGA Registers  
  int i;
  char tmp;
                     
  // CONFIG to Low 
  FCCCONF->Set(0);
  usleep(800);
       
  tmp = FCC->Get();
  if ( (tmp & AL4108_CONF_MASK) != AL4108_CONF_START) {
    char s[255];
    sprintf(s,"<Upload4108RBF> Upload Failed to Start. %02X \n", (unsigned char)tmp);
    throw exHardware(s);
    return 0;
  }
                                 
  // CONFIG to High
  FCCCONF->Set(1);
  usleep(40);
                                        
  for(i = 0; i < size; i++) {
    tmp = FCC->Get();
    if ( (tmp & AL4108_CONF_MASK) == AL4108_CONF_NEXT) {
      int d = data[i] & 0xFF;
      FCCDATA->Set(d);
      RBFdelay();
    } else {  // NOT NEXT
      if ((FCC->Get() & AL4108_CONF_MASK) == AL4108_CONF_DONE) {
         return size;
      }
      char s[255];
      sprintf(s,"CONFIG not NEXT for byte %i during upload, %02X.\n",i,tmp);  
      throw exHardware(s);
    } 
  }
                                                                                          
  usleep(10);  // Delay before access registers

  tmp = FCC->Get();
  if ( (tmp & AL4108_CONF_MASK) == AL4108_CONF_DONE) {
    return size;
  } else if ( (tmp & AL4108_CONF_MASK) == AL4108_CONF_ERROR)
    throw exHardware("Error occured during upload");
    
  return 0;  
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
void AL4108::DEBUG()
{
    ioctl(BoardHandle, IOCTL_PRINTFPGA);    
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
int AL4108::GetBufferStatus()
{
  int retval;
  ioctl(BoardHandle, IOCTL_GETBUFFERSTATUS, &retval);
  return retval;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
void AL4108::ClearBufferStatus(int s)
{
  ioctl(BoardHandle, IOCTL_CLEARBUFFERSTATUS, s);
}

