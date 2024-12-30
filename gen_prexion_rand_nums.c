#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#if (PREXION_USE_LIBGCRYPT == 1)
#include <gcrypt.h>
#define NEED_LIBGCRYPT_VERSION "1.6.5"
#endif



size_t write_file(void *buf, char *fname, char *mode, size_t size, size_t n) {
    FILE *fp = fopen(fname, mode);
    if (fp == NULL) {
        printf("file open error\n");
        exit(1);
    }

    size_t count = fwrite(buf, size, n, fp);

    fclose(fp);

    return count;
}


int main(int argc, char **argv) {
    char *pCh;
    unsigned int page_size = 0, i = 0;

    if (argc < 2) {
        puts("not enough arguments");
        return 1;
    }

    // Convert to uint WITH CHECKING!
    page_size = strtoul (argv[1], &pCh, 10);

    // Ensure argument was okay.
    if ((pCh == argv[1]) || (*pCh != '\0')) {
        puts ("Invalid number");
        return 1;
    }

    unsigned char *random_numbers = (unsigned char *)malloc(sizeof(unsigned char) * page_size);

#if (PREXION_USE_LIBGCRYPT == 1)
    unsigned char nonce;

    if (!gcry_check_version (NEED_LIBGCRYPT_VERSION)){
       fprintf (stderr, "libgcrypt is too old (need %s, have %s)\n",
        NEED_LIBGCRYPT_VERSION, gcry_check_version(NULL));
       exit(2);
    }

    /* Disable secure memory.  */
    gcry_control (GCRYCTL_DISABLE_SECMEM, 0);

    /* Tell Libgcrypt that initialization has completed. */
    gcry_control (GCRYCTL_INITIALIZATION_FINISHED, 0);


    for (i=0; i<page_size; i++) {
        gcry_randomize(&nonce, sizeof(nonce), GCRY_WEAK_RANDOM);
        random_numbers[i] = (nonce % 100) + 1;
        //printf("%d ", random_numbers[i]);
    }
#else
    srand(time(NULL));
    for (i=0; i<page_size; i++) {
        random_numbers[i] = (rand() % 100) + 1;
    }
#endif

    size_t result = write_file(random_numbers, "prexion_rand_numbers", "wb", sizeof(unsigned char), page_size);
    free(random_numbers);

}