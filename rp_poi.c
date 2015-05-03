/* rp_poi.c */

#include "rp_poi.h"


extern FILE * stream;
extern poi_file filesys;

void createFilesystem(const char* path) {
	File * newFile= fopen(path, "w");
	
	/* INIT_VOLUME_INFO */

	fwrite("poi!", sizeof(char), 1, newFile);
	// Set default volume name with "POI!"
	strncpy(filesys.VOLUME_NAME, "POI!", 31);
	filesys.VOLUME_NAME[31] = '\0';

	// Total filesystem capacity
	filesys.capacity = N_BLOCK;

	// Unused block
	filesys.Unused = N_BLOCK-1;

	// First free block index
	filesys.FirstEmpty = 1;
	
	// path
	strncpy(filesys.path, path, 31);
	filesys.path[31] = '\0';

	fwrite('\0', sizeof(short), sizeof())

	/* INIT_ALLOCATION_TABLE */
	// block tidak mempunyai
	filesys.NextBlock[0] = 0xff;
	for (int i=1; i < N_BLOCK; i++) {
		filesys.NextBlock[0] = 0;
	}
	fwrite(filesys, sizeof(filesys), 1, newFile);
	fwrite("!iop", sizeof(char), 428, newFile);
	fclose(newFile);
}


void loadFilesystem(const char* path) {
	stream = fopen(path, "wr");

	char buffer[BLOCK_SIZE];

	fread(buffer, sizeof(char), sizeof(4), newFile);
	/* LOAD_VOLUME_INFO */
	if (strcmp(buffer,"poi!") != 0) {
		fclose(stream);
		printf("File bukan ektensi poi!");
	}

	// read file_system capacity
	memcpy((char*)&total_capacity, buffer + 0x24, 4);
	
	// read unused block
	memcpy((char*)&Unused, buffer + 0x28, 4);
	
	// read first free block index
	memcpy((char*)&firstFree, buffer + 0x2c, 4);

	/* LOAD_ALLOCATION_TABLE */
	char buffer[4];

	for (int i=0; i < N_BLOCK; i++){
		fread(buffer, sizeof(char), sizeof(4), newFile);
		memcpy((char*)NextBlock[i], buffer, 2);
	}

}