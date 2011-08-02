//-----------------------------------------------------------------------------
//title: Acquisition Logic A/D Board Kernel Driver Checkout Program
//version: Linux 1.0
//date:  March 2004
//designer: Michael Wyrick                                                   
//programmer: Michael Wyrick                                                
//platform: Linux 2.4.x
//language: GCC 3.3.1
//module: waveform.cpp
//-----------------------------------------------------------------------------
//  Purpose: test transfers to the A/D Boards
//  Docs:                                  
//-----------------------------------------------------------------------------
// CVS:
// $Id: dual.cpp,v 1.1 2005/01/03 16:41:17 mwyrick Exp $
// $Log: dual.cpp,v $
// Revision 1.1  2005/01/03 16:41:17  mwyrick
// Check in of Working Version
//
// Revision 1.10  2004/12/29 15:20:58  mwyrick
// Preparing to send to Pen
//
// Revision 1.9  2004/12/28 22:02:13  mwyrick
// Working with BTERM and no FASTTERM in driver
//
// Revision 1.8  2004/12/28 16:36:14  mwyrick
// Working
//
// Revision 1.7  2004/12/23 20:36:15  mwyrick
// Serial Number Support
//
// Revision 1.6  2004/12/23 20:27:11  mwyrick
// Added Serial Number
//
// Revision 1.5  2004/12/23 19:59:44  mwyrick
// Working on 400 and 200 MHz mode
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
// Revision 1.1.1.1  2004/12/16 14:27:27  mwyrick
// ALLinux 2004
//
// Revision 1.2  2004/04/29 19:12:15  mwyrick
// Added  and  to Header
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
#include <sys/time.h>
#include <sys/timeb.h>
#include <string.h>

#include "aldriver.h"
#include "Board4108.h"
#include "BoardEnum.h"
#include "reg.h"

#ifdef PARALLEL
#include <iostream>
#include "mpi.h"
int rank,size;
extern "C" {
#include "xfft.h"
}
#endif

#define PROC_DATA_FORMAT 1001

//extern long BufferSize;

#define PROFILE
//#undef PROFILE

const unsigned int AcqSize = NT;

int AcqNum = 0;
int WhichBuffer = 0;
long amountofdata = 0;
struct timeb start, current;
MPI_Comm mpi_acq_comm;
struct timeval tvlocal; struct timezone tzlocal;


//------------------------------------------------------------------------
// This gets called for data returned from the board
//------------------------------------------------------------------------
void WriteDataLong(FILE *outfile, unsigned char *data, long size) {
  int x;
  for(x = 0; x < size; x+=4) {
    fprintf(outfile, "%c", data[x]);
    fprintf(outfile, "%c", data[x+1]);
    fprintf(outfile, "%c", data[x+2]);
    fprintf(outfile, "%c", data[x+3]);
  }
}

//------------------------------------------------------------------------
// This gets called for data returned from the board
//------------------------------------------------------------------------
void WriteDataOneChannel8Bit(FILE *outfile, unsigned char *data, long size, int chan) {
  int x;

  for(x = 0; x < size; x+=4) {
    fprintf(outfile, "%i\n", (char) data[x + chan]);
  }
}

//------------------------------------------------------------------------
// This gets called for data returned from the board
//------------------------------------------------------------------------
void WriteData8Bit(FILE *outfile, unsigned char *data, long size) {
  int x;

  for(x = 0; x < size; x++) {
    fprintf(outfile, "%i\n", (char) data[x]);
  }
}

//------------------------------------------------------------------------
// This gets called for data returned from the board
//------------------------------------------------------------------------
void WriteData4Bit200(FILE *outfile, unsigned char *data, long size) {
  int x;

  for(x = 0; x < size; x++) {
    char t = (char)data[x] & 0x0F; 
    if (t & 0x08) {  // Sign extend
      t |= 0xF0;
    }
    fprintf(outfile, "%i\n", t);
    
    t = (char)data[x] & 0xF0 >> 4; 
    if (t & 0x08) {  // Sign extend
      t |= 0xF0;
    }
    fprintf(outfile, "%i\n", t);
  }
}

