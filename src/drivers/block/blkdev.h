/**
 * @file blkdev.h
 * @brief Block Device Abstraction Layer
 */

#ifndef BLKDEV_H
#define BLKDEV_H

#include <stdint.h>
#include <stdbool.h>

#define BLKDEV_SECTOR_SIZE 512
#define BLKDEV_MAX_DEVICES 8

typedef struct blkdev blkdev_t;

/**
 * @brief Block device operations
 */
typedef struct {
    int (*read)(blkdev_t *dev, uint32_t lba, uint8_t *buffer, uint32_t count);
    int (*write)(blkdev_t *dev, uint32_t lba, const uint8_t *buffer, uint32_t count);
    uint32_t (*get_capacity)(blkdev_t *dev);
} blkdev_ops_t;

/**
 * @brief Block device structure
 */
struct blkdev {
    char name[16];              // e.g., "hda", "sda"
    uint32_t capacity;          // Sectors
    void *driver_data;          // Driver-specific data
    blkdev_ops_t *ops;          // Operations
    bool in_use;
};

/**
 * @brief Initialize block device subsystem
 */
void blkdev_init(void);

/**
 * @brief Register a block device
 * @return Device handle or NULL on failure
 */
blkdev_t* blkdev_register(const char *name, blkdev_ops_t *ops, void *driver_data);

/**
 * @brief Find a block device by name
 * @return Device handle or NULL if not found
 */
blkdev_t* blkdev_find(const char *name);

/**
 * @brief Read sectors from a block device
 */
int blkdev_read(blkdev_t *dev, uint32_t lba, uint8_t *buffer, uint32_t count);

/**
 * @brief Write sectors to a block device
 */
int blkdev_write(blkdev_t *dev, uint32_t lba, const uint8_t *buffer, uint32_t count);

#endif // BLKDEV_H