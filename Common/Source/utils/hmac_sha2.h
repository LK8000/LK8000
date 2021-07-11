/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights
*/


#ifndef HMAC_SHA2_H
#define HMAC_SHA2_H

#define SHA256_DIGEST_SIZE ( 256 / 8)
#define SHA256_BLOCK_SIZE  ( 512 / 8)

typedef unsigned char uint8;
typedef unsigned int  uint32;
typedef unsigned long long uint64;


typedef struct {
    unsigned int tot_len;
    unsigned int len;
    unsigned char block[2 * SHA256_BLOCK_SIZE];
    uint32 h[8];
} sha256_ctx;

typedef sha256_ctx sha224_ctx;

void sha256_init(sha256_ctx * ctx);
void sha256_update(sha256_ctx *ctx, const unsigned char *message,
                   unsigned int len);
void sha256_final(sha256_ctx *ctx, unsigned char *digest);
void sha256(const unsigned char *message, unsigned int len,
            unsigned char *digest);

typedef struct {
    sha256_ctx ctx_inside;
    sha256_ctx ctx_outside;

    sha256_ctx ctx_inside_reinit;
    sha256_ctx ctx_outside_reinit;

    unsigned char block_ipad[SHA256_BLOCK_SIZE];
    unsigned char block_opad[SHA256_BLOCK_SIZE];
} hmac_sha256_ctx;


void hmac_sha256_init(hmac_sha256_ctx *ctx, const unsigned char *key,
                      unsigned int key_size);
void hmac_sha256_reinit(hmac_sha256_ctx *ctx);
void hmac_sha256_update(hmac_sha256_ctx *ctx, const unsigned char *message,
                        unsigned int message_len);
void hmac_sha256_final(hmac_sha256_ctx *ctx, unsigned char *mac,
                       unsigned int mac_size);
void hmac_sha256(const unsigned char *key, unsigned int key_size,
                 const unsigned char *message, unsigned int message_len,
                 unsigned char *mac, unsigned mac_size);


void otpReply(const  char *key,const  char *message, char *mac64, unsigned mac_size);


#endif

