/* $Id: ObitTableMFDef.h 123 2009-09-04 11:26:14Z bill.cotton $   */
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
/*  Define the basic components of the ObitTableMF  structure          */
/*  This is intended to be included in a class structure definition   */
/**
 * \file ObitTableMFDef.h
 * ObitTableMF structure members for derived classes.
 */
#include "ObitTableDef.h"  /* Parent class definitions */
/** Dimensions 3 in image */
ofloat  depth1;
/** Dimensions 4 in image */
ofloat  depth2;
/** Dimensions 5 in image */
ofloat  depth3;
/** Dimensions 6 in image */
ofloat  depth4;
/** Dimensions 7 in image */
ofloat  depth5;
/** Column offset for Plane number in table record */
olong  planeOff;
/** Physical column number for Plane number in table record */
olong  planeCol;
/** Column offset for Peak Ipol in table record */
olong  PeakOff;
/** Physical column number for Peak Ipol in table record */
olong  PeakCol;
/** Column offset for Integrated Ipol flux in table record */
olong  IFluxOff;
/** Physical column number for Integrated Ipol flux in table record */
olong  IFluxCol;
/** Column offset for X offset of center in table record */
olong  DeltaXOff;
/** Physical column number for X offset of center in table record */
olong  DeltaXCol;
/** Column offset for Y offset of center in table record */
olong  DeltaYOff;
/** Physical column number for Y offset of center in table record */
olong  DeltaYCol;
/** Column offset for Fitted major axis size in table record */
olong  MajorAxOff;
/** Physical column number for Fitted major axis size in table record */
olong  MajorAxCol;
/** Column offset for Fitted minor axis size in table record */
olong  MinorAxOff;
/** Physical column number for Fitted minor axis size in table record */
olong  MinorAxCol;
/** Column offset for Fitted PA in table record */
olong  PosAngleOff;
/** Physical column number for Fitted PA in table record */
olong  PosAngleCol;
/** Column offset for Integrated Q flux density in table record */
olong  QFluxOff;
/** Physical column number for Integrated Q flux density in table record */
olong  QFluxCol;
/** Column offset for Integrated U flux density in table record */
olong  UFluxOff;
/** Physical column number for Integrated U flux density in table record */
olong  UFluxCol;
/** Column offset for Integrated Y flux density in table record */
olong  VFluxOff;
/** Physical column number for Integrated Y flux density in table record */
olong  VFluxCol;
/** Column offset for rror in Peak Ipol in table record */
olong  errPeakOff;
/** Physical column number for rror in Peak Ipol in table record */
olong  errPeakCol;
/** Column offset for rror in Integrated Ipol flux in table record */
olong  errIFluxOff;
/** Physical column number for rror in Integrated Ipol flux in table record */
olong  errIFluxCol;
/** Column offset for Error in X offset of center in table record */
olong  errDeltaXOff;
/** Physical column number for Error in X offset of center in table record */
olong  errDeltaXCol;
/** Column offset for Error in Y offset of center in table record */
olong  errDeltaYOff;
/** Physical column number for Error in Y offset of center in table record */
olong  errDeltaYCol;
/** Column offset for Error in Fitted major axis size in table record */
olong  errMajorAxOff;
/** Physical column number for Error in Fitted major axis size in table record */
olong  errMajorAxCol;
/** Column offset for Error in Fitted minor axis size in table record */
olong  errMinorAxOff;
/** Physical column number for Error in Fitted minor axis size in table record */
olong  errMinorAxCol;
/** Column offset for Error in Fitted PA in table record */
olong  errPosAngleOff;
/** Physical column number for Error in Fitted PA in table record */
olong  errPosAngleCol;
/** Column offset for Error in Integrated Q flux density in table record */
olong  errQFluxOff;
/** Physical column number for Error in Integrated Q flux density in table record */
olong  errQFluxCol;
/** Column offset for Error in Integrated U flux density in table record */
olong  errUFluxOff;
/** Physical column number for Error in Integrated U flux density in table record */
olong  errUFluxCol;
/** Column offset for Error in Integrated Y flux density in table record */
olong  errVFluxOff;
/** Physical column number for Error in Integrated Y flux density in table record */
olong  errVFluxCol;
/** Column offset for Model type 1 = Gaussian in table record */
olong  TypeModOff;
/** Physical column number for Model type 1 = Gaussian in table record */
olong  TypeModCol;
/** Column offset for Deconvolved best major axis in table record */
olong  D0MajorOff;
/** Physical column number for Deconvolved best major axis in table record */
olong  D0MajorCol;
/** Column offset for Deconvolved best minor axis in table record */
olong  D0MinorOff;
/** Physical column number for Deconvolved best minor axis in table record */
olong  D0MinorCol;
/** Column offset for Deconvolved best PA in table record */
olong  D0PosAngleOff;
/** Physical column number for Deconvolved best PA in table record */
olong  D0PosAngleCol;
/** Column offset for Deconvolved least major axis in table record */
olong  DmMajorOff;
/** Physical column number for Deconvolved least major axis in table record */
olong  DmMajorCol;
/** Column offset for econvolved least minor axis in table record */
olong  DmMinorOff;
/** Physical column number for econvolved least minor axis in table record */
olong  DmMinorCol;
/** Column offset for Deconvolved least PA in table record */
olong  DmPosAngleOff;
/** Physical column number for Deconvolved least PA in table record */
olong  DmPosAngleCol;
/** Column offset for Deconvolved most major axis in table record */
olong  DpMajorOff;
/** Physical column number for Deconvolved most major axis in table record */
olong  DpMajorCol;
/** Column offset for Deconvolved most minor axis in table record */
olong  DpMinorOff;
/** Physical column number for Deconvolved most minor axis in table record */
olong  DpMinorCol;
/** Column offset for Deconvolved most PA in table record */
olong  DpPosAngleOff;
/** Physical column number for Deconvolved most PA in table record */
olong  DpPosAngleCol;
/** Column offset for RMS of Ipol residual in table record */
olong  ResRMSOff;
/** Physical column number for RMS of Ipol residual in table record */
olong  ResRMSCol;
/** Column offset for Peak in Ipol residual in table record */
olong  ResPeakOff;
/** Physical column number for Peak in Ipol residual in table record */
olong  ResPeakCol;
/** Column offset for Integrated Ipol in residual in table record */
olong  ResFluxOff;
/** Physical column number for Integrated Ipol in residual in table record */
olong  ResFluxCol;
/** Column offset for Center x position in pixels in table record */
olong  PixelCenterXOff;
/** Physical column number for Center x position in pixels in table record */
olong  PixelCenterXCol;
/** Column offset for Center y position in pixels in table record */
olong  PixelCenterYOff;
/** Physical column number for Center y position in pixels in table record */
olong  PixelCenterYCol;
/** Column offset for Fitted major axis in pixels in table record */
olong  PixelMajorAxisOff;
/** Physical column number for Fitted major axis in pixels in table record */
olong  PixelMajorAxisCol;
/** Column offset for Fitted minor axis in pixels in table record */
olong  PixelMinorAxisOff;
/** Physical column number for Fitted minor axis in pixels in table record */
olong  PixelMinorAxisCol;
/** Column offset for Fitted PA(?) in table record */
olong  PixelPosAngleOff;
/** Physical column number for Fitted PA(?) in table record */
olong  PixelPosAngleCol;
