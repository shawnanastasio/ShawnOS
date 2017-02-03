#pragma once

#include <stdint.h>
#include <stddef.h>

#include <kernel/kernel.h>

// fs_inode flags
#define VFS_FILE        (1<<0)
#define VFS_DIRECTORY   (1<<1)
#define VFS_CHARDEVICE  (1<<2)
#define VFS_BLOCKDEVICE (1<<3)
#define VFS_PIPE        (1<<4)
#define VFS_SYMLINK     (1<<5)
#define VFS_MOUNTPOINT  (1<<6)

// Maximum length of filesystem driver name
#define FS_DRIVER_NAME_LENGTH 10
#define VFS_MAX_SUPERBLOCKS 24

// Maximum filename length
#define VFS_MAX_FILENAME_LEN 128

/**
 * Struct that defines a directory entry
 */
struct fs_dirent {
    char name[VFS_MAX_FILENAME_LEN]; // file name
    uint32_t ino; // inode number
};
typedef struct fs_dirent fs_dirent_t;

/**
 * Struct that defines a single inode
 */
struct fs_inode {
    uint32_t ino;      // Unique inode number
    uint32_t device;   // Device number
    uint32_t mask;     // Permissions mask
    uint32_t uid;      // UID that inode belongs to
    uint32_t gid;      // GID that inode belongs to
    uint32_t flags;    // Type of node
    uint32_t size;     // Size in bytes of inode
    uint32_t refcount; // Reference count

    struct fs_inode *redirect; // Node to redirect to (for symlink/mountpoint)

    struct fs_inode_ops *ops; // Struct containing function pointers to handle inode
};
typedef struct fs_inode fs_inode_t;

/**
 * Struct that defines functions that can act on inodes
 */
struct fs_inode_ops {
    /**
     * Function to read an inode
     * @param node pointer to fs_inode_t to read
     * @param offset offset into inode to begin reading at
     * @param size amount of data to read
     * @param[out] buf pointer to buffer to read data in to
     * @return kernel result code
     */
    k_return_t (*read)(fs_inode_t *node, size_t offset, size_t size, uint8_t *buf);
    /**
     * Function to write an inode
     * @param node pointer to fs_inode_t to write
     * @param size amount of data to write
     * @param[out] buf pointer to buffer containing data to be written
     * @return kernel result code
     */
    k_return_t (*write)(fs_inode_t *node, uint32_t offset, uint32_t size, uint8_t *buf);
    /**
     * Function to open an inode
     * @param node inode to open
     * @return kernel result code
     */
    k_return_t (*open)(fs_inode_t *node);
    /**
     * Function to close an inode
     * @param node inode to open
     * @return kernel result code
     */
    k_return_t (*close)(fs_inode_t *node);
    /**
     * Function to read a directory for the requested index
     * @param node inode of directory to read
     * @param index offset to look for
     * @param[out] res pointer to directory entry to output result to
     * @return kernel result code
     */
    k_return_t (*readdir)(fs_inode_t *node, uint32_t index, fs_dirent_t *res);
    /**
     * Function to find a file in a directory and return its inode
     * @param node directory to search
     * @param name name of file to search for
     * @param[out] res pointer to directory entry to output result to
     * @return kernel result code
     */
    k_return_t (*finddir)(fs_inode_t *node, char *name, fs_dirent_t *res);

};
typedef struct fs_inode_ops fs_inode_ops_t;

/**
 * Struct that defines a superblock, an instance of a mounted filesystem.
 */
struct vfs_superblock {
    uint32_t device;        // Device number of filesystem instance
    fs_inode_t *mount_point; // Mountpoint of filesystem instance, or NULL for /
    fs_inode_t *root;       // Root inode of filesystem instance
};
typedef struct vfs_superblock vfs_superblock_t;

/**
 * Struct that defines an installed filesystem driver.
 * Used in linked list of all installed fs drivers.
 */
struct fs_driver {
    struct fs_driver *next;
    /**
     * Name of filesystem driver
     */
    char name[FS_DRIVER_NAME_LENGTH];
    /**
     * Interface to mount a file system.
     * @param device number of device that contains filesystem to mount
     * @param mount_point VFS mountpoint to mount this fs instance at
     * @param mount_options driver-specific mount options
     * @param[out] res pointer to newly created superblock for FS instance
     * @return kernel result code
     */
    k_return_t (*fs_mount)(uint32_t device, fs_inode_t *mount_point, char *options,
                           vfs_superblock_t *res);
};
typedef struct fs_driver fs_driver_t;

void vfs_install_driver(fs_driver_t *driver);
k_return_t vfs_mount(char *driver, uint32_t device, fs_inode_t *mount_point,
                     char *options);
k_return_t vfs_inode_read(fs_inode_t *node, uint32_t offset, uint32_t size, uint8_t *buf);
