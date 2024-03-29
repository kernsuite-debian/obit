/* $Id$  */
/* Obit task to Map beam polarization                                 */
/*--------------------------------------------------------------------*/
/*;  Copyright (C) 2009-2015                                          */
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

#include "ObitUVDesc.h"
#include "ObitImageUtil.h"
#include "ObitUVUtil.h"
#include "ObitSystem.h"
#include "ObitMem.h"
#include "ObitParser.h"
#include "ObitReturn.h"
#include "ObitAIPSDir.h"
#include "ObitHistory.h"
#include "ObitData.h"
#include "ObitTableSUUtil.h"
#include "ObitTableANUtil.h"
#include "ObitPrecess.h"

/* internal prototypes */
/* Get inputs */
ObitInfoList* MapBeamIn (int argc, char **argv, ObitErr *err);
/* Set outputs */
void MapBeamOut (ObitInfoList* outList, ObitErr *err);
/* Give basic usage on error */
void Usage(void);
/* Set default inputs */
ObitInfoList* defaultInputs(ObitErr *err);
/* Set default outputs */
ObitInfoList* defaultOutputs(ObitErr *err);
/* Digest inputs */
void digestInputs(ObitInfoList *myInput, ObitErr *err);

/* Get input data */
ObitUV* getInputData (ObitInfoList *myInput, ObitErr *err);

/* Create output image */
ObitImage* setOutput (gchar *Source, olong iStoke, olong ant, 
		      gboolean doRMS, gboolean doPhase,
		      ObitInfoList *myInput, ObitUV* inData, 
		      ObitErr *err);

/* Loop over sources */
void doSources (ObitInfoList* myInput, ObitUV* inData, ObitErr* err);

/* Loop over Channels/Poln */
void doChanPoln (gchar *Source, olong ant, ObitInfoList* myInput, 
		 ObitUV* inData, ObitErr* err);

/* Image */
ObitFArray** doImage (gboolean doRMS, gboolean doPhase, olong ant, 
		      ObitInfoList* myInput, 
		      ObitUV* inData, olong *nchan, olong *nIF, olong *npoln, 
		      ObitErr* err);
/* Write history */
void MapBeamHistory (gchar *Source, gchar Stok, ObitInfoList* myInput, 
		    ObitUV* inData, ObitImage* outImage, ObitErr* err);

/* Average  data */
ObitUV* doAvgData (ObitInfoList *myInput, ObitUV* inData, ObitErr *err);

/* Get list of antennas */
olong* getAntList (ObitInfoList* myInput, ObitUV* inData, ObitErr* err);

/* Accumulate data into lists */
void  accumData (ObitUV* inData, ObitInfoList* myInput, olong ant,
		 olong nchan, olong nIF, olong selem, olong *nelem,
		 ofloat *SumIr, ofloat *SumIi, ofloat *SumII, ofloat *SumIWt,
		 ofloat *SumQr, ofloat *SumQi, ofloat *SumQQ, ofloat *SumQWt,
		 ofloat *SumUr, ofloat *SumUi, ofloat *SumUU, ofloat *SumUWt,
		 ofloat *SumVr, ofloat *SumVi, ofloat *SumVV, ofloat *SumVWt,
		 ofloat *SumAzCell, ofloat *SumElCell, ofloat *SumPACell, 
		 olong *CntCell, 
		 ofloat *avgAz, ofloat *avgEl, ofloat *avgPA, 
		 ObitErr* err);

/* Grid data into cells */
void  gridData (ObitInfoList* myInput, olong nchan, olong nIF, olong npoln,
		olong selem, olong nelem, gboolean doRMS, gboolean doPhase,
		ofloat *SumIr, ofloat *SumIi, ofloat *SumII, 
		ofloat *SumQr, ofloat *SumQi, ofloat *SumQQ, 
		ofloat *SumUr, ofloat *SumUi, ofloat *SumUU, 
		ofloat *SumVr, ofloat *SumVi, ofloat *SumVV, 
		ofloat *SumAzCell, ofloat *SumElCell, ObitFArray **grids);

/* Lagrangian interpolation coefficients */
void lagrange(ofloat x, ofloat y, olong n, olong hwid, olong minmax[],
	      ofloat *xlist, ofloat *ylist, ofloat *coef);

/* Program globals */
gchar *pgmName = "MapBeam";      /* Program name */
gchar *infile  = "MapBeam.in" ;  /* File with program inputs */
gchar *outfile = "MapBeam.out";  /* File to contain program outputs */
olong  pgmNumber;                /* Program number (like POPS no.) */
olong  AIPSuser;                 /* AIPS user number number (like POPS no.) */
olong  nAIPS=0;                  /* Number of AIPS directories */
gchar **AIPSdirs=NULL;           /* List of AIPS data directories */
olong  nFITS=0;                  /* Number of FITS directories */
gchar **FITSdirs=NULL;           /* List of FITS data directories */
ObitInfoList *myInput  = NULL;   /* Input parameter list */
ObitInfoList *myOutput = NULL;   /* Output parameter list */
ofloat avgAz=0.0, avgEl=0.0, avgPA=0.0; /* Average observing Az, El, par Ang (deg) */
odouble RAMean=0.0, DecMean=0.0; /* Mean position of current source */

/*---------------Private structures----------------*/
/* Threaded function argument */
typedef struct {
  ObitThread *thread;  /* ObitThread to use */
  olong loy;           /* first (0-rel) y*/
  olong hiy;           /* laast (0-rel) y*/
  olong nx;            /* Number of columns */
  olong ny;            /* Number of rows */
  olong hwid;          /* Half width of interpolation */
  olong nchan;         /* Number of channels in output */
  olong nIF;	       /* Number of IFs in output  */
  olong npoln;	       /* Number of polarizations in output  */
  olong selem;         /* Size (floats) of list element */
  olong nelem; 	       /* Number of list elements  */
  gboolean doRMS;      /* If TRUE, image is RMS */
  gboolean doPhase;    /* If TRUE, image is Phase, else Amplitude */
  ofloat xcen;         /* X center cell */
  ofloat ycen;         /* Y center cell */
  ofloat *SumIr;       /* Real Stokes I accumulation list */
  ofloat *SumIi;       /* Imag Stokes I accumulation list  */
  ofloat *SumII;       /* Stokes I*I accumulation list  */
  ofloat *SumQr;       /* Real Stokes Q accumulation list */
  ofloat *SumQi;       /* Imag Stokes Q accumulation list */
  ofloat *SumQQ;       /* Stokes Q*Q accumulation list */
  ofloat *SumUr;       /* Real Stokes U accumulation list */
  ofloat *SumUi;       /* Imag Stokes U accumulation list */
  ofloat *SumUU;       /* Stokes U*U accumulation list  */
  ofloat *SumVr;       /* Real Stokes V accumulation list */
  ofloat *SumVi;       /* Imag Stokes V accumulation list */
  ofloat *SumVV;       /* Stokes V*V accumulation list */
  ofloat *SumAzCell;   /* Azimuth offset accumulation list */
  ofloat *SumElCell;   /* Elevation offset  accumulation list */
  ofloat *coef;        /* Work space */
  ObitFArray **grids;  /* Array of output ObitFArrays
			  fastest to slowest, channel, IF, Stokes  */
  olong      ithread;  /* Thread number  */
} MBFuncArg;
		     
/** Private: Make Threaded args */
static olong MakeMBFuncArgs (ObitThread *thread, 
			     olong nchan, olong nIF, olong npoln,
			     olong selem, olong nelem, gboolean doRMS, gboolean doPhase,
			     olong nx, olong ny, olong hwid, ofloat xcen, ofloat ycen,
			     ofloat *SumIr, ofloat *SumIi, ofloat *SumII, 
			     ofloat *SumQr, ofloat *SumQi, ofloat *SumQQ, 
			     ofloat *SumUr, ofloat *SumUi, ofloat *SumUU, 
			     ofloat *SumVr, ofloat *SumVi, ofloat *SumVV, 
			     ofloat *SumAzCell, ofloat *SumElCell, ObitFArray **grids,
			     MBFuncArg ***ThreadArgs);

/** Private: Delete Threaded args */
static void KillMBFuncArgs (olong nargs, MBFuncArg **ThreadArgs);

/** Private: Threaded Gridding */
static gpointer ThreadMBGrid (gpointer arg);

int main ( int argc, char **argv )
/*----------------------------------------------------------------------- */
/*   Obit task to make beam poln images from quasi holography data        */
/*----------------------------------------------------------------------- */
{
  oint         ierr      = 0;
  ObitSystem   *mySystem = NULL;
  ObitUV       *inData   = NULL;
  ObitErr      *err      = NULL;
 
   /* Startup - parse command line */
  err = newObitErr();
  myInput = MapBeamIn (argc, argv, err);
  if (err->error) {ierr = 1;  ObitErrLog(err);  goto exit;}

  /* Initialize logging */
  ObitErrInit (err, (gpointer)myInput);

  ObitErrLog(err); /* show any error messages on err */
  if (ierr!=0) return 1;

  /* Initialize Obit */
  mySystem = ObitSystemStartup (pgmName, pgmNumber, AIPSuser, nAIPS, AIPSdirs, 
				nFITS, FITSdirs, (oint)TRUE, (oint)FALSE, err);
  if (err->error) ierr = 1; ObitErrLog(err); if (ierr!=0) goto exit;

  /* Digest input */
  digestInputs(myInput, err);
  if (err->error) ierr = 1; ObitErrLog(err); if (ierr!=0) goto exit;

  /* Get input uvdata */
  inData = getInputData (myInput, err);
  if (err->error) ierr = 1; ObitErrLog(err); if (ierr!=0) goto exit;

  /* Process */
  doSources (myInput, inData, err);
  if (err->error) ierr = 1; ObitErrLog(err); if (ierr!=0) goto exit;

  /* cleanup */
  myInput   = ObitInfoListUnref(myInput);    /* delete input list */
  inData    = ObitUnref(inData);
  
  /* Shutdown Obit */
 exit: 
  ObitReturnDumpRetCode (ierr, outfile, myOutput, err);  /* Final output */
  myOutput = ObitInfoListUnref(myOutput);                /* delete output list */
  mySystem = ObitSystemShutdown (mySystem);
  
  return ierr;
} /* end of main */

ObitInfoList* MapBeamIn (int argc, char **argv, ObitErr *err)
/*----------------------------------------------------------------------- */
/*  Parse control info from command line                                  */
/*   Input:                                                               */
/*      argc   Number of arguments from command line                      */
/*      argv   Array of strings from command line                         */
/*   Output:                                                              */
/*      err    Obit Error stack                                           */
/*   return  ObitInfoList with defaults/parsed values                     */
/*----------------------------------------------------------------------- */
{
  olong ax;
  gchar *arg;
  gboolean init=FALSE;
  ObitInfoType type;
  gint32 dim[MAXINFOELEMDIM] = {1,1,1,1,1};
  gchar *strTemp;
  oint    itemp, i, j, k;
  ObitInfoList* list=NULL;
  gchar *routine = "MapBeamIn";

  /* error checks */
  if (err->error) return list;

  /* Make default inputs InfoList */
  list = defaultInputs(err);
  myOutput = defaultOutputs(err);

  /* command line arguments */
  /* fprintf (stderr,"DEBUG arg %d %s\n",argc,argv[0]); DEBUG */
  if (argc<=1) Usage(); /* must have arguments */
  /* parse command line */
  for (ax=1; ax<argc; ax++) {

     /*fprintf (stderr,"DEBUG next arg %s %s\n",argv[ax],argv[ax+1]); DEBUG */
    arg = argv[ax];
    if (strcmp(arg, "-input") == 0){ /* input parameters */
      infile = argv[++ax];
      /* parse input file */
      ObitParserParse (infile, list, err);
      init = TRUE;

    } else if (strcmp(arg, "-output") == 0){ /* output results */
      outfile = argv[++ax];

    } else if (strcmp(arg, "-pgmNumber") == 0) { /*Program number */
      dim[0] = 1;
      itemp = strtol(argv[++ax], NULL, 0);
      ObitInfoListPut (list, "pgmNumber", OBIT_oint, dim, &itemp, err);
      
    } else if (strcmp(arg, "-AIPSuser") == 0) { /* AIPS user number */
      dim[0] = 1;
      itemp = strtol(argv[++ax], NULL, 0);
      ObitInfoListPut (list, "AIPSuser", OBIT_oint, dim, &itemp, err);
      
    } else if (strcmp(arg, "-inSeq") == 0) { /* AIPS sequence number */
      dim[0] = 1;
      itemp = strtol(argv[++ax], NULL, 0);
      ObitInfoListPut (list, "inSeq", OBIT_oint, dim, &itemp, err);
      
    } else if (strcmp(arg, "-inDisk") == 0) { /* input disk number */
      dim[0] = 1;
      itemp = strtol(argv[++ax], NULL, 0);
      ObitInfoListPut (list, "inDisk", OBIT_oint, dim, &itemp, err);
      
     } else if (strcmp(arg, "-DataType") == 0) { /* Image type AIPS or FITS */
      strTemp = argv[++ax];
      dim[0] = strlen (strTemp);
      ObitInfoListAlwaysPut (list, "DataType", OBIT_string, dim, strTemp);
      
     } else if (strcmp(arg, "-outDType") == 0) { /* Image type AIPS or FITS */
      strTemp = argv[++ax];
      dim[0] = strlen (strTemp);
      ObitInfoListAlwaysPut (list, "DataType", OBIT_string, dim, strTemp);
      
    } else if (strcmp(arg, "-BChan") == 0) { /* BChan */
      dim[0] = 1;
      itemp = strtol(argv[++ax], NULL, 0);
      ObitInfoListPut (list, "BChan", OBIT_oint, dim, &itemp, err);
      
    } else if (strcmp(arg, "-EChan") == 0) { /* EChan */
      dim[0] = 1;
      itemp = strtol(argv[++ax], NULL, 0);
      ObitInfoListPut (list, "EChan", OBIT_oint, dim, &itemp, err);
      
    } else if (strcmp(arg, "-BIF") == 0) { /* BIF */
      dim[0] = 1;
      itemp = strtol(argv[++ax], NULL, 0);
      ObitInfoListPut (list, "BIF", OBIT_oint, dim, &itemp, err);
      
    } else if (strcmp(arg, "-EIF") == 0) { /* EIF */
      dim[0] = 1;
      itemp = strtol(argv[++ax], NULL, 0);
      ObitInfoListPut (list, "EIF", OBIT_oint, dim, &itemp, err);
      
     } else if (strcmp(arg, "-inName") == 0) { /* AIPS inName*/
      strTemp = argv[++ax];
      dim[0] = strlen (strTemp);
      ObitInfoListAlwaysPut (list, "inName", OBIT_string, dim, strTemp);
      
     } else if (strcmp(arg, "-inClass") == 0) { /* AIPS inClass*/
      strTemp = argv[++ax];
      dim[0] = strlen (strTemp);
      ObitInfoListAlwaysPut (list, "inClass", OBIT_string, dim, strTemp);
      
     } else if (strcmp(arg, "-inFile") == 0) { /*inFile */
      strTemp = argv[++ax];
      dim[0] = strlen (strTemp);
      ObitInfoListAlwaysPut (list, "inFile", OBIT_string, dim, strTemp);
      
    } else if (strcmp(arg, "-outSeq") == 0) { /* AIPS image sequence number */
      dim[0] = 1;
      itemp = strtol(argv[++ax], NULL, 0);
      ObitInfoListPut (list, "outSeq", OBIT_oint, dim, &itemp, err);
      
    } else if (strcmp(arg, "-outDisk") == 0) { /* output image disk number */
      dim[0] = 1;
      itemp = strtol(argv[++ax], NULL, 0);
      ObitInfoListPut (list, "outDisk", OBIT_oint, dim, &itemp, err);
      
     } else if (strcmp(arg, "-outName") == 0) { /* AIPS image outName */
      strTemp = argv[++ax];
      dim[0] = strlen (strTemp);
      ObitInfoListAlwaysPut (list, "outName", OBIT_string, dim, strTemp);
      
     } else if (strcmp(arg, "-outFile") == 0) { /*outFile */
      strTemp = argv[++ax];
      dim[0] = strlen (strTemp);
      ObitInfoListAlwaysPut (list, "outFile", OBIT_string, dim, strTemp);

     } else if (strcmp(arg, "-AIPSdir") == 0) { /* Single AIPS Directory */
      strTemp = argv[++ax];
      dim[0] = strlen (strTemp);
      ObitInfoListAlwaysPut (list, "AIPSdirs", OBIT_string, dim, strTemp);
      /* Only one AIPS Directory */
      dim[0] = 1;dim[1] = 1;
      itemp = 1; /* number of AIPS directories (1) */
      ObitInfoListPut (list, "nAIPS", OBIT_oint, dim, &itemp, err);

     } else if (strcmp(arg, "-FITSdir") == 0) { /* Single FITS Directory */
      strTemp = argv[++ax];
      dim[0] = strlen (strTemp);
      ObitInfoListAlwaysPut (list, "FITSdirs", OBIT_string, dim, strTemp);
      /* Only one FITS Directory */
      dim[0] = 1;dim[1] = 1;
      itemp = 1; /* number of FITS directories (1) */
      ObitInfoListPut (list, "nFITS", OBIT_oint, dim, &itemp, err);

   } else { /* unknown argument */
      Usage();
    }
    if (err->error) Obit_traceback_val (err, routine, "GetInput", list);
  } /* end parsing input arguments */
  
  /* Read defaults if no file specified */
  if (!init) ObitParserParse (infile, list, err);

  /* Extract basic information to program globals */
  ObitInfoListGet(list, "pgmNumber", &type, dim, &pgmNumber, err);
  ObitInfoListGet(list, "AIPSuser",  &type, dim, &AIPSuser,  err);
  ObitInfoListGet(list, "nAIPS",     &type, dim, &nAIPS,     err);
  ObitInfoListGet(list, "nFITS",     &type, dim, &nFITS,     err);
  if (err->error) Obit_traceback_val (err, routine, "GetInput", list);

  /* Directories more complicated */
  ObitInfoListGetP(list, "AIPSdirs",  &type, dim, (gpointer)&strTemp);
  if (strTemp) {  /* Found? */
    AIPSdirs = g_malloc0(dim[1]*sizeof(gchar*));
    for (i=0; i<dim[1]; i++) {
      AIPSdirs[i] =  g_malloc0(dim[0]*sizeof(gchar));
      k = 0;
      for (j=0; j<dim[0]; j++) { /* Don't copy blanks */
	if (strTemp[j]!=' ') {AIPSdirs[i][k] = strTemp[j]; k++;}
      }
      AIPSdirs[i][k] = 0;
      strTemp += dim[0];
    }
  }

  ObitInfoListGetP(list, "FITSdirs",  &type, dim, (gpointer)&strTemp);
  if (strTemp)   {  /* Found? */
    FITSdirs = g_malloc0(dim[1]*sizeof(gchar*));
    for (i=0; i<dim[1]; i++) {
      FITSdirs[i] =  g_malloc0(dim[0]*sizeof(gchar));
      k = 0;
      for (j=0; j<dim[0]; j++) { /* Don't copy blanks */
	if (strTemp[j]!=' ') {FITSdirs[i][k] = strTemp[j]; k++;}
      }
      FITSdirs[i][k] = 0;
      strTemp += dim[0];
    }
  }

  /* Initialize output */
  ObitReturnDumpRetCode (-999, outfile, myOutput, err);
  if (err->error) Obit_traceback_val (err, routine, "GetInput", list);

  return list;
} /* end MapBeamIn */

