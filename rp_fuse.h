/* File : rp_fuse.h */
#ifndef _RP_FUSE__H_
#define _RP_FUSE__H_

#include <errno.h>
#include <fuse.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "rp_poi.h"


/** Get file attributes.*/
int rp_poi_getattr(const char* path, struct stat* stbuf);
/** Read directory */
int rp_poi_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi);
/** Create a directory */
int rp_poi_mkdir(const char *path, mode_t mode);
/** File open operation */
int rp_poi_open(const char* path, struct fuse_file_info* fi);
/** Remove a directory */
int rp_poi_rmdir(const char *path);
/** Rename a file */
int rp_poi_rename(const char* path, const char* newpath);
/** Remove a file */
int rp_poi_unlink(const char *path);
/** Create a node file */
int rp_poi_mknod(const char *path, mode_t mode, dev_t dev);
/** Change the size of an open file */
int rp_poi_truncate(const char *path, off_t newsize);
/** Read data from open file */
int rp_poi_read(const char *path,char *buf,size_t size,off_t offset,struct fuse_file_info *fi);
/** write data to a open file */
int rp_poi_write(const char *path, const char *buf, size_t size, off_t offset,struct fuse_file_info *fi);
/** Create a hard link to a file */
int rp_poi_link(const char *path, const char *newpath);


#endif