#include "blkdev.h"
#include "libk/string.h"
#include "libk/kprint.h"
#include "kernel/log.h"
#include "kernel/mbr.h"
#include "../fs/ext2.h"
#include "kernel/mbr.h"

static blkdev_t devices[BLKDEV_MAX_DEVICES];

void blkdev_init(void) {
    memset(devices, 0, sizeof(devices));
    klogf("[blkdev] Block device layer initialized\n");
}

/**
 * @brief
 * @param
 */
static void blkdev_scan_partitions(blkdev_t *disk) {
    uint8_t sector[512];

    if (blkdev_read(disk, 0, sector, 1) < 0) {
        kprintf("[part] Failed to read MBR\n");
        return;
    }

    mbr_t *mbr = (mbr_t*)sector;

    if (mbr->disk_signature != 0xAA55) {
        kprintf("[part] No MBR on %s (sig=0x%x)\n",
                disk->name, mbr->disk_signature);
        return;
    }

    kprintf("[part] MBR detected on %s\n", disk->name);

    for (int i = 0; i < 4; i++) {
        mbr_partition_t *p = &mbr->partitions[i];

        if (p->partition_type == PART_TYPE_EMPTY || p->sector_count == 0)
            continue;

        char name[16];
        blkdev_make_part_name(name, disk->name, i + 1);

        blkdev_t *part = blkdev_register(
            name,
            disk->ops,
            disk->driver_data
        );

        if (!part) {
            kprintf("[part] Failed to register %s\n", name);
            continue;
        }

        part->start_lba = p->lba_start;
        part->capacity  = p->sector_count;

        kprintf("[part] %s: type=0x%x start=%u size=%u\n",
                name,
                p->partition_type,
                p->lba_start,
                p->sector_count);
    }
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

void blkdev_make_part_name(char *out, const char *disk_name, int partno) {
    // Doing this the old fashioned way since snprintf isn't real

    int i = 0;

    // Disk name
    while (disk_name[i] && i < 15) {
        out[i] = disk_name[i];
        i++;
    }

    // Partno
    if (partno >= 0 && partno <= 9 && i < 15) {
        out[i++] = '0' + partno;
    }

    out[i] = '\0';
}