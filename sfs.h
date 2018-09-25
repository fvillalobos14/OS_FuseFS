#ifndef sfs_h
#define sfs_h

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <fuse.h>
#include <libgen.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <sys/types.h>
#include "device.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BLOCK_SIZE 4096
#define FAT_SIZE 20971520 //20 MB
#define FAT_BLOCK_AMOUNT (int)(((20*1024)/4096)*1024)
#define DIR_ENT_SIZE (int)sizeof(struct dir_entry)
#define DIR_ID (int)0
#define FILE_ID (int)1
#define MAX_ENT (int)(BLOCK_SIZE/DIR_ENT_SIZE)
#define BLOCK_AMNT (int)(BLOCK_SIZE/sizeof(int))

struct dir_entry{
    char filename[256];
    int filesize;
    int dir_or_file; 
    int index_block;
    int block_amount;
    long creation_date;
    long modification_date;
};

struct Directory{
    struct dir_entry dir_entries[MAX_ENT];
};

struct Index_List{
    int file_blocks[BLOCK_AMNT];
};

void * sfs_init(struct fuse_conn_info *conn);
int sfs_getattr(const char *path, struct stat *statbuf);
int sfs_utime(const char *path, struct utimbuf *ubuf);
int sfs_flush(const char *path, struct fuse_file_info *fileInfo);
int sfs_release(const char *path, struct fuse_file_info *fileInfo);
int sfs_releasedir(const char *path, struct fuse_file_info *fileInfo);
int sfs_mknod(const char *path, mode_t mode, dev_t dev);
int sfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fileInfo);

#ifdef __cplusplus
}
#endif

#endif //sfs_h

