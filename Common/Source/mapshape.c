/*
** This code is entirely based on the previous work of Frank Warmerdam. It is
** essentially shapelib 1.1.5. However, there were enough changes that it was
** incorporated into the MapServer source to avoid confusion. See the README 
** for licence details.
*/
/*
Copyright_License {

  XCSoar Glide Computer - http://xcsoar.sourceforge.net/
  Copyright (C) 2000 - 2005  

  	M Roberts (original release)
	Robin Birch <robinb@ruffnready.co.uk>
	Samuel Gisiger <samuel.gisiger@triadis.ch>
	Jeff Goodenough <jeff@enborne.f2s.com>
	Alastair Harrison <aharrison@magic.force9.co.uk>
	Scott Penrose <scottp@dd.com.au>
	John Wharington <jwharington@bigfoot.com>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

}
*/

#include "StdAfx.h"

#include "mapshape.h"
#include "maperror.h"
#include "maptree.h"

#include <limits.h>

#include "utils/heapcheck.h"

#if UINT_MAX == 65535
typedef long          int32;
#else
typedef int           int32;
#endif

#define ByteCopy( a, b, c )     memcpy( b, a, c )

static int      bBigEndian;

/************************************************************************/
/*                              SwapWord()                              */
/*                                                                      */
/*      Swap a 2, 4 or 8 byte word.                                     */
/************************************************************************/
static void SwapWord( int length, void * wordP )
{
  int i;
  uchar	temp;
  
  for( i=0; i < length/2; i++ )
    {
      temp = ((uchar *) wordP)[i];
      ((uchar *)wordP)[i] = ((uchar *) wordP)[length-i-1];
      ((uchar *) wordP)[length-i-1] = temp;
    }
}

/************************************************************************/
/*                             SfRealloc()                              */
/*                                                                      */
/*      A realloc cover function that will access a NULL pointer as     */
/*      a valid input.                                                  */
/************************************************************************/
void * SfRealloc( void * pMem, int nNewSize )     
{
  if( pMem == NULL )
    return( (void *) malloc(nNewSize) );
  else
    return( (void *) realloc(pMem,nNewSize) );
}

/************************************************************************/
/*                          writeHeader()                               */
/*                                                                      */
/*      Write out a header for the .shp and .shx files as well as the	*/
/*	contents of the index (.shx) file.				*/
/************************************************************************/
static void writeHeader( SHPHandle psSHP )
{
  uchar     	abyHeader[100];
  int		i;
  int32	i32;
  double	dValue;
  int32	*panSHX;
  
  /* -------------------------------------------------------------------- */
  /*      Prepare header block for .shp file.                             */
  /* -------------------------------------------------------------------- */
  for( i = 0; i < 100; i++ )
    abyHeader[i] = 0;
  
  abyHeader[2] = 0x27;				/* magic cookie */
  abyHeader[3] = 0x0a;
  
  i32 = psSHP->nFileSize/2;				/* file size */
  ByteCopy( &i32, abyHeader+24, 4 );
  if( !bBigEndian ) SwapWord( 4, abyHeader+24 );
  
  i32 = 1000;						/* version */
  ByteCopy( &i32, abyHeader+28, 4 );
  if( bBigEndian ) SwapWord( 4, abyHeader+28 );
  
  i32 = psSHP->nShapeType;				/* shape type */
  ByteCopy( &i32, abyHeader+32, 4 );
  if( bBigEndian ) SwapWord( 4, abyHeader+32 );
  
  dValue = psSHP->adBoundsMin[0];			/* set bounds */
  ByteCopy( &dValue, abyHeader+36, 8 );
  if( bBigEndian ) SwapWord( 8, abyHeader+36 );
  
  dValue = psSHP->adBoundsMin[1];
  ByteCopy( &dValue, abyHeader+44, 8 );
  if( bBigEndian ) SwapWord( 8, abyHeader+44 );
  
  dValue = psSHP->adBoundsMax[0];
  ByteCopy( &dValue, abyHeader+52, 8 );
  if( bBigEndian ) SwapWord( 8, abyHeader+52 );
  
  dValue = psSHP->adBoundsMax[1];
  ByteCopy( &dValue, abyHeader+60, 8 );
  if( bBigEndian ) SwapWord( 8, abyHeader+60 );
  
  dValue = psSHP->adBoundsMin[3];			/* m */
  ByteCopy( &dValue, abyHeader+84, 8 );
  if( bBigEndian ) SwapWord( 8, abyHeader+84 );

  dValue = psSHP->adBoundsMax[3];
  ByteCopy( &dValue, abyHeader+92, 8 );
  if( bBigEndian ) SwapWord( 8, abyHeader+92 );

  /* -------------------------------------------------------------------- */
  /*      Write .shp file header.                                         */
  /* -------------------------------------------------------------------- */
  if (psSHP->fpSHP) {
    fseek( psSHP->fpSHP, 0, 0 );
    fwrite( abyHeader, 100, 1, psSHP->fpSHP );
  } else if (psSHP->zfpSHP) {
    if (zzip_file_real(psSHP->zfpSHP)) {
      zzip_seek(psSHP->zfpSHP, 0, 0);
      write (zzip_realfd (psSHP->zfpSHP), abyHeader, 100);
    }
  }
  
  /* -------------------------------------------------------------------- */
  /*      Prepare, and write .shx file header.                            */
  /* -------------------------------------------------------------------- */
  i32 = (psSHP->nRecords * 2 * sizeof(int32) + 100)/2;   /* file size */
  ByteCopy( &i32, abyHeader+24, 4 );
  if( !bBigEndian ) SwapWord( 4, abyHeader+24 );
  
  if (psSHP->fpSHX) {
    fseek( psSHP->fpSHX, 0, 0 );
    fwrite( abyHeader, 100, 1, psSHP->fpSHX );
  } else if (psSHP->zfpSHX) {
    if (zzip_file_real(psSHP->zfpSHX)) {
      zzip_seek(psSHP->zfpSHX, 0, 0);
      write (zzip_realfd (psSHP->zfpSHX), abyHeader, 100);
    }
  }
  
  /* -------------------------------------------------------------------- */
  /*      Write out the .shx contents.                                    */
  /* -------------------------------------------------------------------- */
  panSHX = (int32 *) malloc(sizeof(int32) * 2 * psSHP->nRecords);
  
  for( i = 0; i < psSHP->nRecords; i++ ) {
    panSHX[i*2  ] = psSHP->panRecOffset[i]/2;
    panSHX[i*2+1] = psSHP->panRecSize[i]/2;
    if( !bBigEndian ) SwapWord( 4, panSHX+i*2 );
    if( !bBigEndian ) SwapWord( 4, panSHX+i*2+1 );
  }
  
  if (psSHP->fpSHX) {
    fwrite( panSHX, sizeof(int32) * 2, psSHP->nRecords, psSHP->fpSHX );
  } else if (psSHP->zfpSHX) {
    if (zzip_file_real(psSHP->zfpSHX)) {
      write (zzip_realfd (psSHP->zfpSHX), panSHX, psSHP->nRecords*sizeof(int32)*2);
    }
  }
  
  free( panSHX );
}

