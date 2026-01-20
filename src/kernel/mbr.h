// master boot record header

#ifndef MBR_H   
#define MBR_H

#include <stdint.h>

// Even though we don't use most of these, it's good to know they exist.

#define PART_TYPE_EMPTY     0x00
#define PART_TYPE_FAT16     0x06
#define PART_TYPE_NTFS      0x07
#define PART_TYPE_FAT32     0x0B
#define PART_TYPE_LINUX     0x83  // ext2/ext3/ext4
#define PART_TYPE_SWAP      0x82
#define PART_TYPE_LVM       0x8E

typedef struct {
    uint8_t  status;        // 0x80 = bootable, 0x00 = not bootable
    uint8_t  first_chs[3];  // CHS address (we ignore this)
    uint8_t  partition_type; // 0x83 = Linux/ext2!
    uint8_t  last_chs[3];   // CHS address (we ignore this)
    uint32_t lba_start;     // LBA start sector (we need this)
    uint32_t sector_count;  // Number of sectors (and this as well)
} __attribute__((packed)) partition_entry_t;

typedef struct {
    uint8_t bootstrap[440];
    uint32_t disk_signature;
    uint16_t reserved;
    partition_entry_t partitions[4];  // The good stuff!
    uint16_t boot_signature;          // Must be 0xAA55
} __attribute__((packed)) mbr_t;

int mbr_parse(uint8_t *mbr_data, partition_entry_t *partitions_out);
partition_entry_t* mbr_find_ext2(partition_entry_t *partitions);

typedef partition_entry_t mbr_partition_t; // Mild band-aid for blkdev.c. Awesome.

#endif // MBR_H