//------------------------------------------------------------------------
// This gets called for data returned from the board
//------------------------------------------------------------------------
void WriteData2Bit400(FILE *outfile, unsigned char *data, long size) {
  int x;

  for(x = 0; x < size; x++) {
    char t = ((char)data[x]) & 0x03; 
    if (t & 0x02) {  // Sign extend
      t |= 0xFC;
    }
    fprintf(outfile, "%i\n", t);
    
    t = ((char)data[x] & 0x0C) >> 2; 
    if (t & 0x02) {  // Sign extend
      t |= 0xFC;
    }
    fprintf(outfile, "%i\n", t);

    t = ((char)data[x] & 0x30) >> 4; 
    if (t & 0x02) {  // Sign extend
      t |= 0xFC;
    }
    fprintf(outfile, "%i\n", t);

    t = ((char)data[x] & 0xC0) >> 6; 
    if (t & 0x02) {  // Sign extend
      t |= 0xFC;
    }
    fprintf(outfile, "%i\n", t);
  }
}

//------------------------------------------------------------------------
// This gets called for data returned from the board
//------------------------------------------------------------------------
void WriteData2Chan2Bit200(FILE *outfile, FILE *outfile2, unsigned char *data, long size) {
  int x;

  for(x = 0; x < size; x++) {
    char t = (char)data[x] & 0x03; 
    if (t & 0x02) {  // Sign extend
      t |= 0xFC;
    }
    fprintf(outfile, "%i\n", t);
    
    t = ((char)data[x] & 0x0C) >> 2; 
    if (t & 0x02) {  // Sign extend
      t |= 0xFC;
    }
    fprintf(outfile2, "%i\n", t);

    t = ((char)data[x] & 0x30) >> 4; 
    if (t & 0x02) {  // Sign extend
      t |= 0xFC;
    }
    fprintf(outfile, "%i\n", t);

    t = ((char)data[x] & 0xC0) >> 6; 
    if (t & 0x02) {  // Sign extend
      t |= 0xFC;
    }
    fprintf(outfile2, "%i\n", t);
  }
}

//------------------------------------------------------------------------
// This gets called for data returned from the board
//------------------------------------------------------------------------
void WriteData400(FILE *outfile, unsigned char *data, long size) {
    int x;
    for(x = 0; x < size; x+=4) {
      fprintf(outfile, "%i\n",(char)data[x]);
      fprintf(outfile, "%i\n",(char)data[x+1]);
      fprintf(outfile, "%i\n",(char)data[x+2]);
      fprintf(outfile, "%i\n",(char)data[x+3]);
    }
}

//------------------------------------------------------------------------
// This gets called for data returned from the board
//------------------------------------------------------------------------
void WriteData200(FILE *outfile, unsigned char *data, long size, int chan) {
    int x;
    for(x = 0; x < size; x+=4) {
      fprintf(outfile, "%i\n",(char)data[x + chan]);
      fprintf(outfile, "%i\n",(char)data[x+2 + chan]);
    }
}

//------------------------------------------------------------------------
//
// This gets called for data returned from the board
//
//------------------------------------------------------------------------
void ProcessData(FILE *outfile, unsigned char *data, long size, int format, int chan) {
  //
  // ADD ANY PROCESSING HERE
  //
  switch (format) {
  case 0:
      //WriteDataLong(outfile, data, size);
      WriteDataOneChannel8Bit(outfile, data, size, chan);
      break;
  case 1:
      WriteDataLong(outfile, data, size);
      break;
  case 0x10:
      WriteData8Bit(outfile, data, size);
      break;
  case 0x011C:
      WriteData4Bit200(outfile, data,size);
      break;
  case 0x0204:
      WriteData2Bit400(outfile, data,size);
      break;
  case 0x0200:
      WriteData400(outfile, data, size);
      break;
  case 0x0100:
      WriteData200(outfile, data, size, chan);
      break;
#ifdef PARALLEL
	case 1001:
                        corr_driver((signed char*)data,size,mpi_acq_comm);
			break;
#endif
//  case 0x0104:
//      WriteData2Chan2Bit200(outfile, data, size);
//      break;
  }
}

