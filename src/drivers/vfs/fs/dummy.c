// TEST FILESYSTEM FOR TEST THINGS
// THIS IS NOT A REAL FILESYSTEM
#include "../vfs.h"
#include "../file.h"
#include "libk/string.h"

// Hardcoded "files"
static const char *hello_txt = "Hello from VFS!\n";
static const char *test_txt = "This is a test file.\n";

static int dummy_open(const char *path, int flags, file_t *file) {
    if (strcmp(path, "/hello.txt") == 0) {
        file->fs_data = (void*)hello_txt;
        return 0;
    }
    if (strcmp(path, "/test.txt") == 0) {
        file->fs_data = (void*)test_txt;
        return 0;
    }
    return -1;  // File not found
}

static int dummy_read(file_t *file, void *buf, size_t count) {
    const char *data = (const char*)file->fs_data;
    size_t len = strlen(data);
    
    if (file->offset >= len) return 0;  // EOF
    
    size_t to_read = (count < len - file->offset) ? count : (len - file->offset);
    memcpy(buf, data + file->offset, to_read);
    file->offset += to_read;
    
    return to_read;
}

static int dummy_stat(const char *path, stat_t *st) {
    if (strcmp(path, "/hello.txt") == 0) {
        st->size = strlen(hello_txt);
        st->type = VFS_FILE;
        return 0;
    }
    if (strcmp(path, "/test.txt") == 0) {
        st->size = strlen(test_txt);
        st->type = VFS_FILE;
        return 0;
    }
    return -1;
}

fs_ops_t dummy_fs_ops = {
    .name = "dummy",
    .open = dummy_open,
    .read = dummy_read,
    .stat = dummy_stat,
    // Rest are NULL
};

// Registration helper
int dummy_fs_init(void) {
    return vfs_register_fs(&dummy_fs_ops);
}