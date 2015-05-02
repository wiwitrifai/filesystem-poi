/* rp_poi.c */

#include "rp_poi.h"

void createFilesystem(char* path) {
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

	/* INIT_ALLOCATION_TABLE */
	fwrite('\0', sizeof(short), sizeof())

	// block tidak mempunyai
	filesys.NextBlock[0] = 0xff;
	for (int i=1; i < N_BLOCK; i++) {
		filesys.NextBlock[0] = 0;
	}
	fwrite(filesys, sizeof(filesys), 1, newFile);
	fwrite("!iop", sizeof(char), 428, newFile);
	fclose(newFile);
}


void loadFilesystem(char* path) {
	stream = fopen(path, "wr");

	char buffer[BLOCK_SIZE];

	fread(buffer, sizeof(char), sizeof(char), stream);
	/* LOAD_VOLUME_INFO */
	if (strcmp(buffer,"poi!") != 0) {
		fclose(stream);
		printf("File bukan ektensi poi!");
	}
	fread(buffer, sizeof(char), 32, stream)
	strncpy(filesys.VOLUME_NAME, buffer, 31);
	filesys.VOLUME_NAME[31] = '\0';

	// read file_system capacity
	fread(buffer, sizeof(char), sizeof(char), stream);
	filesys.capacity = buffer;
	
	// read unused block
	fread(buffer, sizeof(char), sizeof(char), stream);
	filesys.Unused = buffer;

	// read first free block index
	filesys.FirstEmpty = buffer;

	// path
	strncpy(filesys.path, path, 31);
	filesys.path[31] = '\0';

	/* LOAD_ALLOCATION_TABLE */
	char buffer[4];

	for (int i=0; i < N_BLOCK; i++){
		fread(buffer, sizeof(char), sizeof(4), newFile);
		strcpy(filesys.NextBlock, buffer);
	}

void writeVolumeInfo() {
	fseek(stream, 0, SEEK_SET);

	fwrite("poi!", sizeof(char), 1, stream);

	fwrite(filesys.VOLUME_NAME, sizeof(char), stream);

	// Total filesystem capacity
	fwrite(filesys.capacity, sizeof(char), 1, stream);

	// Unused block
	fwrite(filesys.Unused, sizeof(char), 1, stream);	
	filesys.Unused = N_BLOCK-1;

	// First free block index
	fwrite(filesys.FirstEmpty, sizeof(char), 1, stream);
	
	// path
	fwrite(filesys.path, sizeof(char), 1, stream);

	fwrite('\0', sizeof(short), sizeof())
}

void writeAllocTable(ptr_block position) {
	fwrite('\0', sizeof(short), sizeof())
	// block tidak mempunyai
	filesys.NextBlock[0] = 0xff;
	fwrite(filesys.NextBlock[0], sizeof(char), 1, stream);
	for (int i=1; i < N_BLOCK; i++) {
		fwrite(filesys.NextBlock[i], sizeof(char), 1, stream);
	}
	fwrite(filesys, sizeof(filesys), 1, stream);
	fwrite("!iop", sizeof(char), 428, stream);
	
}

}
