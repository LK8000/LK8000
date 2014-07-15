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
    
  md5s->a.Init( 0x63e54c01, 0x25adab89, 0x44baecfe, 0x60f25476 );
  md5s->b.Init( 0x41e24d03, 0x23b8ebea, 0x4a4bfc9e, 0x640ed89a );
  md5s->c.Init( 0x61e54e01, 0x22cdab89, 0x48b20cfe, 0x62125476 );
  md5s->d.Init( 0xc1e84fe8, 0x21d1c28a, 0x438e1a12, 0x6c250aee );

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
      md5s->a.Update((const unsigned char*)it->c_str()+i, 1);
      md5s->b.Update((const unsigned char*)it->c_str()+i, 1);
      md5s->c.Update((const unsigned char*)it->c_str()+i, 1);
      md5s->d.Update((const unsigned char*)it->c_str()+i, 1);
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
 
  f = _tfopen(filename,TEXT("rb"));
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

  f = _tfopen(filename,TEXT("wb+"));
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

  _stprintf(fullsrcfile,TEXT("%s/LOGGER_TMP.IGC"),homepath);
  _stprintf(fulldstfile,TEXT("%s/LOGGER_SIG.IGC"),homepath);

  // Force removal of anything existing.
  // We should have checked already that this file does not exist, and rename it eventually.
  // If it is here, we remove it, we are in the hurry and cannot loose time on previous files.
  FILE *ft;
  ft=NULL;
  lk::filesystem::deleteFile(fulldstfile);
  #if TESTBENCH
  StartupStore(_T("... DoSignature: delete ok%s"),NEWLINE);
  #endif
  if ( (ft=_tfopen(fulldstfile,TEXT("r")))!=NULL ) {
	StartupStore(_T("... DoSignature: ERROR existing destination file <%s>!%s"),fulldstfile,NEWLINE);
	fclose(ft);
	return 12;
  }
  #if TESTBENCH
  StartupStore(_T("... DoSignature: tfopen ok%s"),NEWLINE);
  #endif

  if (ReadInputFile(Linelist, fullsrcfile)) {
	StartupStore(_T("... DoSignature: ERROR source file <%s> disappeared%s"),fullsrcfile,NEWLINE);
	return 1;
  }
  #if TESTBENCH
  StartupStore(_T("... DoSignature: readinputfile ok%s"),NEWLINE);
  #endif
  GenerateMD5(Linelist, &md5gen);
  AddGRecordToBuffer(Linelist, &md5gen);
  #if TESTBENCH
  StartupStore(_T("... DoSignature: addgrecord ok%s"),NEWLINE);
  #endif
  WriteOutputFile(Linelist, fulldstfile);	

  #if TESTBENCH
  StartupStore(_T("... DoSignature: signature OK%s"),NEWLINE);
  #endif

  return 0;
} 

