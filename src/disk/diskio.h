#ifndef DISKIO_H
#define DISKIO_H

#include <stdint.h>

int disk_read(uint32_t lba, uint8_t *buf, uint32_t sectors);
int disk_write(uint32_t lba, const uint8_t *buf, uint32_t sectors);

#endif