/************************************************************************/
/*                              msSHPOpen()                             */
/*                                                                      */
/*      Open the .shp and .shx files based on the basename of the       */
/*      files or either file name.                                      */
/************************************************************************/   
SHPHandle msSHPOpen( const TCHAR * pszLayer, const char * pszAccess )
{
  TCHAR     *pszFullname, *pszBasename;
  SHPHandle psSHP;
  
  uchar		*pabyBuf;
  int		i;
  double	dValue;
  
  /* -------------------------------------------------------------------- */
  /*      Ensure the access string is one of the legal ones.  We          */
  /*      ensure the result string indicates binary to avoid common       */
  /*      problems on Windows.                                            */
  /* -------------------------------------------------------------------- */
  if( strcmp(pszAccess,"rb+") == 0 
      || strcmp(pszAccess,"r+b") == 0 
      || strcmp(pszAccess,"r+") == 0 )
    pszAccess = "r+b";
  else
    pszAccess = "rb";
  
  /* -------------------------------------------------------------------- */
  /*	Establish the byte order on this machine.			    */
  /* -------------------------------------------------------------------- */
  i = 1;
  if( *((uchar *) &i) == 1 )
    bBigEndian = MS_FALSE;
  else
    bBigEndian = MS_TRUE;
  
  /* -------------------------------------------------------------------- */
  /*	Initialize the info structure.					    */
  /* -------------------------------------------------------------------- */
  psSHP = (SHPHandle) malloc(sizeof(SHPInfo));
  
  psSHP->bUpdated = MS_FALSE;

  psSHP->pabyRec = NULL;
  psSHP->panParts = NULL;
  psSHP->nBufSize = psSHP->nPartMax = 0;

  /* -------------------------------------------------------------------- */
  /*	Compute the base (layer) name.  If there is any extension	    */
  /*	on the passed in filename we will strip it off.			    */
  /* -------------------------------------------------------------------- */
  pszBasename = (TCHAR *) malloc((_tcslen(pszLayer)+5) * sizeof(TCHAR) * 2);
  _tcscpy( pszBasename, pszLayer );
  for( i = _tcslen(pszBasename)-1; 
       i > 0 && pszBasename[i] != _T('.') && pszBasename[i] != _T('/') && pszBasename[i] != _T('\\');
       i-- ) {}
  
  if( pszBasename[i] == _T('.') )
    pszBasename[i] = _T('\0');
  
  /* -------------------------------------------------------------------- */
  /*	Open the .shp and .shx files.  Note that files pulled from	    */
  /*	a PC to Unix with upper case filenames won't work!		    */
  /* -------------------------------------------------------------------- */
  pszFullname = (TCHAR *) malloc((_tcslen(pszBasename)+5) * sizeof(TCHAR) * 2);
  _stprintf( pszFullname, _T("%s.shp"), pszBasename );
  psSHP->zfpSHP = ppc_fopen(pszFullname, pszAccess );
  psSHP->fpSHP = NULL;
  if( psSHP->zfpSHP == NULL ) {
    free(psSHP);
    free(pszFullname);
    return( NULL );
  }
  
  _stprintf( pszFullname, _T("%s.shx"), pszBasename );
  psSHP->zfpSHX = ppc_fopen(pszFullname, pszAccess );
  psSHP->fpSHX = NULL;
  if( psSHP->zfpSHX == NULL ) {
    free(psSHP);
    free(pszFullname);
    return( NULL );
  }
  
  free( pszFullname );
  free( pszBasename ); 

  
  /* -------------------------------------------------------------------- */
  /*   Read the file size from the SHP file.				    */
  /* -------------------------------------------------------------------- */
  pabyBuf = (uchar *) malloc(100);
  if (pabyBuf == 0) { 
    free(psSHP);
    return NULL;
  }
  if (zzip_fread( pabyBuf, 1, 100, psSHP->zfpSHP )<100) {
    free(psSHP);
    return NULL;
  }
  
  psSHP->nFileSize = (pabyBuf[24] * 256 * 256 * 256
		      + pabyBuf[25] * 256 * 256
		      + pabyBuf[26] * 256
		      + pabyBuf[27]) * 2;
  
  /* -------------------------------------------------------------------- */
  /*  Read SHX file Header info                                           */
  /* -------------------------------------------------------------------- */
  zzip_fread( pabyBuf, 100, 1, psSHP->zfpSHX );
  
  if( pabyBuf[0] != 0 
      || pabyBuf[1] != 0 
      || pabyBuf[2] != 0x27 
      || (pabyBuf[3] != 0x0a && pabyBuf[3] != 0x0d) )
    {
      zzip_fclose( psSHP->zfpSHP );
      zzip_fclose( psSHP->zfpSHX );
      free( psSHP );
      
      return( NULL );
    }
  
  psSHP->nRecords = pabyBuf[27] + pabyBuf[26] * 256
    + pabyBuf[25] * 256 * 256 + pabyBuf[24] * 256 * 256 * 256;
  psSHP->nRecords = (psSHP->nRecords*2 - 100) / 8;
  
  psSHP->nShapeType = pabyBuf[32];
  
  if( bBigEndian ) SwapWord( 8, pabyBuf+36 );
  memcpy( &dValue, pabyBuf+36, 8 );
  psSHP->adBoundsMin[0] = dValue;
  
  if( bBigEndian ) SwapWord( 8, pabyBuf+44 );
  memcpy( &dValue, pabyBuf+44, 8 );
  psSHP->adBoundsMin[1] = dValue;
  
  if( bBigEndian ) SwapWord( 8, pabyBuf+52 );
  memcpy( &dValue, pabyBuf+52, 8 );
  psSHP->adBoundsMax[0] = dValue;
  
  if( bBigEndian ) SwapWord( 8, pabyBuf+60 );
  memcpy( &dValue, pabyBuf+60, 8 );
  psSHP->adBoundsMax[1] = dValue;
  
  if( bBigEndian ) SwapWord( 8, pabyBuf+84 );		/* m */
  memcpy( &dValue, pabyBuf+84, 8 );
  psSHP->adBoundsMin[3] = dValue;

  if( bBigEndian ) SwapWord( 8, pabyBuf+92 );
  memcpy( &dValue, pabyBuf+92, 8 );
  psSHP->adBoundsMax[3] = dValue;
  free( pabyBuf );
  
  /* -------------------------------------------------------------------- */
  /*	Read the .shx file to get the offsets to each record in 	    */
  /*	the .shp file.							    */
  /* -------------------------------------------------------------------- */
  psSHP->nMaxRecords = psSHP->nRecords;
  
  psSHP->panRecOffset = (int *) malloc(sizeof(int) * (psSHP->nMaxRecords) );
  psSHP->panRecSize = (int *) malloc(sizeof(int) * (psSHP->nMaxRecords) );
  
  pabyBuf = (uchar *) malloc(8 * psSHP->nRecords );
  zzip_fread( pabyBuf, 8, psSHP->nRecords, psSHP->zfpSHX );
  
  for( i = 0; i < psSHP->nRecords; i++ ) {
    int32 nOffset, nLength;
    
    memcpy( &nOffset, pabyBuf + i * 8, 4 );
    if( !bBigEndian ) SwapWord( 4, &nOffset );
    
    memcpy( &nLength, pabyBuf + i * 8 + 4, 4 );
    if( !bBigEndian ) SwapWord( 4, &nLength );
    
    psSHP->panRecOffset[i] = nOffset*2;
    psSHP->panRecSize[i] = nLength*2;
  }
  free( pabyBuf );
  
  return( psSHP );
}

