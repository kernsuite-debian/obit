/* $Id: ObitUVGrid.c 159 2010-02-26 17:54:34Z bill.cotton $      */
/*--------------------------------------------------------------------*/
/*;  Copyright (C) 2003-2010                                          */
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

#include <math.h>
#include "ObitUVGrid.h"
#include "ObitFFT.h"
#include "ObitImage.h"

/*----------------Obit: Merx mollis mortibus nuper ------------------*/
/**
 * \file ObitUVGrid.c
 * ObitUVGrid class function definitions.
 * This class is derived from the Obit base class.
 */

/** name of the class defined in this file */
static gchar *myClassName = "ObitUVGrid";

/** Function to obtain parent ClassInfo - Obit */
static ObitGetClassFP ObitParentGetClass = ObitGetClass;

/** Degrees to radians factor */
#ifndef DG2RAD  
#define DG2RAD G_PI / 180.0
#endif

/**  Radians to degrees factor */
#ifndef RAD2DG  
#define RAD2DG 180.0 / G_PI
#endif

/*--------------- File Global Variables  ----------------*/
/**
 * ClassInfo structure ObitUVGridClassInfo.
 * This structure is used by class objects to access class functions.
 */
static ObitUVGridClassInfo myClassInfo = {FALSE};

/*---------------Private structures----------------*/
/* Gridding threaded function argument */
typedef struct {
  /* ObitThread with restart queue */
  ObitThread *thread;
  /* SkyModel with model components loaded (ObitSkyModelLoad) */
  ObitUVGrid *in;
  /* UV data set to model and subtract from current buffer */
  ObitUV       *UVin;
  /* First (1-rel) vis in uvdata buffer to process this thread */
  olong        first;
  /* Highest (1-rel) vis in uvdata buffer to process this thread  */
  olong        last;
  /* thread number, >0 -> no threading   */
  olong        ithread;
  /* Temporary gridding array for thread */
  ObitCArray  *grid;
  /* Number of floats in buffer   */
  olong        buffSize;
  /* I/O buffer to be coppied to UVin buffer */
  ofloat       *buffer;
} UVGridFuncArg;

/** FFT/gridding correction threaded function argument */
typedef struct {
  /* ObitThread with restart queue */
  ObitThread *thread;
  /* SkyModel with model components loaded (ObitSkyModelLoad) */
  ObitUVGrid *in;
  /* UV data set to model and subtract from current buffer */
  ObitFArray *array;
  /* thread number, >0 -> no threading   */
  olong        ithread;
} FFT2ImFuncArg;

/*---------------Private function prototypes----------------*/
/** Private: Initialize newly instantiated object. */
void  ObitUVGridInit  (gpointer in);

/** Private: Deallocate members. */
void  ObitUVGridClear (gpointer in);

/** Private: Grid a single image/Beam possibly with Threads */
static void GridOne (ObitUVGrid* in, ObitUV *UVin, UVGridFuncArg **args, 
		     ObitThread *thread, ObitErr *err);

/** Private: Fill convolving function table */
static void ConvFunc (ObitUVGrid* in, olong fnType);

/** Private: Compute spherical wave functions */
static ofloat sphfn (olong ialf, olong im, olong iflag, ofloat eta);

/** Private: Set Class function pointers. */
static void ObitUVGridClassInfoDefFn (gpointer inClass);

/** Private: Threaded prep/grid buffer */
static gpointer ThreadUVGridBuffer (gpointer arg);

/** Private: Threaded FFT/gridding correct */
static gpointer ThreadFFT2Im (gpointer arg);

/*----------------------Public functions---------------------------*/
/**
 * Constructor.
 * Initializes class if needed on first call.
 * \param name An optional name for the object.
 * \return the new object.
 */
ObitUVGrid* newObitUVGrid (gchar* name)
{
  ObitUVGrid* out;

  /* Class initialization if needed */
  if (!myClassInfo.initialized) ObitUVGridClassInit();

  /* allocate/init structure */
  out = g_malloc0(sizeof(ObitUVGrid));

  /* initialize values */
  if (name!=NULL) out->name = g_strdup(name);
  else out->name = g_strdup("Noname");

  /* set ClassInfo */
  out->ClassInfo = (gpointer)&myClassInfo;

  /* initialize other stuff */
  ObitUVGridInit((gpointer)out);

 return out;
} /* end newObitUVGrid */

/**
 * Returns ClassInfo pointer for the class.
 * \return pointer to the class structure.
 */
gconstpointer ObitUVGridGetClass (void)
{
  /* Class initialization if needed */
  if (!myClassInfo.initialized) ObitUVGridClassInit();

  return (gconstpointer)&myClassInfo;
} /* end ObitUVGridGetClass */

/**
 * Prepares for gridding uv data of the type described by UVin and
 * with derived image as described by imageOut.
 * Input data should be fully edited and calibrated, with any weighting applied 
 * and converted to the appropriate Stokes type.
 * The object UVin will be opened during this call if it is not already open.
 * The output image should describe the center, size and grid spacing of the desired
 * image.
 * The beam corresponding to each image should be made first using the
 * same ObitUVGrid.
 * \param in       Object to initialize
 * \param UVin     Uv data object to be gridded.
 * \param imagee   Image to be gridded (as Obit*)
 * \param doBeam   TRUE is this is a Beam.
 * \param err      ObitErr stack for reporting problems.
 */
void ObitUVGridSetup (ObitUVGrid *in, ObitUV *UVin, Obit *imagee,
		      gboolean doBeam, ObitErr *err)
{
  ObitIOCode retCode;
  ObitUVDesc *uvDesc;
  ObitImageDesc *theDesc=NULL;
  ObitImage *image = (ObitImage*)imagee;
  ObitImage *myBeam;
  olong nx, ny, naxis[2];
  ofloat cellx, celly, dxyzc[3], xt, yt, zt;
  ObitInfoType type;
  gint32 dim[MAXINFOELEMDIM];
  gboolean doCalSelect = FALSE;
  ObitIOAccess access;
  gchar *routine="ObitUVGridSetup";

  /* error checks */
  g_assert (ObitErrIsA(err));
  if (err->error) return;
  g_assert (ObitUVGridIsA(in));
  g_assert (ObitUVIsA(UVin));
  g_assert (ObitImageIsA(image));

  Obit_return_if_fail((image->myDesc->inaxes[0]>0) && 
		      (image->myDesc->inaxes[1]>0), err,
		      "%s: MUST fully define image descriptor %s",
		      routine, image->name);

  /* Need beam */
  myBeam = (ObitImage*)imagee;
  Obit_return_if_fail(ObitImageIsA(myBeam), err,
		      "%s: Beam for %s not defined", 
		      routine, image->name);

  /* Applying calibration or selection? */
  ObitInfoListGetTest(UVin->info, "doCalSelect", &type, dim, &doCalSelect);
  if (doCalSelect) access = OBIT_IO_ReadCal;
  else access = OBIT_IO_ReadOnly;

  /* open uv data to fully instantiate if not already open */
  if (in->myStatus==OBIT_Inactive) {
    retCode = ObitUVOpen (UVin, access, err);
    if (err->error) Obit_traceback_msg (err, routine, in->name);
  }

  uvDesc = UVin->myDesc;

  /* Get source position if it's not already in header */
  if ((uvDesc->crval[uvDesc->jlocr]==0.0) && 
      (uvDesc->crval[uvDesc->jlocd]==0.0)) {
    ObitUVGetRADec (UVin, &uvDesc->crval[uvDesc->jlocr], 
			&uvDesc->crval[uvDesc->jlocd], err);
    if (err->error) Obit_traceback_msg (err, routine, UVin->name);
  }

  /* Beam, image dependent stuff */
  in->nxBeam = myBeam->myDesc->inaxes[0];
  in->nyBeam = myBeam->myDesc->inaxes[1];
  in->icenxBeam = in->nxBeam/2 + 1; 
  in->icenyBeam = in->nyBeam/2 + 1;
  in->nxImage = image->myDesc->inaxes[0];
  in->nyImage = image->myDesc->inaxes[1];
  in->icenxImage = in->nxImage/2 + 1;
  in->icenyImage = in->nyImage/2 + 1;
  
  /* Get values by Beam/Image */
  in->doBeam = doBeam;
  if (doBeam) {
    theDesc = myBeam->myDesc;  /* Which descriptor in use */
    /* shift parameters */
    /* zeros for beam */
    in->dxc = 0.0;
    in->dyc = 0.0;
    in->dzc = 0.0;

  } else {
    /* shift parameters */
    theDesc = image->myDesc;  /* Which descriptor in use */
    ObitUVDescShiftPhase (uvDesc, image->myDesc, dxyzc, err);
    if (err->error) Obit_traceback_msg (err, routine, in->name);
    in->dxc = -dxyzc[0];
    in->dyc = -dxyzc[1];
    in->dzc = -dxyzc[2];
  }

  /* create/resize grid as needed */
  naxis[0] = 1 + theDesc->inaxes[0] / 2;
  naxis[1] = theDesc->inaxes[1];

  if (in->grid==NULL) in->grid = ObitCArrayCreate ("UV Grid", 2, naxis);
  /* reallocate if needbe, zero in any case */
    else in->grid = ObitCArrayRealloc (in->grid, 2, naxis);

   /* Scaling to cells */
  nx = theDesc->inaxes[0];
  ny = theDesc->inaxes[1];
  cellx = (DG2RAD) * theDesc->cdelt[0]; /* x cells spacing in radians */
  celly = (DG2RAD) * theDesc->cdelt[1]; /* y cells spacing in radians */
  in->UScale =  nx * fabs(cellx);
  /* Flip sign on v to make maps come out upside down. */
  in->VScale = -ny * fabs(celly);
  in->WScale = 1.0;

  /* 3D rotation matrix */
  in->rotate = theDesc->crota[1] - uvDesc->crota[1]; /* rotation */
  in->do3Dmul = ObitUVDescShift3DMatrix (uvDesc, theDesc, in->URot3D, in->PRot3D);

  /* Rotate shift parameters if needed. */
  if (in->do3Dmul) {
    xt = (in->dxc)*in->PRot3D[0][0] + (in->dyc)*in->PRot3D[1][0] + (in->dzc)*in->PRot3D[2][0];
    yt = (in->dxc)*in->PRot3D[0][1] + (in->dyc)*in->PRot3D[1][1] + (in->dzc)*in->PRot3D[2][1];
    zt = (in->dxc)*in->PRot3D[0][2] + (in->dyc)*in->PRot3D[1][2] + (in->dzc)*in->PRot3D[2][2];
    in->dxc = xt;
    in->dyc = yt;
    in->dzc = zt;
  }

  /* frequency tables if not defined */
  if ((uvDesc->freqArr==NULL) || (uvDesc->fscale==NULL)) {
    ObitUVGetFreq (UVin, err);
    if (err->error) Obit_traceback_msg (err, routine, in->name);
  } /* end setup frequency table */

}  /* end ObitUVGridSetup */

/**
 * Read a UV data object, applying any shift and accumulating to grid.
 * Buffering of data will use the buffers as defined on UVin 
 * ("nVisPIO" in info member).
 * The UVin object will be closed at the termination of this routine.
 * Requires setup by #ObitUVGridCreate.
 * The gridding information should have been stored in the ObitInfoList on in:
 * \li "Guardband" OBIT_float scalar = maximum fraction of U or v range allowed in grid.
 *             Default = 0.4.
 * \li "MaxBaseline" OBIT_float scalar = maximum baseline length in wavelengths.
 *             Default = 1.0e15.
 * \li "startChann" OBIT_long scalar = first channel (1-rel) in uv data to grid.
 *             Default = 1.
 * \li "numberChann" OBIT_long scalar = number of channels in uv data to grid.
 *             Default = all.
 * \param in      Object to initialize
 * \param UVin    Uv data object to be gridded.
 *                Should be the same as passed to previous call to 
 *                #ObitUVGridSetup for input in.
 * \param err     ObitErr stack for reporting problems.
 */
