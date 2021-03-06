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
#ifndef OBITTABLEIDI_SYSTEM_TEMPERATURE_H 
#define OBITTABLEIDI_SYSTEM_TEMPERATURE_H 

#include "Obit.h"
#include "ObitErr.h"
#include "ObitTable.h"
#include "ObitData.h"

/*-------- Obit: Merx mollis mortibus nuper ------------------*/
/**
 * \file ObitTableIDI_SYSTEM_TEMPERATURE.h
 * ObitTableIDI_SYSTEM_TEMPERATURE class definition.
 *
 * This class is derived from the #ObitTable class.
 *
 * This class contains tabular data and allows access.
 * This table is part of the IDI uv data format.
 * "IDI\_SYSTEM\_TEMPERATURE" contains measured system and/or antenna temperatures
 * Only FITS cataloged data are supported.
 * This class is derived from the ObitTable class.
 *
 * This class contains tabular data and allows access.
 * "IDI_SYSTEM_TEMPERATURE" table
 * An ObitTableIDI_SYSTEM_TEMPERATURE is the front end to a persistent disk resident structure.
 * Only FITS (as Tables) are supported.
 *
 * \section TableDataStorage Table data storage
 * In memory tables are stored in a fashion similar to how they are 
 * stored on disk - in large blocks in memory rather than structures.
 * Due to the word alignment requirements of some machines, they are 
 * stored by order of the decreasing element size: 
 * double, float long, int, short, char rather than the logical order.
 * The details of the storage in the buffer are kept in the 
 * #ObitTableIDI_SYSTEM_TEMPERATUREDesc.
 *
 * In addition to the normal tabular data, a table will have a "_status"
 * column to indicate the status of each row.
 *
 * \section ObitTableIDI_SYSTEM_TEMPERATURESpecification Specifying desired data transfer parameters
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
 * The convenience Macro #ObitTableIDI_SYSTEM_TEMPERATURESetFITS simplifies specifying the 
 * desired data.
 * Binary tables are used for storing visibility data in FITS.
 * For accessing FITS files the following entries in the ObitInfoList 
 * are used:
 * \li "FileName" OBIT_string (?,1,1) FITS file name.
 * \li "TabName"  OBIT_string (?,1,1) Table name (e.g. "AIPS CC").
 * \li "Ver"      OBIT_int    (1,1,1) Table version number
 *
 *
 * \section ObitTableIDI_SYSTEM_TEMPERATUREaccess Creators and Destructors
 * An ObitTableIDI_SYSTEM_TEMPERATURE can be created using newObitTableIDI_SYSTEM_TEMPERATUREValue which attaches the 
 * table to an ObitData for the object.  
 * If the output ObitTableIDI_SYSTEM_TEMPERATURE has previously been specified, including file information,
 * then ObitTableIDI_SYSTEM_TEMPERATURECopy will copy the disk resident as well as the memory 
 * resident information.
 *
 * A copy of a pointer to an ObitTableIDI_SYSTEM_TEMPERATURE should always be made using the
 * ObitTableIDI_SYSTEM_TEMPERATURERef function which updates the reference count in the object.
 * Then whenever freeing an ObitTableIDI_SYSTEM_TEMPERATURE or changing a pointer, the function
 * ObitTableIDI_SYSTEM_TEMPERATUREUnref will decrement the reference count and destroy the object
 * when the reference count hits 0.
 *
 * \section ObitTableIDI_SYSTEM_TEMPERATUREUsage I/O
 * Visibility data is available after an input object is "Opened"
 * and "Read".
 * I/O optionally uses a buffer attached to the ObitTableIDI_SYSTEM_TEMPERATURE or some external
 * location.
 * To Write an ObitTableIDI_SYSTEM_TEMPERATURE, create it, open it, and write.
 * The object should be closed to ensure all data is flushed to disk.
 * Deletion of an ObitTableIDI_SYSTEM_TEMPERATURE after its final unreferencing will automatically
 * close it.
 */

/*--------------Class definitions-------------------------------------*/

/** Number of characters for Table keyword */
 #define MAXKEYCHARTABLEIDI_SYSTEM_TEMPERATURE 24

/** ObitTableIDI_SYSTEM_TEMPERATURE Class structure. */
typedef struct {
#include "ObitTableIDI_SYSTEM_TEMPERATUREDef.h"   /* this class definition */
} ObitTableIDI_SYSTEM_TEMPERATURE;

/** ObitTableIDI_SYSTEM_TEMPERATURERow Class structure. */
typedef struct {
#include "ObitTableIDI_SYSTEM_TEMPERATURERowDef.h"   /* this class row definition */
} ObitTableIDI_SYSTEM_TEMPERATURERow;

/*----------------- Macroes ---------------------------*/
/** 
 * Macro to unreference (and possibly destroy) an ObitTableIDI_SYSTEM_TEMPERATURE
 * returns an ObitTableIDI_SYSTEM_TEMPERATURE*.
 * in = object to unreference
 */
#define ObitTableIDI_SYSTEM_TEMPERATUREUnref(in) ObitUnref (in)

/** 
 * Macro to reference (update reference count) an ObitTableIDI_SYSTEM_TEMPERATURE.
 * returns an ObitTableIDI_SYSTEM_TEMPERATURE*.
 * in = object to reference
 */
#define ObitTableIDI_SYSTEM_TEMPERATURERef(in) ObitRef (in)

/** 
 * Macro to determine if an object is the member of this or a 
 * derived class.
 * Returns TRUE if a member, else FALSE
 * in = object to reference
 */
#define ObitTableIDI_SYSTEM_TEMPERATUREIsA(in) ObitIsA (in, ObitTableIDI_SYSTEM_TEMPERATUREGetClass())

