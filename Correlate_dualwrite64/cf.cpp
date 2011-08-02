//-----------------------------------------------------------------------------
//title: Acquisition Logic A/D Board Kernel Driver
//version: Linux 0.1
//date:  2002/Aug/20
//designer: Michael Wyrick                                                   
//programmer: Michael Wyrick                                                
//platform: Linux 2.4.x
//language: GCC 2.95 and 3.0
//module: testali
//-----------------------------------------------------------------------------
//  Purpose: test transfers to the A/D Boards
//  Docs:                                  
//-----------------------------------------------------------------------------
// RCS:
// $Id: testfile.cpp,v 1.5 2002/11/19 15:07:35 mwyrick Exp $
// $Log: testfile.cpp,v $
// Revision 1.5  2002/11/19 15:07:35  mwyrick
// Standard Checkin before the 8100
//
// Revision 1.4  2002/11/05 20:34:54  mwyrick
// Works
//
// Revision 1.3  2002/10/23 14:57:04  mwyrick
// Works and Reads file size
//
// Revision 1.2  2002/10/23 14:26:14  mwyrick
// Seems to Work to Check Files
//
// Revision 1.1.1.1  2002/10/23 14:17:55  mwyrick
// TEst Program
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

#define BUFSIZE  (16 * 1000 * 1000)
#define TESTSIZE (16 * 1000 * 1000)

int StartOffset = 0x01;

int ErrorFound = 0;
char data[ BUFSIZE ];

//------------------------------------------------------------------------
// TestAddresses
//------------------------------------------------------------------------
void TestAddresses(int a, int Count)
{
  int *pint = (int *)data;
  int loc = a % BUFSIZE;
  int d;

  if (!ErrorFound) {
    for(int x=0; x < Count; x+=4) {
      if ( pint[(loc + x)/4] != ((a + x) / 4) + StartOffset) {
        if (!ErrorFound) {
          printf("DATA ERROR at Address %08X, Data=%08X, ShouldBe=%08X\n", 
               a + x, pint[(loc + x)/4], ((a+x)/4)+StartOffset);
          printf("Last Good Address Tested %08X, Megs=%i\n", a+x, ((a+x) / (1024 * 1024)));
          d  = (pint[(loc + x)/4] - ((a+x)/4)+StartOffset) * 4;
          printf("Difference is 0x%08X, %i, (loops thru buffer) = %i\n", d, d, d / TESTSIZE);
        }
        ErrorFound = 1;
        break;
      }
    }
  }
}

//------------------------------------------------------------------------
// DoTest
//------------------------------------------------------------------------
void DoTest()
{
  int loc = 0;
  int fout;
  int addr = 0;
  // Open Output File
  printf("Opening file\n");
  fout = open("Testfile_0.dat", O_RDONLY);

  if (fout < 0) {
    printf("Error during file open!\n");
    exit(1);
  }

  if (StartOffset > 0) {
    printf("Starting Offset is %08X\n", StartOffset);
  }

  struct stat st;
  fstat(fout, &st);

  loc = st.st_size;
  printf("Testing %i Bytes\n", loc);

  while(loc > 0) {
    int rc = read(fout, data, BUFSIZE);
    TestAddresses(addr, rc);     
    addr += rc;
    loc  -= rc;
  }

  printf("Last Address Tested %08X, Megs=%i\n", addr, (addr / (1024 * 1024)));

  close(fout);
}

//------------------------------------------------------------------------
// main
//------------------------------------------------------------------------
int main(int argc, char *argv[])
{
  DoTest();

  if (!ErrorFound) {
    printf("Data Integrity has been Validated\n");
  }
  return(0);
}
