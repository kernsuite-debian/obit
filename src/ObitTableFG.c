/* $Id: ObitTableFG.c 128 2009-09-23 14:48:29Z bill.cotton $   */
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
#include "ObitTableFG.h"
#include "ObitTableList.h"
#include "ObitData.h"

/*----------------Obit:  Merx mollis mortibus nuper ------------------*/
/**
 * \file ObitTableFG.c
 * ObitTableFG class function definitions.
 *
 * This class is derived from the #ObitTable class.
 */

/** name of the class defined in this file */
static gchar *myClassName = "ObitTableFG";

/**  Function to obtain parent Table ClassInfo - ObitTable */
static ObitGetClassFP ObitParentGetClass = ObitTableGetClass;

/** name of the Row class defined in this file */
static gchar *myRowClassName = "ObitTableFGRow";

/**  Function to obtain parent TableRow ClassInfo */
static ObitGetClassFP ObitParentGetRowClass = ObitTableRowGetClass;

/*--------------- File Global Variables  ----------------*/
/*----------------  Table Row  ----------------------*/
/**
 * ClassInfo structure ObitTableClassInfo.
 * This structure is used by class objects to access class functions.
 */
static ObitTableFGRowClassInfo myRowClassInfo = {FALSE};

/*------------------  Table  ------------------------*/
/**
 * ClassInfo structure ObitTableFGClassInfo.
 * This structure is used by class objects to access class functions.
 */
static ObitTableFGClassInfo myClassInfo = {FALSE};

/*---------------Private function prototypes----------------*/
/** Private: Initialize newly instantiated Row object. */
void  ObitTableFGRowInit  (gpointer in);

/** Private: Deallocate Row members. */
void  ObitTableFGRowClear (gpointer in);

/** Private: Initialize newly instantiated object. */
void  ObitTableFGInit  (gpointer in);

/** Private: Deallocate members. */
void  ObitTableFGClear (gpointer in);

/** Private: update table specific info */
static void ObitTableFGUpdate (ObitTableFG *in, ObitErr *err);

/** Private: copy table keywords to descriptor info list */
static void ObitTableFGDumpKey (ObitTableFG *in, ObitErr *err);

/** Private: Set Class function pointers */
static void ObitTableFGClassInfoDefFn (gpointer inClass);

/** Private: Set Row Class function pointers */
static void ObitTableFGRowClassInfoDefFn (gpointer inClass);
/*----------------------Public functions---------------------------*/

/*------------------  Table Row ------------------------*/
/**
 * Constructor.
 * If table is open and for write, the row is attached to the buffer
 * Initializes Row class if needed on first call.
 * \param name An optional name for the object.
 * \return the new object.
 */
ObitTableFGRow* newObitTableFGRow (ObitTableFG *table)
{
  ObitTableFGRow* out;
  odouble   *dRow;
  oint      *iRow;
  gshort    *siRow;
  ofloat    *fRow;
  gchar     *cRow;
  gboolean  *lRow;
  guint8    *bRow;

  /* Class initialization if needed */
  if (!myRowClassInfo.initialized) ObitTableFGRowClassInit();

  /* allocate/init structure */
  out = g_malloc0(sizeof(ObitTableFGRow));

  /* initialize values */
  out->name = g_strdup("TableFG Row");

  /* set ClassInfo */
  out->ClassInfo = (gpointer)&myRowClassInfo;

  /* initialize other stuff */
  ObitTableFGRowInit((gpointer)out);
  out->myTable   = (ObitTable*)ObitTableRef((ObitTable*)table);

  /* If writing attach to buffer */
  if ((table->buffer) && (table->myDesc->access != OBIT_IO_ReadOnly) &&
      (table->myStatus != OBIT_Inactive)) {
    /* Typed pointers to row of data */  
    dRow  = (odouble*)table->buffer;
    iRow  = (oint*)table->buffer;
    siRow = (gshort*)table->buffer;
    fRow  = (ofloat*)table->buffer;
    cRow  = (gchar*)table->buffer;
    lRow  = (gboolean*)table->buffer;
    bRow  = (guint8*)table->buffer;
  
    /* Set row pointers to buffer */
    out->ants = iRow + table->antsOff;
    out->TimeRange = fRow + table->TimeRangeOff;
    out->ifs = iRow + table->ifsOff;
    out->chans = iRow + table->chansOff;
    out->pFlags = iRow + table->pFlagsOff;
    out->reason = cRow + table->reasonOff;
  } /* end attaching row to table buffer */

 return out;
} /* end newObitTableFGRow */

/**
 * Returns ClassInfo pointer for the Row class.
 * \return pointer to the Row class structure.
 */
gconstpointer ObitTableFGRowGetClass (void)
{
  /* Class initialization if needed */
  if (!myRowClassInfo.initialized) ObitTableFGRowClassInit();
  return (gconstpointer)&myRowClassInfo;
} /* end ObitTableFGRowGetClass */

/*------------------  Table  ------------------------*/
/**
 * Constructor.
 * Initializes class if needed on first call.
 * \param name An optional name for the object.
 * \return the new object.
 */
ObitTableFG* newObitTableFG (gchar* name)
{
  ObitTableFG* out;

  /* Class initialization if needed */
  if (!myClassInfo.initialized) ObitTableFGClassInit();

  /* allocate/init structure */
  out = g_malloc0(sizeof(ObitTableFG));

  /* initialize values */
  if (name!=NULL) out->name = g_strdup(name);
  else out->name = g_strdup("Noname");

  /* set ClassInfo */
  out->ClassInfo = (gpointer)&myClassInfo;

  /* initialize other stuff */
  ObitTableFGInit((gpointer)out);

 return out;
} /* end newObitTableFG */