void ObitUVGridReadUV (ObitUVGrid *in, ObitUV *UVin, ObitErr *err)
{
  ObitIOCode retCode = OBIT_IO_OK;
  ObitInfoType type;
  gint32 dim[MAXINFOELEMDIM];
  ofloat temp, czero[2] = {0.0,0.0};
  olong   itemp;
  olong i, nvis, lovis, hivis, nvisPerThread, nThreads;
  UVGridFuncArg *args=NULL;
  ObitThreadFunc func=(ObitThreadFunc)ThreadUVGridBuffer ;
  gboolean doCalSelect, OK;
  gchar *routine="ObitUVGridReadUV";

  /* error checks */
  g_assert (ObitErrIsA(err));
  if (err->error) return;
  g_assert (ObitUVGridIsA(in));
  g_assert (ObitUVIsA(UVin));
  g_assert (ObitUVDescIsA(UVin->myDesc));
  g_assert (UVin->myDesc->fscale!=NULL); /* frequency scaling table */

   /* If more than one Stokes issue warning */
  if ((UVin->myDesc->jlocs>=0) && 
      (UVin->myDesc->inaxes[UVin->myDesc->jlocs]>1)) {
      Obit_log_error(err, OBIT_InfoWarn, 
		    "%s: More than one Stokes  ( %d) in data, ONLY USING FIRST", 
		     routine, UVin->myDesc->inaxes[UVin->myDesc->jlocs]);
  }

  /* get gridding information */
  /* guardband */
  temp = 0.4;
  /* temp = 0.1; debug */
  ObitInfoListGetTest(in->info, "Guardband", &type, dim, &temp);
  in->guardband = temp;
 
  /* baseline range */
  temp = 1.0e15;
  ObitInfoListGetTest(in->info, "MaxBaseline", &type, dim, &temp);
  in->blmax = temp;
  temp = 0.0;
  ObitInfoListGetTest(in->info, "MinBaseline", &type, dim, &temp);
  in->blmin = temp;

  /* Spectral channels to grid */
  itemp = 1;
  ObitInfoListGetTest(in->info, "startChann", &type, dim, &itemp);
  in->startChann = itemp;
  itemp = 0; /* all */
  ObitInfoListGetTest(in->info, "numberChann", &type, dim, &itemp);
  in->numberChann = itemp;

  /* Calibrating or selecting? */
  doCalSelect = FALSE;
  ObitInfoListGetTest(UVin->info, "doCalSelect", &type, (gint32*)dim, &doCalSelect);

  /* UVin should have been opened in  ObitUVGridSetup */
  
  /* How many threads? */
  in->nThreads = MAX (1, ObitThreadNumProc(in->thread));

  /* Initialize threadArg array  */
  if (in->threadArgs==NULL) {
    in->threadArgs = g_malloc0(in->nThreads*sizeof(UVGridFuncArg*));
    for (i=0; i<in->nThreads; i++) 
      in->threadArgs[i] = g_malloc0(sizeof(UVGridFuncArg)); 
  } 
  
  /* Set up thread arguments */
  for (i=0; i<in->nThreads; i++) {
    args = (UVGridFuncArg*)in->threadArgs[i];
    args->thread = in->thread;
    args->in     = in;
    args->UVin   = UVin;
    args->buffSize = 0;
    args->buffer   = NULL;
    if (i>0) {
      /* Need new zeroed array */
      args->grid = ObitCArrayCreate("Temp grid", in->grid->ndim,  in->grid->naxis);
      ObitCArrayFill (args->grid, czero);
    } else {
      args->grid = ObitCArrayRef(in->grid);
    }
  }
  /* end initialize */

  /* loop gridding data */
  while (retCode == OBIT_IO_OK) {

    /* read buffer */
    if (doCalSelect) retCode = ObitUVReadSelect (UVin, NULL, err);
    else retCode = ObitUVRead (UVin, NULL, err);
    if (err->error) Obit_traceback_msg (err, routine, in->name);
    
    /* Divide up work */
    nvis = UVin->myDesc->numVisBuff;
    if (nvis<1000) nThreads = 1;
    else nThreads = in->nThreads;
    nvisPerThread = nvis/nThreads;
    lovis = 1;
    hivis = nvisPerThread;
    hivis = MIN (hivis, nvis);

    /* Set up thread arguments */
    for (i=0; i<nThreads; i++) {
      if (i==(nThreads-1)) hivis = nvis;  /* Make sure do all */
      args = (UVGridFuncArg*)in->threadArgs[i];
      args->first  = lovis;
      args->last   = hivis;
      if (nThreads>1) args->ithread = i;
      else args->ithread = -1;
     /* Update which vis */
      lovis += nvisPerThread;
      hivis += nvisPerThread;
      hivis = MIN (hivis, nvis);
    }

    /* Do operation on buffer possibly with threads */
    OK = ObitThreadIterator (in->thread, nThreads, func, in->threadArgs);
    
    /* Check for problems */
    if (!OK) {
      Obit_log_error(err, OBIT_Error,"%s: Problem in threading", routine);
      break;
    }
  } /* end loop reading/gridding data */

  /* Accumulate thread grids if more than one */
  if (in->nThreads>1) {
    for (i=1; i<in->nThreads; i++) {
      args = (UVGridFuncArg*)in->threadArgs[i];
      ObitCArrayAdd(in->grid, args->grid, in->grid);
    }
  } /* end accumulating grids */

  /* Shut down any threading */
  ObitThreadPoolFree (in->thread);
  if (in->threadArgs) {
    for (i=0; i<in->nThreads; i++) {
      args = (UVGridFuncArg*)in->threadArgs[i];
      if (args->grid) ObitCArrayUnref(args->grid);
      if (in->threadArgs[i]) g_free(in->threadArgs[i]);
    }
    g_free(in->threadArgs);
  }
  in->threadArgs = NULL;
  in->nThreads   = 0;

  /* Close data */
  retCode = ObitUVClose (UVin, err);
  if (err->error) Obit_traceback_msg (err, routine, in->name);

} /* end ObitUVGridReadUV  */

/**
 * Parallel read a UV data object, applying any shifts and accumulating to grids.
 * Buffering of data will use the buffers as defined on UVin 
 * ("nVisPIO" in info member).
 * The UVin object will be closed at the termination of this routine.
 * Requires setup by #ObitUVGridCreate.
 * The gridding information should have been stored in the ObitInfoList on in[0]:
 * \li "Guardband" OBIT_float scalar = maximum fraction of U or v range allowed in grid.
 *             Default = 0.4.
 * \li "MaxBaseline" OBIT_float scalar = maximum baseline length in wavelengths.
 *             Default = 1.0e15.
 * \li "startChann" OBIT_long scalar = first channel (1-rel) in uv data to grid.
 *             Default = 1.
 * \li "numberChann" OBIT_long scalar = number of channels in uv data to grid.
 *             Default = all.
 * \param nPar    Number of parallel griddings
 * \param in      Array of  objects to grid
 *                Each should be initialized by ObitUVGridSetup
 *                To include beams, double nPar and set doBeam member 
 *                on one of each pair.
 * \param UVin    Array of UV data objects to be gridded.
 *                Should be the same as passed to previous call to 
 *                #ObitUVGridSetup for input in element.
 *                MUST all point to same data set with same selection
 *                but possible different calibration.
 *                All but [0] should be closed.
 * \param err     ObitErr stack for reporting problems.
 */
