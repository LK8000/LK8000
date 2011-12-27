/*
 * This is not under GPL
 */
#ifndef _CRYPTO_H_
#define _CRYPTO_H_

#include "LoggerMD5.h"
#include "LoggerFileHandlers.h"

typedef struct
{
  MD5 a;
  MD5 b;
  MD5 c;
  MD5 d;
} md5generators_t;


void GenerateMD5(LineList &lines, md5generators_t *md5s);


#endif