void Usage(void)
/*----------------------------------------------------------------------- */
/*   Tells about usage of program                                         */
/*----------------------------------------------------------------------- */
{
    fprintf(stderr, "Usage: MapBeam -input file -output ofile [args]\n");
    fprintf(stderr, "MapBeam Obit task to make beam poln images\n");
    fprintf(stderr, "Arguments:\n");
    fprintf(stderr, "  -input input parameter file, def MapBeam.in\n");
    fprintf(stderr, "  -output output result file, def MapBeam.out\n");
    fprintf(stderr, "  -pgmNumber Program (POPS) number, def 1 \n");
    fprintf(stderr, "  -AIPSuser AIPS user number, def 2 \n");
    fprintf(stderr, "  -DataType AIPS or FITS type for input \n");
    fprintf(stderr, "  -outDType AIPS or FITS type for output\n");
    fprintf(stderr, "  -inFile input FITS UV file\n");
    fprintf(stderr, "  -inName input AIPS file name\n");
    fprintf(stderr, "  -inClass input AIPS file class\n");
    fprintf(stderr, "  -inSeq input AIPS file sequence\n");
    fprintf(stderr, "  -inDisk input image (AIPS or FITS) disk number (1-rel) \n");
    fprintf(stderr, "  -BChan first channel to image\n");
    fprintf(stderr, "  -EChan highest channel to image\n");
    fprintf(stderr, "  -BIF first IF to image\n");
    fprintf(stderr, "  -EIF highest IF to image\n");
    fprintf(stderr, "  -outFile output image (FITS Image file\n");  
    fprintf(stderr, "  -outName output image (AIPS file name\n");
    fprintf(stderr, "  -outSeq output image (AIPS file sequence\n");
    fprintf(stderr, "  -outDisk output image ((AIPS or FITS) disk number (1-rel) \n");
    fprintf(stderr, "  -AIPSdir single AIPS data directory\n");
    fprintf(stderr, "  -FITSdir single FITS data directory\n");
    /*/exit(1);  bail out */
  }/* end Usage */

/*----------------------------------------------------------------------- */
/*  Create default input ObitInfoList                                     */
/*  Note: Other parameters may be passed through the input text file      */
/*   Return                                                               */
/*       ObitInfoList  with default values                                */
/*  Values:                                                               */
/*     pgmNumber Int        Program number (like POPS number) def 1       */
/*     nFITS     Int        Number of FITS directories [def. 1]           */
/*     FITSdirs  Str [?,?]  FITS directories [def {"./"}]                 */
/*     AIPSuser  Int        AIPS user number [def 2}]                     */
/*     nAIPS     Int        Number of AIPS directories [def. 1]           */
/*     AIPSdirs  Str [?,?]  AIPS directories [def {"AIPSdata/"}]          */
/*     DataType  Str [4]    "AIPS" or "FITS" [def {"FITS"}]               */
/*     inFile    Str [?]    input FITS uv file name [no def]              */
/*     inName    Str [12]   input AIPS uv name  [no def]                  */
/*     inClass   Str [6]    input AIPS uv class  [no def]                 */
/*     inSeq     Int        input AIPS uv sequence no  [no def]           */
/*     outDisk   Int        output AIPS or FITS image disk no  [def 1]    */
/*     outFile   Str [?]    output FITS image file name [def "Image.fits" */
/*     outName   Str [12]   output AIPS image name  [no def]              */
/*     outSeq    Int        output AIPS image sequence no  [new]          */
/*     Sources   Str (16,1) Sources selected, blank = all                 */
/*     timeRange Flt (2)    Timerange in days , def=all                   */
/*     doCalSelect Boo (1)  Apply calibration/selection?  def=True        */
/*     doCalib   Int (1)    >0 => apply calibration, 2=> cal. wt, def=-1  */
/*     gainUse   Int (1)    Gain table (CL/SN) table to apply, 0=> highest*/
/*     doBand    Int (1)    If >0.5 apply bandpass cal.                   */
/*     flagVer   Int (1)    Flagging table version, def=0                 */
/*     BPVer     Int (1)    Bandpass table version, 0=highest, def=0      */
/*     doPol     Boo (1)    Apply polarization calibration?, def=False    */
/*----------------------------------------------------------------------- */
ObitInfoList* defaultInputs(ObitErr *err)
{
  gint32 dim[MAXINFOELEMDIM] = {1,1,1,1,1};
  gchar *strTemp;
  ofloat farray[3];
  gboolean btemp;
  olong  itemp, iarr[3];
  ObitInfoList *out = newObitInfoList();
  gchar *routine = "defaultInputs";

  /* error checks */
  if (err->error) return out;

  /* add parser items */
  /* Program number */
  dim[0] = 1; dim[1] = 1;
  itemp = 1;
  ObitInfoListPut (out, "pgmNumber", OBIT_oint, dim, &itemp, err);
  if (err->error) Obit_traceback_val (err, routine, "DefInput", out);

  /* Default FITS directories - same directory */
  dim[0] = 1; dim[1] = 1;
  itemp = 0; /* number of FITS directories */
  ObitInfoListPut (out, "nFITS", OBIT_oint, dim, &itemp, err);

  /* AIPS user number */
  dim[0] = 1; dim[1] = 1;
  itemp = 2;
  ObitInfoListPut (out, "AIPSuser", OBIT_oint, dim, &itemp, err);
  if (err->error) Obit_traceback_val (err, routine, "DefInput", out);

  /* Default AIPS directories */
  dim[0] = 1;dim[1] = 1;
  itemp = 0; /* number of AIPS directories */
  ObitInfoListPut (out, "nAIPS", OBIT_oint, dim, &itemp, err);

  /* Default type "FITS" */
  strTemp = "FITS";
  dim[0] = strlen (strTemp); dim[1] = 1;
  ObitInfoListPut (out, "DataType", OBIT_string, dim, strTemp, err);
  ObitInfoListPut (out, "outDType", OBIT_string, dim, strTemp, err);
  if (err->error) Obit_traceback_val (err, routine, "DefInput", out);

  /* input FITS file name */
  strTemp = "MapBeam.uvtab";
  dim[0] = strlen (strTemp); dim[1] = 1;
  ObitInfoListPut (out, "inFile", OBIT_string, dim, strTemp, err);
  if (err->error) Obit_traceback_val (err, routine, "DefInput", out);

  /* input AIPS file name */
  strTemp = "MapBeamName";
  dim[0] = strlen (strTemp); dim[1] = 1;
  ObitInfoListPut (out, "inName", OBIT_string, dim, strTemp, err);
  if (err->error) Obit_traceback_val (err, routine, "DefInput", out);

  /* input AIPS file class */
  strTemp = "Class ";
  dim[0] = strlen (strTemp); dim[1] = 1;
  ObitInfoListPut (out, "inClass", OBIT_string, dim, strTemp, err);
  if (err->error) Obit_traceback_val (err, routine, "DefInput", out);

  /* AIPS sequence */
  dim[0] = 1;dim[1] = 1;
  itemp = 1; 
  ObitInfoListPut (out, "inSeq", OBIT_oint, dim, &itemp, err);
  if (err->error) Obit_traceback_val (err, routine, "DefInput", out);

  /* AIPS or FITS disk number */
  dim[0] = 1;dim[1] = 1;
  itemp = 1; 
  ObitInfoListPut (out, "inDisk", OBIT_oint, dim, &itemp, err);
  if (err->error) Obit_traceback_val (err, routine, "DefInput", out);

  /* output FITS Image file name */
  strTemp = "MapBeamOut.fits";
  dim[0] = strlen (strTemp); dim[1] = 1;
  ObitInfoListPut (out, "outFile", OBIT_string, dim, strTemp, err);
  if (err->error) Obit_traceback_val (err, routine, "DefInput", out);

  /* Output AIPS Image file name */
  strTemp = "MapBeamOut";
  dim[0] = strlen (strTemp); dim[1] = 1;
  ObitInfoListPut (out, "outName", OBIT_string, dim, strTemp, err);
  if (err->error) Obit_traceback_val (err, routine, "DefInput", out);

  /* Output AIPS Image sequence */
  dim[0] = 1;dim[1] = 1;
  itemp = 0; 
  ObitInfoListPut (out, "outSeq", OBIT_oint, dim, &itemp, err);
  if (err->error) Obit_traceback_val (err, routine, "DefInput", out);

  /* output AIPS or FITS Image disk number */
  dim[0] = 1;dim[1] = 1;
  itemp = 1; 
  ObitInfoListPut (out, "outDisk", OBIT_oint, dim, &itemp, err);
  if (err->error) Obit_traceback_val (err, routine, "DefInput", out);

  /* Sources selected, blank = all */
  strTemp = "                ";
  dim[0] = strlen (strTemp); dim[1] = 1;
  ObitInfoListPut (out, "Sources", OBIT_string, dim, strTemp, err);
  if (err->error) Obit_traceback_val (err, routine, "DefInput", out);
    
  /* Stokes parameter to image */
  strTemp = "I   ";
  dim[0] = strlen (strTemp); dim[1] = 1;
  ObitInfoListPut (out, "Stokes", OBIT_string, dim, strTemp, err);
  if (err->error) Obit_traceback_val (err, routine, "DefInput", out);

  /* Timerange in days */
  dim[0] = 2;dim[1] = 1;
  farray[0] = -1.0e20; farray[1] = 1.0e20;
  ObitInfoListPut (out, "timeRange", OBIT_float, dim, farray, err);
  if (err->error) Obit_traceback_val (err, routine, "DefInput", out);

  /*  Apply calibration/selection?, Always True */
  dim[0] = 1; dim[1] = 1;
  btemp = TRUE;
  ObitInfoListPut (out, "doCalSelect", OBIT_bool, dim, &btemp, err);
  if (err->error) Obit_traceback_val (err, routine, "DefInput", out);

  /*  >0 => apply gain calibration, 2=> cal. wt, def=no cal. */
  dim[0] = 1;dim[1] = 1;
  itemp = -1; 
  ObitInfoListPut (out, "doCalib", OBIT_oint, dim, &itemp, err);
  if (err->error) Obit_traceback_val (err, routine, "DefInput", out);

  /*  Gain table (CL/SN) table to apply, 0=> highest, def=0 */
  dim[0] = 1;dim[1] = 1;
  itemp = 0; 
  ObitInfoListPut (out, "gainUse", OBIT_oint, dim, &itemp, err);
  if (err->error) Obit_traceback_val (err, routine, "DefInput", out);

  /*  If >0.5 apply bandpass cal, def = no BP cal. */
  dim[0] = 1;dim[1] = 1;
  itemp = -1; 
  ObitInfoListPut (out, "doBand", OBIT_oint, dim, &itemp, err);
  if (err->error) Obit_traceback_val (err, routine, "DefInput", out);

  /*  Bandpass table version, 0=highest, def=0 */
  dim[0] = 1;dim[1] = 1;
  itemp = 0; 
  ObitInfoListPut (out, "BPVer", OBIT_oint, dim, &itemp, err);
  if (err->error) Obit_traceback_val (err, routine, "DefInput", out);

  /* Flagging table version, def=0 */
  dim[0] = 1;dim[1] = 1;
  itemp = 0; 
  ObitInfoListPut (out, "flagVer", OBIT_oint, dim, &itemp, err);
  if (err->error) Obit_traceback_val (err, routine, "DefInput", out);
  
  /* Apply polarization calibration?, def= False */
  dim[0] = 1; dim[1] = 1;
  btemp = FALSE;
  ObitInfoListPut (out, "doPol", OBIT_bool, dim, &btemp, err);
  if (err->error) Obit_traceback_val (err, routine, "DefInput", out);
  
  /* Reference antennas */
  dim[0] = 3; dim[1] = 1;
  iarr[0] = iarr[1] = iarr[2] = 0;
  ObitInfoListAlwaysPut (out, "RefAnts", OBIT_oint, dim, iarr);

  return out;
} /* end defaultInputs */

/*----------------------------------------------------------------------- */
/*  Create default output ObitInfoList                                    */
/*   Return                                                               */
/*       ObitInfoList  with default values                                */
/*  Values:                                                               */
/*----------------------------------------------------------------------- */
ObitInfoList* defaultOutputs(ObitErr *err)
{
  ObitInfoList *out = newObitInfoList();
  /*  gint32 dim[MAXINFOELEMDIM] = {1,1,1,1,1};
      ofloat ftemp;
      gchar *routine = "defaultOutputs";"*/

  /* error checks */
  if (err->error) return out;

  /* add parser items - nothing */
  return out;
} /* end defaultOutputs */

/*----------------------------------------------------------------------- */
/*  Digest inputs                                                         */
/*   Input:                                                               */
/*      myInput   Input parameters on InfoList                            */
/*   Output:                                                              */
/*      err    Obit Error stack                                           */
/*----------------------------------------------------------------------- */
void digestInputs(ObitInfoList *myInput, ObitErr *err)
{
  /* ObitInfoType type;
     gint32       dim[MAXINFOELEMDIM] = {1,1,1,1,1}; */
  gchar *routine = "digestInputs";

  /* error checks */
  if (err->error) return;
  g_assert (ObitInfoListIsA(myInput));

  /* noScrat - no scratch files for AIPS disks */
  ObitAIPSSetnoScrat(myInput, err);
  if (err->error) Obit_traceback_msg (err, routine, "task Input");

  /* Initialize Threading */
  ObitThreadInit (myInput);

} /* end digestInputs */