/**
 * Returns ClassInfo pointer for the class.
 * \return pointer to the class structure.
 */
gconstpointer ObitTableFGGetClass (void)
{
  /* Class initialization if needed */
  if (!myClassInfo.initialized) ObitTableFGClassInit();

  return (gconstpointer)&myClassInfo;
} /* end ObitFGGetClass */

/**
 * Constructor from values.
 * Creates a new table structure and attaches to the TableList of file.
 * If the specified table already exists then it is returned.
 * Initializes class if needed on first call.
 * Forces an update of any disk resident structures (e.g. AIPS header).
 * \param name   An optional name for the object.
 * \param file   ObitData which which the table is to be associated.
 * \param ver    Table version number. 0=> add higher, value used returned
 * \param access access (OBIT_IO_ReadOnly, means do not create if it doesn't exist.
 * \param err Error stack, returns if not empty.
 * \return the new object, NULL on failure.
 */
ObitTableFG* newObitTableFGValue (gchar* name, ObitData *file, olong *ver,
 	                    ObitIOAccess access,
  		    
		     ObitErr *err)
{
  ObitTableFG* out=NULL;
  ObitTable *testTab=NULL;
  ObitTableDesc *desc=NULL;
  ObitTableList *list=NULL;
  ObitInfoList  *info=NULL;
  gboolean exist, optional;
  olong colNo, i, ncol, highVer;
  ObitIOCode retCode;
  gchar *tabType = "AIPS FG";
  gchar *routine = "newObitTableFGValue";

 /* error checks */
  g_assert(ObitErrIsA(err));
  if (err->error) return NULL;
  g_assert (ObitDataIsA(file));

  /* Class initialization if needed */
  if (!myClassInfo.initialized) ObitTableFGClassInit();

  /* Check if the table already exists */
  /* Get TableList */
  list = ((ObitData*)file)->tableList;
  info = ((ObitData*)file)->info;

  /* Get highest version number if not specified */
  if (*ver==0) { 
    highVer = ObitTableListGetHigh (list, "AIPS FG");
    if (access==OBIT_IO_ReadOnly) *ver = highVer;
    else if (access==OBIT_IO_ReadWrite) *ver = highVer;
    else if (access==OBIT_IO_WriteOnly) *ver = highVer+1;
  }
  /* See if it already exists */
  exist = FALSE;
  if (*ver>0) { /* has to be specified */
    exist = ObitTableListGet(list, tabType, ver, &testTab, err);
    if (err->error) /* add traceback,return */
      Obit_traceback_val (err, routine,"", out);
  
    /* if readonly, it must exist to proceed */
    if ((access==OBIT_IO_ReadOnly) && !exist) return out;
    if (testTab!=NULL) { /* it exists, use it if is an ObitTableFG */
      if (ObitTableFGIsA(testTab)) { /* it is an ObitTableFG */
	out = ObitTableRef(testTab);
      } else { /* needs conversion */
 	out = ObitTableFGConvert(testTab);
	/* Update the TableList */
	ObitTableListPut(list, tabType, ver, (ObitTable*)out, err);
	if (err->error) /* add traceback,return */
	  Obit_traceback_val (err, routine,"", out);
      }
      testTab = ObitTableUnref(testTab); /* remove reference */
      return out; /* done */
    }
  } /* end of test for previously existing table */
  
  /* If access is ReadOnly make sure one exists */
  if (access==OBIT_IO_ReadOnly) { 
    highVer = ObitTableListGetHigh (list, "AIPS FG");
    if (highVer<=0) return out;
  }
  
  /* create basal table */
  testTab = newObitDataTable ((ObitData*)file, access, tabType,
			       ver, err);
  if (err->error) Obit_traceback_val (err, routine,"", out);
  
  /* likely need to convert */
  if (ObitTableFGIsA(testTab)) { 
    out = ObitTableRef(testTab);
  } else { /* needs conversion */
    out = ObitTableFGConvert(testTab);
  }
  testTab = ObitTableUnref(testTab); /* remove reference */

  /* Update the TableList */
  ObitTableListPut(list, tabType, ver, (ObitTable*)out, err);
  if (err->error) Obit_traceback_val (err, routine,"", out);

  /* if it previously existed merely return it */
  if (exist) return out; 

  /* set ClassInfo */
  out->ClassInfo = (gpointer)&myClassInfo;

  /* Set values */

  /* initialize descriptor */
  desc = out->myDesc;
  /* How many columns actually in table? */
  ncol = 9 ;
  desc->FieldName = g_malloc0((ncol+1)*sizeof(gchar*));
  desc->FieldUnit = g_malloc0((ncol+1)*sizeof(gchar*));
  desc->type      = g_malloc0((ncol+1)*sizeof(ObitInfoType));
  desc->dim       = g_malloc0((ncol+1)*sizeof(gint32*));
  for (i=0; i<ncol+1; i++) 
    desc->dim[i] = g_malloc0(MAXINFOELEMDIM*sizeof(gint32));

  desc->TableName = g_strdup(tabType);
  desc->sort[0] = 0;
  desc->sort[1] = 0;
  colNo = 0;

  /* Define Columns */
  desc->FieldName[colNo] = g_strdup("SOURCE  ");
  desc->FieldUnit[colNo] = g_strdup("");
  desc->type[colNo] = OBIT_oint;
  for (i=0; i<MAXINFOELEMDIM; i++) desc->dim[colNo][i] = 1;
  colNo++;
  desc->FieldName[colNo] = g_strdup("SUBARRAY ");
  desc->FieldUnit[colNo] = g_strdup("");
  desc->type[colNo] = OBIT_oint;
  for (i=0; i<MAXINFOELEMDIM; i++) desc->dim[colNo][i] = 1;
  colNo++;
  desc->FieldName[colNo] = g_strdup("FREQ ID ");
  desc->FieldUnit[colNo] = g_strdup("");
  desc->type[colNo] = OBIT_oint;
  for (i=0; i<MAXINFOELEMDIM; i++) desc->dim[colNo][i] = 1;
  colNo++;
  optional = FALSE;
  if ((2 > 0) || (!optional)) {
    desc->FieldName[colNo] = g_strdup("ANTS   ");
    desc->FieldUnit[colNo] = g_strdup("");
    desc->type[colNo] = OBIT_oint;
    for (i=0; i<MAXINFOELEMDIM; i++) desc->dim[colNo][i] = 1;
    desc->dim[colNo][0] = 2;
    colNo++;
  }
  optional = FALSE;
  if ((2 > 0) || (!optional)) {
    desc->FieldName[colNo] = g_strdup("TIME RANGE ");
    desc->FieldUnit[colNo] = g_strdup("DAYS");
    desc->type[colNo] = OBIT_float;
    for (i=0; i<MAXINFOELEMDIM; i++) desc->dim[colNo][i] = 1;
    desc->dim[colNo][0] = 2;
    colNo++;
  }
  optional = FALSE;
  if ((2 > 0) || (!optional)) {
    desc->FieldName[colNo] = g_strdup("IFS     ");
    desc->FieldUnit[colNo] = g_strdup("");
    desc->type[colNo] = OBIT_oint;
    for (i=0; i<MAXINFOELEMDIM; i++) desc->dim[colNo][i] = 1;
    desc->dim[colNo][0] = 2;
    colNo++;
  }
  optional = FALSE;
  if ((2 > 0) || (!optional)) {
    desc->FieldName[colNo] = g_strdup("CHANS   ");
    desc->FieldUnit[colNo] = g_strdup("");
    desc->type[colNo] = OBIT_oint;
    for (i=0; i<MAXINFOELEMDIM; i++) desc->dim[colNo][i] = 1;
    desc->dim[colNo][0] = 2;
    colNo++;
  }
  optional = FALSE;
  if ((4 > 0) || (!optional)) {
    desc->FieldName[colNo] = g_strdup("PFLAGS  ");
    desc->FieldUnit[colNo] = g_strdup("");
    desc->type[colNo] = OBIT_bits;
    for (i=0; i<MAXINFOELEMDIM; i++) desc->dim[colNo][i] = 1;
    desc->dim[colNo][0] = 4;
    colNo++;
  }
  optional = FALSE;
  if ((24 > 0) || (!optional)) {
    desc->FieldName[colNo] = g_strdup("REASON  ");
    desc->FieldUnit[colNo] = g_strdup("");
    desc->type[colNo] = OBIT_string;
    for (i=0; i<MAXINFOELEMDIM; i++) desc->dim[colNo][i] = 1;
    desc->dim[colNo][0] = 24;
    colNo++;
  }
  /* Add _status column at end */
  desc->FieldName[colNo] = g_strdup("_status");
  desc->FieldUnit[colNo] = g_strdup("        ");
  desc->type[colNo] = OBIT_long;
  for (i=0; i<MAXINFOELEMDIM; i++) desc->dim[colNo][i] = 1;
  
  /* number of fields */
  desc->nfield = colNo + 1;

  /* initialize descriptor keywords */
  ObitTableFGDumpKey (out, err);
 
  /* index table descriptor */
  ObitTableDescIndex (desc);

  /* Open and Close to fully instantiate */
  retCode = ObitTableFGOpen(out, OBIT_IO_WriteOnly, err);
  if ((retCode!=OBIT_IO_OK) || (err->error)) /* add traceback,return */
    Obit_traceback_val (err, routine, out->name, out);    
  
  retCode = ObitTableFGClose(out, err);
  if ((retCode!=OBIT_IO_OK) || (err->error)) /* add traceback,return */
    Obit_traceback_val (err, routine, out->name, out); 

  /* Force update of disk resident info */
  retCode = ObitIOUpdateTables (((ObitData*)file)->myIO, info, err);
  if ((retCode!=OBIT_IO_OK) || (err->error)) /* add traceback,return */
    Obit_traceback_val (err, routine, out->name, out); 
  
 return out;
} /* end newObitTableFGValue */

