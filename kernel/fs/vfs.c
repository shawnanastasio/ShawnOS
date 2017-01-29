/**
 * Kernel interface for filesystem operations
 */

#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include <kernel/kernel.h>
#include <fs/vfs.h>
#include <mm/alloc.h>

// Array containing all mounted superblocks
static vfs_superblock_t *vfs_superblocks[VFS_MAX_SUPERBLOCKS];
uint32_t vfs_superblocks_size = 0;

// Linked list containing all installed drivers
fs_driver_t *vfs_drivers_head;

// Current filesystem root
fs_inode_t *vfs_root = NULL;

// Refcount lock
SPINLOCK_DECLARE(vfs_refcount);

/**
 * Internal functions
 */

/**
 * Return a pointer to the driver for the requested filesystem
 * @param  driver name of filesystem driver
 * @return        pointer to driver, or NULL if no driver exists
 */
static inline fs_driver_t *vfs_get_driver(char *driver) {
    // Search installed drivers linked list for requested driver
    fs_driver_t *cur = vfs_drivers_head;
    while (cur) {
        if (strcmp(driver, cur->name) == 0) {
            return cur;
        }
        cur = cur->next;
    }
    return NULL;
}


/**
 * Interface for installing drivers
 */
void vfs_install_driver(fs_driver_t *driver) {
    // Allocate space for a the new driver
    fs_driver_t *new = (fs_driver_t *)kmalloc(sizeof(fs_driver_t));

    // Copy the driver into newly allocated memory
    memcpy(new, driver, sizeof(fs_driver_t));

    // Set new driver to the head of the vfs_drivers linked list
    new->next = vfs_drivers_head;
    vfs_drivers_head = new;
}


/**
 * Interfaces for mounting/unmounting filesystem instances
 */

/**
 * Mount a filesystem instance
 * @param  driver      name of driver to use
 * @param  device      device number containing filesystem
 * @param  mount_point node to mount fs at, or NULL for root
 * @param  options     filesystem-specific mount options
 * @return             kernel result code
 */
k_return_t vfs_mount(char *driver, uint32_t device, fs_inode_t *mount_point,
                     char *options) {
    // Get the driver struct for the requested driver
    fs_driver_t *driver_info = vfs_get_driver(driver);
    if (!driver_info) {
        // No driver installed for requested filesystem
        return -K_NOTSUP;
    }

    // Call driver's mount_fs and get returned superblock
    vfs_superblock_t *res = driver_info->fs_mount(device, mount_point, options);
    if (!res) {
        // Driver failed to mount filesystem
        return -K_FAILED;
    }

    // Add superblock to internal list
    if (vfs_superblocks_size + 1 > VFS_MAX_SUPERBLOCKS) {
        // Not enough space in superblocks list
        return -K_OOM;
    }
    vfs_superblocks[vfs_superblocks_size++] = res;

    // Mount filesystem
    if (mount_point) {
        // Mount at given inode
        // Make sure node isn't already a mountpoint
        ASSERT(!(mount_point & VFS_MOUNTPOINT) && !(mount_point->redirect));
        mount_point->flags &= VFS_MOUNTPOINT;
        mount_point->redirect = res->root;
    } else {
        // Mount at root
        vfs_root = res->root;
    }
    return K_SUCCESS;
}


/**
 * Interfaces to read inodes
 */

/**
 * Read an inode
 * @param node pointer to fs_inode_t to read
 * @param offset offset into inode to begin reading at
 * @param size amount of data to read
 * @param[out] buf pointer to buffer to read data in to
 * @return kernel result code
 */
k_return_t vfs_inode_read(fs_inode_t *node, uint32_t offset, uint32_t size, uint8_t *buf) {
    fs_inode_ops_t *ops = node->ops;
    // Make sure this inode has the requested operation
    if (ops && ops->read) {
        return ops->read(node, offset, size, buf);
    }
    return -K_UNIMPL;
}

/**
 * Write an inode
 * @param node pointer to fs_inode_t to write
 * @param size amount of data to write
 * @param[out] buf pointer to buffer containing data to be written
 * @return kernel result code
 */
k_return_t vfs_inode_write(fs_inode_t *node, uint32_t offset, uint32_t size, uint8_t *buf) {
    fs_inode_ops_t *ops = node->ops;
    // Make sure this inode has the requested operation
    if (ops && ops->write) {
        return ops->write(node, offset, size, buf);
    }
    return -K_UNIMPL;
}

/**
 * Open an inode
 * @param node inode to open
 * @return kernel result code
 */
k_return_t vfs_inode_open(fs_inode_t *node) {
    fs_inode_ops_t *ops = node->ops;
    // Make sure this inode has the requested operation
    if (ops && ops->open) {
        // Increase refcount
        SPINLOCK_LOCK(vfs_refcount);
        node->refcount++;
        SPINLOCK_UNLOCK(vfs_refcount);
        return ops->open(node);
    }
    return -K_UNIMPL;
}

/**
 * Close an inode
 * @param node inode to open
 * @return kernel result code
 */
k_return_t vfs_inode_close(fs_inode_t *node) {
    fs_inode_ops_t *ops = node->ops;
    // Make sure this inode has the requested operation
    if (ops && ops->close) {
        // Decrease refcount
        SPINLOCK_LOCK(vfs_refcount);
        node->refcount--;
        SPINLOCK_UNLOCK(vfs_refcount);
        return ops->close(node);
    }
    return -K_UNIMPL;
}

/**
 * Read a directory for the requested index
 * @param node inode of directory to read
 * @param index offset to look for
 * @param[out] res pointer to directory entry to output result to
 * @return kernel result code
 */
k_return_t vfs_inode_readdir(fs_inode_t *node, uint32_t num, fs_dirent_t *res) {
    fs_inode_ops_t *ops = node->ops;
    // Make sure this inode has the requested operation
    if (ops && ops->readdir) {
        return ops->readdir(node, num, res);
    }
    return -K_UNIMPL;
}

/**
 * Find a file in a directory and return its inode
 * @param node directory to search
 * @param name name of file to search for
 * @param[out] res pointer to directory entry to output result to
 * @return kernel result code
 */
k_return_t vfs_inode_finddir(fs_inode_t *node, char *name, fs_dirent_t *res) {
    fs_inode_ops_t *ops = node->ops;
    // Make sure this inode has the requested operation
    if (ops && ops->finddir) {
        return ops->finddir(node, name, res);
    }
    return -K_UNIMPL;
}