void ObitUVGridReadUVPar (olong nPar, ObitUVGrid **in, ObitUV **UVin, ObitErr *err)
{
  ObitIOCode retCode = OBIT_IO_OK;
  ObitInfoType type;
  gint32 dim[MAXINFOELEMDIM];
  ofloat temp;
  olong i, j, ip, itemp;
  olong nTh, nnTh, off, nCopy, nLeft, doCalib;
  UVGridFuncArg *args=NULL;
  gboolean doCalSelect;
  ObitUV **UVArr  = NULL;
  ofloat **buffers= NULL;
  gchar *routine="ObitUVGridReadUVPar";

  /* error checks */
  if (err->error) return;
  if (nPar<=0) return;
  for (ip=0; ip<nPar; ip++) {
    g_assert (ObitUVGridIsA(in[ip]));
    g_assert (ObitUVIsA(UVin[ip]));
    g_assert (ObitUVDescIsA(UVin[ip]->myDesc));
    g_assert (UVin[ip]->myDesc->fscale!=NULL); /* frequency scaling table */
  }

  /*  ObitErrTimeLog(err, routine);  Add Timestamp */

   /* If more than one Stokes issue warning */
  if ((UVin[0]->myDesc->jlocs>=0) && 
      (UVin[0]->myDesc->inaxes[UVin[0]->myDesc->jlocs]>1)) {
      Obit_log_error(err, OBIT_InfoWarn, 
		    "%s: More than one Stokes  ( %d) in data, ONLY USING FIRST", 
		     routine, UVin[0]->myDesc->inaxes[UVin[0]->myDesc->jlocs]);
  }

  /* get gridding information */
  /* guardband */
  temp = 0.4;
  /* temp = 0.1; debug */
  ObitInfoListGetTest(in[0]->info, "Guardband", &type, dim, &temp);
  for (ip=0; ip<nPar; ip++) in[ip]->guardband = temp;
 
  /* baseline range */
  temp = 1.0e15;
  ObitInfoListGetTest(in[0]->info, "MaxBaseline", &type, dim, &temp);
  for (ip=0; ip<nPar; ip++) in[ip]->blmax = temp;
  
  temp = 0.0;
  ObitInfoListGetTest(in[0]->info, "MinBaseline", &type, dim, &temp);
  for (ip=0; ip<nPar; ip++) in[ip]->blmin = temp;

  /* Spectral channels to grid */
  itemp = 1;
  ObitInfoListGetTest(in[0]->info, "startChann", &type, dim, &itemp);
  for (ip=0; ip<nPar; ip++) in[ip]->startChann = itemp;
  itemp = 0; /* all */
  ObitInfoListGetTest(in[0]->info, "numberChann", &type, dim, &itemp);
  for (ip=0; ip<nPar; ip++) in[ip]->numberChann = itemp;

  /* Calibrating and/or selecting? */
  doCalib = 0;
  ObitInfoListGetTest(UVin[0]->info, "doCalib", &type, dim, &doCalib);
  doCalSelect = FALSE;
  ObitInfoListGetTest(UVin[0]->info, "doCalSelect", &type, dim, &doCalSelect);

  /* UVin[0] should have been opened in  ObitUVGridSetup */
  
  /* How many threads? */
  in[0]->nThreads = MAX (1, ObitThreadNumProc(in[0]->thread));
  in[0]->nThreads = MIN (nPar, in[0]->nThreads);

  /* Initialize threadArg array put all on in[0] */
  if (in[0]->threadArgs==NULL) {
    in[0]->threadArgs = g_malloc0(in[0]->nThreads*sizeof(UVGridFuncArg*));
    for (i=0; i<in[0]->nThreads; i++) 
      in[0]->threadArgs[i] = g_malloc0(sizeof(UVGridFuncArg)); 
  } 
  
  /* Set up thread arguments */
  for (i=0; i<in[0]->nThreads; i++) {
    args = (UVGridFuncArg*)in[0]->threadArgs[i];
    args->thread   = in[0]->thread;
    args->first    = 1;
    args->buffSize = 0;
    args->buffer   = NULL;
  }

  /* Initialize workGrid arrays */
  if (in[0]->nThreads>1) {
    for (j=0; j<nPar; j++) {
      if (in[j]->workGrids==NULL) {
	in[j]->workGrids = g_malloc0((in[0]->nThreads-1)*sizeof(ObitCArray*));
	for (i=1; i<in[0]->nThreads; i++) 
	  in[j]->workGrids[i-1] = 
	    ObitCArrayCreate("Temp grid", in[j]->grid->ndim,  in[j]->grid->naxis);
      } 
    }
  } /* end create workGrid */
  /* end initialize */

  /* How many threads? */
  nTh = in[0]->nThreads;

  /* Array for UV data */
  UVArr = g_malloc0(2*sizeof(ObitUV*));

  /* Buffer array */
  buffers    = g_malloc0(2*sizeof(ofloat*));
  buffers[0] = g_malloc0(UVin[0]->bufferSize*sizeof(ofloat));
  buffers[1] = g_malloc0(UVin[0]->bufferSize*sizeof(ofloat));

  /* delete buffer on UVin[0] */
  g_free(UVin[0]->buffer); UVin[0]->buffer=NULL;
  UVin[0]->bufferSize = 0;

  /*ObitErrTimeLog(err, "Start Grid Loop");   DEBUG */
  ObitErrLog(err);

  /* loop gridding data */
  while (retCode == OBIT_IO_OK) {

    /* Initial buffer load */
    off = 0;
    UVArr[0] = UVin[0]; /* used for master buffer */
    UVArr[1] = UVin[1]; /* UVArr[1] is the one to actually be used */

    /* read buffer - first used as master - do copy in thread */
    nCopy = 1;
    if (doCalSelect) 
      retCode = ObitUVReadMultiSelect (nCopy, UVArr, buffers, err);
    else 
      retCode = ObitUVReadMulti (nCopy, UVArr, buffers, err);
    if (retCode==OBIT_IO_EOF) break;  /* Finished */
    if (err->error) goto cleanup;
    
    /* Set up thread arguments for first Beam/Image */
    for (i=0; i<nTh; i++) {
      args = (UVGridFuncArg*)in[0]->threadArgs[i];
      if (doCalib<=0) { /* Copy buffer info? */
	UVArr[1]->myDesc->numVisBuff =  UVArr[0]->myDesc->numVisBuff;
	args->buffer = buffers[0];
	args->buffSize = UVArr[1]->myDesc->numVisBuff*UVArr[1]->myDesc->lrec;
      }
    }
    if (err->error) goto cleanup;

    /* Do operation on buffer possibly with threads to grid first image/beam */
    in[off]->nThreads = in[0]->nThreads;
    UVArr[1]->buffer  = buffers[1];
    GridOne(in[off], UVArr[1], (UVGridFuncArg **)in[0]->threadArgs, in[0]->thread, err);
    if (err->error) goto cleanup;

    /* reset buffers */
    UVin[0]->buffer  = buffers[0];     /* master buffer */
    UVArr[1]->buffer = NULL;

    /* Loop over rest of griddings */
    nLeft = nPar - 1;
    off   = 1;
    while (nLeft>0) {
      nnTh = MIN (1, nLeft);  /* How many to do? */
 
      /* Set up thread arguments for next griddings
	 UVArr[0] and buffers[0] not used for gridding */
      UVArr[1]   = UVin[off];

      /* reload buffers - first used as master - copy buffer in thread unless doCalib */
      nCopy = 1;
      if (doCalSelect) 
	retCode = ObitUVReReadMultiSelect (nCopy, UVArr, buffers, err);
      else 
	retCode = ObitUVReReadMulti (nCopy, UVArr, buffers, err);
      if (err->error) goto cleanup;
      
      /* Set up thread arguments for next Beam/Image */
      for (i=0; i<nTh; i++) {
	args = (UVGridFuncArg*)in[0]->threadArgs[i];
	if (doCalib<=0) { /* Copy buffer info? */
	  UVArr[1]->myDesc->numVisBuff =  UVArr[0]->myDesc->numVisBuff;
	  args->buffer = buffers[0];
	  args->buffSize = UVArr[1]->myDesc->numVisBuff*UVArr[1]->myDesc->lrec;
	}
      }
      if (err->error) goto cleanup;
      
      /* Do operation on buffer possibly with threads to grid next image/beam */
      in[off]->nThreads = in[0]->nThreads;
      UVArr[1]->buffer  = buffers[1];
      GridOne(in[off], UVArr[1], (UVGridFuncArg **)in[0]->threadArgs, in[0]->thread, err);
      if (err->error) goto cleanup;
      
      /* reset buffers */
      UVin[0]->buffer  = buffers[0]; /* master buffer */
      UVArr[1]->buffer = NULL;

      off   += 1;  /* update offset */
      nLeft -= 1;  /* update number left */
   } /* end loop over others */
  } /* end loop reading/gridding data */

  /* Accumulate and release workGrid arrays */
  if (in[0]->nThreads>1) {
    for (j=0; j<nPar; j++) {
      for (i=1; i<in[0]->nThreads; i++) {
	ObitCArrayAdd(in[j]->grid, in[j]->workGrids[i-1], in[j]->grid);
	in[j]->workGrids[i-1] = ObitCArrayUnref(in[j]->workGrids[i-1]);
      }
      if (in[j]->workGrids) g_free(in[j]->workGrids);  in[j]->workGrids=NULL; /* release array */
    } /* end accumulating grids */
  }

  /*ObitErrTimeLog(err, "Stop Grid Loop");  DEBUG */
  ObitErrLog(err);
		    
 /* Cleanup */
 cleanup:

  /* Shut down any threading */
  ObitThreadPoolFree (in[0]->thread);
  if (in[0]->threadArgs) {
    for (i=0; i<in[0]->nThreads; i++) {
      args = (UVGridFuncArg*)in[0]->threadArgs[i];
     if (in[0]->threadArgs[i]) g_free(in[0]->threadArgs[i]);
    }
    g_free(in[0]->threadArgs);
  }
  in[0]->threadArgs = NULL;
  in[0]->nThreads   = 0;

  /* Remove buffers from UV objects - they are deleted below */
  for (ip=0; ip<nPar; ip++) {
    UVin[ip]->buffer     = NULL;
    UVin[ip]->bufferSize = 0;
  }

  /* Close data */
  retCode = ObitUVClose (UVin[0], err);

 if (buffers) {
   g_free(buffers[0]);
   g_free(buffers[1]);
   g_free(buffers);
  }
  if (UVArr) g_free(UVArr);
  if (err->error) Obit_traceback_msg (err, routine, in[0]->name);


} /* end ObitUVGridReadUVPar  */

 /**
 * Perform half plane complex to real FFT, convert to center at the center order and
 * apply corrections for the convolution  function used in gridding
 * Requires setup by #ObitUVGridCreate and gridding by #ObitUVGridReadUV.
 * Image written to disk
 * \param in      Object to process
 *                info element "Channel" has plane number, def[1]
 * \param oout    Output image, as Obit* should be open for call
 * \param err     ObitErr stack for reporting problems.
 */
void ObitUVGridFFT2Im (ObitUVGrid *in, Obit *oout, ObitErr *err)
{
  ofloat *ramp=NULL, *data=NULL, *imagep=NULL, *xCorrp=NULL, *yCorrp=NULL, fact;
  ObitImage *out = (ObitImage*)oout;
  olong size, naxis[2], pos[5], pln;
  ObitFArray *xCorrTemp=NULL;
  ObitFArray *array = out->image;
  olong xdim[7], plane[5]={1,1,1,1,1};
  gint32 dim[MAXINFOELEMDIM] = {1,1,1,1,1};
  ObitInfoType type;
  gchar *routine = "ObitUVGridFFT2Im";

  /* error checks */
  g_assert(ObitErrIsA(err));
  if (err->error) return;
  g_assert (ObitUVGridIsA(in));
  g_assert (ObitFArrayIsA(array));
  /* Arrays compatable */
  g_assert (in->grid->ndim == array->ndim);
  g_assert (2*(in->grid->naxis[0]-1) == array->naxis[0]);
  g_assert (in->grid->naxis[1] == array->naxis[1]);

  /*ObitErrTimeLog(err, routine);  Add Timestamp */

  /* Beam or image? */
  if (in->doBeam) { 
    /* Making Beam */ 
    /* Create FFT object if not done before */
    if (in->FFTBeam==NULL) {
      xdim[0] = in->nxBeam; 
      xdim[1] = in->nyBeam; 
      in->FFTBeam = newObitFFT ("Beam FFT", OBIT_FFT_Reverse, OBIT_FFT_HalfComplex,
				2, xdim);
    }

    /* do FFT */
    ObitFFTC2R (in->FFTBeam, in->grid, array);
 
    /* reorder to center at center */
    ObitFArray2DCenter (array);

    /* Do gridding corrections */
    /* Create arrays / initialize if not done */
    if ((in->xCorrBeam==NULL) || (in->yCorrBeam==NULL)) {
      size = in->convWidth * in->convNperCell + 1;
      ramp = g_malloc0(2*size*sizeof(float));
      data = g_malloc0(2*size*sizeof(float));
      naxis[0] = in->nxBeam;
      in->xCorrBeam = ObitFArrayUnref(in->xCorrBeam); /* just in case */
      in->xCorrBeam = ObitFArrayCreate ("X Beam gridding correction", 1, naxis);
      naxis[0] = in->nyBeam;
      in->yCorrBeam = ObitFArrayUnref(in->yCorrBeam); /* just in case */
      in->yCorrBeam = ObitFArrayCreate ("Y Beam gridding correction", 1, naxis);
      
      /* X function */
      GridCorrFn (in, in->nxBeam, in->icenxBeam, data, ramp, in->xCorrBeam);
      
      /* If Y axis */
      GridCorrFn (in, in->nyBeam, in->icenyBeam, data, ramp, in->yCorrBeam);
    } /* end initialize correction functions */
    
    /* Normalization: use center value of beam */
    pos[0] = in->icenxBeam-1; pos[1] = in->icenyBeam-1;
    imagep = ObitFArrayIndex(array, pos);
    xCorrp = ObitFArrayIndex(in->xCorrBeam, pos);
    pos[0] = in->icenyBeam-1;
    yCorrp = ObitFArrayIndex(in->yCorrBeam, pos);
    in->BeamNorm = (*imagep) * (*xCorrp) * (*yCorrp);
    
    if (in->BeamNorm==0.0) {
      Obit_log_error(err, OBIT_Error, "%s ERROR peak in beam is zero for: %s",
		     routine, in->name);
      return;
    }
    /* Correct xCorr by normalization factor */
    fact = 1.0 / MAX (1.0e-20, in->BeamNorm);
    xCorrTemp = ObitFArrayCopy (in->xCorrBeam, NULL, err);
    ObitFArraySMul (xCorrTemp, fact);

    /* Do multiply */
    ObitFArrayMulColRow (array, xCorrTemp, in->yCorrBeam, array);

 } else { 
    /* Making Image */ 
    /* MUST have beam peak for normalization */
    if (in->BeamNorm==0.0) {
      Obit_log_error(err, OBIT_Error, 
		     "ObitUVGridFFT2Im: MUST have made beam first: %s",
		     in->name);
      return;
    }

    /* Create FFT object if not done before */
    if (in->FFTImage==NULL) {
      dim[0] = in->nxImage; 
      dim[1] = in->nyImage; 
      in->FFTImage = newObitFFT ("Image FFT", OBIT_FFT_Reverse, OBIT_FFT_HalfComplex,
				 2, dim);
    }
    
    /* do FFT */
    ObitFFTC2R (in->FFTImage, in->grid, array);

    /* reorder to cernter at center */
    ObitFArray2DCenter (array);
    
    /* Do gridding corrections */
    /* Create arrays / initialize if not done */
    if ((in->xCorrImage==NULL) || (in->yCorrImage==NULL)) {
      size = in->convWidth * in->convNperCell + 1;
      ramp = g_malloc0(2*size*sizeof(float));
      data = g_malloc0(2*size*sizeof(float));
      naxis[0] = in->nxImage;
      in->xCorrImage = ObitFArrayUnref(in->xCorrImage); /* just in case */
      in->xCorrImage = ObitFArrayCreate ("X Image gridding correction", 1, naxis);
      naxis[0] = in->nyImage;
      in->yCorrImage = ObitFArrayUnref(in->yCorrImage); /* just in case */
      in->yCorrImage = ObitFArrayCreate ("Y Image gridding correction", 1, naxis);
      
      /* X function */
      GridCorrFn (in, in->nxImage, in->icenxImage, data, ramp, in->xCorrImage);
      
      /* If Y axis */
      GridCorrFn (in, in->nyImage, in->icenyImage, data, ramp, in->yCorrImage);
    } /* end initialize correction functions */
    
    /* Normalization: use center value of beam */
    /* Correct xCorr by normalization factor */
    fact = 1.0 / MAX (1.0e-20, in->BeamNorm);
    xCorrTemp = ObitFArrayCopy (in->xCorrImage, NULL, err);
    ObitFArraySMul (xCorrTemp, fact);
    
    /* Do multiply   */
       ObitFArrayMulColRow (array, xCorrTemp, in->yCorrImage, array);
  } /* end make image */

  /* cleanup */
  xCorrTemp = ObitFArrayUnref(xCorrTemp);
  if (ramp) g_free (ramp); ramp = NULL;
  if (data) g_free (data); data = NULL;

  /* Write output */
  pln = 1;  /* Get channel/plane number */
  ObitInfoListGetTest(in->info, "Channel", &type, dim, &pln);
  plane[0] = pln;
  ObitImagePutPlane (out, array->array, plane, err);
  if (err->error) Obit_traceback_msg (err, routine, in->name);
	
} /* end ObitUVGridFFT2Im */

