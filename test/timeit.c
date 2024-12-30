#include "zlib.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// #define TRIALS 10  // 10
// #define WARMUPS 5 // 10
#define TRIALS 1000  // 10
#define WARMUPS 200 // 10
#define BUFLEN 16384
#define MAX_NAME_LEN 1024
#define MAX_UNSAFE_LEN 1024


#define local

void *   myalloc(q, n, m) void *q;
unsigned n, m;
{
    q = Z_NULL;
    return calloc(n, m);
}

void myfree(q, p) void *q, *p;
{
    q = Z_NULL;
    free(p);
}

typedef struct gzFile_st {
    // We aren't writing out to a file
    // FILE *file;
    int      write;
    int      err;
    char *   msg;
    z_stream strm;
} * gzFilet;

const char *gzerrort(gz, err) gzFilet gz;
int *       err;
{
    *err = gz->err;
    return gz->msg;
}

int gzcloset(gz) gzFilet gz;
{
    z_stream *    strm;
    unsigned char out[BUFLEN];

    if (gz == NULL)
        return Z_STREAM_ERROR;
    strm = &(gz->strm);
    if (gz->write) {
        strm->next_in = Z_NULL;
        strm->avail_in = 0;
        do {
            strm->next_out = out;
            strm->avail_out = BUFLEN;
            (void)deflate(strm, Z_FINISH);
        } while (strm->avail_out == 0);
        // Don't want to free the state
        // debreachEnd(strm);
    }
    // fclose(gz->file);
    // free(gz);
    return Z_OK;
}

#ifdef PREXION
gzFilet gz_opent(mode, next_secret, next_userinput, prexion_rand_numbers, buf_len)
const char *mode;
unsigned short *next_secret;
unsigned short *next_userinput;
unsigned char *prexion_rand_numbers;
unsigned int buf_len;
{
    gzFilet gz;
    int     ret;

    gz = malloc(sizeof(struct gzFile_st));
    if (gz == NULL)
        return NULL;
    gz->write = strchr(mode, 'w') != NULL;
    gz->strm.zalloc = myalloc;
    gz->strm.zfree = myfree;
    gz->strm.opaque = Z_NULL;
    if (gz->write) {
        ret = prexionInit2(&(gz->strm), -1, 8, 15 + 16, 8, 0, next_secret, next_userinput, prexion_rand_numbers, buf_len);
    }
    else {
        gz->strm.next_in = 0;
        gz->strm.avail_in = Z_NULL;
        ret = inflateInit2(&(gz->strm), 15 + 16);
    }
    if (ret != Z_OK) {
        free(gz);
        return NULL;
    }
    // Not writing out to file
    // gz->file = path == NULL ? fdopen(fd, gz->write ? "wb" : "rb") :
    //                          fopen(path, gz->write ? "wb" : "rb");
    // if (gz->file == NULL) {
    //    gz->write ? debreachEnd(&(gz->strm)) : inflateEnd(&(gz->strm));
    //    free(gz);
    //    return NULL;
    //}
    gz->err = 0;
    gz->msg = "";
    return gz;
}
#else
gzFilet gz_opent(mode) const char *mode;
{
    gzFilet gz;
    int     ret;

    gz = malloc(sizeof(struct gzFile_st));
    if (gz == NULL)
        return NULL;
    gz->write = strchr(mode, 'w') != NULL;
    gz->strm.zalloc = myalloc;
    gz->strm.zfree = myfree;
    gz->strm.opaque = Z_NULL;
    if (gz->write) {
        ret = deflateInit2(&(gz->strm), -1, 8, 15 + 16, 8, 0);
    }
    else {
        gz->strm.next_in = 0;
        gz->strm.avail_in = Z_NULL;
        ret = inflateInit2(&(gz->strm), 15 + 16);
    }
    if (ret != Z_OK) {
        free(gz);
        return NULL;
    }
    // Not writing out to file
    // gz->file = path == NULL ? fdopen(fd, gz->write ? "wb" : "rb") :
    //                          fopen(path, gz->write ? "wb" : "rb");
    // if (gz->file == NULL) {
    //    gz->write ? debreachEnd(&(gz->strm)) : inflateEnd(&(gz->strm));
    //    free(gz);
    //    return NULL;
    //}
    gz->err = 0;
    gz->msg = "";
    return gz;
}
#endif

int         gzwritet(gz, buf, len) gzFilet gz;
const void *buf;
unsigned    len;
{
    z_stream *    strm;
    unsigned char out[BUFLEN];
    if (gz == NULL || !gz->write)
        return 0;
    strm = &(gz->strm);
    strm->next_in = (void *)buf;
    strm->avail_in = len;
    do {
        strm->next_out = out;
        strm->avail_out = BUFLEN;
        (void)deflate(strm, Z_NO_FLUSH);
    } while (strm->avail_out == 0);
    return len;
}

