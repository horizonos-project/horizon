#include "blkdev.h"
#include "libk/string.h"
#include "libk/kprint.h"
#include "kernel/log.h"
#include "../fs/ext2.h"
#include "kernel/mbr.h"

static blkdev_t devices[BLKDEV_MAX_DEVICES];

void blkdev_init(void) {
    memset(devices, 0, sizeof(devices));
    klogf("[blkdev] Block device layer initialized\n");
}


blkdev_t* blkdev_register(const char *name, blkdev_ops_t *ops, void *driver_data) {
    for (int i = 0; i < BLKDEV_MAX_DEVICES; i++) {
        if (!devices[i].in_use) {
            strncpy(devices[i].name, name, 15);
            devices[i].name[15] = '\0';
            devices[i].ops = ops;
            devices[i].driver_data = driver_data;
            devices[i].capacity = ops->get_capacity(&devices[i]);
            devices[i].in_use = true;
            
            klogf("[blkdev] Registered device '%s' (%u sectors)\n", 
                    name, devices[i].capacity);
            return &devices[i];
        }
    }
    return NULL;
}

blkdev_t* blkdev_find(const char *name) {
    klogf("[blkdev] Looking for device '%s'\n", name);
    for (int i = 0; i < BLKDEV_MAX_DEVICES; i++) {
        if (devices[i].in_use) {
            klogf("[blkdev] Checking device '%s'\n", devices[i].name);
        }
        if (devices[i].in_use && strcmp(devices[i].name, name) == 0) {
            klogf("[blkdev] Found!\n");
            return &devices[i];
        }
    }
    klogf("[blkdev] Not found!\n");
    return NULL;
}

int blkdev_read(blkdev_t *dev, uint32_t lba, uint8_t *buffer, uint32_t count) {
    if (!dev || !dev->ops || !dev->ops->read) return -1;
    
    return dev->ops->read(
        dev,
        dev->start_lba + lba,
        buffer,
        count
    );
}

int blkdev_write(blkdev_t *dev, uint32_t lba, const uint8_t *buffer, uint32_t count) {
    if (!dev || !dev->ops || !dev->ops->write) return -1;
    return dev->ops->write(dev, lba, buffer, count);
}