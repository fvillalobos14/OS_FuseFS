#include "sfs.h"

#include <fuse.h>
#include <stdio.h>

struct fuse_operations sfs_oper;

int main(int argc, char *argv[]) {

	int i, fuse_stat;

	sfs_oper.getattr = sfs_getattr;
	sfs_oper.mknod = sfs_mknod;
	sfs_oper.utime = sfs_utime;
	sfs_oper.flush = sfs_flush;
	sfs_oper.release = sfs_release;
	sfs_oper.readdir = sfs_readdir;
	sfs_oper.releasedir = sfs_releasedir;
	sfs_oper.init = sfs_init;

	printf("mounting file system...\n");
	
	for(i = 1; i < argc && (argv[i][0] == '-'); i++) {
		if(i == argc) {
			return (-1);
		}
	}
	puts(argv[i+1]);
	if (!device_open(argv[i], atoi(argv[i+1])) ) {
	    printf("Cannot open device file %s\n", argv[i]);
	    return 1;
	}

	for(i=1; i < argc; i++) {
		argv[i] = argv[i+1];
	}
	argc--;


	for(i=1; i < argc; i++) {
		argv[i] = argv[i+1];
	}
	argc--;

	fuse_stat = fuse_main(argc, argv, &sfs_oper, NULL);

    device_close();
    
	printf("fuse_main returned %d\n", fuse_stat);

	return fuse_stat;
}