/*----------------------------------------------------------------------- */
/*  Get input data                                                        */
/*   Input:                                                               */
/*      myInput   Input parameters on InfoList                            */
/*   Output:                                                              */
/*      err    Obit Error stack                                           */
/*   Return                                                               */
/*       ObitUV with input data                                           */
/*----------------------------------------------------------------------- */
ObitUV* getInputData (ObitInfoList *myInput, ObitErr *err)
{
  ObitUV       *inData = NULL;
  ObitInfoType type;
  olong         Aseq, disk, cno, nvis;
  gchar        *Type, *strTemp, inFile[129];
  oint         doCalib;
  gchar        Aname[13], Aclass[7], *Atype = "UV";
  gint32       dim[MAXINFOELEMDIM] = {1,1,1,1,1};
  gboolean     doCalSelect;
  gchar        Stokes[5];
  gchar *routine = "getInputData";

  /* error checks */
  if (err->error) return inData;
  g_assert (ObitInfoListIsA(myInput));

  /* Create basic input UV data Object */
  inData = newObitUV("input UV data");
  
  /* File type - could be either AIPS or FITS */
  ObitInfoListGetP (myInput, "DataType", &type, dim, (gpointer)&Type);
  if (!strncmp (Type, "AIPS", 4)) { /* AIPS input */
    /* input AIPS disk */
    ObitInfoListGet(myInput, "inDisk", &type, dim, &disk, err);
    /* input AIPS name */
    if (ObitInfoListGetP(myInput, "inName", &type, dim, (gpointer)&strTemp)) {
      strncpy (Aname, strTemp, 13);
    } else { /* Didn't find */
      strncpy (Aname, "No Name ", 13);
    } 
    Aname[12] = 0;
    /* input AIPS class */
    if  (ObitInfoListGetP(myInput, "inClass", &type, dim, (gpointer)&strTemp)) {
      strncpy (Aclass, strTemp, 7);
    } else { /* Didn't find */
      strncpy (Aclass, "NoClas", 7);
    }
    Aclass[6] = 0;
    /* input AIPS sequence */
    ObitInfoListGet(myInput, "inSeq", &type, dim, &Aseq, err);

    /* if ASeq==0 want highest existing sequence */
    if (Aseq<=0) {
      Aseq = ObitAIPSDirHiSeq(disk, AIPSuser, Aname, Aclass, Atype, TRUE, err);
      if (err->error) Obit_traceback_val (err, routine, "myInput", inData);
      /* Save on myInput*/
      dim[0] = dim[1] = 1;
      ObitInfoListAlwaysPut(myInput, "inSeq", OBIT_oint, dim, &Aseq);
    }

    /* Find catalog number */
    cno = ObitAIPSDirFindCNO(disk, AIPSuser, Aname, Aclass, Atype, Aseq, err);
    if (err->error) Obit_traceback_val (err, routine, "myInput", inData);
    
    /* define object  */
    nvis = 1000;
    ObitUVSetAIPS (inData, nvis, disk, cno, AIPSuser, err);
    if (err->error) Obit_traceback_val (err, routine, "myInput", inData);
    
  } else if (!strncmp (Type, "FITS", 4)) {  /* FITS input */
    /* input FITS file name */
    if (ObitInfoListGetP(myInput, "inFile", &type, dim, (gpointer)&strTemp)) {
      strncpy (inFile, strTemp, 128);
    } else { 
      strncpy (inFile, "No_Filename_Given", 128);
    }
    
    /* input FITS disk */
    ObitInfoListGet(myInput, "inDisk", &type, dim, &disk, err);

    /* define object */
    nvis = 1;
    ObitUVSetFITS (inData, nvis, disk, inFile,  err); 
    if (err->error) Obit_traceback_val (err, routine, "myInput", inData);
    
  } else { /* Unknown type - barf and bail */
    Obit_log_error(err, OBIT_Error, "%s: Unknown Data type %s", 
                   pgmName, Type);
    return inData;
  }

  /* Make sure doCalSelect set properly */
  doCalSelect = TRUE;
  ObitInfoListGetTest(myInput, "doCalSelect",  &type, dim, &doCalSelect);
  doCalib = -1;
  ObitInfoListGetTest(myInput, "doCalib",  &type, dim, &doCalib);
  doCalSelect = doCalSelect || (doCalib>0);
  ObitInfoListAlwaysPut (myInput, "doCalSelect", OBIT_bool, dim, &doCalSelect);
 

  /* Convert to IQUV */
  strcpy (Stokes, "IQUV");
  dim[0] = strlen(Stokes);
  ObitInfoListAlwaysPut (inData->info, "Stokes", OBIT_string, dim, Stokes);

  /* Ensure inData fully instantiated and OK */
  ObitUVFullInstantiate (inData, TRUE, err);
  if (err->error) Obit_traceback_val (err, routine, "myInput", inData);

  /* Set number of vis per IO */
  nvis = 1000;  /* How many vis per I/O? */
  nvis =  ObitUVDescSetNVis (inData->myDesc, myInput, nvis);
  dim[0] = dim[1] = dim[2] = dim[3] = 1;
  ObitInfoListAlwaysPut (inData->info, "nVisPIO", OBIT_long, dim,  &nvis);

  return inData;
} /* end getInputData */


/*----------------------------------------------------------------------- */
/*  Create output image                                                   */
/*  One output image cube for requested Antenna/Stokes                    */
/*  output axes, Azimuth, Elevation, Channel, IF                          */
/*   Input:                                                               */
/*      Source    Source name                                             */
/*      iStoke    Stokes number (0-rel), I, Q, U, V                       */
/*      ant       Antenna number of image, 0=>all averaged                */
/*      doRMS     If TRUE, image is RMS                                   */
/*      doPhase   If TRUE, image is phase                                 */
/*      myInput   Input parameters on InfoList                            */
/*      inData    ObitUV defining data                                    */
/*   Output:                                                              */
/*      err       Obit Error stack                                        */
/*   Return  Output image depending on Stokes request                     */
/*----------------------------------------------------------------------- */
ObitImage* setOutput (gchar *Source, olong iStoke, olong ant, 
		      gboolean doRMS, gboolean doPhase, 
		      ObitInfoList *myInput, ObitUV* inData, 
		      ObitErr *err)
{
  ObitImage *outImage=NULL;
  ObitInfoType type;
  ObitIOType IOType;
  olong     i, n, Aseq, disk, cno, axNum, axFNum, axIFNum, nx=11, ny=11;
  olong     jStoke;
  gchar     *Type, *strTemp, outFile[129], *outName, *outF, stemp[32];
  gchar     Aname[13], Aclass[7], *Atype = "MA";
  gchar     *today=NULL;
  gint32    dim[MAXINFOELEMDIM] = {1,1,1,1,1};
  olong     blc[IM_MAXDIM] = {1,1,1,1,1,1,1};
  olong     trc[IM_MAXDIM] = {0,0,0,0,0,0,0};
  gboolean  exist;
  gboolean trueStokes=FALSE;
  gchar     tname[129], *chStokes, *chTStokes="IQUV", *chCcor="RLPQ", *chLcor="XYPQ";
  odouble   *StokCrval, TStokCrval[4] = {1.0,2.0,3.0,4.0};
  odouble   CFCrval[4] = {-1.0, -2.0,-3.0,-4.0}, LFCrval[4] = {-5.0, -6.0,-7.0,-8.0};
  gfloat    xCells, yCells;
  gchar     *FITS = "FITS";
  ObitImageDesc *outDesc;
  gchar     *routine = "setOutput";

  /* error checks */
  if (err->error) return outImage;
  g_assert (ObitInfoListIsA(myInput));
  g_assert (ObitUVIsA(inData));

  /* Stokes or correlator values */
  trueStokes = inData->myDesc->crval[inData->myDesc->jlocs]>0.0;
  if (trueStokes)                                             chStokes = chTStokes;
  else if (inData->myDesc->crval[inData->myDesc->jlocs]<-3.0) chStokes = chLcor;
  else                                                        chStokes = chCcor;
  if (trueStokes)                                             StokCrval = TStokCrval;
  else if (inData->myDesc->crval[inData->myDesc->jlocs]<-3.0) StokCrval = LFCrval;
  else                                                        StokCrval = CFCrval;
  /* Which poln? */
  jStoke = iStoke;
  if (!trueStokes && inData->myDesc->crval[inData->myDesc->jlocs]==-2) jStoke = 1;
  if (!trueStokes && inData->myDesc->crval[inData->myDesc->jlocs]==-6) jStoke = 1;

  /* Notes:
     1) AIPS Name = Source +outName
            Class = "Santno"/"Sall" S=Stokes, specific or average ant.
        FITS Name = Source+"Santno"/"Sall"+outFile S=Stokes, specific or average an
        for RMS add "RM" to end of class or equivalent
        for Phase add "Ph" to end of class or equivalent
  */

  /* Create basic output Image Object */
  g_snprintf (tname, 100, "output Image %cPol",chStokes[jStoke]);
  outImage = newObitImage(tname);
    
  /* File type - could be either AIPS or FITS */
  ObitInfoListGetP (myInput, "outDType", &type, dim, (gpointer)&Type);
  if ((Type==NULL) || (!strncmp(Type,"    ",4)))
    ObitInfoListGetP (myInput, "DataType", &type, dim, (gpointer)&Type);
  if ((Type==NULL) || (!strncmp(Type,"    ",4))) Type = FITS;
  if (!strncmp (Type, "AIPS", 4)) { /* AIPS output */
    /* Generate output name from Source, outName */
    ObitInfoListGetP (myInput, "outName", &type, dim, (gpointer)&outName);
    /* Something in source name? */
    if ((Source[0]==' ') || (Source[0]==0)) g_snprintf (tname, 100, "%s", outName);
    else g_snprintf (tname, 100, "%s%s", Source, outName);
    /* If no name use input name */
    if ((tname[0]==' ') || (tname[0]==0)) {
      ObitInfoListGetP (myInput, "inName", &type, dim, (gpointer)&strTemp);
      g_snprintf (tname, 100, "%s", strTemp);
    }
      
    IOType = OBIT_IO_AIPS;  /* Save file type */
    /* input AIPS disk */
    ObitInfoListGet(myInput, "outDisk", &type, dim, &disk, err);
    /* input AIPS sequence */
    ObitInfoListGet(myInput, "outSeq", &type, dim, &Aseq, err);
    for (i=0; i<12; i++) Aname[i] = ' '; Aname[i] = 0;
    strncpy (Aname, tname, 13); 
    Aname[12] = 0;

    /* output AIPS class */
    /* Stokes or RMS? */
    if (doRMS) {  /* RMS */
      /* One or all antennas? */
      if (ant>0) {   /* One */
	g_snprintf (tname, 100, "%c%3.3dRM", chStokes[jStoke],ant);
      } else {       /* all */
	g_snprintf (tname, 100, "%cAllRM", chStokes[jStoke]);
      }
    }  else if (doPhase) {  /* Phase */
      /* One or all antennas? */
      if (ant>0) {   /* One */
	g_snprintf (tname, 100, "%c%3.3dPH", chStokes[jStoke],ant);
      } else {       /* all */
	g_snprintf (tname, 100, "%cAllPH", chStokes[jStoke]);
      }
    } else {      /* Stokes */
      if (ant>0) {   /* One */
	g_snprintf (tname, 100, "%c%3.3d  ", chStokes[jStoke],ant);
      } else {       /* all */
	g_snprintf (tname, 100, "%cAll  ", chStokes[jStoke]);
      }
    } /* end of branch by type */
    strncpy (Aclass, tname, 7); 
    Aclass[6] = 0;

    /* if ASeq==0 create new, high+1 */
    if (Aseq<=0) {
      Aseq = ObitAIPSDirHiSeq(disk, AIPSuser, Aname, Aclass, Atype, FALSE, err);
      if (err->error) Obit_traceback_val (err, routine, "myInput", outImage);
      /* Save on myInput*/
      dim[0] = dim[1] = 1;
      ObitInfoListAlwaysPut(myInput, "outSeq", OBIT_oint, dim, &Aseq);
    } 

    /* Find catalog number */
    cno = ObitAIPSDirAlloc(disk, AIPSuser, Aname, Aclass, Atype, Aseq, &exist, err);
    if (err->error) Obit_traceback_val (err, routine, "myInput", outImage);
    
    /* Tell about it */
    Obit_log_error(err, OBIT_InfoErr, "Output AIPS image %s %s %d on disk %d cno %d",
		   Aname, Aclass, Aseq, disk, cno);

    /* define object */
    ObitImageSetAIPS (outImage, OBIT_IO_byPlane, disk, cno, AIPSuser, 
		      blc, trc, err);
    if (err->error) Obit_traceback_val (err, routine, "myInput", outImage);

  } else if (!strncmp (Type, "FITS", 4)) {  /* FITS output */
    /* Generate output name from Source, outName */
    ObitInfoListGetP (myInput, "outFile", &type, dim, (gpointer)&outF);
    n = MIN (128, dim[0]);
    for (i=0; i<n; i++) tname[i] = outF[i]; tname[i] = 0;
    /* If blank use ".fits" */
    if ((tname[0]==' ') || (tname[0]==0)) g_snprintf (tname, 128, ".fits");
    /* Something in source name? */
    if ((Source[0]==' ') || (Source[0]==0)) 
      g_snprintf (stemp, 30, "Beam");
    else g_snprintf (stemp, 30, "%s", Source);
    ObitTrimTrail(stemp);  /* remove trailing blanks */
	   
    IOType = OBIT_IO_FITS;  /* Save file type */

    /* Set output file name */
    /* Stokes or RMS? */
    if (doRMS) {  /* RMS */
      /* One or all antennas? */
      if (ant>0) {   /* One */
	g_snprintf (outFile, 128, "%s%c%3.3dRMS%s", stemp,chStokes[jStoke],ant,tname);
      } else {       /* all */
	g_snprintf (outFile, 128, "%s%cAllRMS%s", stemp,chStokes[jStoke],tname);
      }
     } else if (doPhase) {  /* Phase */
      /* One or all antennas? */
      if (ant>0) {   /* One */
	g_snprintf (outFile, 128, "%s%c%3.3dPh%s", stemp,chStokes[jStoke],ant,tname);
      } else {       /* all */
	g_snprintf (outFile, 128, "%s%cAllPh%s", stemp,chStokes[jStoke],tname);
      }
    } else {      /* Stokes */
      if (ant>0) {   /* One */
	g_snprintf (outFile, 128, "%s%c%3.3d%s", stemp,chStokes[jStoke],ant,tname);
      } else {       /* all */
	g_snprintf (outFile, 128, "%s%cAll%s", stemp,chStokes[jStoke],tname);
      }
    }  /* end branch by type */

    /* output FITS disk */
    ObitInfoListGet(myInput, "outDisk", &type, dim, &disk, err);
    
    /* Give output Image name */
    Obit_log_error(err, OBIT_InfoErr, "Output FITS image %s on disk %d ",
		   outFile, disk);

    /* define object */
    ObitImageSetFITS (outImage, OBIT_IO_byPlane, disk, outFile, blc, trc, err);
    if (err->error) Obit_traceback_val (err, routine, "myInput", outImage);
  } else { /* Unknown type - barf and bail */
    Obit_log_error(err, OBIT_Error, "%s: Unknown Data type %s", 
		   pgmName, Type);
    return outImage;
  } /* end of branch by output type */
  if (err->error) Obit_traceback_val (err, routine, "myInput", outImage);
  
  /* Size of image */
  ObitInfoListGetTest(myInput, "nx", &type, dim, &nx);
  ObitInfoListGetTest(myInput, "ny", &type, dim, &ny);
  ObitInfoListGetTest(myInput, "xCells", &type, dim, &xCells);
  ObitInfoListGetTest(myInput, "yCells", &type, dim, &yCells);

  /* Define header */
  /* Most info from UV Data descriptor */
  outDesc = outImage->myDesc;
  ObitImageUtilUV2ImageDesc (inData->myDesc, outDesc, TRUE, 1);

  /* Creation date today */
  today = ObitToday();
  strncpy (outDesc->date, today, IMLEN_VALUE-1);
  if (today) g_free(today);

  /* units */
  if (doPhase) {
    strcpy (outImage->myDesc->bunit, "DEGREE  ");
  }else {
    strcpy (outImage->myDesc->bunit, "JY/BEAM ");
  }

  /* Define axes */
  axNum = 0;

  /* Azimuth */
  strcpy (outImage->myDesc->ctype[axNum], "AZIMUTH");
  outImage->myDesc->inaxes[axNum] = nx;
  outImage->myDesc->crpix[axNum]  = nx/2 + 1.0;
  outImage->myDesc->cdelt[axNum]  = xCells/3600.;
  outImage->myDesc->crval[axNum]  = 0.0;
  axNum++;

  /* Elevation */
  strcpy (outImage->myDesc->ctype[axNum], "ELEVATIO");
  outImage->myDesc->inaxes[axNum] = ny;
  outImage->myDesc->crpix[axNum]  = ny/2 + 1.0;
  outImage->myDesc->cdelt[axNum]  = yCells/3600.;
  outImage->myDesc->crval[axNum]  = 0.0;
  axNum++;

  /* If there is only one channel and multiple IFs, IF axis is first - else Freq */
  if ((inData->myDesc->inaxes[inData->myDesc->jlocf]==1) && 
      ((inData->myDesc->jlocif>=0) && (inData->myDesc->inaxes[inData->myDesc->jlocif]>1))) {
    axFNum  = axNum+1;
    axIFNum = axNum;
  } else {
    axFNum  = axNum;
    axIFNum = axNum+1;
  }

  /* Channel */
  for (i=0; i<8; i++)
    outImage->myDesc->ctype[axFNum][i] = inData->myDesc->ctype[inData->myDesc->jlocf][i];
  outImage->myDesc->inaxes[axFNum] = inData->myDesc->inaxes[inData->myDesc->jlocf];
  outImage->myDesc->crpix[axFNum]  = inData->myDesc->crpix[inData->myDesc->jlocf];
  outImage->myDesc->cdelt[axFNum]  = inData->myDesc->cdelt[inData->myDesc->jlocf];
  outImage->myDesc->crval[axFNum]  = inData->myDesc->crval[inData->myDesc->jlocf];
  axNum++;


  /* IF */
  for (i=0; i<8; i++)
    outImage->myDesc->ctype[axIFNum][i] = inData->myDesc->ctype[inData->myDesc->jlocif][i];
  outImage->myDesc->inaxes[axIFNum] = inData->myDesc->inaxes[inData->myDesc->jlocif];
  outImage->myDesc->crpix[axIFNum]  = inData->myDesc->crpix[inData->myDesc->jlocif];
  outImage->myDesc->cdelt[axIFNum]  = inData->myDesc->cdelt[inData->myDesc->jlocif];
  outImage->myDesc->crval[axIFNum]  = inData->myDesc->crval[inData->myDesc->jlocif];
  axNum++;
  
   /* Stokes */
  for (i=0; i<8; i++)
    outImage->myDesc->ctype[axNum][i] = inData->myDesc->ctype[inData->myDesc->jlocs][i];
  outImage->myDesc->inaxes[axNum] = 1;
  outImage->myDesc->crpix[axNum]  = 1.0;
  outImage->myDesc->cdelt[axNum]  = 1.0;
  outImage->myDesc->crval[axNum]  = StokCrval[jStoke];
  axNum++;

  outImage->myDesc->naxis = axNum;  /* Total number of axes */

  /* reset image max/min */
  outDesc->maxval    = -1.0e20;
  outDesc->minval    =  1.0e20;

  /* Instantiate */
  ObitImageFullInstantiate (outImage, FALSE, err);
  if (err->error) Obit_traceback_val (err, routine, "myInput", outImage);


  return outImage;
} /* end setOutput */

