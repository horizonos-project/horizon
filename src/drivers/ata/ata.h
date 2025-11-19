/**
 * @file ata.h
 * @brief ATA/IDE Hard Disk Driver
 * 
 * Implements PIO (Programmed Input/Output) mode access to ATA/IDE hard disks.
 * This is the 90s way of talking to hard drives - simple, slow, but it works!
 * 
 * ATA (AT Attachment) is the interface used by IDE hard drives. We implement
 * 28-bit LBA (Logical Block Addressing) mode for simplicity.
 * 
 * Features:
 * - Read sectors from primary master drive (0x1F0 port range)
 * - 28-bit LBA addressing (up to 128GB disks)
 * - Polling mode (no DMA, no interrupts yet)
 * - Single-sector and multi-sector reads
 * 
 * Limitations:
 * - PIO mode is SLOW (transfers via CPU, blocks everything)
 * - No DMA support (yet)
 * - No write support (yet - read-only is safer!)
 * - Only supports primary master drive
 * 
 * Modern alternatives we're ignoring:
 * - AHCI (too complex)
 * - NVMe (requires PCIe, even more complex)
 * - DMA transfers (requires setting up PRDT tables)
 * 
 * @note ATA is also called PATA (Parallel ATA) to distinguish from SATA
 */

#ifndef ATA_H
#define ATA_H

#include <stdint.h>
#include <stdbool.h>


/** @brief Primary ATA bus I/O port base (master and slave) */
#define ATA_PRIMARY_IO      0x1F0

/** @brief Primary ATA bus control port base */
#define ATA_PRIMARY_CTRL    0x3F6

/** @brief Secondary ATA bus I/O port base (we don't use this yet) */
#define ATA_SECONDARY_IO    0x170

/** @brief Secondary ATA bus control port base */
#define ATA_SECONDARY_CTRL  0x376

/** @brief Data register (16-bit, for reading/writing data) */
#define ATA_REG_DATA        0x00

/** @brief Error register (read) / Features register (write) */
#define ATA_REG_ERROR       0x01
#define ATA_REG_FEATURES    0x01

/** @brief Sector count register */
#define ATA_REG_SECCOUNT    0x02

/** @brief LBA low byte (bits 0-7) */
#define ATA_REG_LBA_LOW     0x03

/** @brief LBA mid byte (bits 8-15) */
#define ATA_REG_LBA_MID     0x04

/** @brief LBA high byte (bits 16-23) */
#define ATA_REG_LBA_HIGH    0x05

/** @brief Drive/Head register (bits 24-27 of LBA + drive select) */
#define ATA_REG_DRIVE       0x06

/** @brief Status register (read) / Command register (write) */
#define ATA_REG_STATUS      0x07
#define ATA_REG_COMMAND     0x07

/** @brief Error bit - an error occurred */
#define ATA_SR_ERR          0x01

/** @brief Index bit (obsolete) */
#define ATA_SR_IDX          0x02

/** @brief Corrected data (obsolete) */
#define ATA_SR_CORR         0x04

/** @brief Data request - drive is ready to transfer data */
#define ATA_SR_DRQ          0x08

/** @brief Drive seek complete (obsolete) */
#define ATA_SR_DSC          0x10

/** @brief Drive fault error */
#define ATA_SR_DF           0x20

/** @brief Drive ready */
#define ATA_SR_DRDY         0x40

/** @brief Drive busy - wait before sending commands */
#define ATA_SR_BSY          0x80

/** @brief Read sectors with retry (old command) */
#define ATA_CMD_READ_PIO    0x20

/** @brief Read sectors with retry (28-bit LBA) */
#define ATA_CMD_READ_PIO_EXT 0x24

/** @brief Write sectors with retry (28-bit LBA) */
#define ATA_CMD_WRITE_PIO   0x30

/** @brief Write sectors with retry (48-bit LBA) */
#define ATA_CMD_WRITE_PIO_EXT 0x34

/** @brief Flush write cache */
#define ATA_CMD_CACHE_FLUSH 0xE7

/** @brief Identify drive - get drive info */
#define ATA_CMD_IDENTIFY    0xEC

/** @brief Master drive on primary bus */
#define ATA_MASTER          0xA0

/** @brief Slave drive on primary bus */
#define ATA_SLAVE           0xB0

