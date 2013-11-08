/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

*/

#include "md5.h"

MD5::MD5() 
{
    Init();
}

MD5::MD5(unsigned int key1, unsigned int key2, unsigned int key3, unsigned int key4)
{
  Init(key1, key2, key3, key4);
}

void MD5::Init()
{
  md5_init_ctx(&context);
}

void MD5::Init(unsigned int key1, unsigned int key2, unsigned int key3, unsigned int key4)
{
  md5_init_ctx(&context);
  context.A = key1;
  context.B = key2;
  context.C = key3;
  context.D = key4;
}

// MD5 block update operation. Continues an MD5 message-digest
// operation, processing another message block, and updating the
// context.
void MD5::Update( const unsigned char *input, unsigned int inputLen)
{
 md5_process_bytes(input, inputLen, &context);
}

// MD5 finalization. Ends an MD5 message-digest operation, writing the
// the message digest and zeroizing the context.
// Writes to digestRaw
void MD5::Final()
{
  md5_finish_ctx(&context, resbuf);
  for ( int pos = 0 ; pos < 16 ; pos++ ) sprintf( digestChars+(pos*2), "%02x", resbuf[pos] ) ;
  digestChars[32]=0;
}