/**
 * Convert an ObitTable to an ObitTableFG.
 * New object will have references to members of in.
 * \param in  The object to copy, will still exist afterwards 
 *            and should be Unrefed if not needed.
 * \return pointer to the new object.
 */
ObitTableFG* ObitTableFGConvert (ObitTable* in)
{
  ObitTableFG *out;

  /* error check */
  g_assert(ObitTableIsA(in));

  /* create basic object */
  out = newObitTableFG(in->name);

  /* Delete structures on new */
  out->info   = ObitInfoListUnref(out->info);
  out->thread = ObitThreadUnref(out->thread);
  out->myDesc = ObitTableDescUnref(out->myDesc);
  out->mySel  = ObitTableSelUnref(out->mySel);
  
  /* Reference members of in */
  
  out->info   = ObitInfoListRef(in->info);
  out->thread = ObitThreadRef(in->thread);
  out->myDesc = ObitTableDescRef(in->myDesc);
  out->mySel  = ObitTableSelRef(in->mySel);

  /* Remember who I am */
 out->tabType = g_strdup(in->tabType);
 out->tabVer  = in->tabVer;
  /* Secret reference to host */ 
 out->myHost  = in->myHost;

  return out;
} /* end ObitTableFGConvert */


/**
 * Make a deep copy of input object.
 * Copies are made of complex members including disk files; these 
 * will be copied applying whatever selection is associated with the input.
 * Objects should be closed on input and will be closed on output.
 * In order for the disk file structures to be copied, the output file
 * must be sufficiently defined that it can be written.
 * The copy will be attempted but no errors will be logged until
 * both input and output have been successfully opened.
 * ObitInfoList and ObitThread members are only copied if the output object
 * didn't previously exist.
 * Parent class members are included but any derived class info is ignored.
 * \param in  The object to copy
 * \param out An existing object pointer for output or NULL if none exists.
 * \param err Error stack, returns if not empty.
 * \return pointer to the new object.
 */
