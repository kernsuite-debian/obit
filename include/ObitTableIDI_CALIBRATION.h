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
#ifndef OBITTABLEIDI_CALIBRATION_H 
#define OBITTABLEIDI_CALIBRATION_H 

#include "Obit.h"
#include "ObitErr.h"
#include "ObitTable.h"
#include "ObitData.h"

/*-------- Obit: Merx mollis mortibus nuper ------------------*/
/**
 * \file ObitTableIDI_CALIBRATION.h
 * ObitTableIDI_CALIBRATION class definition.
 *
 * This class is derived from the #ObitTable class.
 *
 * This class contains tabular data and allows access.
 * This table is part of the IDI uv data format.
 * "IDI\_CALIBRATION" contains calibration correction for uv data.
 * Only FITS cataloged data are supported.
 * This class is derived from the ObitTable class.
 *
 * This class contains tabular data and allows access.
 * "IDI_CALIBRATION" table
 * An ObitTableIDI_CALIBRATION is the front end to a persistent disk resident structure.
 * Only FITS (as Tables) are supported.
 *
 * \section TableDataStorage Table data storage
 * In memory tables are stored in a fashion similar to how they are 
 * stored on disk - in large blocks in memory rather than structures.
 * Due to the word alignment requirements of some machines, they are 
 * stored by order of the decreasing element size: 
 * double, float long, int, short, char rather than the logical order.
 * The details of the storage in the buffer are kept in the 
 * #ObitTableIDI_CALIBRATIONDesc.
 *
 * In addition to the normal tabular data, a table will have a "_status"
 * column to indicate the status of each row.
 *
 * \section ObitTableIDI_CALIBRATIONSpecification Specifying desired data transfer parameters
 * The desired data transfers are specified in the member ObitInfoList.
 * In the following an ObitInfoList entry is defined by 
 * the name in double quotes, the data type code as an #ObitInfoType enum 
 * and the dimensions of the array (? => depends on application).
 *
 * The following apply to both types of files:
 * \li "nRowPIO", OBIT_int, Max. Number of visibilities per 
 *     "Read" or "Write" operation.  Default = 1.
 *
 * \subsection TableFITS FITS files
 * This implementation uses cfitsio which allows using, in addition to 
 * regular FITS images, gzip compressed files, pipes, shared memory 
 * and a number of other input forms.
 * The convenience Macro #ObitTableIDI_CALIBRATIONSetFITS simplifies specifying the 
 * desired data.
 * Binary tables are used for storing visibility data in FITS.
 * For accessing FITS files the following entries in the ObitInfoList 
 * are used:
 * \li "FileName" OBIT_string (?,1,1) FITS file name.
 * \li "TabName"  OBIT_string (?,1,1) Table name (e.g. "AIPS CC").
 * \li "Ver"      OBIT_int    (1,1,1) Table version number
 *
 *
 * \section ObitTableIDI_CALIBRATIONaccess Creators and Destructors
 * An ObitTableIDI_CALIBRATION can be created using newObitTableIDI_CALIBRATIONValue which attaches the 
 * table to an ObitData for the object.  
 * If the output ObitTableIDI_CALIBRATION has previously been specified, including file information,
 * then ObitTableIDI_CALIBRATIONCopy will copy the disk resident as well as the memory 
 * resident information.
 *
 * A copy of a pointer to an ObitTableIDI_CALIBRATION should always be made using the
 * ObitTableIDI_CALIBRATIONRef function which updates the reference count in the object.
 * Then whenever freeing an ObitTableIDI_CALIBRATION or changing a pointer, the function
 * ObitTableIDI_CALIBRATIONUnref will decrement the reference count and destroy the object
 * when the reference count hits 0.
 *
 * \section ObitTableIDI_CALIBRATIONUsage I/O
 * Visibility data is available after an input object is "Opened"
 * and "Read".
 * I/O optionally uses a buffer attached to the ObitTableIDI_CALIBRATION or some external
 * location.
 * To Write an ObitTableIDI_CALIBRATION, create it, open it, and write.
 * The object should be closed to ensure all data is flushed to disk.
 * Deletion of an ObitTableIDI_CALIBRATION after its final unreferencing will automatically
 * close it.
 */

/*--------------Class definitions-------------------------------------*/

/** Number of characters for Table keyword */
 #define MAXKEYCHARTABLEIDI_CALIBRATION 24

/** ObitTableIDI_CALIBRATION Class structure. */
typedef struct {
#include "ObitTableIDI_CALIBRATIONDef.h"   /* this class definition */
} ObitTableIDI_CALIBRATION;

/** ObitTableIDI_CALIBRATIONRow Class structure. */
typedef struct {
#include "ObitTableIDI_CALIBRATIONRowDef.h"   /* this class row definition */
} ObitTableIDI_CALIBRATIONRow;

/*----------------- Macroes ---------------------------*/
/** 
 * Macro to unreference (and possibly destroy) an ObitTableIDI_CALIBRATION
 * returns an ObitTableIDI_CALIBRATION*.
 * in = object to unreference
 */
#define ObitTableIDI_CALIBRATIONUnref(in) ObitUnref (in)

/** 
 * Macro to reference (update reference count) an ObitTableIDI_CALIBRATION.
 * returns an ObitTableIDI_CALIBRATION*.
 * in = object to reference
 */
