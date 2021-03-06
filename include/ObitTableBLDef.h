/* $Id: ObitTableBLDef.h 123 2009-09-04 11:26:14Z bill.cotton $   */
/* DO NOT EDIT - file generated by ObitTables.pl                      */
/*--------------------------------------------------------------------*/
/*;  Copyright (C)  2009                                              */
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
/*  Define the basic components of the ObitTableBL  structure          */
/*  This is intended to be included in a class structure definition   */
/**
 * \file ObitTableBLDef.h
 * ObitTableBL structure members for derived classes.
 */
#include "ObitTableDef.h"  /* Parent class definitions */
/** The number of antennas */
oint  numAnt;
/** The number of polarizations */
oint  numPol;
/** The number of IFs */
oint  numIF;
/** Column offset for The center time. in table record */
olong  TimeOff;
/** Physical column number for The center time. in table record */
olong  TimeCol;
/** Column offset for Source ID number in table record */
olong  SourIDOff;
/** Physical column number for Source ID number in table record */
olong  SourIDCol;
/** Column offset for Subarray number in table record */
olong  SubAOff;
/** Physical column number for Subarray number in table record */
olong  SubACol;
/** Column offset for First antenna number of baseline in table record */
olong  ant1Off;
/** Physical column number for First antenna number of baseline in table record */
olong  ant1Col;
/** Column offset for Second antenna number of baseline in table record */
olong  ant2Off;
/** Physical column number for Second antenna number of baseline in table record */
olong  ant2Col;
/** Column offset for Freqid number in table record */
olong  FreqIDOff;
/** Physical column number for Freqid number in table record */
olong  FreqIDCol;
/** Column offset for Real (Multiplicative correction Poln # 1 ) in table record */
olong  RealM1Off;
/** Physical column number for Real (Multiplicative correction Poln # 1 ) in table record */
olong  RealM1Col;
/** Column offset for Imaginary (Multiplicative correction Poln # 1 ) in table record */
olong  ImagM1Off;
/** Physical column number for Imaginary (Multiplicative correction Poln # 1 ) in table record */
olong  ImagM1Col;
/** Column offset for Real (Additive correction Poln # 1 ) in table record */
olong  RealA1Off;
/** Physical column number for Real (Additive correction Poln # 1 ) in table record */
olong  RealA1Col;
/** Column offset for Imaginary (Additive correction Poln # 1 ) in table record */
olong  ImagA1Off;
/** Physical column number for Imaginary (Additive correction Poln # 1 ) in table record */
olong  ImagA1Col;
/** Column offset for Real (Multiplicative correction Poln # 2 ) in table record */
olong  RealM2Off;
/** Physical column number for Real (Multiplicative correction Poln # 2 ) in table record */
olong  RealM2Col;
/** Column offset for Imaginary (Multiplicative correction Poln # 2 ) in table record */
olong  ImagM2Off;
/** Physical column number for Imaginary (Multiplicative correction Poln # 2 ) in table record */
olong  ImagM2Col;
/** Column offset for Real (Additive correction Poln # 2 ) in table record */
olong  RealA2Off;
/** Physical column number for Real (Additive correction Poln # 2 ) in table record */
olong  RealA2Col;
/** Column offset for Imaginary (Additive correction Poln # 2 ) in table record */
olong  ImagA2Off;
/** Physical column number for Imaginary (Additive correction Poln # 2 ) in table record */
olong  ImagA2Col;