ObitTableFG* ObitTableFGCopy (ObitTableFG *in, ObitTableFG *out, ObitErr *err)
{
  gchar *routine = "ObitTableFGCopy";

  /* Class initialization if needed */
  if (!myClassInfo.initialized) ObitTableFGClassInit();

 /* error checks */
  g_assert(ObitErrIsA(err));
  if (err->error) return NULL;
  g_assert (ObitIsA(in, &myClassInfo));
  if (out) g_assert (ObitIsA(out, &myClassInfo));

  /* Use parent class to copy */
  out = (ObitTableFG*)ObitTableCopy ((ObitTable*)in, (ObitTable*)out, err);
  if (err->error) /* add traceback,return */
    Obit_traceback_val (err, routine,in->name, out);

  /* Copy this class  info */
  /* Update class specific info */
  ObitTableFGUpdate (out, err);
    
  return out;
} /* end ObitTableFGCopy */

/**
 * Initialize structures and open file.
 * The image descriptor is read if OBIT_IO_ReadOnly or 
 * OBIT_IO_ReadWrite and written to disk if opened OBIT_IO_WriteOnly.
 * After the file has been opened the member, buffer is initialized
 * for reading/storing the table unless member bufferSize is <0.
 * If the requested version ("Ver" in InfoList) is 0 then the highest
 * numbered table of the same type is opened on Read or Read/Write, 
 * or a new table is created on on Write.
 * The file etc. info should have been stored in the ObitInfoList:
 * \li "FileType" OBIT_long scalar = OBIT_IO_FITS or OBIT_IO_AIPS 
 *               for file type (see class documentation for details).
 * \li "nRowPIO" OBIT_long scalar = Maximum number of table rows
 *               per transfer, this is the target size for Reads (may be 
 *               fewer) and is used to create buffers.
 * \param in Pointer to object to be opened.
 * \param access access (OBIT_IO_ReadOnly,OBIT_IO_ReadWrite,
 *               or OBIT_IO_WriteOnly).
 *               If OBIT_IO_WriteOnly any existing data in the output file
 *               will be lost.
 * \param err ObitErr for reporting errors.
 * \return return code, OBIT_IO_OK=> OK
 */
ObitIOCode ObitTableFGOpen (ObitTableFG *in, ObitIOAccess access, 
			  ObitErr *err)
{
  ObitIOCode retCode = OBIT_IO_SpecErr;
  gint32 dim[MAXINFOELEMDIM] = {1,1,1,1,1};
  olong nRowPIO;
  gchar *routine = "ObitTableFGOpen";

  /* Class initialization if needed */
  if (!myClassInfo.initialized) ObitTableFGClassInit();

  /* error checks */
  g_assert (ObitErrIsA(err));
  if (err->error) return retCode;
  g_assert (ObitIsA(in, &myClassInfo));

   /* Do one row at a time */
   nRowPIO = 1;
   ObitInfoListPut(in->info, "nRowPIO", OBIT_long, dim, (gconstpointer)&nRowPIO, err);
   if (err->error) /* add traceback,return */
     Obit_traceback_val (err, routine, in->name, retCode);
   
   /* use parent class open */
   retCode = ObitTableOpen ((ObitTable*)in, access, err);
   if ((retCode!=OBIT_IO_OK) || (err->error)) /* add traceback,return */
     Obit_traceback_val (err, routine, in->name, retCode);
   
   /* Update class specific info */
   ObitTableFGUpdate (in, err);
   
   return retCode;
} /* end ObitTableFGOpen */

/**
 * Read a table row and return an easily digested version.
 * Scalar values are copied but for array values, pointers 
 * into the data array are returned.
 * \param in       Table to read
 * \param iFGRow   Row number, -1 -> next
 * \param row      Table Row structure to receive data
 * \param err ObitErr for reporting errors.
 * \return return code, OBIT_IO_OK=> OK
 */