/************************************************************************/
/*                              msSHPClose()                            */
/*								       	*/
/*	Close the .shp and .shx files.					*/
/************************************************************************/
void msSHPClose(SHPHandle psSHP )
{
  /* -------------------------------------------------------------------- */
  /*	Update the header if we have modified anything.		    	  */
  /* -------------------------------------------------------------------- */
  if( psSHP->bUpdated)
    writeHeader( psSHP );
  // JMW, only for write files!
  
  /* -------------------------------------------------------------------- */
  /*      Free all resources, and close files.                            */
  /* -------------------------------------------------------------------- */
  free( psSHP->panRecOffset );
  free( psSHP->panRecSize );
  
  if(psSHP->pabyRec) free(psSHP->pabyRec);
  if(psSHP->panParts) free(psSHP->panParts);

  if (psSHP->zfpSHX) 
    zzip_fclose( psSHP->zfpSHX );
  if (psSHP->zfpSHP)
    zzip_fclose( psSHP->zfpSHP );
  if (psSHP->fpSHX)
    fclose( psSHP->fpSHX );
  if (psSHP->fpSHP)
    fclose( psSHP->fpSHP );
  
  free( psSHP );
}

/************************************************************************/
/*                             msSHPGetInfo()                           */
/*                                                                      */
/*      Fetch general information about the shape file.                 */
/************************************************************************/
void msSHPGetInfo(SHPHandle psSHP, int * pnEntities, int * pnShapeType )
{
  if( pnEntities )
    *pnEntities = psSHP->nRecords;
  
  if( pnShapeType )
    *pnShapeType = psSHP->nShapeType;
}

/************************************************************************/
/*                             msSHPCreate()                            */
/*                                                                      */
/*      Create a new shape file and return a handle to the open         */
/*      shape file with read/write access.                              */
/************************************************************************/
SHPHandle msSHPCreate( const TCHAR * pszLayer, int nShapeType )
{
  TCHAR *pszBasename, *pszFullname;
  int		i;
  FILE	*fpSHP, *fpSHX;
  uchar     	abyHeader[100];
  int32	i32;
  double	dValue;
  
  /* -------------------------------------------------------------------- */
  /*      Establish the byte order on this system.                        */
  /* -------------------------------------------------------------------- */
  i = 1;
  if( *((uchar *) &i) == 1 )
    bBigEndian = MS_FALSE;
  else
    bBigEndian = MS_TRUE;
  
  /* -------------------------------------------------------------------- */
  /*	Compute the base (layer) name.  If there is any extension  	    */
  /*	on the passed in filename we will strip it off.			    */
  /* -------------------------------------------------------------------- */
  pszBasename = (TCHAR *) malloc((_tcslen(pszLayer) + 5) * sizeof(TCHAR) * 2);
  _tcscpy( pszBasename, pszLayer );
  for( i = _tcslen(pszBasename)-1; 
       i > 0 && pszBasename[i] != _T('.') && pszBasename[i] != _T('/') && pszBasename[i] != _T('\\');
       i-- ) {}
  
  if( pszBasename[i] == _T('.') )
    pszBasename[i] = _T('\0');
  
  /* -------------------------------------------------------------------- */
  /*      Open the two files so we can write their headers.               */
  /* -------------------------------------------------------------------- */
  pszFullname = (TCHAR *) malloc((_tcslen(pszBasename) + 5) * sizeof(TCHAR) * 2);
  _stprintf( pszFullname, _T("%s.shp"), pszBasename );
  fpSHP = _tfopen(pszFullname, _T("wb") );
  _stprintf( pszFullname, _T("%s.shx"), pszBasename );

  free( pszBasename );

  if( fpSHP == NULL ) 
    return( NULL );
  fpSHX = _tfopen(pszFullname, _T("wb") );
  if( fpSHX == NULL )
    return( NULL );
  
  free( pszFullname );
  
  /* -------------------------------------------------------------------- */
  /*      Prepare header block for .shp file.                             */
  /* -------------------------------------------------------------------- */
  for( i = 0; i < 100; i++ )
    abyHeader[i] = 0;
  
  abyHeader[2] = 0x27;				/* magic cookie */
  abyHeader[3] = 0x0a;
  
  i32 = 50;						/* file size */
  ByteCopy( &i32, abyHeader+24, 4 );
  if( !bBigEndian ) SwapWord( 4, abyHeader+24 );
  
  i32 = 1000;						/* version */
  ByteCopy( &i32, abyHeader+28, 4 );
  if( bBigEndian ) SwapWord( 4, abyHeader+28 );
  
  i32 = nShapeType;					/* shape type */
  ByteCopy( &i32, abyHeader+32, 4 );
  if( bBigEndian ) SwapWord( 4, abyHeader+32 );
  
  dValue = 0.0;					/* set bounds */
  ByteCopy( &dValue, abyHeader+36, 8 );
  ByteCopy( &dValue, abyHeader+44, 8 );
  ByteCopy( &dValue, abyHeader+52, 8 );
  ByteCopy( &dValue, abyHeader+60, 8 );
  
  /* -------------------------------------------------------------------- */
  /*      Write .shp file header.                                         */
  /* -------------------------------------------------------------------- */
  fwrite( abyHeader, 100, 1, fpSHP );
  
  /* -------------------------------------------------------------------- */
  /*      Prepare, and write .shx file header.                            */
  /* -------------------------------------------------------------------- */
  i32 = 50;						/* file size */
  ByteCopy( &i32, abyHeader+24, 4 );
  if( !bBigEndian ) SwapWord( 4, abyHeader+24 );
  
  fwrite( abyHeader, 100, 1, fpSHX );
  
  /* -------------------------------------------------------------------- */
  /*      Close the files, and then open them as regular existing files.  */
  /* -------------------------------------------------------------------- */
  if (fpSHP)
    fclose( fpSHP );
  fpSHP= NULL;
  if (fpSHX)
    fclose( fpSHX );
  fpSHX= NULL;
  
  return( msSHPOpen( pszLayer, "rb+" ) );
}

/************************************************************************/
/*                           writeBounds()                              */
/*                                                                      */
/*      Compute a bounds rectangle for a shape, and set it into the     */
/*      indicated location in the record.                               */
/************************************************************************/
static void writeBounds( uchar * pabyRec, shapeObj *shape, int nVCount )
{
  double	dXMin, dXMax, dYMin, dYMax;
  int		i, j;
  
  if( nVCount == 0 ) {
    dXMin = dYMin = dXMax = dYMax = 0.0;
  } else {
    dXMin = dXMax = shape->line[0].point[0].x;
    dYMin = dYMax = shape->line[0].point[0].y;
    
    for( i=0; i<shape->numlines; i++ ) {
      for( j=0; j<shape->line[i].numpoints; j++ ) {
	dXMin = MS_MIN(dXMin, shape->line[i].point[j].x);
	dXMax = MS_MAX(dXMax, shape->line[i].point[j].x);
	dYMin = MS_MIN(dYMin, shape->line[i].point[j].y);
	dYMax = MS_MAX(dYMax, shape->line[i].point[j].y);
      }
    }
  }
  
  if( bBigEndian ) { 
    SwapWord( 8, &dXMin );
    SwapWord( 8, &dYMin );
    SwapWord( 8, &dXMax );
    SwapWord( 8, &dYMax );
  }
  
  ByteCopy( &dXMin, pabyRec +  0, 8 );
  ByteCopy( &dYMin, pabyRec +  8, 8 );
  ByteCopy( &dXMax, pabyRec + 16, 8 );
  ByteCopy( &dYMax, pabyRec + 24, 8 );
}

