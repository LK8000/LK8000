/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

*/

#ifndef MD5_H
#define MD5_H

#include "md5internal.h"

class MD5
{
private:
  md5_ctx context;
  unsigned char resbuf[16];
  
public:
  // This version of the digest is actually a "printf'd" version of the digest.
  char digestChars[33];

  MD5();
  MD5(unsigned int key1, unsigned int key2, unsigned int key3, unsigned int key4);
  
  void Init();
  void Init(unsigned int key1, unsigned int key2, unsigned int key3, unsigned int key4);
  
  // MD5 block update operation. Continues an MD5 message-digest
  // operation, processing another message block, and updating the
  // context.
  void Update( const unsigned char *input, unsigned int inputLen);

  // MD5 finalization. Ends an MD5 message-digest operation, writing the
  // the message digest and zeroizing the context.
  // Writes to digestRaw
  void Final();
};

#endif