ObitIOCode 
ObitTableFGReadRow  (ObitTableFG *in, olong iFGRow, ObitTableFGRow *row,
		     ObitErr *err)
{
  ObitIOCode retCode = OBIT_IO_SpecErr;
  odouble   *dRow;
  oint      *iRow;
  gshort    *siRow;
  ofloat    *fRow;
  gchar     *cRow;
  gboolean  *lRow;
  guint8    *bRow;
  gchar *routine = "ObitTableFGReadRow";
  
  /* error checks */
  g_assert (ObitErrIsA(err));
  if (err->error) return retCode;
  g_assert (ObitIsA(in, &myClassInfo));

  if (in->myStatus == OBIT_Inactive) {
    Obit_log_error(err, OBIT_Error,
		   "AIPS FG Table is inactive for  %s ", in->name);
    return retCode;
 }

  /* read row iFGRow */
  retCode = ObitTableRead ((ObitTable*)in, iFGRow, NULL,  err);
  if (err->error) 
    Obit_traceback_val (err, routine, in->name, retCode);

  /* Typed pointers to row of data */  
  dRow  = (odouble*)in->buffer;
  iRow  = (oint*)in->buffer;
  siRow = (gshort*)in->buffer;
  fRow  = (ofloat*)in->buffer;
  cRow  = (gchar*)in->buffer;
  lRow  = (gboolean*)in->buffer;
  bRow  = (guint8*)in->buffer;
  
  /* Copy scalar fields, for arrays only set pointer*/
  row->SourID = iRow[in->SourIDOff];
  row->SubA = iRow[in->SubAOff];
  row->freqID = iRow[in->freqIDOff];
  row->ants = iRow + in->antsOff;
  row->TimeRange = fRow + in->TimeRangeOff;
  row->ifs = iRow + in->ifsOff;
  row->chans = iRow + in->chansOff;
  row->pFlags = iRow + in->pFlagsOff;
  row->reason = cRow + in->reasonOff;
  row->status = iRow[in->myDesc->statusOff];

  return retCode;
} /*  end ObitTableFGReadRow */

/**
 * Attach an ObitTableRow to the buffer of an ObitTable.
 * This is only useful prior to filling a row structure in preparation .
 * for a WriteRow operation.  Array members of the Row structure are .
 * pointers to independently allocated memory, this routine allows using .
 * the table IO buffer instead of allocating yet more memory..
 * This routine need only be called once to initialize a Row structure for write..
 * \param in  Table with buffer to be written 
 * \param row Table Row structure to attach 
 * \param err ObitErr for reporting errors.
 */
void 
ObitTableFGSetRow  (ObitTableFG *in, ObitTableFGRow *row,
		     ObitErr *err)
{
  odouble   *dRow;
  oint      *iRow;
  gshort    *siRow;
  ofloat    *fRow;
  gchar     *cRow;
  gboolean  *lRow;
  guint8    *bRow;
  
  /* error checks */
  g_assert (ObitErrIsA(err));
  if (err->error) return;
  g_assert (ObitIsA(in, &myClassInfo));
  g_assert (ObitIsA(row, &myRowClassInfo));

  if (in->myStatus == OBIT_Inactive) {
    Obit_log_error(err, OBIT_Error,
		   "FG Table is inactive for  %s ", in->name);
    return;
 }

  /* Typed pointers to row of data */  
  dRow  = (odouble*)in->buffer;
  iRow  = (oint*)in->buffer;
  siRow = (gshort*)in->buffer;
  fRow  = (ofloat*)in->buffer;
  cRow  = (gchar*)in->buffer;
  lRow  = (gboolean*)in->buffer;
  bRow  = (guint8*)in->buffer;
  
  /* Set row pointers to buffer */
  row->ants = iRow + in->antsOff;
  row->TimeRange = fRow + in->TimeRangeOff;
  row->ifs = iRow + in->ifsOff;
  row->chans = iRow + in->chansOff;
  row->pFlags = iRow + in->pFlagsOff;
  row->reason = cRow + in->reasonOff;

} /*  end ObitTableFGSetRow */

/**
 * Write a table row.
 * Before calling this routine, the row structure needs to be initialized
 * and filled with data. The array members of the row structure are  
 * pointers to independently allocated memory.  These pointers can be set to the 
 * correct table buffer locations using ObitTableFGSetRow  
 * \param in       Table to read
 * \param iFGRow   Row number, -1 -> next
 * \param row Table Row structure containing data
 * \param err ObitErr for reporting errors.
 * \return return code, OBIT_IO_OK=> OK
 */