/** @brief Sector size in bytes (always 512 for ATA) */
#define ATA_SECTOR_SIZE     512

/** @brief Maximum number of sectors to read in one operation */
#define ATA_MAX_SECTORS     256

/**
 * @brief Initialize the ATA driver
 * 
 * Detects and initializes the primary master ATA drive. Sends IDENTIFY
 * command to verify the drive exists and is responding.
 * 
 * @return 0 on success, -1 if no drive found or initialization failed
 * 
 * Example:
 * @code
 * if (ata_init() < 0) {
 *     kprintf("No ATA drive found!\n");
 * }
 * @endcode
 * 
 * @note Must be called before any read/write operations
 */
int ata_init(void);


/**
 * @brief Read a single sector from disk
 * 
 * Reads 512 bytes from the specified LBA sector into the buffer.
 * Uses PIO mode (blocking, slow).
 * 
 * @param lba Logical Block Address (sector number, 28-bit)
 * @param buffer Buffer to read into (must be at least 512 bytes)
 * @return 0 on success, -1 on error
 * 
 * Example:
 * @code
 * uint8_t sector[512];
 * if (ata_read_sector(0, sector) == 0) {
 *     // sector[0..511] now contains sector 0 (MBR)
 * }
 * @endcode
 * 
 * @warning Buffer must be at least 512 bytes!
 * @note This function BLOCKS until the read completes (or times out)
 */
int ata_read_sector(uint32_t lba, uint8_t *buffer);

/**
 * @brief Read multiple sectors from disk
 * 
 * Reads multiple consecutive sectors starting at the specified LBA.
 * More efficient than calling ata_read_sector() in a loop.
 * 
 * @param lba Starting Logical Block Address
 * @param count Number of sectors to read (1-256)
 * @param buffer Buffer to read into (must be at least count * 512 bytes)
 * @return Number of sectors successfully read, or -1 on error
 * 
 * Example:
 * @code
 * uint8_t buffer[512 * 8];  // 8 sectors = 4KB
 * if (ata_read_sectors(100, 8, buffer) == 8) {
 *     // Successfully read 8 sectors starting at LBA 100
 * }
 * @endcode
 * 
 * @warning Buffer must be large enough for count * 512 bytes!
 */
int ata_read_sectors(uint32_t lba, uint8_t count, uint8_t *buffer);

/**
 * @brief Write a single sector to disk
 * 
 * Writes 512 bytes from the buffer to the specified LBA sector.
 * Uses PIO mode (blocking, slow).
 * 
 * @param lba Logical Block Address (sector number, 28-bit)
 * @param buffer Buffer containing data to write (must be 512 bytes)
 * @return 0 on success, -1 on error
 * 
 * Example:
 * @code
 * uint8_t sector[512];
 * // ... fill sector with data ...
 * if (ata_write_sector(100, sector) == 0) {
 *     kprintf("Sector written successfully!\n");
 * }
 * @endcode
 * 
 * @warning This is DANGEROUS - can corrupt filesystems if used incorrectly!
 * @note Consider implementing this AFTER read-only ext2 is working
 * @todo Implement write support
 */
int ata_write_sector(uint32_t lba, const uint8_t *buffer);

/**
 * @brief Check if ATA drive is present
 * 
 * Quick check to see if a drive is connected and responding.
 * Useful for detecting drive removal or testing.
 * 
 * @return true if drive is present and ready, false otherwise
 */
bool ata_drive_present(void);

/**
 * @brief Get drive capacity in sectors
 * 
 * Returns the total number of sectors available on the drive.
 * Obtained from IDENTIFY command during initialization.
 * 
 * @return Number of sectors (28-bit LBA), or 0 if drive not initialized
 * 
 * Example:
 * @code
 * uint32_t sectors = ata_get_capacity();
 * uint32_t size_mb = (sectors * 512) / (1024 * 1024);
 * kprintf("Disk size: %u MB\n", size_mb);
 * @endcode
 */
uint32_t ata_get_capacity(void);

/**
 * @brief Print drive information
 * 
 * Displays information about the detected ATA drive:
 * - Model string
 * - Serial number
 * - Firmware version
 * - Capacity
 * - Supported features
 * 
 * Useful for debugging and showing what hardware was detected.
 */
void ata_print_info(void);

#endif // ATA_H