/** 
 * Macro to unreference (and possibly destroy) an ObitTableIDI_SYSTEM_TEMPERATURERow
 * returns an ObitTableIDI_SYSTEM_TEMPERATURERow*.
 * in = object to unreference
 */
#define ObitTableIDI_SYSTEM_TEMPERATURERowUnref(in) ObitUnref (in)

/** 
 * Macro to reference (update reference count) an ObitTableIDI_SYSTEM_TEMPERATURERow.
 * returns an ObitTableIDI_SYSTEM_TEMPERATURERow*.
 * in = object to reference
 */
#define ObitTableIDI_SYSTEM_TEMPERATURERowRef(in) ObitRef (in)

/** 
 * Macro to determine if an object is the member of this or a 
 * derived class.
 * Returns TRUE if a member, else FALSE
 * in = object to reference
 */
#define ObitTableIDI_SYSTEM_TEMPERATURERowIsA(in) ObitIsA (in, ObitTableIDI_SYSTEM_TEMPERATURERowGetClass())

/*---------------Public functions---------------------------*/
/*----------------Table Row Functions ----------------------*/
/** Public: Row Class initializer. */
void ObitTableIDI_SYSTEM_TEMPERATURERowClassInit (void);

/** Public: Constructor. */
ObitTableIDI_SYSTEM_TEMPERATURERow* newObitTableIDI_SYSTEM_TEMPERATURERow (ObitTableIDI_SYSTEM_TEMPERATURE *table);

/** Public: ClassInfo pointer */
gconstpointer ObitTableIDI_SYSTEM_TEMPERATURERowGetClass (void);

/*------------------Table Functions ------------------------*/
/** Public: Class initializer. */
void ObitTableIDI_SYSTEM_TEMPERATUREClassInit (void);

/** Public: Constructor. */
ObitTableIDI_SYSTEM_TEMPERATURE* newObitTableIDI_SYSTEM_TEMPERATURE (gchar* name);

/** Public: Constructor from values. */
ObitTableIDI_SYSTEM_TEMPERATURE* 
newObitTableIDI_SYSTEM_TEMPERATUREValue (gchar* name, ObitData *file, olong *ver,
  		     ObitIOAccess access,
                     oint numPol, oint no_band,
		     ObitErr *err);

/** Public: Class initializer. */
void ObitTableIDI_SYSTEM_TEMPERATUREClassInit (void);

/** Public: ClassInfo pointer */
gconstpointer ObitTableIDI_SYSTEM_TEMPERATUREGetClass (void);

/** Public: Copy (deep) constructor. */
ObitTableIDI_SYSTEM_TEMPERATURE* ObitTableIDI_SYSTEM_TEMPERATURECopy  (ObitTableIDI_SYSTEM_TEMPERATURE *in, ObitTableIDI_SYSTEM_TEMPERATURE *out, 
			   ObitErr *err);

/** Public: Copy (shallow) constructor. */
ObitTableIDI_SYSTEM_TEMPERATURE* ObitTableIDI_SYSTEM_TEMPERATUREClone (ObitTableIDI_SYSTEM_TEMPERATURE *in, ObitTableIDI_SYSTEM_TEMPERATURE *out);

/** Public: Convert an ObitTable to an ObitTableIDI_SYSTEM_TEMPERATURE */
ObitTableIDI_SYSTEM_TEMPERATURE* ObitTableIDI_SYSTEM_TEMPERATUREConvert  (ObitTable *in);

/** Public: Create ObitIO structures and open file */
ObitIOCode ObitTableIDI_SYSTEM_TEMPERATUREOpen (ObitTableIDI_SYSTEM_TEMPERATURE *in, ObitIOAccess access, 
			  ObitErr *err);

/** Public: Read a table row */
ObitIOCode 
ObitTableIDI_SYSTEM_TEMPERATUREReadRow  (ObitTableIDI_SYSTEM_TEMPERATURE *in, olong iIDI_SYSTEM_TEMPERATURERow, ObitTableIDI_SYSTEM_TEMPERATURERow *row,
		     ObitErr *err);

/** Public: Init a table row for write */
void 
ObitTableIDI_SYSTEM_TEMPERATURESetRow  (ObitTableIDI_SYSTEM_TEMPERATURE *in, ObitTableIDI_SYSTEM_TEMPERATURERow *row,
		     ObitErr *err);

/** Public: Write a table row */
ObitIOCode 
ObitTableIDI_SYSTEM_TEMPERATUREWriteRow  (ObitTableIDI_SYSTEM_TEMPERATURE *in, olong iIDI_SYSTEM_TEMPERATURERow, ObitTableIDI_SYSTEM_TEMPERATURERow *row,
		     ObitErr *err);

/** Public: Close file and become inactive */
ObitIOCode ObitTableIDI_SYSTEM_TEMPERATUREClose (ObitTableIDI_SYSTEM_TEMPERATURE *in, ObitErr *err);

/*----------- ClassInfo Structure -----------------------------------*/
/**
 * ClassInfo Structure.
 * Contains class name, a pointer to any parent class
 * (NULL if none) and function pointers.
 */
typedef struct  {
#include "ObitTableIDI_SYSTEM_TEMPERATUREClassDef.h"
} ObitTableIDI_SYSTEM_TEMPERATUREClassInfo; 

/**
 * ClassInfo Structure For TableIDI_SYSTEM_TEMPERATURERow.
 * Contains class name, a pointer to any parent class
 * (NULL if none) and function pointers.
 */
typedef struct  {
#include "ObitTableIDI_SYSTEM_TEMPERATURERowClassDef.h"
} ObitTableIDI_SYSTEM_TEMPERATURERowClassInfo; 
#endif /* OBITTABLEIDI_SYSTEM_TEMPERATURE_H */ 
