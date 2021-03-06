/* $Id: ObitAntenna.h 2 2008-06-10 15:32:27Z bill.cotton $     */
/*--------------------------------------------------------------------*/
/*;  Copyright (C) 2003-2008                                          */
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
/*; Correspondence about this software should be addressed as follows: */
/*;         Internet email: bcotton@nrao.edu.                         */
/*;         Postal address: William Cotton                            */
/*;                         National Radio Astronomy Observatory      */
/*;                         520 Edgemont Road                         */
/*;                         Charlottesville, VA 22903-2475 USA        */
/*--------------------------------------------------------------------*/
#ifndef OBITANTENNA_H 
#define OBITANTENNA_H 
#include "Obit.h"
#include "ObitErr.h"

/*-------- Obit: Merx mollis mortibus nuper ------------------*/
/**
 * \file ObitAntenna.h
 * ObitAntenna class definition.
 *
 * This class is derived from the #Obit class.
 *
 * This class contains information about a given antenna.
 *
 * \section ObitAntennaUsage Usage
 * Instances can be obtained using the #newObitAntenna constructor,
 * the #ObitAntennaCopy constructor or a pointer duplicated using 
 * the #ObitAntennaRef macro.
 * When an instance is no longer needed, use the #ObitAntennaUnref 
 * macro to release it.
 */

/*---------------Class Structure---------------------------*/
/** ObitAntenna Class. */
typedef struct {
#include "ObitAntennaDef.h"   /* actual definition */
} ObitAntenna;

/*----------------- Macroes ---------------------------*/
/** 
 * Macro to unreference (and possibly destroy) an ObitAntenna
 * returns a ObitAntenna*.
 * in = object to unreference
 */
#define ObitAntennaUnref(in) ObitUnref (in)

/** 
 * Macro to reference (update reference count) an ObitAntenna.
 * returns a ObitAntenna*.
 * in = object to reference
 */
#define ObitAntennaRef(in) ObitRef (in)

/** 
 * Macro to determine if an object is the member of this or a 
 * derived class.
 * Returns TRUE if a member, else FALSE
 * in = object to reference
 */
#define ObitAntennaIsA(in) ObitIsA (in, ObitAntennaGetClass())

/*---------------Public functions---------------------------*/
/** Public: Class initializer. */
void ObitAntennaClassInit (void);

/** Public: Constructor. */
ObitAntenna* newObitAntenna (gchar* name);

/** Public: ClassInfo pointer */
gconstpointer ObitAntennaGetClass (void);

/** Public: Copy  constructor. */
ObitAntenna* 
ObitAntennaCopy  (ObitAntenna *in, ObitAntenna *out, ObitErr *err);

/*-------------------Class Info--------------------------*/
/**
 * ClassInfo Structure.
 * Contains class name, a pointer to parent class
 * and function pointers.
 */
typedef struct  {
#include "ObitAntennaClassDef.h" /* Actual definition */
} ObitAntennaClassInfo; 


#endif /* OBITANTENNA_H */ 
