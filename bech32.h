// bech32.h
#ifndef BECH32_H
#define BECH32_H
#include <stdint.h>
#include <stddef.h>

int bech32_convert_bits(uint8_t *out, size_t *outlen, int outbits,
                        const uint8_t *in, size_t inlen, int inbits, int pad);

int bech32_encode(char *output, const char *hrp, const uint8_t *data, size_t datalen);

#endif
