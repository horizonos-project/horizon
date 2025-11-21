/**
 * @file ata.c
 * @brief ATA/IDE Hard Disk Driver Implementation
 */

#include "ata.h"
#include "blkdev.h"
#include "kernel/io.h"
#include "kernel/log.h"
#include "libk/kprint.h"
#include "libk/string.h"

// Global drive info
static bool drive_initialized = false;
static uint32_t drive_capacity = 0;     // In sectors
static char drive_model[41];            // 40 chars + null terminator

/**
 * @brief Wait for drive to become ready (not busy)
 * 
 * Polls the status register until BSY bit clears.
 * Times out after a reasonable number of iterations.
 */
static int ata_wait_ready(void) {
    uint8_t status;
    int timeout = 100000;  // Arbitrary timeout
    
    while (timeout--) {
        status = inb(ATA_PRIMARY_IO + ATA_REG_STATUS);
        
        // If BSY is clear, drive is ready
        if (!(status & ATA_SR_BSY)) {
            return 0;
        }
    }
    
    return -1;
}

/**
 * @brief 
 * 
 * @param dev 
 * @param lba 
 * @param buffer 
 * @param count 
 * @return int 
 */
static int ata_blkdev_read(blkdev_t *dev, uint32_t lba, uint8_t *buffer, uint32_t count) {
    (void)dev;
    return ata_read_sectors(lba, count, buffer);
}

/**
 * @brief 
 * 
 * @param dev 
 * @param lba 
 * @param buffer 
 * @param count 
 * @return int 
 */
static int ata_blkdev_write(blkdev_t *dev, uint32_t lba, const uint8_t *buffer, uint32_t count) {
    (void)dev;
    // Write one sector at a time for now
    for (uint32_t i = 0; i < count; i++) {
        if (ata_write_sector(lba + i, buffer + (i * 512)) < 0) {
            return i;
        }
    }
    return count;
}

/**
 * @brief 
 * 
 * @param dev 
 * @return uint32_t 
 */
static uint32_t ata_blkdev_get_capacity(blkdev_t *dev) {
    (void)dev;
    return ata_get_capacity();
}

static blkdev_ops_t ata_blkdev_ops = {
    .read = ata_blkdev_read,
    .write = ata_blkdev_write,
    .get_capacity = ata_blkdev_get_capacity,
};

/**
 * @brief Wait for Data Request Ready (DRQ) bit
 * 
 * After sending a read command, wait for the drive to signal
 * that data is ready to be read.
 */
static int ata_wait_drq(void) {
    uint8_t status;
    int timeout = 100000;
    
    while (timeout--) {
        status = inb(ATA_PRIMARY_IO + ATA_REG_STATUS);
        
        // Check for error
        if (status & ATA_SR_ERR) {
            klogf("[ata] Drive error detected\n");
            return -1;
        }
        
        // Check for DRQ
        if (status & ATA_SR_DRQ) {
            return 0;
        }
    }
    
    return -1;
}

/**
 * @brief Read 256 words (512 bytes) from data port
 * 
 * ATA transfers data in 16-bit words, so we read 256 words
 * to get one 512-byte sector.
 */
static void ata_read_buffer(uint16_t *buffer) {
    for (int i = 0; i < 256; i++) {
        buffer[i] = inw(ATA_PRIMARY_IO + ATA_REG_DATA);
    }
}

/**
 * @brief 400ns delay by reading alternate status register
 * 
 * Some ATA operations require a delay. Reading the alternate
 * status register 4 times provides ~400ns delay.
 */
static void ata_delay_400ns(void) {
    for (int i = 0; i < 4; i++) {
        inb(ATA_PRIMARY_CTRL);
    }
}