/*----------------------------------------------------------------------- */
/*  Loop over selected sources, these are all sources in the source table */
/*  with selection by souCode, Sources, timeRange etc.                    */
/*   Input:                                                               */
/*      myInput   Input parameters on InfoList                            */
/*      inData    ObitUV to image                                         */
/*   Output:                                                              */
/*      err    Obit Error stack                                           */
/*----------------------------------------------------------------------- */
void doSources  (ObitInfoList* myInput, ObitUV* inData, ObitErr* err)
{
  gchar        Source[17], lastSource[17];
  ObitSourceList* doList;
  gint32       dim[MAXINFOELEMDIM] = {1,1,1,1,1};
  olong         maxlen, isource, failed=0, good=0;
  olong         iant, *antList=NULL, ant;
  gboolean     isBad = FALSE;
  gchar        *Fail="Failed  ", *Done="Done    ";
  gchar        *dataParms[] = {  /* Source selection*/
    "Sources", "souCode", "Qual", "Stokes", "timeRange", "UVRange", "FreqID",
    NULL
  };
  gchar *routine = "doSources";

  /* Get input parameters from myInput, copy to inData */
  ObitInfoListCopyList (myInput, inData->info, dataParms);
  if (err->error) Obit_traceback_msg (err, routine, inData->name);

  /* Make sure selector set on inData */
  ObitUVOpen (inData, OBIT_IO_ReadCal, err);
  ObitUVClose (inData, err);
  if (err->error) Obit_traceback_msg (err, routine, inData->name);
  
  /* Get source list to do */
  doList = ObitUVUtilWhichSources (inData, err);
  if (err->error) Obit_traceback_msg (err, routine, inData->name);

  /* Get list of antennas */
  antList = getAntList(myInput, inData, err);
  if (err->error) Obit_traceback_msg (err, routine, inData->name);

  /* Loop over list of sources */
  strncpy (lastSource, "None such       ", 16);
  for (isource = 0; isource<doList->number; isource++) {
    if (!doList->SUlist[isource]) continue; /* removed? */
    maxlen = MIN (16, strlen(doList->SUlist[isource]->SourceName));
    strncpy (Source, doList->SUlist[isource]->SourceName, maxlen);
    Source[maxlen] = 0;
    /* Ignore if same source name as last - just different qualifier */
    if (!strncmp(lastSource, Source, 16)) continue;
    strncpy (lastSource, Source, 16);
    /* Save position in global */
    RAMean  = doList->SUlist[isource]->RAMean;
    DecMean = doList->SUlist[isource]->DecMean;

    Obit_log_error(err, OBIT_InfoErr, " ******  Source %s ******", Source);
    ObitTrimTrail(Source);  /* remove trailing blanks */

    /* Save field name */
    dim[0] = 16; dim[1] = 1;
    ObitInfoListAlwaysPut (myInput, "FieldName", OBIT_string, dim, Source);

    /* Loop over antennas */
    iant = 0;
    while (antList[iant]>=0) {
      ant = antList[iant];
      iant++;

      /* Process source */
      doChanPoln (Source, ant, myInput, inData, err);
      /* Allow up to 10 failures before first success or up to 10% of large run */
      if (err->error) {
	ObitErrLog(err); /* Show failure messages */
	failed++;
	isBad = TRUE;
	if (((failed>=10) && (good<=0)) || 
	    (((failed>=10)&&(failed>0.1*doList->number)))) {
	  /* This isn't working - Give up */
	  Obit_log_error(err, OBIT_Error, "%s: Too many failures, giving up", 
			 routine);
	  return;
	}
      } else {
	isBad = FALSE;
	good++;
      } /* OK */
    } /* end antenna loop */


    /* Save processing summary - success or failure? */
    dim[0] = 8; dim[1] = 1;
    if (isBad) 
      ObitInfoListAlwaysPut (myInput, "Status", OBIT_string, dim, Fail);
    else
      ObitInfoListAlwaysPut (myInput, "Status", OBIT_string, dim, Done);
  } /* end source loop */

  doList = ObitSourceListUnref(doList);
  if (antList) g_free(antList);

}  /* end doSources */

/*----------------------------------------------------------------------- */
/*  Loop over frequencies and polarizations for a single source/antenna   */
/*   Input:                                                               */
/*      Source    Name of source being imaged                             */
/*      ant       Antenna number of image, 0=>all averaged                */
/*      myInput   Input parameters on InfoList                            */
/*      inData    ObitUV to image                                         */
/*   Output:                                                              */
/*      err    Obit Error stack                                           */
/*----------------------------------------------------------------------- */
void doChanPoln (gchar *Source, olong ant, ObitInfoList* myInput, 
		 ObitUV* inData, ObitErr* err)
{
  ObitUV       *avgData = NULL;
  ObitImage    *outImage[4]={NULL,NULL,NULL,NULL};
  ObitFArray   **imgList = NULL;
  ObitInfoType type;
  gint32       dim[MAXINFOELEMDIM] = {1,1,1,1,1};
  olong        i, loop, nloop, nchan, nIF, npoln, ichan, iIF, ipoln, plane[5];
  gboolean     doRMS=FALSE, doPhase=FALSE, isRMS[3] = {FALSE, TRUE, TRUE};
  gboolean     isPhase[3] = {FALSE, TRUE, TRUE};
  gchar        *chStokes=" IQUV";
  gchar        *tableList[]={"AIPS FQ","AIPS AN", NULL};
  gchar        *routine = "doChanPoln";

  /* error checks */
  if (err->error) return;
  g_assert (ObitInfoListIsA(myInput));
  g_assert (ObitUVIsA(inData));

  /* Get averaged data */
  avgData = doAvgData (myInput, inData, err);
  if (err->error) goto cleanup;

  /* Want RMS or phase images as well? */
  ObitInfoListGetTest(myInput, "doRMS",   &type, dim, &doRMS);
  ObitInfoListGetTest(myInput, "doPhase", &type, dim, &doPhase);
  nloop = 1;
  /* Set up loops for, ampl, ampl+phase, amp+rms and amp+phase+rms */
  if (doRMS)   nloop++;
  if (doPhase) nloop++;
  if (doRMS && doPhase) {isPhase[2] = isRMS[1] = FALSE; }

  /* Loop over poln, phase, RMS */
  for (loop=0; loop<nloop; loop++) {

    /* Image data in order (fastest to slowest, channel, IF, Stokes ) */
    imgList = doImage (isRMS[loop], isPhase[loop], ant, myInput, avgData, &nchan, &nIF, &npoln,  
		       err);
    if (err->error) goto cleanup;
    
    /* Write images */
    for (i=0; i<5; i++) plane[i] = 1;
    i = 0;
    for (ipoln=0; ipoln<npoln; ipoln++) {
      /* Create output for each poln */
      outImage[ipoln] = setOutput (Source, ipoln, ant, isRMS[loop], isPhase[loop], 
				   myInput, avgData, err);
      if (err->error) goto cleanup;
      ObitImageOpen (outImage[ipoln], OBIT_IO_WriteOnly, err);
      /* Save average geometry */
      dim[0] = dim[1] = dim[2] = 1;
      ObitInfoListAlwaysPut (outImage[ipoln]->myDesc->info, "avgAz", OBIT_float, dim, &avgAz);
      ObitInfoListAlwaysPut (outImage[ipoln]->myDesc->info, "avgEl", OBIT_float, dim, &avgEl);
      ObitInfoListAlwaysPut (outImage[ipoln]->myDesc->info, "avgPA", OBIT_float, dim, &avgPA);

      /* If there is only one channel and multiple IFs, IF axis is first - else Freq */
      if ((nchan==1) && (nIF>1)) { /* IF first */
	for (ichan=0; ichan<nchan; ichan++) {
	  plane[1] = ichan+1;
	  for (iIF=0; iIF<nIF; iIF++) {
	    plane[0] = iIF+1;
	    /* DEBUG
	    if ((iIF==11) && (ichan==2)) {
	      fprintf (stderr,"IF %d ch %d val %f\n",iIF,ichan,imgList[i]->array[51+50*100]);
	    } */
	    ObitImagePutPlane (outImage[ipoln], imgList[i++]->array, plane,  err);
	    if (err->error) goto cleanup;
	  }
	}
      } else { /* Freq first */
	for (iIF=0; iIF<nIF; iIF++) {
	  plane[1] = iIF+1;
	  for (ichan=0; ichan<nchan; ichan++) {
	    plane[0] = ichan+1;
	    /* DEBUG 
	    if ((iIF==11) && (ichan==2)) {
	      fprintf (stdout,"IF %d ch %d 50,48 val %f\n",iIF,ichan,imgList[i]->array[50+48*100] );
	      fprintf (stdout,"IF %d ch %d 50,49 val %f\n",iIF,ichan,imgList[i]->array[50+49*100] );
	      fprintf (stdout,"IF %d ch %d 50,50 val %f\n",iIF,ichan,imgList[i]->array[50+50*100] );
	      fprintf (stdout,"IF %d ch %d 50,51 val %f\n",iIF,ichan,imgList[i]->array[50+51*100] );
	      fprintf (stdout,"IF %d ch %d 51,51 val %f\n",iIF,ichan,imgList[i]->array[51+51*100] );
	    } */
	    ObitImagePutPlane (outImage[ipoln], imgList[i++]->array, plane,  err);
	    if (err->error) goto cleanup;
	  } /* end Channel loop */
	} /* end IF loop */
      }
      ObitImageClose (outImage[ipoln], err);
      if (err->error) goto cleanup;
      /* Copy FQ & AN tables */
      ObitDataCopyTables ((ObitData*)inData, (ObitData*)outImage[ipoln], NULL,
			  tableList, err);
    } /* end stokes loop */
    
    /* History */
    for (ipoln=0; ipoln<npoln; ipoln++) {
      MapBeamHistory (Source, chStokes[ipoln], myInput, inData, 
		      outImage[ipoln], err);
      if (err->error) goto cleanup;
    }  
    
    /* Cleanup */
    if (imgList) {
      i = 0;
      for (ipoln=0; ipoln<npoln; ipoln++) {
	outImage[ipoln] = ObitImageUnref(outImage[ipoln]);
	for (iIF=0; iIF<nIF; iIF++) {
	  for (ichan=0; ichan<nchan; ichan++) {
	    ObitFArrayUnref(imgList[i++]);
	  }
	}
      }
      g_free(imgList); imgList = NULL;
    } /* end cleanup image list */
  } /* end Stokes/RMS loop */
  
    /* Cleanup */
 cleanup:
  if (imgList) {
    i = 0;
    for (ipoln=0; ipoln<npoln; ipoln++) {
      outImage[ipoln] = ObitImageUnref(outImage[ipoln]);
      for (iIF=0; iIF<nIF; iIF++) {
	for (ichan=0; ichan<nchan; ichan++) {
	  ObitFArrayUnref(imgList[i++]);
	}
      }
    }
    g_free(imgList); imgList = NULL;
  } /* end cleanup image list */

  avgData  = ObitUVUnref(avgData); /* Scratch averaged data */
  if (err->error) Obit_traceback_msg (err, routine, inData->name);
}  /* end doChanPoln */