//------------------------------------------------------------------------
// Wait for the Board to be ready for a Trigger
//------------------------------------------------------------------------
void WaitForBoardToBeReady(AL4108 *brd) {
  while(brd->TRIGREADY->Get() == 0) {
    usleep(1);
  }
}

//------------------------------------------------------------------------
// 
//------------------------------------------------------------------------
void DoTrigger(AL4108 *brd) {
  brd->TriggerAcq();          // Trigger the Next Acquisition
}

//------------------------------------------------------------------------
// 
//------------------------------------------------------------------------
bool DoOneBuffer(AL4108 *brd)
{
  bool retval = true;
  int ClearWhichBuffer;
  // Process DMA Data
  // Lets check the buffer status to see if there is data already

  usleep(100);
  int s = brd->GetBufferStatus();
  printf("rank %d Buffer Status is %02X\n", rank,s);
  if (s == 3) {
	  printf("error: both buffers full!\n");
//	retval = false;
//	return;
  }
  int b = 1 << WhichBuffer;
  ClearWhichBuffer = WhichBuffer;

  if ((s & b) != b ) {
	struct timeb start, current;
      ftime( &start );
      brd->WaitForDMAInterrupt(); // Wait for the Data to Finish
      s = brd->GetBufferStatus();
      ftime( &current );
      double sec = (((current.time - start.time) * 1000) + (current.millitm - start.millitm)) / 1000.0;
      printf("rank %d  After Waiting %f sec, Buffer Status is %02X\n",rank, sec,s);
  } else {
//      printf("Skipping Wait because data is already present.\n");
  }

  amountofdata += AcqSize/1000;

#ifdef PROFILE
  // Get the current time for profiling
  ftime( &current );

  double sec = (((current.time - start.time) * 1000) + (current.millitm - start.millitm)) / 1000.0;
  double mbs = (amountofdata / sec) / (1000 * 1);

  printf("rank %d Transfered %li MB in %0.3f seconds, Rate is %0.3f MB/s\n", rank,amountofdata / 1000, sec, mbs);

#ifdef PARALLEL
  FILE *outfile; 
  unsigned char *d = brd->Data;
#endif
	if (WhichBuffer == 0) {
#ifdef PARALLEL
    ProcessData(outfile, d, AcqSize, PROC_DATA_FORMAT, 0);
#endif
    WhichBuffer = 1;
  } else {
#ifdef PARALLEL
    ProcessData(outfile, &d[AcqSize], AcqSize, PROC_DATA_FORMAT, 0);
#endif
    WhichBuffer = 0;
  }

#else
  unsigned char *d = brd->Data;
  
  // Process the Data
  char st[255];
  sprintf(st, "Testfile_%i.bin", AcqNum);
  FILE *outfile = fopen(st, "w");
  if (WhichBuffer == 0) {
    ProcessData(outfile, d, AcqSize, PROC_DATA_FORMAT, 0);
    WhichBuffer = 1;
  } else {
    ProcessData(outfile, &d[AcqSize], AcqSize, PROC_DATA_FORMAT, 0);
    WhichBuffer = 0;
  }
  fclose(outfile);

#endif // PROFILE
  
  brd->ClearBufferStatus(ClearWhichBuffer);
  
  AcqNum++;
  //
  // Check for a DMA Timeout
  if (brd->DMAStatus() == -2) {
    printf("rank %d DMA Timed out Waiting for Interrupt.\n",rank);
    retval = false;
  }


  // Check for A/D Board Buffer OverFlow
  if (brd->FifoOverFlow()) {
    printf("rank %d The Board Fifo Overflowed, Corrupted Data.\n",rank);
    brd->OFOVERFLOW->Set(1);
//    retval = false;
  }

  if (brd->TRIGREADY->Get() == 1) {
    if (retval) {
    }
  } else {
    printf("rank %d Board not ready when we are for Trigger.\n",rank);
    WaitForBoardToBeReady(brd);


  }
#ifdef PARALLEL
//  MPI::COMM_WORLD.Barrier();
  MPI_Barrier(mpi_acq_comm);
        tv=tvlocal;
        if( gettimeofday(&tvlocal, &tzlocal) < 0 ) perror("gettimeofday");
	if (rank==MASTERNODE) {
  		//DoTrigger(brd);
	}
        DoTrigger(brd);
#else
    if( gettimeofday(&tvlocal, &tzlocal) < 0 ) perror("gettimeofday");
#endif



  return retval;
}

