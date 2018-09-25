#include "sfs.h"

unsigned char * bitmap;
struct Directory root;
int dev_size;
int bm_size;

void set_block_state(int idx, int state){
    puts("Starting \"set_block_state\" function...");

    if(state == 1){ 
        bitmap[idx/32] |= (1<<(idx%32));
    }else{  
        bitmap[idx/32] &=~ (1<<(idx%32));
    }     

    puts("Ending \"set_block_state\" function...");
}


int search_for_free_block(){
    puts("Starting \"search_for_free_block\" function...");

    int i = 0, j = 0; 
    int result = 0;

    while(i < BLOCK_SIZE*bm_size){
        if(bitmap[i] == 0){
            i++;
        }else{
            break;
        }
    }
    while(!((bitmap[i] >> j)&1) && j < 8){
        j++;
    }

    result = !(((bitmap[i])>>(j))&1) ? -1 : 8*i + j;

    puts("Ending \"search_for_free_block\" function...");

    printf("I: %d , J: %d, Result: %d\n", i, j,result);
    return result;
}

void load_dir_root(){
    unsigned char * dir_orig = (unsigned char *)malloc(BLOCK_SIZE);
    device_read_sector(dir_orig, 0);
    memcpy(&root, dir_orig, sizeof(root));
    free(dir_orig);
}

void load_bm(){ 
    int i = 0;   

    bitmap = (unsigned char*)malloc(BLOCK_SIZE*bm_size);

    unsigned char * bmp = (unsigned char *)malloc(BLOCK_SIZE*bm_size);
    unsigned char * bmp_orig = bmp;
    
    i = 5122;

    while( i < (5122 + bm_size)){
        device_read_sector(bmp,i);
        bmp += 4096;
        i++;
    }

    bmp = bmp_orig;
    memcpy(bitmap,bmp,BLOCK_SIZE*bm_size);
    bmp = bmp_orig;
    free(bmp);
}

void load_dsz(){
    unsigned char *dsz=(unsigned char*)malloc(BLOCK_SIZE); 
    device_read_sector(dsz,5120);
    memcpy(&dev_size,dsz,sizeof(dev_size));
    free(dsz);

    printf("DEVICE SIZE: %d\n",dev_size);
}

void load_bmsz(){
    unsigned char *bmsz=(unsigned char*)malloc(BLOCK_SIZE); 
    device_read_sector(bmsz,5121);
    memcpy(&bm_size,bmsz,sizeof(bm_size));
    free(bmsz);

    printf("BITMAP SIZE (IN BLOCKS): %d\n",bm_size);
}

void * sfs_init(struct fuse_conn_info *conn){
    load_dir_root();
    load_dsz();
    load_bmsz();
    load_bm();
    return NULL;
};


void sfs_get_file_stat(struct dir_entry *dentry, int *size, int *block_count){
    *size = dentry->filesize;
    *block_count = dentry->block_amount;
}

struct dir_entry *sfs_get_entry(const char *name)
{   
    puts("Starting \"sfs_get_entry\" function...");
    int i = 0;

    struct Directory *directory = &root;

    char fname[256];
    memset(fname, 0, 256);    
    strcpy(fname, &name[i+1]);

    i = 0;
    while(i<MAX_ENT)
    {    
        if(directory->dir_entries[i].index_block != 0)
        {
            if(strcmp(fname, directory->dir_entries[i].filename) == 0) return &directory->dir_entries[i];
        }   
        i++;
    }
    
    puts("Ending \"sfs_get_entry\" function...");
    return NULL;
}

int sfs_getattr(const char *path, struct stat *statbuf) 
{   
    int path_len = strlen(path);
    
    if ( (path_len == 1) && path[0] == '/') {
        statbuf->st_mode = S_IFDIR | S_IRWXU | S_IRWXG | S_IRWXO;
        statbuf->st_uid = 0;
        statbuf->st_gid = 0;
        statbuf->st_nlink = 1;
        statbuf->st_ino = 0;
        statbuf->st_size = BLOCK_SIZE;
        statbuf->st_blksize = BLOCK_SIZE;
        statbuf->st_blocks = 1;
    } else {

        struct dir_entry *dentry = sfs_get_entry(path);
        int size, block_count;
        
        if (dentry == NULL)
            return -ENOENT;
        
        if(dentry->dir_or_file == 0) //is a directory ... 1 is a file
        {
            statbuf->st_mode = S_IFDIR | S_IRWXU | S_IRWXG | S_IRWXO;
            statbuf->st_uid = 0;
            statbuf->st_gid = 0;
            statbuf->st_nlink = 1;
            statbuf->st_ino = 0;
            statbuf->st_size = BLOCK_SIZE;
            statbuf->st_blksize = BLOCK_SIZE;
            statbuf->st_blocks = 1;
        }
        else
        {
            sfs_get_file_stat(dentry, &size, &block_count);

            statbuf->st_mode = S_IFREG | S_IRWXU | S_IRWXG | S_IRWXO;
            statbuf->st_nlink = 1;
            statbuf->st_ino = 0;
            statbuf->st_uid = 0;
            statbuf->st_gid = 0;
            statbuf->st_size = size; 
            statbuf->st_blksize = BLOCK_SIZE;
            statbuf->st_blocks = block_count;
        }        
    }
    return 0;
}


int sfs_readdir(const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fileInfo){
    
    struct Directory directory = root;

    int i=0;
    while(i<MAX_ENT)
    {   
        if(directory.dir_entries[i].index_block!=0)
        {
            if(filler(buffer, directory.dir_entries[i].filename, NULL, 0)!= 0)
            {
                return -ENOMEM;
            }
        }   
        i++;
    }

    return 0;
}


int sfs_mknod(const char *path, mode_t mode, dev_t dev){
    if(S_ISREG(mode)){
        struct dir_entry * entry = sfs_get_entry(path);

        if(entry != NULL){
            return -EEXIST;
        }

        int i;
        for(i = 0 ; i < MAX_ENT; i++ ){
            if(root.dir_entries[i].index_block == 0 ){
                break;
            }
        }

        int free_blocks = search_for_free_block();

        entry = &(root.dir_entries[i]);

        memcpy(entry->filename, &path[1], 256);
        entry->index_block = free_blocks;
        entry->filesize = 0;
        entry->dir_or_file = 1;
        set_block_state(free_blocks,0);

        unsigned char * nou = (unsigned char*)malloc(sizeof(root));
        memcpy(nou, &root, sizeof(root));
        device_write_sector(nou, 0);
        free(nou);
        return 0;
    }

    return -EPERM;
}


int sfs_utime(const char *path, struct utimbuf *ubuf) {
    return 0;
}

int sfs_flush(const char *path, struct fuse_file_info *fileInfo){
    return 0;
}

int sfs_release(const char *path, struct fuse_file_info *fileInfo){
    fileInfo->fh = 0;
    return 0;
}

int sfs_releasedir(const char *path, struct fuse_file_info *fileInfo) {
    return 0;
}