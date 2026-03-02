#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <argon2.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <openssl/hmac.h>
#include <openssl/sha.h>
#include "bech32.h"

#define MASTER_LEN 64
#define OUT_LEN 32
#define MAX_PASS_LEN 256

static const char *salt = "age-keygen-deterministic-hardcoded-salt";

void derive_master(const char *pass, uint8_t *out) {

    if (argon2id_hash_raw(
            10,                // time_cost
            65536,             // mem_cost (KiB)
            2,                 // lanes
            pass, strlen(pass),
            salt, strlen(salt),
            out, MASTER_LEN) != ARGON2_OK) {
        fprintf(stderr, "Argon2 failed\n");
        exit(1);
    }

}

void derive_key(uint8_t *master, uint64_t index, uint8_t *out) {
    unsigned int len;
    uint8_t idx_be[8];

    for (int i = 0; i < 8; i++) {
        idx_be[7 - i] = (index >> (i * 8)) & 0xff;
    }

    HMAC(EVP_sha256(), master, MASTER_LEN, idx_be, 8, out, &len);

}

void print_bech32(uint8_t *data32) {
    uint8_t data5[52];
    size_t data5_len = sizeof(data5);
    char output[200];

    bech32_convert_bits(data5, &data5_len, 5,
                        data32, OUT_LEN, 8, 1);

    bech32_encode(output, "age-secret-key-", data5, data5_len);

    for (char *p = output; *p; ++p) {
        *p = toupper(*p);
    }

    printf("%s\n", output);

}

char* get_input(char* buf, size_t size, const char* prompt, int echo) {
    int tty_fd = open("/dev/tty", O_RDWR);

    if (tty_fd < 0) {
        fprintf(stderr, "Warning: /dev/tty not available, falling back to stdin\n");
        tty_fd = STDIN_FILENO;
    }

    FILE *tty_out = fdopen(dup(tty_fd), "w");
    FILE *tty_in = fdopen(tty_fd, "r");

    if (prompt) {
        fprintf(tty_out, "%s", prompt);
        fflush(tty_out);
    }

    struct termios oldt, newt;
    if (!echo) {
        tcgetattr(fileno(tty_in), &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ECHO);
        tcsetattr(fileno(tty_in), TCSANOW, &newt);
    }

    if (!fgets(buf, size, tty_in)) {
        if (!echo) tcsetattr(fileno(tty_in), TCSANOW, &oldt);
        fclose(tty_in); fclose(tty_out);
        return NULL;
    }

    if (!echo) {
        tcsetattr(fileno(tty_in), TCSANOW, &oldt);
        fprintf(tty_out, "\n");
    }

    fclose(tty_in);
    fclose(tty_out);

    size_t len = strlen(buf);
    if (len > 0 && buf[len-1] == '\n') buf[len-1] = '\0';

    return buf;
}

void print_usage(const char *bin_name) {

    fprintf(stderr, 
        "age-keygen-deterministic\n"
        "original tool (c) Klaus Eisentraut\n"
        "Tool for deterministic age key generation from passphrase.\n\n"
        "USAGE:\n"
        "    %s [OPTIONS]\n\n"
        "OPTIONS:\n"
        "    -c, --count <COUNT>            number of keys to generate [default: 1]\n"
        "    -o, --offset <OFFSET>          starting index of the keys [default: 0]\n"
        "    -s, --stdin                    read passphrase from stdin\n"
        "    -d, --doublecheck-passphrase   confirm passphrase if entered interactively\n"
        "    -h, --help                     print this help information\n", 
        bin_name);

    exit(0);
}

int main(int argc, char **argv) {
    uint64_t count = 1, offset = 0;
    int double_check = 0;
    const char *pass = NULL;
    char p1[MAX_PASS_LEN], p2[MAX_PASS_LEN];

    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
            print_usage(argv[0]);
        } else if (!strcmp(argv[i], "-c") || !strcmp(argv[i], "--count")) {
            if (++i > argc) {
                fprintf(stderr, "Error: COUNT is missing\n");
                return 1;
            }
            count = strtoull(argv[i], NULL, 10);
            if (!count) {
                fprintf(stderr, "COUNT is 0. nothing to do\n");
                return 1;
            }
        } else if (!strcmp(argv[i], "-o") || !strcmp(argv[i], "--offset")) {
            if (++i > argc) {
                fprintf(stderr, "Error: OFFSET is missing\n");
                return 1;
            }
            offset = strtoull(argv[i], NULL, 10);
        } else if (!strcmp(argv[i], "-s") || !strcmp(argv[i], "--stdin")) {
            if (!fgets(p1, sizeof(p1), stdin)) return 1;
            size_t len = strlen(p1);
            if (len > 0 && p1[len-1] == '\n') p1[len-1] = '\0';
            pass = p1;
        } else if (!strcmp(argv[i], "-d") || !strcmp(argv[i], "--doublecheck-passphrase")) {
            double_check = 1;
        } else {
            fprintf(stderr, "error: Found argument '%s' which wasn't expected, or isn't valid in this context\n", argv[i]);
            return 1;
        }
    }

    if (!pass) {
        if (!get_input(p1, sizeof(p1), "Enter passphrase: ", 0)) return 1;
        if (double_check) {
            if (!get_input(p2, sizeof(p2), "Confirm passphrase: ", 0)) return 1;
            if (strcmp(p1, p2) != 0) {
                fprintf(stderr, "Error: Passphrases do not match\n");
                return 1;
            }
        }
        pass = p1;
    }

    if (strlen(pass) < 16) {
        fprintf(stderr, "Error: Passphrase must be at least 16 characters\n");
        return 1;
    }

    uint8_t master[MASTER_LEN];
    derive_master(pass, master);

    for (uint64_t i = offset; i < offset + count; i++) {
        uint8_t key[OUT_LEN];
        derive_key(master, i, key);
        printf("# secret key %llu below\n", (unsigned long long)i);
        print_bech32(key);
    }

    memset(p1, 0, sizeof(p1));
    memset(p2, 0, sizeof(p2));
    return 0;
}
