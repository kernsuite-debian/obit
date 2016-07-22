/* $Id:  $   */
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
/*  Define the basic components of the ObitTableIDI_PHASE_CAL  structure          */
/*  This is intended to be included in a class structure definition   */
/**
 * \file ObitTableIDI_PHASE_CALDef.h
 * ObitTableIDI_PHASE_CAL structure members for derived classes.
 */
#include "ObitTableDef.h"  /* Parent class definitions */
/** Table format revision number */
oint  tabrev;
/** The number of polarizations */
oint  numPol;
/** Number of Stokes parameters */
oint  no_stkd;
/** First Stokes parameter */
oint  stk_1;
/** Number of frequency bands */
oint  no_band;
/** Number of frequency channels */
oint  no_chan;
/** Reference frequency (Hz) */
odouble  ref_freq;
/** Channel bandwidth (Hz) */
odouble  chan_bw;
/** Reference frequency bin */
oint  ref_pixl;
/** Observation project code */
gchar  obscode[MAXKEYCHARTABLEIDI_PHASE_CAL];
/** Array name */
gchar  ArrName[MAXKEYCHARTABLEIDI_PHASE_CAL];
/** Reference date as "YYYY-MM-DD" */
gchar  RefDate[MAXKEYCHARTABLEIDI_PHASE_CAL];
/** Number of tones used */
oint  numTones;
/** Column offset for The center time. in table record */
olong  TimeOff;
/** Physical column number for The center time. in table record */
olong  TimeCol;
/** Column offset for The spanned time. in table record */
olong  TimeIOff;
/** Physical column number for The spanned time. in table record */
olong  TimeICol;
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
/** Column offset for CABLE_CAL in table record */
olong  CableCalOff;
/** Physical column number for CABLE_CAL in table record */
olong  CableCalCol;
/** Column offset for State counts(?) in table record */
olong  State1Off;
/** Physical column number for State counts(?) in table record */
olong  State1Col;
/** Column offset for Frequencies of the phase tones in table record */
olong  PCFreq1Off;
/** Physical column number for Frequencies of the phase tones in table record */
olong  PCFreq1Col;
/** Column offset for Real part of tone phase in table record */
olong  PCReal1Off;
/** Physical column number for Real part of tone phase in table record */
olong  PCReal1Col;
/** Column offset for Imaginary part of tone phase in table record */
olong  PCImag1Off;
/** Physical column number for Imaginary part of tone phase in table record */
olong  PCImag1Col;
/** Column offset for Tone rate phase in table record */
olong  PCRate1Off;
/** Physical column number for Tone rate phase in table record */
olong  PCRate1Col;
/** Column offset for State counts(?) in table record */
olong  State2Off;
/** Physical column number for State counts(?) in table record */
olong  State2Col;
/** Column offset for Frequencies of the phase tones in table record */
olong  PCFreq2Off;
/** Physical column number for Frequencies of the phase tones in table record */
olong  PCFreq2Col;
/** Column offset for Real part of tone phase in table record */
olong  PCReal2Off;
/** Physical column number for Real part of tone phase in table record */
olong  PCReal2Col;
/** Column offset for Imaginary part of tone phase in table record */
olong  PCImag2Off;
/** Physical column number for Imaginary part of tone phase in table record */
olong  PCImag2Col;
/** Column offset for Tone rate phase in table record */
olong  PCRate2Off;
/** Physical column number for Tone rate phase in table record */
olong  PCRate2Col;