int msSHPWritePoint(SHPHandle psSHP, pointObj *point )
{
  int nRecordOffset, nRecordSize=0;
  uchar	*pabyRec;
  int32	i32, nPoints, nParts;
  
  if( psSHP->nShapeType != SHP_POINT ) return(-1);

  psSHP->bUpdated = MS_TRUE;

  /* -------------------------------------------------------------------- */
  /*      Add the new entity to the in memory index.                      */
  /* -------------------------------------------------------------------- */
  psSHP->nRecords++;
  if( psSHP->nRecords > psSHP->nMaxRecords ) {
    psSHP->nMaxRecords = (int)(psSHP->nMaxRecords * 1.3 + 100);
    
    psSHP->panRecOffset = (int *) 
      SfRealloc(psSHP->panRecOffset,sizeof(int) * psSHP->nMaxRecords );
    psSHP->panRecSize = (int *) 
      SfRealloc(psSHP->panRecSize,sizeof(int) * psSHP->nMaxRecords );
  }

  /* -------------------------------------------------------------------- */
  /*      Compute a few things.                                           */
  /* -------------------------------------------------------------------- */
  nPoints = 1;
  nParts = 1;
  
  /* -------------------------------------------------------------------- */
  /*      Initialize record.                                              */
  /* -------------------------------------------------------------------- */
  psSHP->panRecOffset[psSHP->nRecords-1] = nRecordOffset = psSHP->nFileSize;
  
  pabyRec = (uchar *) malloc(nPoints * 2 * sizeof(double) + nParts * 4 + 128);
  
  /* -------------------------------------------------------------------- */
  /*      Write vertices for a point.                                     */
  /* -------------------------------------------------------------------- */
  ByteCopy( &(point->x), pabyRec + 12, 8 );
  ByteCopy( &(point->y), pabyRec + 20, 8 );
    
  if( bBigEndian ) {
    SwapWord( 8, pabyRec + 12 );
    SwapWord( 8, pabyRec + 20 );
  }
    
  nRecordSize = 20;

  /* -------------------------------------------------------------------- */
  /*      Set the shape type, record number, and record size.             */
  /* -------------------------------------------------------------------- */
  i32 = psSHP->nRecords-1+1;					/* record # */
  if( !bBigEndian ) SwapWord( 4, &i32 );
  ByteCopy( &i32, pabyRec, 4 );
  
  i32 = nRecordSize/2;				/* record size */
  if( !bBigEndian ) SwapWord( 4, &i32 );
  ByteCopy( &i32, pabyRec + 4, 4 );
  
  i32 = psSHP->nShapeType;				/* shape type */
  if( bBigEndian ) SwapWord( 4, &i32 );
  ByteCopy( &i32, pabyRec + 8, 4 );
  
  /* -------------------------------------------------------------------- */
  /*      Write out record.                                               */
  /* -------------------------------------------------------------------- */
  if (psSHP->fpSHP) {
    fseek( psSHP->fpSHP, nRecordOffset, 0 );
    fwrite( pabyRec, nRecordSize+8, 1, psSHP->fpSHP );
  } else if (psSHP->zfpSHP) {
      zzip_seek(psSHP->zfpSHP, nRecordOffset, 0);
      write (zzip_realfd (psSHP->zfpSHP), pabyRec, nRecordSize+8);
  }

  free( pabyRec );
  
  psSHP->panRecSize[psSHP->nRecords-1] = nRecordSize;
  psSHP->nFileSize += nRecordSize + 8;
  
  /* -------------------------------------------------------------------- */
  /*	Expand file wide bounds based on this shape.			  */
  /* -------------------------------------------------------------------- */
  if( psSHP->nRecords == 1 ) {
    psSHP->adBoundsMin[0] = psSHP->adBoundsMax[0] = point->x;
    psSHP->adBoundsMin[1] = psSHP->adBoundsMax[1] = point->y;
  } else {
    psSHP->adBoundsMin[0] = MS_MIN(psSHP->adBoundsMin[0], point->x);
    psSHP->adBoundsMin[1] = MS_MIN(psSHP->adBoundsMin[1], point->y);
    psSHP->adBoundsMax[0] = MS_MAX(psSHP->adBoundsMax[0], point->x);
    psSHP->adBoundsMax[1] = MS_MAX(psSHP->adBoundsMax[1], point->y);
  }
  
  return( psSHP->nRecords - 1 );
}


