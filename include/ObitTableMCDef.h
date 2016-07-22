/* $Id:  $   */
/* DO NOT EDIT - file generated by ObitTables.pl                      */
/*--------------------------------------------------------------------*/
/*;  Copyright (C)  2010                                              */
/*;  Associated Universities, Inc. Washington DC, USA.                */
/*;                                                                   */
/*;  This program is free software; you can redistribute it and/or    */
/*;  modify it under the terms of the GNU General Public License as   */
/*;  published by the Free Software Foundation; either version 2 of   */
/*;  the License, or (at your option) any later version.              */
/*;                                                                   */
/*;  This program is distributed in the hope that it will be useful,  */
/*;  but WITHOUT ANY WARRANTY; without even the implied warranty of   */
/*;  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the    */
/*;  GNU General Public License for more details.                     */
/*;                                                                   */
/*;  You should have received a copy of the GNU General Public        */
/*;  License along with this program; if not, write to the Free       */
/*;  Software Foundation, Inc., 675 Massachusetts Ave, Cambridge,     */
/*;  MA 02139, USA.                                                   */
/*;                                                                   */
/*;Correspondence about this software should be addressed as follows: */
/*;         Internet email: bcotton@nrao.edu.                         */
/*;         Postal address: William Cotton                            */
/*;                         National Radio Astronomy Observatory      */
/*;                         520 Edgemont Road                         */
/*;                         Charlottesville, VA 22903-2475 USA        */
/*--------------------------------------------------------------------*/
/*  Define the basic components of the ObitTableMC  structure          */
/*  This is intended to be included in a class structure definition   */
/**
 * \file ObitTableMCDef.h
 * ObitTableMC structure members for derived classes.
 */
#include "ObitTableDef.h"  /* Parent class definitions */
/** Observation code */
gchar  obscode[MAXKEYCHARTABLEMC];
/** The number of polarizations. */
oint  numPol;
/** Reference date as "YYYYMMDD" */
gchar  RefDate[MAXKEYCHARTABLEMC];
/** The number of Stokes(?) */
oint  numStkd;
/** First Stokes(?) */
oint  stk1;
/** The number of Bands(?). */
oint  numBand;
/** The number of spectral channels. */
oint  numChan;
/** Reference Frequency. */
odouble  refFreq;
/** Channel bandwidth. */
ofloat  chanBW;
/** Reference Pixel.. */
ofloat  refPixl;
/** FFT size. */
oint  FFTSize;
/** Oversampling factor */
oint  oversamp;
/** Zero padding factor */
oint  zeroPad;
/** Tapering function */
gchar  taperFn[MAXKEYCHARTABLEMC];
/** Revision number of the table definition. */
oint  revision;
/** Column offset for The center time. in table record */
olong  TimeOff;
/** Physical column number for The center time. in table record */
olong  TimeCol;
/** Column offset for Source ID number in table record */
olong  SourIDOff;
/** Physical column number for Source ID number in table record */
olong  SourIDCol;
/** Column offset for Antenna number in table record */
olong  antennaNoOff;
/** Physical column number for Antenna number in table record */
olong  antennaNoCol;
/** Column offset for Array number in table record */
olong  ArrayOff;
/** Physical column number for Array number in table record */
olong  ArrayCol;
/** Column offset for Frequency ID in table record */
olong  FreqIDOff;
/** Physical column number for Frequency ID in table record */
olong  FreqIDCol;
/** Column offset for Atmospheric delay in table record */
olong  atmosOff;
/** Physical column number for Atmospheric delay in table record */
olong  atmosCol;
/** Column offset for Time derivative of ATMOS in table record */
olong  DatmosOff;
/** Physical column number for Time derivative of ATMOS in table record */
olong  DatmosCol;
/** Column offset for Group delay in table record */
olong  GDelayOff;
/** Physical column number for Group delay in table record */
olong  GDelayCol;
/** Column offset for Group delay rate in table record */
olong  GRateOff;
/** Physical column number for Group delay rate in table record */
olong  GRateCol;
/** Column offset for "Clock" epoch error in table record */
olong  clock1Off;
/** Physical column number for "Clock" epoch error in table record */
olong  clock1Col;
/** Column offset for Time derivative of CLOCK in table record */
olong  Dclock1Off;
/** Physical column number for Time derivative of CLOCK in table record */
olong  Dclock1Col;
/** Column offset for LO Offset in table record */
olong  LOOffset1Off;
/** Physical column number for LO Offset in table record */
olong  LOOffset1Col;
/** Column offset for Time derivative of LO offset in table record */
olong  DLOOffset1Off;
/** Physical column number for Time derivative of LO offset in table record */
olong  DLOOffset1Col;
/** Column offset for Dispersive delay (sec at wavelength = 1m)for Poln # 1 in table record */
olong  disp1Off;
/** Physical column number for Dispersive delay (sec at wavelength = 1m)for Poln # 1 in table record */
olong  disp1Col;
/** Column offset for Time derivative of DISPfor Poln # 1 in table record */
olong  Ddisp1Off;
/** Physical column number for Time derivative of DISPfor Poln # 1 in table record */
olong  Ddisp1Col;
/** Column offset for "Clock" epoch error in table record */
olong  clock2Off;
/** Physical column number for "Clock" epoch error in table record */
olong  clock2Col;
/** Column offset for Time derivative of CLOCK in table record */
olong  Dclock2Off;
/** Physical column number for Time derivative of CLOCK in table record */
olong  Dclock2Col;
/** Column offset for LO Offset in table record */
olong  LOOffset2Off;
/** Physical column number for LO Offset in table record */
olong  LOOffset2Col;
/** Column offset for Time derivative of LO offset in table record */
olong  DLOOffset2Off;
/** Physical column number for Time derivative of LO offset in table record */
olong  DLOOffset2Col;
/** Column offset for Dispersive delay (sec at wavelength = 1m)for Poln # 2 in table record */
olong  disp2Off;
/** Physical column number for Dispersive delay (sec at wavelength = 1m)for Poln # 2 in table record */
olong  disp2Col;
/** Column offset for Time derivative of DISPfor Poln # 2 in table record */
olong  Ddisp2Off;
/** Physical column number for Time derivative of DISPfor Poln # 2 in table record */
olong  Ddisp2Col;