ObitIOCode 
ObitTableFGWriteRow  (ObitTableFG *in, olong iFGRow, ObitTableFGRow *row,
		      ObitErr *err)
{
  ObitIOCode retCode = OBIT_IO_SpecErr;
  gshort    *siRow;
  odouble   *dRow;
  oint      *iRow, i;
  ofloat    *fRow;
  gchar     *cRow;
  gboolean  *lRow;
  guint8    *bRow;
  gchar *routine = "ObitTableFGWriteRow";
  

  /* error checks */
  g_assert (ObitErrIsA(err));
  if (err->error) return retCode;
  g_assert (ObitIsA(in, &myClassInfo));

  if (in->myStatus == OBIT_Inactive) {
    Obit_log_error(err, OBIT_Error,
		   "AIPS FG Table is inactive for %s ", in->name);
    return retCode;
 }

  /* Typed pointers to row of data */  
  dRow  = (odouble*)in->buffer;
  siRow = (gshort*)in->buffer;
  iRow  = (oint*)in->buffer;
  fRow  = (ofloat*)in->buffer;
  cRow  = (gchar*)in->buffer;
  lRow  = (gboolean*)in->buffer;
  bRow  = (guint8*)in->buffer;
  
  /* Make full copy of all data */
  iRow[in->SourIDOff] = row->SourID;
  iRow[in->SubAOff] = row->SubA;
  iRow[in->freqIDOff] = row->freqID;
  if (in->antsCol >= 0) { 
    for (i=0; i<in->myDesc->repeat[in->antsCol]; i++) 
      iRow[in->antsOff+i] = row->ants[i];
  } 
  if (in->TimeRangeCol >= 0) { 
    for (i=0; i<in->myDesc->repeat[in->TimeRangeCol]; i++) 
      fRow[in->TimeRangeOff+i] = row->TimeRange[i];
  } 
  if (in->ifsCol >= 0) { 
    for (i=0; i<in->myDesc->repeat[in->ifsCol]; i++) 
      iRow[in->ifsOff+i] = row->ifs[i];
  } 
  if (in->chansCol >= 0) { 
    for (i=0; i<in->myDesc->repeat[in->chansCol]; i++) 
      iRow[in->chansOff+i] = row->chans[i];
  } 
  if (in->pFlagsCol >= 0) { 
    for (i=0; i<in->myDesc->repeat[in->pFlagsCol]; i++) 
      iRow[in->pFlagsOff+i] = row->pFlags[i];
  } 
  if (in->reasonCol >= 0) { 
    for (i=0; i<in->myDesc->repeat[in->reasonCol]; i++) 
      cRow[in->reasonOff+i] = row->reason[i];
  } 

  /* copy status */
  iRow[in->myDesc->statusOff] = row->status;
   
  /* Write one row */
  in->myDesc->numRowBuff = 1;
 
  /* Write row iFGRow */
  retCode = ObitTableWrite ((ObitTable*)in, iFGRow, NULL,  err);
  if (err->error) 
    Obit_traceback_val (err, routine,in->name, retCode);

  return retCode;
} /*  end ObitTableFGWriteRow */

/**
 * Shutdown I/O.
 * \param in Pointer to object to be closed.
 * \param err ObitErr for reporting errors.
 * \return error code, OBIT_IO_OK=> OK
 */
ObitIOCode ObitTableFGClose (ObitTableFG *in, ObitErr *err)
{
  ObitIOCode retCode = OBIT_IO_SpecErr;
  gchar *routine = "ObitTableFGClose";

  /* error checks */
  g_assert (ObitErrIsA(err));
  if (err->error) return retCode;
  g_assert (ObitIsA((Obit*)in, &myClassInfo));
  /* Something going on? */
  if (in->myStatus == OBIT_Inactive) return OBIT_IO_OK;

  /* Update keywords on descriptor if not ReadOnly*/
  if (in->myDesc->access != OBIT_IO_ReadOnly) 
    ObitTableFGDumpKey (in, err);
  if (err->error) 
    Obit_traceback_val (err, routine, in->name, retCode);

  /* Close */
  retCode = ObitTableClose ((ObitTable*)in, err);
  if (err->error) 
    Obit_traceback_val (err, routine, in->name, retCode);

  return retCode;
} /* end ObitTableFGClose */

/*---------------Private functions--------------------------*/
/*----------------  TableFG Row  ----------------------*/
/**
 * Creates empty member objects, initialize reference count.
 * Parent classes portions are (recursively) initialized first
 * \param inn Pointer to the object to initialize.
 */
void ObitTableFGRowInit  (gpointer inn)
{
  ObitClassInfo *ParentClass;
  ObitTableFGRow *in = inn;

  /* error checks */
  g_assert (in != NULL);

  /* recursively initialize parent class members */
  ParentClass = (ObitClassInfo*)(myRowClassInfo.ParentClass);
  if ((ParentClass!=NULL) && ( ParentClass->ObitInit!=NULL)) 
    ParentClass->ObitInit (inn);

  /* set members in this class */
  /* Set array members to NULL */
  in->ants = NULL;
  in->TimeRange = NULL;
  in->ifs = NULL;
  in->chans = NULL;
  in->pFlags = NULL;
  in->reason = NULL;

} /* end ObitTableFGRowInit */

/**
 * Deallocates member objects.
 * Does (recursive) deallocation of parent class members.
 * For some reason this wasn't build into the GType class.
 * \param  inn Pointer to the object to deallocate.
 *           Actually it should be an ObitTableFGRow* cast to an Obit*.
 */
void ObitTableFGRowClear (gpointer inn)
{
  ObitClassInfo *ParentClass;
  ObitTableFGRow *in = inn;

  /* error checks */
  g_assert (ObitIsA(in, &myRowClassInfo));

  /* delete this class members */
  /* Do not free data array pointers as they were not malloced */
  
  /* unlink parent class members */
  ParentClass = (ObitClassInfo*)(myRowClassInfo.ParentClass);
  /* delete parent class members */
  if ((ParentClass!=NULL) && ( ParentClass->ObitClear!=NULL)) 
    ParentClass->ObitClear (inn);
  
} /* end ObitTableFGRowClear */

/**
 * Initialize global ClassInfo Structure.
 */
void ObitTableFGRowClassInit (void)
{
  if (myRowClassInfo.initialized) return;  /* only once */
  
  /* Set name and parent for this class */
  myRowClassInfo.ClassName   = g_strdup(myRowClassName);
  myRowClassInfo.ParentClass = ObitParentGetRowClass();

  /* Set function pointers */
  ObitTableFGRowClassInfoDefFn ((gpointer)&myRowClassInfo);
 
  myRowClassInfo.initialized = TRUE; /* Now initialized */
 
} /* end ObitTableFGRowClassInit */