/**
 * Parallel perform half plane complex to real FFT, convert to center at the 
 * center order and apply corrections for the convolution  function used in gridding
 * Requires setup by #ObitUVGridCreate and gridding by #ObitUVGridReadUV.
 * If Beams are being made, there should be entries in in and array for both 
 * beam and image with the beam immediately prior to the associated image.
 * Apparently the threading in FFTW clashes with that in Obit so here the
 * FFTs are done sequentially 
 * Images written to disk
 * \param nPar    Number of parallel griddings
 * \param in      Array of  objects to process
 *                info element "Channel" has plane number, def[1]
 * \param oout    Array of output images,  pixel array elements must correspond 
 *                to those in in., as Obit* 
 * \param err     ObitErr stack for reporting problems.
 */
void ObitUVGridFFT2ImPar (olong nPar, ObitUVGrid **in, Obit **oout, ObitErr *err)
{
  ObitImage **out = (ObitImage**)oout;
  olong i, nTh, nnTh, off, channel, nLeft, pos[5], xdim[7], plane[5]={1,1,1,1,1};
  FFT2ImFuncArg *args=NULL;
  ObitFArray *array;
  gint32 dim[MAXINFOELEMDIM] = {1,1,1,1,1};
  ObitInfoType type;
  ObitThreadFunc func=(ObitThreadFunc)ThreadFFT2Im;
  gboolean OK;
  ofloat BeamNorm, fact, *Corrp;
  gchar *routine = "ObitUVGridFFT2ImPar";

  /* error checks */
  if (err->error) return;
  if (nPar<=0)    return;

  for (i=0; i<nPar; i++) {
    array = out[i]->image;  /* image pixel matrix */
    g_assert (ObitUVGridIsA(in[i]));
    g_assert (ObitFArrayIsA(array));
    /* Arrays compatable */
    g_assert (in[i]->grid->ndim == array->ndim);
    g_assert (2*(in[i]->grid->naxis[0]-1) == array->naxis[0]);
    g_assert (in[i]->grid->naxis[1] == array->naxis[1]);
  }

  /* FFTs */
  for (i=0; i<nPar; i++) {
    array = out[i]->image;  /* image pixel matrix */
    /* Create FFT object if not done before */
    if (in[i]->doBeam) { /* Beam? */
      if (in[i]->FFTBeam==NULL) {
	xdim[0] = in[i]->nxBeam; 
	xdim[1] = in[i]->nyBeam; 
	in[i]->FFTBeam = newObitFFT ("Beam FFT", OBIT_FFT_Reverse, OBIT_FFT_HalfComplex,
				     2, xdim);
      }
      
      /* do FFT */
      ObitFFTC2R (in[i]->FFTBeam, in[i]->grid, array);
      
    } else { /* Image */
      /* Create FFT object if not done before */
      if (in[i]->FFTImage==NULL) {
	xdim[0] = in[i]->nxImage; 
	xdim[1] = in[i]->nyImage; 
	in[i]->FFTImage = newObitFFT ("Image FFT", OBIT_FFT_Reverse, OBIT_FFT_HalfComplex,
				      2, xdim);
      }
      
      /* do FFT */
      ObitFFTC2R (in[i]->FFTImage, in[i]->grid, array);
    }
  } /* end loop doing FFTs */

  /* How many threads? */
  in[0]->nThreads = MAX (1, ObitThreadNumProc(in[0]->thread));
  in[0]->nThreads = MIN (nPar, in[0]->nThreads);

  /* Initialize threadArg array put all on in[0] */
  if (in[0]->threadArgs==NULL) {
    in[0]->threadArgs = g_malloc0(in[0]->nThreads*sizeof(FFT2ImFuncArg*));
    for (i=0; i<in[0]->nThreads; i++) 
      in[0]->threadArgs[i] = g_malloc0(sizeof(FFT2ImFuncArg)); 
  } 
  
  /* How many threads? */
  nTh = in[0]->nThreads;

  /* do jobs, doing nTh in parallel */
  off = 0;
  /* Set up thread arguments for first nTh girddings */
  for (i=0; i<nTh; i++) {
    args = (FFT2ImFuncArg*)in[0]->threadArgs[i];
    args->thread = in[0]->thread;
    args->in     = in[i+off];
    args->array  = out[i+off]->image;
    if (nTh>1) args->ithread = i;
    else args->ithread = -1;
  }
  
  /* Do operation on buffer possibly with threads */
  OK = ObitThreadIterator (in[0]->thread, nTh, func, in[0]->threadArgs);
  
  /* Check for problems */
  if (!OK) {
    Obit_log_error(err, OBIT_Error,"%s: Problem in threading", routine);
    return;
  }
  
  /* Loop over rest of images */
  nLeft = nPar - nTh;
  off   = nTh;
  while (nLeft>0) {
    nnTh = MIN (nTh, nLeft);  /* How many to do? */
    
    for (i=0; i<nnTh; i++) {
      args = (FFT2ImFuncArg*)in[0]->threadArgs[i];
      args->thread = in[0]->thread;
      args->in    = in[i+off];
      args->array = out[i+off]->image;
      if (nnTh>1) args->ithread = i;
      else args->ithread = -1;
    }
    
    /* Do operation on buffer possibly with threads */
    OK = ObitThreadIterator (in[0]->thread, nnTh, func, in[0]->threadArgs);
    
    /* Check for problems */
    if (!OK) {
      Obit_log_error(err, OBIT_Error,"%s: Problem in threading", routine);
      return;
    }
    off   += nnTh;  /* update offset */
    nLeft -= nnTh;  /* update number left */
  } /* end loop over rest */
  
  /* Normalize & write - loop looking for an in entry with doBeam member set,
   the center peak is measured and used to normalize,  this peak is assumed
   to be the normalization for the subsequent image.
   if an image without corresponding beam is encountered, the BeamNorm
   member of it in[] is used to normalize */

  for (i=0; i<nPar; i++) {
    array = out[i]->image;  /* image pixel matrix */
    /* is this a beam? */
    if (in[i]->doBeam) {
      pos[0] = in[i]->icenxBeam-1; pos[1] = in[i]->icenyBeam-1; pos[2] = 1;
      Corrp = ObitFArrayIndex(array, pos);
      BeamNorm = *Corrp;
      /* Check */
      if (BeamNorm==0.0) {
	Obit_log_error(err, OBIT_Error, "%s ERROR peak in beam is zero for: %s",
		       routine, in[i]->name);
	return;
      }
      fact = 1.0 / MAX (1.0e-20, BeamNorm);
      ObitFArraySMul (array, fact);  /* Normalize beam */
      /* Save normalization on in[i,i+1] */
      if (!in[i+1]->doBeam) in[i+1]->BeamNorm = BeamNorm;
       in[i]->BeamNorm = BeamNorm;

       /* Write output */
       channel = 1;  /* get channel/plane number */
       ObitInfoListGetTest(in[i]->info, "Channel", &type, dim, &channel);
       plane[0] = channel;
       ObitImagePutPlane (out[i], array->array, plane, err);
       if (err->error) Obit_traceback_msg (err, routine, out[0]->name);
	out[i]->image = ObitFArrayUnref(out[i]->image);  /* Free buffer */

       i++;       /* Advance to image */
    } /* end if beam */

    /*  Now image */
    if (in[i]->BeamNorm==0.0) {
      Obit_log_error(err, OBIT_Error, "%s ERROR image normalization is zero for: %s",
		     routine, in[i]->name);
      return;
    }
    array = out[i]->image;  /* image pixel matrix */
    fact = 1.0 / MAX (1.0e-20, in[i]->BeamNorm);
    ObitFArraySMul (array, fact);  /* Normalize image */

    /* Write output */
    channel = 1;  /* get channel/plane number */
    ObitInfoListGetTest(in[i]->info, "Channel", &type, dim, &channel);
    plane[0] = channel;
    ObitImagePutPlane (out[i], array->array, plane, err);
    if (err->error) Obit_traceback_msg (err, routine, out[0]->name);
	out[i]->image = ObitFArrayUnref(out[i]->image);  /* Free buffer */
  } /* end normalization loop */

} /* end ObitUVGridFFT2ImPar */

/**
 * Initialize global ClassInfo Structure.
 */
void ObitUVGridClassInit (void)
{
  if (myClassInfo.initialized) return;  /* only once */
  
  /* Set name and parent for this class */
  myClassInfo.ClassName   = g_strdup(myClassName);
  myClassInfo.ParentClass = ObitParentGetClass();

  /* Set function pointers */
  ObitUVGridClassInfoDefFn ((gpointer)&myClassInfo);
 
  myClassInfo.initialized = TRUE; /* Now initialized */
 
} /* end ObitUVGridClassInit */

/**
 * Initialize global ClassInfo Function pointers.
 */
static void ObitUVGridClassInfoDefFn (gpointer inClass)
{
  ObitUVGridClassInfo *theClass = (ObitUVGridClassInfo*)inClass;
  ObitClassInfo *ParentClass = (ObitClassInfo*)myClassInfo.ParentClass;

  if (theClass->initialized) return;  /* only once */

  /* Check type of inClass */
  g_assert (ObitInfoIsA(inClass, (ObitClassInfo*)&myClassInfo));

  /* Initialize (recursively) parent class first */
  if ((ParentClass!=NULL) && 
      (ParentClass->ObitClassInfoDefFn!=NULL))
    ParentClass->ObitClassInfoDefFn(theClass);

  /* function pointers defined or overloaded this class */
  theClass->ObitClassInit = (ObitClassInitFP)ObitUVGridClassInit;
  theClass->ObitClassInfoDefFn = (ObitClassInfoDefFnFP)ObitUVGridClassInfoDefFn;
  theClass->ObitGetClass  = (ObitGetClassFP)ObitUVGridGetClass;
  theClass->newObit       = (newObitFP)newObitUVGrid;
  theClass->ObitCopy      = NULL;
  theClass->ObitClone     = NULL;
  theClass->ObitClear     = (ObitClearFP)ObitUVGridClear;
  theClass->ObitInit      = (ObitInitFP)ObitUVGridInit;
  theClass->ObitUVGridSetup      = (ObitUVGridSetupFP)ObitUVGridSetup;
  theClass->ObitUVGridReadUV     = (ObitUVGridReadUVFP)ObitUVGridReadUV;
  theClass->ObitUVGridReadUVPar  = (ObitUVGridReadUVParFP)ObitUVGridReadUVPar;
  theClass->ObitUVGridFFT2Im     = (ObitUVGridFFT2ImFP)ObitUVGridFFT2Im;
  theClass->ObitUVGridFFT2ImPar  = (ObitUVGridFFT2ImParFP)ObitUVGridFFT2ImPar;
  theClass->PrepBuffer           = (PrepBufferFP)PrepBuffer;
  theClass->GridBuffer           = (GridBufferFP)GridBuffer;
  theClass->GridCorrFn           = (GridCorrFnFP)GridCorrFn;

} /* end ObitUVGridClassDefFn */

/*---------------Private functions--------------------------*/

/**
 * Creates empty member objects, initialize reference count.
 * Parent classes portions are (recursively) initialized first
 * \param inn Pointer to the object to initialize.
 */
