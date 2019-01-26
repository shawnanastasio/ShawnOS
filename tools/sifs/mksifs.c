/*
 * A utility for creating SIFS images on a *NIX host.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#include "sifs.h"

void show_usage(const char *name) {
    fprintf(stderr, "Usage: %s [-o output.sifs] files...\n", name);
}

void *malloc_wrapper(size_t s) {
    void *ret = malloc(s);
    if (!ret) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        show_usage(argv[0]);
        return EXIT_FAILURE;
    }

    int arg;
    char *output = "sifs_out.img"; /* default output path */
    while ((arg = getopt(argc, argv, "o:-")) != -1) {
        switch (arg) {
            case 'o':
                output = strdup(optarg);
                break;

            default:
                show_usage(argv[0]);
                return EXIT_FAILURE;
        }
    }
    if (!argv[optind]) {
        fprintf(stderr, "No file(s) specified!\n");
        return EXIT_FAILURE;
    }

    for (int i = optind; argv[i]; i++) {
        printf("Got file: %s\n", argv[i]);
    }


    // Create a sifs image in memory
    void *sifs = malloc_wrapper(sizeof(sifs_header_t));
    sifs_header_t *header = sifs;
    header->sifs_magic = SIFS_MAGIC;
    header->n_inode = 1; // root ino

    // Write the root inode


}