//------------------------------------------------------------------------
// Setup Acquisition
//------------------------------------------------------------------------
void SetupAcq(AL4108 *brd, int loops) 
{ 
#ifdef PARALLEL
	printf("rank %d in TRIGGER\n",rank);
#endif

  printf("Setting up Acquisition\n");
  // Setup and Run an Acquisition
  
  brd->AMODE->Set(0x30);  // Set to ABCD 25/33MHz
  brd->DECIMATE3->Set(1); // Set to Decimate by 3 (133MHz Mode)
//  brd->DECIMATE3->Set(0); // Set to Decimate by 4 (100MHz Mode)

  brd->SetupAcq(AcqSize / 4);
  brd->EnableAcq();

  printf("Setting up DMA\n");

  // Turn on the Circle Buffer with two Interrupts
  brd->TurnOnOffCircleBuffer(1, 2);  

  brd->ClearBufferStatus(0);
  brd->ClearBufferStatus(1);

  // Turn on the DMA with two buffers
  brd->StartDMA(AcqSize * 2);  
 
#ifdef PARALLEL
//	MPI::COMM_WORLD.Barrier();
  MPI_Barrier(mpi_acq_comm);
  if (rank==MASTERNODE) {
    printf("Sending First Trigger, rank %d\n",rank);
        if( gettimeofday(&tv, &tz) < 0 ) perror("gettimeofday");
		//DoTrigger(brd);
	}
		DoTrigger(brd);
#else
  printf("Sending First Trigger\n");
  // Trigger the First Acquisition
  DoTrigger(brd);
#endif

#ifdef PROFILE
  // Get the Current Time for Profiling
  ftime( &start );
#endif
  
  WaitForBoardToBeReady(brd);
  //MPI::COMM_WORLD.Barrier();
  MPI_Barrier(mpi_acq_comm);
  WaitForBoardToBeReady(brd);
//      printf("rank %d bufferstatus=%d\n",rank,brd->GetBufferStatus());
#ifdef PARALLEL
  //MPI::COMM_WORLD.Barrier();
  MPI_Barrier(mpi_acq_comm);
  if (rank==MASTERNODE) {
	  printf("Sending Second Trigger, rank %d\n",rank);
        if( gettimeofday(&tvlocal, &tzlocal) < 0 ) perror("gettimeofday");
	  //DoTrigger(brd);
	}
	  DoTrigger(brd);
#else	
  printf("Sending Second Trigger\n");
  // Trigger the Second Acquisition
  DoTrigger(brd);
#endif

  if (brd->GetPLLStatus() != true) {
    printf("rank %d ERROR: PLL on A/D Board is Not Locked!\n",rank);
  }


  // Process some number of acquisitions
  for(int i=0; i < loops; i++) {
     if (DoOneBuffer(brd) == false)
        break;
  }

  // End the DMA
  brd->EndDMA();

  // End the Acquisition
  brd->EndAcq();
}
int CalcBNCTriggerRange(int mV) {
  int retval = 0;
  //              if (hardware.FPGA.BoardRevision == 0)
  //                      retval = 2048 - (int)(4096 * (double)(mV / 8070.0));
  //              else
  retval = 2048 + (int)(mV / 3.0);
  if (retval < 0)
    retval = 0;
  if (retval > 4095)
    retval = 4095;
  return retval;
}