void ObitUVGridInit  (gpointer inn)
{
  ObitClassInfo *ParentClass;
  ObitUVGrid *in = inn;

  /* error checks */
  g_assert (in != NULL);

  /* recursively initialize parent class members */
  ParentClass = (ObitClassInfo*)(myClassInfo.ParentClass);
  if ((ParentClass!=NULL) && ( ParentClass->ObitInit!=NULL)) 
    ParentClass->ObitInit (inn);

  /* set members in this class */
  in->thread       = newObitThread();
  in->info         = newObitInfoList(); 
  in->myStatus     = OBIT_Inactive;
  in->grid         = NULL;
  in->convfn       = NULL;
  in->FFTBeam      = NULL;
  in->FFTImage     = NULL;
  in->xCorrBeam    = NULL;
  in->yCorrBeam    = NULL;
  in->xCorrImage   = NULL;
  in->yCorrImage   = NULL;
  in->nThreads     = 0;
  in->threadArgs   = NULL;
  in->workGrids    = NULL;

  /* initialize convolving function table */
  /* pillbox (0) for testing (4=exp*sinc, 5=Spherodial wave) */
  ConvFunc(in, 5);
  /* ConvFunc(in, 0);   DEBUG - use pillbox */
} /* end ObitUVGridInit */

/**
 * Deallocates member objects.
 * Does (recursive) deallocation of parent class members.
 * For some reason this wasn't build into the GType class.
 * \param  inn Pointer to the object to deallocate.
 *           Actually it should be an ObitUVGrid* cast to an Obit*.
 */
void ObitUVGridClear (gpointer inn)
{
  olong i;
  UVGridFuncArg *args;
  ObitClassInfo *ParentClass;
  ObitUVGrid *in = inn;

  /* error checks */
  g_assert (ObitIsA(in, &myClassInfo));

  /* delete this class members */
  in->thread    = ObitThreadUnref(in->thread);
  in->info      = ObitInfoListUnref(in->info);
  in->grid      = ObitCArrayUnref(in->grid);  
  in->convfn    = ObitFArrayUnref(in->convfn);
  in->FFTBeam   = ObitFFTUnref(in->FFTBeam);
  in->FFTImage  = ObitFFTUnref(in->FFTImage);
  in->xCorrBeam = ObitFArrayUnref(in->xCorrBeam);
  in->yCorrBeam = ObitFArrayUnref(in->yCorrBeam);
  in->xCorrImage= ObitFArrayUnref(in->xCorrImage);
  in->yCorrImage= ObitFArrayUnref(in->yCorrImage);
   if (in->threadArgs) {
    for (i=0; i<in->nThreads; i++) {
      args = (UVGridFuncArg*)in->threadArgs[i];
      if (in->threadArgs[i]) g_free(in->threadArgs[i]);
    }
    g_free(in->threadArgs);
  }
   if (in->workGrids) {
    for (i=1; i<in->nThreads; i++) {
      if (in->workGrids[i-1]) g_free(in->workGrids[i-1]);
    }
    g_free(in->workGrids);
  }
 
  /* unlink parent class members */
  ParentClass = (ObitClassInfo*)(myClassInfo.ParentClass);
  /* delete parent class members */
  if ((ParentClass!=NULL) && ( ParentClass->ObitClear!=NULL)) 
    ParentClass->ObitClear (inn);
  
} /* end ObitUVGridClear */


/**
 * Prepares a buffer load of visibility data for gridding:
 * \li rotate (u,v,w) if doing 3D imaging and a shift.
 * \li shift position if needed.
 * \li if doBeam then replace data with (1,0).
 * \li enforce guardband - no data near outer edges of grid 
 * \li Convert to cells at the reference frequency.
 * \li All data  converted to the positive V half plane.
 * \param in      Object with grid to accumulate.
 * \param uvdata  Object with uvdata in buffer.
 * \param loVis   (0-rel) first vis in buffer in uv data
 * \param hiVis   (1-rel) highest vis in buffer in uv data
 */
void PrepBuffer (ObitUVGrid* in, ObitUV *uvdata, olong loVis, olong hiVis)
{
  olong ivis, nvis, ifreq, nif, iif, nfreq, ifq, loFreq, hiFreq;
  ofloat *u, *v, *w, *vis, *ifvis, *vvis;
  ofloat phase, cp, sp, vr, vi, uu, vv, ww, uf, vf, wf;
  ofloat bl2, blmax2, blmin2, wt, guardu, guardv;
  ObitUVDesc *desc;
  olong fincf, fincif;
  gboolean doShift, doFlag, flip;

  /* error checks */
  g_assert (ObitUVGridIsA(in));
  g_assert (ObitUVIsA(uvdata));
  g_assert (uvdata->myDesc != NULL);
  g_assert (uvdata->buffer != NULL);

  /* how much data? */
  desc  = uvdata->myDesc;
  nvis  = desc->numVisBuff;
  if (nvis<=0) return; /* need something */
  nfreq = desc->inaxes[desc->jlocf];
  nif = 1;
  if (desc->jlocif>=0) nif = desc->inaxes[desc->jlocif];
  
  /* range of channels (0-rel) */
  loFreq = in->startChann-1;
  hiFreq = loFreq + in->numberChann;
  if (in->numberChann<=0) hiFreq = (nfreq - in->startChann);

  /* Channel and IF increments in frequency scaling array */
  fincf  = MAX (1, (desc->incf  / 3) / desc->inaxes[desc->jlocs]);
  fincif = MAX (1, (desc->incif / 3) / desc->inaxes[desc->jlocs]);

 /* initialize data pointers into buffer */
  u   = uvdata->buffer+desc->lrec*loVis+desc->ilocu;
  v   = uvdata->buffer+desc->lrec*loVis+desc->ilocv;
  w   = uvdata->buffer+desc->lrec*loVis+desc->ilocw;
  vis = uvdata->buffer+desc->lrec*loVis+desc->nrparm;

  /* what needed */
  doShift = (in->dxc!=0.0) || (in->dyc!=0.0) || (in->dzc!=0.0);
  doShift = doShift && (!in->doBeam); /* no shift for beam */

  /* Baseline max, min values */
  blmax2 = in->blmax * in->blmax;
  blmin2 = in->blmin * in->blmin;

  /* guardband in wavelengths */
  guardu = ((1.0-in->guardband) * (ofloat)in->grid->naxis[0]) / fabs(in->UScale);
  guardv = ((1.0-in->guardband) * ((ofloat)in->grid->naxis[1])/2) / fabs(in->VScale);

  /* Loop over visibilities */
  for (ivis=loVis; ivis<hiVis; ivis++) {

    /* check exterma */
    bl2 = (*u)*(*u) + (*v)*(*v);
    doFlag = ((bl2<blmin2) || (bl2>blmax2));

    /* rotate (u,v,w) if 3D */
    if (in->do3Dmul ) {
      uu = (*u)*in->URot3D[0][0] + (*v)*in->URot3D[0][1] + (*w)*in->URot3D[0][2];
      vv = (*u)*in->URot3D[1][0] + (*v)*in->URot3D[1][1] + (*w)*in->URot3D[1][2];
      ww = (*u)*in->URot3D[2][0] + (*v)*in->URot3D[2][1] + (*w)*in->URot3D[2][2];
      *u = uu;
      *v = vv;
      *w = ww;
    } /* end rotate u,v,w */
    
    /* in the correct half plane? */
    flip = (*u) <= 0.0;

    /* loop over IFs */
    ifvis = vis;
    for (iif = 0; iif<nif; iif++) {

      /* loop over frequencies */
      vvis = ifvis;
      for (ifreq = loFreq; ifreq<=hiFreq; ifreq++) {
	ifq = iif*fincif + ifreq*fincf;  /* index in IF/freq table */

	/* is this one wanted? */
	if (doFlag)  vvis[2] = 0.0;  /* baseline out of range? */
	wt = vvis[2];                /* data weight */
	if (wt <= 0.0) {vvis += desc->incf; continue;}
	
	/* Scale coordinates to frequency */
	uf = *u * desc->fscale[ifq];
	vf = *v * desc->fscale[ifq];
	wf = *w * desc->fscale[ifq];

	/* shift position if needed */
	if (doShift) {
	  phase = (uf*in->dxc + vf*in->dyc + wf*in->dzc);
	  cp = cos(phase);
	  sp = sin(phase);
	  vr = vvis[0];
	  vi = vvis[1];
	  /* rotate phase of visibility */
	  vvis[0] = cp * vr - sp * vi;
	  vvis[1] = sp * vr + cp * vi;
	}
	
	/* Making a beam - if so replace data with (1,0) */
	if (in->doBeam) {
	  vvis[0] = 1.0;
	  vvis[1] = 0.0;
	}
	
	/* conjugate phase if needed */
	if (flip)  vvis[1] = - vvis[1];
	
	/* enforce guardband */
	if ((fabs(uf)>guardu) || (fabs(vf)>guardv)) vvis[2] = 0.0;
	
	vvis += desc->incf; /* visibility pointer */
      } /* end loop over frequencies */
      ifvis += desc->incif; /* visibility pointer */
    } /* Loop over IFs */

    /* Scale u,v,w to cells at reference frequency */
    if (flip) { /* put in other half plane */
      *u = -((*u) * in->UScale);
      *v = -((*v) * in->VScale);
      *w = -((*w) * in->WScale);
    } else { /* no flip */
      *u *= in->UScale;
      *v *= in->VScale;
      *w *= in->WScale;
    }

    /* update data pointers */
    u += desc->lrec;
    v += desc->lrec;
    w += desc->lrec;
    vis += desc->lrec;
  } /* end loop over visibilities */
} /* end PrepBuffer */

/**
 * Convolves data in buffer on uvdata onto accGrid
 * Rows in the grid are in U and the data should have all been converted to the 
 * positive U half plane.
 * U, V, and W should be in cells and data not to be included on the grid should 
 * have zero weight.  Convolution functions must be created.
 * Details of data organization are set by FFTW, the zero v row is first and v=-1
 * row is last.
 * \param in      UVGrid Object 
 * \param uvdata  Object with uv data in buffer, prepared for gridding.
 * \param loVis   (0-rel) first vis in buffer in uv data
 * \param hiVis   (1-rel) highest vis in buffer in uv data
 * \param accGrid Grid to accumulate onto
 */