/*----------------------------------------------------------------------- */
/*  Image all selected data                                               */
/*  Returns a list of ObitFArrays containing pixel data                   */
/*  fastest to slowest, channel, IF, Stokes                               */
/*   Input:                                                               */
/*      doRMS     If true make images of RMS, else Poln                   */
/*      doPhase   If TRUE, image is Phase, else Amplitude                 */
/*      ant       Antenna number of image, 0=>all                         */
/*      myInput   Input parameters on InfoList                            */
/*      inData    ObitUV to image                                         */
/*   Output:                                                              */
/*      nchan     Number of channels in output                            */
/*      nIF       Number of IFs in output                                 */
/*      npoln     Number of Stokes in output                              */
/*      err       Obit Error stack                                        */
/*   Output in globals                                                    */
/*      avgAz     Average Azimuth (deg)                                   */
/*      avgEl     Average Elevation (deg)                                 */
/*      avgPA     Average Parallactic angle (deg)                         */
/*   Return: List of FArrays, Unref/g_free when done                      */
/*----------------------------------------------------------------------- */
ObitFArray** doImage (gboolean doRMS, gboolean doPhase, olong ant, 
		      ObitInfoList* myInput, 
		      ObitUV* inData, olong *nchan, olong *nIF, olong *npoln, 
		      ObitErr* err)
{
  ObitFArray   **out = NULL;
  ObitFArray   *fatemp = NULL;
  ObitInfoType type;
  gint32       dim[MAXINFOELEMDIM] = {1,1,1,1,1};
  olong        nx=11, ny=11, naxis[2], ipos[2], i, ipoln, ichan, iIF;
  olong        selem, nelem, size, indx, qndx, undx, vndx;
  ofloat       *SumIr=NULL, *SumIi=NULL, *SumII=NULL, *SumIWt=NULL;
  ofloat       *SumQr=NULL, *SumQi=NULL, *SumQQ=NULL, *SumQWt=NULL;
  ofloat       *SumUr=NULL, *SumUi=NULL, *SumUU=NULL, *SumUWt=NULL;
  ofloat       *SumVr=NULL, *SumVi=NULL, *SumVV=NULL, *SumVWt=NULL;
  ofloat       *SumAzCell=NULL, *SumElCell=NULL, *SumPACell=NULL;
  ofloat       *Center, *ICenter, value;
  olong        *CntCell=NULL;
  gchar        Stokes[10];
  gboolean trueStokes=FALSE, doPolStokes=FALSE;
  gchar *routine = "doImage";

  /* error checks */
  if (err->error) return out;
  g_assert (ObitInfoListIsA(myInput));
  g_assert (ObitUVIsA(inData));

  /*  Number of channels */
  *nchan = inData->myDesc->inaxes[inData->myDesc->jlocf];
  /* Number of IFs */
  if (inData->myDesc->jlocif>=0)
    *nIF  = inData->myDesc->inaxes[inData->myDesc->jlocif];
  else *nIF = 1;
  /* Number of polarizations */
  *npoln = inData->myDesc->inaxes[inData->myDesc->jlocs];
  /* Stokes or correlator values */
  trueStokes = inData->myDesc->crval[inData->myDesc->jlocs]>0.0;

  /* Stokes */
  Stokes[0] = 'I'; Stokes[1] = 0;
  ObitInfoListGetTest(myInput, "Stokes", &type, dim, Stokes);
  doPolStokes = trueStokes && (Stokes[1]=='Q') && (Stokes[2]=='U');

  /* How big an image? */
  ObitInfoListGetTest(myInput, "nx", &type, dim, &nx);
  ObitInfoListGetTest(myInput, "ny", &type, dim, &ny);
  naxis[0] = nx; naxis[1] = ny;

  /* Make accumulation lists */
  selem = (*nIF) * (*nchan); /* size of list element */
  nelem = nx * ny;           /* Number of list elements */
  nelem *= 2;                /* To be sure big enough */
  /* May need alot bigger if averaging antennas and offsets given */
  if (ant<=0) nelem *= 100;
  size = selem * nelem;      /* size of list */
  SumIr     = g_malloc0(size*sizeof(ofloat));
  SumIi     = g_malloc0(size*sizeof(ofloat));
  SumII     = g_malloc0(size*sizeof(ofloat));
  SumIWt    = g_malloc0(size*sizeof(ofloat));
  if (doPolStokes) {
    SumQr     = g_malloc0(size*sizeof(ofloat));
    SumQi     = g_malloc0(size*sizeof(ofloat));
    SumQQ     = g_malloc0(size*sizeof(ofloat));
    SumQWt    = g_malloc0(size*sizeof(ofloat));
    SumUr     = g_malloc0(size*sizeof(ofloat));
    SumUi     = g_malloc0(size*sizeof(ofloat));
    SumUU     = g_malloc0(size*sizeof(ofloat));
    SumUWt    = g_malloc0(size*sizeof(ofloat));
    SumVr     = g_malloc0(size*sizeof(ofloat));
    SumVi     = g_malloc0(size*sizeof(ofloat));
    SumVWt    = g_malloc0(size*sizeof(ofloat));
    SumVV     = g_malloc0(size*sizeof(ofloat));
  }
  SumAzCell = g_malloc0(nelem*sizeof(ofloat));  
  SumElCell = g_malloc0(nelem*sizeof(ofloat));
  SumPACell = g_malloc0(nelem*sizeof(ofloat));
  CntCell   = g_malloc0(nelem*sizeof(olong));

  /* Accumulate data onto lists */
  accumData (inData, myInput, ant, *nchan, *nIF, selem, &nelem,
	     SumIr, SumIi, SumII, SumIWt, SumQr, SumQi, SumQQ, SumQWt,
	     SumUr, SumUi, SumUU, SumUWt, SumVr, SumVi, SumVV, SumVWt,
	     SumAzCell, SumElCell, SumPACell, CntCell, 
	     &avgAz, &avgEl, &avgPA, err);
  if (err->error) goto cleanup;

  /* Create output */
  out = g_malloc0((*nchan)*(*nIF)*(*npoln)*sizeof(ObitFArray*));
  i = 0;
  for (ipoln=0; ipoln<(*npoln); ipoln++) {
    for (iIF=0; iIF<(*nIF); iIF++) {
      for (ichan=0; ichan<(*nchan); ichan++) {
	out[i++] = ObitFArrayCreate (NULL, 2, naxis);
      }
    }
  }

  /* Grid data */
  gridData (myInput, *nchan, *nIF, *npoln, selem, nelem, doRMS, doPhase,
	    SumIr, SumIi, SumII, SumQr, SumQi, SumQQ, 
	    SumUr, SumUi, SumUU, SumVr, SumVi, SumVV, 
	    SumAzCell, SumElCell, out);

  /* No normalization for RMS, phase */
  if (doRMS|| doPhase) goto cleanup;

  /* DEBUG IF 12, ch 3 
    indx = 0*selem + 11*(*nchan) + 2;
    value =  1.0/out[indx]->array[50+50*100];
    fprintf (stdout,"Grid IF %d ch %d 50,48 val %f\n",11,2,out[indx]->array[50+48*100]*value );
    fprintf (stdout,"Grid IF %d ch %d 50,49 val %f\n",11,2,out[indx]->array[50+49*100]*value );
    fprintf (stdout,"Grid IF %d ch %d 50,50 val %f\n",11,2,out[indx]->array[50+50*100]*value );
    fprintf (stdout,"Grid IF %d ch %d 50,51 val %f\n",11,2,out[indx]->array[50+51*100]*value );
    fprintf (stdout,"Grid IF %d ch %d 51,51 val %f\n",11,2,out[indx]->array[51+51*100]*value );*/

   /* true Stokes: Subtract center Q, U (source poln) * Ipol beam from rest */
  if (doPolStokes) {
    fatemp = ObitFArrayCreate (NULL, 2, naxis);  /* Work array */
    ipos[0] = nx/2; ipos[1] = ny/2;
    for (iIF=0; iIF<(*nIF); iIF++) {
      for (ichan=0; ichan<(*nchan); ichan++) {
	/* Q */ 
	indx = 0*selem + iIF*(*nchan) + ichan;
	qndx = 1*selem + iIF*(*nchan) + ichan;
	ICenter = ObitFArrayIndex (out[indx],  ipos);
	Center  = ObitFArrayIndex (out[qndx],  ipos);
	value = -(*Center) / (*ICenter);  /* Normalize IPol beam */
	fatemp = ObitFArrayCopy (out[indx], fatemp, err);
	if (err->error) goto cleanup;
	ObitFArraySMul (fatemp, value);  /* Q * IPol beam */
	if ((*npoln>1) && out[qndx]) ObitFArrayAdd (out[qndx], fatemp, out[qndx]);
	/* U */ 
	undx = 2*selem + iIF*(*nchan) + ichan;
	Center = ObitFArrayIndex (out[undx],  ipos);
	value = -(*Center) / (*ICenter);
	fatemp = ObitFArrayCopy (out[indx], fatemp, err);
	if (err->error) goto cleanup;
	ObitFArraySMul (fatemp, value);  /* U * IPol beam */
	if ((*npoln>2) && out[undx]) ObitFArrayAdd (out[undx], fatemp, out[undx]);
      }
    }

    /* Divide Q,U,V by I  */
    for (iIF=0; iIF<(*nIF); iIF++) {
      for (ichan=0; ichan<(*nchan); ichan++) {
	indx = 0*selem + iIF*(*nchan) + ichan;
	qndx = 1*selem + iIF*(*nchan) + ichan;
	if ((*npoln>2) && out[qndx]) ObitFArrayDiv (out[qndx], out[indx], out[qndx]);
	undx = 2*selem + iIF*(*nchan) + ichan;
	if ((*npoln>2) && out[undx]) ObitFArrayDiv (out[undx], out[indx], out[undx]);
	vndx = 3*selem + iIF*(*nchan) + ichan;
	if ((*npoln>3) && out[vndx]) ObitFArrayDiv (out[vndx], out[indx], out[vndx]);
      }
    }
  } /* end true Stokes */

  /* Normalize I or parallel hand correlations */
  ipos[0] = nx/2; ipos[1] = ny/2;
  for (iIF=0; iIF<(*nIF); iIF++) {
    for (ichan=0; ichan<(*nchan); ichan++) {
      indx = 0*selem + iIF*(*nchan) + ichan;
      Center = ObitFArrayIndex (out[indx],  ipos);
      value = (*Center);
      if (value!=0.0) value = 1.0 / value;
      ObitFArraySMul (out[indx], value);
      if (!trueStokes && (*npoln>1)) {
       indx = 1*selem + iIF*(*nchan) + ichan;
       Center = ObitFArrayIndex (out[indx],  ipos);
       value = (*Center);
       if (value!=0.0) value = 1.0 / value;
       ObitFArraySMul (out[indx], value);
      }
    }
  } /* end IF Loop */

  /* Cleanup */
 cleanup:
  if (SumIr)     g_free(SumIr);
  if (SumIi)     g_free(SumIi);
  if (SumII)     g_free(SumII);
  if (SumIWt)    g_free(SumIWt);
  if (SumQr)     g_free(SumQr);
  if (SumQi)     g_free(SumQi);
  if (SumQQ)     g_free(SumQQ);
  if (SumQWt)    g_free(SumQWt);
  if (SumUr)     g_free(SumUr);
  if (SumUi)     g_free(SumUi);
  if (SumUU)     g_free(SumUU);
  if (SumUWt)    g_free(SumUWt);
  if (SumVr)     g_free(SumVr);
  if (SumVi)     g_free(SumVi);
  if (SumVV)     g_free(SumVV);
  if (SumVWt)    g_free(SumVWt);
  if (SumAzCell) g_free(SumAzCell);
  if (SumElCell) g_free(SumElCell);
  if (CntCell)   g_free(CntCell);
  fatemp = ObitFArrayUnref(fatemp);
  if (err->error) Obit_traceback_val (err, routine, inData->name, out);

  return out;
} /* end doImage */

/*----------------------------------------------------------------------- */
/*  Write History for MapBeam                                             */
/*   Input:                                                               */
/*      Source    Name of source being imaged                             */
/*      Stoke     Stokes's parameter imaged I, Q, U, V                    */
/*      myInput   Input parameters on InfoList                            */
/*      inData    ObitUV to copy history from                             */
/*      outImage  ObitImage to write history to                           */
/*   Output:                                                              */
/*      err    Obit Error stack                                           */
/*----------------------------------------------------------------------- */
void MapBeamHistory (gchar *Source, gchar Stoke, ObitInfoList* myInput, 
		    ObitUV* inData, ObitImage* outImage, ObitErr* err)
{
  ObitHistory *inHistory=NULL, *outHistory=NULL;
  gchar        hicard[81];
  gchar        *hiEntries[] = {
    "DataType", "inFile",  "inDisk", "inName", "inClass", "inSeq",
    "Sources", "Qual", "souCode", "timeRange",  "Stokes",
    "FreqID", "BIF", "EIF", "BChan", "EChan",  
    "doCalib", "gainUse", "doPol",  "PDVer", "flagVer", "doBand ",  "BPVer", "Smooth",
    "outDType", "outFile",  "outDisk", "outName", "outSeq",
    "nx", "ny", "xCells", "yCells", "hwid", "avgTime", "avgFreq", "chAvg", "ChanSel",
    "blnkTime", "avgAnt", "doRMS", "doPhase", "doPolNorm", "Antennas", "RefAnts",
    "OffAz", "OffEl",
    NULL};
  gchar *routine = "MapBeamHistory";

  /* error checks */
  if (err->error) return;
  g_assert (ObitInfoListIsA(myInput));
  g_assert (ObitUVIsA(inData));
  g_assert (ObitImageIsA(outImage));

  /* Do Image history  */
  inHistory  = newObitDataHistory ((ObitData*)inData, OBIT_IO_ReadOnly, err);
  outHistory = newObitDataHistory ((ObitData*)outImage, OBIT_IO_WriteOnly, err);

  /* If FITS copy header */
  if (inHistory->FileType==OBIT_IO_FITS) {
    ObitHistoryCopyHeader (inHistory, outHistory, err);
  } else { /* simply copy history */
    ObitHistoryCopy (inHistory, outHistory, err);
  }
  if (err->error) Obit_traceback_msg (err, routine, inData->name);
  
  /* Add this programs history */
  ObitHistoryOpen (outHistory, OBIT_IO_ReadWrite, err);
  g_snprintf (hicard, 80, " Start Obit task %s ",pgmName);
  ObitHistoryTimeStamp (outHistory, hicard, err);
  if (err->error) Obit_traceback_msg (err, routine, outImage->name);

  /* Write source and poln */
  if (Stoke==' ') Stoke = 'I';
  g_snprintf (hicard, 80, "%s Source = '%s', Stokes= '%c'", pgmName, Source, Stoke);
  ObitHistoryWriteRec (outHistory, -1, hicard, err);
  if (err->error) Obit_traceback_msg (err, routine, outImage->name);

  /* Copy selected values from myInput */
  ObitHistoryCopyInfoList (outHistory, pgmName, hiEntries, myInput, err);
  if (err->error) Obit_traceback_msg (err, routine, outImage->name);

  /* Average geometry */
  g_snprintf (hicard, 80, "%s / Average observing Azimuth %f deg", pgmName, avgAz);
  ObitHistoryWriteRec (outHistory, -1, hicard, err);
  if (err->error) Obit_traceback_msg (err, routine, outImage->name);
  g_snprintf (hicard, 80, "%s / Average observing Elevation %f deg", pgmName, avgEl);
  ObitHistoryWriteRec (outHistory, -1, hicard, err);
  if (err->error) Obit_traceback_msg (err, routine, outImage->name);
  g_snprintf (hicard, 80, "%s / Average observing Para. Angle %f deg", pgmName, avgPA);
  ObitHistoryWriteRec (outHistory, -1, hicard, err);
  if (err->error) Obit_traceback_msg (err, routine, outImage->name);

  ObitHistoryClose (outHistory, err);
  if (err->error) Obit_traceback_msg (err, routine, outImage->name);

  inHistory  = ObitHistoryUnref(inHistory);  /* cleanup */
  outHistory = ObitHistoryUnref(outHistory);
 
} /* end MapBeamHistory  */

/*----------------------------------------------------------------------- */
/*   Average data as requested                                            */
/*   Input:                                                               */
/*      myInput   Input parameters on InfoList, uses:                     */
/*      inData    ObitUV to average from                                  */
/*   Output:                                                              */
/*      err    Obit Error stack                                           */
/*   Return:                                                              */
/*     Averaged scratch data file, Unref when done                        */
/*----------------------------------------------------------------------- */
ObitUV* doAvgData (ObitInfoList *myInput, ObitUV* inData, ObitErr *err)
{
  ObitUV       *avgData=NULL;
  ObitUV       *scrData=NULL;
  ObitInfoType type;
  gint32       dim[MAXINFOELEMDIM]= {1,1,1,1,1};
  olong        avgFreq, nchAvg;
  ofloat       timeAvg;
  gboolean     isScratch, doAvgAll;
  /*gchar        Stokes[5];*/
  gchar        *dataParms[] = {  /* Parameters to calibrate/select data */
    "UVRange", "timeRange", "doCalSelect", 
    "BIF", "EIF", "BChan", "EChan","subA", "Antennas", "Stokes",
    "doCalib", "gainUse", "doBand", "BPVer", "Smooth", "flagVer", 
    "doPol", "PDVer",  "avgTime", "avgFreq", "ChanSel", 
    NULL
  };
  gchar        *routine= "doAvgData";

  if (err->error) return avgData;  /* Prior error? */

  /* Get input parameters from myInput, copy to inData */
  ObitInfoListCopyList (myInput, inData->info, dataParms);
  if (err->error) Obit_traceback_val (err, routine, inData->name, avgData);
  
  /* Convert to IQUV 
  strcpy (Stokes, "IQUV");
  dim[0] = strlen(Stokes);
  ObitInfoListAlwaysPut (inData->info, "Stokes", OBIT_string, dim, Stokes);*/

  /* Make scratch file, possibly with time and freq averaging */
  avgFreq = 0;
  ObitInfoListGetTest(myInput, "avgFreq",  &type, dim, &avgFreq);
  nchAvg = 1;
  ObitInfoListGetTest(myInput, "chAvg",  &type, dim, &nchAvg);
  timeAvg = 0.0;
  ObitInfoListGetTest(myInput, "avgTime",  &type, dim, &timeAvg);
  timeAvg /= 60.0;  /* Convert to min. */

  /* Average all channels/IFs? */
  doAvgAll = (avgFreq==3);

  /* If both temporal and frequency averaging, frequency average to scratch */
  if ((avgFreq>0) && (timeAvg>0.0)) {
    /* First frequency */
    dim[0] = dim[1] = 1;
    ObitInfoListAlwaysPut (inData->info, "NumChAvg", OBIT_long, dim, &nchAvg);
    dim[0] = dim[1] = 1;
    ObitInfoListAlwaysPut (inData->info, "doAvgAll", OBIT_bool, dim, &doAvgAll);
    isScratch = TRUE;
    scrData = ObitUVUtilAvgF (inData, isScratch, NULL, err);
    if (err->error) Obit_traceback_val (err, routine, inData->name, avgData);
    /* Then time */
    dim[0] = 1;
    ObitInfoListAlwaysPut (scrData->info, "timeAvg", OBIT_float, dim, &timeAvg);
    isScratch = TRUE;
    avgData = ObitUVUtilAvgT (scrData, isScratch, NULL, err);
    if (err->error) Obit_traceback_val (err, routine, inData->name, avgData);
    scrData = ObitUVUnref(scrData);

  } else if (avgFreq>0) {    /* Freq averaging only */
    dim[0] = dim[1] = 1;
    ObitInfoListAlwaysPut (inData->info, "NumChAvg", OBIT_long, dim, &nchAvg);
    dim[0] = dim[1] = 1;
    ObitInfoListAlwaysPut (inData->info, "doAvgAll", OBIT_bool, dim, &doAvgAll);
    isScratch = TRUE;
    avgData = ObitUVUtilAvgF (inData, isScratch, NULL, err);
    if (err->error) Obit_traceback_val (err, routine, inData->name, avgData);

  } else if (timeAvg>0.0) {  /* Time averaging only */
    dim[0] = 1;
    ObitInfoListAlwaysPut (inData->info, "timeAvg", OBIT_float, dim, &timeAvg);
    isScratch = TRUE;
    avgData = ObitUVUtilAvgT (inData, isScratch, NULL, err);
    if (err->error) Obit_traceback_val (err, routine, inData->name, avgData);

  } else { /* No averaging - straight copy */
    /* Scratch file for copy */
    avgData = newObitUVScratch (inData, err);
    /* Calibrate/edit/copy data to scratch file */
    avgData = ObitUVCopy (inData, avgData, err);
    if (err->error) Obit_traceback_val (err, routine, inData->name, avgData);
  }

  return avgData;
} /* end avgData */

/*----------------------------------------------------------------------- */
/*  Get list of antennas                                                  */
/*   Input:                                                               */
/*      myInput   Input parameters on InfoList, uses:                     */
/*        avgAnt    If TRUE average all antennas                          */
/*        Antennas  Desired antennas                                      */
/*   Output:                                                              */
/*      err    Obit Error stack                                           */
/*   Return:                                                              */
/*     -1 terminated list, 0=> average antennas, g_free when done         */
/*----------------------------------------------------------------------- */
olong* getAntList (ObitInfoList* myInput, ObitUV *inData, ObitErr* err)
{
  olong        *out=NULL;
  ObitInfoType type;
  gint32       dim[MAXINFOELEMDIM];
  gboolean     avgAnt=TRUE, want;
  olong        i, j, iout, count, *Antennas, *RefAnts, nant, nref;
  gchar        *routine= "getAntList";

  if (err->error) return out;  /* Prior error? */
  /* reference antennas */
  ObitInfoListGetP(myInput, "RefAnts",  &type, dim, (gpointer)&RefAnts);
  nref = dim[0];

  ObitInfoListGetTest(myInput, "avgAnt", &type, dim, &avgAnt);
  if (avgAnt) { /* Average all selected */
    out = g_malloc0(2*sizeof(olong));
    out[0] = 0;
    out[1] = -1;
    return out;
  } else { /* individual antenna output */
    /* Antenna list */
    ObitInfoListGetP(myInput, "Antennas",  &type, dim, (gpointer)&Antennas);
    if (Antennas==NULL) {  /* Not given */
      Obit_log_error(err, OBIT_Error, "%s: Antenna list not given", routine);
      return out;
    }
    /* Count */
    count = 0;
    for (i=0; i<dim[0]; i++) {
      if (Antennas[i]==0) break;
      count++;
    }
    /* If none selected all wanted */
    if (count<=0) {
      out  = g_malloc0((dim[0]+1)*sizeof(olong));
      iout = 0;
      nant = MIN (dim[0], inData->myDesc->numAnt[0]);
      for (i=0; i<nant; i++) {
	/* Want? */
	want = TRUE;
	for (j=0; j<nref; j++) if ((i+1)==RefAnts[j]) want = FALSE;
	if (want) out[iout++] = i+1;
      }
      out[iout] = -1;
    } else { /* explicit list */
      out  = g_malloc0((count+1)*sizeof(olong));
      iout = 0;
      for (i=0; i<count; i++) {
 	/* Want? */
	want = TRUE;
	for (j=0; j<nref; j++) if (Antennas[i]==RefAnts[j]) want = FALSE;
	if (want) out[iout++] = Antennas[i];
     }
      out[iout] = -1;
    }
  } /* end individual antenna output */

  return out;
} /* end getAntList */