/**
 * Initialize global ClassInfo Function pointers.
 */
static void ObitTableFGRowClassInfoDefFn (gpointer inClass)
{
  ObitTableFGRowClassInfo *theClass = (ObitTableFGRowClassInfo*)inClass;
  ObitClassInfo *ParentClass = (ObitClassInfo*)myRowClassInfo.ParentClass;

  if (theClass->initialized) return;  /* only once */

  /* Check type of inClass */
  g_assert (ObitInfoIsA(inClass, (ObitClassInfo*)&myRowClassInfo));

  /* Initialize (recursively) parent class first */
  if ((ParentClass!=NULL) && 
      (ParentClass->ObitClassInfoDefFn!=NULL))
    ParentClass->ObitClassInfoDefFn(theClass);

  /* function pointers defined or overloaded this class */
  theClass->ObitClassInit = (ObitClassInitFP)ObitTableFGRowClassInit;
  theClass->ObitClassInfoDefFn = (ObitClassInfoDefFnFP)ObitTableFGRowClassInfoDefFn;
  theClass->ObitGetClass  = (ObitGetClassFP)ObitTableFGRowGetClass;
  theClass->newObit         = NULL;
  theClass->newObitTableRow = (newObitTableRowFP)newObitTableFGRow;
  theClass->ObitCopy        = NULL;
  theClass->ObitClone       = NULL;
  theClass->ObitClear       = (ObitClearFP)ObitTableFGRowClear;
  theClass->ObitInit        = (ObitInitFP)ObitTableFGRowInit;

} /* end ObitTableFGRowClassDefFn */

/*------------------  TableFG  ------------------------*/

/**
 * Creates empty member objects, initialize reference count.
 * Parent classes portions are (recursively) initialized first
 * \param inn Pointer to the object to initialize.
 */
void ObitTableFGInit  (gpointer inn)
{
  ObitClassInfo *ParentClass;
  ObitTableFG *in = inn;

  /* error checks */
  g_assert (in != NULL);

  /* recursively initialize parent class members */
  ParentClass = (ObitClassInfo*)(myClassInfo.ParentClass);
  if ((ParentClass!=NULL) && ( ParentClass->ObitInit!=NULL)) 
    ParentClass->ObitInit (inn);

  /* set members in this class */

} /* end ObitTableFGInit */

/**
 * Deallocates member objects.
 * Does (recursive) deallocation of parent class members.
 * For some reason this wasn't build into the GType class.
 * \param  inn Pointer to the object to deallocate.
 *           Actually it should be an ObitTableFG* cast to an Obit*.
 */
void ObitTableFGClear (gpointer inn)
{
  ObitClassInfo *ParentClass;
  ObitTableFG *in = inn;

  /* error checks */
  g_assert (ObitIsA(in, &myClassInfo));

  /* delete this class members */
  
  /* unlink parent class members */
  ParentClass = (ObitClassInfo*)(myClassInfo.ParentClass);
  /* delete parent class members */
  if ((ParentClass!=NULL) && ( ParentClass->ObitClear!=NULL)) 
    ParentClass->ObitClear (inn);
  
} /* end ObitTableFGClear */

/**
 * Initialize global ClassInfo Structure.
 */
void ObitTableFGClassInit (void)
{
  if (myClassInfo.initialized) return;  /* only once */
  
  /* Set name and parent for this class */
  myClassInfo.ClassName   = g_strdup(myClassName);
  myClassInfo.ParentClass = ObitParentGetClass();

  /* Set function pointers */
  ObitTableFGClassInfoDefFn ((gpointer)&myClassInfo);
 
  myClassInfo.initialized = TRUE; /* Now initialized */
 
} /* end ObitTableFGClassInit */

/**
 * Initialize global ClassInfo Function pointers.
 */
static void ObitTableFGClassInfoDefFn (gpointer inClass)
{
  ObitTableFGClassInfo *theClass = (ObitTableFGClassInfo*)inClass;
  ObitClassInfo *ParentClass = (ObitClassInfo*)myClassInfo.ParentClass;

  if (theClass->initialized) return;  /* only once */

  /* Check type of inClass */
  g_assert (ObitInfoIsA(inClass, (ObitClassInfo*)&myClassInfo));

  /* Initialize (recursively) parent class first */
  if ((ParentClass!=NULL) && 
      (ParentClass->ObitClassInfoDefFn!=NULL))
    ParentClass->ObitClassInfoDefFn(theClass);

  /* function pointers defined or overloaded this class */
  theClass->ObitClassInit = (ObitClassInitFP)ObitTableFGClassInit;
  theClass->ObitClassInfoDefFn = (ObitClassInfoDefFnFP)ObitTableFGClassInfoDefFn;
  theClass->ObitGetClass  = (ObitGetClassFP)ObitTableFGGetClass;
  theClass->newObit       = (newObitFP)newObitTableFG;
  theClass->ObitCopy      = (ObitCopyFP)ObitTableFGCopy;
  theClass->ObitClone     = (ObitCloneFP)ObitTableClone;
  theClass->ObitClear     = (ObitClearFP)ObitTableFGClear;
  theClass->ObitInit      = (ObitInitFP)ObitTableFGInit;
  theClass->ObitTableConvert = (ObitTableConvertFP)ObitTableFGConvert;
  theClass->ObitTableOpen    = (ObitTableOpenFP)ObitTableFGOpen;
  theClass->ObitTableClose   = (ObitTableCloseFP)ObitTableFGClose;
  theClass->ObitTableRead    = (ObitTableReadFP)ObitTableRead;
  theClass->ObitTableReadSelect = 
    (ObitTableReadSelectFP)ObitTableReadSelect;
  theClass->ObitTableWrite = (ObitTableWriteFP)ObitTableWrite;
  theClass->ObitTableReadRow = 
    (ObitTableReadRowFP)ObitTableFGReadRow;
  theClass->ObitTableWriteRow = 
    (ObitTableWriteRowFP)ObitTableFGWriteRow;

} /* end ObitTableFGClassDefFn */

