#include "disk/diskio.h"
#include <stddef.h>
#include <stdint.h>

// Filesystem configuration
#define FS_MAX_FILES       16
#define FS_MAX_FILENAME    16
#define FS_MAX_FILESIZE    4096
#define FS_MAX_DIRS        4
#define FS_MAX_DIRNAME     16
#define FS_DISK_START      1         // LBA sector for file table start
#define FS_DISK_FTABLE_SECTORS 2     // Number of sectors for file table
#define FS_DISK_FILEDATA_START 10    // LBA sector for file data start
#define FS_DISK_BLOCK_SIZE 512
#define FS_DISK_FILE_BLOCKS (FS_MAX_FILESIZE / FS_DISK_BLOCK_SIZE)

// Directory entry
struct fs_dir_entry {
    char name[FS_MAX_DIRNAME];
    uint8_t used;
    uint8_t reserved[7];
};

// On-disk file table entry
struct fs_disk_entry {
    char name[FS_MAX_FILENAME];
    uint32_t size;
    uint32_t block; // Start sector for file data
    uint8_t used;
    uint8_t dir;    // Directory index
    uint8_t reserved[2];
};

static struct fs_dir_entry dirtable[FS_MAX_DIRS];
static struct fs_disk_entry filetable[FS_MAX_FILES];

// Freestanding string/memory functions
size_t strlen(const char *s) {
    size_t i = 0; while (s[i]) i++; return i;
}
void *memcpy(void *d, const void *s, size_t n) {
    char *dd = d; const char *ss = s;
    for (size_t i = 0; i < n; i++) dd[i] = ss[i];
    return d;
}
char *strcpy(char *d, const char *s) {
    char *dd = d; while ((*dd++ = *s++)); return d;
}
char *strncpy(char *d, const char *s, size_t n) {
    size_t i = 0;
    for (; i < n && s[i]; i++) d[i] = s[i];
    for (; i < n; i++) d[i] = 0;
    return d;
}


// Load file and dir tables from disk
static void fs_disk_load_table() {
    disk_read(FS_DISK_START, (uint8_t*)dirtable, 1);
    disk_read(FS_DISK_START + 1, (uint8_t*)filetable, FS_DISK_FTABLE_SECTORS);
}

// Save file and dir tables to disk
static void fs_disk_save_table() {
    disk_write(FS_DISK_START, (const uint8_t*)dirtable, 1);
    disk_write(FS_DISK_START + 1, (const uint8_t*)filetable, FS_DISK_FTABLE_SECTORS);
}

// Initialize (load table)
int fs_init() {
    fs_disk_load_table();
    return 0;
}

// Find directory entry by name
static int fs_dir_find(const char* name) {
    for (int i = 0; i < FS_MAX_DIRS; i++)
        if (dirtable[i].used && strncmp(dirtable[i].name, name, FS_MAX_DIRNAME) == 0)
            return i;
    return -1;
}

// Find file entry by name and directory
static int fs_disk_find(const char* name, int dir_idx) {
    for (int i = 0; i < FS_MAX_FILES; i++)
        if (filetable[i].used && filetable[i].dir == dir_idx &&
            strncmp(filetable[i].name, name, FS_MAX_FILENAME) == 0)
            return i;
    return -1;
}

// Create directory
int fs_mkdir(const char* dirname) {
    if (fs_dir_find(dirname) >= 0) return 0; // Already exists
    for (int i = 0; i < FS_MAX_DIRS; i++) {
        if (!dirtable[i].used) {
            strncpy(dirtable[i].name, dirname, FS_MAX_DIRNAME-1);
            dirtable[i].name[FS_MAX_DIRNAME-1] = 0;
            dirtable[i].used = 1;
            fs_disk_save_table();
            return 0;
        }
    }
    return -1;
}

// List files in a directory
int fs_listdir(const char* dirname, char* out, size_t maxlen) {
    int dir_idx = fs_dir_find(dirname);
    if (dir_idx < 0) return -1;
    size_t total = 0;
    for (int i = 0; i < FS_MAX_FILES; i++) {
        if (filetable[i].used && filetable[i].dir == dir_idx) {
            size_t len = strlen(filetable[i].name);
            if (total + len + 2 < maxlen) {
                strcpy(out + total, filetable[i].name);
                total += len;
                out[total++] = '\n';
            }
        }
    }
    out[total] = 0;
    return total;
}

// Create file (optionally in a directory)
int fs_create(const char* name, const char* dirname) {
    int dir_idx = 0; // Default to root dir
    if (dirname && dirname[0])
        dir_idx = fs_dir_find(dirname);
    if (dir_idx < 0) dir_idx = 0;
    if (fs_disk_find(name, dir_idx) >= 0) return 0; // Already exists
    for (int i = 0; i < FS_MAX_FILES; i++) {
        if (!filetable[i].used) {
            strncpy(filetable[i].name, name, FS_MAX_FILENAME-1);
            filetable[i].name[FS_MAX_FILENAME-1] = 0;
            filetable[i].size = 0;
            filetable[i].block = FS_DISK_FILEDATA_START + i * FS_DISK_FILE_BLOCKS;
            filetable[i].used = 1;
            filetable[i].dir = dir_idx;
            fs_disk_save_table();
            return 0;
        }
    }
    return -1;
}

// Write file
int fs_write(const char* name, const char* dirname, const char* data, size_t len) {
    int dir_idx = 0;
    if (dirname && dirname[0]) dir_idx = fs_dir_find(dirname);
    if (dir_idx < 0) dir_idx = 0;
    int idx = fs_disk_find(name, dir_idx);
    if (idx < 0) return -1;
    uint32_t to_write = len > FS_MAX_FILESIZE ? FS_MAX_FILESIZE : len;
    uint32_t sectors = (to_write + FS_DISK_BLOCK_SIZE - 1) / FS_DISK_BLOCK_SIZE;
    disk_write(filetable[idx].block, (const uint8_t *)data, sectors);
    filetable[idx].size = to_write;
    fs_disk_save_table();
    return 0;
}

// Read file
int fs_read(const char* name, const char* dirname, char* out, size_t maxlen) {
    int dir_idx = 0;
    if (dirname && dirname[0]) dir_idx = fs_dir_find(dirname);
    if (dir_idx < 0) dir_idx = 0;
    int idx = fs_disk_find(name, dir_idx);
    if (idx < 0) return -1;
    uint32_t to_read = filetable[idx].size > maxlen ? maxlen : filetable[idx].size;
    uint32_t sectors = (to_read + FS_DISK_BLOCK_SIZE - 1) / FS_DISK_BLOCK_SIZE;
    disk_read(filetable[idx].block, (uint8_t*)out, sectors);
    return to_read;
}

// List all files (in all directories)
int fs_list(char* out, size_t maxlen) {
    size_t total = 0;
    for (int i = 0; i < FS_MAX_FILES; i++) {
        if (filetable[i].used) {
            size_t len = strlen(filetable[i].name);
            if (total + len + 2 < maxlen) {
                strcpy(out + total, filetable[i].name);
                total += len;
                out[total++] = '\n';
            }
        }
    }
    out[total] = 0;
    return total;
}