int msSHPWriteShape(SHPHandle psSHP, shapeObj *shape )
{
  int nRecordOffset, i, j, k, nRecordSize=0;
  uchar	*pabyRec;
  int32	i32, nPoints, nParts;
  double dfMMin, dfMMax = 0;
  psSHP->bUpdated = MS_TRUE;
  
  /* -------------------------------------------------------------------- */
  /*      Add the new entity to the in memory index.                      */
  /* -------------------------------------------------------------------- */
  psSHP->nRecords++;
  if( psSHP->nRecords > psSHP->nMaxRecords ) {
    psSHP->nMaxRecords = (int)(psSHP->nMaxRecords * 1.3 + 100);
    
    psSHP->panRecOffset = (int *) 
      SfRealloc(psSHP->panRecOffset,sizeof(int) * psSHP->nMaxRecords );
    psSHP->panRecSize = (int *) 
      SfRealloc(psSHP->panRecSize,sizeof(int) * psSHP->nMaxRecords );
  }
  
  /* -------------------------------------------------------------------- */
  /*      Compute a few things.                                           */
  /* -------------------------------------------------------------------- */
  nPoints = 0;
  for(i=0; i<shape->numlines; i++)
    nPoints += shape->line[i].numpoints;
  
  nParts = shape->numlines;
  
  /* -------------------------------------------------------------------- */
  /*      Initialize record.                                              */
  /* -------------------------------------------------------------------- */
  psSHP->panRecOffset[psSHP->nRecords-1] = nRecordOffset = psSHP->nFileSize;
  
  pabyRec = (uchar *) malloc(nPoints * 2 * sizeof(double) + nParts * 4 + 128);
  
  
  /* -------------------------------------------------------------------- */
  /*  Write vertices for a Polygon or Arc.				    */
  /* -------------------------------------------------------------------- */
  if(psSHP->nShapeType == SHP_POLYGON || psSHP->nShapeType == SHP_ARC ||
     psSHP->nShapeType == SHP_POLYGONM || psSHP->nShapeType == SHP_ARCM) {
    int32 t_nParts, t_nPoints, partSize;
    
    t_nParts = nParts;
    t_nPoints = nPoints;
    
    writeBounds( pabyRec + 12, shape, t_nPoints );
    
    if( bBigEndian ) { 
      SwapWord( 4, &nPoints );
      SwapWord( 4, &nParts );
    }
    
    ByteCopy( &nPoints, pabyRec + 40 + 8, 4 );
    ByteCopy( &nParts, pabyRec + 36 + 8, 4 );

    partSize = 0; // first part always starts at 0
    ByteCopy( &partSize, pabyRec + 44 + 8 + 4*0, 4 );
    if( bBigEndian ) SwapWord( 4, pabyRec + 44 + 8 + 4*0);

    for( i = 1; i < t_nParts; i++ ) {
      partSize += shape->line[i-1].numpoints;
      ByteCopy( &partSize, pabyRec + 44 + 8 + 4*i, 4 );
      if( bBigEndian ) SwapWord( 4, pabyRec + 44 + 8 + 4*i);
    }
    
    k = 0; // overall point counter
    for( i = 0; i < shape->numlines; i++ ) {
      for( j = 0; j < shape->line[i].numpoints; j++ ) {
	ByteCopy( &(shape->line[i].point[j].x), pabyRec + 44 + 4*t_nParts + 8 + k * 16, 8 );
	ByteCopy( &(shape->line[i].point[j].y), pabyRec + 44 + 4*t_nParts + 8 + k * 16 + 8, 8 );
	
	if( bBigEndian ) {
	  SwapWord( 8, pabyRec + 44+4*t_nParts+8+k*16 );
	  SwapWord( 8, pabyRec + 44+4*t_nParts+8+k*16+8 );
	}

	k++;
      }
    }

/* -------------------------------------------------------------------- */
/*      measured shape : polygon and arc.                               */
/* -------------------------------------------------------------------- */
    if(psSHP->nShapeType == SHP_POLYGONM || psSHP->nShapeType == SHP_ARCM)
    {
        dfMMin = shape->line[0].point[0].m;
        dfMMax = 
            shape->line[shape->numlines-1].point[shape->line[shape->numlines-1].numpoints-1].m;
            
        nRecordSize = 44 + 4*t_nParts + 8 + (t_nPoints* 16);

        ByteCopy( &(dfMMin), pabyRec + nRecordSize, 8 );
        if( bBigEndian ) SwapWord( 8, pabyRec + nRecordSize );
        nRecordSize += 8;

        ByteCopy( &(dfMMax), pabyRec + nRecordSize, 8 );
        if( bBigEndian ) SwapWord( 8, pabyRec + nRecordSize );
        nRecordSize += 8;
            
        for( i = 0; i < shape->numlines; i++ ) 
        {
            for( j = 0; j < shape->line[i].numpoints; j++ ) 
            {
                ByteCopy( &(shape->line[i].point[j].m), pabyRec + nRecordSize, 8 );
                if( bBigEndian ) SwapWord( 8, pabyRec + nRecordSize );
                nRecordSize += 8;
            }
        }
    }
    else
      nRecordSize = 44 + 4*t_nParts + 16 * t_nPoints;
  }
  
  /* -------------------------------------------------------------------- */
  /*  Write vertices for a MultiPoint.				          */
  /* -------------------------------------------------------------------- */
  else if( psSHP->nShapeType == SHP_MULTIPOINT ||
           psSHP->nShapeType == SHP_MULTIPOINTM) {
    int32 t_nPoints;
    
    t_nPoints = nPoints;
    
    writeBounds( pabyRec + 12, shape, nPoints );
    
    if( bBigEndian ) SwapWord( 4, &nPoints );
    ByteCopy( &nPoints, pabyRec + 44, 4 );
    
    for( i = 0; i < shape->line[0].numpoints; i++ ) {
      ByteCopy( &(shape->line[0].point[i].x), pabyRec + 48 + i*16, 8 );
      ByteCopy( &(shape->line[0].point[i].y), pabyRec + 48 + i*16 + 8, 8 );
      
      if( bBigEndian ) { 
	SwapWord( 8, pabyRec + 48 + i*16 );
	SwapWord( 8, pabyRec + 48 + i*16 + 8 );
      }
    }
    if (psSHP->nShapeType == SHP_MULTIPOINTM)
    {
        nRecordSize = 48 + 16 * t_nPoints;

        dfMMin = shape->line[0].point[0].m;
        dfMMax = shape->line[0].point[shape->line[0].numpoints-1].m;
        
        ByteCopy( &(dfMMin), pabyRec + nRecordSize, 8 );
        if( bBigEndian ) SwapWord( 8, pabyRec + nRecordSize );
        nRecordSize += 8;

        ByteCopy( &(dfMMax), pabyRec + nRecordSize, 8 );
        if( bBigEndian ) SwapWord( 8, pabyRec + nRecordSize );
        nRecordSize += 8;
        
        for( i = 0; i < shape->line[0].numpoints; i++ ) 
        {
            ByteCopy( &(shape->line[0].point[i].m), pabyRec + nRecordSize, 8 );
            if( bBigEndian ) SwapWord( 8, pabyRec + nRecordSize );
            nRecordSize += 8;
        }
    }
    else
      nRecordSize = 40 + 16 * t_nPoints;
  }
  
  /* -------------------------------------------------------------------- */
  /*      Write vertices for a point.                                     */
  /* -------------------------------------------------------------------- */
  else if( psSHP->nShapeType == SHP_POINT ||  psSHP->nShapeType == SHP_POINTM) {
    ByteCopy( &(shape->line[0].point[0].x), pabyRec + 12, 8 );
    ByteCopy( &(shape->line[0].point[0].y), pabyRec + 20, 8 );
    
    if( bBigEndian ) {
      SwapWord( 8, pabyRec + 12 );
      SwapWord( 8, pabyRec + 20 );
    }
    
    if (psSHP->nShapeType == SHP_POINTM)
    {
        nRecordSize = 28;

        ByteCopy( &(shape->line[0].point[0].m), pabyRec + nRecordSize, 8 );
        if( bBigEndian ) SwapWord( 8, pabyRec + nRecordSize );
        nRecordSize += 8;
    }
    else
      nRecordSize = 20;
  }
  
  /* -------------------------------------------------------------------- */
  /*      Set the shape type, record number, and record size.             */
  /* -------------------------------------------------------------------- */
  i32 = psSHP->nRecords-1+1;					/* record # */
  if( !bBigEndian ) SwapWord( 4, &i32 );
  ByteCopy( &i32, pabyRec, 4 );
  
  i32 = nRecordSize/2;				/* record size */
  if( !bBigEndian ) SwapWord( 4, &i32 );
  ByteCopy( &i32, pabyRec + 4, 4 );
  
  i32 = psSHP->nShapeType;				/* shape type */
  if( bBigEndian ) SwapWord( 4, &i32 );
  ByteCopy( &i32, pabyRec + 8, 4 );
  
  /* -------------------------------------------------------------------- */
  /*      Write out record.                                               */
  /* -------------------------------------------------------------------- */

  // JMW here
  //     if (zzip_file_real(file))
  //        return write (zzip_realfd (file), ptr, len);

  if (psSHP->fpSHP) {
    fseek( psSHP->fpSHP, nRecordOffset, 0 );
    fwrite( pabyRec, nRecordSize+8, 1, psSHP->fpSHP );
  } else if (psSHP->zfpSHP) {
    zzip_seek(psSHP->zfpSHP, nRecordOffset, 0);
    write (zzip_realfd (psSHP->zfpSHP), pabyRec, nRecordSize+8);
  }

  free( pabyRec );
  
  psSHP->panRecSize[psSHP->nRecords-1] = nRecordSize;
  psSHP->nFileSize += nRecordSize + 8;
  
  /* -------------------------------------------------------------------- */
  /*	Expand file wide bounds based on this shape.			  */
  /* -------------------------------------------------------------------- */
  if( psSHP->nRecords == 1 ) {
    psSHP->adBoundsMin[0] = psSHP->adBoundsMax[0] = shape->line[0].point[0].x;
    psSHP->adBoundsMin[1] = psSHP->adBoundsMax[1] = shape->line[0].point[0].y;
    psSHP->adBoundsMin[3] = psSHP->adBoundsMax[3] = shape->line[0].point[0].m;
  }
  
  for( i=0; i<shape->numlines; i++ ) {
    for( j=0; j<shape->line[i].numpoints; j++ ) {
      psSHP->adBoundsMin[0] = MS_MIN(psSHP->adBoundsMin[0], shape->line[i].point[j].x);
      psSHP->adBoundsMin[1] = MS_MIN(psSHP->adBoundsMin[1], shape->line[i].point[j].y);
      psSHP->adBoundsMin[3] = MS_MIN(psSHP->adBoundsMin[3], shape->line[i].point[j].m);
      psSHP->adBoundsMax[0] = MS_MAX(psSHP->adBoundsMax[0], shape->line[i].point[j].x);
      psSHP->adBoundsMax[1] = MS_MAX(psSHP->adBoundsMax[1], shape->line[i].point[j].y);
      psSHP->adBoundsMax[3] = MS_MAX(psSHP->adBoundsMax[3], shape->line[i].point[j].m);
    }
  }
  
  return( psSHP->nRecords - 1 );
}

