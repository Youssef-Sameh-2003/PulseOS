#ifndef FS_H
#define FS_H

#include <stddef.h>
#include <stdint.h>

#define FS_MAX_FILES 16
#define FS_MAX_FILENAME 16
#define FS_MAX_FILESIZE 4096

int fs_init();
int fs_create(const char* name, const char* dirname);
int fs_write(const char* name, const char* dirname, const char* data, size_t len);
int fs_read(const char* name, char* out, size_t maxlen);
int fs_list(char* out, size_t maxlen);

#endif