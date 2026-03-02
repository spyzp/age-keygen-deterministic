#include "bech32.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>

static const char *charset = "qpzry9x8gf2tvdw0s3jn54khce6mua7l";
static const uint32_t generator[5] = {0x3b6a57b2UL,0x26508e6dUL,0x1ea119faUL,0x3d4233ddUL,0x2a1462b3UL};

static uint32_t bech32_polymod(const uint8_t *values, size_t length) {
    uint32_t chk = 1;

    for (size_t i = 0; i < length; i++) {
        uint8_t top = chk >> 25;
        chk = ((chk & 0x1ffffff) << 5) ^ values[i];
        for (int j = 0; j < 5; j++) {
            if ((top >> j) & 1) {
                chk ^= generator[j];
            }
        }
    }

    return chk;
}

static void bech32_create_checksum(const char *hrp, const uint8_t *data, size_t datalen, uint8_t *checksum) {
    size_t hrp_len = strlen(hrp);
    size_t hrp_ext_len = hrp_len * 2 + 1;
    size_t full_len = hrp_ext_len + datalen + 6;
    size_t i;
    uint8_t values[full_len];

    for (i = 0; i < hrp_len; i++) {
        values[i] = hrp[i] >> 5;
        values[hrp_len + 1 + i] = hrp[i] & 0x1f;
    }
    values[hrp_len] = 0;

    memcpy(values + hrp_ext_len, data, datalen);
    memset(values + hrp_ext_len + datalen, 0, 6);

    uint32_t mod = bech32_polymod(values, full_len) ^ 1;

    for (i = 0; i < 6; i++) {
        checksum[i] = (mod >> (5 * (5 - i))) & 0x1f;
    }

}

int bech32_convert_bits(uint8_t *out, size_t *outlen, int outbits, const uint8_t *in, size_t inlen, int inbits, int pad) {
    unsigned int acc = 0;
    int bits = 0;
    size_t max_outlen = *outlen;
    size_t idx = 0;
    unsigned int maxv = (1 << outbits) - 1;

    for (size_t i = 0; i < inlen; i++) {
        acc = (acc << inbits) | in[i];
        bits += inbits;
        while (bits >= outbits) {
            bits -= outbits;
            if (idx >= max_outlen) return 0;
            out[idx++] = (acc >> bits) & maxv;
        }
    }

    if (pad && bits) {
        if (idx >= max_outlen) return 0;
        out[idx++] = (acc << (outbits - bits)) & maxv;
    } else if (bits >= inbits || ((acc << (outbits - bits)) & maxv)) {
        return 0;
    }

    *outlen = idx;

    return 1;
}

int bech32_encode(char *output, const char *hrp, const uint8_t *data, size_t datalen) {
    uint8_t checksum[6];
    bech32_create_checksum(hrp, data, datalen, checksum);

    strcpy(output, hrp);
    size_t hrp_len = strlen(hrp);
    output[hrp_len] = '1';

    for (size_t i = 0; i < datalen; i++) {
        output[hrp_len + 1 + i] = charset[data[i] & 0x1f];
    }

    for (int i = 0; i < 6; i++) {
        output[hrp_len + 1 + datalen + i] = charset[checksum[i]];
    }

    output[hrp_len + 1 + datalen + 6] = '\0';

    return 1;
}
