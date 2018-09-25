#ifndef _DEVICE_H_
#define _DEVICE_H_

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include "sfs.h"

#define BLOCK_SIZE 4096

#ifdef __cplusplus
extern "C" {
#endif

void device_init_dir(); 
void device_init_map(int size);
void device_create(const char * path, int size);
void device_fill(int size);
int device_open(const char *path, int size);
void device_close();
int device_read_sector(unsigned char buffer[], int sector);
int device_write_sector(unsigned char buffer[], int sector);
void device_flush();

#ifdef __cplusplus
}
#endif

#endif
