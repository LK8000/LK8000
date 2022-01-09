/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

*/

#ifndef MD5_H
#define MD5_H

#include "md5internal.h"

class MD5_Base {
protected:
  md5_ctx context;

public:

  MD5_Base();
  MD5_Base(unsigned int key1, unsigned int key2, unsigned int key3, unsigned int key4);
  
  void Init();
  void Init(unsigned int key1, unsigned int key2, unsigned int key3, unsigned int key4);
  
  // MD5 block update operation. Continues an MD5 message-digest
  // operation, processing another message block, and updating the
  // context.
  void Update(const void* input, size_t size);

  template<typename T>
  void Update(const T& data) {
    static_assert(std::is_trivially_copyable_v<T>, "incompatible type");
    static_assert(!std::is_pointer_v<T>, "incompatible type");
    Update(&data, sizeof(data));
  }

  void Update(const std::string& data) {
    Update(data.data(), data.size());
  }
};

class MD5 : public MD5_Base
{
  unsigned char resbuf[16];
  
public:
  MD5() : MD5_Base() { }
  explicit MD5(const MD5_Base& md5) : MD5_Base(md5) { }

  // This version of the digest is actually a "printf'd" version of the digest.
  char digestChars[33];

  // MD5 finalization. Ends an MD5 message-digest operation, writing the
  // the message digest and zeroizing the context.
  // Writes to digestRaw
  void Final();
};

#endif

