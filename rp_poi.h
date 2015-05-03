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
#define ENTRY_SIZE 32
#define DATA_POOL_OFFSET 257
/* Konstanta untuk ptr_block */
#define EMPTY_BLOCK 0x0000
#define END_BLOCK 0xFFFF

/* Struct for file or directory */
typedef struct  {
  char Name[21];
  char Atribut;
  char Time[2];
  char Date[2];
  ptr_block IndexFirst;
  int Size;
  ptr_block Position;
  unsigned char Offset;
} entry_block;

/* Struct for filesystem */
typedef struct {
// Volume Information
  char VolumeName[32];
  int Capacity;
  int Unused;
  int FirstEmpty;
  entry_block Root;
// Alocation Table
  ptr_block NextBlock[N_BLOCK];
} poi_file;


/* Membuat filesystem baru */
void createFilesystem (const char * path);
/* Meload filesystem yang sudah ada */
void loadFilesystem(const char * path);

/* Mengupdate Volume Information */
void writeVolumeInfo();
/* Mengupdate Allocation Table */
void writeAllocTable(ptr_block position);

/* Mengupdate NextBlock dari suatu block*/
void setNextBlock(ptr_block position, ptr_block next);

/* Mencari block ksosong */
ptr_block allocateBlock();
/* Mengosongkan suatu block */
void freeBlock(ptr_block position);

/* Membaca block */
int readBlock(ptr_block position, char *buffer, int size, int offset);
/* menulis Block */
int writeBlock(ptr_block position, const char *buffer, int size, int offset);

void readEntryBlock(entry_block *, int);
entry_block createEntryBlockEmpty();
/** entry block from location */
entry_block createEntryBlock (ptr_block position, unsigned char offset);
  /** Mendapatkan Entry berikutnya */
entry_block nextEntry(entry_block * eb);
/** Mendapatkan Entry dari path */
entry_block getEntry(entry_block * eb, const char *path);
/** Mendapatkan Entry dari path */
entry_block getNewEntry(entry_block * eb, const char *path);
/** Mengembalikan entry kosong selanjutnya. Jika blok penuh, akan dibuatkan entri baru */
entry_block getNextEmptyEntry(entry_block * this);
/** Memeriksa apakah Entry kosong atau tidak */
int isEmpty(entry_block * eb);

time_t getDateTime(entry_block * e);

void setCurrentDateTime(entry_block * eb);
/** Menuliskan entry ke filesystem */
void writeEntryBlock(entry_block * eb);

#endif