/*----------------------------------------------------------------------- */
/*  Accumulate data into lists                                            */
/*  Accumulate real part of correlations for all Stokes/freq/IF           */
/*  Sums are in a list with each set of entries corresponding to a given  */
/*  pointing.                                                             */
/*  Q and U corrected for parallactic angle                               */
/*   Input:                                                               */
/*      myInput   Input parameters on InfoList                            */
/*      inData    ObitUV to image                                         */
/*      ant       Antenna number of image, 0=>all                         */
/*      nchan     Number of channels in output                            */
/*      nIF       Number of IFs in output                                 */
/*      selem     Size (floats) of list element                           */
/*   In/Output:                                                           */
/*      nelem     (in) Max number of list elements                        */
/*                (out) Actual number of list elements                    */
/*   Output:                                                              */
/*      SumIr     Real Stokes I accumulation list                         */
/*      SumIi     Imag Stokes I accumulation list                         */
/*      SumII     Stokes I*I accumulation list                            */
/*      SumIWt    I Weight accumulation list                              */
/*      SumQr     Real Stokes Q accumulation list                         */
/*      SumQi     Imag Stokes Q accumulation list                         */
/*      SumQQ     Stokes Q*Q accumulation list                            */
/*      SumQWt    Q Weight accumulation list                              */
/*      SumUr     Real Stokes U accumulation list                         */
/*      SumUi     Imag Stokes U accumulation list                         */
/*      SumUU     Stokes U*U accumulation list                            */
/*      SumUWt    U Weight accumulation list                              */
/*      SumVr     Real Stokes V accumulation list                         */
/*      SumVi     Imag Stokes V accumulation list                         */
/*      SumVV     Stokes V*V accumulation list                            */
/*      SumVWt    V Weight accumulation list                              */
/*      SumAzCell Azimuth offset accumulation list                        */
/*      SumElCell Elevation offset accumulation list                      */
/*      SumPACell Parallactic angle accumulation list                     */
/*      CntCell   Counts in geometry accumulation                         */
/*      avgAz     Average Azimuth (deg)                                   */
/*      avgEl     Average Elevation (deg)                                 */
/*      avgPA     Average Parallactic angle (deg)                         */
/*      err       Obit Error stack                                        */
/*----------------------------------------------------------------------- */
void  accumData (ObitUV* inData, ObitInfoList* myInput, olong ant,
		 olong nchan, olong nIF, olong selem, olong *nelem,
		 ofloat *SumIr, ofloat *SumIi, ofloat *SumII, ofloat *SumIWt,
		 ofloat *SumQr, ofloat *SumQi, ofloat *SumQQ, ofloat *SumQWt,
		 ofloat *SumUr, ofloat *SumUi, ofloat *SumUU, ofloat *SumUWt,
		 ofloat *SumVr, ofloat *SumVi, ofloat *SumVV, ofloat *SumVWt,
		 ofloat *SumAzCell, ofloat *SumElCell, ofloat *SumPACell, 
		 olong *CntCell, 
		 ofloat *avgAz, ofloat *avgEl, ofloat *avgPA, 
		 ObitErr* err)
{
  ObitAntennaList *AList=NULL;
  ObitSource *Source=NULL;
  ObitTableAN *ANTable=NULL;
  ObitIOCode retCode = OBIT_IO_SpecErr;
  ObitInfoType type;
  gint32   dim[MAXINFOELEMDIM] = {1,1,1,1,1};
  ofloat   xCells=1.0, yCells=1.0, blnkTime=0.0, tblank=-1.0e20;
  ofloat   u, v, time, ulast, vlast, tol, Az, El, PA, *farr;
  ofloat   ss, cc, xr, xi, *OffAz=NULL, *OffEl=NULL, fblank =  ObitMagicF();
  odouble  sumAz, sumEl, sumPA;
  olong    count, maxElem=*nelem, iElem, indx, iant, ant1, ant2, suba, off=0, iver;
  olong    i, j, jlocs, jlocf, jlocif, incs, incf, incif, doff, ddoff;
  olong    nx, ny, iIF, ichan, *refAnts, nRefAnt, ix, iy, prtLv=0;
  gboolean OK1, OK2;
  gchar    *routine = "accumData";

  /* error checks */
  if (err->error) return;

  /* Get control parameters */
  ObitInfoListGetTest(myInput, "xCells", &type,   dim, &xCells);
  ObitInfoListGetTest(myInput, "yCells", &type,   dim, &yCells);
  if (yCells==0.0) yCells = xCells;
  ObitInfoListGetTest(myInput, "blnkTime", &type, dim, &blnkTime);
  ObitInfoListGetTest(myInput, "prtLv",  &type, dim, &prtLv);
  /* How big an image? */
  ObitInfoListGetTest(myInput, "nx", &type, dim, &nx);
  ObitInfoListGetTest(myInput, "ny", &type, dim, &ny);
  /* Cell spacing to radians */
  xCells = (xCells / 3600.0) * DG2RAD;
  yCells = (yCells / 3600.0) * DG2RAD;
  /* Blanking time to days */
  blnkTime /= 86400.0;

  /* Pointing offsets */
  if (ObitInfoListGetP(myInput, "OffAz",  &type, dim, (gpointer)&farr)) {
    OffAz = g_malloc0(dim[0]*sizeof(ofloat));
    for (i=0; i<dim[0]; i++) OffAz[i] = DG2RAD * farr[i]/60.0;  /* to radians */
  }
  if (ObitInfoListGetP(myInput, "OffEl",  &type, dim, (gpointer)&farr)) {
    OffEl = g_malloc0(dim[0]*sizeof(ofloat));
    for (i=0; i<dim[0]; i++) OffEl[i] = DG2RAD * farr[i]/60.0;  /* to radians */
  }
  
  /* Ref antennas */
  nRefAnt = 0;
  ObitInfoListGetP(myInput, "RefAnts",  &type, dim, (gpointer)&refAnts);
  for (j=0; j<dim[0]; j++) {
    if (refAnts[j]<=0) break;
    nRefAnt++;
  }
  /* Check that a ref antenna given */
  Obit_return_if_fail ((nRefAnt>0), err, 
		       "%s NO reference antennas given",  routine);  

  /* Initialize */
  *avgAz = 0.0;
  *avgEl = 0.0;
  *avgPA = 0.0;
  sumAz  = sumEl = sumPA = 0.0;
  count  = 0;
  iElem  = -1;
  ulast  = vlast = 1.0e20;
  tol    = 0.3 * xCells;  /* Tolerance 0.3 cells  */
  iant = MAX (1, ant);

  /* Get Antenna List */
  iver = 1;  /* Or Subarray no. */
  ANTable = newObitTableANValue (inData->name, (ObitData*)inData, &iver, 
				 OBIT_IO_ReadOnly, 0, 0, 0, err);
  AList   = ObitTableANGetList (ANTable, err);
  ANTable = ObitTableANUnref(ANTable);   /* Done with table */
  if (err->error) Obit_traceback_msg (err, routine, ANTable->name);

  /* Source - get position from global */
  Source = newObitSource("Temp Source");
  Source->equinox = inData->myDesc->equinox;
  Source->RAMean  = RAMean;
  Source->DecMean = DecMean;
  if (err->error) Obit_traceback_msg (err, routine, inData->name);
  /*  Source->RAMean  = inData->myDesc->crval[inData->myDesc->jlocr];
      Source->DecMean = inData->myDesc->crval[inData->myDesc->jlocd];*/
  /* Compute apparent position */
  ObitPrecessUVJPrecessApp (inData->myDesc, Source);
  
  jlocs = inData->myDesc->jlocs;
  incs  = inData->myDesc->incs;
  jlocf = inData->myDesc->jlocf;
  incf  = inData->myDesc->incf;
  if (inData->myDesc->jlocif>=0) {
    jlocif = inData->myDesc->jlocif;
    incif  = inData->myDesc->incif;
  } else {
    jlocif = 0;
    incif  = 0;
  }

  /* Open uv data if not already open */
  if (inData->myStatus==OBIT_Inactive) {
    retCode = ObitUVOpen (inData, OBIT_IO_ReadOnly, err);
    if (err->error) Obit_traceback_msg (err, routine, inData->name);
  }

  /* Loop through data */
  while (retCode==OBIT_IO_OK) {
    /* read buffer full */
    retCode = ObitUVRead (inData, NULL, err);
    if (err->error) Obit_traceback_msg (err, routine, inData->name);
    indx = 0;

    /* First time? */
    if (iElem<0) {
      iElem  = 0;
      tblank = inData->buffer[indx+inData->myDesc->iloct] + blnkTime;
      ulast  = inData->buffer[indx+inData->myDesc->ilocu];
      vlast  = inData->buffer[indx+inData->myDesc->ilocv];
    }

    /* loop over buffer */
    for (i=0; i<inData->myDesc->numVisBuff; i++) { 
      /* where are we? */
      time =  inData->buffer[indx+inData->myDesc->iloct];
      /* Sign of u seems OK */
      u    = inData->buffer[indx+inData->myDesc->ilocu];
      v    = inData->buffer[indx+inData->myDesc->ilocv];

      /* In blanked time? */
      if (time<tblank) goto next;

      /* Want antennas? */
      ObitUVDescGetAnts(inData->myDesc, &inData->buffer[indx], &ant1, &ant2, &suba);
      if (ant>0) {
	if ((ant1!=ant) && (ant2!=ant)) goto next;
      }
      /* One and only one must be a reference antenna */
      OK1 = FALSE;
      OK2 = FALSE;
      for (j=0; j<nRefAnt; j++) {
	OK1 = OK1 || (ant1==refAnts[j]);
	OK2 = OK2 || (ant2==refAnts[j]);
      }
      if (!(OK1 || OK2)) goto next;
      if (OK1 && OK2)    goto next;

      /* Add pointing offsets */
      if (OffAz) {
	u += OffAz[ant1];
	u += OffAz[ant2];
      }
      if (OffEl) {
	v += OffEl[ant1];
	v += OffEl[ant2];
      }

      /* First time in pointing? */
      if (CntCell[iElem]<=0) {
 	ulast  = u;
	vlast  = v;
     }

      /* New pointing? */
      if ((fabs(u-ulast)>tol) || (fabs(v-vlast)>tol)) {
	ulast  = u;
	vlast  = v;
	tblank = time + blnkTime;
	iElem++;
	/* Check if in bounds */
	Obit_return_if_fail ((iElem<maxElem), err, 
			     "%s Too many pointings %d >= %d",  
			     routine, iElem, maxElem);  
      }

      /* Observing geometry (radians) */
      Az = ObitAntennaListAz (AList, ant1, time, Source);
      El = ObitAntennaListElev (AList, ant1, time, Source);
      PA = ObitAntennaListParAng (AList, ant1, time, Source);

      /* Accumulate */
      sumAz += Az;
      sumEl += El;
      sumPA += PA;
      count++;

      SumAzCell[iElem] += u/xCells;
      SumElCell[iElem] += v/yCells;
      SumPACell[iElem] += PA;
      CntCell[iElem]++;
      /* Loop over freq, IF */
      for (iIF=0; iIF<nIF; iIF++) {
	for (ichan=0; ichan<nchan; ichan++) {
	  off  = iElem*selem + iIF*nchan + ichan;
	  doff = indx + inData->myDesc->nrparm + iIF*incif + ichan*incf;
	  ddoff = doff;
	  /* DEBUG
	  if ((iIF==11) && (ichan==3) && (fabs((u/xCells)+2.0)<2.1) && (fabs((v/yCells)-2.0)<2.1)) {
	    fprintf (stderr,"bl %2d - %2d cx %5.0f cy %5.0f sum %f vis %f %f wt %f\n",
		     ant1, ant2, u/xCells, v/yCells, SumIr[off],
		     inData->buffer[ddoff],inData->buffer[ddoff+1], inData->buffer[ddoff+2]);
	  } */
	  /* I */
	  if (inData->buffer[ddoff+2]>0.0) {
	    SumIr[off]  += inData->buffer[ddoff]*inData->buffer[ddoff+2];
	    SumIi[off]  += inData->buffer[ddoff+1]*inData->buffer[ddoff+2];
	    SumII[off]  += inData->buffer[ddoff]*inData->buffer[ddoff]*inData->buffer[ddoff+2];
	    SumIWt[off] += inData->buffer[ddoff+2]; 
	  }
	  /* Q */
	  ddoff = doff + incs;
	  if (SumQr && inData->buffer[ddoff+2]>0.0) {
	    SumQr[off]  += inData->buffer[ddoff]*inData->buffer[ddoff+2];
	    SumQi[off]  += inData->buffer[ddoff+1]*inData->buffer[ddoff+2];
	    SumQQ[off]  += inData->buffer[ddoff]*inData->buffer[ddoff]*inData->buffer[ddoff+2];
	    SumQWt[off] += inData->buffer[ddoff+2]; 
	  }
	  /* U */
	  ddoff = doff + 2*incs;
	  if (SumUr && inData->buffer[ddoff+2]>0.0) {
	    SumUr[off]  += inData->buffer[ddoff]*inData->buffer[ddoff+2];
	    SumUi[off]  += inData->buffer[ddoff+1]*inData->buffer[ddoff+2];
	    SumUU[off]  += inData->buffer[ddoff]*inData->buffer[ddoff]*inData->buffer[ddoff+2];
	    SumUWt[off] += inData->buffer[ddoff+2]; 
	  }
	  /* V */
	  ddoff = doff + 3*incs;
	  if (SumVr && inData->buffer[ddoff+2]>0.0) {
	    SumVr[off]   += inData->buffer[ddoff]*inData->buffer[ddoff+2];
	    SumVi[off]   += inData->buffer[ddoff+1]*inData->buffer[ddoff+2];
	    SumVV[off]   += inData->buffer[ddoff]*inData->buffer[ddoff]*inData->buffer[ddoff+2];
	    SumVWt[off]  += inData->buffer[ddoff+2]; 
	  }
	}
      }

      /* update data pointers */
    next:
      indx  += inData->myDesc->lrec;
    } /* end loop over buffer */
  } /* end loop over file */
  
    /* Close */
  retCode = ObitUVClose (inData, err);
  if (err->error) Obit_traceback_msg (err, routine, inData->name);

  /* Better have something */
  Obit_return_if_fail ((iElem>=2), err, 
		       "%s: Insufficient data included or bad cell spacing",  
		       routine);  

  /* Number of actual elements */
  *nelem = iElem+1;

  /* Normalize things */
  *avgAz = (sumAz/count)*RAD2DG;
  *avgEl = (sumEl/count)*RAD2DG;
  *avgPA = (sumPA/count)*RAD2DG;

  for (i=0; i<(*nelem); i++) {  /* Loop over elements */
    if (CntCell[i]>0) {
      SumAzCell[i] /= CntCell[i];
      SumElCell[i] /= CntCell[i];
      SumPACell[i] /= CntCell[i];
    } else {  /* Big value so they will be ignored */
      SumAzCell[i] = 1.0e10;
      SumElCell[i] = 1.0e10;
      SumPACell[i] = 1.0e10;
    }

    /* Loop over freq, IF */
    for (iIF=0; iIF<nIF; iIF++) {
      for (ichan=0; ichan<nchan; ichan++) {
	off  = i*selem + iIF*nchan + ichan;
	if (SumIWt[off]>0) {
	  SumIr[off] /= SumIWt[off];
	  SumIi[off] /= SumIWt[off];
	  SumII[off] = (((SumII[off]/SumIWt[off]) - SumIr[off]*SumIr[off]))/SumIWt[off];
	  if (SumII[off]>0.0) SumII[off] = sqrt(SumII[off]);
	  SumII[off] /= SumIr[off];  /* Normalize variance by I */
	  /* DEBUG
	  if ((iIF==11) && (ichan==2) && (fabs((SumAzCell[i])+2.0)<2.1) && (fabs((SumElCell[i])-2.0)<2.1)) {
	    fprintf (stderr,"%5d cx %5.1f cy %5.1f sum %f \n",
		     i, SumAzCell[i], SumElCell[i], SumIr[off]/SumIr[1624*selem + iIF*nchan + ichan]);
	  }  */
	} else {
	  SumIr[off] = fblank;
	  SumIi[off] = fblank;
	  SumII[off] = fblank;
	}
	if (SumQWt) {
	  if (SumQWt[off]>0) {
	    SumQr[off] /= SumQWt[off];
	    SumQi[off] /= SumQWt[off];
	    SumQQ[off] = (((SumQQ[off]/SumQWt[off]) - SumQr[off]*SumQr[off]))/SumQWt[off];
	    if (SumQQ[off]>0.0) SumQQ[off] = sqrt(SumQQ[off]);
	    SumQQ[off] /= SumIr[off];  /* Normalize variance by I */
	  } else {
	    SumQr[off]  = fblank;
	    SumQi[off]  = fblank;
	    SumQQ[off] = fblank;
	  }
	}
	if (SumUWt) {
	  if (SumUWt[off]>0) {
	    SumUr[off] /= SumUWt[off];
	    SumUi[off] /= SumUWt[off];
	    SumUU[off] = (((SumUU[off]/SumUWt[off]) - SumUr[off]*SumUr[off]))/SumUWt[off];
	    if (SumUU[off]>0.0) SumUU[off] = sqrt(SumUU[off]);
	    SumUU[off] /= SumIr[off];  /* Normalize variance by I */
	  } else {
	    SumUr[off] = fblank;
	    SumUi[off] = fblank;
	    SumUU[off] = fblank;
	  }
	}
	if (SumVWt) {
	  if (SumVWt[off]>0) {
	    SumVr[off] /= SumVWt[off];
	    SumVi[off] /= SumVWt[off];
	    SumVV[off] = (((SumVV[off]/SumVWt[off]) - SumVr[off]*SumVr[off]))/SumVWt[off];
	    if (SumVV[off]>0.0) SumVV[off] = sqrt(SumVV[off]);
	    SumVV[off] /= SumIr[off];  /* Normalize variance by I */
	  } else {
	    SumVr[off] = fblank;
	    SumVi[off] = fblank;
	    SumVV[off] = fblank;
	  }
	}
	/* Counter rotate (Q+iU) for parallactic angle */
	if (SumQr && (SumQr[off]!=fblank) && (SumUr[off]!=fblank)) {
	  cc =  cos(2.0*SumPACell[i]);
	  ss = -sin(2.0*SumPACell[i]);
	  /* Not sure this works */
	  xr = SumQr[off];
	  xi = SumUr[off];
	  SumQr[off] = cc*xr - ss*xi;
	  SumUr[off] = cc*xi + ss*xr;
	  xr = SumQi[off];
	  xi = SumUi[off];
	  SumQi[off] = cc*xr - ss*xi;
	  SumUi[off] = cc*xi + ss*xr;
	} /* end counter rotate */
      } /* end channel loop */
    } /* end IF loop */


    /* Add diagnostics IF 12, channel 2*/
    off  = 11*nchan + 2; 
    if (prtLv>=2)  { 
      ix = (olong) (SumAzCell[i] + nx/2 + 1.5);
      iy = (olong) (SumElCell[i] + ny/2 + 1.5);
      if ((SumAzCell[i]>1000.) || (SumElCell[i]>1000.)) continue;
      if (SumQr) { /* Poln data? */
	Obit_log_error(err, OBIT_InfoErr, 
		       "%3.3d Cell %3d %3d Az%8.1f cell, El%8.1f cell, I %6.3f %6.3f Q %6.3f %6.3f U %6.3f %6.3f V %6.3f %6.3f Jy",
		       i, ix,iy, 
		       /*SumAzCell[i]*xCells*206265., SumElCell[i]*yCells*206265., offset in asec */
		       SumAzCell[i], SumElCell[i],   /* offset in cells */
		       SumIr[off+i*selem],SumIi[off+i*selem], SumQr[off+i*selem],SumQi[off+i*selem],
		       SumUr[off+i*selem],SumUi[off+i*selem], SumVr[off+i*selem],SumVi[off+i*selem]);
      } else {
	Obit_log_error(err, OBIT_InfoErr, 
		       "%3.3d Cell %3d %3d Az %8.1f cell, El %8.1f cell, I %6.4f %6.4f Jy",
		       i, ix,iy, 
		       /*SumAzCell[i]*xCells*206265., SumElCell[i]*yCells*206265., offset in asec */
		       SumAzCell[i], SumElCell[i],   /* offset in cells */
		       SumIr[off+i*selem],SumIi[off+i*selem]);
      }
    }
    ObitErrLog(err);
  } /* End loop normalizing list */

  /* Cleanup */
  if (OffAz) g_free(OffAz);
  if (OffEl) g_free(OffEl);

} /* end accumData  */