void GridBuffer (ObitUVGrid* in, ObitUV *uvdata, olong loVis, olong hiVis,
			ObitCArray *accGrid)
{
  olong ivis, nvis, ifreq, nfreq, ncol=0, iu, iv, iuu, ivv, icu, icv, lGridRow, lGridCol, itemp;
  olong iif, nif, ifq, loFreq, hiFreq;
  ofloat *grid, *ggrid, *cconvu, *convu, *convv, *cconvv, *u, *v, *w, *vis, *vvis, *ifvis, *wt;
  ofloat *convfnp, *gridStart, *gridTop, visWtR, visWtI, visWtVR, visWtVI, rtemp, xtemp;
  ofloat uf, vf;
  olong fincf, fincif;
  olong pos[] = {0,0,0,0,0};
  ObitUVDesc *desc;

  /* error checks */
  g_assert (ObitUVGridIsA(in));
  g_assert (ObitUVIsA(uvdata));
  g_assert (accGrid != NULL);
  g_assert (uvdata->myDesc != NULL);
  g_assert (uvdata->buffer != NULL);

    /* debug
    rtemp = ObitCArrayMaxAbs(accGrid, pos);
    fprintf (stderr,"before grid buffer Beam grid max %f at %d  %d\n",rtemp, pos[0],pos[1]); */

  /* how much data? */
  desc  = uvdata->myDesc;
  nvis  = desc->numVisBuff;
  if (nvis<=0) return; /* need something */
  nfreq = desc->inaxes[desc->jlocf];
  nif = 1;
  if (desc->jlocif>=0) nif = desc->inaxes[desc->jlocif];
 
  /* range of channels (0-rel) */
  loFreq = in->startChann-1;
  hiFreq = loFreq + in->numberChann;
  if (in->numberChann<=0) hiFreq = (nfreq - in->startChann);

  /* Channel and IF increments in frequency scaling array */
  fincf  = MAX (1, (desc->incf  / 3) / desc->inaxes[desc->jlocs]);
  fincif = MAX (1, (desc->incif / 3) / desc->inaxes[desc->jlocs]);

 /* initialize data pointers */
  u   = uvdata->buffer+desc->lrec*loVis+desc->ilocu;
  v   = uvdata->buffer+desc->lrec*loVis+desc->ilocv;
  w   = uvdata->buffer+desc->lrec*loVis+desc->ilocw;
  vis = uvdata->buffer+desc->lrec*loVis+desc->nrparm;

  lGridRow = 2*accGrid->naxis[0]; /* length of row as floats */
  lGridCol = accGrid->naxis[1];   /* length of column */

  /* convolution fn pointer */
  pos[0] = 0; pos[1] = 0;
  convfnp = ObitFArrayIndex (in->convfn, pos);

  /* beginning of the grid */
  pos[0] = 0;  pos[1] = 0;
  gridStart = ObitCArrayIndex (accGrid, pos); 
  /* beginning of highest row */
  pos[1] = lGridCol-1;
  gridTop = ObitCArrayIndex (accGrid, pos); 
  
  /* Loop over visibilities */
  for (ivis=loVis; ivis<hiVis; ivis++) {

    /* loop over IFs */
    ifvis = vis;
    for (iif=0; iif<nif; iif++) {

   /* loop over frequencies */
      vvis = ifvis;
      for (ifreq = loFreq; ifreq<=hiFreq; ifreq++) {
	ifq = iif*fincif + ifreq*fincf;  /* index in IF/freq table */

	/* is this one wanted? */
	wt = vvis + 2; /* data weight */
	if (*wt <= 0.0) {vvis += desc->incf; continue;}

	/* data times weight */
	visWtR = vvis[0] * (*wt);
	visWtI = vvis[1] * (*wt);
	
	/* Scale u,v for frequency (w not used) */
	uf = *u * desc->fscale[ifq];
	vf = *v * desc->fscale[ifq];
	
	/* get center cell */
	if (vf > 0.0) iv = (olong)(vf + 0.5);
	else iv = (olong)(vf - 0.5);
	iu = (olong)(uf + 0.5);
	
	/* back off half Kernel width */
	iu -= in->convWidth/2;
	iv -= in->convWidth/2;
	
	/* Starting convolution location, table has in->convNperCell points per cell */
	/* Determine fraction of the cell to get start location in convolving table. */
	if (uf > 0.0) itemp = (olong)(uf + 0.5);
	else itemp = ((olong)(uf - 0.5));
	xtemp = in->convNperCell*(itemp - (uf) - 0.5);
	if (xtemp > 0.0) xtemp += 0.5;
	else xtemp -= 0.5;
	convu = convfnp + in->convNperCell + (olong)xtemp;
	
	/* now v convolving fn */
	if (vf > 0.0) itemp = (olong)(vf + 0.5);
	else itemp = ((olong)(vf - 0.5));
	rtemp = in->convNperCell*(itemp - (vf) - 0.5);
	if (rtemp > 0.0) rtemp += 0.5;
	else rtemp -= 0.5;
	convv = convfnp + in->convNperCell + (olong)rtemp;
	
	/* if too close to the center, have to break up and do conjugate halves */
	if (iu >= 0) { /* all in same half */
	  ncol = in->convWidth; /* Complex addressed as floats */
	  pos[0] = iu;
	  /* Do v center at the edges */
	  if (iv>=0) pos[1] = iv;
	  else pos[1] = iv + lGridCol;
	  
	} else { 
	  /* have to split - grid part in conjugate half */
	  /* FFTW uses only half of the first plane so split U */
	  iuu = -iu; /* hermitian */
	  ivv = -iv;
	  pos[0] = iuu;
	  /* Do v center at the edges */
	  if (ivv>=0) pos[1] = ivv;
	  else pos[1] = ivv + lGridCol;
	  /*grid = ObitCArrayIndex (accGrid, pos); pointer in grid */ 
	  if ((pos[0]<=accGrid->naxis[0]) && (pos[1]<=accGrid->naxis[1]) &&
	      (pos[0]>=0) && (pos[1]>=0) )
	    grid = accGrid->array+2*(pos[1]*accGrid->naxis[0]+pos[0]);
	  else
	    grid = NULL;

	  /* Ignore if outside grid */
	  if (grid!=NULL) {
	    ncol = iuu;
	    cconvv = convv;
	    for (icv=0; icv<in->convWidth; icv++) {
	      cconvu = convu;
	      visWtVR = visWtR * (*cconvv);
	      visWtVI = visWtI * (*cconvv);
	      /* Trickery with the v row to get data in the correct place for the FFT 
		 the following will only be triggered if the iv wraps */
	      if ((pos[1]-icv)==-1) {
		grid = gridTop+2*iuu; /* top of grid */
	      }
	      ggrid  = grid;
	      for (icu=0; icu<=ncol; icu++) {
		ggrid[0]   += visWtVR * (*cconvu);
		ggrid[1]   -= visWtVI * (*cconvu); /* conjugate */
		cconvu += in->convNperCell;  /* U Convolution kernel pointer */
		ggrid -= 2; /* gridding pointer - opposite of normal gridding */
	      } /* end inner u gridding loop */
	      cconvv += in->convNperCell;  /* V Convolution kernel pointer */
	      grid -= lGridRow; /* gridding pointer - reverse direction for conjugate */
	    } /* end outer v loop */
	    
	    /* set up for rest of grid */
	    ncol = (in->convWidth + iu); /* how many columns left? */
	    iu = 0;      /* by definition  start other half plane at iu=0 */
	    pos[0] = iu; 
	    /* Do v center at the edges */
	    if (iv>=0) pos[1] = iv;
	    else pos[1] = iv + lGridCol;
	    convu = convu + iuu * in->convNperCell; /* for other half in u */
	  } /* end if in grid */
	} /* End of dealing with conjugate portion */
	  
	/* main loop gridding - only if in grid */
	/*grid = ObitCArrayIndex (accGrid, pos);   pointer in grid */
	if ((pos[0]<=accGrid->naxis[0]) && (pos[1]<=accGrid->naxis[1]) &&
	    (pos[0]>=0) && (pos[1]>=0) )
	  grid = accGrid->array+2*(pos[1]*accGrid->naxis[0]+pos[0]);
	else
	  grid = NULL;

	if (grid!=NULL) {
	  for (icv=0; icv<in->convWidth; icv++) {
	    cconvu = convu;
	    visWtVR = visWtR * (*convv);
	    visWtVI = visWtI * (*convv);
	    /* Trickery with the v row to get data in the correct place for the FFT 
	       the following will only be triggered if the iv row goes non negative */
	    if ((iv<0) && ((iv+icv)==0)) grid = gridStart+2*iu; /* beginning of grid */
	    ggrid  = grid;
	    for (icu=0; icu<ncol; icu++) {
	      ggrid[0] += visWtVR * (*cconvu);  /* real */
	      ggrid[1] += visWtVI * (*cconvu) ; /* imag */

	      /* Hard core debug
	      if (ggrid-gridStart==72) {
		fprintf (stdout," reglr %10.5f %10.5f %3ld %10.5f %10.5f %15.5f %15.5f %5ld %5ld  %d\n",
			 uf, vf, ifq, visWtVR*(*cconvu), visWtVI*(*cconvu),ggrid[0],ggrid[1],icu,icv,ggrid-gridStart);
	      } */
	      cconvu += in->convNperCell;  /* Convolution kernel pointer */
	      ggrid += 2; /* gridding pointer */
	    } /* end inner gridding loop */
	    convv += in->convNperCell;  /* Convolution kernel pointer */
	    grid += lGridRow; /* gridding pointer */
	  } /* end outer gridding loop */
	} /* end if in grid */
	vvis += desc->incf; /* visibility pointer */
	
      } /* end loop over frequencies */
      ifvis += desc->incif; /* visibility pointer */
    } /* Loop over IFs */
    
    /* update data pointers */
    u += desc->lrec;
    v += desc->lrec;
    w += desc->lrec;
    vis += desc->lrec;
  } /* end loop over visibilities */
} /* end GridBuffer */

/**
 * Calculates convolving function and attaches it to in.
 * Compute Spherodial wave function convolving function table.
 * Algorithm lifted from AIPS.
 * \param in      Object with table to init.
 * \param fnType  Function type
 *                \li 0 = pillbox, 
 *                \li 4 = =Exp*Sinc
 *                \li 5 = Spherodial wave
 */
static void ConvFunc (ObitUVGrid* in, olong fnType)
{
  ofloat parm[4]; /* default parameters */
  ofloat xinc, eta, psi, p1, p2, u, absu, umax, *convfnp;
  olong ialf, im, nmax, i, size, lim, limit, bias, naxis[1];
  /*gfloat shit[701]; DEBUG */

  /* error checks */
  g_assert (ObitUVGridIsA(in));


  /*+++++++++++++++++ Pillbox ++++++++++++++++++++++++++++++++++++++++*/
  if (fnType==0) {
   /* set parameters */
    parm[0] = 0.5;   /* AIPS defaults */
    in->convWidth      = 3; /* Width of convolving kernel in cells */
    in->convNperCell = 100; /* Number of of tabulated points per cell in convfn */

    /* allocate array*/
    lim = in->convWidth * in->convNperCell + 1;
    size = lim;
    naxis[0] = size;
    in->convfn = ObitFArrayUnref(in->convfn);
    in->convfn = ObitFArrayCreate (in->name, 1L, naxis);

    /* get pointer to memory array */
    naxis[0] = 0;
    convfnp = ObitFArrayIndex (in->convfn, naxis);

    /* fill function */
    xinc = 1.0 / (ofloat)in->convNperCell;
    umax = parm[0];
    bias = (in->convNperCell/2) * in->convWidth;
    for (i=0; i<lim; i++) {
      u = (i-bias) * xinc;
      absu = fabs (u);
      convfnp[i] = 1.0;
      if (absu == umax) convfnp[i] = 0.5;
      else if (absu > umax)  convfnp[i] = 0.0;
    }
    
   } else if (fnType==4) {
  /*+++++++++++++++++ Exp Sinc ++++++++++++++++++++++++++++++++++++++++*/
    /* set parameters */
     parm[0] = 3.0;   /* AIPS defaults */
     parm[1] = 1.55;
     parm[2] = 2.52;
     parm[3] = 2.00;
    in->convWidth    = 1.5 + 2*parm[0]; /* Width of convolving kernel in cells */
    in->convNperCell = 100; /* Number of of tabulated points per cell in convfn */
    p1 = G_PI / parm[1];
    p2 = 1.0 / parm[2];

    /* allocate array*/
    lim = in->convWidth * in->convNperCell + 1;
    size = lim;
    naxis[0] = size;
    in->convfn = ObitFArrayUnref(in->convfn);
    in->convfn = ObitFArrayCreate (in->name, 1L, naxis);

    /* get pointer to memory array */
    naxis[0] = 0;
    convfnp = ObitFArrayIndex (in->convfn, naxis);

    /* fill function */
    bias = (in->convNperCell/2) * in->convWidth;
    xinc = 1.0 / (ofloat)in->convNperCell;
    umax = parm[0];
    for (i=0; i<lim; i++) {
      u = (i - lim/2 - 1) * xinc;
      absu = fabs (u);
      convfnp[i] = 0.0;

      /* trap center */
      if (absu<xinc) convfnp[i] = 1.0;
      else if (absu <= umax) convfnp[i] =  sin(u*p1) / (u*p1) *
			       exp (-pow ((absu * p2), parm[3]));
    }
    

   } else if (fnType==5) {

    /*+++++++++++++++++ Spherodial wave ++++++++++++++++++++++++++++++++*/
    /* set parameters */
    in->convWidth    = 7;   /* Width of convolving kernel in cells */
    in->convNperCell = 100; /* Number of of tabulated points per cell in convfn */
    parm[0] = in->convWidth/ 2;
    parm[1] = 1.0;
    xinc = 1.0 / ((ofloat)in->convNperCell);
    
    /* allocate array*/
    lim = in->convWidth * in->convNperCell + 1;
    size = lim;
    naxis[0] = size;
    in->convfn = ObitFArrayUnref(in->convfn);
    in->convfn = ObitFArrayCreate (in->name, 1L, naxis);
    /* get pointer to memory array */
    naxis[0] = 0;
    convfnp = ObitFArrayIndex (in->convfn, naxis);
    
    nmax = parm[0]*in->convNperCell + 0.1;
    bias = (in->convNperCell/2) * in->convWidth;
    ialf = 2.0 * parm[1] + 1.1;
    im = 2.0 * parm[0] + 0.1;
    
    /* constrain range */
    im = MAX (4, MIN (8, im));
    
    /* compute half of the (symmetric) function */
    for (i=0; i<nmax; i++) {
      eta = (float)i / (float)(nmax - 1);
      psi = sphfn (ialf, im, 0, eta);
      /* DEBUG - use pillbox
	 if (i<in->convWidth/2) psi = 1.0;
	 else psi = 0.0;  */
      convfnp[bias+i] = psi;
    }
    
    /* Fill in other half */
    limit = bias-1;
    for (i=1; i<=limit; i++) convfnp[bias-i] = convfnp[bias+i];
    
  } /* end computing convolving fn */
  else { /* should never get here */
    g_error("Unknown convolving function type %d",fnType);
  }
    /* DEBUG  - fuck gdb
  for (i=0; i<701; i++) shit[i] = in->convfn->array[i];*/
} /* end ConvFunc */


