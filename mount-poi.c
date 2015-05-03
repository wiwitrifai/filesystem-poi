/* Rakjat Priboemi */
/* File : mount_poi.c */

#include <stdio.h>
#include <stdlib.h>
#include "rp_poi.h"
#include "rp_fuse.h"

FILE * stream;
poi_file filesys;
time_t mount_time;

static struct fuse_operations rp_oper = {
  .getattr  = rp_poi_getattr,
  .readdir  = rp_poi_readdir,
  .mkdir  = rp_poi_mkdir,
  .open = rp_poi_open,
  .rmdir = rp_poi_rmdir,
  .rename = rp_poi_rename,
  .unlink = rp_poi_unlink,
  .mknod  = rp_poi_mknod,
  .truncate= rp_poi_truncate,
  .write  = rp_poi_write,
  .read = rp_poi_read,
  .link = rp_poi_link,
};

int main(int argc, char* argv[]) {
  time(&mount_time);
  if(argc < 3) {
    printf("How to use : ./mount-poi <mount_folder> <filesystem.poi> [-new]\n");
    return 0;
  }
  if(argc > 3) {
    if(strcmp("-new", argv[3]) == 0) {
      createFilesystem(argv[2]);
    }
  }

  int fargc = 2;
  char* fargv[2] = { argv[0], argv[1] };
  loadFilesystem(argv[2]);
  return fuse_main(fargc, fargv, &rp_oper, NULL);
}