//------------------------------------------------------------------------
// main
//------------------------------------------------------------------------
int main(int argc, char *argv[])
{
//  try {
    printf("Software Correlator\n");
    printf("Copyright 2004-2007, Acquisition Logic, University of Toronto\n");

    outfile="/dev/null";
    if (argc == 2) {
	    outfile=argv[1];
    	    printf("output written to %s\n",outfile);
    }


    // Create a New AL4108 Board Object
    // Pass true to the constructor to upload the RBF file, this only has
    // to be done the first time after a power cycle.  Pass false to skip
    // the RBF file upload.  
    AL4108 *Board;
    MPI_Group mpi_acq_group;
    MPI_Group mpi_group;
    int ranges[1][3]={NNODECORR,NNODECORR+NNODEACQ-1,1};
    
    int BoardNumber = 0;  // Set This to the Board Number to Use

#ifdef PARALLEL
		printf("MPI version 2005, CITA\n");
#if 0
    	MPI::Init(argc,argv);
	size=MPI::COMM_WORLD.Get_size();
	rank=MPI::COMM_WORLD.Get_rank();
	std::cout << "rank:\t" << rank << "\tof:\t" << size << std::endl;
#else
    MPI_Init(&argc,&argv);
    int ierr=MPI_Comm_size(MPI_COMM_WORLD,&size);
    if (ierr != 0) printf("error finding comm size=%d\n",ierr);
    ierr=MPI_Comm_rank(MPI_COMM_WORLD,&rank);
    if (ierr != 0) printf("error finding comm rank=%d\n",ierr);
    printf("rank: %d of %d\n",rank,size);
#endif
#endif
    MPI_Comm_group(MPI_COMM_WORLD, &mpi_group);
    MPI_Group_range_incl(mpi_group,1,ranges,&mpi_acq_group);
    MPI_Comm_create(MPI_COMM_WORLD,mpi_acq_group,&mpi_acq_comm);
    int loops = 3600*4; // 1 hour
//    int loops = 2400;
    if (rank>=NNODECORR) {
    BoardEnum *brdinfo = new BoardEnum();
    printf("BoardEnum reports %i board(s) in the system\n", brdinfo->NumberOfBoards);
    for(int x=0; x < brdinfo->NumberOfBoards;x++) {
        printf("  Board %i Type is %s\n", x, brdinfo->TypeToString(brdinfo->BoardTypes[x]));
    }
 
    Board = new AL4108(false, BoardNumber);
    printf("rank %d Board is Serial Number %i\n",rank, Board->SerialNumber());

    // Print the FPGA Version
    int ver = Board->FPGAVERSION->Get();
    printf("Version = %02X:%02X\n", (ver & 0xFF00) >> 8, ver & 0x00FF);

    printf("Channel   Master\n");
    printf("----------------\n");
    for(int c=0; c<16; c++)
      printf("  %02i     %7.3f\n", c, Board->ReadTestAD(c));

    Board->ENCDATA->Set(0x0000);
    usleep(100);
    
    // Setup the Number of Channels
    int numchan = 4;
    switch (numchan) {
    case 1 :
      Board->AINRANGE->Set(0);               // 0=1V,1=500mV,2=250mV,3=125mV
      Board->SetUpClockSelectregs(0x200);
      break;
    case 2 :
      Board->AINRANGE->Set(0);               // 0=1V,1=500mV,2=250mV,3=125mV
      Board->BINRANGE->Set(0);               // 0=1V,1=500mV,2=250mV,3=125mV
      Board->SetUpClockSelectregs(0x100);
      break;
    case 4 :
      Board->SetUpClockSelectregs(0);
      //int again=150;
        //int again=180;  // for no equalized
        //int again1=255;  // for no equalized
	//Board->SetUpTGC(again,again,again,again);
      FILE *gtab = fopen("/home/gsbuser/gain/gaintab.dat","r");
      int gain[4];
      for(int j=0; j<4; j++)
        fscanf(gtab,"%d", &gain[j]);
      Board->SetUpTGC(gain[0],gain[1],gain[2],gain[3]);

      break;
    }

    // Set the A Channel Params
    Board->AACCOUPLE->Set(0);              // AC Coupled, 0=on, 1=off
    Board->SetInputOffset(0, 0);           // Setup the Input Offset

    // Set the B Channel Params
    Board->BACCOUPLE->Set(0);              // AC Coupled, 0=on, 1=off
    Board->SetInputOffset(1, 0);           // Setup the Input Offset

    // Set the C Channel Params
    Board->CACCOUPLE->Set(0);              // AC Coupled, 0=on, 1=off
    Board->SetInputOffset(2, 0);           // Setup the Input Offset

    // Set the D Channel Params
    Board->DACCOUPLE->Set(0);              // AC Coupled, 0=on, 1=off
    Board->SetInputOffset(3, 0);           // Setup the Input Offset

#ifdef PARALLEL
		if (rank==MASTERNODE) {
			printf("master board, rank: %d\n",rank);
#endif
		  	//Board->CMS->Set(3);  // 3 -> 5 MHz clock in
#ifdef OLDCLOCK
         std::cout<<"using old clock settings"<<std::endl;
		  	Board->CMS->Set(3);  // 3 -> 5 MHz clock in
#else
		  	//Board->CMS->Set(1);  // 1 -> 5 MHz clock out
		  	Board->CMS->Set(3);  // 3 -> 5 MHz clock in
#endif
      			Board->TRIGPOL->Set(0); // 1=non-inverted
			Board->TRIGSRC->Set(0);  // 0=software
			//Board->TRIGSRC->Set(2);   // 2=BNC input

		/* use the following trigger in production */
			//Board->TRIGSRC->Set(5);   // 6=PLL sync, rising
			Board->BNCTERM->Set(1);  // 1=on
			Board->BNCOUTEN->Set(1);  // 1=output
			Board->BNCOUT->Set(3);   // 3= totem pole
			Board->BNCSRC->Set(2);	// 2=trigger sync to falling PLL edge 
			//Board->BNCSRC->Set(0);	
			Board->BNCPOL->Set(0);  // 1=non-inverted
			Board->SMCOUT->Set(3);	// 3=totem-pole
			Board->SMCPOL->Set(0);	// 0=inverted
			Board->SMCSRC->Set(0);	// 0=software
			Board->SetDAC("BNCTriggerRef", CalcBNCTriggerRange(1250));
#ifdef PARALLEL
		}
		else {
			printf("slave board, rank: %d\n",rank);
#ifndef OLDCLOCK
			Board->CMS->Set(3);  // 3 -> 5 MHz clock in
#endif
			Board->TRIGPOL->Set(1); 
			//Board->TRIGSRC->Set(5);// PLL sync
			Board->TRIGSRC->Set(0); // software
			//Board->TRIGSRC->Set(2);  // external
			//Board->TRIGSRC->Set(1);  // internal SMC
			Board->BNCTERM->Set(1);
		//	if (rank == size-1) Board->BNCTERM->Set(1);
			Board->BNCOUTEN->Set(0);
			Board->BNCOUT->Set(0);
			Board->BNCPOL->Set(1);
			Board->SMCOUT->Set(0);
			Board->SetDAC("BNCTriggerRef", CalcBNCTriggerRange(1250));
			//Board->SetDAC("BNCTriggerRef", CalcBNCTriggerRange(750));
		}
#endif
    // Setup and Run
    system("date");
    SetupAcq(Board, loops);

    delete Board;
  }  else {
    for(int i=0; i < loops; i++) 
    corr_driver((signed char*)NULL,NT,mpi_acq_comm);
  }
/*
  } catch (exHardware ex) {
    printf(ex.s);
  } catch (...) {
    printf("rank %d: An Exception was Thrown\n",rank);
  }
 */

  return(0);
}