/*
** msSHPReadPoint() - Reads a single point from a POINT shape file.
*/
int msSHPReadPoint( SHPHandle psSHP, int hEntity, pointObj *point )
{

    /* -------------------------------------------------------------------- */
    /*      Only valid for point shapefiles                                 */
    /* -------------------------------------------------------------------- */
    if( psSHP->nShapeType != SHP_POINT ) {
      msSetError(MS_SHPERR, "msSHPReadPoint only operates on point shapefiles.", "msSHPReadPoint()");
      return(MS_FAILURE);
    }

    /* -------------------------------------------------------------------- */
    /*      Validate the record/entity number.                              */
    /* -------------------------------------------------------------------- */
    if( hEntity < 0 || hEntity >= psSHP->nRecords ) {
      msSetError(MS_SHPERR, "Record index out of bounds.", "msSHPReadPoint()");
      return(MS_FAILURE);
    }

    if( psSHP->panRecSize[hEntity] == 4 ) {
      msSetError(MS_SHPERR, "NULL feature encountered.", "msSHPReadPoint()");
      return(MS_FAILURE);
    }   

    /* -------------------------------------------------------------------- */
    /*      Ensure our record buffer is large enough.                       */
    /* -------------------------------------------------------------------- */
    if( psSHP->panRecSize[hEntity]+8 > psSHP->nBufSize ) {
	psSHP->nBufSize = psSHP->panRecSize[hEntity]+8;
	psSHP->pabyRec = (uchar *) SfRealloc(psSHP->pabyRec,psSHP->nBufSize);
    }    

    /* -------------------------------------------------------------------- */
    /*      Read the record.                                                */
    /* -------------------------------------------------------------------- */
    zzip_seek( psSHP->zfpSHP, psSHP->panRecOffset[hEntity], 0 );
    zzip_fread( psSHP->pabyRec, psSHP->panRecSize[hEntity]+8, 1, psSHP->zfpSHP );
      
    memcpy( &(point->x), psSHP->pabyRec + 12, 8 );
    memcpy( &(point->y), psSHP->pabyRec + 20, 8 );
      
    if( bBigEndian ) {
      SwapWord( 8, &(point->x));
      SwapWord( 8, &(point->y));
    }

    return(MS_SUCCESS);
}

