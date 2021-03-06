/* $Id: TablePC.inc 123 2009-09-04 11:26:14Z bill.cotton $   */
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
%{
#include "Obit.h"
#include "ObitData.h"
#include "ObitTablePC.h"
%}
 
%inline %{
 
extern ObitTable* TablePC (ObitData *inData, long *tabVer,
 	                   int access,
 	                   char *tabName,
                           int numPol, int numBand, int numTones,
                           ObitErr *err)
 {
   ObitIOAccess laccess;
   /* Cast structural keywords to correct type */
   oint lnumPol = (oint)numPol;
   oint lnumBand = (oint)numBand;
   oint lnumTones = (oint)numTones;
   olong ltabVer = (olong)*tabVer;
   ObitTable *outTable=NULL;
   laccess = OBIT_IO_ReadOnly;
   if (access==2) laccess = OBIT_IO_WriteOnly;
   else if (access==3) laccess = OBIT_IO_ReadWrite;
   outTable = (ObitTable*)newObitTablePCValue ((gchar*)tabName, inData, (olong*)&ltabVer,
   			   laccess, 
                           lnumPol, lnumBand, lnumTones,
                           err);
   *tabVer = (long)ltabVer;
   return outTable;
   }
 
extern PyObject* TablePCGetHeadKeys (ObitTable *inTab) {
  PyObject *outDict=PyDict_New();
  ObitTablePC *lTab = (ObitTablePC*)inTab;
  PyDict_SetItemString(outDict, "numPol",  PyInt_FromLong((long)lTab->numPol));
  PyDict_SetItemString(outDict, "numBand",  PyInt_FromLong((long)lTab->numBand));
  PyDict_SetItemString(outDict, "numTones",  PyInt_FromLong((long)lTab->numTones));
  PyDict_SetItemString(outDict, "revision",  PyInt_FromLong((long)lTab->revision));

  return outDict;
} 

extern void TablePCSetHeadKeys (ObitTable *inTab, PyObject *inDict) {
  ObitTablePC *lTab = (ObitTablePC*)inTab;
  char *tstr;
  int lstr=MAXKEYCHARTABLEPC;

  lTab->revision = (oint)PyInt_AsLong(PyDict_GetItemString(inDict, "revision"));

  if ((lTab->myDesc->access==OBIT_IO_ReadWrite) || (lTab->myDesc->access==OBIT_IO_WriteOnly)) 
    lTab->myStatus = OBIT_Modified;
} 

%}