/**
 * Compute Spherodial wave function convolving function table.
 * Algorithm lifted from AIPS (author F. Schwab).
 * \param ialf  Selects the weighting exponent, alpha
 *              (IALF = 1, 2, 3, 4, and 5 correspond to
 *              alpha = 0, 1/2, 1, 3/2, and 2, resp.).
 * \param im    support width (4, 5, 6, 7, or 8)
 * \param iflag Chooses whether the spheroidal function itself, 
 *              or its Fourier transform, is to be approximated.  
 *              The latter is appropriate for gridding, and the former 
 *              for the u-v plane convolution.  
 *              The two differ by a factor (1-eta**2)**alpha.  
 *              iflag less than or equal to zero chooses the function
 *              appropriate for gridding, and iflag positive chooses 
 *              its Fourier transform.
 * \param eta   Eta, as the argument of the spheroidal function, 
 *              is a variable which ranges from 0 at the center of the 
 *              convoluting function to 1 at its edge (also from 0 at 
 *              the center of the gridding correction function to unity at
 *              the edge of the map).  range [0,1].
 * \return spherical wave function. -999.99 -> input error.
 */
static ofloat sphfn (olong ialf, olong im, olong iflag, ofloat eta)
{
  float psi=0.0, eta2, x;
  olong   j, ierr;
  static ofloat alpha[5] = {0.0, 0.5, 1.0, 1.5, 2.0};
  static ofloat p4[5][5] = {
    {1.584774e-2, -1.269612e-1,  2.333851e-1, -1.636744e-1, 5.014648e-2},
    {3.101855e-2, -1.641253e-1,  2.385500e-1, -1.417069e-1, 3.773226e-2},
    {5.007900e-2, -1.971357e-1,  2.363775e-1, -1.215569e-1, 2.853104e-2},
    {7.201260e-2, -2.251580e-1,  2.293715e-1, -1.038359e-1, 2.174211e-2},
    {9.585932e-2, -2.481381e-1,  2.194469e-1, -8.862132e-2, 1.672243e-2}};
  static ofloat q4[5][2] = {
    {4.845581e-1,  7.457381e-2},  {4.514531e-1,  6.458640e-2},
    {4.228767e-1,  5.655715e-2},  {3.978515e-1,  4.997164e-2},
    {3.756999e-1,  4.448800e-2}};
  static ofloat p5[5][7] = {
    {3.722238e-3, -4.991683e-2,  1.658905e-1, -2.387240e-1, 1.877469e-1, -8.159855e-2,  3.051959e-2},  
    {8.182649e-3, -7.325459e-2,  1.945697e-1, -2.396387e-1, 1.667832e-1, -6.620786e-2,  2.224041e-2},  
    {1.466325e-2, -9.858686e-2,  2.180684e-1, -2.347118e-1, 1.464354e-1, -5.350728e-2,  1.624782e-2},  
    {2.314317e-2, -1.246383e-1,  2.362036e-1, -2.257366e-1, 1.275895e-1, -4.317874e-2,  1.193168e-2},
    {3.346886e-2, -1.503778e-1,  2.492826e-1, -2.142055e-1, 1.106482e-1, -3.486024e-2,  8.821107e-3}};
  static ofloat q5[5] = 
    {2.418820e-1,  2.291233e-1,  2.177793e-1,  2.075784e-1, 1.983358e-1};
  static ofloat p6l[5][5] = {
    {5.613913e-2, -3.019847e-1,  6.256387e-1, -6.324887e-1, 3.303194e-1},  
    {6.843713e-2, -3.342119e-1,  6.302307e-1, -5.829747e-1, 2.765700e-1},  
    {8.203343e-2, -3.644705e-1,  6.278660e-1, -5.335581e-1, 2.312756e-1},  
    {9.675562e-2, -3.922489e-1,  6.197133e-1, -4.857470e-1, 1.934013e-1},
    {1.124069e-1, -4.172349e-1,  6.069622e-1, -4.405326e-1, 1.618978e-1}};
  static ofloat q6l[5][2] = {
    {9.077644e-1,  2.535284e-1},  {8.626056e-1,  2.291400e-1}, 
    {8.212018e-1,  2.078043e-1},  {7.831755e-1,  1.890848e-1},  
    {7.481828e-1, 1.726085e-1}};
  static ofloat p6u[5][5] = {
    {8.531865e-4, -1.616105e-2,  6.888533e-2, -1.109391e-1, 7.747182e-2},  
    {2.060760e-3, -2.558954e-2,  8.595213e-2, -1.170228e-1, 7.094106e-2},  
    {4.028559e-3, -3.697768e-2,  1.021332e-1, -1.201436e-1, 6.412774e-2},  
    {6.887946e-3, -4.994202e-2,  1.168451e-1, -1.207733e-1, 5.744210e-2},
    {1.071895e-2, -6.404749e-2,  1.297386e-1, -1.194208e-1, 5.112822e-2}};
  static ofloat q6u[5][2] = {
    {1.101270e+0,  3.858544e-1},  {1.025431e+0,  3.337648e-1},
    {9.599102e-1,  2.918724e-1},  {9.025276e-1,  2.575336e-1},
    {8.517470e-1,  2.289667e-1}};
  static ofloat p7l[5][5] = {
    {2.460495e-2, -1.640964e-1,  4.340110e-1, -5.705516e-1, 4.418614e-1},  
    {3.070261e-2, -1.879546e-1,  4.565902e-1, -5.544891e-1, 3.892790e-1},  
    {3.770526e-2, -2.121608e-1,  4.746423e-1, -5.338058e-1, 3.417026e-1},  
    {4.559398e-2, -2.362670e-1,  4.881998e-1, -5.098448e-1, 2.991635e-1},
    {5.432500e-2, -2.598752e-1,  4.974791e-1, -4.837861e-1, 2.614838e-1}};
  static ofloat q7l[5][2] = {
    {1.124957e+0,  3.784976e-1},  {1.075420e+0,  3.466086e-1},
    {1.029374e+0,  3.181219e-1},  {9.865496e-1,  2.926441e-1},
    {9.466891e-1,  2.698218e-1}};
  static  ofloat p7u[5][5] = {
    {1.924318e-4, -5.044864e-3,  2.979803e-2, -6.660688e-2, 6.792268e-2},  
    {5.030909e-4, -8.639332e-3,  4.018472e-2, -7.595456e-2, 6.696215e-2},  
    {1.059406e-3, -1.343605e-2,  5.135360e-2, -8.386588e-2, 6.484517e-2},  
    {1.941904e-3, -1.943727e-2,  6.288221e-2, -9.021607e-2, 6.193000e-2},
    {3.224785e-3, -2.657664e-2,  7.438627e-2, -9.500554e-2, 5.850884e-2}};
  static ofloat q7u[5][2] = {
    {1.450730e+0,  6.578685e-1},  {1.353872e+0,  5.724332e-1}, 
    {1.269924e+0,  5.032139e-1},  {1.196177e+0,  4.460948e-1},  
    {1.130719e+0,  3.982785e-1}};
  static ofloat p8l[5][6] = {
    {1.378030e-2, -1.097846e-1,  3.625283e-1, -6.522477e-1, 6.684458e-1, -4.703556e-1},  
    {1.721632e-2, -1.274981e-1,  3.917226e-1, -6.562264e-1, 6.305859e-1, -4.067119e-1},
    {2.121871e-2, -1.461891e-1,  4.185427e-1, -6.543539e-1, 5.904660e-1, -3.507098e-1},  
    {2.580565e-2, -1.656048e-1,  4.426283e-1, -6.473472e-1, 5.494752e-1, -3.018936e-1},
    {3.098251e-2, -1.854823e-1,  4.637398e-1, -6.359482e-1, 5.086794e-1, -2.595588e-1}};
  static ofloat q8l[5][2] = {
    {1.076975e+0,  3.394154e-1},  {1.036132e+0,  3.145673e-1},
    {9.978025e-1,  2.920529e-1},  {9.617584e-1,  2.715949e-1},
    {9.278774e-1,  2.530051e-1}};
  static ofloat p8u[5][6] = {
    {4.290460e-5, -1.508077e-3,  1.233763e-2, -4.091270e-2, 6.547454e-2, -5.664203e-2},  
    {1.201008e-4, -2.778372e-3,  1.797999e-2, -5.055048e-2, 7.125083e-2, -5.469912e-2},
    {2.698511e-4, -4.628815e-3,  2.470890e-2, -6.017759e-2, 7.566434e-2, -5.202678e-2},  
    {5.259595e-4, -7.144198e-3,  3.238633e-2, -6.946769e-2, 7.873067e-2, -4.889490e-2},
    {9.255826e-4, -1.038126e-2,  4.083176e-2, -7.815954e-2, 8.054087e-2, -4.552077e-2}};
 static ofloat q8u[5][2] = {
   {1.379457e+0,  5.786953e-1},  {1.300303e+0,  5.135748e-1},
   {1.230436e+0,  4.593779e-1},  {1.168075e+0,  4.135871e-1},
   {1.111893e+0,  3.744076e-1}};

   ierr = 0;
   /*  Check inputs. */
   if ((ialf<1) || (ialf>5)) ierr = 1;
   if ((im<4) || (im>8)) ierr = 2 + 10 * ierr;
   if (fabs(eta)>1.) ierr = 3 + 10 * ierr;
   if (ierr!=0) return -999.9;
   /*  So far, so good. */
   eta2 = eta*eta;
   j = ialf-1;

   /*  Branch on support width. */
   switch (im) {
 
   case 4:   /*  Support width = 4 cells. */
     x = eta2 - 1.0;
     psi = (p4[j][0] + x * (p4[j][1] + x * (p4[j][2] + x * (p4[j][3] + x * p4[j][4])))) / 
       (1.0 + x * (q4[j][0] + x * q4[j][1]));
     break;

   case 5:   /* Support width = 5 cells. */
     x = eta2 - 1.0;
     psi = (p5[j][0] + x * (p5[j][1] + x * (p5[j][2] + x * (p5[j][3] +  x * (p5[j][4] + x * (p5[j][5] + x * p5[j][6]))))))
       / (1.0 + x * q5[j]);
     break;

  case 6: /* Support width = 6 cells. */
       if (fabs(eta)<=0.75) {
	 x = eta2 - 0.5625;
	 psi = (p6l[j][0] + x * (p6l[j][1] + x * (p6l[j][2] + x * (p6l[j][3] + x * p6l[j][4])))) / 
	   (1.0 + x * (q6l[j][0] +  x * q6l[j][1]));
       } else {
	 x = eta2 - 1.0;
	 psi = (p6u[j][0] + x * (p6u[j][1] + x * (p6u[j][2] + x * (p6u[j][3] + x * p6u[j][4])))) / 
	   (1.0 + x * (q6u[j][0] + x * q6u[j][1]));
       }
     break;
     
   case 7: /* Support width = 7 cells. */
       if (fabs(eta)<=0.775) {
         x = eta2 - 0.600625;
         psi = (p7l[j][0] + x * (p7l[j][1] + x * (p7l[j][2] + x * (p7l[j][3] + x * p7l[j][4])))) / 
	   (1.0 + x * (q7l[j][0] + x * q7l[j][1]));
       } else {
	 x = eta2 - 1.0;
	 psi = (p7u[j][0] + x * (p7u[j][1] + x * (p7u[j][2] + x * (p7u[j][3] +  x * p7u[j][4])))) / 
	   (1.0 + x * (q7u[j][0] + x * q7u[j][1]));
       }
     break;

  case 8:      /* Support width = 8 cells. */
      if (abs(eta)<=0.775) {
	x = eta2 - .600625;
         psi = (p8l[j][0] + x * (p8l[j][1] + x * (p8l[j][2] + x * (p8l[j][3] + x * (p8l[j][4] + x * p8l[j][5]))))) /
           (1.0 + x * (q8l[j][0] + x * q8l[j][1]));
      } else {
	x = eta2 - 1.0;
      psi = (p8u[j][0] + x * (p8u[j][1] + x * (p8u[j][2] + x * (p8u[j][3] + x * (p8u[j][4] + x * p8u[j][5]))))) / 
	(1.0 + x * (q8u[j][0] + x * q8u[j][1]));
      }
      break;
      
   default: /* should never get here */
     g_assert_not_reached(); 
   }; /* end switch */

   /* Done? */
   if ((iflag>0) || (ialf==1) || (eta==0.0)) return psi;

   /* correction function */
   if (abs(eta) == 1.0) psi = 0.0;
   else psi = pow((1.0 - eta2), alpha[ialf-1]) * psi;

   return psi;
} /* end sphfn */

