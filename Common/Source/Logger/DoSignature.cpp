/*
 * The LK8000 Project
 *
 * This file is NOT distributed under the GPL. This is free software.
 * In other words, you will not be obliged to publish the source code
 * of your program, if you include this material in your own code.
 * There are no copyrights on this file.
 *
 * $Id$
 */

#include "externs.h"
#include "LoggerFileHandlers.h"
#include "md5.h"

typedef struct
{
  MD5 a;
  MD5 b;
  MD5 c;
  MD5 d;
} md5generators_t;

LineList Linelist;
md5generators_t md5gen;

#define MAXLINELEN 1000


void GenerateMD5(LineList &lines, md5generators_t *md5s)
{
    
  md5s->a.Init( 0x1C80A301,0x9EB30b89,0x39CB2Afe,0x0D0FEA76 );
  md5s->b.Init( 0x48327203,0x3948ebea,0x9a9b9c9e,0xb3bed89a );
  md5s->c.Init(	0x67452301,0xefcdab89,0x98badcfe,0x10325476 );
  md5s->d.Init( 0xc8e899e8,0x9321c28a,0x438eba12,0x8cbe0aee );

  int skipwhitespaces = 1;
  
  LineList::const_iterator it;
  
  for (it = lines.begin(); it != lines.end(); ++it) {
    for (unsigned int i=0; i<(it->length()); ++i) {
      //Skip whitespaces
      if (skipwhitespaces) {
        char c = it->c_str()[i];
        if (c >= 0x20 && c <= 0x7E &&
            c != 0x0D &&
            c != 0x0A &&
            c != 0x24 &&
            c != 0x2A &&
            c != 0x2C &&
            c != 0x21 &&
            c != 0x5C &&
            c != 0x5E &&
            c != 0x7E) ; else continue;
      }
      md5s->a.Update((unsigned char*)it->c_str()+i, 1);
      md5s->b.Update((unsigned char*)it->c_str()+i, 1);
      md5s->c.Update((unsigned char*)it->c_str()+i, 1);
      md5s->d.Update((unsigned char*)it->c_str()+i, 1);
    }
  }
  md5s->a.Final();
  md5s->b.Final();
  md5s->c.Final();
  md5s->d.Final();
}



// Read input file into memory buffer
int ReadInputFile(LineList &lines, const TCHAR *filename)
{
  FILE *f;
  char linebuf[MAXLINELEN];

  lines.clear();
 
  f = _wfopen(filename,TEXT("rb"));
  if (f==NULL) return 1;
  while (fgets(linebuf,MAXLINELEN,f)!=NULL) lines.push_back(linebuf);
  fclose(f);
  return 0;
}

// Write file from memory buffer
int WriteOutputFile(LineList &lines, const TCHAR *filename)
{
  FILE *f;
  LineList::const_iterator it;

  f = _wfopen(filename,TEXT("wb+"));
  if (f==NULL) return 1;
  for (it = lines.begin(); it != lines.end(); ++it) fputs(it->c_str(),f);
  fclose(f);
  return 0;
}



void AddGRecordToBuffer(LineList &lines, md5generators_t *md5s)
{
  std::string str;

  str = "G";
  for (int i=0; i<16; i++) str += md5s->a.digestChars[i];
  str += "\r\n";
  str += "G";
  for (int i=0; i<16; i++) str += md5s->a.digestChars[i+16];
  str += "\r\n";
  lines.push_back(str);

  str = "G";
  for (int i=0; i<16; i++) str += md5s->b.digestChars[i];
  str += "\r\n";
  str += "G";
  for (int i=0; i<16; i++) str += md5s->b.digestChars[i+16];
  str += "\r\n";
  lines.push_back(str);

  str = "G";
  for (int i=0; i<16; i++) str += md5s->c.digestChars[i];
  str += "\r\n";
  str += "G";
  for (int i=0; i<16; i++) str += md5s->c.digestChars[i+16];
  str += "\r\n";
  lines.push_back(str);

  str = "G";
  for (int i=0; i<16; i++) str += md5s->d.digestChars[i];
  str += "\r\n";
  str += "G";
  for (int i=0; i<16; i++) str += md5s->d.digestChars[i+16];
  str += "\r\n";
  lines.push_back(str);
  
}



//
// Return values:
//  0	Ok
//  1	src File error
//  12  existing dstfile
//

int DoSignature(TCHAR *homepath) {

  TCHAR fullsrcfile[MAX_PATH],fulldstfile[MAX_PATH];

  #if TESTBENCH
  StartupStore(_T(".... DoSignature start, homepath=<%s>\n"),homepath);
  #endif

  wsprintf(fullsrcfile,TEXT("%s/LOGGER_TMP.IGC"),homepath);
  wsprintf(fulldstfile,TEXT("%s/LOGGER_SIG.IGC"),homepath);

  // Force removal of anything existing.
  // We should have checked already that this file does not exist, and rename it eventually.
  // If it is here, we remove it, we are in the hurry and cannot loose time on previous files.
  FILE *ft;
  ft=NULL;
  DeleteFile(fulldstfile);
  if ( (ft=_wfopen(fulldstfile,TEXT("r")))!=NULL ) {
	StartupStore(_T("... DoSignature: ERROR existing destination file <%S>!%s"),fulldstfile,NEWLINE);
	fclose(ft);
	return 12;
  }

  if (ReadInputFile(Linelist, fullsrcfile)) {
	StartupStore(_T("... DoSignature: ERROR source file <%S> disappeared%s"),fullsrcfile,NEWLINE);
	return 1;
  }
  GenerateMD5(Linelist, &md5gen);
  AddGRecordToBuffer(Linelist, &md5gen);
  WriteOutputFile(Linelist, fulldstfile);	

  #if TESTBENCH
  StartupStore(_T("... DoSignature: signature OK%s"),NEWLINE);
  #endif

  return 0;
} 

