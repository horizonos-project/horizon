#include "initramfs.h"
#include "../vfs/vfs.h"
#include "../../libk/string.h"
#include "../../libk/kprint.h"
#include "../../mm/pmm.h"
#include "../../mm/vmm.h"
#include "kernel/log.h"

// Convert octal string to integer since octal is bleh
static uint32_t octal_to_int(const char *str, size_t len) {
    uint32_t result = 0;
    for (size_t i = 0; i < len && str[i] && str[i] != ' '; i++) {
        if (str[i] >= '0' && str[i] <= '7') {
            result = result * 8 + (str[i] - '0');
        }
    }
    return result;
}

// In-memory file cache (very nice)
#define MAX_INITRAMFS_FILES 256

struct initramfs_file {
    char name[256];
    void *data;
    uint32_t size;
    uint8_t type;  // '0' = file, '5' = directory
};

static struct initramfs_file files[MAX_INITRAMFS_FILES];
static int num_files = 0;

// Load tar archive into memory
static void initramfs_load(void *tar_start, size_t tar_size) {
    klogf("[initramfs] Loading from 0x%08x (size: %u bytes)\n", 
          (uint32_t)tar_start, tar_size);
    
    struct tar_header *header = (struct tar_header*)tar_start;
    void *tar_end = (uint8_t*)tar_start + tar_size;
    
    while ((void*)header < tar_end && header->name[0] != '\0') {
        // Verify ustar magic
        if (strncmp(header->magic, "ustar", 5) != 0) {
            klogf("[initramfs] Bad magic at 0x%08x, stopping\n", (uint32_t)header);
            break;
        }
        
        // Parse file size (octal)
        uint32_t size = octal_to_int(header->size, 12);
        
        // File data comes right after header (512 bytes)
        void *data = (void*)((uint8_t*)header + 512);
        
        // Store file info
        if (num_files < MAX_INITRAMFS_FILES) {
            // Copy filename
            size_t name_len = strlen(header->name);
            if (name_len > 0 && name_len < 256) {
                // Ensure leading slash
                if (header->name[0] != '/') {
                    files[num_files].name[0] = '/';
                    strncpy(files[num_files].name + 1, header->name, 254);
                } else {
                    strncpy(files[num_files].name, header->name, 255);
                }
                
                files[num_files].data = data;
                files[num_files].size = size;
                files[num_files].type = header->typeflag;
                
                klogf("[initramfs] [%d] %s (%u bytes, type '%c')\n",
                      num_files, files[num_files].name, size, header->typeflag);
                
                num_files++;
            }
        }
        
        // Move to next header (aligned to 512 bytes)
        uint32_t blocks = (size + 511) / 512;
        header = (struct tar_header*)((uint8_t*)header + 512 + (blocks * 512));
    }
    
    klogf("[initramfs] Loaded %d files\n", num_files);
}

// VFS operations
static int initramfs_open(const char *path, int flags, file_t *file) {
    (void)flags;
    
    klogf("[initramfs] open('%s')\n", path);
    
    for (int i = 0; i < num_files; i++) {
        if (strcmp(files[i].name, path) == 0) {
            // Only open regular files
            if (files[i].type == '0' || files[i].type == '\0') {
                file->fs_data = (void*)&files[i];
                file->offset = 0;
                klogf("[initramfs] Found file: %s\n", files[i].name);
                return 0;
            }
        }
    }
    
    klogf("[initramfs] File not found: %s\n", path);
    return -1;
}

static int initramfs_close(file_t *file) {
    (void)file;
    return 0;
}

static int initramfs_read(file_t *file, void *buf, size_t count) {
    struct initramfs_file *f = (struct initramfs_file*)file->fs_data;
    
    if (!f) return -1;
    
    // Check for EOF
    if (file->offset >= f->size) {
        return 0;
    }
    
    // Calculate bytes to read
    size_t to_read = count;
    if (file->offset + to_read > f->size) {
        to_read = f->size - file->offset;
    }
    
    // Copy data
    memcpy(buf, (uint8_t*)f->data + file->offset, to_read);
    file->offset += to_read;
    
    return to_read;
}

static int initramfs_stat(const char *path, stat_t *st) {
    for (int i = 0; i < num_files; i++) {
        if (strcmp(files[i].name, path) == 0) {
            st->size = files[i].size;
            st->type = (files[i].type == '5') ? VFS_DIR : VFS_FILE;
            st->inode = i;
            return 0;
        }
    }
    
    return -1;
}

// Filesystem operations table
static fs_ops_t initramfs_ops = {
    .name = "initramfs",
    .open = initramfs_open,
    .close = initramfs_close,
    .read = initramfs_read,
    .stat = initramfs_stat,
};

// Initialize from embedded data
void initramfs_init(void) {
    extern uint8_t initramfs_start[];
    extern uint8_t initramfs_end[];
    
    size_t size = (size_t)(initramfs_end - initramfs_start);
    
    klogf("[initramfs] Initializing...\n");
    klogf("[initramfs] Start: 0x%08x, End: 0x%08x\n", 
          (uint32_t)initramfs_start, (uint32_t)initramfs_end);
    klogf("[initramfs] Size: %u bytes\n", size);
    
    if (size == 0) {
        klogf("[initramfs] WARNING: No initramfs data found!\n");
        return;
    }
    
    // Load the tar archive
    initramfs_load(initramfs_start, size);
    
    // Register with VFS
    vfs_register_fs(&initramfs_ops);
    klogf("[initramfs] Registered with VFS\n");
}