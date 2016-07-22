/* $Id: ObitTableBLRowDef.h 123 2009-09-04 11:26:14Z bill.cotton $   */
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
/*  Define the basic components of the ObitTableRow structure       */
/*  This is intended to be included in a class structure definition   */
/**
 * \file ObitTableBLRowDef.h
 * ObitTableBLRow structure members for derived classes.
 */
#include "ObitTableRowDef.h"  /* Parent class definitions */
/** The center time. */
ofloat  Time;
/** Source ID number */
oint  SourID;
/** Subarray number */
oint  SubA;
/** First antenna number of baseline */
oint  ant1;
/** Second antenna number of baseline */
oint  ant2;
/** Freqid number */
oint  FreqID;
/** Real (Multiplicative correction Poln # 1 ) */
ofloat*  RealM1;
/** Imaginary (Multiplicative correction Poln # 1 ) */
ofloat*  ImagM1;
/** Real (Additive correction Poln # 1 ) */
ofloat*  RealA1;
/** Imaginary (Additive correction Poln # 1 ) */
ofloat*  ImagA1;
/** Real (Multiplicative correction Poln # 2 ) */
ofloat*  RealM2;
/** Imaginary (Multiplicative correction Poln # 2 ) */
ofloat*  ImagM2;
/** Real (Additive correction Poln # 2 ) */
ofloat*  RealA2;
/** Imaginary (Additive correction Poln # 2 ) */
ofloat*  ImagA2;
/** status 0=normal, 1=modified, -1=flagged */
olong  status;
