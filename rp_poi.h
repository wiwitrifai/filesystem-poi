/* rp_poi.h */
#ifndef RP_POI_H__
#define RP_POI_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>
#include <time.h>

typedef unsigned short ptr_block;

/** Konstanta **/
/* Konstanta ukuran */
#define BLOCK_SIZE 512
#define N_BLOCK 65536
#define FILL_SIZE 32
#define DATA_POOL_OFFSET 257
/* Konstanta untuk ptr_block */
#define EMPTY_BLOCK 0x0000
#define END_BLOCK 0xFFFF

/* Struct for filesystem */
typedef struct {
// Volume Information
  char VolumeName[32];
  int Capacity;
  int Unused;
  ptr_block FirstEmpty;
  char path[32];
// Alocation Table
  ptr_block NextBlock[N_BLOCK];
} poi_file;

/* Struct for file or directory */
typedef struct  {
  ptr_block position;
  char Name[21];
  char Atribut;
  unsigned short Time;
  unsigned short Date;
  ptr_block IndexFirst;
  int size;
} file_dir;

/* Membuat filesystem baru */
void createFilesystem (const char * path);
/* Meload filesystem yang sudah ada */
void loadFilesystem(const char * path);

// /* Mengupdate Volume Information */
// void writeVolumeInfo();
// /* Mengupdate Allocation Table */
// void writeAllocTable(ptr_block position);

// /* Mengupdate NextBlock dari suatu block*/
// void setNextBlock(ptr_block position, ptr_block next);

// /* Mencari block ksosong */
// ptr_block allocateBlock();
// /* Mengosongkan suatu block */
// void freeBlock(ptr_block position);

// /* Membaca block */
// int readBlock(ptr_block position, char *buffer, int size, int offset);
// /* menulis Block */
// int writeBlock(ptr_block position, const char *buffer, int size, int offset);



#endif
