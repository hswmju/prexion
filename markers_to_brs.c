#include "zutil.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gcrypt.h>
#define NEED_LIBGCRYPT_VERSION "1.6.5"

const char *const marker_secret_start = "BPBPBPB{";
const char *const marker_secret_end = "BPBPBPB}";
const char *const marker_input_start = "TGTGTGT{";
const char *const marker_input_end = "TGTGTGT}";

char *read_file(char *filename, long *fsize)
{
    FILE *f = fopen(filename, "rb");
    fseek(f, 0, SEEK_END);
    *fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *string = (char *)malloc(*fsize + 1);
    fread(string, 1, *fsize, f);
    fclose(f);

    string[*fsize] = '\0';

    return string;
}

size_t write_file(void *buf, char *fname, char *mode, size_t size, size_t n) {
    FILE *fp = fopen(fname, mode);
    if (fp == NULL)
    {
        printf("file open error\n");
        exit(1);
    }

    size_t count = fwrite(buf, size, n, fp);

    fclose(fp);

    return count;
}

void foo(char *in_buf, long len, char *filename) {
    char *last = in_buf;
    char *in_buf_end = in_buf + len;
    char *out_buf = (char *)malloc(sizeof(char) * len);

    unsigned short *next_secret = (unsigned short *)malloc(sizeof(unsigned short) * len);
    unsigned short *next_userinput = (unsigned short *)malloc(sizeof(unsigned short) * len);

    char *cur_secret_start, *cur_userinput_start, *cur_taint_start, *cur_taint_end;
    short cur_taint_type = 0; // 1: secret, 2: user input

    unsigned int i = 0;

    while (last < in_buf_end) {
        cur_secret_start = strstr(last, marker_secret_start);
        cur_userinput_start = strstr(last, marker_input_start);

        if (cur_secret_start == NULL) {
            cur_secret_start = in_buf_end;
        }
        if (cur_userinput_start == NULL) {
            cur_userinput_start = in_buf_end;
        }

        cur_taint_start = (cur_secret_start < cur_userinput_start) ? cur_secret_start : cur_userinput_start;
        cur_taint_type = (cur_secret_start < cur_userinput_start) ? 1 : 2;

        while (last < cur_taint_start) {
            next_secret[i] = ((cur_secret_start - last) > MAX_MATCH) ? MAX_MATCH : (cur_secret_start - last);
            next_userinput[i] = ((cur_userinput_start - last) > MAX_MATCH) ? MAX_MATCH : (cur_userinput_start - last);
            out_buf[i] = *last;
            i++;
            last++;
        }

        if (last < in_buf_end) {
            last += strlen(marker_secret_start);

            if (cur_taint_type == 1) {
                cur_taint_end = strstr(last, marker_secret_end);
            } else if (cur_taint_type == 2) {
                cur_taint_end = strstr(last, marker_input_end);
            } else {
                printf("invalid cur_taint_type %d\n", cur_taint_type);
                exit(1);
            }

            if (cur_taint_end == NULL) {
                printf("invalid cur_taint_end\n");
                exit(1);
            }

            while (last < cur_taint_end) {
                if (cur_taint_type == 1) {
                    next_secret[i] = 0;
                    next_userinput[i] = ((cur_userinput_start - last) > MAX_MATCH) ? MAX_MATCH : (cur_userinput_start - last);
                } else {
                    next_secret[i] = ((cur_secret_start - last) > MAX_MATCH) ? MAX_MATCH : (cur_secret_start - last);
                    next_userinput[i] = 0;
                }

                out_buf[i] = *last;
                i++;
                last++;
            }

            last += strlen(marker_secret_end);
        }
    }

    size_t result = write_file(next_secret, "brs_secret", "wb", sizeof(unsigned short), i);    
    result = write_file(next_userinput, "brs_input", "wb", sizeof(unsigned short), i);

    /**
     * write out_buf to argv[1].nomarkers file
     */
    result = write_file(out_buf, filename, "wb", sizeof(char), i);

    printf("%d", i);

    free(out_buf);
    free(next_userinput);
    free(next_secret);
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("no file specified\n");
        return 1;
    }
    long fsize;
    char *in_buf = read_file(argv[1], &fsize);
    char *last = in_buf;

    char output_filename[300] = "";
    strcat(output_filename, argv[1]);
    strcat(output_filename, ".nomarkers");

    foo(in_buf, fsize, output_filename);
}