/*
** msSHPReadShape() - Reads the vertices for one shape from a shape file.
*/
void msSHPReadShape( SHPHandle psSHP, int hEntity, shapeObj *shape )
{
    int	       		i, j, k;
    int nOffset = 0;

 
    msInitShape(shape); /* initialize the shape */

    /* -------------------------------------------------------------------- */
    /*      Validate the record/entity number.                              */
    /* -------------------------------------------------------------------- */
    if( hEntity < 0 || hEntity >= psSHP->nRecords )
      return;

    if( psSHP->panRecSize[hEntity] == 4 ) {      
      shape->type = MS_SHAPE_NULL;
      return;
    }

    /* -------------------------------------------------------------------- */
    /*      Ensure our record buffer is large enough.                       */
    /* -------------------------------------------------------------------- */
    if( psSHP->panRecSize[hEntity]+8 > psSHP->nBufSize )
    {
	psSHP->nBufSize = psSHP->panRecSize[hEntity]+8;
	psSHP->pabyRec = (uchar *) SfRealloc(psSHP->pabyRec,psSHP->nBufSize);
    }    

    /* -------------------------------------------------------------------- */
    /*      Read the record.                                                */
    /* -------------------------------------------------------------------- */
    zzip_seek( psSHP->zfpSHP, psSHP->panRecOffset[hEntity], 0 );
    zzip_fread( psSHP->pabyRec, psSHP->panRecSize[hEntity]+8, 1, psSHP->zfpSHP );

    /* -------------------------------------------------------------------- */
    /*  Extract vertices for a Polygon or Arc.				    */
    /* -------------------------------------------------------------------- */
    if( psSHP->nShapeType == SHP_POLYGON || psSHP->nShapeType == SHP_ARC || 
        psSHP->nShapeType == SHP_POLYGONM || psSHP->nShapeType == SHP_ARCM)
    {
      int32		nPoints, nParts;      

      // copy the bounding box
      memcpy( &shape->bounds.minx, psSHP->pabyRec + 8 + 4, 8 );
      memcpy( &shape->bounds.miny, psSHP->pabyRec + 8 + 12, 8 );
      memcpy( &shape->bounds.maxx, psSHP->pabyRec + 8 + 20, 8 );
      memcpy( &shape->bounds.maxy, psSHP->pabyRec + 8 + 28, 8 );

      if( bBigEndian ) {
	SwapWord( 8, &shape->bounds.minx);
	SwapWord( 8, &shape->bounds.miny);
	SwapWord( 8, &shape->bounds.maxx);
	SwapWord( 8, &shape->bounds.maxy);
      }

      memcpy( &nPoints, psSHP->pabyRec + 40 + 8, 4 );
      memcpy( &nParts, psSHP->pabyRec + 36 + 8, 4 );
      
      if( bBigEndian ) {
	SwapWord( 4, &nPoints );
        SwapWord( 4, &nParts );
      }

      /* -------------------------------------------------------------------- */
      /*      Copy out the part array from the record.                        */
      /* -------------------------------------------------------------------- */
      if( psSHP->nPartMax < nParts )
      {
	psSHP->nPartMax = nParts;
	psSHP->panParts = (int *) SfRealloc(psSHP->panParts, psSHP->nPartMax * sizeof(int) );
      }
      
      memcpy( psSHP->panParts, psSHP->pabyRec + 44 + 8, 4 * nParts );
      for( i = 0; i < nParts; i++ )
	if( bBigEndian ) SwapWord( 4, psSHP->panParts+i );
      
      /* -------------------------------------------------------------------- */
      /*      Fill the shape structure.                                       */
      /* -------------------------------------------------------------------- */
      if( (shape->line = (lineObj *)malloc(sizeof(lineObj)*nParts)) == NULL ) {
	msSetError(MS_MEMERR, NULL, "SHPReadShape()");
	return;
      }

      shape->numlines = nParts;
      
      k = 0; /* overall point counter */
      for( i = 0; i < nParts; i++) 
      { 	  
	if( i == nParts-1)
	  shape->line[i].numpoints = nPoints - psSHP->panParts[i];
	else
	  shape->line[i].numpoints = psSHP->panParts[i+1] - psSHP->panParts[i];
	
	if( (shape->line[i].point = (pointObj *)malloc(sizeof(pointObj)*shape->line[i].numpoints)) == NULL ) {
	  free(shape->line);
	  shape->numlines = 0;
	  return;
	}

        //nOffset = 44 + 8 + 4*nParts;
	for( j = 0; j < shape->line[i].numpoints; j++ )
	{
	  memcpy(&(shape->line[i].point[j].x), psSHP->pabyRec + 44 + 4*nParts + 8 + k * 16, 8 );
	  memcpy(&(shape->line[i].point[j].y), psSHP->pabyRec + 44 + 4*nParts + 8 + k * 16 + 8, 8 );
	  
	  if( bBigEndian ) {
	    SwapWord( 8, &(shape->line[i].point[j].x) );
	    SwapWord( 8, &(shape->line[i].point[j].y) );
	  }
          
          shape->line[i].point[j].m = 0; // initialize
/* -------------------------------------------------------------------- */
/*      Measured arc and polygon support.                               */
/* -------------------------------------------------------------------- */
          if (psSHP->nShapeType == SHP_POLYGONM || psSHP->nShapeType == SHP_ARCM)
          {
              nOffset = 44 + 8 + (4*nParts) + (16*nPoints) ;
              if( psSHP->panRecSize[hEntity]+8 >= nOffset + 16 + 8*nPoints )
              {
                   memcpy(&(shape->line[i].point[j].m), 
                          psSHP->pabyRec + nOffset + 16 + k*8, 8 );
                   if( bBigEndian ) 
                       SwapWord( 8, &(shape->line[i].point[j].m) );
              }   
          }
	  k++;
	}
      }

      if(psSHP->nShapeType == SHP_POLYGON || psSHP->nShapeType == SHP_POLYGONM)
        shape->type = MS_SHAPE_POLYGON;
      else
        shape->type = MS_SHAPE_LINE;

    }

    /* -------------------------------------------------------------------- */
    /*  Extract a MultiPoint.   			                    */
    /* -------------------------------------------------------------------- */
    else if( psSHP->nShapeType == SHP_MULTIPOINT || psSHP->nShapeType == SHP_MULTIPOINTM)
    {
      int32		nPoints;

      // copy the bounding box
      memcpy( &shape->bounds.minx, psSHP->pabyRec + 8 + 4, 8 );
      memcpy( &shape->bounds.miny, psSHP->pabyRec + 8 + 12, 8 );
      memcpy( &shape->bounds.maxx, psSHP->pabyRec + 8 + 20, 8 );
      memcpy( &shape->bounds.maxy, psSHP->pabyRec + 8 + 28, 8 );

      if( bBigEndian ) {
	SwapWord( 8, &shape->bounds.minx);
	SwapWord( 8, &shape->bounds.miny);
	SwapWord( 8, &shape->bounds.maxx);
	SwapWord( 8, &shape->bounds.maxy);
      }

      memcpy( &nPoints, psSHP->pabyRec + 44, 4 );
      if( bBigEndian ) SwapWord( 4, &nPoints );
    
      /* -------------------------------------------------------------------- */
      /*      Fill the shape structure.                                       */
      /* -------------------------------------------------------------------- */
      if( (shape->line = (lineObj *)malloc(sizeof(lineObj))) == NULL ) {
	msSetError(MS_MEMERR, NULL, "SHPReadShape()");
	return;
      }

      shape->numlines = 1;
      shape->line[0].numpoints = nPoints;
      shape->line[0].point = (pointObj *) malloc( nPoints * sizeof(pointObj) );
      
      for( i = 0; i < nPoints; i++ ) {
	memcpy(&(shape->line[0].point[i].x), psSHP->pabyRec + 48 + 16 * i, 8 );
	memcpy(&(shape->line[0].point[i].y), psSHP->pabyRec + 48 + 16 * i + 8, 8 );
	
	if( bBigEndian ) {
	  SwapWord( 8, &(shape->line[0].point[i].x) );
	  SwapWord( 8, &(shape->line[0].point[i].y) );
	}

        shape->line[0].point[i].m = 0; //initialize
/* -------------------------------------------------------------------- */
/*      Measured shape : multipont.                                     */
/* -------------------------------------------------------------------- */
        if (psSHP->nShapeType == SHP_MULTIPOINTM)
        {
            nOffset = 48 + 16*nPoints;
            memcpy(&(shape->line[0].point[i].m), psSHP->pabyRec + nOffset + 16 + i*8, 8 );
            if( bBigEndian ) 
              SwapWord( 8, &(shape->line[0].point[i].m));
        }
      }

      shape->type = MS_SHAPE_POINT;
    }

    /* -------------------------------------------------------------------- */
    /*  Extract a Point.   			                    */
    /* -------------------------------------------------------------------- */
    else if( psSHP->nShapeType == SHP_POINT ||  psSHP->nShapeType == SHP_POINTM)
    {    

      /* -------------------------------------------------------------------- */
      /*      Fill the shape structure.                                       */
      /* -------------------------------------------------------------------- */
      if( (shape->line = (lineObj *)malloc(sizeof(lineObj))) == NULL ) {
	msSetError(MS_MEMERR, NULL, "SHPReadShape()");
	return;
      }

      shape->numlines = 1;
      shape->line[0].numpoints = 1;
      shape->line[0].point = (pointObj *) malloc(sizeof(pointObj));
      
      memcpy( &(shape->line[0].point[0].x), psSHP->pabyRec + 12, 8 );
      memcpy( &(shape->line[0].point[0].y), psSHP->pabyRec + 20, 8 );
      
      if( bBigEndian ) {
	SwapWord( 8, &(shape->line[0].point[0].x));
	SwapWord( 8, &(shape->line[0].point[0].y));
      }

      shape->line[0].point[0].m = 0; //initialize
/* -------------------------------------------------------------------- */
/*      Measured support : point.                                       */
/* -------------------------------------------------------------------- */
      if (psSHP->nShapeType == SHP_POINTM)
      {
          nOffset = 20 + 8;
          if( psSHP->panRecSize[hEntity]+8 >= nOffset + 8 )
          {
              memcpy(&(shape->line[0].point[0].m), psSHP->pabyRec + nOffset, 8 );
        
              if( bBigEndian ) 
                SwapWord( 8, &(shape->line[0].point[0].m));
          }
      }
      // set the bounding box to the point
      shape->bounds.minx = shape->bounds.maxx = shape->line[0].point[0].x;
      shape->bounds.miny = shape->bounds.maxy = shape->line[0].point[0].y;

      shape->type = MS_SHAPE_POINT;
    }

    shape->index = hEntity;
    return;
}

