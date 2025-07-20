#include <stdint.h>
#include "diskio.h"

// ATA Primary channel I/O ports (for QEMU/Bochs, first IDE disk)
#define ATA_PRIMARY_IO      0x1F0
#define ATA_PRIMARY_CTRL    0x3F6
#define SECTOR_SIZE         512

// I/O port helpers
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}
static inline uint16_t inw(uint16_t port) {
    uint16_t ret;
    __asm__ volatile ("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}
static inline void outw(uint16_t port, uint16_t val) {
    __asm__ volatile ("outw %0, %1" : : "a"(val), "Nd"(port));
}

// Wait until drive is ready to transfer (BSY clear, DRQ set)
static void ata_wait() {
    while (inb(ATA_PRIMARY_IO + 7) & 0x80);      // Wait for BSY = 0
    while (!(inb(ATA_PRIMARY_IO + 7) & 0x08));   // Wait for DRQ = 1
}

// Read sectors from disk using LBA
int disk_read(uint32_t lba, uint8_t *buf, uint32_t sectors) {
    for (uint32_t s = 0; s < sectors; s++) {
        // Setup registers for PIO LBA read
        outb(ATA_PRIMARY_IO + 2, 1); // sector count
        outb(ATA_PRIMARY_IO + 3, (uint8_t)((lba + s) & 0xFF));         // LBA low
        outb(ATA_PRIMARY_IO + 4, (uint8_t)(((lba + s) >> 8) & 0xFF));  // LBA mid
        outb(ATA_PRIMARY_IO + 5, (uint8_t)(((lba + s) >> 16) & 0xFF)); // LBA high
        outb(ATA_PRIMARY_IO + 6, 0xE0 | (((lba + s) >> 24) & 0x0F));   // drive/head
        outb(ATA_PRIMARY_IO + 7, 0x20); // READ SECTORS

        ata_wait();

        uint16_t *ptr = (uint16_t*)(buf + s * SECTOR_SIZE);
        for (int i = 0; i < SECTOR_SIZE / 2; i++) { // 256 words = 512 bytes
            ptr[i] = inw(ATA_PRIMARY_IO);
        }
    }
    return 0;
}

// Write sectors to disk using LBA
int disk_write(uint32_t lba, const uint8_t *buf, uint32_t sectors) {
    for (uint32_t s = 0; s < sectors; s++) {
        // Setup registers for PIO LBA write
        outb(ATA_PRIMARY_IO + 2, 1); // sector count
        outb(ATA_PRIMARY_IO + 3, (uint8_t)((lba + s) & 0xFF));         // LBA low
        outb(ATA_PRIMARY_IO + 4, (uint8_t)(((lba + s) >> 8) & 0xFF));  // LBA mid
        outb(ATA_PRIMARY_IO + 5, (uint8_t)(((lba + s) >> 16) & 0xFF)); // LBA high
        outb(ATA_PRIMARY_IO + 6, 0xE0 | (((lba + s) >> 24) & 0x0F));   // drive/head
        outb(ATA_PRIMARY_IO + 7, 0x30); // WRITE SECTORS

        ata_wait();

        const uint16_t *ptr = (const uint16_t*)(buf + s * SECTOR_SIZE);
        for (int i = 0; i < SECTOR_SIZE / 2; i++) { // 256 words = 512 bytes
            outw(ATA_PRIMARY_IO, ptr[i]);
        }
    }
    return 0;
}