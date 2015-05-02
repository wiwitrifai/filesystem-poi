/* rp_fuse.h */

#include <stdlib.h>
#include <string.h>
#include <string>
#include <fstream>
#include <time.h>

typedef unsigned short ptr_block;

/** Konstanta **/
/* Konstanta ukuran */
#define BLOCK_SIZE 512
#define N_BLOCK 65536
#define ENTRY_SIZE 32
#define DATA_POOL_OFFSET 257
/* Konstanta untuk ptr_block */
#define EMPTY_BLOCK 0x0000
#define END_BLOCK 0xFFFF

/* Struct for filesystem */
typedef struct {
// Voluem Information
  char VolumeName[32];
  int Capacity;
  int Unused;
  ptr_block FirstEmpty;
  char path[32];
// Alocation Table
  ptr_block NextBlock[N_BLOCK];
} poi_file;
/* Membuat filesystem baru */
void createFilesystem (const char * path);
/* Meload filesystem yang sudah ada */
void load(const char * path, poi_file * filesys);

/* Mengupdate Volume Information */
void writeVolumeInfo(poi_file * filesys);
/* Mengupdate Allocation Table */
void writeAllocTable(ptr_block position, poi_file filesys);

/* bagian alokasi block */
void setNextBlock(ptr_block position, ptr_block next);
ptr_block allocateBlock();
void freeBlock(ptr_block position);

int readBlock(ptr_block position, char *buffer, int size, int offset = 0);
int writeBlock(ptr_block position, const char *buffer, int size, int offset = 0);
