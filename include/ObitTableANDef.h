/* $Id: ObitTableANDef.h 123 2009-09-04 11:26:14Z bill.cotton $   */
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
/*  Define the basic components of the ObitTableAN  structure          */
/*  This is intended to be included in a class structure definition   */
/**
 * \file ObitTableANDef.h
 * ObitTableAN structure members for derived classes.
 */
#include "ObitTableDef.h"  /* Parent class definitions */
/** Array center X coord. (meters, earth center) */
odouble  ArrayX;
/** Array center Y coord. (meters, earth center) */
odouble  ArrayY;
/** Array center Z coord. (meters, earth center) */
odouble  ArrayZ;
/** GST at time=0 (degrees) on the reference date */
odouble  GSTiat0;
/** Earth rotation rate (deg/IAT day) */
odouble  DegDay;
/** Obs. Reference Frequency for subarray(Hz) */
odouble  Freq;
/** Reference date as "YYYYMMDD" */
gchar  RefDate[MAXKEYCHARTABLEAN];
/** Polar position X (meters) on ref. date */
ofloat  PolarX;
/** Polar position Y (meters) on ref. date */
ofloat  PolarY;
/** UT1-UTC  (time sec.) */
ofloat  ut1Utc;
/** data time-UTC  (time sec.) */
ofloat  dataUtc;
/** Time system, 'IAT' or 'UTC' */
gchar  TimeSys[MAXKEYCHARTABLEAN];
/** Array name */
gchar  ArrName[MAXKEYCHARTABLEAN];
/** Number of orbital parameters */
oint  numOrb;
/** Number of polarization calibration constants */
oint  numPCal;
/** Denotes the FQ ID for which the AN poln. parms have been modified. */
oint  FreqID;
/** IAT - UTC (sec). */
ofloat  iatUtc;
/** Polarization parameterazation type, 'APPR', 'RAPPR', 'ORI-ELP' */
gchar  polType[MAXKEYCHARTABLEAN];
/** Polarization reference antenna */
oint  P_Refant;
/** Right-Left Phase difference in radians IF 1 */
ofloat  P_Diff01;
/** Right-Left Phase difference in radians IF 2 */
ofloat  P_Diff02;
/** Right-Left Phase difference in radians IF 3 */
ofloat  P_Diff03;
/** Right-Left Phase difference in radians IF 4 */
ofloat  P_Diff04;
/** Right-Left Phase difference in radians IF 5 */
ofloat  P_Diff05;
/** Right-Left Phase difference in radians IF 6 */
ofloat  P_Diff06;
/** Right-Left Phase difference in radians IF 7 */
ofloat  P_Diff07;
/** Right-Left Phase difference in radians IF 8 */
ofloat  P_Diff08;
/** Column offset for Station number, used as an index in other tables, uv data in table record */
olong  noStaOff;
/** Physical column number for Station number, used as an index in other tables, uv data in table record */
olong  noStaCol;
/** Column offset for Mount type, 0=altaz, 1=equatorial, 2=orbiting in table record */
olong  mntStaOff;
/** Physical column number for Mount type, 0=altaz, 1=equatorial, 2=orbiting in table record */
olong  mntStaCol;
/** Column offset for Axis offset in table record */
olong  staXofOff;
/** Physical column number for Axis offset in table record */
olong  staXofCol;
/** Column offset for Feed A feed position angle in table record */
olong  PolAngAOff;
/** Physical column number for Feed A feed position angle in table record */
olong  PolAngACol;
/** Column offset for Feed B feed position angle in table record */
olong  PolAngBOff;
/** Physical column number for Feed B feed position angle in table record */
olong  PolAngBCol;
/** Column offset for Station name in table record */
olong  AntNameOff;
/** Physical column number for Station name in table record */
olong  AntNameCol;
/** Column offset for X,Y,Z offset from array center in table record */
olong  StaXYZOff;
/** Physical column number for X,Y,Z offset from array center in table record */
olong  StaXYZCol;
/** Column offset for Orbital parameters. in table record */
olong  OrbParmOff;
/** Physical column number for Orbital parameters. in table record */
olong  OrbParmCol;
/** Column offset for Feed A feed poln. type 'R','L','X','Y', actually only one valid character. in table record */
olong  polTypeAOff;
/** Physical column number for Feed A feed poln. type 'R','L','X','Y', actually only one valid character. in table record */
olong  polTypeACol;
/** Column offset for Feed A poln. cal parameter. in table record */
olong  PolCalAOff;
/** Physical column number for Feed A poln. cal parameter. in table record */
olong  PolCalACol;
/** Column offset for Feed B feed poln. type 'R','L','X','Y' in table record */
olong  polTypeBOff;
/** Physical column number for Feed B feed poln. type 'R','L','X','Y' in table record */
olong  polTypeBCol;
/** Column offset for Feed B poln. cal parameter in table record */
olong  PolCalBOff;
/** Physical column number for Feed B poln. cal parameter in table record */
olong  PolCalBCol;