void    gz_compress(in, out) FILE *in;
gzFilet out;
{
    local char buf[BUFLEN];
    int        len;
    int        err;

    for (;;) {
        len = (int)fread(buf, 1, sizeof(buf), in);
        if (ferror(in)) {
            perror("fread");
            exit(1);
        }
        if (len == 0)
            break;

        if (gzwritet(out, buf, (unsigned)len, NULL) != len) {
            printf("write error\n");
            exit(1);
        }
    }
    // fclose(in);
    if (gzcloset(out) != Z_OK)
        printf("failed gzclose\n");
}

int isNumber(str) char *str;
{
    char *temp = str;
    while (*temp != '\0') {
        if (isdigit((int)*temp) == 0)
            return 0;
        temp++;
    }
    return 1;
}

int  charCount(str, c) char *str;
char c;
{
    int   ct = 0;
    char *temp = str;
    while (*temp != '\0') {
        if (*temp == c)
            ct++;
        temp++;
    }
    return ct;
}


#ifdef PREXION
unsigned short *read_brs_file(char *filename, unsigned int *len) {
    FILE *in_fp = fopen(filename, "rb");
    if (in_fp == NULL) {
#if (PREXION_DEBUG_LEVEL >= 1)
        printf("read_brs_file: file open error\n");
#endif
        return NULL;
    }

    fseek(in_fp, 0, SEEK_END);
    long file_size = ftell(in_fp);
    fseek(in_fp, 0, SEEK_SET);
#if (PREXION_DEBUG_LEVEL >= 1)
    printf("read_brs_file: file_size=%ld\n", file_size);
#endif

    unsigned short *read_next_secret = (unsigned short *)malloc(file_size);

    size_t read_count = fread(read_next_secret, file_size, 1, in_fp);

    fclose(in_fp);

    if (file_size % sizeof(unsigned short) != 0) {
#if (PREXION_DEBUG_LEVEL >= 1)
        printf("read_brs_file: invalid file_size %ld\n", file_size);
#endif
        free(read_next_secret);
        return NULL;
    }

    *len = (file_size / sizeof(unsigned short));

    return read_next_secret;
}

void *read_rand_nums_file(char *filename, unsigned int *len) {
        FILE *in_fp = fopen(filename, "rb");
        if (in_fp == NULL) {
#if (PREXION_DEBUG_LEVEL >= 1)
            printf("read_rand_nums_file: file open error\n");
#endif
            return NULL;
        }

        fseek(in_fp, 0, SEEK_END);
        long file_size = ftell(in_fp);
        fseek(in_fp, 0, SEEK_SET);

        void *in_buf = malloc(file_size);

        size_t read_count = fread(in_buf, file_size, 1, in_fp);

        fclose(in_fp);

        *len = file_size;

        return in_buf;
    }
#endif

int   main(argc, argv) int argc;
char *argv[];
{
#ifdef PREXION
    unsigned int    next_secret_len = 0, next_userinput_len = 0, prexion_rand_numbers_len = 0;
    unsigned short *next_secret = NULL;
    unsigned short *next_userinput = NULL;
    unsigned char *prexion_rand_numbers = NULL;

    next_secret = read_brs_file("brs_secret", &next_secret_len);
    next_userinput = read_brs_file("brs_input", &next_userinput_len);
    prexion_rand_numbers = (unsigned char *)read_rand_nums_file("prexion_rand_numbers", &prexion_rand_numbers_len);
#if (PREXION_DEBUG_LEVEL >= 1)
    printf("next_secret_len=%d, next_userinput_len=%d\n", next_secret_len, next_userinput_len);
#endif
#endif

    // Compression data structures that we only want to allocate once
    FILE *in;
    in = fopen(argv[1], "rb");
#ifdef PREXION
    gzFilet out = gz_opent("w", next_secret, next_userinput, prexion_rand_numbers, next_secret_len);
#else
    gzFilet out = gz_opent("w");
#endif

    int     t;
    clock_t start, diff;

    for (t = 0; t < WARMUPS; t++) {
        fseek(in, 0, SEEK_SET);
        deflateReset(&(out->strm));
        gz_compress(in, out);
    }

    for (t = 0; t < TRIALS; t++) {
        struct timespec tstart = {0, 0}, tend = {0, 0};
        clock_gettime(CLOCK_MONOTONIC, &tstart);

        fseek(in, 0, SEEK_SET);
        deflateReset(&(out->strm));
        gz_compress(in, out);

        clock_gettime(CLOCK_MONOTONIC, &tend);
        printf("%.9f ", ((double)tend.tv_sec + 1.0e-9 * tend.tv_nsec) -
                            ((double)tstart.tv_sec + 1.0e-9 * tstart.tv_nsec));
    }

    // Cleanup
    deflateEnd(&(out->strm));
    free(out);
    fclose(in);
    // free(taint);
#ifdef PREXION
    free(next_secret);
    free(next_userinput);
    free(prexion_rand_numbers);
#endif
    return 0;
}
