/* $Id: ObitTableIDI_FREQUENCYDef.h 190 2010-05-04 15:21:06Z bill.cotton $   */
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
/*  Define the basic components of the ObitTableIDI_FREQUENCY  structure          */
/*  This is intended to be included in a class structure definition   */
/**
 * \file ObitTableIDI_FREQUENCYDef.h
 * ObitTableIDI_FREQUENCY structure members for derived classes.
 */
#include "ObitTableDef.h"  /* Parent class definitions */
/** Table format revision number */
oint  tabrev;
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
gchar  obscode[MAXKEYCHARTABLEIDI_FREQUENCY];
/** Reference date as "YYYY-MM-DD" */
gchar  RefDate[MAXKEYCHARTABLEIDI_FREQUENCY];
/** Column offset for Frequency ID number for row, this is a random parameter in the uv data in table record */
olong  fqidOff;
/** Physical column number for Frequency ID number for row, this is a random parameter in the uv data in table record */
olong  fqidCol;
/** Column offset for Offset from reference frequency for each Band in table record */
olong  bandfreqOff;
/** Physical column number for Offset from reference frequency for each Band in table record */
olong  bandfreqCol;
/** Column offset for Bandwidth of an individual channel, now always written and read as a signed value in table record */
olong  chWidthOff;
/** Physical column number for Bandwidth of an individual channel, now always written and read as a signed value in table record */
olong  chWidthCol;
/** Column offset for Total bandwidth of the IF, now written and read as an unsigned value in table record */
olong  totBWOff;
/** Physical column number for Total bandwidth of the IF, now written and read as an unsigned value in table record */
olong  totBWCol;
/** Column offset for Sideband of the IF (-1 =$>$ lower, +1 =$>$ upper), now always written and read as +1 in table record */
olong  sideBandOff;
/** Physical column number for Sideband of the IF (-1 =$>$ lower, +1 =$>$ upper), now always written and read as +1 in table record */
olong  sideBandCol;
/** Column offset for Look this one up in table record */
olong  bb_chanOff;
/** Physical column number for Look this one up in table record */
olong  bb_chanCol;
