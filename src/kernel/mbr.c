#include "mbr.h"
#include "log.h"

int mbr_parse(uint8_t *mbr_data, partition_entry_t *partitions_out) {
    mbr_t *mbr = (mbr_t*)mbr_data;

    if (mbr->boot_signature != 0xAA55) {
        klogf("[mbr] Invalid boot signature >> 0x%x << !\n", mbr->boot_signature);
        return -1;
    }

    for (int i = 0; i < 4; i++) {
        partition_entry_t *part = &mbr->partitions[i];

        if (part->partition_type == 0) {
            klogf("[mbr] Partition %d is empty.\n", i);
            continue;
        }

        klogf("[mbr] Partition %d:\n", i);
        klogf("      Type: 0x%02x\n", part->partition_type);
        klogf("      Start LBA: %u\n", part->lba_start);
        klogf("      Sectors: %u\n", part->sector_count);
        klogf("      Size: %u MB\n", (part->sector_count * 512) / (1024 * 1024));
        
        // Copy to output
        partitions_out[i] = *part;
    }

    return 0;
}

partition_entry_t *mbr_find_ext2(partition_entry_t *partitions) {
    for (int i = 0; i < 4; i++) {
        if (partitions[i].partition_type == PART_TYPE_LINUX) {
            return &partitions[i];
        }
    }
}