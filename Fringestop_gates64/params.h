#ifndef PARAMS_H
#define PARAMS_H

#define PNT_RA 5.847677405 //B2217+47 June 29, 2008
#define PNT_DEC 0.8371095282
//#define PNT_RA 2.2139692 //B0823+26 Feb 17, 2009                                                                               
//#define PNT_DEC 0.46413228
//#define PNT_RA 2.2139270 //B0823+26 Dec 20, 2008
//#define PNT_DEC 0.46414174 
//#define PNT_RA  2.2138397  //B0823+26 Aug 20, 2008
//#define PNT_DEC 0.46416103
//#define PNT_RA 5.8475253 //B2217+47 Aug 19, 2008
//#define PNT_DEC 0.83703426
//#define PNT_RA 5.8474053 //B2217+47 Dec 18, 2007
//#define PNT_DEC 0.83697487
//#define PNT_RA 2.2136608  //B0823+26 Dec 15,2007
//#define PNT_DEC 0.46420079
//#define PNT_RA 1.4975653  //3c147
//#define PNT_DEC 0.87013877
//#define PNT_RA 2.163884  //B0809+74
//#define PNT_DEC 1.2995778 
//#define PNT_RA 2.2136542 // B0823+26  Dec 7, 2007
//#define PNT_DEC 0.46420220
//#define PNT_RA 2.2580708  //B0834+06        
//#define PNT_DEC 0.107214
//#define PNT_RA 2.2863116  //J0843+0719
//#define PNT_DEC 0.127139
#define EPOCH_START (0)
#define T_CUTOFF 0
#define F_CUTOFF 0
#define NF 2048
#define FFTLEN 4096
#define FFTLENOUT 256
//#define NPOD 60 // changes by Ue-Li, June 17, 2011
#define NPOD 64
#define NNODE 16
#ifndef NEPOCH
#define NEPOCH 400
#endif
#define NGATE 16
//#define FOLDSIN 
#define ONEGATE 1
#define NTBIN 2
#define NCORR ((NPOD)*(NPOD+1)/2)
#define NCROSS (FFTLEN/NNODE*NCORR)
#define NEPOCH_READ (EPOCH_START+NEPOCH)
//#define WRITELOCAL

#endif