#define ObitTableIDI_CALIBRATIONRef(in) ObitRef (in)

/** 
 * Macro to determine if an object is the member of this or a 
 * derived class.
 * Returns TRUE if a member, else FALSE
 * in = object to reference
 */
#define ObitTableIDI_CALIBRATIONIsA(in) ObitIsA (in, ObitTableIDI_CALIBRATIONGetClass())

/** 
 * Macro to unreference (and possibly destroy) an ObitTableIDI_CALIBRATIONRow
 * returns an ObitTableIDI_CALIBRATIONRow*.
 * in = object to unreference
 */
#define ObitTableIDI_CALIBRATIONRowUnref(in) ObitUnref (in)

/** 
 * Macro to reference (update reference count) an ObitTableIDI_CALIBRATIONRow.
 * returns an ObitTableIDI_CALIBRATIONRow*.
 * in = object to reference
 */
#define ObitTableIDI_CALIBRATIONRowRef(in) ObitRef (in)

/** 
 * Macro to determine if an object is the member of this or a 
 * derived class.
 * Returns TRUE if a member, else FALSE
 * in = object to reference
 */
#define ObitTableIDI_CALIBRATIONRowIsA(in) ObitIsA (in, ObitTableIDI_CALIBRATIONRowGetClass())

/*---------------Public functions---------------------------*/
/*----------------Table Row Functions ----------------------*/
/** Public: Row Class initializer. */
void ObitTableIDI_CALIBRATIONRowClassInit (void);

/** Public: Constructor. */
ObitTableIDI_CALIBRATIONRow* newObitTableIDI_CALIBRATIONRow (ObitTableIDI_CALIBRATION *table);

/** Public: ClassInfo pointer */
gconstpointer ObitTableIDI_CALIBRATIONRowGetClass (void);

/*------------------Table Functions ------------------------*/
/** Public: Class initializer. */
void ObitTableIDI_CALIBRATIONClassInit (void);

/** Public: Constructor. */
ObitTableIDI_CALIBRATION* newObitTableIDI_CALIBRATION (gchar* name);

/** Public: Constructor from values. */
ObitTableIDI_CALIBRATION* 
newObitTableIDI_CALIBRATIONValue (gchar* name, ObitData *file, olong *ver,
  		     ObitIOAccess access,
                     oint numPol, oint no_band, oint numAnt,
		     ObitErr *err);

/** Public: Class initializer. */
void ObitTableIDI_CALIBRATIONClassInit (void);

/** Public: ClassInfo pointer */
gconstpointer ObitTableIDI_CALIBRATIONGetClass (void);

/** Public: Copy (deep) constructor. */
ObitTableIDI_CALIBRATION* ObitTableIDI_CALIBRATIONCopy  (ObitTableIDI_CALIBRATION *in, ObitTableIDI_CALIBRATION *out, 
			   ObitErr *err);

/** Public: Copy (shallow) constructor. */
ObitTableIDI_CALIBRATION* ObitTableIDI_CALIBRATIONClone (ObitTableIDI_CALIBRATION *in, ObitTableIDI_CALIBRATION *out);

/** Public: Convert an ObitTable to an ObitTableIDI_CALIBRATION */
ObitTableIDI_CALIBRATION* ObitTableIDI_CALIBRATIONConvert  (ObitTable *in);

/** Public: Create ObitIO structures and open file */
ObitIOCode ObitTableIDI_CALIBRATIONOpen (ObitTableIDI_CALIBRATION *in, ObitIOAccess access, 
			  ObitErr *err);

/** Public: Read a table row */
ObitIOCode 
ObitTableIDI_CALIBRATIONReadRow  (ObitTableIDI_CALIBRATION *in, olong iIDI_CALIBRATIONRow, ObitTableIDI_CALIBRATIONRow *row,
		     ObitErr *err);

/** Public: Init a table row for write */
void 
ObitTableIDI_CALIBRATIONSetRow  (ObitTableIDI_CALIBRATION *in, ObitTableIDI_CALIBRATIONRow *row,
		     ObitErr *err);

/** Public: Write a table row */
ObitIOCode 
ObitTableIDI_CALIBRATIONWriteRow  (ObitTableIDI_CALIBRATION *in, olong iIDI_CALIBRATIONRow, ObitTableIDI_CALIBRATIONRow *row,
		     ObitErr *err);

/** Public: Close file and become inactive */
ObitIOCode ObitTableIDI_CALIBRATIONClose (ObitTableIDI_CALIBRATION *in, ObitErr *err);

/*----------- ClassInfo Structure -----------------------------------*/
/**
 * ClassInfo Structure.
 * Contains class name, a pointer to any parent class
 * (NULL if none) and function pointers.
 */
typedef struct  {
#include "ObitTableIDI_CALIBRATIONClassDef.h"
} ObitTableIDI_CALIBRATIONClassInfo; 

/**
 * ClassInfo Structure For TableIDI_CALIBRATIONRow.
 * Contains class name, a pointer to any parent class
 * (NULL if none) and function pointers.
 */
typedef struct  {
#include "ObitTableIDI_CALIBRATIONRowClassDef.h"
} ObitTableIDI_CALIBRATIONRowClassInfo; 
#endif /* OBITTABLEIDI_CALIBRATION_H */ 
