#ifndef SHA256_H
#define SHA256_H
#include <stdint.h>
#include <stddef.h>
typedef struct { uint32_t h[8]; uint8_t buf[64]; uint64_t len; size_t n; } sha256_ctx;
void sha256_init(sha256_ctx*); void sha256_update(sha256_ctx*,const uint8_t*,size_t);
void sha256_final(sha256_ctx*,uint8_t out[32]);
void sha256_put_i64(sha256_ctx*,int64_t v); /* endian-independent LE absorb */
void sha256_hex(const uint8_t d[32],char out[65]);
#endif
