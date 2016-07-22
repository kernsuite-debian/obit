/* $Id: ObitTableBPDef.h 123 2009-09-04 11:26:14Z bill.cotton $   */
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
/*  Define the basic components of the ObitTableBP  structure          */
/*  This is intended to be included in a class structure definition   */
/**
 * \file ObitTableBPDef.h
 * ObitTableBP structure members for derived classes.
 */
#include "ObitTableDef.h"  /* Parent class definitions */
/** The number of antennas */
oint  numAnt;
/** The number of antennas */
oint  numPol;
/** The number of IFs */
oint  numIF;
/** Number of frequency channels */
oint  numChan;
/** Start channel number */
oint  startChan;
/** If numShifts = 1 BP entries are from cross-power data, if 2 are from total power, if 3 are a mixture, anything else then type is unknown and will assume cross-power */
oint  numShifts;
/** Most negative shift */
oint  lowShift;
/** Shift increment */
oint  shiftInc;
/** BP type: ' ' => standard BP table, CHEBSHEV' => Chebyshev polynomial coeff. */
gchar  BPType[MAXKEYCHARTABLEBP];
/** Column offset for The center time. in table record */
olong  TimeOff;
/** Physical column number for The center time. in table record */
olong  TimeCol;
/** Column offset for Time interval of record in table record */
olong  TimeIOff;
/** Physical column number for Time interval of record in table record */
olong  TimeICol;
/** Column offset for Source ID number in table record */
olong  SourIDOff;
/** Physical column number for Source ID number in table record */
olong  SourIDCol;
/** Column offset for Subarray number in table record */
olong  SubAOff;
/** Physical column number for Subarray number in table record */
olong  SubACol;
/** Column offset for Antenna number in table record */
olong  antNoOff;
/** Physical column number for Antenna number in table record */
olong  antNoCol;
/** Column offset for andwidth of an individual channel in table record */
olong  BWOff;
/** Physical column number for andwidth of an individual channel in table record */
olong  BWCol;
/** Column offset for Freq. id number in table record */
olong  FreqIDOff;
/** Physical column number for Freq. id number in table record */
olong  FreqIDCol;
/** Column offset for Frequency shift for each IF in table record */
olong  ChanShiftOff;
/** Physical column number for Frequency shift for each IF in table record */
olong  ChanShiftCol;
/** Column offset for Reference Antenna in table record */
olong  RefAnt1Off;
/** Physical column number for Reference Antenna in table record */
olong  RefAnt1Col;
/** Column offset for Weights for complex bandpass in table record */
olong  Weight1Off;
/** Physical column number for Weights for complex bandpass in table record */
olong  Weight1Col;
/** Column offset for Real (channel gain Poln # 1 ) in table record */
olong  Real1Off;
/** Physical column number for Real (channel gain Poln # 1 ) in table record */
olong  Real1Col;
/** Column offset for Imaginary (channel gain Poln # 1) in table record */
olong  Imag1Off;
/** Physical column number for Imaginary (channel gain Poln # 1) in table record */
olong  Imag1Col;
/** Column offset for Reference Antenna in table record */
olong  RefAnt2Off;
/** Physical column number for Reference Antenna in table record */
olong  RefAnt2Col;
/** Column offset for Weights for complex bandpass in table record */
olong  Weight2Off;
/** Physical column number for Weights for complex bandpass in table record */
olong  Weight2Col;
/** Column offset for Real (channel gain Poln # 2 ) in table record */
olong  Real2Off;
/** Physical column number for Real (channel gain Poln # 2 ) in table record */
olong  Real2Col;
/** Column offset for Imaginary (channel gain Poln # 2) in table record */
olong  Imag2Off;
/** Physical column number for Imaginary (channel gain Poln # 2) in table record */
olong  Imag2Col;