/**
 * Use direct Fourier transform of tabulated convolving function
 * to compute gridding correction function in one dimension.
 * This heavy handed approach is needed since the convolution function
 * actually used was the tabulated one.
 * Algorithm lifted from AIPS.
 * \param in      Object with convolution function.
 * \param n       Dimensionality of image on this axis.
 * \param icent   Center pixel (1-rel) on this axis.
 * \param data    (complex) work array the size of the convolution table.
 * \param ramp    (complex) work array the size of the convolution table.
 * \param out     1-D correction function for this axis.
 */
void GridCorrFn (ObitUVGrid* in, long n, olong icent, 
		 ofloat *data, ofloat *ramp, ObitFArray *out)
{
  ofloat p1, p2, p3, p4, sumre, tr, ti, amp, phase; 
  olong i, j, size, bias;

/* Phases for ramp */
  size = in->convWidth * in->convNperCell + 1;
  bias = in->convWidth * (in->convNperCell/2) + 1;
  p4 = 2.0 * G_PI / (in->convNperCell * n);
  p1 = p4 * (1.0 - (ofloat)icent) * (1.0 - (ofloat)bias);
  p2 = p4 * (1.0 - (ofloat)icent);
  p3 = p4 * (1.0 - (ofloat)bias);

  /* fill arrays */
  for (i=0, j=0; i<size; i++) {
    /* Convolution function with initial phase */
    amp = in->convfn->array[i];
    phase = p1 + i * p2;
    data[j]   = amp * cos(phase);
    data[j+1] = amp * sin(phase);

    /* phase ramp for FT */
    phase = p3 + i * p4;
    ramp[j]   = cos(phase);
    ramp[j+1] = sin(phase);
    j += 2;
  }
  
  /* Loop doing cosine Fourier transform by iteratively applying phase ramp 
     and summing the real part. */
  for (i=0; i<n; i++) {
    sumre = 0.0;
    for (j=0; j<2*size; j+=2) {
      sumre += data[j]; /* sum real part */
      /* rotate phase by ramp */
      tr = data[j]*ramp[j] - data[j+1]*ramp[j+1];
      ti = data[j]*ramp[j+1] + data[j+1]*ramp[j];
      data[j]   = tr;
      data[j+1] = ti;
    }
    out->array[i] = sumre;
  } /* end FT loop */

  /* Normalize  function by the center */
  ObitFArraySMul (out, 1.0/out->array[n/2]);

  /* Need inverse for correction */
  ObitFArraySDiv (out, 1.0);
 
} /* end GridCorrFn */

 /**
 * Grid a buffer load of data into a single image
 * \param in      Gridding Object
 * \param UVin    UV data set to grid from current buffer
 * \param sargs   Array of arguments to use, Must be in->nThreads of these
 * \param thread  Thread object to use
 * \param err     ObitErr stack for reporting problems.
 */
static void GridOne (ObitUVGrid* in, ObitUV *UVin, UVGridFuncArg **sargs, 
		     ObitThread *thread, ObitErr *err)
{
  olong i, nvis, lovis, hivis, nvisPerThread, nThreads;
  ObitThreadFunc func=(ObitThreadFunc)ThreadUVGridBuffer ;
  UVGridFuncArg *args;
  gboolean  OK;
  gchar *routine="GridOne";

  /* error checks */
  if (err->error) return;

  /* Set up thread arguments */
  for (i=0; i<in->nThreads; i++) {
    args = sargs[i];
    args->thread = thread;
    args->in     = in;
    args->UVin   = UVin;
    if (i>0) { /* Which accumulation grid */
      args->grid = in->workGrids[i-1];
    } else { /* Use main one for first or only thread */
      args->grid = in->grid;
    }
  }

  /* Divide up work */
  nvis = UVin->myDesc->numVisBuff;
  if (nvis<100) nThreads = 1;
  else nThreads = in->nThreads;
  nvisPerThread = nvis/nThreads;
  lovis = 1;
  hivis = nvisPerThread;
  hivis = MIN (hivis, nvis);
  
  /* Set up thread arguments */
  for (i=0; i<nThreads; i++) {
    if (i==(nThreads-1)) hivis = nvis;  /* Make sure do all */
    args =  sargs[i];
    args->first  = lovis;
    args->last   = hivis;
    if (nThreads>1) args->ithread = i;
    else args->ithread = -1;
    /* Update which vis */
    lovis += nvisPerThread;
    hivis += nvisPerThread;
    hivis = MIN (hivis, nvis);
  }
  
  /* Do operation on buffer possibly with threads */
  OK = ObitThreadIterator (thread, nThreads, func, (gpointer)sargs);
    
  /* Check for problems */
  if (!OK) 
    Obit_log_error(err, OBIT_Error,"%s: Problem in threading", routine);

} /* end GridOne  */

/** 
 * Prepare and Grid a portion of the data buffer
 * Arguments are given in the structure passed as arg
 * Note the images and beams are not normalized.
 * \param arg  Pointer to UVGridFuncArg argument with elements
 * \li thread Thread with restart queue
 * \li in     ObitUVGrid object
 * \li UVin   UV data set to grid from current buffer
 * \li first  First (1-rel) vis in UVin buffer to process this thread
 * \li last   Highest (1-rel) vis in UVin buffer to process this thread
 * \li ithread thread number, >0 -> no threading 
 * \li buffSize if >0 then the number of ofloats to copy from buffer to buffer on UVin
 * \li buffer   Data buffer to copy
 */
static gpointer ThreadUVGridBuffer (gpointer arg)
{
  /* Get arguments from structure */
  UVGridFuncArg *largs = (UVGridFuncArg*)arg;
  ObitUVGrid *in   = largs->in;
  ObitUV *UVin     = largs->UVin;
  olong loVis      = largs->first-1;
  olong hiVis      = largs->last;
  ObitCArray *grid = largs->grid;
  olong buffSize   = largs->buffSize;
  ofloat *buffer   = largs->buffer;
 
  gsize size, offset;
  ObitUVGridClassInfo *gridClass = (ObitUVGridClassInfo*)in->ClassInfo;

  /* Need to copy data? */
  if ((buffer!=NULL) && (buffSize>0)) {
    size = (hiVis-loVis)*UVin->myDesc->lrec*sizeof(ofloat);
    offset = loVis*UVin->myDesc->lrec;
    memcpy (&UVin->buffer[offset], &buffer[offset], size);
  }

  /* prepare data */
  gridClass->PrepBuffer (in, UVin, loVis, hiVis);
  
  /* grid */
  gridClass->GridBuffer (in, UVin, loVis, hiVis, grid);

  /* Indicate completion */
  if (largs->ithread>=0)
    ObitThreadPoolDone (largs->thread, (gpointer)&largs->ithread);
  
  return NULL;
} /* end ThreadUVGridBuffer */

/** 
 * Reorders grid and do gridding correction.
 * NOTE: threading in FFTW apparently conflicts with Obit threads.
 * Arguments are given in the structure passed as arg
 * \param arg  Pointer to FFT2ImFuncArg argument with elements
 * \li thread Thread with restart queue
 * \li in     Input ObitUVGrid object
 * \li array  Output ObitFArray Image array
 * \li ithread thread number, >0 -> no threading 
 */
static gpointer ThreadFFT2Im (gpointer arg)
{
  /* Get arguments from structure */
  FFT2ImFuncArg *largs = (FFT2ImFuncArg*)arg;
  ObitUVGrid *in     = largs->in;
  ObitFArray *array  = largs->array;

  /* local */
  ofloat *ramp=NULL, *data=NULL;
  olong size, naxis[2];

  /* Beam or image? */
  if (in->doBeam) { 
    /* Making Beam */ 
    /* reorder to center at center */
    ObitFArray2DCenter (array);
    
    /* Do gridding corrections */
    /* Create arrays / initialize if not done */
    if ((in->xCorrBeam==NULL) || (in->yCorrBeam==NULL)) {
      size = in->convWidth * in->convNperCell + 1;
      ramp = g_malloc0(2*size*sizeof(float));
      data = g_malloc0(2*size*sizeof(float));
      naxis[0] = in->nxBeam;
      in->xCorrBeam = ObitFArrayUnref(in->xCorrBeam); /* just in case */
      in->xCorrBeam = ObitFArrayCreate ("X Beam gridding correction", 1, naxis);
      naxis[0] = in->nyBeam;
      in->yCorrBeam = ObitFArrayUnref(in->yCorrBeam); /* just in case */
      in->yCorrBeam = ObitFArrayCreate ("Y Beam gridding correction", 1, naxis);
      
      /* X function */
      GridCorrFn (in, in->nxBeam, in->icenxBeam, data, ramp, in->xCorrBeam);
      
      /* If Y axis */
      GridCorrFn (in, in->nyBeam, in->icenyBeam, data, ramp, in->yCorrBeam);
    } /* end initialize correction functions */
    
    /* Do multiply to make griding correction */
    ObitFArrayMulColRow (array, in->xCorrBeam, in->yCorrBeam, array);
    
  } else { 
    /* Making Image */ 
    /* reorder to center at center */
    ObitFArray2DCenter (array);
    
    /* Do gridding corrections */
    /* Create arrays / initialize if not done */
    if ((in->xCorrImage==NULL) || (in->yCorrImage==NULL)) {
      size = in->convWidth * in->convNperCell + 1;
      ramp = g_malloc0(2*size*sizeof(float));
      data = g_malloc0(2*size*sizeof(float));
      naxis[0] = in->nxImage;
      in->xCorrImage = ObitFArrayUnref(in->xCorrImage); /* just in case */
      in->xCorrImage = ObitFArrayCreate ("X Image gridding correction", 1, naxis);
      naxis[0] = in->nyImage;
      in->yCorrImage = ObitFArrayUnref(in->yCorrImage); /* just in case */
      in->yCorrImage = ObitFArrayCreate ("Y Image gridding correction", 1, naxis);
      
      /* X function */
      GridCorrFn (in, in->nxImage, in->icenxImage, data, ramp, in->xCorrImage);
      
      /* If Y axis */
      GridCorrFn (in, in->nyImage, in->icenyImage, data, ramp, in->yCorrImage);
 
      /* Do multiply to make griding correction */
      ObitFArrayMulColRow (array, in->xCorrImage, in->yCorrImage, array);
    } /* end initialize correction functions */
  } /* end make image */
  
  /* cleanup */
  if (ramp) g_free (ramp); ramp = NULL;
  if (data) g_free (data); data = NULL;

  /* Indicate completion */
  if (largs->ithread>=0)
    ObitThreadPoolDone (largs->thread, (gpointer)&largs->ithread);
  
  return NULL;
} /* end ThreadFFT2Im */

