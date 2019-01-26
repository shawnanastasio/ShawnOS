#pragma once

#include <stdint.h>

// SIFS is a simple read-only filesystem designed for initramfs images.
// Files are limited to 4GB.
// Data regions of directory = an array of children inodes
// Root = ino=0 directory

#define SIFS_MAGIC 0x51F5EEEE
#define SIFS_MAX_NAME_LEN 256

#define SIFS_INO_FLAG_DIR (1 << 0) // inode is a directory

typedef struct sifs_header {
    uint32_t sifs_magic; // Should be SIFS_MAGIC
    uint32_t n_inode;    // Total number of inodes
} sifs_header_t;

typedef struct sifs_inode {
    uint64_t data_off; // Offset of the data region from the start of the SIFS image
    uint32_t size;  // Size of data region
    uint32_t ino;   // inode number
    uint8_t flags;  // SIFS_INO_FLAG*
    char name[SIFS_MAX_NAME_LEN]; // File name. Null terminated
} sifs_inode_t;
