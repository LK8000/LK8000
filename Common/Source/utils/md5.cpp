/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

*/

#include <assert.h>
#include <iterator>
#include "stl_utils.h"
#include "md5.h"


MD5_Base::MD5_Base()
{
    Init();
}

MD5_Base::MD5_Base(unsigned int key1, unsigned int key2, unsigned int key3, unsigned int key4)
{
  Init(key1, key2, key3, key4);
}

void MD5_Base::Init()
{
  md5_init_ctx(&context);
}

void MD5_Base::Init(unsigned int key1, unsigned int key2, unsigned int key3, unsigned int key4)
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
void MD5_Base::Update( const unsigned char *input, unsigned int inputLen)
{
 md5_process_bytes(input, inputLen, &context);
}

// MD5 finalization. Ends an MD5 message-digest operation, writing the
// the message digest and zeroizing the context.
// Writes to digestRaw
void MD5::Final()
{
  assert(std::size(digestChars) == (std::size(resbuf) * 2 + 1));
  static constexpr char Digit[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
  
  md5_finish_ctx(&context, resbuf);
  
  char * out_iterator = std::begin(digestChars);
  for ( char c : resbuf) {
    *(out_iterator++) = Digit[(c & 0xF0) >> 4];
    *(out_iterator++) = Digit[(c & 0x0F) >> 0];
  }
  *out_iterator = '\0';
}