int msSHPReadBounds( SHPHandle psSHP, int hEntity, rectObj *padBounds)
{
  /* -------------------------------------------------------------------- */
  /*      Validate the record/entity number.                              */
  /* -------------------------------------------------------------------- */
  if( psSHP->nRecords <= 0 || hEntity < -1 || hEntity >= psSHP->nRecords ) {
    padBounds->minx = padBounds->miny = padBounds->maxx = padBounds->maxy = 0.0;
    return(-1);
  }

  /* -------------------------------------------------------------------- */
  /*	If the entity is -1 we fetch the bounds for the whole file.	  */
  /* -------------------------------------------------------------------- */
  if( hEntity == -1 ) {
    padBounds->minx = psSHP->adBoundsMin[0];
    padBounds->miny = psSHP->adBoundsMin[1];
    padBounds->maxx = psSHP->adBoundsMax[0];
    padBounds->maxy = psSHP->adBoundsMax[1];
  } else {    
    if( psSHP->panRecSize[hEntity] == 4 ) { // NULL shape
      padBounds->minx = padBounds->miny = padBounds->maxx = padBounds->maxy = 0.0;
      return(-1);
    } 
    
    if( psSHP->nShapeType != SHP_POINT ) {
      zzip_seek( psSHP->zfpSHP, psSHP->panRecOffset[hEntity]+12, 0 );
      zzip_fread( padBounds, sizeof(double)*4, 1, psSHP->zfpSHP );
      
      if( bBigEndian ) {
	SwapWord( 8, &(padBounds->minx) );
	SwapWord( 8, &(padBounds->miny) );
	SwapWord( 8, &(padBounds->maxx) );
	SwapWord( 8, &(padBounds->maxy) );
      }
    } else {
      /* -------------------------------------------------------------------- */
      /*      For points we fetch the point, and duplicate it as the          */
      /*      minimum and maximum bound.                                      */
      /* -------------------------------------------------------------------- */
      
      zzip_seek( psSHP->zfpSHP, psSHP->panRecOffset[hEntity]+12, 0 );
      zzip_fread( padBounds, sizeof(double)*2, 1, psSHP->zfpSHP );
      
      if( bBigEndian ) {
	SwapWord( 8, &(padBounds->minx) );
	SwapWord( 8, &(padBounds->miny) );
      }
      
      padBounds->maxx = padBounds->minx;
      padBounds->maxy = padBounds->miny;
    }
  }

  return(0);
}

int msSHPOpenFile(shapefileObj *shpfile, char *mode, const TCHAR *filename)
{
  int i;
  TCHAR *dbfFilename;

  if(!filename) {
    msSetError(MS_IOERR, "No (NULL) filename provided.", "msSHPOpenFile()");
    return(-1);
  }

  /* initialize a few things */
  shpfile->status = NULL;
  shpfile->lastshape = -1;

  /* open the shapefile file (appending ok) and get basic info */
  if(!mode) 	
    shpfile->hSHP = msSHPOpen( filename, "rb");
  else
    shpfile->hSHP = msSHPOpen( filename, mode);

  if(!shpfile->hSHP) {
    msSetError(MS_IOERR, "(%ls)", "msSHPOpenFile()", filename);
    return(-1);
  }

  _tcscpy(shpfile->source, filename);
  
  /* load some information about this shapefile */
  msSHPGetInfo( shpfile->hSHP, &shpfile->numshapes, &shpfile->type);
  msSHPReadBounds( shpfile->hSHP, -1, &(shpfile->bounds));
  
  dbfFilename = (TCHAR *)malloc((_tcslen(filename)+5) * sizeof(TCHAR) * 2);
  _tcscpy(dbfFilename, filename);
  
  /* clean off any extention the filename might have */
  for (i = _tcslen(dbfFilename) - 1; 
       i > 0 && dbfFilename[i] != _T('.') && dbfFilename[i] != _T('/') && dbfFilename[i] != _T('\\');
       i-- ) {}

  if( dbfFilename[i] == _T('.') )
    dbfFilename[i] = _T('\0');
  
  _tcscat(dbfFilename, _T(".dbf"));

  shpfile->hDBF = msDBFOpen(dbfFilename, "rb");

  if(!shpfile->hDBF) {
    msSetError(MS_IOERR, "(%ls)", "msSHPOpenFile()", dbfFilename);    
    free(dbfFilename);
    return(-1);
  }
  free(dbfFilename);

  return(0); /* all o.k. */
}

// Creates a new shapefile
int msSHPCreateFile(shapefileObj *shpfile, const TCHAR *filename, int type)
{
  if(type != SHP_POINT && type != SHP_MULTIPOINT && type != SHP_ARC &&
     type != SHP_POLYGON && type != SHP_POINTM && type != SHP_MULTIPOINTM &&
     type != SHP_ARCM && type != SHP_POLYGONM) {
    msSetError(MS_SHPERR, "Invalid shape type.", "msNewSHPFile()");
    return(-1);
  }

  // create the spatial portion
  shpfile->hSHP = msSHPCreate(filename, type);
  if(!shpfile->hSHP) {
    msSetError(MS_IOERR, "(%ls)", "msNewSHPFile()",filename);    
    return(-1);
  }

  // retrieve a few things about this shapefile
  msSHPGetInfo( shpfile->hSHP, &shpfile->numshapes, &shpfile->type);
  msSHPReadBounds( shpfile->hSHP, -1, &(shpfile->bounds));

  // initialize a few other things
  shpfile->status = NULL;
  shpfile->lastshape = -1;

  shpfile->hDBF = NULL; // XBase file is NOT created here...
  return(0);
}

void msSHPCloseFile(shapefileObj *shpfile)
{
  if (shpfile) { // Silently return if called with NULL shpfile by freeLayer()
    if(shpfile->hSHP) msSHPClose(shpfile->hSHP);
    if(shpfile->hDBF) msDBFClose(shpfile->hDBF);
    if(shpfile->status) free(shpfile->status);
  }
}

// status array lives in the shpfile, can return MS_SUCCESS/MS_FAILURE/MS_DONE
int msSHPWhichShapes(shapefileObj *shpfile, rectObj rect, int debug)
{
  int i;
  rectObj shaperect;
  TCHAR *filename;

  if(shpfile->status) {
    free(shpfile->status);
    shpfile->status = NULL;
  }

  shpfile->statusbounds = rect; // save the search extent

  // rect and shapefile DON'T overlap...
  if(msRectOverlap(&shpfile->bounds, &rect) != MS_TRUE) 
    return(MS_DONE);

  if(msRectContained(&shpfile->bounds, &rect) == MS_TRUE) {
    shpfile->status = msAllocBitArray(shpfile->numshapes);
    if(!shpfile->status) {
      msSetError(MS_MEMERR, NULL, "msSHPWhichShapes()");
      return(MS_FAILURE);
    }
    for(i=0;i<shpfile->numshapes;i++) 
      msSetBit(shpfile->status, i, 1);
  } else {
    if((filename = (TCHAR *)malloc((_tcslen(shpfile->source)+_tcslen(_T(MS_INDEX_EXTENSION))+1) * sizeof(TCHAR) * 2)) == NULL) {
      msSetError(MS_MEMERR, NULL, "msSHPWhichShapes()");    
      return(MS_FAILURE);
    }
    _stprintf(filename, _T("%s%s"), shpfile->source, _T(MS_INDEX_EXTENSION));
    
    shpfile->status = msSearchDiskTree(filename, rect, debug);
    free(filename);

    if(shpfile->status) // index 
      msFilterTreeSearch(shpfile, shpfile->status, rect);
    else { // no index 
      shpfile->status = msAllocBitArray(shpfile->numshapes);
      if(!shpfile->status) {
	msSetError(MS_MEMERR, NULL, "msSHPWhichShapes()");       
	return(MS_FAILURE);
      }
      
      for(i=0;i<shpfile->numshapes;i++) {
	if(!msSHPReadBounds(shpfile->hSHP, i, &shaperect))
	  if(msRectOverlap(&shaperect, &rect) == MS_TRUE)
	    msSetBit(shpfile->status, i, 1);
      }
    }   
  }
 
  shpfile->lastshape = -1;

  return(MS_SUCCESS); /* success */
}

