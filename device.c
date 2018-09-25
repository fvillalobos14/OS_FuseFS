#include "device.h"

static const char *device_path;
static FILE *f;

void device_init_dir(){
    puts("Beginning to write directory...\n");
    struct Directory dir;

    int i = 0;
    while(i < MAX_ENT){
        dir.dir_entries[i].index_block = 0;
        i++;
    }

    unsigned char * dir_orig = (unsigned char *)malloc(sizeof(dir));
    memcpy(dir_orig, &dir, sizeof(dir));

    i = 0;
    while(i < FAT_BLOCK_AMOUNT){
        device_write_sector(dir_orig,i);
        i++;
    }

    free(dir_orig);
    puts("Finished writing directory..\n");
}

void device_init_map(int size){  //size enters in byte
    puts("Inside init_map...");
    int bm_sz = (size / BLOCK_SIZE) / 8; //bytes
    int bm_blks = bm_sz / BLOCK_SIZE; //blocks needed for bitmap

    unsigned char *sz=(unsigned char*)malloc(BLOCK_SIZE); 
    memcpy(sz, &bm_blks, sizeof(bm_blks));
    device_write_sector(sz, 5121);
    free(sz);

    unsigned char *bm = (unsigned char*)malloc(BLOCK_SIZE*bm_blks);
    unsigned char *bitmap_orig = bm;
    unsigned int filler[BLOCK_SIZE];
 
    int i = 0;

    while(i < BLOCK_SIZE){
        if(i < 161){
            filler[i] = 0;
        }else{
            filler[i] = 0xFFFFFFFF;
        }
        i++;
    }

    memcpy(bm,filler,BLOCK_SIZE);
    bm += BLOCK_SIZE;

    while(i < BLOCK_SIZE){
        filler[i] = 0xFFFFFFFF;
        i++;
    }

    while(i < bm_blks-1){
        memcpy(bm,filler,BLOCK_SIZE);
        bm += BLOCK_SIZE;
        i++;
    }

    i = 5122;
    bm = bitmap_orig;
    while( i < (5122+bm_blks)){
        device_write_sector(bm,i);
        bm += 4096;
        i++;
    }

    bm = bitmap_orig;
    free(bm);
    puts("Finished writing bitmap...\n");
}

void device_create(const char * path, int size){
    puts("------  BEGINNING DEVICE CREATE  ------\n");
    f = fopen(path, "w+");

    device_fill(size);
    device_init_dir();

    unsigned char *sz=(unsigned char*)malloc(BLOCK_SIZE); 
    memcpy(sz, &size, sizeof(size));
    device_write_sector(sz, 5120);
    free(sz);

    device_init_map(size);
    fflush(f);

    puts("------  ENDING DEVICE CREATE  ------\n");

}

void device_fill(int size){
    int sz = size / BLOCK_SIZE;
    unsigned char *fil = (unsigned char*)malloc(size);
    unsigned char *fil_orig = fil;
    unsigned int filler[BLOCK_SIZE];

    int i = 0;
    puts("before first while\n");
    while(i < BLOCK_SIZE){
        filler[i] = 0x00;
        i++;
    }

    i = 0;
    puts("before 2nd while\n");
    while(i < sz){
        memcpy(fil,filler,BLOCK_SIZE);
        printf("before device %d write sector\n", i);
        device_write_sector(fil,i);
        fil+=4096;
        i++;
    }

    puts("before free\n");
    fil = fil_orig;
    free(fil);
}

int device_open(const char *path, int size) 
{
    device_path = path;
    f = fopen(path, "r+");

    if(f == NULL){
        device_create(path, size);
    }

    return 1;
}

void device_close()
{
    fflush(f);
    fclose(f);
}

int device_read_sector(unsigned char buffer[], int sector) 
{
    fseek(f, sector*BLOCK_SIZE, SEEK_SET);
	
    return ( fread(buffer, 1, BLOCK_SIZE, f) == BLOCK_SIZE );
}

int device_write_sector(unsigned char buffer[], int sector) 
{   
    puts("before fseek \n");
    fseek(f, sector*BLOCK_SIZE , SEEK_SET);
	puts("before return\n");
    return ( fwrite(buffer, 1, BLOCK_SIZE, f) == BLOCK_SIZE );
}

void device_flush()
{
    fflush(f);
}