/**
 * Get table specific information from the infolist or descriptor
 * \param info Table to update
 * \param err  ObitErr for reporting errors.
 */
static void ObitTableFGUpdate (ObitTableFG *in, ObitErr *err)
{
  olong i;
  ObitTableDesc *desc;
   

 /* error checks */
   g_assert(ObitErrIsA(err));
  if (err->error) return;
  g_assert (ObitIsA(in, &myClassInfo));

  /* Get Keywords */

  /* initialize column numbers/offsets */
  in->SourIDOff = -1;
  in->SourIDCol = -1;
  in->SubAOff = -1;
  in->SubACol = -1;
  in->freqIDOff = -1;
  in->freqIDCol = -1;
  in->antsOff = -1;
  in->antsCol = -1;
  in->TimeRangeOff = -1;
  in->TimeRangeCol = -1;
  in->ifsOff = -1;
  in->ifsCol = -1;
  in->chansOff = -1;
  in->chansCol = -1;
  in->pFlagsOff = -1;
  in->pFlagsCol = -1;
  in->reasonOff = -1;
  in->reasonCol = -1;
  /* Find columns and set offsets */
  desc = in->myDesc;
  if (desc->FieldName) {
    for (i=0; i<desc->nfield; i++) {
      if (!strncmp (desc->FieldName[i], "SOURCE  ", 8)) {
	 in->SourIDOff = desc->offset[i];
 	 in->SourIDCol = i;
      }
      if (!strncmp (desc->FieldName[i], "SUBARRAY ", 9)) {
	 in->SubAOff = desc->offset[i];
 	 in->SubACol = i;
      }
      if (!strncmp (desc->FieldName[i], "FREQ ID ", 8)) {
	 in->freqIDOff = desc->offset[i];
 	 in->freqIDCol = i;
      }
      if (!strncmp (desc->FieldName[i], "ANTS   ", 7)) {
	 in->antsOff = desc->offset[i];
 	 in->antsCol = i;
      }
      if (!strncmp (desc->FieldName[i], "TIME RANGE ", 11)) {
	 in->TimeRangeOff = desc->offset[i];
 	 in->TimeRangeCol = i;
      }
      if (!strncmp (desc->FieldName[i], "IFS     ", 8)) {
	 in->ifsOff = desc->offset[i];
 	 in->ifsCol = i;
      }
      if (!strncmp (desc->FieldName[i], "CHANS   ", 8)) {
	 in->chansOff = desc->offset[i];
 	 in->chansCol = i;
      }
      if (!strncmp (desc->FieldName[i], "PFLAGS  ", 8)) {
	 in->pFlagsOff = desc->offset[i];
 	 in->pFlagsCol = i;
      }
      if (!strncmp (desc->FieldName[i], "REASON  ", 8)) {
	 in->reasonOff = desc->offset[i];
 	 in->reasonCol = i;
      }
     }
  }

  /* Check required columns */
  Obit_return_if_fail((in->SourIDOff > -1), err,
       "ObitTableFGUpdate: Could not find column SourID");
  Obit_return_if_fail((in->SubAOff > -1), err,
       "ObitTableFGUpdate: Could not find column SubA");
  Obit_return_if_fail((in->freqIDOff > -1), err,
       "ObitTableFGUpdate: Could not find column freqID");
  Obit_return_if_fail((in->antsOff > -1), err,
       "ObitTableFGUpdate: Could not find column ants");
  Obit_return_if_fail((in->TimeRangeOff > -1), err,
       "ObitTableFGUpdate: Could not find column TimeRange");
  Obit_return_if_fail((in->ifsOff > -1), err,
       "ObitTableFGUpdate: Could not find column ifs");
  Obit_return_if_fail((in->chansOff > -1), err,
       "ObitTableFGUpdate: Could not find column chans");
  Obit_return_if_fail((in->pFlagsOff > -1), err,
       "ObitTableFGUpdate: Could not find column pFlags");
} /* end ObitTableFGUpdate */

/**
 * Copy table specific (keyword) information  to infolist.
 * \param info Table to update
 * \param err  ObitErr for reporting errors.
 */
static void ObitTableFGDumpKey (ObitTableFG *in, ObitErr *err)
{
  ObitInfoList *info=NULL;

 /* error checks */
   g_assert(ObitErrIsA(err));
  if (err->error) return;
  g_assert (ObitIsA(in, &myClassInfo));

  /* Set Keywords */
  if (in->myIO!=NULL) info = ((ObitTableDesc*)(in->myIO->myDesc))->info;
  else info = in->myDesc->info;
   
} /* end ObitTableFGDumpKey */