/*----------------------------------------------------------------------- */
/*  Interpolate quasi regular lists of points onto a regular grid         */
/*  Accumulate real part of correlations for all Stokes/freq/IF           */
/*  Sums are in a list with each set of entries corresponding to a given  */
/*  pointing.                                                             */
/*  May use threading.                                                    */
/*  Adapted from AIPS/MAPBM.FOR                                           */
/*   Input:                                                               */
/*      myInput   Input parameters on InfoList                            */
/*      nchan     Number of channels in output                            */
/*      nIF       Number of IFs in output                                 */
/*      npoln     Number of polarizations in output                       */
/*      selem     Size (floats) of list element                           */
/*      nelem     Number of list elements                                 */
/*      doRMS     If TRUE, image is RMS                                   */
/*      doPhase   If TRUE, image is Phase, else Amplitude                 */
/*      SumIr     Real Stokes I accumulation list                         */
/*      SumIi     Imag Stokes I accumulation list                         */
/*      SumII     Stokes I*I accumulation list                            */
/*      SumQr     Real Stokes Q accumulation list                         */
/*      SumQi     Imag Stokes Q accumulation list                         */
/*      SumQQ     Stokes Q*Q accumulation list                            */
/*      SumUr     Real Stokes U accumulation list                         */
/*      SumUi     Imag Stokes U accumulation list                         */
/*      SumUU     Stokes U*U accumulation list                            */
/*      SumVr     Real Stokes V accumulation list                         */
/*      SumVi     Imag Stokes V accumulation list                         */
/*      SumVV     Stokes V*V accumulation list                            */
/*      SumAzCell Azimuth offset accumulation list                        */
/*      SumElCell Elevation offset  accumulation list                     */
/*   Output:                                                              */
/*      grids     Array of ObitFArrays                                    */
/*                fastest to slowest, channel, IF, Stokes                 */
/*      err       Obit Error stack                                        */
/*----------------------------------------------------------------------- */
void  gridData (ObitInfoList* myInput, olong nchan, olong nIF, olong npoln,
		olong selem, olong nelem, gboolean doRMS, gboolean doPhase,
		ofloat *SumIr, ofloat *SumIi, ofloat *SumII, 
		ofloat *SumQr, ofloat *SumQi, ofloat *SumQQ, 
		ofloat *SumUr, ofloat *SumUi, ofloat *SumUU, 
		ofloat *SumVr, ofloat *SumVi, ofloat *SumVV, 
		ofloat *SumAzCell, ofloat *SumElCell, ObitFArray **grids)
{
  ObitInfoType type;
  gint32   dim[MAXINFOELEMDIM] = {1,1,1,1,1};
  olong nThreads, nx, ny, hwid;
  ofloat xcen, ycen;
  gboolean OK;
  ObitThread *thread;
  MBFuncArg **threadArgs;

  /* How big an image? */
  ObitInfoListGetTest(myInput, "nx", &type, dim, &nx);
  ObitInfoListGetTest(myInput, "ny", &type, dim, &ny);
  ObitInfoListGetTest(myInput, "hwid",   &type, dim, &hwid);
  xcen = (ofloat)(nx/2);
  ycen = (ofloat)(ny/2);

  /* Initialize Threading */
  thread = newObitThread ();
  nThreads = 
    MakeMBFuncArgs (thread, 
		    nchan, nIF, npoln,selem, nelem, doRMS, doPhase,
		    nx, ny, hwid, xcen, ycen,
		    SumIr, SumIi, SumII, SumQr, SumQi, SumQQ,
		    SumUr, SumUi, SumUU, SumVr, SumVi, SumVV, 
		    SumAzCell, SumElCell, grids,
		    &threadArgs);
  /* Do operation */
  OK = ObitThreadIterator (thread, nThreads, 
			   (ObitThreadFunc)ThreadMBGrid,
			   (gpointer**)threadArgs);

  /* Check for problems */
  if (!OK) return;

  /* Free local objects */
  KillMBFuncArgs(nThreads, threadArgs);
  ObitThreadUnref(thread);
} /* end gridData */

/*----------------------------------------------------------------------- */
/*  Lagrangian interpolation coefficients                                 */
/*  For interpolating in a quasi regular grid represented by lists        */
/*  Determine coefficients for elements in lists to interpolate to (x,y)  */
/*   Doesn't seem to shift correctly                                                         */
/*   Input:                                                               */
/*      x         Coordinate on first axis                                */
/*      y         Coordinate on second axis                               */
/*      n         Length of lists                                         */
/*      hwid      Halfwidth of interpolation kernal, decreased at edges   */
/*                0=> closest pixel                                       */
/*      minmax    max, min, x, y in lists                                 */
/*      xlist     List of coordinates on  first axis                      */
/*      ylist     List coordinate on second axis                          */
/*   Output:                                                              */
/*      coef      Array of interpolation coefficients for xlist,ylist     */
/*----------------------------------------------------------------------- */
void lagrange(ofloat x, ofloat y, olong n, olong hwid, olong minmax[],
	      ofloat *xlist, ofloat *ylist, ofloat *coef)
{
  ofloat xhwid, yhwid, sum;
  odouble prodx, prodxd, prody, prodyd;
  olong  i, j, countx, county;

  for (i=0; i<n; i++) coef[i] = 0.0;  /* Init output */

  /* Anything to do? */
  if ((x<minmax[0]) || (x>minmax[1]) || (y<minmax[2]) || (y>minmax[3])) return;

  sum = 0.0;
  /* chose type, hwid=0 => closest */
  if (hwid<=0) {
    /* Loop over list */
    for (i=0; i<n; i++) {
      if ((fabs(x-xlist[i])<0.2) && (fabs(y-ylist[i])<0.2)) {
	coef[i] = 1.0;
	sum++;
      } else coef[i] = 0.0;
    }
  } else {  /* Interpolate */
    /* Trim halfwidth near edges */
    xhwid = hwid + 0.5;
    xhwid = MIN (xhwid, (x-minmax[0]+0.5));
    xhwid = MIN (xhwid, (minmax[1]-x+0.5));
    yhwid = hwid + 0.5;
    yhwid = MIN (yhwid, (y-minmax[2]+0.5));
    yhwid = MIN (yhwid, (minmax[3]-y+0.5));
    for (j=0; j<n; j++) {
      prodx =  prody  = 1.0;
      prodxd = prodyd = 1.0;
      countx = county = 0;
      coef[j] = 0.0; 
      
      /* Within hwid? and i!=j */
      if ((fabs(x-xlist[j])<=xhwid) && (fabs(y-ylist[j])<=yhwid)) {
	coef[j] = 1.0;  /* In case nothing else within hwid */
	
	/* Inner loop over list */
	for (i=0; i<n; i++) {
	  if (fabs(x-xlist[i])>xhwid) continue;  /* X within halfwidth */
	  if (fabs(y-ylist[i])>yhwid) continue;  /* Y within halfwidth */
	  if (i==j) continue;                    /* i!=j */
	  if (fabs(x-xlist[i])>0.5) {   /*  Not if same X  */
	    countx++;
	    prodx  *= (odouble)(x - xlist[i]);}
	  if (fabs(xlist[j]-xlist[i])>0.5) prodxd *= (odouble)(xlist[j] - xlist[i]);
	  if (fabs(y-ylist[i])>0.5) {   /*   Not if same Y*/
	    county++;
	    prody  *= (odouble)(y - ylist[i]);}
	  if (fabs(ylist[j]-ylist[i])>0.5) prodyd *= (odouble)(ylist[j] - ylist[i]);
	} /* end inner loop */
      } /* end j within half width */
      
      /* put it together */
      if ((countx>=1)  || (county>=1)) {
	if ((prodxd!=0.0) && (prodyd!=0.0))
	  coef[j] = (ofloat)(prodx*prody / (prodxd*prodyd));
	else
	  coef[j] = 1.0;
	sum += coef[j];
      }
    } /* end loop over list */
  } /* end if interpolate */

  /* Normalize if anything found */
  if (sum==0.0) return;
  prodx = 1.0 / sum;
  for (j=0; j<n; j++) coef[j] *= prodx;

   /* DEBUG
  if ((x==0.0) && (y>=-1.0) && (y<=1.0)) {
    fprintf(stdout,"lagrange x %f, y %f, sum %f\n", x, y, sum);
     for (i=0; i<n; i++) 
      if (fabs(coef[i])>0.0001) 
	fprintf(stdout," final %6d, %8.4f %6.1f %6.1f\n",
		i, coef[i], xlist[i], ylist[i]);
  }  end DEBUG */
} /* end lagrange */

/*----------------------------------------------------------------------- */
/*  Gaussian weighted interpolation coefficients                          */
/*  For interpolating in a quasi regular grid represented by lists        */
/*  Determine coefficients for elements in lists to interpolate to (x,y)  */
/*   Input:                                                               */
/*      x         Coordinate on first axis                                */
/*      y         Coordinate on second axis                               */
/*      n         Length of lists                                         */
/*      hwid      Halfwidth of interpolation kernal                       */
/*      minmax    max, min, x, y in lists                                 */
/*      xlist     List of coordinates on  first axis                      */
/*      ylist     List coordinate on second axis                          */
/*   Output:                                                              */
/*      coef      Array of interpolation coefficients for xlist,ylist     */
/*----------------------------------------------------------------------- */
void gauss(ofloat x, ofloat y, olong n, olong hwid, olong minmax[],
	   ofloat *xlist, ofloat *ylist, ofloat *coef)
{
  ofloat xhwid = (ofloat)hwid, sum, d2;
  olong  i;

  for (i=0; i<n; i++) coef[i] = 0.0;  /* Init output */

  /* Anything to do? */
  if ((x<minmax[0]) || (x>minmax[1]) || (y<minmax[2]) || (y>minmax[3])) return;

  sum = 0.0;
  /* hwid=0 => closest */
  if (hwid<=0) {
    for (i=0; i<n; i++) {
      if ((fabs(x-xlist[i])<0.49) && (fabs(y-ylist[i])<0.49)) {
	coef[i] = 1.0;
	sum++;
      } else coef[i] = 0.0;
    }
  } else {  /* Interpolate */
    /* Loop over list */
    for (i=0; i<n; i++) {
      if ((fabs(x-xlist[i])<=xhwid) && (fabs(y-ylist[i])<=xhwid)) {
	/* distance**2 in pixels */
	d2 = (x-xlist[i])*(x-xlist[i]) + (y-ylist[i])*(y-ylist[i]);
	coef[i] = exp(-d2);
	sum += coef[i];
      } /* end close enough */
    } /* end loop */
  }
  /* Normalize if anything found */
  if (fabs(sum)<0.01) return;
  if (sum==1.0) return;
  sum = 1.0 / sum;
  for (i=0; i<n; i++) coef[i] *= sum;
 } /* end gauss */

/**
 * Make arguments for a Threaded ThreadMBFunc?
 * \param thread     ObitThread object to be used
 * \param  nchan     Number of channels in output
 * \param  npoln     Number of polarizations in output
 * \param  nIF       Number of IFs in output
 * \param  npoln     Number of polarizations in output
 * \param  selem     Size (floats) of list element
 * \param  nelem     Number of list elements
 * \param  doRMS     If TRUE, image is RMS
 * \param  doPhase   If TRUE, image is Phase, else Amplitude
 * \param  nx;       Number of columns
 * \param  ny;       Number of rows
 * \param  hwid;     Half width of interpolation
 * \param  xcen;     X center cell
 * \param  ycen;     Y center cell
 * \param  SumIr     Real Stokes I accumulation list
 * \param  SumIi     Imag Stokes I accumulation list
 * \param  SumII     Stokes I*I accumulation list
 * \param  SumQr     Real Stokes Q accumulation list
 * \param  SumQi     Imag Stokes Q accumulation list
 * \param  SumQQ     Stokes Q*Q accumulation list
 * \param  SumUr     Real Stokes U accumulation list
 * \param  SumUi     Imag Stokes U accumulation list
 * \param  SumUU     Stokes U*U accumulation list
 * \param  SumVr     Real Stokes V accumulation list
 * \param  SumVi     Imag Stokes V accumulation list
 * \param  SumVV     Stokes V*V accumulation list
 * \param  SumAzCell Azimuth offset accumulation list
 * \param  SumElCell Elevation offset  accumulation list
 * \param  grids     [out] Array of ObitFArrays
 *                   fastest to slowest, channel, IF, Stokes
 * \param ThreadArgs[out] Created array of MBFuncArg, 
 *                   delete with KillMBFuncArgs
 * \return number of elements in args (number of allowed threads).
 */
static olong MakeMBFuncArgs (ObitThread *thread, 
			     olong nchan, olong nIF, olong npoln,
			     olong selem, olong nelem, gboolean doRMS, gboolean doPhase,
			     olong nx, olong ny, olong hwid, ofloat xcen, ofloat ycen,
			     ofloat *SumIr, ofloat *SumIi, ofloat *SumII, 
			     ofloat *SumQr, ofloat *SumQi, ofloat *SumQQ, 
			     ofloat *SumUr, ofloat *SumUi, ofloat *SumUU, 
			     ofloat *SumVr, ofloat *SumVi, ofloat *SumVV, 
			     ofloat *SumAzCell, ofloat *SumElCell, ObitFArray **grids,
			     MBFuncArg ***ThreadArgs)