int ata_init(void) {
    klogf("[ata] Initializing ATA driver...\n");
    
    // Select master drive
    outb(ATA_PRIMARY_IO + ATA_REG_DRIVE, ATA_MASTER);
    ata_delay_400ns();
    
    // Wait for drive to be ready
    if (ata_wait_ready() < 0) {
        klogf("[ata] No drive detected\n");
        return -1;
    }
    
    // Send IDENTIFY command
    outb(ATA_PRIMARY_IO + ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
    ata_delay_400ns();
    
    // Check if drive exists
    uint8_t status = inb(ATA_PRIMARY_IO + ATA_REG_STATUS);
    if (status == 0) {
        klogf("[ata] Drive does not exist\n");
        return -1;
    }
    
    // Wait for DRQ
    if (ata_wait_drq() < 0) {
        return -1;
    }
    
    // Read IDENTIFY data (256 words = 512 bytes)
    uint16_t identify[256];
    ata_read_buffer(identify);
    
    // Parse capacity (words 60-61, 28-bit LBA)
    drive_capacity = ((uint32_t)identify[61] << 16) | identify[60];
    
    // Parse model string (words 27-46, 40 characters)
    // Model string is stored as word-swapped pairs
    for (int i = 0; i < 20; i++) {
        uint16_t word = identify[27 + i];
        drive_model[i * 2] = (word >> 8) & 0xFF;
        drive_model[i * 2 + 1] = word & 0xFF;
    }
    drive_model[40] = '\0';
    
    // Trim trailing spaces
    for (int i = 39; i >= 0 && drive_model[i] == ' '; i--) {
        drive_model[i] = '\0';
    }
    
    drive_initialized = true;
    
    blkdev_t *dev = blkdev_register("hda", &ata_blkdev_ops, NULL);
    if (!dev) {
        klogf("[ata] Failed to register block device!\n");
        return -1;
    }

    klogf("[ata] Registered as /dev/hda\n");
    return 0;
}

int ata_read_sector(uint32_t lba, uint8_t *buffer) {
    if (!drive_initialized) {
        klogf("[ata] Driver not initialized\n");
        return -1;
    }
    
    if (lba >= drive_capacity) {
        klogf("[ata] LBA %u out of range (max %u)\n", lba, drive_capacity - 1);
        return -1;
    }
    
    // Wait for drive to be ready
    if (ata_wait_ready() < 0) {
        return -1;
    }
    
    // Select master drive + LBA mode + highest 4 bits of LBA
    outb(ATA_PRIMARY_IO + ATA_REG_DRIVE, 
         0xE0 | ((lba >> 24) & 0x0F));
    
    // Send sector count (1 sector)
    outb(ATA_PRIMARY_IO + ATA_REG_SECCOUNT, 1);
    
    // Send LBA address (low, mid, high bytes)
    outb(ATA_PRIMARY_IO + ATA_REG_LBA_LOW,  lba & 0xFF);
    outb(ATA_PRIMARY_IO + ATA_REG_LBA_MID,  (lba >> 8) & 0xFF);
    outb(ATA_PRIMARY_IO + ATA_REG_LBA_HIGH, (lba >> 16) & 0xFF);
    
    // Send READ command
    outb(ATA_PRIMARY_IO + ATA_REG_COMMAND, ATA_CMD_READ_PIO);
    
    // Wait for data to be ready
    if (ata_wait_drq() < 0) {
        return -1;
    }
    
    // Read data (256 words = 512 bytes)
    ata_read_buffer((uint16_t*)buffer);
    
    return 0;
}

int ata_read_sectors(uint32_t lba, uint8_t count, uint8_t *buffer) {
    if (count == 0 || count > ATA_MAX_SECTORS) {
        klogf("[ata] Invalid sector count: %u\n", count);
        return -1;
    }
    
    // Read sectors one at a time
    // TODO: Optimize by sending multi-sector read command
    for (uint8_t i = 0; i < count; i++) {
        if (ata_read_sector(lba + i, buffer + (i * ATA_SECTOR_SIZE)) < 0) {
            klogf("[ata] Failed to read sector %u\n", lba + i);
            return i;  // Return number of sectors successfully read
        }
    }
    
    return count;
}

int ata_write_sector(uint32_t lba, const uint8_t *buffer) {
    if (!drive_initialized) {
        klogf("[ata] Driver not initialized!\n");
        return -1;
    }

    if (lba >= drive_capacity) {
        klogf("[ata] LBA %u out of range (max %u)\n", lba, drive_capacity - 1);
        return -1;
    }

    // Wait for drive to be ready (major important)
    if (ata_wait_ready() < 0) {
        return -1;
    }
    
    // Select master drive + LBA mode + highest 4 bits of LBA
    outb(ATA_PRIMARY_IO + ATA_REG_DRIVE, 
         0xE0 | ((lba >> 24) & 0x0F));
    
    // Send sector count (1 sector)
    outb(ATA_PRIMARY_IO + ATA_REG_SECCOUNT, 1);
    
    // Send LBA address
    outb(ATA_PRIMARY_IO + ATA_REG_LBA_LOW,  lba & 0xFF);
    outb(ATA_PRIMARY_IO + ATA_REG_LBA_MID,  (lba >> 8) & 0xFF);
    outb(ATA_PRIMARY_IO + ATA_REG_LBA_HIGH, (lba >> 16) & 0xFF);
    
    // Send WRITE command
    outb(ATA_PRIMARY_IO + ATA_REG_COMMAND, ATA_CMD_WRITE_PIO);
    
    // Wait for drive to be ready for data
    if (ata_wait_drq() < 0) {
        return -1;
    }
    
    // Write data (256 words = 512 bytes)
    const uint16_t *words = (const uint16_t*)buffer;
    for (int i = 0; i < 256; i++) {
        outw(ATA_PRIMARY_IO + ATA_REG_DATA, words[i]);
    }
    
    // Flush cache to ensure data is written
    outb(ATA_PRIMARY_IO + ATA_REG_COMMAND, ATA_CMD_CACHE_FLUSH);
    
    // Wait for flush to complete
    if (ata_wait_ready() < 0) {
        return -1;
    }
    
    return 0;
}

bool ata_drive_present(void) {
    return drive_initialized;
}

uint32_t ata_get_capacity(void) {
    return drive_capacity;
}

void ata_print_info(void) {
    if (!drive_initialized) {
        kprintf("[ata] No drive initialized\n");
        return;
    }
    
    kprintf("[ata] === Drive Information ===\n");
    kprintf("[ata] Model: %s\n", drive_model);
    kprintf("[ata] Capacity: %u sectors\n", drive_capacity);
    kprintf("[ata] Size: %u MB\n", (drive_capacity * 512) / (1024 * 1024));
    kprintf("[ata] ===========================\n");
}