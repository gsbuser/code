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
// $Id: Board4108.h,v 1.1 2005/01/03 16:41:16 mwyrick Exp $
// $Log: Board4108.h,v $
// Revision 1.1  2005/01/03 16:41:16  mwyrick
// Check in of Working Version
//
// Revision 1.8  2004/12/28 16:36:14  mwyrick
// Working
//
// Revision 1.7  2004/12/23 20:27:11  mwyrick
// Added Serial Number
//
// Revision 1.6  2004/12/23 19:39:57  sschulz
// added TGC registers and TGC set without profile, forced 100MS/sec to TGC path on all channels
//
// Revision 1.5  2004/12/23 19:18:51  sschulz
// merged in changes to DAC setting and use of calibration data
//
// Revision 1.4  2004/12/23 18:55:18  mwyrick
// Added BoardEnum to report the number of board in the system.
//
// Revision 1.3  2004/12/23 16:12:47  mwyrick
// Working Well with StartAcq and then Multiple DMAs to collect data.
//
// Revision 1.2  2004/12/23 14:51:10  mwyrick
// Multi Board seems to work.  Added Max5841 and Max504.  They are in but untested
//
// Revision 1.1.1.1  2004/12/16 14:27:28  mwyrick
// ALLinux 2004
//
// Revision 1.2  2004/04/29 19:12:04  mwyrick
// Added  and  to Header
//
//
//-----------------------------------------------------------------------------
#ifndef al4108H
#define al4108H

#include "aldriver.h"
#include "reg.h"
#include "Dacs.h"
#include "II2C.h"
#include "EEPROM_24xx64.h"
#include "Exceptions.h"

//----------------------------------------------------------------------
//
// AL4108 Class
//
//----------------------------------------------------------------------
class AL4108 {
  int BoardHandle;
  int OldAcqMode;
  
  IDAC *dacs[5];
  II2C *i2cBus;

  bool ProgramPLL(int);
  void SetupRegisters(unsigned char *FPGAaddr);
  unsigned short GetEEPROMShort(int address);
  void RBFdelay();
  void UploadDefault();
  
  // Private FPGA Registers
  // Init Control
  TReg *FCC;
  TReg *FCCDONE;
  TReg *FCCSTATUS;
  TReg *FCCREADY;
  TReg *FCCCONF;
  TReg *FCCDATA;

public:
  unsigned char *Data;
  unsigned char *FPGAaddr;
  EEPROM_24xx64 *userEEPROM;
  unsigned int SerialNumber();

	// Setup Registers
    TReg *ENCDATA;
    TReg *ENCHI;
    TReg *MCR;
    TReg *FIFOCLR;
    TReg *GENABLE;

    TReg *FOVERRUN;
    TReg *WDGLEN;

    TReg *DATA;
    TReg *TIMER;
    TReg *TRIGCNT;
    TReg *DMACNT;
    TReg *XFERCNT;
    TReg *FPGAVERSION;

    TReg *ACR;
    TReg *TRIGREADY;
    TReg *DECIMATE3;
    TReg *GENCLR;
    TReg *AENABLE;

    TReg *AMR;
    TReg *OFOVERFLOW;
    TReg *OFEMPTY;
    TReg *ACS;
    TReg *AMODE;

    TReg *FINEPOST;
    TReg *CPOST;
    TReg *CACQLEN;
    TReg *ACQLEN;
    TReg *NUMWF;

	TReg *SWTRIG;

	TReg *TRIGPOL;
	TReg *TRIGSRC;

	TReg *BNCTERM;
	TReg *BNCOUTEN;
	TReg *BNCOUT;
	TReg *BNCPOL;
	TReg *BNCSRC;

	TReg *DACBUSY;
	TReg *DACSEL;
	TReg *DACDATA;
	TReg *SMCOUT;
	TReg *SMCPOL;
	TReg *SMCSRC;
    TReg *DACSEL2;

	TReg *PCBREV;

	TReg *AINRANGE;
	TReg *AACCOUPLE;
	TReg *A50TERM;
	TReg *AADSRC;

	TReg *BINRANGE;
	TReg *BACCOUPLE;
	TReg *B50TERM;
	TReg *BADSRC;

	TReg *CACCOUPLE;
	TReg *C50TERM;
	TReg *CADSRC;

	TReg *DACCOUPLE;
	TReg *D50TERM;
	TReg *DADSRC;

	TReg *CCR;
	TReg *PLLSTATUS;
	TReg *PLLLATCH;
	TReg *PLLDATA;
	TReg *PLLCLK;
	TReg *ADIMODE;
	TReg *CMS;

    TReg *I2CCMD;
    TReg *I2CDATA;

    TReg *ADCSEL;
    TReg *ADCBUSY;
    TReg *ADCDATA;

    TReg *TGCRESET;		
    TReg *TGCENABLE;	
    TReg *TGCFOLLOW;	
    TReg *TGCAPH;		
    TReg *TGCAPTW;		
    TReg *TGCAPLEN;		
    TReg *TGCAPADDR;	
    TReg *TGCAPGAIN;	
    TReg *TGCGAINA;		
    TReg *TGCGAINB;		
    TReg *TGCGAINC;		
    TReg *TGCGAIND;		

	AL4108(bool, int);  // Constructor
	~AL4108(); // Destructor
  
    void SetInputOffset(int Chan, short value);
    void SetUpTGC(char GainA, char GainB, char GainC, char GainD);
    void SetUpClockSelectregs(int AcqMode);

	void SetupAcq(int Samples);
	void EnableAcq();
	void TriggerAcq();
	void EndAcq();

    void StartDMA(int Size);
	void WaitForDMAInterrupt();
	void EndDMA();

	long DMAStatus();
	long BoardStatus();
	bool FifoOverFlow();
	int DACNameToChannel(char *s);
	void SetDAC(char *s, unsigned short value);
	void WaitForDACSettle();
    int UploadRBF(char *data, long size);

    void TurnOnOffCircleBuffer(int, int);
    void DEBUG();

    int GetBufferStatus();
    void ClearBufferStatus(int);

    bool GetPLLStatus(void);
    double ReadTestAD(int channel);

};

#endif