{
  olong i, iy, nThreads, nrow;

  /* Setup for threading */
  /* How many threads? */
  nThreads = MAX (1, ObitThreadNumProc(thread));

  /* How many rows per thread? */
  nrow = ny/nThreads;
  iy   = 0;

  /* Initialize threadArg array */
  *ThreadArgs = g_malloc0(nThreads*sizeof(MBFuncArg*));
  for (i=0; i<nThreads; i++) 
    (*ThreadArgs)[i] = g_malloc0(sizeof(MBFuncArg)); 
  for (i=0; i<nThreads; i++) {
    (*ThreadArgs)[i]->thread    = ObitThreadRef(thread);
    (*ThreadArgs)[i]->loy       = iy;     /* Divvy up processing */
    (*ThreadArgs)[i]->hiy       = MIN (iy+nrow, ny);
    iy += nrow;
    (*ThreadArgs)[i]->nchan     = nchan;
    (*ThreadArgs)[i]->npoln     = npoln;
    (*ThreadArgs)[i]->nIF       = nIF;
    (*ThreadArgs)[i]->npoln     = npoln;
    (*ThreadArgs)[i]->selem     = selem;
    (*ThreadArgs)[i]->nelem     = nelem;
    (*ThreadArgs)[i]->doRMS     = doRMS;
    (*ThreadArgs)[i]->doPhase   = doPhase;
    (*ThreadArgs)[i]->nx        = nx;
    (*ThreadArgs)[i]->ny        = ny;
    (*ThreadArgs)[i]->hwid      = hwid;
    (*ThreadArgs)[i]->xcen      = xcen;
    (*ThreadArgs)[i]->ycen      = ycen;
    (*ThreadArgs)[i]->SumIr     = SumIr;
    (*ThreadArgs)[i]->SumIi     = SumIi;
    (*ThreadArgs)[i]->SumII     = SumII;
    (*ThreadArgs)[i]->SumQr     = SumQr;
    (*ThreadArgs)[i]->SumQi     = SumQi;
    (*ThreadArgs)[i]->SumQQ     = SumQQ;
    (*ThreadArgs)[i]->SumUr     = SumUr;
    (*ThreadArgs)[i]->SumUi     = SumUi;
    (*ThreadArgs)[i]->SumUU     = SumUU;
    (*ThreadArgs)[i]->SumVr     = SumVr;
    (*ThreadArgs)[i]->SumVi     = SumVi;
    (*ThreadArgs)[i]->SumVV     = SumVV;
    (*ThreadArgs)[i]->SumAzCell = SumAzCell;
    (*ThreadArgs)[i]->SumElCell = SumElCell;
    (*ThreadArgs)[i]->grids     = grids;
    (*ThreadArgs)[i]->coef      = g_malloc0(nelem*sizeof(ofloat));
    (*ThreadArgs)[i]->ithread   = i;
  }

  return nThreads;
} /*  end MakeMBFuncArgs */

/**
 * Delete arguments for ThreadMBFunc
 * \param nargs      number of elements in ThreadArgs.
 * \param ThreadArgs Array of MBFuncArg
 */
static void KillMBFuncArgs (olong nargs, MBFuncArg **ThreadArgs)
{
  olong i;

  if (ThreadArgs==NULL) return;
  ObitThreadPoolFree (ThreadArgs[0]->thread);  /* Free thread pool */
  for (i=0; i<nargs; i++) {
    if (ThreadArgs[i]) {
      if (ThreadArgs[i]->thread) ObitThreadUnref(ThreadArgs[i]->thread);
      if (ThreadArgs[i]->coef)   g_free(ThreadArgs[i]->coef);
      g_free(ThreadArgs[i]);
    }
  }
  g_free(ThreadArgs);
} /*  end KillMBFuncArgs */

/**
 * Thread grid a set of rows in the image
 * Magic value blanking supported.
 * Callable as thread
 * \param arg Pointer to MBFuncArg argument with elements:
 * \li  loy       First row (0-rel) 
 * \li  hiy       Highest row (0-rel) 
 * \li  nchan     Number of channels in output
 * \li  npoln     Number of polarizations in output
 * \li  nIF       Number of IFs in output
 * \li  npoln     Number of polarizations in output
 * \li  selem     Size (floats) of list element
 * \li  nelem     Number of list elements
 * \li  doRMS     If TRUE, image is RMS
 * \li  doPhase   If TRUE, image is Phase, else Amplitude
 * \li  nx;       Number of columns
 * \li  ny;       Number of rows
 * \li  hwid;     Half width of interpolation
 * \li  xcen;     X center cell
 * \li  ycen;     Y center cell
 * \li  SumIr     Real Stokes I accumulation list
 * \li  SumIi     Imag Stokes I accumulation list
 * \li  SumII     Stokes I*I accumulation list
 * \li  SumQr     Real Stokes Q accumulation list
 * \li  SumQi     Imag Stokes Q accumulation list
 * \li  SumQQ     Stokes Q*Q accumulation list
 * \li  SumUr     Real Stokes U accumulation list
 * \li  SumUi     Imag Stokes U accumulation list
 * \li  SumUU     Stokes U*U accumulation list
 * \li  SumVr     Real Stokes V accumulation list
 * \li  SumVi     Imag Stokes V accumulation list
 * \li  SumVV     Stokes V*V accumulation list
 * \li  SumAzCell Azimuth offset accumulation list
 * \li  SumElCell Elevation offset  accumulation list
 * \li  grids     [out] Array of ObitFArrays
 *                   fastest to slowest, channel, IF, Stokes
 * \li ithread  thread number, <0 -> no threading
 * \return NULL
 */
static gpointer ThreadMBGrid (gpointer arg)
{
  /* Get arguments from structure */
  MBFuncArg *largs = (MBFuncArg*)arg;
  olong   loy        = largs->loy;
  olong   hiy        = largs->hiy;
  olong   nIF        = largs->nIF;
  olong   nchan      = largs->nchan;
  olong   npoln      = largs->npoln;
  olong   selem      = largs->selem;
  olong   nelem      = largs->nelem;
  gboolean doRMS     = largs->doRMS;
  gboolean doPhase   = largs->doPhase;
  olong  nx          = largs->nx; 
  /*olong  ny          = largs->ny;*/
  olong  hwid        = largs->hwid;
  ofloat xcen        = largs->xcen;  
  ofloat ycen        = largs->ycen;  
  ofloat* SumIr      = largs->SumIr;
  ofloat* SumIi      = largs->SumIi;
  ofloat* SumII      = largs->SumII;
  ofloat* SumQr      = largs->SumQr;
  ofloat* SumQi      = largs->SumQi;
  ofloat* SumQQ      = largs->SumQQ;
  ofloat* SumUr      = largs->SumUr;
  ofloat* SumUi      = largs->SumUi;
  ofloat* SumUU      = largs->SumUU;
  ofloat* SumVr      = largs->SumVr;
  ofloat* SumVi      = largs->SumVi;
  ofloat* SumVV      = largs->SumVV;
  ofloat* SumAzCell  = largs->SumAzCell;
  ofloat* SumElCell  = largs->SumElCell;
  ofloat* coef       = largs->coef;
  ObitFArray** grids = largs->grids;
  
  /* local */
  olong i, ix, iy, iIF, ichan, indx, jndx, off;
  ofloat x, y, closest;
  ofloat amp, ph, fblank = ObitMagicF();
  odouble sumIWt , sumQWt, sumUWt, sumVWt;
  odouble valIr,  valIi, valII, valQr, valQi, valQQ;
  odouble valUr,  valUi, valUU, valVr, valVi, valVV;
  ObitFArray *array;
  olong minmax[] = {1000,-1000,1000,-1000};

  if (hiy<loy) goto finish;

  /* Get min, max x, y from lists */
  for (i=0; i<nelem; i++ ) {
    if (SumAzCell[i]>0.0) ix = (olong)(SumAzCell[i]+0.5);
    else                  ix = (olong)(SumAzCell[i]-0.5);
    if (SumElCell[i]>0.0) iy = (olong)(SumElCell[i]+0.5);
    else                  iy = (olong)(SumElCell[i]-0.5);
    minmax[0] = MIN (minmax[0], ix);
    minmax[1] = MAX (minmax[1], ix);
    minmax[2] = MIN (minmax[2], iy);
    minmax[3] = MAX (minmax[3], iy);
  }
  
  /* Loop over y (El) */
  for (iy=loy; iy<hiy; iy++) {
    y = iy - ycen;
    
    /* Loop over x (Az) */
    for (ix=0; ix<nx; ix++) {
      x = ix - xcen;
      
      /* Get interpolation coefficients */
      lagrange (x, y, nelem, hwid, minmax, SumAzCell, SumElCell, coef);
	/* gauss (x, y, nelem, hwid, minmax, SumAzCell, SumElCell, coef);*/

      /* Loop over IFs */
      for (iIF=0; iIF<nIF; iIF++) {
	/* Loop over chans */
	for (ichan=0; ichan<nchan; ichan++) {
	  valIr = valII = valQr = valQQ = valUr = valUU = valVr = valVV = 0.0;
	  valIi = valQi = valUi = valVi = 0.0;
	  sumIWt = sumQWt = sumUWt = sumVWt = 0.0;
	  off = ichan + iIF*nchan;
	  closest = 1000.0; /* Closest element */
	  /* Loop over lists summing */
	  for (i=0; i<nelem; i++) {
	    closest = MIN (closest, MAX (fabs(SumAzCell[i]-x), fabs(SumElCell[i]-y)));
	    if (coef[i]!=0.0) {
	      if (SumIr[i*selem+off]!=fblank) {
		valIr  += coef[i]*SumIr[i*selem+off];
		valIi  += coef[i]*SumIi[i*selem+off];
		valII  += coef[i]*SumII[i*selem+off];
		sumIWt += coef[i];
		/* DEBUG
		if ((x==0.0) && (y>=-1.0) && (y<=1.0) && (iIF==11) && (ichan==2) && (fabs(coef[i])>0.0001)) {
		  amp = sqrt (valIr*valIr + valIi*valIi);
		  fprintf(stdout,"grid %d x %4.1f, y %4.1f, sum I %8.4f sumWt %8.4f I %8.4f, %8.4f coef %8.4f total %4.4f amp %4.4f \n", 
			    i, x, y, valIr, sumIWt, SumIr[i*selem+off]/SumIr[1624*selem+off], 
			  SumIi[i*selem+off]/SumIr[1624*selem+off], coef[i], valIr/sumIWt, amp);
		  }  end DEBUG */
	      }
	      if (SumQr && SumQr[i*selem+off]!=fblank) {
		valQr  += coef[i]*SumQr[i*selem+off];
		valQi  += coef[i]*SumQi[i*selem+off];
		valQQ  += coef[i]*SumQQ[i*selem+off];
		sumQWt += coef[i];
	      }
	      if (SumUr && SumUr[i*selem+off]!=fblank) {
		valUr += coef[i]*SumUr[i*selem+off];
		valUi += coef[i]*SumUi[i*selem+off];
		valUU += coef[i]*SumUU[i*selem+off];
		sumUWt += coef[i];
	      }
	      if (SumVr && SumVr[i*selem+off]!=fblank) {
		valVr += coef[i]*SumVr[i*selem+off];
		valVi += coef[i]*SumVi[i*selem+off];
		valVV += coef[i]*SumVV[i*selem+off];
		sumVWt += coef[i];
	      }
	    }
	  } /* end loop over lists */
	  /* Better be something within 0.5 cells */
	  if (closest>0.5) {
	    sumIWt = sumQWt = sumUWt = sumVWt = 0.0;
	  }
	  if (fabs(sumIWt)>0.1) {
	    valIr /= sumIWt;
	    valIi /= sumIWt;
	    valII /= sumIWt;
	  } else {
	    valIr = fblank;
	    valIi = fblank;
	    valII = fblank;
	  }
	  if (fabs(sumQWt)>0.1) {
	    valQr /= sumQWt;
	    valQi /= sumQWt;
	    valQQ /= sumQWt;
	  } else {
	    valQr  = fblank;
	    valQi  = fblank;
	    valQQ = fblank;
	  }
	  if (fabs(sumUWt)>0.1) {
	    valUr /= sumUWt;
	    valUi /= sumUWt;
	    valUU /= sumUWt;
	  } else {
	    valUr = fblank;
	    valUi = fblank;
	    valUU = fblank;
	  }
	  if (fabs(sumVWt)>0.1) {
	    valVr /= sumVWt;
	    valVi /= sumVWt;
	    valVV /= sumVWt;
	  } else {
	    valVr = fblank;
	    valVi = fblank;
	    valVV = fblank;
	  }
	  
	  /* Insert into FArrays */
	  jndx  = iy*nx + ix;
	  /* I */
	  indx  = ichan + iIF*nchan;
	  array = grids[indx];
	  if (array) {   /* Needed? */
	    if ((valIr==fblank) || (valIi==fblank)) {
	      amp = ph = fblank;
	    } else {
	      /* Amplitude and phase within +/- 90 deg */
	      amp = sqrt (valIr*valIr + valIi*valIi);
	      ph  = RAD2DG*atan2(valIi, valIr);
	      if (ph>90.0) {
		amp = -amp;
		ph -= 180.0;
	      } else if (ph<-90.0) {
		amp = -amp;
		ph += 180.0;
	      }
	    }
	    /* DEBUG 
	    if ((ix==50) && (iy>=49) && (iy<=51) && (iIF==11) && (ichan==2)) {
	      fprintf(stdout,"grid %d x %4d, y %4d, amp %8.4f phase %8.4f RMS %8.4f \n", 
		      i, ix, iy, amp, ph, valII);
	    }  end DEBUG */
	    if (doRMS) {  /* RMS */
	      array->array[jndx] = valII;
	    } else if (doPhase) {  /* phase */
	      array->array[jndx] = ph;
	    } else {      /* Stokes ampl */
	      array->array[jndx] = amp;
	    }
	  } /* end if Stokes needed */
	  if (npoln<=1) continue;   /* multiple polarizations */
	  /* Q */
	  indx  = ichan + iIF*nchan + nchan*nIF;
	  array = grids[indx];
	  if (array && (npoln>=2)) {   /* Needed? */
	    if ((valQr==fblank) || (valQi==fblank)) {
	      amp = ph = fblank;
	    } else {
	      /* Amplitude and phase within +/- 90 deg */
	      amp = sqrt (valQr*valQr + valQi*valQi);
	      ph  = RAD2DG*atan2(valQi, valQr);
	      if (ph>90.0) {
		amp = -amp;
		ph -= 180.0;
	      } else if (ph<-90.0) {
		amp = -amp;
		ph += 180.0;
	      }
	    }
	    if (doRMS) {  /* RMS */
	      array->array[jndx] = valQQ;
	    } else if (doPhase) {  /* phase */
	      array->array[jndx] = ph;
	    } else {      /* Stokes ampl */
	      array->array[jndx] = amp;
	    }
	  } /* end if Stokes needed */
	  /* U */
	  indx  = ichan + iIF*nchan + 2*nchan*nIF;
	  array = grids[indx];
	  if (array && (npoln>=3)) {   /* Needed? */
	    if ((valUr==fblank) || (valUi==fblank)) {
	      amp = ph = fblank;
	    } else {
	      /* Amplitude and phase within +/- 90 deg */
	      amp = sqrt (valUr*valUr + valUi*valUi);
	      ph  = RAD2DG*atan2(valUi, valUr);
	      if (ph>90.0) {
		amp = -amp;
		ph -= 180.0;
	      } else if (ph<-90.0) {
		amp = -amp;
		ph += 180.0;
	      }
	    }
	    if (doRMS) {  /* RMS */
	      array->array[jndx] = valUU;
	    } else if (doPhase) {  /* phase */
	      array->array[jndx] = ph;
	    } else {      /* Stokes ampl */
	      array->array[jndx] = amp;
	    }
	  } /* end if Stokes needed */
	  /* V */
	  indx  = ichan + iIF*nchan + 3*nchan*nIF;
	  array = grids[indx];
	  if (array && (npoln>=4)) {   /* Needed? */
	    if ((valVr==fblank) || (valVi==fblank)) {
	      amp = ph = fblank;
	    } else {
	      /* Amplitude and phase within +/- 90 deg */
	      amp = sqrt (valVr*valVr + valVi*valVi);
	      ph  = RAD2DG*atan2(valVi, valVr);
	      if (ph>90.0) {
		amp = -amp;
		ph -= 180.0;
	      } else if (ph<-90.0) {
		amp = -amp;
		ph += 180.0;
	      }
	    }
	    if (doRMS) {  /* RMS */
	      array->array[jndx] = valVV;
	    } else if (doPhase) {  /* phase */
	      array->array[jndx] = ph;
	    } else {      /* Stokes ampl */
	      array->array[jndx] = amp;
	    }
	  } /* end if Stokes */
	} /* end channel Loop */
      } /* end IF Loop */
    } /* end x loop */
  } /* end y loop */

  /* Indicate completion */
  finish: 
  if (largs->ithread>=0)
    ObitThreadPoolDone (largs->thread, (gpointer)&largs->ithread);
  
  return NULL;
  
} /*  end ThreadMBGrid */
